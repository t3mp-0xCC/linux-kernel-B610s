/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and 
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may 
 * *    be used to endorse or promote products derived from this software 
 * *    without specific prior written permission.
 * 
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


/*lint --e{537} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/rtc.h>

#include <product_config.h>
#include <mdrv_sysboot.h>
#include <mdrv_chg.h>
#include <bsp_icc.h>
#include <bsp_onoff.h>
#include "power_exchange.h"
#include "mdrv_chg.h"
#include <soc_onchiprom.h>
#include <drv_comm.h>
#include <bsp_efuse.h>


#include "mbb_config.h" /*引入MBB产品配置文件*/
#include <bsp_sram.h>
#include <linux/mlog_lib.h>
#define POWER_ON_MONITOR_TIMEROUT       (60*1000)
#define SRAM_REBOOT_ADDR  (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_REBOOT_INFO)

#define POWER_ON_MONITOR_TIMEROUT       (60*1000)
#define EXCH_POWER_LOG_PATH     "/modem_log/poweron.txt"
#define POWER_SYS_PRIV_RO  (S_IRUSR | S_IRGRP)
#define POWER_SYS_PRIV_WR  (S_IWUSR | S_IRUGO)
extern BATT_LEVEL_E chg_get_batt_level(void);

typedef struct
{
    DRV_START_MODE_E mode;
    DRV_START_REASON reason;
    struct rtc_time         time;
    struct softtimer_list   on_timer;
}power_on_ctrl_s;


static power_on_ctrl_s power_item_info = {0}; //lint !e64

static ssize_t his_boot_store_property(struct device *dev,
                            struct device_attribute *attr,const char *buf,
                            size_t count);
static ssize_t his_boot_show_property(struct device *dev,
                            struct device_attribute *attr, char *buf);

#define POWER_ON_ATTR(_name, _mode)					\
{									            \
	.attr = { .name = #_name, .mode = _mode },					\
	.show = his_boot_show_property,          \
	.store = his_boot_store_property,	        \
}

/* 设置成只读 */
static struct device_attribute power_on_attrs[] = {
	POWER_ON_ATTR(power_status, POWER_SYS_PRIV_RO),
	POWER_ON_ATTR(power_reason, POWER_SYS_PRIV_RO),
	POWER_ON_ATTR(power_time, POWER_SYS_PRIV_RO),
};

static struct attribute *
his_boot_attributes[ARRAY_SIZE(power_on_attrs) + 1]; //lint !e30 !e84

static const struct attribute_group his_boot_group = {
	.attrs = his_boot_attributes,
};

/*****************************************************************************
 函 数 名  : power_on_init_attrs
 功能描述  : init attributes
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static void power_on_init_attrs(void)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(power_on_attrs); i++) //lint !e30 !e84
		his_boot_attributes[i] = &power_on_attrs[i].attr;
}

#define BOOT_CLASS_NODE  "onoff"
#define MAX_BUF_SIZE     32
/*此处的描述需与 power_com.h中枚举boot_on_reason_e保持一致,修改此描述需知会应用*/
static char* boot_reason_des[] = {
    "cold_boot",
    "normal_reboot",
    "wdt_acore", 
    "wdt_ccore",
    "wdt_mcore",
    "exc_acore",
    "exc_ccore",
    "exc_mcore",
    "exc_unknow",
    "invalid"
};
/******************************************************************************
*Function:           show_boot_reason_info
*Description:        提供给应用的节点，返回重启原因
*Calls:              
*Input:              
*Output:             NA
*Return:             魔术字的数值
*Others:             NA
******************************************************************************/
static int show_boot_reason_info(struct class *class, 
                    struct class_attribute *attr, char *buf)
{
    power_info_s *power_info = (power_info_s *)SRAM_REBOOT_ADDR;
    unsigned int boot_reason = power_info->boot_reason;
    
    if (NULL == buf)
    {
        pr_dbg("show_boot_reason_info:the buf is null.\n");
        return -ENOMEM;
    }

    if (BOOT_ON_REASON_INVALID < boot_reason || BOOT_ON_REASON_START > boot_reason)
    {
        boot_reason = BOOT_ON_REASON_INVALID; 
    }
    
    return snprintf(buf, MAX_BUF_SIZE, "%s\n", boot_reason_des[boot_reason - BOOT_ON_REASON_START]);

}

static struct class* onoff_class = NULL;
static CLASS_ATTR(boot_reason, S_IRUGO, show_boot_reason_info, NULL);
static struct class_attribute *boot_attributes[] = {
        &class_attr_boot_reason,   /* boot_reason */
};

/******************************************************************************
*Function:           boot_create_class_files
*Description:        创建跟应用交互的sys/class/目录节点
*Calls:              class_create  /  class_create_file
*Input:              
*Output:             NA
*Return:             0: success, 非0: failed
*Others:             NA
******************************************************************************/
static int boot_create_class_files(void)
{
    int ret = 1;
    int i = 0;
    int attrs_num = 0;

    struct class_attribute** boot_attrs = boot_attributes;

    onoff_class = class_create(THIS_MODULE, BOOT_CLASS_NODE);
    if (IS_ERR(onoff_class))
    { 
        pr_dbg("Error: Failed to create class.");
        return PTR_ERR(onoff_class); 
    }

    attrs_num = ARRAY_SIZE(boot_attributes);
    for(i = 0; i < attrs_num; i++)
    {
        ret = class_create_file(onoff_class, boot_attrs[i]);
        if (ret < 0)
        {
            (void)class_destroy(onoff_class);
            pr_dbg("Failed to create file, %d", i);
            return ret;
        }
    }
    return 0;
}
/******************************************************************************
*Function:           dload_destory_class_files
*Description:        删除跟应用交互的sys/class/目录节点
*Calls:              class_remove_file
*Input:              
*Output:             NA
*Return:             
*Others:             NA
******************************************************************************/
static void boot_destory_class_files(void)
{
    int i = 0;
    int attrs_num = 0;
    struct class_attribute** boot_attrs = boot_attributes;
    
    attrs_num = ARRAY_SIZE(boot_attributes);
    for(i = 0; i < attrs_num; i++)
    {
        (void)class_remove_file(onoff_class, boot_attrs[i]);
    }

    (void)class_destroy(onoff_class);
    return;
}


/*****************************************************************************
 函 数 名  : power_on_log_save
 功能描述  : 保存开机log
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static int power_on_log_save( void )
{
    long pos;
    unsigned int pf;
    mm_segment_t old_fs;
    ssize_t rt = 0;
    char buf[128] = {0};
    struct rtc_time tm;
    char *power_on_reason[4];
    char *power_on_mode[3];
    BATT_LEVEL_E  battery_level;

    tm = power_item_info.time;
    battery_level = chg_get_batt_level();
    power_info_s *power_info = (power_info_s *)SRAM_REBOOT_ADDR;
    unsigned int boot_reason = power_info->boot_reason;

    power_on_reason[0] = "Charger";
    power_on_reason[1] = "Power Key";
    power_on_reason[2] = "Warm Reset";
    power_on_reason[3] = "Unknown";
    power_on_mode[0] = "PWN CHARGING";
    power_on_mode[1] = "NORMAL";
    power_on_mode[2] = "UPDATE";

    if (BOOT_ON_REASON_INVALID < boot_reason || BOOT_ON_REASON_START > boot_reason)
    {
        boot_reason = BOOT_ON_REASON_INVALID; 
    }
    if(boot_reason <= BOOT_ON_REASON_MCORE_EXCEPTION && boot_reason >= BOOT_ON_REASON_ACORE_WDT)
    {
        mlog_print(MLOG_POWER, mlog_lv_info, "reboot flag POWER_OFF_REASON_CRASH\n");
        mlog_set_statis_info("abnormal_reboot_times",1);
    }
    /* 记录开机信息(时间、次数、关机原因) */
    snprintf(buf, sizeof(buf) - 1, "power reason: %s(%s), mode: %s, cur_volt: %d, cur_time:%4d-%02d-%02d %02d:%02d:%02d\r\n", \
            power_on_reason[power_item_info.reason], boot_reason_des[boot_reason - BOOT_ON_REASON_START], power_on_mode[power_item_info.mode], \
            battery_level, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    mlog_print(MLOG_POWER, mlog_lv_info, "%s", buf);
    mlog_set_statis_info("on_times",1);
    old_fs = get_fs(); //lint !e63

    set_fs(KERNEL_DS);

    /*coverity[check_return] */
    pf = (unsigned int)sys_open(EXCH_POWER_LOG_PATH, O_RDWR | O_CREAT, 0666);
    /*coverity[unchecked_value] */
    if(IS_ERR((const void*)pf))
    {
        pr_dbg( "error occured happened when open file %s, exiting.\n", EXCH_POWER_LOG_PATH);
        return (int)pf;
    }

    /*coverity[unchecked_value] */
    pos = sys_lseek(pf, 0, SEEK_END);
    if(pos > EXCH_ONOFF_LOG_MAX){
        /* 文件超过 16k,删除重新打开 */
        sys_rmdir(EXCH_POWER_LOG_PATH);
        /*coverity[check_return] */
        pf = (unsigned int)sys_open(EXCH_POWER_LOG_PATH, O_RDWR | O_CREAT, 0666);
        /*coverity[unchecked_value] */
        if(IS_ERR((const void*)pf))
        {
            pr_dbg( "error occured happened when open file %s, exiting.\n", EXCH_POWER_LOG_PATH);
            return (int)pf;
        }
    }
    else{
        /*coverity[unchecked_value] */
        sys_lseek(pf, pos, SEEK_SET);
    }

    /*coverity[unchecked_value] */
    rt = sys_write(pf, (const char*)buf, strlen(buf));
    if(rt<0)
    {
        pr_dbg("error occured happened when write file %s, exiting.\n", EXCH_POWER_LOG_PATH);
        /*coverity[unchecked_value] */
        sys_close(pf);
        set_fs(old_fs);
        return (int)rt;
    }

    /*coverity[unchecked_value] */
    sys_close(pf);
    set_fs(old_fs);

    pr_dbg(KERN_DEBUG "power on log save.\n ");

    return (int)rt;
}

/*****************************************************************************
 函 数 名  : power_on_time_get
 功能描述  : 记录开机时间.
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static void power_on_time_get( void )
{
    struct rtc_time tm;
    struct timespec ts;

    getnstimeofday(&ts);

    rtc_time_to_tm((unsigned long)ts.tv_sec, &tm);

    power_item_info.time = tm;

    pr_dbg("system power on time:%4d-%02d-%02d %02d:%02d:%02d\n",tm.tm_year, tm.tm_mon, \
            tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

/*****************************************************************************
 函 数 名  : power_on_reason_get
 功能描述  : 获取开机原因
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static void power_on_reason_get( void )
{
    DRV_START_REASON reason = DRV_START_REASON_BUTT;
    power_on_start_reason sram_data = POWER_ON_START_REASON_BUTT;

    sram_data = power_on_start_reason_get();
    switch(sram_data)
    {
    case POWER_ON_START_REASON_CHARGE:
        reason = DRV_START_REASON_CHARGE;
        break;
    case POWER_ON_START_REASON_POWER_KEY:
        reason = DRV_START_REASON_POWER_KEY;
        break;
    case POWER_ON_START_REASON_WARM_RESET:
        reason = DRV_START_REASON_WARM_RESET;
        break;
    default:
        reason = DRV_START_REASON_BUTT;
        break;
    }

    power_item_info.reason = reason;
}

/*****************************************************************************
 函 数 名  : power_on_mode_get
 功能描述  : 获取开机模式
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static void power_on_mode_get( void )
{
    DRV_START_MODE_E mode = DRV_START_MODE_NORMAL;
    DRV_RUN_MODE boot_mode = get_run_mode();
    DRV_CHARGING_MODE pwd_flag = get_pd_charge_flag();
    if((RUN_MODE_NORMAL == boot_mode) && (POWER_DOWN_CHARGING_MODE != pwd_flag))
    {
        /* 正常模式 */
        mode = DRV_START_MODE_NORMAL;
    }
    else if((RUN_MODE_NORMAL == boot_mode) && ((POWER_DOWN_CHARGING_MODE == pwd_flag)))
    {
        /* 关机充电模式 */
        mode = DRV_START_MODE_CHARGING;
    }
    else if(RUN_MODE_RECOVERY == boot_mode)
    {
        /* 升级模式 */
        mode = DRV_START_MODE_UPDATE;     
    }
    power_item_info.mode = mode;
}

/*****************************************************************************
 函 数 名  : power_off_timeout_isr
 功能描述  : 开机定时器处理，检测C核状态
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static int power_on_timeout_isr( void )
{
    c_power_st_e status = POWER_OS_OK;
    DRV_RUN_MODE run_mode = get_run_mode();

    pr_dbg("power_on_timeout_isr, system had been powered one minute ago\n ");
    if(RUN_MODE_NORMAL == run_mode)
    {
        /* 读取C核启动状态 */
        status = power_on_c_status_get();

        if(status < POWER_MSP_OK)
        {
           pr_dbg("c core power on fail!\n ");
           /* 进入recovery模式 */
           bsp_drv_power_reboot();

           /* coverity[no_escape] */
           for(;;) ;
        }
    }
    else if(RUN_MODE_RECOVERY == run_mode){
        /* for test */
    }

    pr_dbg("system power on ok!\n ");

    /* clear warm boot cnt */
    power_on_wdt_cnt_set();

    (void)bsp_softtimer_free(&power_item_info.on_timer);

    return 0;
}

/*****************************************************************************
 函 数 名  : power_on_timer_init
 功能描述  : 创建开机timer,用于开机后一段时间检测系统是否正常工作
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
void power_on_timer_init( void )
{
	s32 ret = 0;

	power_item_info.on_timer.func = (softtimer_func)power_on_timeout_isr;

	power_item_info.on_timer.para = (u32)0;
	power_item_info.on_timer.timeout = POWER_ON_MONITOR_TIMEROUT;
	power_item_info.on_timer.wake_type = SOFTTIMER_WAKE;

	ret = bsp_softtimer_create(&power_item_info.on_timer);
    if (ret)
    {
        pr_dbg("create softtimer failed \n");
    }

    bsp_softtimer_add(&power_item_info.on_timer);
}

/*****************************************************************************
 函 数 名  : his_boot_store_property
 功能描述  : echo sysfs   do nothing
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static ssize_t his_boot_store_property(struct device *dev,
                            struct device_attribute *attr,const char *buf,
                            size_t count)
{
    pr_dbg("his_boot_store\n");

    return 0;
}

/*****************************************************************************
 函 数 名  : his_boot_show_property
 功能描述  : show power on status/reason/time item property.
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static ssize_t his_boot_show_property(struct device *dev,
                            struct device_attribute *attr, char *buf)
{
    struct rtc_time tm = power_item_info.time;

	static char *power_status[] = {
		"Power Down Charging", "Normal", "Update", "Unkown"
	};

	static char *power_reason[] = {
		"Charger", "Power Key", "Warm Reset", "Unknown",
	};

	const ptrdiff_t off = attr - power_on_attrs;

	if (off == POWER_ON_PROP_STATUS)
		return sprintf(buf, "%s\n", power_status[power_item_info.mode]);
	else if (off == POWER_ON_PROP_REASON)
		return sprintf(buf, "%s\n", power_reason[power_item_info.reason]);
	else if (off == POWER_ON_PROP_TIME){
		return sprintf(buf, "%4d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year, tm.tm_mon, \
        tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    }

    /* coverity[secure_coding] */
	return sprintf(buf, "%s\n", "error item");
}

/*****************************************************************************
 函 数 名  : mbb_kernel_secboot_id_check
 功能描述  : 判断硬加密/安全启动版本号校验结果
 输入参数  : 无
 输出参数  : 无
 返 回 值  : TRUE:成功 FALSE :失败
 调用函数  : his_boot_probe
 被调函数  : 无
*****************************************************************************/
bool mbb_kernel_secboot_id_check(void)
{
    u32 efuse_security_flag = 0;
    u32 efuse_secboot_id = 0;
    s32 sec_err_code = OK;

    sec_err_code = bsp_efuse_read(&efuse_security_flag, EFUSE_GROUP_INDEX_SECURE, 1);
    if( OK != sec_err_code)
    {
        /*efuse 读失败则校验失败，返回FALSE */
        printk(KERN_ERR "mbb_kernel_secboot_id_check: efuse_security_flag read error\n");
        return FALSE;
    }

    if(efuse_security_flag & EFUSE_BIT_FLAG_SEC_EN)
    {    
        /* 安全校验使能，读efuse 值  */
        sec_err_code = bsp_efuse_read(&efuse_secboot_id, EFUSE_GRP_SECBOOT_ID, 1);
        if( OK != sec_err_code)
        {
            /*efuse 读失败则校验失败，返回FALSE */
            printk(KERN_ERR "mbb_kernel_secboot_id_check: efuse_secboot_id read error\n");
            return FALSE;
        }

        if(MBB_SECBOOT_ID >= efuse_secboot_id)
        {
            /* 软件版本里的ID数值大于或者等于efuse里的，认为校验通过 */
            return TRUE;
        }
        else
        {
            /*efuse 校验失败，返回FALSE */
            printk(KERN_ERR "mbb_kernel_secboot_id_check: efuse_secboot_id check error\n");
            return FALSE;
        }
    }
    else
    {
        /* 安全校验没有使能,默认校验成功  */
        return TRUE;
    }
}

/*****************************************************************************
 函 数 名  : bsp_start_mode_get
 功能描述  : 用于获取开机模式
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
int bsp_start_mode_get(void)
{
    power_on_mode_get();
    return power_item_info.mode;
}


/*****************************************************************************
 函 数 名  : bsp_power_icc_send_state
 功能描述  : C核核间通信函数
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static void bsp_power_icc_send_state(void)
{
    int ret = 0;
    int mode = 0;
    u32 icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

    mode = bsp_start_mode_get();

    ret = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, (u8*)&mode, sizeof(mode));
    if (ret != (int)sizeof(mode))
    {
        pr_dbg("send len(%x) != expected len(%lu)\n", ret, (unsigned long)sizeof(mode));
    }
}

/*****************************************************************************
 函 数 名  : bsp_power_ctrl_read_cb
 功能描述  : C核核间回调函数
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static s32 bsp_power_ctrl_read_cb( void )
{
    int rt = 0;
    int read_len = 0;
    stCtrlMsg msg;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

	read_len = bsp_icc_read(channel_id, (u8*)&msg, sizeof(stCtrlMsg));
	if(read_len != (int)sizeof(stCtrlMsg))
	{
		pr_dbg("read len(%x) != expected len(%lu)\n", read_len, (unsigned long)sizeof(stCtrlMsg));
		return -1;
	}

	pr_dbg("bsp_power_ctrl_read_cb 0x%x\n", msg.pwr_type);

    switch(msg.pwr_type)
    {
    case E_POWER_ON_MODE_GET:
        bsp_power_icc_send_state();
        break;
    case E_POWER_SHUT_DOWN:
        drv_shut_down(msg.reason);
        break;
    case E_POWER_POWER_OFF:
        bsp_drv_power_off();
        break;
    case E_POWER_POWER_REBOOT:
        bsp_drv_power_reboot();
        break;
    default:
        pr_dbg("invalid ctrl by ccore\n");
        break;
    }

    return rt;
}

/*****************************************************************************
 函 数 名  : his_boot_probe
 功能描述  : power on
 输入参数  :
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
*****************************************************************************/
static int __init his_boot_probe(struct platform_device *pdev)
{
    int rt;
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_ONOFF;

    if( FALSE == mbb_kernel_secboot_id_check() )
    {
        /* A核校验失败，单板重启 */
        bsp_drv_power_reboot();
        for(;;) ;
    }

    /* get early param */

    /* 清除reboot flag */
    power_on_reboot_flag_set(POWER_OFF_REASON_INVALID);

    power_on_mode_get();

    /* get power on reason */
    power_on_reason_get();

    /* get power time */
    power_on_time_get();

    /* sysfs create */
    power_on_init_attrs();

    rt = (int)sysfs_create_group(&pdev->dev.kobj, &his_boot_group);
    if (rt < 0)
    {
        pr_dbg("create sys filesystem node failed.\n");
        return rt;
    }

    /* file create and power on info record */
    rt = power_on_log_save();
    if(rt < 0){
        /*log保存失败继续执行,不影响正常启动*/
        pr_dbg("power on log record failed.\n");
    }
    
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        pr_dbg("his_boot_probe: smem_data is NULL \n"); 
        goto icc;
    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /*升级模式，不创建timer*/
        pr_dbg("his_boot_probe:in update mode,not creat timer !\n"); 
    }
    else
    {
        /* 创建开机timer */
        power_on_timer_init();
    }
icc:
    rt = bsp_icc_event_register(channel_id, (read_cb_func)bsp_power_ctrl_read_cb, NULL, NULL, NULL);
    if(rt != 0){
        pr_dbg("icc event register failed.\n");
    }

    return rt;
}

static struct platform_device his_boot_dev = {
    .name = "his_boot",
    .id = 0,
    .dev = {
    .init_name = "his_boot",
    },
};

static struct platform_driver his_boot_drv = {
    .probe      = his_boot_probe,
	.driver		= {
		.name	= "his_boot",
		.owner	= THIS_MODULE,
	},
};

static int __init his_boot_init(void)
{
    ssize_t ret = 0;

    pr_dbg(KERN_DEBUG "his_boot_init.\r\n");

    ret = platform_device_register(&his_boot_dev);
    if(ret)
    {
        pr_dbg("register his_boot device failed. \r\n");
        return ret;
    }

    ret = platform_driver_register(&his_boot_drv);
    if(ret)
    {
        pr_dbg("register his_boot driver failed. \r\n");
        platform_device_unregister(&his_boot_dev);
    }
    (void)boot_create_class_files();
    return ret;

}

static void __exit his_boot_exit(void)
{
    platform_driver_unregister(&his_boot_drv);
    platform_device_unregister(&his_boot_dev);
    boot_destory_class_files();
}

late_initcall(his_boot_init);

MODULE_AUTHOR("E5 FEATURE");

