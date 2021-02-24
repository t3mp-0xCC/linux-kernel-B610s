/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/dcache.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/freezer.h>
#include <linux/utsname.h>
#include "f_mass_storage_api.h"

#include "hw_pnp_api.h"
#include "gadget_chips.h"
#include "usb_nv_get.h"
#include "hw_pnp.h"
#include "android.h"

#include "usb_fast_on_off.h"

static const USB_CHAR fsg_string_interface[] = "Mass Storage";

#include "storage_common.c"

/********************************************************************************/
static USB_VOID fsg_channel_disconnect(struct fsg_common* common);
static USB_INT fsg_channel_connect(struct fsg_common* common);
static USB_VOID fsg_start_rx(struct fsg_common* common);
static USB_VOID fsg_start_tx(struct fsg_common* common, USB_INT length, USB_INT zero_flag);
USB_VOID mass_storage_dump(USB_VOID);
/*********************************************************************************/

USB_INT g_scsi_stat = 0;  // 0表示定时器到期后不做处理
sd_card_info g_sd_card_info =
{
    .sd_card_status     = SD_CARD_NOT_PRESENT,
    .sd_card_workmode   = SD_WORKMODE_MASS,
};

#include "f_scsi.c"

#define FSG_DRIVER_DESC		"Mass Storage Function"
#define FSG_DRIVER_VERSION	"GADGET 2.0"

static struct android_usb_function* g_enabled_mass_function_list[SUPPORT_MASS_FUNCTION_NUM] = {NULL};
static struct mass_storage_status_info g_support_mass_functions_lun_info[SUPPORT_MASS_FUNCTION_NUM] =
{
    {{LUN_NONE}, "mass"},
    {{LUN_NONE}, "mass_two"}
};

static struct timer_list g_soft_mass_timer ;



/*****************************************************************************
 Prototype    : soft_mass_handler
 Description  : 为了规避在某些系统的bioss对scsi的处理不标准，进不了系统
   当scsi 在 Get Max Lun 之后超过2秒没有scsi交互，则做拉低
   D+，10秒后再拉高的操作。
 Input        : USB_ULONG data
 Output       : None
 Return Value : USB_VOID
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/24
    Author       : a
    Modification : Created function

*****************************************************************************/
USB_VOID soft_mass_handler( USB_ULONG data )
{
    struct fsg_common* mass_common = NULL;
    struct mass_storage_function_config_data* configdata = NULL;

    if (1 == g_scsi_stat) // 如果是0则直接返回，表示已经有scsi交互
    {
        DBG_T(MBB_PNP, " soft_mass_handler  scsi ok \n");
        return;
    }

    if (g_enabled_mass_function_list[0])
    {
        configdata = g_enabled_mass_function_list[0]->config;
        if (NULL != configdata)
        {
            mass_common =  configdata->common;
        }
    }
    if (NULL != mass_common && NULL != mass_common->gadget)
    {
        if ( 0 == data )
        {
            usb_gadget_disconnect(mass_common->gadget);
            g_soft_mass_timer.data = 1;
            mod_timer(&g_soft_mass_timer, jiffies + msecs_to_jiffies(10000));
        }

        if ( 1 == data )
        {
            usb_gadget_connect(mass_common->gadget);
        }
    }

    DBG_T(MBB_MASS, " soft_mass_handler process! \n");
}
#define fsg_cnt(a,b) ((a > b)?(a-b):0)
static ssize_t fsg_show_state(struct device* dev, struct device_attribute* attr,
                              USB_CHAR* buf)
{
    size_t count = 0;
    struct fsg_common* common = NULL;
    struct mass_storage_function_config_data* configdata = NULL;
    struct fsg_state_count* fsg_state_for_common = NULL;
    if (dev->platform_data)
    {
        configdata = dev->platform_data;
        fsg_state_for_common = &configdata->fsg_state;
        common = configdata->common;
    }
    else
    {
        DBG_T(MBB_MASS, "dev/%p, dev->platform_data/%p\n", dev, dev->platform_data);
        return 0;
    }
    count = snprintf(buf, MAX_SHOW_LEN, "\n");

    count += snprintf(buf, fsg_cnt(MAX_SHOW_LEN,count), "| |--file storage dump:common for %s\n", dev->kobj.name);

    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--bulk in name-------------%s\n",
                      common->fsg ?
                      common->fsg->bulk_in->name : "");
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--bulk out name------------%s\n",
                      common->fsg ?
                      common->fsg->bulk_out->name : "");

    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count),"| |--fsg command phase--------%d\n", fsg_state_for_common->command_phase);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg data phase-----------%d\n", fsg_state_for_common->data_phase);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg status phase---------%d\n", fsg_state_for_common->status_phase);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg idle-----------------%d\n", fsg_state_for_common->idle);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg abort_bulk_out-------%d\n", fsg_state_for_common->abort_bulk_out);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg reset----------------%d\n", fsg_state_for_common->reset);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg interface change-----%d\n", fsg_state_for_common->interface_change);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg config change--------%d\n", fsg_state_for_common->config_change);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg disconnect-----------%d\n", fsg_state_for_common->disconnect);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg exit-----------------%d\n", fsg_state_for_common->exit);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg terminated-----------%d\n", fsg_state_for_common->terminated);

    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg phase error----------%d\n", fsg_state_for_common->phase_error);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg command failure------%d\n", fsg_state_for_common->command_failure);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg cbw------------------%d\n", fsg_state_for_common->cbw);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg invalid cbw----------%d\n", fsg_state_for_common->invalid_cbw);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg non meaningful cbw---%d\n", fsg_state_for_common->non_meaningful_cbw);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg lun values not consistent----%d\n", fsg_state_for_common->lun_values_not_consistent);

    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg unknown cbw----------%d\n", fsg_state_for_common->unknown_cbw);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg data-----------------%d\n", fsg_state_for_common->data);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg csw------------------%d\n", fsg_state_for_common->csw);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg invalid csw----------%d\n", fsg_state_for_common->invalid_csw);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg non meaningful csw---%d\n", fsg_state_for_common->non_meaningful_csw);

    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg bulk in set halt-----%d\n", fsg_state_for_common->bulk_in_set_halt);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg bulk out set halt----%d\n", fsg_state_for_common->bulk_out_set_halt);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg bulk in set wedge----%d\n", fsg_state_for_common->bulk_in_set_wedge);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg signal pending-------%d\n", fsg_state_for_common->signal_pending);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg signal_pending_line--%d\n", fsg_state_for_common->signal_pending_line);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "| |--fsg short packet received----%d\n", fsg_state_for_common->short_packet_received);
    count += snprintf(buf + count, fsg_cnt(MAX_SHOW_LEN,count), "\n");
    return count;
}
static ssize_t mass_storage_dump_show(struct device* dev, struct device_attribute* attr, USB_CHAR* buf)
{
    mass_storage_dump();
    return 1;
}
/*************************** DEVICE ATTRIBUTES ***************************/

/* Write permission is checked per LUN in store_*() functions. */
static DEVICE_ATTR(ro, S_IRUGO | S_IWUSR, fsg_show_ro, fsg_store_ro);
static DEVICE_ATTR(nofua, S_IRUGO | S_IWUSR, fsg_show_nofua, fsg_store_nofua);
static DEVICE_ATTR(file, S_IRUGO | S_IWUSR, fsg_show_file, fsg_store_file);
static DEVICE_ATTR(mode, S_IRUGO | S_IWUSR, fsg_show_mode, fsg_store_mode);
static DEVICE_ATTR(fsg_state, S_IRUGO, fsg_show_state, NULL);
static DEVICE_ATTR(mass_dump, S_IRUGO, mass_storage_dump_show, NULL);
/****************************** FSG COMMON ******************************/
#define USB_SD
#ifdef USB_SD
USB_INT mass_storage_open_usb_sd(USB_VOID)
{
    struct mass_storage_function_config_data* configdata = NULL;
    struct fsg_common* common = NULL;
    struct fsg_lun*     curlun = NULL;
    USB_CHAR    filename[32];
    USB_INT     openlun = 0;
    USB_INT     ret     = -1;
    USB_INT     i       = 0;
    USB_INT     j       = 0;
    USB_INT     idex;

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        if (NULL != g_enabled_mass_function_list[i])
        {
            configdata = g_enabled_mass_function_list[i]->config;
            if ( NULL == configdata)
            {
                DBG_E(MBB_MASS, " configdata NULL  \n");
                return ret;
            }

            common = configdata->common;
            if ( NULL == common)
            {
                DBG_E(MBB_MASS, " common NULL  \n");
                return ret;
            }
        }
        else
        {
            continue;
        }

        for (j = 0; j < common->nluns; j++)
        {
            curlun = &common->luns[j];

            if ( NULL != curlun && !curlun->cdrom )
            {
                DBG_T(MBB_MASS, "sd_card_workmode %s\n",
                      curlun->mode ? "usb" : "web");
                DBG_T(MBB_MASS, "sd_card_status %s\n",
                      g_sd_card_info.sd_card_status ? "PRESENT" : "NOT_PRESENT");
            }

            if ( NULL != curlun && !curlun->cdrom
                 && SD_WORKMODE_MASS == curlun->mode
                 && SD_CARD_PRESENT == g_sd_card_info.sd_card_status)
            {
                idex =  usb_get_mmc_dev_idex();

                if (idex > -1)
                {
                    snprintf(filename, 30, "/dev/block/mmcblk%d", idex);
                }
                else
                {
                    DBG_T(MBB_MASS, "usb_get_mmc_dev_idex return err\n");
                    snprintf(filename, 30, SDCARD_MNT_DIR);
                }

                if (fsg_lun_is_open(curlun))
                {
                    DBG_T(MBB_MASS, "%s already open file: %s\n",
                          curlun->dev.kobj.name, filename);
                    ret = 0;
                }
                else
                {
                    openlun = fsg_lun_open(curlun, filename);

                    if ( 0 == openlun )
                    {
                        curlun->unit_attention_data =
                            SS_NOT_READY_TO_READY_TRANSITION;
                    }
                    else
                    {
                        DBG_T(MBB_MASS, "usb open sd\n");
                        ret = 0;
                    }
                }
            }
        }
    }

    return ret;
}

USB_INT mass_storage_close_usb_sd(USB_VOID)
{
    struct fsg_lun* curlun = NULL;
    USB_INT ret = -1;
    USB_INT i = 0;
    USB_INT j = 0;
    struct fsg_common* common = NULL;

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        struct mass_storage_function_config_data* configdata = NULL;

        if (NULL != g_enabled_mass_function_list[i])
        {
            configdata = g_enabled_mass_function_list[i]->config;
            if ( NULL == configdata)
            {
                DBG_E(MBB_MASS, " configdata NULL  \n");
                return ret;
            }

            common = configdata->common;
            if ( NULL == common)
            {
                DBG_E(MBB_MASS, " common NULL  \n");
                return ret;
            }
        }
        else
        {
            continue;
        }

        for (j = 0; j < common->nluns; j++)
        {
            curlun = &common->luns[j];

            if (NULL != curlun && !curlun->cdrom)
            {
                if (fsg_lun_is_open(curlun))
                {
                    fsg_lun_close(curlun);
                    DBG_T(MBB_MASS, "usb close sd\n");
                    ret = 0;
                    return ret;
                }
            }
        }
    }

    return ret;
}

USB_INT mass_storage_usb_sd_is_open(USB_VOID)
{
    struct fsg_lun* curlun = NULL;
    USB_INT ret = -1;
    USB_INT i = 0;
    USB_INT j = 0;
    struct fsg_common* common = NULL;

    DBG_T(MBB_MASS, "\n");
    DBG_T(MBB_MASS, "sd_card_status %s\n", g_sd_card_info.sd_card_status ? "PRESENT " : "NOT_PRESENT ");

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        struct mass_storage_function_config_data* configdata = NULL;

        if (NULL != g_enabled_mass_function_list[i])
        {
            configdata = g_enabled_mass_function_list[i]->config;
            if ( NULL == configdata)
            {
                DBG_E(MBB_MASS, " configdata NULL  \n");
                return ret;
            }

            common = configdata->common;
            if ( NULL == common)
            {
                DBG_E(MBB_MASS, " common NULL  \n");
                return ret;
            }
        }
        else
        {
            continue;
        }

        for (j = 0; j < common->nluns; j++)
        {
            curlun = &common->luns[j];

            if ( NULL != curlun && !curlun->cdrom )
            {
                if (fsg_lun_is_open(curlun))
                {
                    DBG_T(MBB_MASS, "usb_sd_is_open\n");
                    ret = 0;
                    return ret;
                }
            }
        }
    }

    DBG_T(MBB_MASS, "usb sd is not open\n");
    return ret;
}

USB_VOID mass_storage_set_sd_card_workmode(USB_INT mode)
{
    struct fsg_lun* curlun = NULL;
    USB_INT i = 0;
    USB_INT j = 0;
    struct fsg_common* common = NULL;

    DBG_T(MBB_MASS, "%s\n", mode ? "USB" : "WEB");
    switch (mode)
    {
        case SD_CARD_WEB_MODE:
            g_sd_card_info.sd_card_workmode = SD_WORKMODE_WEBNAS;
            break;
        case SD_CARD_USB_MODE:
            g_sd_card_info.sd_card_workmode = SD_WORKMODE_MASS;
            break;
        default :
            DBG_T(MBB_MASS, "err mode %d\n", mode);
            return;
    }

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        struct mass_storage_function_config_data* configdata = NULL;

        if (NULL != g_enabled_mass_function_list[i])
        {
            configdata = g_enabled_mass_function_list[i]->config;
            if ( NULL == configdata)
            {
                DBG_E(MBB_MASS, " configdata NULL  \n");
                return ;
            }

            common = configdata->common;
            if ( NULL == common)
            {
                DBG_E(MBB_MASS, " common NULL  \n");
                return ;
            }
        }
        else
        {
            continue;
        }

        for (j = 0; j < common->nluns; j++)
        {
            curlun = &common->luns[j];

            if ( NULL != curlun && !curlun->cdrom )
            {
                if ( SD_WORKMODE_WEBNAS == mode)
                {
                    curlun->mode = SD_CARD_WEB_MODE;
                }
                else if ( SD_WORKMODE_MASS == mode)
                {
                    curlun->mode = SD_CARD_USB_MODE;
                }
            }
        }
    }
}
USB_VOID mass_storage_set_sd_card_status(USB_INT sd_removed)
{
    switch (sd_removed)
    {
        case SD_CARD_NOT_PRESENT:
            /* SD卡拔出，将SD卡的filp指针赋空 */
            DBG_T(MBB_MASS, "sd_card removed usb need close sd\n");
            g_sd_card_info.sd_card_status = SD_CARD_NOT_PRESENT;
            break;

        case SD_CARD_PRESENT:
            /* SD卡插入，重新open sd卡分区 */
            DBG_T(MBB_MASS, "sd_card attach usb can open sd\n");
            g_sd_card_info.sd_card_status = SD_CARD_PRESENT;
            mass_storage_open_usb_sd();
            break;
        default:
            DBG_T(MBB_MASS, "unknown event\n");
            break;
    }

#ifdef  MBB_USB_RESERVED
    if (sd_removed)
    {
        /* SD卡拔出，将SD卡的filp指针赋空 */
        DBG_T(MBB_MASS, "sd_card removed usb need close sd\n");
        g_sd_card_info.sd_card_status = SD_CARD_NOT_PRESENT;
    }
    else
    {
        /* SD卡插入，重新open sd卡分区 */
        DBG_T(MBB_MASS, "sd_card attach usb can open sd\n");
        g_sd_card_info.sd_card_status = SD_CARD_PRESENT;
        mass_storage_open_usb_sd();
    }
#endif
}
EXPORT_SYMBOL(mass_storage_set_sd_card_status);
#endif

static USB_VOID fsg_common_release(struct kref* ref);

static USB_VOID fsg_lun_release(struct device* dev)
{
    /* Nothing needs to be done */
}

static inline USB_VOID fsg_common_get(struct fsg_common* common)
{
    kref_get(&common->ref);
}

static inline USB_VOID fsg_common_put(struct fsg_common* common)
{
    kref_put(&common->ref, fsg_common_release);
}
static inline struct fsg_dev* fsg_from_func(struct usb_function* f)
{
    return container_of(f, struct fsg_dev, function);
}

struct mass_storage_status_info* mass_find_function_by_name(USB_CHAR* func_name)
{
    USB_INT i = 0;
    USB_CHAR* mass_func_name = NULL;

    if ( NULL == func_name)
    {
        DBG_E(MBB_MASS, " func_name NULL  \n");
        return NULL;
    }

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        mass_func_name =
            g_support_mass_functions_lun_info[i].mass_func_name;

        if ( NULL != mass_func_name)
        {
            if (!strcmp(mass_func_name, func_name))
            {
                DBG_I(MBB_MASS, "find function:%s \n", func_name);
                return &(g_support_mass_functions_lun_info[i]);
            }
        }
    }

    if ( SUPPORT_MASS_FUNCTION_NUM == i)
    {
        DBG_E(MBB_MASS, " can not find function:%s \n", func_name);
    }
    return NULL;
}
/*****************************************************************************
 Prototype    : check_mass_lun_info_exist
 Description  : check have lun info or not
 Input          : function name
 Output       : None
 Return Value : MBB_USB_FALSE : exist   MBB_USB_TRUE  : no exist

  History        :
  1.Date         : 2014/11/18
    Author       :
    Modification : Created function

*****************************************************************************/
USB_INT  check_mass_lun_info_exist(USB_CHAR* func_name)
{
    USB_INT ret = MBB_USB_TRUE;
    mass_lun_info lun_type = LUN_NONE;
    struct mass_storage_status_info* mass_function = NULL;

    DBG_I(MBB_MASS, " enter\n");

    mass_function = mass_find_function_by_name(func_name);

    if (NULL == mass_function)
    {
        DBG_E(MBB_MASS, "mass_function NULL \n");
        return MBB_USB_FALSE;
    }

    lun_type = (USB_INT)mass_function->mass_luns[0];
    if ( LUN_CD != lun_type && LUN_SD != lun_type )
    {
        ret = MBB_USB_FALSE;
    }
    return ret;
}


/*if the PNP module do not config the lun info ,usb the default one*/
USB_VOID mass_function_default_lun_info(USB_CHAR* function_name)
{
    struct mass_storage_status_info* mass_function = NULL;
    USB_INT j = 0;

    DBG_I(MBB_MASS, "\n");

    mass_function = mass_find_function_by_name(function_name);

    if (NULL == mass_function || NULL == mass_function->mass_func_name)
    {
        DBG_E(MBB_MASS, "mass_function/%p or mass_function->mass_func_name NULL \n",mass_function);
        return;
    }

    if (!strcmp("mass", mass_function->mass_func_name))
    {
        mass_function->mass_luns[0] = LUN_CD;
        for (j = 1; j < FSG_MAX_LUNS; j++)
        {
            mass_function->mass_luns[j] = LUN_NONE;
        }
    }
    else if (!strcmp("mass_two", mass_function->mass_func_name))
    {
        mass_function->mass_luns[0] = LUN_SD;
        for (j = 1; j < FSG_MAX_LUNS; j++)
        {
            mass_function->mass_luns[j] = LUN_NONE;
        }
    }
    else
    {
        DBG_I(MBB_MASS, "do not support name '%s' \n", mass_function->mass_func_name);
    }
}


/*****************************************************************************
 Prototype    : mass_function_config_lun_info
 Description  : use the lun info
 Input        : USB_CHAR* function_name
                struct fsg_module_parameters* fsg_mod_data
 Output       : None
 Return Value : USB_VOID
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/19
    Author       :
    Modification : Created function

*****************************************************************************/
USB_VOID mass_function_config_lun_info(USB_CHAR* func_name,  struct fsg_module_parameters* fsg_mod_data)
{
    USB_INT i = 0;
    USB_INT lun_type = 0;
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    struct mass_storage_status_info* mass_function = NULL;

    DBG_I(MBB_MASS, "enter\n");
    mass_function = mass_find_function_by_name(func_name);

    if ( NULL == mass_function ||  NULL == fsg_mod_data)
    {
        DBG_E(MBB_MASS, "can not find mass_function  OR fsg_mod_data/%p  NULL \n", fsg_mod_data);
        return;
    }

    fsg_mod_data->luns  = 0;
    fsg_mod_data->stall = MBB_USB_YES;
    fsg_mod_data->ro_count = 0;
    fsg_mod_data->file_count = 0;
    fsg_mod_data->cdrom_count = 0;
    fsg_mod_data->nofua_count = 0;
    fsg_mod_data->removable_count = 0;

    if ( !check_mass_lun_info_exist(func_name))
    {
        mass_function_default_lun_info(func_name);
    }

    for (i = 0; i < FSG_MAX_LUNS; i++)
    {
        lun_type = (USB_INT)mass_function->mass_luns[i];
        switch (lun_type)
        {
            case LUN_CD:
                DBG_T(MBB_MASS, "lun[%d] CD\n", i);
                fsg_mod_data->luns++;
                fsg_mod_data->ro_count++;
                fsg_mod_data->cdrom_count++;
                fsg_mod_data->nofua_count++;
                fsg_mod_data->removable_count++;
                /*if the lun is cdrom*/
                fsg_mod_data->cdrom[i]     = MBB_USB_YES;
                /*if the lun can be removed*/
                fsg_mod_data->removable[i] = MBB_USB_YES;
                /*the wokemode of the lun*/
                fsg_mod_data->mode[i]      = SD_WORKMODE_MASS;
                /*if the lun is read-only*/
                fsg_mod_data->ro[i]        = MBB_USB_YES;
                /*if the lun is noFUA*/
                fsg_mod_data->nofua[i]     = MBB_USB_NO;
                break;
            case LUN_SD:
                DBG_T(MBB_MASS, "lun[%d] SD\n", i);
                fsg_mod_data->luns++;
                fsg_mod_data->nofua_count++;
                fsg_mod_data->removable_count++;
                /*if the lun is cdrom*/
                fsg_mod_data->cdrom[i]     = MBB_USB_NO;
                /*if the lun can be removed*/
                fsg_mod_data->removable[i] = MBB_USB_YES;
                /*the wokemode of the lun*/
                fsg_mod_data->mode[i]      = usb_nv_info->ulSDWorkMode;
                /*if the lun is read-only*/
                fsg_mod_data->ro[i]        = MBB_USB_NO;
                /*if the lun is noFUA*/
                fsg_mod_data->nofua[i]     = MBB_USB_NO;
                break;
            default:
                DBG_I(MBB_MASS, "%s config end with lun[%d] \n", func_name, i);
                return;
        }
    }
}

/*****************************************************************************
 Prototype    : fsg_config_from_params
 Description  :
 Input        : struct fsg_config* cfgconfig
                const struct fsg_module_parameters* params
                struct mass_storage_function_config_data* privatedata
 Output       : None
 Return Value :
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/19
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_VOID fsg_config_from_params(struct fsg_config* cfgconfig,
                                       const struct fsg_module_parameters* params,
                                       struct mass_storage_function_config_data* configdata)
{
    struct fsg_lun_config* lun_cfg;
    USB_UINT i = 0;

    if ( NULL == cfgconfig || NULL == params || NULL == configdata)
    {
        DBG_E(MBB_MASS, "cfgconfig/%p OR params/%p OR configdata/%p NULL \n",
              cfgconfig, params, configdata);
        return;
    }

    /* Configure LUNs */
    cfgconfig->nluns = min(params->luns ? : (params->file_count ? : 1u), (USB_UINT)FSG_MAX_LUNS);

    for (i = 0, lun_cfg = cfgconfig->luns; i < cfgconfig->nluns; ++i, ++lun_cfg)
    {
        lun_cfg->ro = !!params->ro[i];
        lun_cfg->cdrom = !!params->cdrom[i];
        lun_cfg->mode = !!params->mode[i];
        lun_cfg->removable = /* Removable by default */
            params->removable_count <= i || params->removable[i];
        lun_cfg->filename = params->file_count > i && params->file[i][0] ? params->file[i] : 0;
    }

    /* Let MSF use defaults */
    cfgconfig->lun_name_format = 0;
    cfgconfig->thread_name = 0;
    cfgconfig->vendor_name = 0;
    cfgconfig->product_name = 0;
    cfgconfig->release = 0xffff;
    cfgconfig->ops = NULL;
    cfgconfig->private_data = configdata;
    /* Finalise */
    cfgconfig->can_stall = params->stall;
}


static struct usb_request* fsg_alloc_req(struct usb_ep* ep , unsigned size)
{
    struct usb_request* req;
    req = usb_ep_alloc_request(ep, GFP_ATOMIC);
    if (!req)
    {
        DBG_T(MBB_MASS, "fsg alloc req failed\n");
        return NULL;
    }
    req->length = size;
    return req;
}

static int fsg_alloc_requests(struct usb_ep* ep , struct list_head* head, int num, int size,
                              void (*cb) (struct usb_ep* ep , struct usb_request* req) , void* context)
{
    int i = 0;
    struct usb_request* req;

    DBG_I(MBB_MASS, "ep:%p head:%p num:%d size:%d cb:%p",
          ep, head, num, size, cb);

    for (i = 0; i < num; i++)
    {
        req = fsg_alloc_req(ep, size);
        if (!req)
        {
            pr_debug("%s: req allocated:%d\n", __func__, i);
            return list_empty(head) ? -ENOMEM : 0;
        }
        req->context = context;
        req->complete = cb;
        list_add(&req->list, head);
    }

    return 0;
}

static USB_VOID fsg_clear_port(struct fsg_common* common)
{
    struct fsg_lun*		curlun;
    USB_INT i = 0;
    DBG_I(MBB_MASS, "enter ! \n");
    //fsg_thread_exit(common);
    /* Clear out the controller's fifos */
    if (common->fsg->bulk_in_enabled)
    { usb_ep_fifo_flush(common->fsg->bulk_in); }
    if (common->fsg->bulk_out_enabled)
    { usb_ep_fifo_flush(common->fsg->bulk_out); }

    for (i = 0; i < (common->nluns); ++i)
    {
        curlun = &(common->luns[i]);
        curlun->prevent_medium_removal = 0;
        curlun->sense_data = SS_NO_SENSE;
        curlun->unit_attention_data = SS_NO_SENSE;
        curlun->sense_data_info = 0;
        curlun->info_valid = 0;
    }

}
/*****************************************************************************
 Prototype    : fsg_register_lun_drivers
 Description  :  Create the LUNs, open their backing files, and register the LUN devices in sysfs
 Input        :   struct fsg_common* common
                    struct fsg_config* cfgconfig
                    struct device*  mass_dev
 Output       : None
 Return Value : USB_INT
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/20
    Author       :
    Modification : Created function

*****************************************************************************/
USB_INT  fsg_register_lun_drivers(
    struct fsg_common* common, struct fsg_config* cfgconfig ,
    struct device*  mass_dev)
{
    struct mass_storage_function_config_data*    configdata = NULL;
    struct android_usb_function*  lunparent     = NULL;
    mbb_usb_nv_info_st*  usb_nv_info = NULL;
    struct fsg_lun*          curlun;
    struct fsg_lun_config*   lun_cfg;
    USB_CHAR    filename[FILE_NAME_LEN];
    USB_CHAR    version[USB_NUM_4] = {0};
    USB_PCHAR   pathbuf;
    USB_INT i      = 0;
    USB_INT nluns  = 0;
    USB_INT rc     = 0;
    USB_INT idex   = 0;
    usb_pnp_api_hanlder* pnp_api_handler = pnp_adp_get_api_handler();

    DBG_I(MBB_MASS, " enter\n");

    if (NULL == common || NULL == cfgconfig || NULL == mass_dev)
    {
        DBG_E(MBB_MASS, "common/%p ,cfgconfig/%p,mass_dev/%p maybe NULL! \n",
              common, cfgconfig, mass_dev);
        return -EINVAL;
    }

    if (NULL == cfgconfig->private_data)
    {
        DBG_E(MBB_MASS, "cfgconfig->private_data NULL! \n" );
        return -EINVAL;
    }
    else
    {
        configdata  = cfgconfig->private_data;
    }

    if (NULL == configdata ->parent)
    {
        DBG_E(MBB_MASS, "configdata->parent NULL! \n" );
        return -EINVAL;
    }
    else
    {
        lunparent   = configdata ->parent;
    }

    usb_nv_info = usb_nv_get_ctx();

    /* Find out how many LUNs there should be */
    nluns = cfgconfig->nluns;
    curlun = kcalloc(nluns, sizeof(*curlun), GFP_KERNEL);
    if (unlikely(!curlun))
    {
        DBG_E(MBB_MASS, "fail to kzalloc curlun\n");
        return -ENOMEM;
    }
    DBG_I(MBB_MASS, "curlun %p\n", curlun);

    common->luns = curlun;
    init_rwsem(&common->filesem);

    for (i = 0, lun_cfg = cfgconfig->luns; i < nluns; ++i, ++curlun, ++lun_cfg)
    {
        curlun->cdrom = !!lun_cfg->cdrom;
        curlun->ro = lun_cfg->cdrom || lun_cfg->ro;
        curlun->initially_ro = curlun->ro;
        curlun->removable = lun_cfg->removable;
        curlun->mode = lun_cfg->mode;
        curlun->dev.release = fsg_lun_release;
        curlun->dev.parent = mass_dev;
        dev_set_drvdata(&curlun->dev, &common->filesem);
        dev_set_name(&curlun->dev,
                     cfgconfig->lun_name_format
                     ? cfgconfig->lun_name_format
                     : "lun%d_%s",
                     i, (curlun->cdrom ? "CD" : "SD"));

        rc = device_register(&curlun->dev);
        DBG_I(MBB_MASS, "register device LUN%d: %s\n", i, curlun->dev.kobj.name);

        if (rc)
        {
            DBG_E(MBB_MASS, "failed to register device %s: %d\n", curlun->dev.kobj.name, rc);
            common->nluns = i;
            put_device(&curlun->dev);
            return rc;
        }

        rc = device_create_file(&curlun->dev, &dev_attr_ro);
        DBG_I(MBB_MASS, "create_file ro for %s \n", curlun->dev.kobj.name);

        if (rc)
        {
            DBG_E(MBB_MASS, "fail to create_file ro for %s \n", curlun->dev.kobj.name);
            common->nluns = i + 1;
            return rc;
        }

        rc = device_create_file(&curlun->dev, &dev_attr_file);
        DBG_I(MBB_MASS, "create_file file for %s \n", curlun->dev.kobj.name);

        if (rc)
        {
            DBG_E(MBB_MASS, "fail to create_file file for %s \n", curlun->dev.kobj.name);
            common->nluns = i + 1;
            return rc;
        }

        rc = device_create_file(&curlun->dev, &dev_attr_nofua);
        DBG_I(MBB_MASS, "create_file nofua for %s \n", curlun->dev.kobj.name);

        if (rc)
        {
            DBG_E(MBB_MASS, "fail to create_file nofua for %s \n", curlun->dev.kobj.name);
            common->nluns = i + 1;
            return rc;
        }

        rc = device_create_file(&curlun->dev, &dev_attr_mode);
        DBG_I(MBB_MASS, "create_file mode for %s \n", curlun->dev.kobj.name);

        if (rc)
        {
            DBG_E(MBB_MASS, "fail to create_file mode for %s \n", curlun->dev.kobj.name);
            common->nluns = i + 1;
            return rc;
        }

        /*link luns infomations to android_usb_functions like  'f_mass'*/
        rc = sysfs_create_link(&lunparent->dev->kobj, &curlun->dev.kobj, curlun->dev.kobj.name);
        DBG_I(MBB_MASS, "create_link 'lun' sysfs  for %s \n", curlun->dev.kobj.name);

        if (rc)
        {
            DBG_E(MBB_MASS, "fail to create_link 'lun' sysfs  for %s \n", curlun->dev.kobj.name);
            return rc;
        }

        memset(filename, 0, FILE_NAME_LEN);
        idex  = 0;

        /*打开CD*/
        if (curlun->cdrom && pnp_api_handler->pnp_if_cdrom_can_open_cb)
        {
            if (pnp_api_handler->pnp_if_cdrom_can_open_cb())
            {
                /*USB init 比 sysfs init早，数据卡和E5插USB开机.靠sysfs init调用脚本
                 *mass_boot.sh打开光盘
                 *E5非插USB开机fsg_lun_open 会返回成功*/
                (USB_VOID)fsg_lun_open(curlun, CDROM_MNT_DIR);
            }
        }

        /*打开SD*/
#ifdef USB_SD
        if (!curlun->cdrom)
        {
            if (SD_WORKMODE_MASS == curlun->mode)
            {
                DBG_I(MBB_MASS, "usb have condition to open sd\n");
                idex =  usb_get_mmc_dev_idex();

                if (idex > -1)
                {
                    snprintf(filename, 30, "dev/block/mmcblk%d", idex);
                }
                else
                {
                    DBG_T(MBB_MASS, "usb_get_mmc_dev_idex return err\n");
                    snprintf(filename, 30, "dev/block/mmcblk0");
                }
            }

            if (filename[0])
            {   /*fsg_lun_open 无法构造返回值，UT不进行错误分支测试。*/
                rc = fsg_lun_open(curlun, filename);

                if (rc && !lun_cfg->cdrom)
                {
                    if (idex > -1)
                    {
                        snprintf(filename, 30, "dev/block/mmcblk%dp1", idex);
                    }
                    else
                    {
                        snprintf(filename, 30, "dev/block/mmcblk0p1");
                    }
                    (USB_VOID)fsg_lun_open(curlun, filename);
                }
            }
        }
#endif

        if (lun_cfg->filename)
        {
            /*usb not conect*/
        }
        else if (!curlun->removable)
        {
            DBG_E(MBB_MASS, "you configure removable for LUN%d not correct\n", i);
            common->nluns = i + 1;
            return -EINVAL;
        }

        if (curlun->cdrom)
        {
            cfgconfig->vendor_name = usb_nv_info->mass_dynamic_name.huawei_cdrom_dynamic_name;
            cfgconfig->product_name = usb_nv_info->mass_dynamic_name.huawei_cdrom_dynamic_name + USB_NUM_8;
            cfgconfig->release = get_unaligned_be16(usb_nv_info->mass_dynamic_name.huawei_cdrom_dynamic_name + USB_NUM_24);
            memcpy(version, usb_nv_info->mass_dynamic_name.huawei_cdrom_dynamic_name + USB_NUM_24, USB_NUM_4);
        }
        else
        {
            cfgconfig->vendor_name = usb_nv_info->mass_dynamic_name.huawei_sd_dynamic_name;
            cfgconfig->product_name = usb_nv_info->mass_dynamic_name.huawei_sd_dynamic_name + USB_NUM_8;
            cfgconfig->release = get_unaligned_be16(usb_nv_info->mass_dynamic_name.huawei_sd_dynamic_name + USB_NUM_24);
            memcpy(version, usb_nv_info->mass_dynamic_name.huawei_sd_dynamic_name + USB_NUM_24, USB_NUM_4);
        }

        snprintf(curlun->inquiry_string, sizeof curlun->inquiry_string,
                 "%-8s%-16s%-4s\n", cfgconfig->vendor_name ? : "Linux",
                 /* Assume product name dependent on the curlun */
                 cfgconfig->product_name ? : (curlun->cdrom ? "File-CD Gadget" : "File-Stor Gadget" ),
                 version);


    }
    common->nluns = nluns;

    pathbuf = kmalloc(PATH_MAX, GFP_KERNEL);

    for (i = 0, nluns = common->nluns, curlun = common->luns; i < nluns; ++curlun, ++i)
    {
        USB_PCHAR p = "(no medium)";

        if (fsg_lun_is_open(curlun))
        {
            p = "(medium)";

            if (pathbuf)
            {
                p = d_path(&curlun->filp->f_path, pathbuf, PATH_MAX);

                if (IS_ERR(p))
                {
                    p = "(error)";
                }
            }
        }

        DBG_T(MBB_MASS, "LUN%d: %s%s%s%sfile: %s\n", i,
              curlun->removable ? "removable;" : "",
              curlun->ro ? "read only;" : "RW;",
              curlun->cdrom ? "CD-ROM;" : "SD;",
              curlun->mode ? "usb mode;" : "web mode;",
              p);
    }

    kfree(pathbuf);
    return 0;
}


/*****************************************************************************
 Prototype    : fsg_common_init
 Description  : fsg_common init & register luns driver
 Input        : struct fsg_common* common
                struct usb_composite_dev* cdev
                struct fsg_config* cfgconfig
 Output       : None
 Return Value : static struct fsg_common*
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/19
    Author       :
    Modification : Created function

*****************************************************************************/
static struct fsg_common* fsg_common_init(struct fsg_common* common, struct usb_composite_dev* cdev,
        struct fsg_config* cfgconfig)
{
    struct mass_storage_function_config_data*    configdata = NULL;
    struct usb_gadget*               gadget = NULL;
    struct android_usb_function*  lunparent     = NULL;
    struct device*                     mass_dev    = NULL;
    USB_INT     nluns = 0;
    USB_INT rc = 0;

    DBG_I(MBB_MASS, "in\n");

    if ( NULL == cdev || NULL == cfgconfig)
    {
        DBG_E(MBB_MASS, "cdev /%p ,cfgconfig/%p maybe NULL !!\n", cdev, cfgconfig);
        return ERR_PTR(-EINVAL);
    }

    if ( NULL == cdev->gadget)
    {
        DBG_E(MBB_MASS, "cdev->gadget NULL !!\n");
        return ERR_PTR(-EINVAL);
    }
    else
    {
        gadget = cdev->gadget;
    }

    configdata = cfgconfig->private_data;

    lunparent = configdata->parent;

    rc = fsg_num_buffers_validate();

    if (rc != 0)
    {
        return ERR_PTR(rc);
    }

    /* Find out how many LUNs there should be */
    nluns = cfgconfig->nluns;
    if (nluns < 1 || nluns > FSG_MAX_LUNS)
    {
        DBG_E(MBB_MASS, "invalid number of LUNs: %u error!!\n", nluns);
        return ERR_PTR(-EINVAL);
    }

    /* Allocate? */
    if (!common)
    {
        common = kzalloc(sizeof ( struct fsg_common), GFP_KERNEL);

        if (!common)
        {
            DBG_T(MBB_MASS, "fail to kzalloc common \n");
            return ERR_PTR(-ENOMEM);
        }

        common->free_storage_on_release = 1;
    }
    else
    {
        memset(common, 0, sizeof ( struct fsg_common));
        common->free_storage_on_release = 0;
    }

    /*for sysfile*/
    common->ops     = cfgconfig->ops;
    common->gadget  = gadget;
    common->ep0     = gadget->ep0;
    common->ep0req  = cdev->req;
    common->cdev    = cdev;
    common->private_data = configdata;

    /*alloc buff*/
    common->rx_buf = kmalloc(FSG_BUFLEN, GFP_KERNEL);
    if (unlikely(!common->rx_buf))
    {
        kfree(common);
        return ERR_PTR(-ENOMEM);
    }
    common->buf = kmalloc(FSG_BUFLEN, GFP_KERNEL);
    /*common->buf 代表data TX buff*/
    if (unlikely(!common->buf))
    {
        kfree(common->rx_buf);
        kfree(common);
        return ERR_PTR(-ENOMEM);
    }
    common->csw_buf = kmalloc(US_BULK_CS_WRAP_LEN , GFP_KERNEL);
    if (unlikely(!common->csw_buf))
    {
        kfree(common->buf);
        kfree(common->rx_buf);
        kfree(common);
        return ERR_PTR(-ENOMEM);
    }

    /* Maybe allocate device-global string IDs, and patch descriptors */
    if (fsg_strings[FSG_STRING_INTERFACE].id == 0)
    {
        rc = usb_string_id(cdev);
        DBG_I(MBB_MASS, "usb_string_id %d\n", rc);
        if (unlikely(rc < 0))
        { goto error_release; }

        fsg_strings[FSG_STRING_INTERFACE].id = rc;
        fsg_intf_desc.iInterface = rc;
    }

    /*create sysfs mass dev for multi interface*/
    mass_dev = kzalloc(sizeof (struct device), GFP_KERNEL);
    if (!mass_dev)
    {
        DBG_E(MBB_MASS, "fail to kzalloc mass_dev \n");
        goto error_release;
    }
    DBG_I(MBB_MASS, "mass_dev %p\n", mass_dev);
    mass_dev->parent = &gadget->dev;
    mass_dev->release = fsg_lun_release;
    mass_dev->platform_data = configdata;
    dev_set_name(mass_dev, "%s", lunparent->name);
    rc = device_register(mass_dev);
    DBG_I(MBB_MASS, "register device %s\n", mass_dev->kobj.name);

    if (rc)
    {
        DBG_E(MBB_MASS, "failed to register device %s %d\n", mass_dev->kobj.name, rc);
        put_device(mass_dev);
        kfree(mass_dev);
        mass_dev = NULL;
        goto error_release;
    }

    /*create fsg_state att for mass dev*/
    rc = device_create_file(mass_dev, &dev_attr_fsg_state);
    DBG_I(MBB_MASS, "create_file fsg_state for %s \n", mass_dev->kobj.name);

    if (rc)
    {
        DBG_E(MBB_MASS, "fail to create_file fsg_state for %s \n", mass_dev->kobj.name);
        goto error_release;
    }

    /*register lun drivers*/
    rc = fsg_register_lun_drivers( common, cfgconfig , mass_dev);
    if (rc)
    {
        DBG_T(MBB_MASS, "register lun drivers failed\n");
        goto error_release;
    }
    spin_lock_init(&common->rx_lock);
    spin_lock_init(&common->tx_lock);
    INIT_LIST_HEAD(&common->rx_req_pool);
    INIT_LIST_HEAD(&common->tx_req_pool);
    INIT_LIST_HEAD(&common->rx_queue);
    INIT_LIST_HEAD(&common->rx_pool);
    INIT_LIST_HEAD(&common->tx_pool);
    /*
     * Some peripheral controllers are known not to be able to
     * halt bulk endpoints correctly.  If one of them is present,
     * disable stalls.
     */
    common->can_stall = cfgconfig->can_stall && !(gadget_is_at91(common->gadget));

    spin_lock_init(&common->lock);

    kref_init(&common->ref);

    /* Tell the thread to start working */
    common->thread_task = kthread_create(fsg_rw_thread, common,
                                         cfgconfig->thread_name ? : "file-storage");

    if (IS_ERR(common->thread_task))
    {
        rc = PTR_ERR(common->thread_task);
        goto error_release;
    }

    init_completion(&common->thread_notifier);
    init_waitqueue_head(&common->fsg_wait);

    /* Information */
    INFO(common, FSG_DRIVER_DESC ", version: " FSG_DRIVER_VERSION "\n");
    INFO(common, "Number of LUNs=%d\n", common->nluns);

    DBG_I(MBB_MASS, "I/O thread pid: %d\n", task_pid_nr(common->thread_task));

    wake_up_process(common->thread_task);
    return common;

error_release:
    fsg_common_release(&common->ref);
    if (NULL !=mass_dev)
    {
        kfree(mass_dev);
        mass_dev=NULL;
    }
    DBG_E(MBB_MASS, "fsg_common_init error\n");
    return ERR_PTR(rc);
}
/*****************************************************************************
 Prototype    : fsg_common_release
 Description  :
 Input        : struct kref* ref
 Output       : None
 Return Value : static USB_VOID
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/20
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_VOID fsg_common_release(struct kref* ref)
{
    struct fsg_common* common = container_of(ref, struct fsg_common, ref);
    struct mass_storage_function_config_data* configdata = NULL;
    struct android_usb_function* f = NULL;
    USB_INT i = 0;

    if (likely(common->luns))
    {
        struct fsg_lun* lun = common->luns;
        struct device* luns_parent = common->luns->dev.parent;
        i = common->nluns;

        if (likely(common->private_data))
        {
            configdata = common->private_data;
            f = configdata->parent;
        }

        /* In error recovery common->nluns may be zero. */
        for (; i; --i, ++lun)
        {
            if (NULL != f)
            {
                DBG_I(MBB_MASS, "%s remove_link%s\n", f->name, lun->dev.kobj.name);
                sysfs_remove_link(&f->dev->kobj, lun->dev.kobj.name);
            }

            DBG_I(MBB_MASS, "%s remove_file nofua\n", lun->dev.kobj.name);
            device_remove_file(&lun->dev, &dev_attr_nofua);

            DBG_I(MBB_MASS, "%s remove_file ro\n", lun->dev.kobj.name);
            device_remove_file(&lun->dev, &dev_attr_ro);

            DBG_I(MBB_MASS, "%s remove_file file\n", lun->dev.kobj.name);
            device_remove_file(&lun->dev, &dev_attr_file);

            DBG_I(MBB_MASS, "%s remove_file mode\n", lun->dev.kobj.name);
            device_remove_file(&lun->dev, &dev_attr_mode);
            fsg_lun_close(lun);

            DBG_I(MBB_MASS, "unregister%s\n", lun->dev.kobj.name);
            device_unregister(&lun->dev);
        }

        DBG_I(MBB_MASS, "%s remove_file  fsg_state\n", luns_parent->kobj.name);
        device_remove_file(luns_parent, &dev_attr_fsg_state);

        DBG_I(MBB_MASS, "unregister %s\n", luns_parent->kobj.name);
        device_unregister(luns_parent);

        kfree(common->luns);

        kfree(luns_parent);
    }

    kfree(common->buf);
    kfree(common->rx_buf);
    kfree(common->csw_buf);

    if (common->free_storage_on_release)
    {
        kfree(common);
    }

}

/*-------------------------------------------------------------------------*/

static USB_INT ep0_queue(struct fsg_common* common)
{
    USB_INT	rc;

    rc = usb_ep_queue(common->ep0, common->ep0req, GFP_ATOMIC);
    common->ep0->driver_data = common;

    if (rc != 0 && rc != -ESHUTDOWN)
    {
        /* We can't do much more than wait for a reset */
        DBG_W(MBB_MASS, "error in submission: %s --> %d\n",
              common->ep0->name, rc);
    }

    return rc;
}

/*****************************************************************************
 Prototype    : fsg_bind
 Description  :
 Input        : struct usb_configuration* c
                struct usb_function* f
 Output       : None
 Return Value : static USB_INT
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/20
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_INT fsg_bind(struct usb_configuration* c, struct usb_function* f)
{
    struct fsg_dev*		fsg = NULL;
    struct usb_gadget*	gadget = NULL;
    USB_INT			interface_id = 0;
    struct usb_ep*		ep;

    if (NULL == c || NULL == f)
    {
        DBG_E(MBB_MASS, "c/%p ,f/%p NULL!!\n", c , f);
        return -EINVAL;
    }

    fsg = fsg_from_func(f);

    if (c->cdev && c->cdev->gadget)
    {
        gadget = c->cdev->gadget;
    }
    else
    {
        DBG_E(MBB_MASS, "c->cdev/%p OR c->cdev->gadget NULL!!\n", c->cdev);
        return -EINVAL;
    }

    fsg->gadget = gadget;

    /* New interface */
    interface_id = usb_interface_id(c, f);

    if (interface_id < 0)
    {
        return interface_id;
    }

    fsg_intf_desc.bInterfaceNumber = interface_id;
    fsg->interface_number = interface_id;

    /* Find all the endpoints we will use */
    ep = usb_ep_autoconfig(gadget, &fsg_fs_bulk_in_desc);

    if (!ep)
    {
        goto autoconf_fail;
    }

    ep->driver_data = fsg->common;	/* claim the endpoint */
    fsg->bulk_in = ep;

    ep = usb_ep_autoconfig(gadget, &fsg_fs_bulk_out_desc);

    if (!ep)
    {
        goto autoconf_fail;
    }

    ep->driver_data = fsg->common;	/* claim the endpoint */
    fsg->bulk_out = ep;

    /* Copy descriptors */
    f->fs_descriptors = usb_copy_descriptors(fsg_fs_function);
    if (unlikely(!f->fs_descriptors))
    {
        return -ENOMEM;
    }

    if (gadget_is_dualspeed(gadget))
    {
        /* Assume endpoint addresses are the same for both speeds */
        fsg_hs_bulk_in_desc.bEndpointAddress = fsg_fs_bulk_in_desc.bEndpointAddress;
        fsg_hs_bulk_out_desc.bEndpointAddress = fsg_fs_bulk_out_desc.bEndpointAddress;
        f->hs_descriptors = usb_copy_descriptors(fsg_hs_function);

        if (unlikely(!f->hs_descriptors))
        {
            usb_free_descriptors(f->fs_descriptors);
            return -ENOMEM;
        }
    }

    if (gadget_is_superspeed(gadget))
    {
        USB_UINT	max_burst;

        /* Calculate bMaxBurst, we know packet size is 1024 */
        max_burst = min_t(USB_UINT, FSG_BUFLEN / 1024, 15);

        fsg_ss_bulk_in_desc.bEndpointAddress = fsg_fs_bulk_in_desc.bEndpointAddress;
        fsg_ss_bulk_in_comp_desc.bMaxBurst = max_burst;

        fsg_ss_bulk_out_desc.bEndpointAddress = fsg_fs_bulk_out_desc.bEndpointAddress;
        fsg_ss_bulk_out_comp_desc.bMaxBurst = max_burst;

        f->ss_descriptors = usb_copy_descriptors(fsg_ss_function);

        if (unlikely(!f->ss_descriptors))
        {
            usb_free_descriptors(f->hs_descriptors);
            usb_free_descriptors(f->fs_descriptors);
            return -ENOMEM;
        }
    }
    DBG_I(MBB_MASS, "bind ep finished!\n");
    return 0;

autoconf_fail:
    DBG_E(MBB_MASS, "unable to autoconfigure all endpoints\n");
    return -ENOTSUPP;
}

/*****************************************************************************
 Prototype    : fsg_unbind
 Description  :
 Input        : struct usb_configuration* c
                struct usb_function* f
 Output       : None
 Return Value : static USB_VOID
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/20
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_VOID fsg_unbind(struct usb_configuration* c, struct usb_function* f)
{
    struct fsg_dev*		fsg = NULL;
    struct fsg_common*	common = NULL;
    USB_INT ret = 0;

    DBG_T(MBB_MASS, "\n");

    if (NULL == c || NULL == f)
    {
        DBG_E(MBB_MASS, "c/%p ,f/%p NULL!!\n", c , f);
        return ;
    }
    fsg = fsg_from_func(f);
    common = fsg->common;
#ifdef MBB_USB_UNITARY_Q
    DBG_T(MBB_MASS, "qc_pm_runtime_get_sync first\n");
    qc_pm_runtime_get_sync();
#endif



    /* If the thread isn't already dead, tell it to exit now */
    if (common->file_op != FSG_EXIT)
    {
        fsg_thread_exit(common);
        wait_for_completion(&common->thread_notifier);
    }


    if (NULL != common->thread_task)
    {
        ret = kthread_stop(common->thread_task);
        DBG_I(MBB_MASS, "thread function has run %ds\n", ret);
    }
    else
    {
        common->thread_task = NULL;
    }

    if (0 != ret)
    {
        DBG_T(MBB_MASS, "thread function has run %ds\n", ret);
    }
    if (common->fsg == fsg)
    {
        fsg_channel_disconnect(fsg->common);
    }
#ifdef MBB_USB_UNITARY_Q
    DBG_T(MBB_MASS, "qc_pm_runtime_put_sync\n");
    qc_pm_runtime_put_sync();
#endif
    usb_free_descriptors(fsg->function.fs_descriptors);
    usb_free_descriptors(fsg->function.hs_descriptors);
    usb_free_descriptors(fsg->function.ss_descriptors);
    kfree(fsg);
    DBG_I(MBB_MASS, "fsg_unbind success \n");
}
/*****************************************************************************
 Prototype    : fsg_setup
 Description  :
 Input        : struct usb_function* f
                const struct usb_ctrlrequest* ctrl
 Output       : None
 Return Value : static USB_INT
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/20
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_INT fsg_setup(struct usb_function* f, const struct usb_ctrlrequest* ctrl)
{
    struct fsg_dev*		fsg = NULL;
    struct usb_request*	req = NULL;
    USB_UINT16			w_index = 0;
    USB_UINT16			w_value = 0;
    USB_UINT16			w_length = 0;
    struct usb_request*	request;
#ifdef USB_RNDIS
    usb_pnp_api_hanlder* pnp_api_handler = pnp_adp_get_api_handler();
    USB_INT is_rewind_before = MBB_USB_FALSE;
    USB_INT is_service_switch = MBB_USB_FALSE;
#endif
    DBG_T(MBB_MASS, "fsg_setup\n");


    if (NULL == ctrl || NULL == f)
    {
        DBG_E(MBB_MASS, "ctrl/%p ,f/%p NULL!!\n", ctrl , f);
        return -EOPNOTSUPP;
    }

    fsg = fsg_from_func(f);

    req = fsg->common->ep0req;
    w_index = le16_to_cpu(ctrl->wIndex);
    w_value = le16_to_cpu(ctrl->wValue);
    w_length = le16_to_cpu(ctrl->wLength);

    if (!fsg_is_set(fsg->common))
    { return -EOPNOTSUPP; }

    ++fsg->common->ep0_req_tag;	/* Record arrival of a new request */
    req->context = NULL;
    req->length = 0;
    dump_msg(fsg, "ep0-setup", (USB_PUINT8) ctrl, sizeof(*ctrl));

    switch (ctrl->bRequest)
    {

        case US_BULK_RESET_REQUEST:
            DBG_T(MBB_MASS, "bulk reset request\n");
            if (ctrl->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            {
                break;
            }

            if (w_index != fsg->interface_number || w_value != 0 || w_length != 0)
            {
                return -EDOM;
            }
            /*
             * Raise an exception to stop the current operation
             * and reinitialize our state.
             */
#if 1
            list_for_each_entry(request, &fsg->common->rx_req_pool, list)
            {
                usb_ep_dequeue(fsg->bulk_out, request);
            }
            list_for_each_entry(request, &fsg->common->tx_req_pool, list)
            {
                usb_ep_dequeue(fsg->bulk_in, request);
            }
#endif
            fsg_clear_port(fsg->common);
            if (!fsg_is_set(fsg->common))
            { break; }

            if (test_and_clear_bit(IGNORE_BULK_OUT, &fsg->atomic_bitflags))
            {
                usb_ep_clear_halt(fsg->bulk_in);
            }
            return 0;
        case US_BULK_GET_MAX_LUN:
            if (ctrl->bRequestType != (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            {
                break;
            }

            if (w_index != fsg->interface_number || w_value != 0 || w_length != 1)
            {
                return -EDOM;
            }

            *(USB_PUINT8)req->buf = fsg->common->nluns - 1;

            DBG_T(MBB_MASS, "get max LUN %d\n", fsg->common->nluns);
            /* Respond with data/status */
            req->length = min((USB_UINT16)1, w_length);

            /*  HILINK 切换  */
#ifdef USB_RNDIS
            if (pnp_api_handler->pnp_is_rewind_before_mode_cb
                && pnp_api_handler->pnp_is_service_switch_cb)
            {
                is_rewind_before = pnp_api_handler->pnp_is_rewind_before_mode_cb();
                is_service_switch = pnp_api_handler->pnp_is_service_switch_cb();
                if (is_rewind_before && is_service_switch)
                {
                    if (pnp_api_handler->pnp_switch_rewind_after_mode_cb)
                    {
                        DBG_T(MBB_MASS, "GET MAX LUN REWIND\n");
                        pnp_api_handler->pnp_switch_rewind_after_mode_cb();
                    }
                }
            }
#endif
            g_scsi_stat = 0;
            /*2s没有SCSI交互规避bios卡住问题*/
            mod_timer(&g_soft_mass_timer, jiffies + msecs_to_jiffies(2000));
            return ep0_queue(fsg->common);
    }

    DBG_T(MBB_MASS, "unknown class-specific control req %02x.%02x v%04x i%04x l%u\n",
          ctrl->bRequestType, ctrl->bRequest, le16_to_cpu(ctrl->wValue), w_index, w_length);
    return -EOPNOTSUPP;
}

/*****************************************************************************
 Prototype    : fsg_set_alt
 Description  :
 Input        : struct usb_function* f
                USB_UINT intf
                USB_UINT alt
 Output       : None
 Return Value : static USB_INT
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/20
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_INT fsg_set_alt(struct usb_function* f, USB_UINT intf, USB_UINT alt)
{
    USB_INT rc = 0;
    struct fsg_dev* fsg = NULL;
    DBG_T(MBB_MASS, "fsg_set_alt \n");
    if ( NULL == f )
    {
        DBG_E(MBB_MASS, "f NULL \n");
        return -EOPNOTSUPP;
    }
    fsg = fsg_from_func(f);

    fsg_channel_disconnect(fsg->common);
    fsg->common->fsg = fsg;
    rc = fsg_channel_connect(fsg->common);
    if (rc)
    {
        DBG_T(MBB_MASS, "fsg enable ep failed \n");
    }
    fsg_start_rx(fsg->common);
    return rc;
}

/*****************************************************************************
 Prototype    : fsg_disable
 Description  :
 Input        : struct usb_function* f
 Output       : None
 Return Value : static USB_VOID
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/20
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_VOID fsg_disable(struct usb_function* f)
{
    struct fsg_dev* fsg = NULL;
    DBG_T(MBB_MASS, "fsg_disable \n");

    if (NULL == f)
    {
        DBG_E(MBB_MASS, "f NULL\n");
        return;
    }

    fsg = fsg_from_func(f);
    fsg_channel_disconnect(fsg->common);
}

/****************************** ADD FUNCTION ******************************/

static struct usb_gadget_strings* fsg_strings_array[] =
{
    &fsg_stringtab,
    NULL,
};

void fsg_resume(struct usb_function *func)
{
    struct fsg_dev* fsg = NULL;
    fsg = fsg_from_func(func);
    DBG_I(MBB_MASS, "fsg_resume\n");
    if(NULL != fsg)
    {
        fsg_start_rx(fsg->common);
    }
}

/*****************************************************************************
 Prototype    : fsg_bind_config
 Description  :
 Input        : struct usb_composite_dev* cdev
                struct usb_configuration* c
                struct fsg_common* common
 Output       : None
 Return Value : static USB_INT
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/20
    Author       :
    Modification : Created function

*****************************************************************************/
static USB_INT fsg_bind_config(struct usb_composite_dev* cdev, struct usb_configuration* c,
                               struct fsg_common* common)
{
    struct fsg_dev* fsg;
    USB_INT rc;
    struct mass_storage_function_config_data* configdata = NULL;
    struct android_usb_function* fsgparent = NULL;

    if (NULL == common || NULL == c)
    {
        DBG_E(MBB_MASS, "common/%p OR c/%p NULL\n", common, c);
        return -EINVAL;
    }

    fsg = kzalloc(sizeof * fsg, GFP_KERNEL);
    if (unlikely(!fsg))
    {
        DBG_E(MBB_MASS, "fail to kzalloc fsg \n");
        return -ENOMEM;
    }

    configdata = common->private_data;
    fsgparent = configdata->parent;

    fsg->function.name        = FSG_DRIVER_DESC;
    fsg->function.strings     = fsg_strings_array;
    fsg->function.bind        = fsg_bind;
    fsg->function.unbind      = fsg_unbind;
    fsg->function.setup       = fsg_setup;
    fsg->function.set_alt     = fsg_set_alt;
    fsg->function.disable     = fsg_disable;
    fsg->function.resume     = fsg_resume;

    fsg->common               = common;
    /*
     * Our caller holds a reference to common structure so we
     * don't have to be worry about it being freed until we return
     * from this function.  So instead of incrementing counter now
     * and decrement in error recovery we increment it only when
     * call to usb_add_function() was successful.
     */

    rc = usb_add_function(c, &fsg->function);

    if (unlikely(rc))
    {
        DBG_E(MBB_MASS, "fail to add_function %s type %s\n", fsgparent->name, fsg->function.name);
        kfree(fsg);
    }
    else
    {
        fsg_common_get(fsg->common);
    }
    DBG_I(MBB_MASS, "bind config success \n");
    return rc;
}

static USB_INT fsg_unbind_config(struct usb_composite_dev* cdev,
                                 struct usb_configuration* c,
                                 struct fsg_common* common)
{

    return 0;
}

static inline struct fsg_common* fsg_common_config(
    struct android_usb_function* f , struct fsg_common* common,
    struct usb_composite_dev* cdev)
__attribute__((unused));

static inline struct fsg_common* fsg_common_config(
    struct android_usb_function* f , struct fsg_common* common,
    struct usb_composite_dev* cdev)
{
    struct mass_storage_function_config_data* configdata = f->config;
    struct fsg_config* cfgconfig = &(configdata->fsgconfig);
    struct fsg_module_parameters fsg_mod_data;

    memset(&fsg_mod_data, 0, sizeof (struct fsg_module_parameters));

    mass_function_config_lun_info(f->name, &fsg_mod_data);
    fsg_config_from_params(cfgconfig, &fsg_mod_data, configdata);
    return fsg_common_init(common, cdev, cfgconfig);
}

/*****************************************************************************
 Prototype    : mass_save_enabled_function
 Description  : save the availability func
 Input        : struct android_usb_function* enabled_function
 Output       : None
 Return Value : USB_INT
 Calls        :
 Called By    :

  History        :
  1.Date         : 2014/11/19
    Author       :
    Modification : Created function

*****************************************************************************/
USB_INT mass_save_enabled_function(struct android_usb_function* enabled_function)
{
    USB_INT i = 0;

    if ( NULL == enabled_function)
    {
        DBG_E(MBB_MASS, "enabled_function IS NULL\n");
        return MBB_USB_ERROR;
    }

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        if (enabled_function == g_enabled_mass_function_list[i])
        {
            DBG_I(MBB_MASS, "already enabled \n");
            break;
        }
        if (NULL == g_enabled_mass_function_list[i])
        {
            g_enabled_mass_function_list[i] = enabled_function;
            break;
        }
    }

    if (SUPPORT_MASS_FUNCTION_NUM == i)
    {
        DBG_E(MBB_MASS, "function_list IS FULL\n");
        return MBB_USB_ERROR;
    }

    return MBB_USB_OK;
}
USB_VOID mass_del_enabled_function(struct android_usb_function* del_function)
{
    USB_INT i = 0;

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        if (del_function == g_enabled_mass_function_list[i])
        {
            g_enabled_mass_function_list[i] = NULL;
        }
    }
}
/*****************************************************************************

    Prototype    : mass_storage_function_init

    Description  :

*****************************************************************************/
static USB_INT mass_storage_function_init(struct android_usb_function* f, struct usb_composite_dev* cdev)
{
    struct mass_storage_function_config_data* config;

    if ( NULL ==  f )
    {
        DBG_E(MBB_MASS, "f IS NULL\n");
        return -1;
    }

    config = kzalloc(sizeof(struct mass_storage_function_config_data), GFP_KERNEL);

    if (!config)
    {
        return -ENOMEM;
    }

    f->config = config;
    config->parent = f;
    return 0;
}

/*****************************************************************************

    Prototype    : mass_storage_function_cleanup

    Description  :

*****************************************************************************/
static USB_VOID mass_storage_function_cleanup(struct android_usb_function* f)
{
    if ( NULL == f )
    {
        DBG_E(MBB_MASS, "f NULL \n");
        return;
    }

    if ( NULL == f->config )
    {
        DBG_I(MBB_MASS, "f->config NULL \n");
        return;
    }

    kfree(f->config);
    f->config = NULL;
}

/*****************************************************************************

    Prototype    : mass_storage_function_bind_config

    Description  :

*****************************************************************************/
static USB_INT mass_storage_function_bind_config(
    struct android_usb_function* f, struct usb_configuration* c)
{
    struct mass_storage_function_config_data* configdata = NULL;
    struct fsg_common* common = NULL;
    USB_INT status = 0;
    USB_PVOID retp;
    usb_pnp_api_hanlder* pnp_api_handler = pnp_adp_get_api_handler();

    USB_INT is_multi_lun = MBB_USB_FALSE;

    if ( NULL == f || NULL == c )
    {
        DBG_E(MBB_MASS, "f/%p OR c/%p NULL \n", f , c);
        return -1;
    }

    configdata = f->config;

    if (pnp_api_handler->pnp_is_multi_lun_mode_cb)
    {
        is_multi_lun = pnp_api_handler->pnp_is_multi_lun_mode_cb();
    }
    else
    {
        DBG_I(MBB_MASS, "pnp_is_multi_lun_mode_cb NULL \n");
    }

    DBG_I(MBB_MASS, " enter\n");

    if (NULL == configdata )
    {
        DBG_T(MBB_MASS, "configdata/%p NULL", configdata );
        DBG_T(MBB_MASS, "bind --%-10s to config --%-25s failed\n",
              f->name, c->label);
        return  -1;
    }

    if (NULL != configdata->common)
    {
        DBG_T(MBB_MASS, "configdata->common/%p not NULL\n", configdata->common);
        DBG_T(MBB_MASS, "bind --%-10s to config --%-25s failed\n",
              f->name, c->label);
        return  -1;
    }

    if (!is_multi_lun || !check_mass_lun_info_exist(f->name))
    {
        mass_function_default_lun_info(f->name);
    }

    retp = fsg_common_config(f, common, c->cdev);
    if (IS_ERR(retp))
    {
        DBG_T(MBB_MASS, "fsg_common return error retp=%p\n", retp);
        DBG_T(MBB_MASS, "bind --%-10s to config --%-25s failed\n",
              f->name, c->label);

        status = PTR_ERR(retp);
        if (status)
        {
            //kfree(configdata);
            return status;
        }
    }
    configdata->common = retp;
    mass_save_enabled_function(f);
    return fsg_bind_config(c->cdev, c, configdata->common);
}

/*cleanup the func lun info when unbind func*/
USB_VOID mass_function_cleanup_lun_info(USB_CHAR* function_name)
{
    USB_INT j = 0;
    struct mass_storage_status_info* mass_function = NULL;
    DBG_I(MBB_MASS, "\n");

    mass_function = mass_find_function_by_name(function_name);

    if ( NULL != mass_function)
    {
        for (j = 0; j < FSG_MAX_LUNS; j++)
        {
            mass_function->mass_luns[j] = LUN_NONE;
        }
    }
}

/*used in pnp*/
USB_VOID mass_cleanup_alllun_info(USB_VOID)
{
    USB_INT i = 0;
    USB_CHAR* mass_func_name = NULL;
    DBG_I(MBB_MASS, "\n");
    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        mass_func_name = g_support_mass_functions_lun_info[i].mass_func_name;
        mass_function_cleanup_lun_info(mass_func_name);
    }
}
EXPORT_SYMBOL_GPL(mass_cleanup_alllun_info);

/*add a new lun info to func*/
USB_VOID mass_add_lun2func(mass_lun_info* mass_luns, USB_CHAR* addname)
{
    USB_INT i = 0;

    if ( NULL == addname || NULL == mass_luns)
    {
        DBG_E(MBB_MASS, " addname/%s OR mass_luns%p NULL  \n", addname, mass_luns);
        return ;
    }

    for (i = 0; i < FSG_MAX_LUNS; i++)
    {
        if (LUN_NONE == mass_luns[i])
        {
            if (!strcmp("CD", addname))
            {
                DBG_T(MBB_MASS, "add %s\n", addname);
                mass_luns[i] = LUN_CD;
            }
            else if (!strcmp("SD", addname))
            {
                DBG_T(MBB_MASS, "add %s\n", addname);
                mass_luns[i] = LUN_SD;
            }
            else
            {
                DBG_T(MBB_MASS, "err lun name %s\n", addname);
            }

            break;
        }
    }
}

/*the PNP module to add lun info*/
USB_VOID mass_function_add_lun(USB_CHAR* function_name, USB_CHAR* addname)
{
    struct mass_storage_status_info* mass_function = NULL;

    mass_function = mass_find_function_by_name(function_name);

    if ( NULL != mass_function)
    {
        mass_add_lun2func(mass_function->mass_luns, addname);
    }
    else
    {
        DBG_T(MBB_MASS, "can not find %s\n", function_name);
    }
}
EXPORT_SYMBOL_GPL(mass_function_add_lun);

static USB_VOID mass_function_unbind_config(struct android_usb_function* f, struct usb_configuration* c)
{
    struct mass_storage_function_config_data* configdata = f->config;
    struct fsg_lun* curlun;

    curlun = configdata->common->luns;
    fsg_unbind_config(c->cdev, c, configdata->common);

    fsg_common_put(configdata->common);
    /* Call fsg_common_release() directly, ref might be not initialised. */
    fsg_common_release(&configdata->common->ref);

    memset(&configdata->fsgconfig, 0, sizeof (struct fsg_config));
    memset(&configdata->fsg_state, 0, sizeof (struct fsg_state_count));
    configdata->common = NULL;

    mass_del_enabled_function(f);
    mass_function_cleanup_lun_info(f->name);
}


/******************************************************/


static void fsg_free_requests(struct usb_ep* ep, struct list_head* head)
{
    struct usb_request*	req;

    while (!list_empty(head))
    {
        req = list_entry(head->next, struct usb_request, list);
        list_del(&req->list);
        req->context = NULL;
        usb_ep_free_request(ep, req);
    }
}

static USB_VOID fsg_start_rx(struct fsg_common* common)
{
    unsigned long flags;
    USB_INT ret = 0 ;
    struct usb_request* req;
    DBG_I(MBB_MASS, "enter\n");

    if ( NULL == common )
    {
        DBG_E(MBB_MASS, "common NULL \n" );
        return;
    }

    spin_lock_irqsave(&common->rx_lock,flags);
    if (list_empty(&common->rx_req_pool))
    {
        spin_unlock_irqrestore(&common->rx_lock,flags);
        return;
    }    
    req = list_entry(common->rx_req_pool.next, struct usb_request, list);
    req->length = FSG_BUFLEN;
    req->buf = common->rx_buf;
    list_del(&req->list);
    spin_unlock_irqrestore(&common->rx_lock,flags);
    ret = usb_ep_queue(common->fsg->bulk_out, req, GFP_ATOMIC);
    
    if (ret)
    {
        spin_lock_irqsave(&common->rx_lock,flags);
        list_add_tail(&req->list, &common->rx_req_pool);
        spin_unlock_irqrestore(&common->rx_lock,flags);
    }
}

static USB_VOID fsg_start_tx(struct fsg_common* common, USB_INT length, USB_INT zero_flag)
{
    unsigned long flags;
    USB_INT ret = 0 ;
    struct usb_request* req;
    if (0 == length ||  NULL == common )
    {
        DBG_I(MBB_MASS, " length/%d = 0 common/%p NULL \n", length, common);
        return ;
    }
    spin_lock_irqsave(&common->tx_lock,flags);
    if (list_empty(&common->tx_req_pool))
    {
        spin_unlock_irqrestore(&common->tx_lock,flags);
        return;
    }    

    req = list_entry(common->tx_req_pool.next, struct usb_request, list);
    list_del(&req->list);
    req->length = length;
    req->buf = common->buf;
    req->zero = zero_flag;
    spin_unlock_irqrestore(&common->tx_lock,flags);
    ret = usb_ep_queue(common->fsg->bulk_in, req, GFP_ATOMIC);
    if (ret)
    {
        DBG_T(MBB_MASS, "tx failed\n");
        spin_lock_irqsave(&common->tx_lock,flags);
        list_add_tail(&req->list, &common->tx_req_pool);
        spin_unlock_irqrestore(&common->tx_lock,flags);
    }
    return ;
}
/*-------------------------------------------------------------------------*/

static USB_INT check_valid_cbw(struct fsg_common*	common, struct usb_request* req)
{
    struct bulk_cb_wrap*	cbw = req->buf;
    struct fsg_dev* fsg = common->fsg;
    struct mass_storage_function_config_data* configdata = fsg->common->private_data;
    struct fsg_state_count* fsg_state_for_common = &configdata->fsg_state;
    if (req->status || test_bit(IGNORE_BULK_OUT, &fsg->atomic_bitflags))
    {
        return -EINVAL;
    }

    /* Is the CBW valid? */
    if (req->actual != US_BULK_CB_WRAP_LEN || cbw->Signature != cpu_to_le32(US_BULK_CB_SIGN))
    {
        DBG_T(MBB_MASS, "invalid CBW: len %u sig 0x%x\n", req->actual, le32_to_cpu(cbw->Signature));
        wedge_bulk_in_endpoint(fsg);
        set_bit(IGNORE_BULK_OUT, &fsg->atomic_bitflags);
        fsg_state_for_common->invalid_cbw++;
        return -EINVAL;
    }

    /* Is the CBW meaningful? */
    if (cbw->Lun >= FSG_MAX_LUNS || cbw->Flags & ~US_BULK_FLAG_IN ||
        cbw->Length > MAX_COMMAND_SIZE)
    {
        DBG_T(MBB_MASS, "non-meaningful CBW: lun = %u, flags = 0x%x, "
              "cmdlen %u\n", cbw->Lun, cbw->Flags, cbw->Length);
        fsg_state_for_common->non_meaningful_cbw++;
        if (common->can_stall)
        {
            fsg_set_halt(fsg, fsg->bulk_out);
            halt_bulk_in_endpoint(fsg);
        }
        return -EINVAL;
    }

    /* Save the command for later */
    common->cmnd_size = MAX_COMMAND_SIZE;
    memcpy(common->cmnd, cbw->CDB, common->cmnd_size);

    if (cbw->Flags & US_BULK_FLAG_IN)
    { common->data_dir = DATA_DIR_TO_HOST; }
    else
    { common->data_dir = DATA_DIR_FROM_HOST; }

    common->data_size = le32_to_cpu(cbw->DataTransferLength);

    if (common->data_size == 0)
    { common->data_dir = DATA_DIR_NONE; }

    common->lun = cbw->Lun;

    if (common->lun < common->nluns)
    { common->curlun = &common->luns[common->lun]; }
    else
    { common->curlun = NULL; }

    common->tag = cbw->Tag;   /* unique per command id */
    return 0;
}

static USB_VOID fsg_bulk_in_complete(struct usb_ep* ep, struct usb_request* req)
{
    struct fsg_common*	common = ep->driver_data;
    if (req->status || req->actual != req->length)
    {
        DBG_T(MBB_MASS, "%d, %u/%u\n", req->status, req->actual, req->length);
    }

    if (req->status == -ECONNRESET)		/* Request was cancelled */
    {
        DBG_T(MBB_MASS, "req->status == -ECONNRESET\n");
        usb_ep_fifo_flush(ep);
    }
    smp_wmb();
    spin_lock(&common->tx_lock);
    memset(req->buf , 0 , req->actual);
    req->actual = 0;
    list_add_tail(&req->list, &common->tx_req_pool);
    spin_unlock(&common->tx_lock);
}

static USB_VOID fsg_bulk_out_complete(struct usb_ep* ep, struct usb_request* req)
{
    struct fsg_common*	common = ep->driver_data;
    USB_INT ret = 0;

    if (req->actual == US_BULK_CB_WRAP_LEN )
    {
        dump_msg(common, "bulk-out", req->buf, req->actual);
    }

    if (req->status == -ECONNRESET)		/* Request was cancelled */
    {
        DBG_T(MBB_MASS, "req->status == -ECONNRESET\n");
        usb_ep_fifo_flush(ep);
    }
    if (req->status)
    {
        DBG_T(MBB_MASS, "%d, %u\n", req->status, req->actual);
        req->actual = 0;
        spin_lock(&common->rx_lock);
        memset(common->rx_buf, 0 , FSG_BUFLEN);
        list_add_tail(&req->list, &common->rx_req_pool);
        spin_unlock(&common->rx_lock);
        return;
    }
    /*开始接受处理命令*/
    smp_wmb();
    ret = fsg_is_set(common) ? check_valid_cbw(common, req) : -EIO;
    memset(req->buf , 0 , req->actual);
    req->actual = 0;
    spin_lock(&common->rx_lock);
    list_add_tail(&req->list, &common->rx_req_pool);
    spin_unlock(&common->rx_lock);
    if (ret)
    {
        DBG_T(MBB_MASS, " invalid CBW ,err cord %d\n", ret);
        fsg_start_rx(common);
        return;
    }
    fsg_cmd_handle(common);
}

static USB_VOID fsg_channel_disconnect(struct fsg_common* common)
{
    struct fsg_dev* fsg;
    /* Deallocate the requests */
    if (common->fsg)
    {
        DBG_I(MBB_MASS, "enter ! \n");
        fsg_clear_port(common);
        fsg = common->fsg;
        /* Disable the endpoints */
        if (fsg->bulk_in_enabled)
        {
            usb_ep_disable(fsg->bulk_in);
            fsg->bulk_in_enabled = 0;
            fsg->bulk_in->driver_data = NULL;
        }

        if (fsg->bulk_out_enabled)
        {
            usb_ep_disable(fsg->bulk_out);
            fsg->bulk_out_enabled = 0;
            fsg->bulk_out->driver_data = NULL;
        }
        //spin_lock_irq(&common->tx_lock);
        if (fsg->bulk_in)
        {
            fsg_free_requests(fsg->bulk_in, &common->tx_req_pool);
            free_rw_requests(fsg->bulk_in, &common->tx_pool);
        }
        if (fsg->bulk_out)
        {
            fsg_free_requests(fsg->bulk_out, &common->rx_req_pool);
            free_rw_requests(fsg->bulk_out, &common->rx_pool);
            free_rw_requests(fsg->bulk_out, &common->rx_queue);
            if (common->throw_data_req && common->throw_data_req->buf)
            {
                kfree(common->throw_data_req->buf);
                common->throw_data_req->buf = NULL;
            }
            usb_ep_free_request(fsg->bulk_out, common->throw_data_req);
        }
        //spin_unlock_irq(&common->tx_lock);
        common->fsg = NULL;
        common->running = 0;
    }
}

static USB_INT fsg_channel_connect(struct fsg_common* common)
{
    struct fsg_dev* fsg;
    void * req_buf;
    USB_INT rc = 0;
    int i = 0;
    DBG_I(MBB_MASS, "enter ! \n");

    if (NULL == common)
    {
        DBG_T(MBB_MASS, " common NULL\n");
        return -1;
    }
    /* Enable the endpoints */
    fsg = common->fsg;

    if (NULL == fsg)
    {
        DBG_T(MBB_MASS, " fsg NULL\n");
        return -1;
    }

    rc = config_ep_by_speed(common->gadget, &(fsg->function), fsg->bulk_in);
    if (rc)
    {
        return rc;
    }

    rc = usb_ep_enable(fsg->bulk_in);
    if (rc)
    {
        return rc;
    }

    fsg->bulk_in->driver_data = common;
    fsg->bulk_in_enabled = 1;

    rc = config_ep_by_speed(common->gadget, &(fsg->function), fsg->bulk_out);
    if (rc)
    {
        return rc;
    }

    rc = usb_ep_enable(fsg->bulk_out);
    if (rc)
    {
        return rc;
    }

    fsg->bulk_out->driver_data = common;
    fsg->bulk_out_enabled = 1;
    common->bulk_out_maxpacket = usb_endpoint_maxp(fsg->bulk_out->desc);
    clear_bit(IGNORE_BULK_OUT, &fsg->atomic_bitflags);

    rc = fsg_alloc_requests(fsg->bulk_in, &fsg->common->tx_req_pool, FSG_QUEUE_SIZE, FSG_BUFLEN,
                            fsg_bulk_in_complete, (void*)fsg->common);
    if (rc)
    {
        DBG_E(MBB_MASS, "unable to allocate data in requests\n");
        return rc;
    }
    rc = fsg_alloc_requests(fsg->bulk_out, &fsg->common->rx_req_pool, FSG_RX_SIZE, FSG_BUFLEN,
                            fsg_bulk_out_complete, (void*)fsg->common);
    if (rc)
    {
        DBG_E(MBB_MASS, "unable to allocate data out requests\n");
        fsg_free_requests(fsg->bulk_in, &common->tx_req_pool);
        return rc;
    }
    rc = alloc_rw_requests(common->fsg->bulk_in, &common->tx_pool, FSG_QUEUE_SIZE, FSG_BUFLEN,
                           do_read_tx_complete, (void*)common);
    if (rc)
    {
        DBG_E(MBB_MASS, "unable to allocate data in requests\n");
        fsg_free_requests(fsg->bulk_in, &common->tx_req_pool);
        fsg_free_requests(fsg->bulk_out, &common->rx_req_pool);
        return rc;
    }

    rc = alloc_rw_requests(common->fsg->bulk_out, &common->rx_pool, FSG_QUEUE_SIZE, FSG_BUFLEN,
                           do_write_rx_complete, (void*)common);
    if (rc)
    {
        DBG_E(MBB_MASS, "unable to allocate data in requests\n");
        fsg_free_requests(fsg->bulk_in, &common->tx_req_pool);
        fsg_free_requests(fsg->bulk_out, &common->rx_req_pool);
        free_rw_requests(fsg->bulk_in, &common->tx_pool);
        return rc;
    }
    common->throw_data_req = fsg_alloc_req(common->fsg->bulk_out, FSG_BUFLEN);
    if (!common->throw_data_req )
    {
        DBG_E(MBB_MASS, "fsg_alloc_req err \n");
        fsg_free_requests(fsg->bulk_in, &common->tx_req_pool);
        fsg_free_requests(fsg->bulk_out, &common->rx_req_pool);
        free_rw_requests(fsg->bulk_in, &common->tx_pool);
        free_rw_requests(fsg->bulk_out, &common->rx_pool);
        return rc;
    }
    
    req_buf = kzalloc(FSG_BUFLEN, GFP_ATOMIC);
    if ( NULL == req_buf )
    {
        DBG_E(MBB_MASS, "req_buf null\n");
        fsg_free_requests(fsg->bulk_in, &common->tx_req_pool);
        fsg_free_requests(fsg->bulk_out, &common->rx_req_pool);
        free_rw_requests(fsg->bulk_in, &common->tx_pool);
        free_rw_requests(fsg->bulk_out, &common->rx_pool);
        return rc;
    }
    
    common->throw_data_req->context = common;
    common->throw_data_req->complete = bulk_out_to_throw_complete;
    common->throw_data_req->buf = req_buf;
    common->running = 1;
    common->file_op = FSG_IO_IDEL;

    for (i = 0; i < common->nluns; ++i)
    {
        common->luns[i].unit_attention_data = SS_RESET_OCCURRED;
    }
    return rc;

}

USB_VOID mass_add_to_support_func(USB_CHAR* function_name)
{
    USB_INT i = 0;
    USB_CHAR* mass_func_name = NULL;

    if (NULL == function_name)
    {
        DBG_E(MBB_MASS, "function_name NULL\n");
        return;
    }

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        mass_func_name = g_support_mass_functions_lun_info[i].mass_func_name;

        if (!mass_func_name)
        {
            DBG_I(MBB_MASS, "add support func %s\n", function_name);
            g_support_mass_functions_lun_info[i].mass_func_name = function_name;
            return;
        }
        else
        {
            if (!strcmp(mass_func_name, function_name))
            {
                DBG_I(MBB_MASS, "already add support func %s\n", function_name);
                return;
            }
        }
    }

    DBG_E(MBB_MASS, "can not add support func %s\n", function_name);
}

static struct device_attribute* mass_storage_function_attributes[] =
{
    &dev_attr_fsg_state,
    &dev_attr_mass_dump,
    NULL
};

static struct android_usb_function mass_storage_function_config_list[SUPPORT_MASS_FUNCTION_NUM] =
{
    {
        .name           = "mass",
        .init           = mass_storage_function_init,
        .cleanup        = mass_storage_function_cleanup,
        .bind_config    = mass_storage_function_bind_config,
        .unbind_config  = mass_function_unbind_config,
        .attributes     = mass_storage_function_attributes,
    },
    {
        .name           = "mass_two",
        .init           = mass_storage_function_init,
        .cleanup        = mass_storage_function_cleanup,
        .bind_config    = mass_storage_function_bind_config,
        .unbind_config  = mass_function_unbind_config,
        .attributes     = mass_storage_function_attributes,
    }
};

USB_VOID  mass_support_func_init( USB_VOID )
{
    USB_INT i = 0;
    USB_INT j = 0;

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        g_support_mass_functions_lun_info[i].mass_func_name = NULL;

        for (j = 0; j < FSG_MAX_LUNS; j++)
        {
            g_support_mass_functions_lun_info[i].mass_luns[j] = LUN_NONE;
        }
    }
}

USB_VOID usb_mass_storage_init( USB_VOID)
{
    USB_INT i = 0;
    DBG_I(MBB_MASS, " enter\n");
    mass_support_func_init();
    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        pnp_register_usb_support_function(&mass_storage_function_config_list[i]);
        mass_add_to_support_func(mass_storage_function_config_list[i].name);
    }
    setup_timer(&g_soft_mass_timer , soft_mass_handler, (USB_ULONG)0);
}

USB_VOID mass_storage_dump(USB_VOID)
{
    struct fsg_lun* curlun = NULL;
    USB_INT i = 0;
    USB_INT j = 0;
    USB_PCHAR p = "no medium";
    USB_PCHAR pathbuf = NULL;

    struct fsg_common* common = NULL;
    struct mass_storage_function_config_data* configdata = NULL;
    struct android_usb_function* f = NULL;

    for (i = 0; i < SUPPORT_MASS_FUNCTION_NUM; i++)
    {
        if (NULL != g_enabled_mass_function_list[i])
        {
            configdata = g_enabled_mass_function_list[i]->config;
            f = g_enabled_mass_function_list[i];
            if (NULL != configdata)
            {
                common = configdata->common;
            }
        }
        else
        {
            DBG_I(MBB_MASS, "g_enabled_mass_function_list[%d] %p \n", i, g_enabled_mass_function_list[i]);
            break;
        }

        DBG_T(MBB_DEBUG, "|--------------------------------------------------------------\n");
        DBG_T(MBB_DEBUG, "|--------mass_storage func '%s' dump msg--------------\n", f->name);
        DBG_T(MBB_DEBUG, "|--------------------------------------------------------------\n");
        if (NULL == common)
        {
            DBG_T(MBB_DEBUG, "|mass_storage func '%s' common null !\n", f->name);
            break;
        }
        curlun = common->curlun;

        if (NULL == curlun)
        {
            DBG_T(MBB_DEBUG, "|mass_storage func '%s' curlun null !\n", f->name);
            break;
        }
        if (NULL != curlun->dev.parent)
        {
            DBG_T(MBB_DEBUG, "|The name of---------- mass_storage is '%s'.\n",
                  curlun->dev.parent->kobj.name);
        }

        DBG_T(MBB_DEBUG, "|The nluns of--------- mass_storage is %d.\n", common->nluns);
        pathbuf = kmalloc(PATH_MAX, GFP_KERNEL);

        for (j = 0; j < common->nluns; j++)
        {
            curlun = &common->luns[j];

            if (fsg_lun_is_open(curlun))
            {
                p = "medium";

                if (pathbuf)
                {
                    p = d_path(&curlun->filp->f_path,
                               pathbuf, PATH_MAX);

                    if (IS_ERR(p))
                    { p = "error"; }
                }
            }

            DBG_T(MBB_DEBUG, "|--------------------------------------------------------------\n");
            DBG_T(MBB_DEBUG, "|The string of lun%d is '%s'.\n", j, curlun->inquiry_string);
            DBG_T(MBB_DEBUG, "|The type of------------------ lun%d is '%s'.\n", j, (curlun->cdrom ? "CD" : "SD"));
            DBG_T(MBB_DEBUG, "|The filp of------------------ lun%d is %p.\n", j, curlun->filp);
            DBG_T(MBB_DEBUG, "|The file of------------------ lun%d is '%s'.\n", j, p);
            DBG_T(MBB_DEBUG, "|The file_length of----------- lun%d is %d.\n", j, (USB_INT)curlun->file_length);
            DBG_T(MBB_DEBUG, "|The num_sectors of----------- lun%d is %d.\n", j, (USB_INT)curlun->num_sectors);
            DBG_T(MBB_DEBUG, "|The read only of------------- lun%d is '%s'.\n", j, curlun->ro ? "Read-only" : "RW");
            DBG_T(MBB_DEBUG, "|The removable of------------- lun%d is '%s'.\n", j, curlun->removable ? "Yes" : "No");
            DBG_T(MBB_DEBUG, "|The prevent_medium_removal of lun%d is %d.\n", j, curlun->prevent_medium_removal);
            DBG_T(MBB_DEBUG, "|The mode of------------------ lun%d is '%s'.\n", j, (curlun->mode ? "usb mode" : "web mode"));
            DBG_T(MBB_DEBUG, "|The keep_eject of------------ lun%d is %d.\n", j, curlun->keep_eject);
            DBG_T(MBB_DEBUG, "|The keep_no_medium_state of-- lun%d is %d.\n", j, curlun->keep_no_medium_state);
            DBG_T(MBB_DEBUG, "|The blkbits of--------------- lun%d is %d.\n", j, curlun->blkbits);
            DBG_T(MBB_DEBUG, "|The blksize of--------------- lun%d is %d.\n", j, curlun->blksize);
            DBG_T(MBB_DEBUG, "|--------------------------------------------------------------\n");
        }

        kfree(pathbuf);
    }
}

