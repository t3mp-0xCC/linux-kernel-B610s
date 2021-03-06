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

/*===========================================================================
                       linux系统头文件
===========================================================================*/
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

/*===========================================================================
                       平台头文件
===========================================================================*/
#include "wlan_utils.h"

/*===========================================================================
                        内部使用对象声明
===========================================================================*/
STATIC int g_wlan_log_flag = WLAN_LOG_DRV_LOW;
#define WIFI_SH_TOOL        "/system/bin/sh"
#define SYSTEM_ENV_PATH     "PATH=/app/bin:/system/bin:/sbin:/bin:/usr/sbin:/usr/bin"

#define DEFINE_CHIP_INFO(chip) {chip, #chip}
static struct _chip_list 
{
    int type;
    const char *name;
}
g_chip_list[] =
{
    DEFINE_CHIP_INFO(bcm43362),
    DEFINE_CHIP_INFO(bcm43239),
    DEFINE_CHIP_INFO(bcm43241),
    DEFINE_CHIP_INFO(bcm43236),
    DEFINE_CHIP_INFO(bcm43217),
    DEFINE_CHIP_INFO(bcm4354),
    DEFINE_CHIP_INFO(bcm4352),
    DEFINE_CHIP_INFO(rtl8189),
    DEFINE_CHIP_INFO(rtl8192),
    DEFINE_CHIP_INFO(bcm4356),
    DEFINE_CHIP_INFO(bcm4359),
    DEFINE_CHIP_INFO(bcm4360),
};


/*===========================================================================

                        函数实现部分

===========================================================================*/
/*****************************************************************************
函数名称  : wlan_chip_type2name
功能描述  : 获取芯片名称
输入参数  : chip_type: 芯片类型
输出参数  : NA
返 回 值  : 芯片名称字符串
*****************************************************************************/
const char *wlan_chip_type2name(int chip_type)
{
    unsigned int idx = 0;

    for (idx = 0; idx < ARRAY_SIZE(g_chip_list); idx++)
    {
        if (chip_type == g_chip_list[idx].type)
        {
            return g_chip_list[idx].name;
        }
    }

    return "unknown";
}
EXPORT_SYMBOL(wlan_chip_type2name);

/*****************************************************************************
函数名称  : wlan_chip_name2type
功能描述  : 获取芯片类型
输入参数  : chip_name: 芯片名称
输出参数  : NA
返 回 值  : 芯片类型
*****************************************************************************/
int wlan_chip_name2type(const char *chip_name)
{
    unsigned int idx = 0;

    if (NULL != chip_name)
    {
        for (idx = 0; idx < ARRAY_SIZE(g_chip_list); idx++)
        {
            if (0 == strcmp(chip_name, g_chip_list[idx].name))
            {
                return g_chip_list[idx].type;
            }
        }
    }

    return WLAN_CHIP_MAX;
}
EXPORT_SYMBOL(wlan_chip_name2type);

/*****************************************************************************
 函数名称  : wlan_run_cmd
 功能描述  : 向WiFi芯片下发配置命令
 输入参数  : pcmd:执行的命令
 输出参数  : NA
 返 回 值  : WLAN_RETURN_TYPE
*****************************************************************************/
WLAN_RETURN_TYPE wlan_run_cmd(const char *pcmd)
{
    int  ret = 0;
    char *envp[] = {"HOME=/", SYSTEM_ENV_PATH, NULL};
    char **argv = NULL;

    ASSERT_NULL_POINTER(pcmd, WLAN_POINTER_NULL);

    argv = argv_split(0, pcmd, &ret);
    ASSERT_NULL_POINTER(argv, WLAN_POINTER_NULL);
    if(ret <= 0)
    {
        argv_free(argv);
        PLAT_WLAN_ERR("params=%d <= 0", ret);
        return WLAN_PARA_ERR;
    }

    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    argv_free(argv);

    PLAT_WLAN_INFO("ret = %d, run cmd:%s", (int)ret, pcmd);

    return  ((0 == ret) ? WLAN_SUCCESS : WLAN_EFFECT_FAILURE);
}
EXPORT_SYMBOL(wlan_run_cmd);

/*****************************************************************************
 函数名称  : wlan_run_shell
 功能描述  : 向WiFi芯片下发shell配置命令
 输入参数  : pcmd:执行的命令
 输出参数  : NA
 返 回 值  : WLAN_RETURN_TYPE
*****************************************************************************/
WLAN_RETURN_TYPE wlan_run_shell(const char *pshell)
{
    int  ret = 0;
    char *envp[] = {"HOME=/", SYSTEM_ENV_PATH, NULL};
    char *argv[] = {WIFI_SH_TOOL, "-c", (char *)pshell, NULL};

    ASSERT_NULL_POINTER(pshell, WLAN_POINTER_NULL);
    ret = call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
    PLAT_WLAN_INFO("ret = %d, run shell:%s", (int)ret, pshell);

    return  ((0 == ret) ? WLAN_SUCCESS : WLAN_EFFECT_FAILURE);
}
EXPORT_SYMBOL(wlan_run_shell);

/*****************************************************************************
 函数名称  : int wlan_read_file(const char *filename, char *buf, int len)
 功能描述  : 读取一行数据
 输入参数  : filp:数据指针，len:缓存大小
 输出参数  : buffer:数据缓存
 返 回 值  : int
*****************************************************************************/
int wlan_read_file(const char *filename, char *buf, int len)
{
    int bytes_read = 0;
    struct file *fp = NULL;
    mm_segment_t old_fs;

    ASSERT_NULL_POINTER(filename, WLAN_POINTER_NULL);
    ASSERT_NULL_POINTER(buf, WLAN_POINTER_NULL);
    if (len <= 0)
    {
        PLAT_WLAN_ERR("error with len = %d", (int)len);
    }
    PLAT_WLAN_INFO("Enty read_from_file:%s", filename);

    fp = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(fp))
    {
        PLAT_WLAN_ERR("Exit ret = %d, on NULL Pointer fp", (int)WLAN_POINTER_NULL);
        return WLAN_POINTER_NULL;
    }
    old_fs = get_fs();
    set_fs(get_ds());

    bytes_read = fp->f_op->read(fp, buf, len, &fp->f_pos);
    buf[bytes_read] = '\0';

    PLAT_WLAN_INFO("bytes_read = %d, buf:%s", bytes_read, buf);
    set_fs(old_fs);
    (void)filp_close(fp, NULL);

    return bytes_read;
}
EXPORT_SYMBOL(wlan_read_file);

/*****************************************************************************
 函数名称  : int wlan_strtoi(const char *nptr, char **endptr, WLAN_BASE_TYPE base)
 功能描述  : 字符串转整形数
 输入参数  : nptr:字符串指针，base:转换采用的进制
 输出参数  : endptr:结束位指针
 返 回 值  : int:转换后数值
*****************************************************************************/
int wlan_strtoi(const char *nptr, const char **endptr, WLAN_BASE_TYPE base)
{
    int sign = 1; /* 默认为正数 */
    int ret = 0;
    if (NULL == nptr)
    {
        return ret;
    }

    /* 去空格 */
    while (isspace(*nptr))
    {
        nptr++;
    }
    
    /* 正号判断 */
    while ('+' == *nptr)
    {
        nptr++;
    }
    
    /* 负号判断 */
    while ('-' == *nptr)
    {
        sign *= (-1);
        nptr++;
    }

    /* 数值转换 */
    while ((isdigit(*nptr) && (WLAN_BASE10 == base || WLAN_BASE16 == base))
        || (isalpha(*nptr) && (WLAN_BASE16 == base)))
    {
        if (isdigit(*nptr))
        {
            ret = (ret * base) + (*nptr - '0');
        }
        else /* a-f/A-F 转换 */
        {
            if (isupper(*nptr))
            {
                ret = (ret * base) + (*nptr - 'A');
            }
            else
            {
                ret = (ret * base) + (*nptr - 'a');
            }
            ret += WLAN_BASE10;
        }
        nptr++;
    }

    ret *= sign;
    if (NULL != endptr)
    {
        *endptr = nptr;
    }
    return ret;
}
EXPORT_SYMBOL(wlan_strtoi);

/*****************************************************************************
 函数名称  : int wlan_set_log_flag()
 功能描述  : 配置log打印
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
int wlan_set_log_flag(int flag)
{
    SYS_WLAN_INFO("set log_flag, old = %p, new = %p", (void *)g_wlan_log_flag, (void *)flag);
    g_wlan_log_flag = flag;
    return (0);
}
EXPORT_SYMBOL(wlan_set_log_flag);

/*****************************************************************************
 函数名称  : int wlan_get_log_flag(void)
 功能描述  : 获取log打印配置
 输入参数  : NA
 输出参数  : WLAN_LOG_TYPE
 返 回 值  : int
*****************************************************************************/
int wlan_get_log_flag(void)
{
    return g_wlan_log_flag;
}
EXPORT_SYMBOL(wlan_get_log_flag);

/*****************************************************************************
 函数名称  : wlan_log_fprintf
 功能描述  : 产线WT/MT内核log打印至文件
 输入参数  : filename:文件名
 返 回 值  : int
*****************************************************************************/
void wlan_log_fprintf(const char *fmt, ...)
{
#define WLAN_LOG_MAX_SIZE  (2 * 1024 * 1024) /* log占用最大空间 */
#define WLAN_LOG_FILE_PATH "/online/log/wifi_mfg.log"
    const char *filename = WLAN_LOG_FILE_PATH;
    struct file *fp = NULL;
    va_list args = {0};
    char log_buf[1024] = {0}; /* 单条log的最大长度 */
    unsigned int log_len = 0;
    mm_segment_t old_fs = {0};
    int ret = O_WRONLY | O_CREAT | O_APPEND; /* 默认用扩展写入方式打开文件 */
    static unsigned long s_total_log_size = 0;
    static unsigned long s_log_start_jiffies = 0;

    /* 获取log字符串 */
    log_len = snprintf(log_buf, sizeof(log_buf), "[%09lums] ", jiffies_to_msecs(jiffies - s_log_start_jiffies));

    va_start(args, fmt);
    log_len += vsnprintf(&log_buf[log_len], (sizeof(log_buf) - log_len), fmt, args);
    va_end(args);

    /* log文件大小判断，超过长度则截取 */
    if (s_total_log_size + log_len > WLAN_LOG_MAX_SIZE)
    {
        s_total_log_size = 0;
        ret = O_WRONLY | O_CREAT | O_TRUNC;
    }

    fp = filp_open(filename, ret, 0777); /* 打开文件 */
    if (IS_ERR(fp))
    {
        /* 初始化打开文件失败，则清除标记位，不再尝试写文件 */
        if (ret == (O_WRONLY | O_CREAT | O_TRUNC))
        {
            (void)wlan_set_log_flag(WLAN_LOG((WLAN_LOG_SIGN & (~WLAN_LOG_FILE)), WLAN_LOG_LEVE));
        }
        return;
    }
    
    old_fs = get_fs();
    set_fs(get_ds());

    s_total_log_size += do_sync_write(fp, log_buf, log_len, &fp->f_pos);

    set_fs(old_fs);
    (void)filp_close(fp, NULL);
}
EXPORT_SYMBOL(wlan_log_fprintf);

