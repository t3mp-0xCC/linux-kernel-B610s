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



#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/capability.h>
#include <linux/uaccess.h>
#include <linux/compat.h>
#include <asm/uaccess.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include "bsp_sram.h"
#include "dload_nark_api.h"
#include "dload_nark_core.h"

#include "mbb_config.h"
#if  (FEATURE_ON == MBB_DLOAD_VER_VERIFY)
#include "linux/mtd/flash_huawei_dload.h"
#endif /* MBB_DLOAD_VER_VERIFY */

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************
  1 ģ��˽�� (�ꡢö�١��ṹ�塢�Զ�����������) ������:
******************************************************************************/
#define DLOAD_PRINT(level, fmt, ...) (printk(level"[*DLOAD_NARK_CORE*] %s:%d: "fmt"\n", __func__, __LINE__, ##__VA_ARGS__))
#define DLOAD_IOC_TYPE   'D'

typedef struct _IOCRL_MSG
{
    dload_module_t       module_index;  /* ������ģ��ID */
    uint32               msg_index;     /* ��ģ����ϢID */
    uint32               data_len;      /* ���ݻ��泤�� */
    void*                data_buffer;   /* ���ݻ���ָ�� */
} ioctl_msg_t;

enum
{
    DLOAD_SET_CMD  = _IOW(DLOAD_IOC_TYPE, 0, ioctl_msg_t),  /* �������� */
    DLOAD_GET_CMD  = _IOR(DLOAD_IOC_TYPE, 1, ioctl_msg_t),  /* ��ȡ���� */
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(xx) (sizeof(xx) / sizeof((xx)[0]))
#endif /* ARRAY_SIZE */
#define USB_REPORT_MAGIC_LENGTH     (30)
#define DLOAD_CLASS_NODE            "huawei_dload"
#define USB_REPORT_MAGIC            "online_update"

/* ���������ѯ���õײ���Ϣ ʱ����Ϣ�������ֵ��
Ŀǰ��ѯ��Կ�����Ϊ4K���������������Ӵ˴�Ҫ���� */
#define MSG_MAX_SIZE            (4 * 4096)

/******************************************************************************
  2 ģ��˽�� (ȫ�ֱ���) ������:
******************************************************************************/
struct mutex ioctl_mutex;
STATIC struct class* dload_class = NULL;

/*******************************************************************
  ��������  : map_search
  ��������  : ����������Ϣ��Ϣ����ӳ�亯��
  �������  : ioctl_msg : ������Ϣ
  �������  : NV
  ��������ֵ: msg_func_t: ӳ�亯��
********************************************************************/
STATIC msg_func_t map_search(ioctl_msg_t*  ioctl_msg)
{
    uint32 msg_map_index = 0;
    msg_func_t  msg_func;

    /* ����ӳ������ */
    msg_map_index  = MAP_INDEX(ioctl_msg->module_index, ioctl_msg->msg_index);
    
    /* ������Ϣ���� */
    msg_func = search_map_callback(msg_map_index);
    if(NULL == msg_func)
    {
        DLOAD_PRINT(KERN_ERR, "msg_map_index = %lu : map callback is null.", msg_map_index);
        return NULL;
    }
    
    return msg_func;
}

/*******************************************************************
  ��������  : nark_get_data
  ��������  : ��ȡ������Ϣ�ɷ�
  �������  : ioctl_msg : ������Ϣ
  �������  : NA
  ��������ֵ: DLOAD_OK    : �ɹ�
              DLOAD_ERR : ʧ��
********************************************************************/
STATIC int32 nark_get_data(ioctl_msg_t*  ioctl_msg)
{
    int32           ret  = DLOAD_OK;
    int32           dret = -1;
    uint32          local_len = 0;
    int8*           local_buffer = NULL;
    msg_func_t      msg_func;

    msg_func = map_search(ioctl_msg);
    if(NULL == msg_func)
    {
        return DLOAD_ERR;
    }
    
    if((0 != ioctl_msg->data_len) && (ioctl_msg->data_len < MSG_MAX_SIZE) && (NULL != ioctl_msg->data_buffer))
    {
        local_buffer = (char *)kzalloc(ioctl_msg->data_len, GFP_KERNEL);
        if(NULL == local_buffer)
        {
            DLOAD_PRINT(KERN_ERR, "kzalloc failed.");
            return DLOAD_ERR;
        }
        else
        {
            dret = copy_from_user(local_buffer, ioctl_msg->data_buffer, ioctl_msg->data_len);
            if(0 > dret)
            {
                if(NULL != local_buffer)
                {
                    kfree(local_buffer);
                }
                
                DLOAD_PRINT(KERN_ERR, "copy_from_user ERROR.");
                return DLOAD_ERR;
            }
        }
		
        local_len = ioctl_msg->data_len;
    }

    ret = msg_func((void *)local_buffer, local_len);
    
    if(NULL != local_buffer)
    {
        dret = copy_to_user(ioctl_msg->data_buffer, local_buffer, local_len);
        if(0 > dret)
        {
            DLOAD_PRINT(KERN_ERR, "copy_to_user ERROR");
            ret = DLOAD_ERR;
        }
    }

    if(NULL != local_buffer)
    {
        /* �ͷ��ڴ� */
        kfree(local_buffer);
    }
        
    return ret;
}

/*******************************************************************
  ��������  : nark_set_data
  ��������  : ����������Ϣ�ɷ�
  �������  : ioctl_msg : ������Ϣ
  �������  : NA
  ��������ֵ: DLOAD_OK    : �ɹ�
              DLOAD_ERR : ʧ��
********************************************************************/
STATIC int32 nark_set_data(ioctl_msg_t*  ioctl_msg)
{
    int32           ret  = DLOAD_OK;
    int32           dret = -1;
    uint32          local_len = 0;
    int8*           local_buffer = NULL;
    msg_func_t      msg_func;

    msg_func = map_search(ioctl_msg);
    if(NULL == msg_func)
    {
        return DLOAD_ERR;
    }
    
    if((0 != ioctl_msg->data_len) && (ioctl_msg->data_len < MSG_MAX_SIZE) && (NULL != ioctl_msg->data_buffer))
    {
        local_buffer = (char *)kzalloc(ioctl_msg->data_len, GFP_KERNEL);
        if(NULL == local_buffer)
        {
            DLOAD_PRINT(KERN_ERR, "kzalloc failed.");
            return DLOAD_ERR;
        }
        else
        {
            dret = copy_from_user(local_buffer, ioctl_msg->data_buffer, ioctl_msg->data_len);
            if(0 > dret)
            {
                if(NULL != local_buffer)
                {
                    kfree(local_buffer);
                }
                
                DLOAD_PRINT(KERN_ERR, "copy_from_user ERROR.");
                return DLOAD_ERR;
            }
        }

        local_len = ioctl_msg->data_len;
    }
    
    ret = msg_func((void *)local_buffer, local_len);
    
    if(NULL != local_buffer)
    {
        kfree(local_buffer);
    }
    
    return ret;
}

/*******************************************************************
  ��������  : dload_Ioctl
  ��������  : �����������ioctl����
  �������  : 
  �������  : NA
  ��������ֵ: DLOAD_OK    : �ɹ�
              DLOAD_ERR : ʧ��
********************************************************************/
STATIC long dload_Ioctl(struct file *file, unsigned int cmd, unsigned long data)
{
    int32 ret  = DLOAD_OK;

    if((unsigned long)NULL == data)
    {
        DLOAD_PRINT(KERN_ERR, "data is NULL.");
        return DLOAD_ERR;
    }
    
    /* ����������Ч�� */
    if (DLOAD_IOC_TYPE != _IOC_TYPE(cmd)) 
    {
        DLOAD_PRINT(KERN_ERR, "ioctl_type = %c  Ioctl command TYPE ERROR.", _IOC_TYPE(cmd));
        return DLOAD_ERR;
    }

    if (sizeof(ioctl_msg_t) != _IOC_SIZE(cmd)) 
    {
        DLOAD_PRINT(KERN_ERR, "ioctl_size = %d  Ioctl command SIZE ERROR.", _IOC_SIZE(cmd));
        return DLOAD_ERR;
    }

    mutex_lock(&ioctl_mutex);
    switch(cmd)
    {
        case DLOAD_GET_CMD:
            if(_IOC_READ == _IOC_DIR(cmd))
            {
                ret = nark_get_data((ioctl_msg_t *)data);
            }
            else
            {
                DLOAD_PRINT(KERN_ERR, "ioctl_type = %c  Ioctl command DIR ERROR.", _IOC_DIR(cmd));
                ret = DLOAD_ERR;
            }
            break;
        case DLOAD_SET_CMD:
            if(_IOC_WRITE == _IOC_DIR(cmd))
            {
                ret = nark_set_data((ioctl_msg_t *)data);
            }
            else
            {
                DLOAD_PRINT(KERN_ERR, "ioctl_type = %c  Ioctl command DIR ERROR.", _IOC_DIR(cmd));
                ret = DLOAD_ERR;
            }
            break;
        default:
            ret = DLOAD_ERR;
            DLOAD_PRINT(KERN_ERR, "Ioctl command ERROR.");
            break;
    }
    mutex_unlock(&ioctl_mutex);
    
    return ret;
}

STATIC int dload_proc_show(struct seq_file *pSeq, void *pVal)
{
    int32  ret                 = -1;
    uint32 visit_value        = GET_SMEM_IS_SWITCH_PORT;

    mutex_lock(&ioctl_mutex);
    ret = visit_dload_smem((uint8 *)&visit_value, sizeof(visit_value));
    if(DLOAD_OK == ret)
    {
        seq_puts(pSeq, "nv_restore_start");
    }
    else
    {
        seq_puts(pSeq, "nv_restore_end");
	}
    mutex_unlock(&ioctl_mutex);
	
    return 0;
}

STATIC int dload_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, dload_proc_show, NULL);
}

#define PORT_NORMAL         (0) /* �������� */
#define PORT_NO_PCUI        (1) /* ����pcui�� */
#define PORT_DLOAD          (2) /* ����ģʽ���� */
#define PORT_NV_RES         (3) /* nv�ָ�ģʽ���� */
/******************************************************************************
*Function:           dload_report_port_status
*Description:        ��usbģ���ϱ��˿�״ֵ̬
*Calls:              
*Input:              
*Output:             NA
*Return:             0: normal, 1: ����pcui�� 2: ����ģʽ����, 3: nv�ָ�ģʽ����
*Others:             NA
******************************************************************************/
int dload_report_port_status(void)
{
    int status = PORT_NORMAL;
    huawei_smem_info *smem_data = NULL;
  
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        DLOAD_PRINT(KERN_ERR, "dload_report_port_status error.");
        return DLOAD_ERR;
    }

    if(SMEM_ONUP_FLAG_NUM == smem_data->smem_online_upgrade_flag /* ����������װ�׶� */
        || SMEM_SDUP_FLAG_NUM == smem_data->smem_sd_upgrade            /* SD������װ�׶� */
        || SMEM_ONNR_FLAG_NUM == smem_data->smem_online_upgrade_flag   /* ��������NV�ָ��׶� */
        || SMEM_SDNR_FLAG_NUM == smem_data->smem_online_upgrade_flag)  /* SD����NV�ָ��׶� */
    {
        /* �����ߡ�sd����ģʽ/nv�ָ��׶ζ˿��ϱ����� */
        status = PORT_NO_PCUI;
    }
    else if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /* ��һ������ģʽ�˿��ϱ����� */
        status = PORT_DLOAD;
    }
    else if(SMEM_SWITCH_PUCI_FLAG_NUM == smem_data->smem_switch_pcui_flag)
    {
        /* ��һ������ģʽ�˿��ϱ����� */
        status = PORT_NV_RES;
    }
    else
    {
        /* ���������� */
        status = PORT_NORMAL;
    }

    DLOAD_PRINT(KERN_INFO, "Info: port mode is %d", (int)status);
    return status;
}
EXPORT_SYMBOL(dload_report_port_status);

/******************************************************************************
*Function:           dload_show_ota_info
*Description:        �鿴ħ����
*Calls:              
*Input:              
*Output:             NA
*Return:             ħ���ֵ���ֵ
*Others:             NA
******************************************************************************/
STATIC ssize_t dload_show_ota_info(struct class *class, 
                    struct class_attribute *attr, char *buf)
{
    huawei_smem_info *smem_data = NULL;
  
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        DLOAD_PRINT(KERN_ERR, "dload_show_ota_info error.");
        return DLOAD_ERR;
    }
    if(NULL == buf)
    {
        DLOAD_PRINT(KERN_EMERG, "Error: para err");
        return - ENOMEM;
    }

    return snprintf(buf, USB_REPORT_MAGIC_LENGTH, "0x%x\n", 
                (unsigned int)smem_data->smem_online_upgrade_flag);
}

/******************************************************************************
*Function:           dload_store_usb_report_info
*Description:        ����usb port�ϱ���ħ���֣����ݱ����ħ����ȷ���Ƿ��ϱ��˿�
*Calls:              
*Input:              
*Output:             NA
*Return:             ��0: �ϲ����ڵ�д������ݳ���, 0: �ϲ����ڵ�д��
*Others:             NA
******************************************************************************/
STATIC ssize_t dload_store_ota_info(struct class *class, 
            struct class_attribute *attr, const char *buf, size_t count)
{
    unsigned char   bufTmp[USB_REPORT_MAGIC_LENGTH + 1] = {0};
    huawei_smem_info *smem_data = NULL;
  
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        DLOAD_PRINT(KERN_ERR, "dload_store_ota_info error.");
        return DLOAD_ERR;
    }

    if(count >= USB_REPORT_MAGIC_LENGTH)
    {
        DLOAD_PRINT(KERN_EMERG, "Error: user input data illegal");
        return -EINVAL;
    }
    (void)strncpy(bufTmp, buf, count);

    if(!strncmp(bufTmp, USB_REPORT_MAGIC, strlen(USB_REPORT_MAGIC)))
    {
        smem_data->smem_online_upgrade_flag = SMEM_ONUP_FLAG_NUM;
    }
    return count;
}

#if (FEATURE_ON == MBB_DLOAD_AUTO_FLAG)
/*****************************************************************
* �� �� ��   : dload_set_online_flag
* ��������  : at^godload��������ǰ����flag, ����
         ��������,�ڽ���һ������ʱ����ñ��
         ���ǵ�����֮����ܸ���,��˲�����Ϊstatic����
* �������   : 1--���ñ�־/0--�����־/����--����
* �������   : ��
* �� �� ֵ   : DLOAD_OK/DLOAD_ERR
* ����˵��  : at^godload�������,���������Ա�ǲ��ı�
******************************************************************/
int dload_set_auto_flag(unsigned int auto_flag)
{
    boolean bret = true;
    part_complete_flag part_complete_buf = {0};

    if(1 < auto_flag)
    {
        /*��ǿ�ж�,ֻ����0/1����ֵ,�������Ƿ�*/
        printk(KERN_ERR "dload_set_auto_flag: Input para error, %d.\n", auto_flag);
        return DLOAD_ERR;
    }

    bret = flash_get_share_region_info(RGN_PART_COMPLETE_FLAG,
                    (void*)&part_complete_buf,sizeof(part_complete_flag));
    if((true != bret)
        || (PART_COMPLETE_FLAG_MAGIC_NUM != part_complete_buf.magic_number))
    {
        part_complete_buf.dload_times = 0;
        part_complete_buf.complete_flag = 1;  /*����ģʽ�¶���������Ϊ1*/
    }

    part_complete_buf.magic_number = PART_COMPLETE_FLAG_MAGIC_NUM;
    /*auto flag����Ϊħ����0,�����Ϸ��Ժ���������ж�*/
    part_complete_buf.auto_flag = ((1 == auto_flag) ? AUTO_UPDATE_FLAG_MAGIC_NUM : 0);

    bret = flash_update_share_region_info(RGN_PART_COMPLETE_FLAG, (void *)(&part_complete_buf), sizeof(part_complete_flag));
    if(true != bret)
    {
        printk(KERN_ERR "dload_set_auto_flag: set auto flag fail.\n");
        return DLOAD_ERR;
    }

    return DLOAD_OK;
}

/******************************************************************************
*Function:           dload_store_auto_flag
*Description:        ���ñ����Զ�������־,�������ú�����,����ģʽʹ��
*Calls:              
*Input:              �ں�class��׼����,����0��ʾ�����־������ʾ���ñ�־
*Output:             NA
*Return:             ok: �����ϲ����ڵ�д������ݳ���;
                   error: ����
*Others:             NA
******************************************************************************/
STATIC ssize_t dload_store_auto_flag(struct class *class, 
            struct class_attribute *attr, const char *buf, size_t count)
{
    int ret = DLOAD_ERR;
    unsigned int wr_val = 0;

    if((NULL == buf) || (count < sizeof(unsigned int)))
    {
        /*����ж�,class&attrΪ�ں˹��к������,���ﲻ�ж�*/
        printk(KERN_ERR "dload_store_auto_flag: Input para error, %d.\n", count);
        return DLOAD_ERR;
    }

    wr_val = *((unsigned int *)buf);
    ret = dload_set_auto_flag(wr_val);
    if(DLOAD_OK != ret)
    {
        printk(KERN_ERR "dload_store_auto_flag: set auto flag failed.\n");
        return DLOAD_ERR;
    }

    return count;
}

STATIC CLASS_ATTR(ota_up, S_IRUGO | S_IWUSR, dload_show_ota_info, dload_store_ota_info);
STATIC CLASS_ATTR(auto_up, S_IRUGO | S_IWUSR, NULL, dload_store_auto_flag);
STATIC struct class_attribute *dload_attributes[] = {
        &class_attr_ota_up,   /* ota_up */
        &class_attr_auto_up,  /*online_flag--�Զ�������־����class�ļ�,��������ʹ��*/
};
#else

STATIC CLASS_ATTR(ota_up, S_IRUGO | S_IWUSR, dload_show_ota_info, dload_store_ota_info);
STATIC struct class_attribute *dload_attributes[] = {
        &class_attr_ota_up,   /* ota_up */
};
#endif

/******************************************************************************
*Function:           dload_init_class_files
*Description:        ����dloadģ���Ӧ�ý�����sys/class/Ŀ¼�ڵ�
*Calls:              class_create  /  class_create_file
*Input:              
*Output:             NA
*Return:             0: success, ��0: failed
*Others:             NA
******************************************************************************/
STATIC int dload_init_class_files(void)
{
    int ret = 1;
    int i = 0;
    int attrs_num = 0;
    struct class_attribute** dload_attrs = dload_attributes;

    dload_class = class_create(THIS_MODULE, DLOAD_CLASS_NODE);
    if (IS_ERR(dload_class))
    { 
        DLOAD_PRINT(KERN_EMERG, "Error: Failed to create class.");
        return PTR_ERR(dload_class); 
    }

    attrs_num = ARRAY_SIZE(dload_attributes);
    for(i = 0; i < attrs_num; i++)
    {
        ret = class_create_file(dload_class, dload_attrs[i]);
        if (ret)
        {
            (void)class_destroy(dload_class);
            DLOAD_PRINT(KERN_EMERG, "Failed to create file, %d", i);
            return ret;
        }
    }

    return 0;
}

/******************************************************************************
*Function:           dload_destory_class_files
*Description:        ɾ��dloadģ���Ӧ�ý�����sys/class/Ŀ¼�ڵ�
*Calls:              class_remove_file
*Input:              
*Output:             NA
*Return:             
*Others:             NA
******************************************************************************/
STATIC void dload_destory_class_files(void)
{
    int i = 0;
    int attrs_num = 0;
    struct class_attribute** dload_attrs = dload_attributes;
    
    attrs_num = ARRAY_SIZE(dload_attributes);
    for(i = 0; i < attrs_num; i++)
    {
        (void)class_remove_file(dload_class, dload_attrs[i]);
    }

    (void)class_destroy(dload_class);
    return;
}

#if  (FEATURE_ON == MBB_DLOAD_VER_VERIFY)
STATIC int flash_web_proc_read(struct file *file, char __user *buf,
        size_t length, loff_t *ppos)
{
    return 0;
}

STATIC int flash_web_proc_write(struct file *file, const char __user *buf,
        size_t length, loff_t *ppos)
{
    unsigned char   bufTmp[WEBUI_VER_BUF_LEN] = {0};
    webver_buffer_s  *webver_info = NULL;
    webver_buffer_s verTmp= {0};
    platform_info_t platform_info;

    memset(bufTmp, '\0', sizeof(bufTmp));

    if(length <= 1 || length >= WEBUI_VER_BUF_LEN)
    {
        DLOAD_PRINT(KERN_ERR, "webui version is null, %d, return", length);
        return length;
    }

    /* ��ȡwebui�汾�� */
    if (true == flash_get_share_region_info(RGN_WEBHD_FLAG, (void *)&verTmp, sizeof(webver_buffer_s)))
    {
        webver_info = &verTmp;
        if (webver_info->magic_number == WEBUI_VER_FLAG_MAGIC)
        {
            DLOAD_PRINT(KERN_ERR, "Do not need to update version success.");
            return length;
        }
    }

    if (copy_from_user(bufTmp, buf, length))
    {
        DLOAD_PRINT(KERN_ERR, "copy user data err.");
        return -EFAULT;
    }

    memset(&verTmp, 0, sizeof(verTmp));
    verTmp.magic_number = WEBUI_VER_FLAG_MAGIC;
    
    if((bufTmp[0] <= '9') && (bufTmp[0] >= '0'))
    {
        /* ��ΪMBB&&HOME ,����webui */
        (void)snprintf(verTmp.version_buffer, WEBUI_VER_BUF_LEN, "%s", HUAWEI_WEBUI_PREFIX);

        /* �汾�Ų��ᳬ��128�ֽ�,�淶����,�ȱ���汾�� */
        /* ��һ������,ȥ����������ַ� */
        if(bufTmp[length - 1] == '\r' || bufTmp[length - 1] == '\n')
        {
            bufTmp[length - 1]= '\0';
            length -= 1;
        }
        (void)strncat(verTmp.version_buffer, bufTmp, length);

        /* �����»��� */
        (void)strncat(verTmp.version_buffer, "_", strlen("_"));

        /* �����Ʒ��̬���� */
        memset(&platform_info, 0, sizeof(platform_info_t));
        (void)get_platform_information((uint8*)&platform_info, sizeof(platform_info_t));
        (void)strncat(verTmp.version_buffer, 
            platform_info.product_info_st.feature_name, strlen(platform_info.product_info_st.feature_name));
    }
    else
    {
        /* VDF webui */
        (void)snprintf(verTmp.version_buffer, WEBUI_VER_BUF_LEN, "%s", VDF_WEBUI_PREFIX);
        /* �汾�Ų��ᳬ��128�ֽ�,�淶����,�ȱ���汾�� */
        (void)strncat(verTmp.version_buffer, bufTmp, length);
    }

    DLOAD_PRINT(KERN_ERR, "@@@[%s]###", verTmp.version_buffer);
    (void)flash_update_share_region_info(RGN_WEBHD_FLAG, (void *)&verTmp, sizeof(webver_buffer_s));
    DLOAD_PRINT(KERN_ERR, "Update version success.");
    return length;
}

STATIC struct file_operations flush_web_proc_fops = {
    .owner          = THIS_MODULE,
    .open           = NULL,
    .read           = flash_web_proc_read,
    .write          = flash_web_proc_write,
};
#endif /* MBB_DLOAD_VER_VERIFY */

STATIC struct file_operations dload_proc_fops = {
    .owner          = THIS_MODULE,
    .open           = dload_proc_open,
    .read           = seq_read,
    .llseek         = seq_lseek,
    .release        = single_release,
};

STATIC struct file_operations dload_dev_fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = dload_Ioctl,
};

STATIC struct miscdevice dload_miscdev = {
    .minor    = MISC_DYNAMIC_MINOR,
    .name     = "dload_nark",
    .fops     = &dload_dev_fops
};

STATIC __init int dload_init(void)
{
    int32 ret = DLOAD_OK;
    
    /*nark apiģ���ʼ��*/
    if (DLOAD_ERR == nark_api_init())
    {
        DLOAD_PRINT(KERN_ERR, "nark_api_init ERROR.");
        return DLOAD_ERR;
    }
    
    ret = misc_register(&dload_miscdev);
    if (0 > ret)
    {
        DLOAD_PRINT(KERN_ERR, "misc_register FAILED.");
    }

    /* ��ʼ�������� */
    mutex_init(&ioctl_mutex);

    /* ��������proc�ڵ� */
    proc_create("dload_nark", S_IRUGO, NULL, &dload_proc_fops);

#if  (FEATURE_ON == MBB_DLOAD_VER_VERIFY)
    /* ��������proc�ڵ� */
    proc_create("flush_webver", 0660, NULL, &flush_web_proc_fops);  /*  */
#endif

    (void)dload_init_class_files();
    return ret;
}

STATIC void __exit dload_exit(void)
{
    int32 ret = 0;

    /* �Ƴ�proc�ڵ� */
    remove_proc_entry("dload_nark", NULL); 
    
#if  (FEATURE_ON == MBB_DLOAD_VER_VERIFY)
    /* �Ƴ�proc�ڵ� */
    remove_proc_entry("flush_webver", NULL);
#endif

    dload_destory_class_files();
    ret = misc_deregister(&dload_miscdev);
    if (0 > ret)
    {
        DLOAD_PRINT(KERN_ERR, "misc_deregister FAILED.");
    }
}

module_init(dload_init);
module_exit(dload_exit);

MODULE_AUTHOR("MBB.Huawei Device");
MODULE_DESCRIPTION("Dload Driver");
MODULE_LICENSE("GPL");
#ifdef __cplusplus
}
#endif
