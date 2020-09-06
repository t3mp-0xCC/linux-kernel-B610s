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
  1 模块私有 (宏、枚举、结构体、自定义数据类型) 定义区:
******************************************************************************/
#define DLOAD_PRINT(level, fmt, ...) (printk(level"[*DLOAD_NARK_CORE*] %s:%d: "fmt"\n", __func__, __LINE__, ##__VA_ARGS__))
#define DLOAD_IOC_TYPE   'D'

typedef struct _IOCRL_MSG
{
    dload_module_t       module_index;  /* 升级子模块ID */
    uint32               msg_index;     /* 子模块消息ID */
    uint32               data_len;      /* 数据缓存长度 */
    void*                data_buffer;   /* 数据缓存指针 */
} ioctl_msg_t;

enum
{
    DLOAD_SET_CMD  = _IOW(DLOAD_IOC_TYPE, 0, ioctl_msg_t),  /* 设置命令 */
    DLOAD_GET_CMD  = _IOR(DLOAD_IOC_TYPE, 1, ioctl_msg_t),  /* 获取命令 */
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(xx) (sizeof(xx) / sizeof((xx)[0]))
#endif /* ARRAY_SIZE */
#define USB_REPORT_MAGIC_LENGTH     (30)
#define DLOAD_CLASS_NODE            "huawei_dload"
#define USB_REPORT_MAGIC            "online_update"

/* 升级代码查询设置底层信息 时的消息长度最大值，
目前查询公钥最大，设为4K，后面有数据增加此处要调整 */
#define MSG_MAX_SIZE            (4 * 4096)

/******************************************************************************
  2 模块私有 (全局变量) 定义区:
******************************************************************************/
struct mutex ioctl_mutex;
STATIC struct class* dload_class = NULL;

/*******************************************************************
  函数名称  : map_search
  函数描述  : 根据请求消息信息索引映射函数
  函数入参  : ioctl_msg : 请求消息
  函数输出  : NV
  函数返回值: msg_func_t: 映射函数
********************************************************************/
STATIC msg_func_t map_search(ioctl_msg_t*  ioctl_msg)
{
    uint32 msg_map_index = 0;
    msg_func_t  msg_func;

    /* 计算映射索引 */
    msg_map_index  = MAP_INDEX(ioctl_msg->module_index, ioctl_msg->msg_index);
    
    /* 查找消息函数 */
    msg_func = search_map_callback(msg_map_index);
    if(NULL == msg_func)
    {
        DLOAD_PRINT(KERN_ERR, "msg_map_index = %lu : map callback is null.", msg_map_index);
        return NULL;
    }
    
    return msg_func;
}

/*******************************************************************
  函数名称  : nark_get_data
  函数描述  : 获取数据消息派发
  函数入参  : ioctl_msg : 请求消息
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR : 失败
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
        /* 释放内存 */
        kfree(local_buffer);
    }
        
    return ret;
}

/*******************************************************************
  函数名称  : nark_set_data
  函数描述  : 设置数据消息派发
  函数入参  : ioctl_msg : 请求消息
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR : 失败
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
  函数名称  : dload_Ioctl
  函数描述  : 交互框架驱动ioctl函数
  函数入参  : 
  函数输出  : NA
  函数返回值: DLOAD_OK    : 成功
              DLOAD_ERR : 失败
********************************************************************/
STATIC long dload_Ioctl(struct file *file, unsigned int cmd, unsigned long data)
{
    int32 ret  = DLOAD_OK;

    if((unsigned long)NULL == data)
    {
        DLOAD_PRINT(KERN_ERR, "data is NULL.");
        return DLOAD_ERR;
    }
    
    /* 检测命令的有效性 */
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

#define PORT_NORMAL         (0) /* 正常流程 */
#define PORT_NO_PCUI        (1) /* 不报pcui口 */
#define PORT_DLOAD          (2) /* 升级模式报口 */
#define PORT_NV_RES         (3) /* nv恢复模式报口 */
/******************************************************************************
*Function:           dload_report_port_status
*Description:        给usb模块上报端口状态值
*Calls:              
*Input:              
*Output:             NA
*Return:             0: normal, 1: 不报pcui， 2: 升级模式报口, 3: nv恢复模式报口
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

    if(SMEM_ONUP_FLAG_NUM == smem_data->smem_online_upgrade_flag /* 在线升级安装阶段 */
        || SMEM_SDUP_FLAG_NUM == smem_data->smem_sd_upgrade            /* SD升级安装阶段 */
        || SMEM_ONNR_FLAG_NUM == smem_data->smem_online_upgrade_flag   /* 在线升级NV恢复阶段 */
        || SMEM_SDNR_FLAG_NUM == smem_data->smem_online_upgrade_flag)  /* SD升级NV恢复阶段 */
    {
        /* ①在线、sd升级模式/nv恢复阶段端口上报处理 */
        status = PORT_NO_PCUI;
    }
    else if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /* ②一键升级模式端口上报处理 */
        status = PORT_DLOAD;
    }
    else if(SMEM_SWITCH_PUCI_FLAG_NUM == smem_data->smem_switch_pcui_flag)
    {
        /* ③一键升级模式端口上报处理 */
        status = PORT_NV_RES;
    }
    else
    {
        /* ④正常处理 */
        status = PORT_NORMAL;
    }

    DLOAD_PRINT(KERN_INFO, "Info: port mode is %d", (int)status);
    return status;
}
EXPORT_SYMBOL(dload_report_port_status);

/******************************************************************************
*Function:           dload_show_ota_info
*Description:        查看魔术字
*Calls:              
*Input:              
*Output:             NA
*Return:             魔术字的数值
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
*Description:        保存usb port上报的魔术字，根据保存的魔术字确定是否上报端口
*Calls:              
*Input:              
*Output:             NA
*Return:             非0: 上层往节点写入的数据长度, 0: 上层往节点写空
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
* 函 数 名   : dload_set_online_flag
* 功能描述  : at^godload中在重启前设置flag, 若非
         在线升级,在进入一键升级时清除该标记
         考虑到函数之后可能复用,因此不设置为static类型
* 输入参数   : 1--设置标志/0--清除标志/其他--报错
* 输出参数   : 无
* 返 回 值   : DLOAD_OK/DLOAD_ERR
* 其它说明  : at^godload命令调用,分区完整性标记不改变
******************************************************************/
int dload_set_auto_flag(unsigned int auto_flag)
{
    boolean bret = true;
    part_complete_flag part_complete_buf = {0};

    if(1 < auto_flag)
    {
        /*加强判断,只接收0/1两个值,其他均非法*/
        printk(KERN_ERR "dload_set_auto_flag: Input para error, %d.\n", auto_flag);
        return DLOAD_ERR;
    }

    bret = flash_get_share_region_info(RGN_PART_COMPLETE_FLAG,
                    (void*)&part_complete_buf,sizeof(part_complete_flag));
    if((true != bret)
        || (PART_COMPLETE_FLAG_MAGIC_NUM != part_complete_buf.magic_number))
    {
        part_complete_buf.dload_times = 0;
        part_complete_buf.complete_flag = 1;  /*正常模式下读出错设置为1*/
    }

    part_complete_buf.magic_number = PART_COMPLETE_FLAG_MAGIC_NUM;
    /*auto flag设置为魔数或0,参数合法性函数入口已判断*/
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
*Description:        设置保存自动升级标志,用来设置和清零,正常模式使用
*Calls:              
*Input:              内核class标准函数,输入0表示清零标志其他表示设置标志
*Output:             NA
*Return:             ok: 返回上层往节点写入的数据长度;
                   error: 其他
*Others:             NA
******************************************************************************/
STATIC ssize_t dload_store_auto_flag(struct class *class, 
            struct class_attribute *attr, const char *buf, size_t count)
{
    int ret = DLOAD_ERR;
    unsigned int wr_val = 0;

    if((NULL == buf) || (count < sizeof(unsigned int)))
    {
        /*入参判断,class&attr为内核固有函数入参,这里不判断*/
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
        &class_attr_auto_up,  /*online_flag--自动升级标志设置class文件,在线升级使用*/
};
#else

STATIC CLASS_ATTR(ota_up, S_IRUGO | S_IWUSR, dload_show_ota_info, dload_store_ota_info);
STATIC struct class_attribute *dload_attributes[] = {
        &class_attr_ota_up,   /* ota_up */
};
#endif

/******************************************************************************
*Function:           dload_init_class_files
*Description:        创建dload模块跟应用交互的sys/class/目录节点
*Calls:              class_create  /  class_create_file
*Input:              
*Output:             NA
*Return:             0: success, 非0: failed
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
*Description:        删除dload模块跟应用交互的sys/class/目录节点
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

    /* 获取webui版本号 */
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
        /* 华为MBB&&HOME ,自有webui */
        (void)snprintf(verTmp.version_buffer, WEBUI_VER_BUF_LEN, "%s", HUAWEI_WEBUI_PREFIX);

        /* 版本号不会超过128字节,规范定义,先保存版本号 */
        /* 多一个换行,去除这个换行字符 */
        if(bufTmp[length - 1] == '\r' || bufTmp[length - 1] == '\n')
        {
            bufTmp[length - 1]= '\0';
            length -= 1;
        }
        (void)strncat(verTmp.version_buffer, bufTmp, length);

        /* 保存下划线 */
        (void)strncat(verTmp.version_buffer, "_", strlen("_"));

        /* 保存产品形态名称 */
        memset(&platform_info, 0, sizeof(platform_info_t));
        (void)get_platform_information((uint8*)&platform_info, sizeof(platform_info_t));
        (void)strncat(verTmp.version_buffer, 
            platform_info.product_info_st.feature_name, strlen(platform_info.product_info_st.feature_name));
    }
    else
    {
        /* VDF webui */
        (void)snprintf(verTmp.version_buffer, WEBUI_VER_BUF_LEN, "%s", VDF_WEBUI_PREFIX);
        /* 版本号不会超过128字节,规范定义,先保存版本号 */
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
    
    /*nark api模块初始化*/
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

    /* 初始化互斥锁 */
    mutex_init(&ioctl_mutex);

    /* 创建升级proc节点 */
    proc_create("dload_nark", S_IRUGO, NULL, &dload_proc_fops);

#if  (FEATURE_ON == MBB_DLOAD_VER_VERIFY)
    /* 创建升级proc节点 */
    proc_create("flush_webver", 0660, NULL, &flush_web_proc_fops);  /*  */
#endif

    (void)dload_init_class_files();
    return ret;
}

STATIC void __exit dload_exit(void)
{
    int32 ret = 0;

    /* 移除proc节点 */
    remove_proc_entry("dload_nark", NULL); 
    
#if  (FEATURE_ON == MBB_DLOAD_VER_VERIFY)
    /* 移除proc节点 */
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
