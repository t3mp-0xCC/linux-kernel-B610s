/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2016. All rights reserved.
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



#include "carrier_adapter.h"
#include "drv_comm.h"
#include "bsp_version.h"
#include "bsp_rfile.h"
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <bsp_nvim.h>
#include <linux/stat.h>
#include <linux/mtd/flash_huawei_dload.h>
#include <bsp_icc.h>

#if (FEATURE_ON == MBB_COMMON)
#include "product_nv_id.h"
#include "product_nv_def.h"
#endif /* (FEATURE_ON == MBB_COMMON) */

#if (FEATURE_ON == MBB_DLOAD)
#include <bsp_sram.h>
#endif /* (FEATURE_ON == MBB_DLOAD) */

/*xml配置文件的产品配置信息的结构体变量*/
PRODUCT_CONFIG_INFO_STRU g_product_config_info;

static CA_HPLMN_WITH_MNC_LEN_STRU g_plmn_info_stru;
static struct semaphore carrier_adapter_sem;
static struct task_struct *carrier_adapter_tsk = NULL;
static struct wake_lock carrier_adapter_lock;

static int TASK_KEEP_RUNNING = TRUE;


/*****************************************************************************
 函 数 名  : carrier_strstr
 功能描述  : 在buf_start和buf_end之前查找第一个出现find_str字符串的地址，如果没有查找到的话返回NULL
 输入参数  : buf_start: 要查找数据的起始地址
             buf_end: 要查找数据的结束地址
 输出参数  : 无
 返 回 值  : 如果查找到指定格式的字符串就返回字符的地址，如果没有查找到就返回NULL
*****************************************************************************/
static char *carrier_strstr(const char *buf_start, const char *buf_end, const char *find_str)
{
    const char *str_temp = NULL;
    if ((buf_start == NULL) || (buf_end == NULL) || (find_str == NULL))
    {
        return NULL;
    }
    str_temp = strstr(buf_start, find_str);
    /*lint -save -e613*/
    if (str_temp > buf_end)
    {
        str_temp = NULL;
    }
    /*lint -restore*/
    return (char *)str_temp;
}

/*****************************************************************************
 函 数 名  : carrier_decode_config_info
 功能描述  : 从buffer中按照特定的格式解析数据存放在ca_support_sim_info的数据结构中去
 输入参数  : buf_start: 要查找数据的起始地址
             buf_end: 要查找数据的结束地址
 输出参数  : ca_support_sim_info: 把解析完成的数据以ca_support_sim_info的数据结构存放在此变量中
             plmn_cnt: 实际读到的plmn_cnt的数目
 返 回 值  : TRUE: 获取成功
             FALSE: 获取失败
*****************************************************************************/
static bool carrier_decode_config_info(char* buffer_start, char* buffer_end)
{

    char *buf_start = NULL;
    char *buf_end = NULL;
    char *buf_temp_end = NULL;
    int plmn_id = 0;
    int support_plmn_cnt = 0;
    bool ret = FALSE;

    if ((NULL == buffer_start) || (NULL == buffer_end))
    {
        carrier_adapter_log(LOG_ERR,"carrier_decode_config_info: param fail");
        return FALSE;
    }

    buf_start = buffer_start;

    while ((buf_start != NULL) && \
        ((buf_start = carrier_strstr(buf_start, buffer_end, START_PLMN_ID)) != NULL))
    {
        buf_end = carrier_strstr(buf_start, buffer_end, END_PLMN_ID);
        if (NULL == buf_end)
        {
            carrier_adapter_log(LOG_ERR,"carrier_decode_carrier: plmn index end fail");
            return FALSE;
        }
        g_product_config_info.ca_support_sim_info[plmn_id].plmn_name = \
            simple_strtol(buf_start + strlen(START_PLMN_ID), NULL, 0);

        buf_start = carrier_strstr(buf_start,buf_end,START_CARRIER_NAME);
        if (NULL == buf_start)
        {
            carrier_adapter_log(LOG_ERR, "carrier name start fail");
            return FALSE;
        }
        g_product_config_info.ca_support_sim_info[plmn_id].carrier_name = \
            simple_strtol(buf_start + strlen(START_CARRIER_NAME), NULL, 0);

        buf_start = carrier_strstr(buf_start, buf_end, START_PLMN_STR);
        if (NULL == buf_start)
        {
            carrier_adapter_log(LOG_ERR, "plmn str start fail");
            return FALSE;
        }
        buf_temp_end = carrier_strstr(buf_start, buf_end, END_PLMN_STR);
        if (NULL == buf_temp_end)
        {
            carrier_adapter_log(LOG_ERR, "plmn str end fail");
            return FALSE;
        }
        memcpy(g_product_config_info.ca_support_sim_info[plmn_id].plmn_str, \
            buf_start + strlen(START_PLMN_STR), \
            buf_temp_end - buf_start - strlen(START_PLMN_STR));

        buf_start = carrier_strstr(buf_start, buf_end, START_NV_PATH);
        if (NULL == buf_start)
        {
            carrier_adapter_log(LOG_ERR, "nv path start fail");
            return FALSE;
        }

        buf_temp_end = carrier_strstr(buf_start, buf_end, END_NV_PATH);
        if (NULL == buf_start)
        {
            carrier_adapter_log(LOG_ERR, "nv path end fail");
            return FALSE;
        }

        memcpy(g_product_config_info.ca_support_sim_info[plmn_id].cust_nv_path, \
            buf_start + strlen(START_NV_PATH), \
            buf_temp_end - buf_start - strlen(START_NV_PATH));

        plmn_id++;
        buf_start = buf_end;

        if (plmn_id >= MAXPLMN_ARRAY)
        {
            /*每个产品最多支持plmn组数*/
            carrier_adapter_log(LOG_WARN, "support  plmn_cnt max is %d", plmn_id);
            g_product_config_info.plmn_cnt = plmn_id;
            return TRUE;
        }

    }

    /*如果配置信息中的plmn组数和plmn计数的不符则以实际读到的为准*/
    if (g_product_config_info.plmn_cnt != plmn_id)
    {
        carrier_adapter_log(LOG_WARN, "not equal to plmn_id plmn_cnt=%d,plmn_id=%d", \
            g_product_config_info.plmn_cnt, plmn_id);
        g_product_config_info.plmn_cnt = plmn_id;
    }
    return TRUE;
}

/*****************************************************************************
 函 数 名  : carrier_decode_product_info
 功能描述  : 从buffer中按照特定的格式解析数据存放在g_product_config_info的数据结构中去
 输入参数  : buf_start: 要查找数据的起始地址
             buf_end: 要查找数据的结束地址
 输出参数  : g_product_config_info 把解析完成的数据以g_product_config_info的数据结构存放在此变量中
 返 回 值  : BSP_OK: 获取成功
             BSP_ERROR: 获取失败
*****************************************************************************/
static int carrier_decode_product_info(char* buffer, char* end)
{
    char *buf_start = NULL;
    char *buf_end = NULL;
    char *plmn_cnt_start = NULL;
    char *common_path_start = NULL;
    char *common_path_end = NULL;
    u32 plmn_cnt = 0;
    u32 board_id = 0;
    u32 temp_board_id = 0;
    const BSP_VERSION_INFO_S *board_info = NULL;

    if ((NULL == buffer) || (NULL == end))
    {
        carrier_adapter_log(LOG_ERR, "carrier_decode_product_info: param fail");
        return BSP_ERROR;
    }

    board_info = bsp_get_version_info();
    if (NULL == board_info)
    {
        carrier_adapter_log(LOG_ERR, "get board info failed");
        return BSP_ERROR;
    }
    board_id = board_info->board_id;

    buf_start = buffer;

    while ((buf_start != NULL) && \
        (NULL != (buf_start = carrier_strstr(buf_start,end, START_PRODUCT_ID))))
    {
        buf_end = carrier_strstr(buf_start, end, END_PRODUCT_ID);
        if (NULL == buf_end)
        {
            carrier_adapter_log(LOG_ERR, "product id end fail");
            return BSP_ERROR;
        }
        /*读取配置信息的产品id*/
        temp_board_id = simple_strtol(buf_start + strlen(START_PRODUCT_ID), NULL, 16);
        carrier_adapter_log(LOG_INFO, "board id=%d config file board_id=%d\n", board_id,temp_board_id);
        if (board_id == temp_board_id)
        {
            plmn_cnt_start = carrier_strstr(buf_start, buf_end, START_PLMN_CNT);
            if (NULL == plmn_cnt_start)
            {
                carrier_adapter_log(LOG_ERR,"plmn_cnt_start fail");
                return BSP_ERROR;
            }
            g_product_config_info.plmn_cnt = \
                simple_strtol(plmn_cnt_start + strlen(START_PLMN_CNT), NULL, 0);
            carrier_adapter_log(LOG_INFO,"product_config_info.plmn_cnt=%d", g_product_config_info.plmn_cnt);
            common_path_start = carrier_strstr(buf_start,buf_end,START_COMMON_NV_PATH);
            if (NULL == plmn_cnt_start)
            {
                carrier_adapter_log(LOG_ERR,"common_path_start fail");
                return BSP_ERROR;
            }
            common_path_end = carrier_strstr(common_path_start, buf_end, END_COMMON_NV_PATH);
            if (NULL == common_path_end)
            {
                carrier_adapter_log(LOG_ERR,"common_path_end fail");
                return BSP_ERROR;
            }
            memcpy(g_product_config_info.common_nv_path, \
                common_path_start + strlen(START_COMMON_NV_PATH), \
                common_path_end - common_path_start - strlen(START_COMMON_NV_PATH));
            carrier_adapter_log(LOG_INFO, "common_nv_path=%s", g_product_config_info.common_nv_path);
            break;
        }
        buf_start = buf_end;
    }

    if (FALSE == carrier_decode_config_info(buf_start, buf_end))
    {
        carrier_adapter_log(LOG_ERR, "carrier_decode_config_info failed");
        return BSP_ERROR;
    }
    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : decode_carrier_config_file
 功能描述  : 从 配置文件 中获取配置 信息
 输入参数  : src_path 配置文件路径
 输出参数  : count 配置文件的配置组数
 返 回 值  :  配置文件结构体地址: 获取成功
                        NULL: 获取失败
*****************************************************************************/
int decode_carrier_config_file(char *src_path)
{
    int fd = -1;
    int i;
    int ret = BSP_ERROR;
    long read_len = 0; 
    struct rfile_stat_stru fd_stat;
    char *data_start = NULL;
    char *data_end = NULL;

    /*检查一下文件是否存在*/
    if (0 != bsp_stat(CONFIG_FILE_PATH, &fd_stat))
    {
        carrier_adapter_log(LOG_ERR, "config file is not exit %s\n", CONFIG_FILE_PATH);
        return BSP_ERROR;
    }

    fd = bsp_open(CONFIG_FILE_PATH, RFILE_RDONLY , S_IRUSR | S_IWUSR);
    if (-1 == fd)
    {
        carrier_adapter_log(LOG_ERR, "bsp_open fail");
        return BSP_ERROR;
    }

    data_start = (char *)kmalloc((size_t)fd_stat.size, GFP_KERNEL);
    if (data_start == NULL)
    {
        carrier_adapter_log(LOG_ERR, "get memory fail");
        (void)bsp_close(fd);
        return BSP_ERROR;
    }
    data_end = data_start + fd_stat.size;
    /*从文件中读数据*/
    if (fd_stat.size != bsp_read(fd, data_start, fd_stat.size))
    {
        carrier_adapter_log(LOG_ERR, "bsp_read fail");
        (void)bsp_close(fd);
        kfree(data_start);
        return BSP_ERROR;
    }

    ret = carrier_decode_product_info(data_start, data_end);
    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "carrier_decode_product_info fail");
        bsp_close(fd);
        kfree(data_start);
        return BSP_ERROR;
    }

    bsp_close(fd);
    kfree(data_start);

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : get_carrier_info_from_oem
 功能描述  : 从 OMEINFO 中获取plmn 信息
 输入参数  : 无
 输出参数  : oeminfo: 获取到的plmn 信息
 返 回 值  :  BSP_OK: 获取成功
                        BSP_ERROR: 获取失败
*****************************************************************************/
static int get_carrier_info_from_oem(CA_OEM_INFO_STRU *oeminfo)
{
    bool ret = FALSE;

    if (NULL == oeminfo)
    {
        return BSP_ERROR;
    }

    /* 从oeminfo分区读取数据 */
    ret = flash_get_share_region_info(RGN_CARRIER_ADAPTER_NV_FLAG, oeminfo, \
        sizeof(CA_OEM_INFO_STRU));

    return (ret ? BSP_OK : BSP_ERROR);
}

/*****************************************************************************
 函 数 名  : set_carrier_info_to_oem
 功能描述  : 更新OMEINFO 中的plmn 信息
 输入参数  :oeminfo: 要写入的数据信息
 输出参数  : 无
 返 回 值  : BSP_OK: 更新成功
                       BSP_ERROR: 更新失败
*****************************************************************************/
static int set_carrier_info_to_oem(CA_OEM_INFO_STRU *oeminfo)
{
    bool ret = FALSE;

    if (NULL == oeminfo)
    {
        return BSP_ERROR;
    }

    /* 更新oeminfo 分区数据 */
    ret = flash_update_share_region_info(RGN_CARRIER_ADAPTER_NV_FLAG, oeminfo, sizeof(CA_OEM_INFO_STRU));

    return (ret ? BSP_OK : BSP_ERROR);
}

/*****************************************************************************
 函 数 名  : ca_decode_plmn_info
 功能描述  : 根据plmn 信息结构体，解析出plmn (字符串形式)
 输入参数  : stHplmn: 原始PLMN 信息结构体
 输出参数  : sim_plmn: plmn 字符串
 返 回 值  :  BSP_OK: 解析成功
                        BSP_ERROR: 解析失败
*****************************************************************************/
static int ca_decode_plmn_info(CA_HPLMN_WITH_MNC_LEN_STRU *stHplmn, char *sim_plmn)
{
    char plmn_temp[PLMN_SIZE_MAX];
    char basic_zero = '0';
    if ((NULL == stHplmn) || (NULL == sim_plmn))
    {
        carrier_adapter_log(LOG_ERR, "NULL pointer!");
        return BSP_ERROR;
    }

    (void)memset(plmn_temp, 0, sizeof(plmn_temp));

    /* 获取MCC的第1个数值 */
    plmn_temp[0] = ((FIRST_BIT_MASK & stHplmn->stHplmn.Mcc) >> FIRST_BIT_SHIFT) \
        + basic_zero;
    /* 获取MCC的第2个数值 */
    plmn_temp[1] = ((SECOND_BIT_MASK & stHplmn->stHplmn.Mcc) >> SECOND_BIT_SHIFT) \
        + basic_zero;
    /* 获取MCC的第3个数值 */
    plmn_temp[2] = ((THIRD_BIT_MASK & stHplmn->stHplmn.Mcc) >> THIRD_BIT_SHIFT) \
        + basic_zero;

    /*2位mnc*/
    if (2 == stHplmn->ucHplmnMncLen)
    {
        /* 获取MNC的第1个数值 */
        plmn_temp[3] = ((FIRST_BIT_MASK & stHplmn->stHplmn.Mnc) >> FIRST_BIT_SHIFT) \
            + basic_zero;
        /* 获取MNC的第2个数值 */
        plmn_temp[4] = ((SECOND_BIT_MASK & stHplmn->stHplmn.Mnc) >> SECOND_BIT_SHIFT) \
            + basic_zero;
    }
    /*3位mnc*/
    else if (3 == stHplmn->ucHplmnMncLen)
    {
        /* 获取MNC的第1个数值 */
        plmn_temp[3] = ((FIRST_BIT_MASK & stHplmn->stHplmn.Mnc) >> FIRST_BIT_SHIFT) \
            + basic_zero;
        /* 获取MNC的第2个数值 */
        plmn_temp[4] = ((SECOND_BIT_MASK & stHplmn->stHplmn.Mnc) >> SECOND_BIT_SHIFT) \
            + basic_zero;
        /* 获取MNC的第3个数值 */
        plmn_temp[5] = ((THIRD_BIT_MASK & stHplmn->stHplmn.Mnc) >> THIRD_BIT_SHIFT) \
            + basic_zero;
    }
    else
    {
        carrier_adapter_log(LOG_ERR, "HPLMN MNC LEN INVAILID");
        return BSP_ERROR;
    }

    (void)strncpy(sim_plmn, plmn_temp, strlen(plmn_temp));
    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_icc_cb
 功能描述  : SIM卡初始化后向A核发icc消息的回调函数
 输入参数  : 无
 输出参数  : 无
 返 回 值  : BSP_OK: 处理成功
                       BSP_ERROR: 处理失败
*****************************************************************************/
static signed int carrier_adapter_icc_cb(void)
{
    int read_len = 0;

    /* 随卡匹配的消息ID */
    unsigned int channel_id = (ICC_CHN_IFC << THIRD_BIT_SHIFT) | (IFC_RECV_FUNC_SIM_CA);

    /* 读取C 核发送的PLMN 信息 */
    read_len = bsp_icc_read(channel_id, (unsigned char *)&g_plmn_info_stru, \
        sizeof(g_plmn_info_stru));
    if ( sizeof(g_plmn_info_stru) != read_len )
    {
        carrier_adapter_log(LOG_ERR, "bsp_icc_read len is %d.", read_len);
        return BSP_ERROR;
    }

    carrier_adapter_log(LOG_INFO, "carrier_adapter_icc_cb proccess finish");
    up(&carrier_adapter_sem);

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_update_nv
 功能描述  : 切换nv 主流程
 输入参数  : carrier_name: 运营商名称
 输出参数  : 无
 返 回 值  : BSP_OK:成功  BSP_ERROR:失败
*****************************************************************************/
static int carrier_adapter_update_nv(int carrier_name)
{
    int ret = BSP_ERROR;
    char cust_nv_path[CA_CUST_PATH_MAX_LENGTH] = {0};

    /* 匹配plmn 对应的准入NV 文件路径 */
    ret = carrier_name_match_cust_nv_path(carrier_name, cust_nv_path);
    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "can not find cust nv path");
        return BSP_ERROR;
    }

    /* 解析xml 文件 */
    ret = bsp_nvm_ca_xml_decode(cust_nv_path);
    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_WARN, "decode xml error");
        return BSP_ERROR;
    }

    /* 更新CRC */
    ret = nv_make_ddr_crc();
    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_WARN, "Fail to caculate the crc error!");
        return BSP_ERROR;
    }
    /* 将最新数据写入各个分区 */
    ret = nv_data_writeback();
    if (NV_OK != ret)
    {
        carrier_adapter_log(LOG_WARN, "Fail to updata nv partitons!");
        return BSP_ERROR;
    }
    return BSP_OK;
}


/*****************************************************************************
 函 数 名  : plmn_get_carrier_name
 功能描述  : 获取plmn 所属的运营商
 输入参数  : plmn: plmn字符串信息
 输出参数  : out: 运营商名称
 返 回 值  : BSP_OK:成功  BSP_ERROR:失败
*****************************************************************************/
int plmn_get_carrier_name(int *out, const char *plmn)
{
    int ret = BSP_ERROR;
    int i = 0;
    int plmn_cnt = 0;

    if (NULL == out || NULL == plmn)
    {
        return ret;
    }

    /*判断是否为支持的PLMN总数*/
    if ((g_product_config_info.plmn_cnt <= 0) || \
        (g_product_config_info.plmn_cnt > MAXPLMN_ARRAY))
    {
        return ret;
    }

    plmn_cnt = g_product_config_info.plmn_cnt;


    /* 查表匹配PLMN 对应的运营商名称 */
    for (i = 0; i < plmn_cnt; i++)
    {
        ret = strcmp(g_product_config_info.ca_support_sim_info[i].plmn_str, plmn);
        if (0 == ret)
        {
            /* PLMN匹配运营商成功，且允许切换到该运营商 */
            *out = g_product_config_info.ca_support_sim_info[i].carrier_name;
            return BSP_OK;
        }
    }

    /* 如果找不到对应的plmn，则匹配其他运营商名称 */
    if (g_product_config_info.ca_support_sim_info[plmn_cnt - 1].carrier_name == \
        OTHERS_CARRIER_NAME)
    {
        carrier_adapter_log(LOG_INFO, "adapter other carriername is %d", \
            g_product_config_info.ca_support_sim_info[plmn_cnt - 1].carrier_name);
        *out = g_product_config_info.ca_support_sim_info[plmn_cnt - 1].carrier_name;
        return BSP_OK;
    }

    return BSP_ERROR;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_task
 功能描述  : 随卡匹配处理流程入口函数
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : BSP_OK:成功  BSP_ERROR:失败
*****************************************************************************/
static int carrier_adapter_task(void *args)
{
    int ret = BSP_ERROR;
    CA_OEM_INFO_STRU oeminfo;
    char sim_plmn[PLMN_SIZE_MAX];
    char oem_plmn[PLMN_SIZE_MAX];
    int oem_carrier_name = 0;
    int sim_carrier_name = 0;
    MTCSWT_AUTO_MANUL_STATUS_STRU switch_status;

    (void)memset(&oeminfo, 0, sizeof(oeminfo));
    (void)memset(sim_plmn, 0, sizeof(sim_plmn));

    /* 要求此任务的优先级比较高,唤醒了就执行*/
    set_user_nice(current, 10);

    /*获得配置文件解析的产品的配置信息*/
    ret = decode_carrier_config_file(CONFIG_FILE_PATH);
    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "decode_carrier_config_file fialed");
        return BSP_ERROR;
    }

    do
    {
        /* 一直等待直到获得信号量*/
        if (0 != down_interruptible(&carrier_adapter_sem))
        {
            carrier_adapter_log(LOG_INFO, "waiting for sema timeout");
            continue;
        }

        /* 切换前延迟系统 休眠5s ，防止模块休眠切换失效 */
        wake_lock_timeout(&carrier_adapter_lock, NV_SWITCH_WAIT_TIME);

        /* 解析PLMN 信息为字符串 */
        ret = ca_decode_plmn_info(&g_plmn_info_stru, sim_plmn);
        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "decode plmn info failed!");
            continue;
        }
        else
        {
            carrier_adapter_log(LOG_INFO, "SIM PLMN is %s", sim_plmn);
        }

        ret = NV_Read(NV_HUAWEI_MULTI_CARRIER_AUTO_SWITCH_I, \
            &switch_status, sizeof(switch_status));

        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "switch carrier NV_Read fialed");
            continue;
        }

        /* NV 50413 锁定配置的plmn的值当不为0时写入 */
        if (0 != switch_status.plmn)
        {
            carrier_adapter_log(LOG_INFO, "SIM PLMN is %s,nv plmn is 0x%x ", \
                sim_plmn, switch_status.plmn);
            ret = sprintf(sim_plmn, "%x", switch_status.plmn);
            carrier_adapter_log(LOG_INFO, "SIM PLMN is %s,nv plmn is 0x%x ret=%d", \
                sim_plmn, switch_status.plmn,ret);
        }

        /* 根据SIM 卡的PLMN 获取运营商名称 */
        ret = plmn_get_carrier_name(&sim_carrier_name, sim_plmn);
        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_WARN, "SIM carrier name is unknown");
            continue;
        }

        /* 读取oeminfo 分区存储的当前nv 的plmn 信息 */
        ret = get_carrier_info_from_oem(&oeminfo);
        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "failed to get oeminfo region");
            continue;
        }

        (void)snprintf(oem_plmn, (PLMN_SIZE_MAX - 1), oeminfo.plmn);

        carrier_adapter_log(LOG_INFO, "oeminfo stored plmn is '%s'", oem_plmn);
        /* 获取当前oem 分区存储的PLMN 的运营商名称 */
        (void)plmn_get_carrier_name(&oem_carrier_name, oem_plmn);

        /* oeminfo 的运营商 和sim 卡的运营商不同, 或者获取
               * oeminfo 分区运营商名称出错，都会启动nv切换流程 */
        if (sim_carrier_name == oem_carrier_name)
        {
            carrier_adapter_log(LOG_INFO, "don't need switch carrier NV");
            continue;
        }
        else
        {
            carrier_adapter_log(LOG_INFO, \
                "SIM Carrier changed, start to switch carrier NV");
        }

        /* 切换NV */
        ret = carrier_adapter_update_nv(sim_carrier_name);
        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "switch carrier nv failed, ret= %d", ret);
            continue;
        }
        else
        {
            carrier_adapter_log(LOG_INFO, "NV switch finish");
        }

        /* 更新oeminfo分区的plmn信息 */

        (void)memset(oeminfo.plmn, 0, PLMN_SIZE_MAX);
        (void)snprintf(oeminfo.plmn, (PLMN_SIZE_MAX - 1), sim_plmn);

        /* 新数据更新到oeminfo分区 */
        ret = set_carrier_info_to_oem(&oeminfo);
        if (BSP_OK != ret)
        {
            carrier_adapter_log(LOG_ERR, "update oeminfo failed");
        }
        else
        {
            carrier_adapter_log(LOG_INFO, "update oeminfo successfully");
        }
        /*更新成功后重启让配置信息重启生效*/
        mdrv_dload_normal_reboot();

    } while(TASK_KEEP_RUNNING);

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_task_init
 功能描述  : 创建随卡匹配任务
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : BSP_OK:成功  BSP_ERROR:失败
*****************************************************************************/
static int carrier_adapter_task_init(void)
{
    /* 第一次的时候创建多镜像的切换任务 */
    if (NULL == carrier_adapter_tsk)
    {
        carrier_adapter_tsk = kthread_run(carrier_adapter_task, NULL, CA_TSK_NAME);
        if (IS_ERR(carrier_adapter_tsk))
        {
            carrier_adapter_log(LOG_ERR, "fork failed for carrier_adapter_task");
            return BSP_ERROR;
        }
    }

    return BSP_OK;
}

/*****************************************************************************
 函 数 名  : carrier_adapter_init
 功能描述  : 随卡匹配模块初始化函数
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : BSP_OK:成功  其他: 错误码
*****************************************************************************/
static int __init carrier_adapter_init(void)
{
    int ret = BSP_ERROR;
    /* 随卡匹配的消息ID */
    unsigned int channel_id = (ICC_CHN_IFC << THIRD_BIT_SHIFT) | (IFC_RECV_FUNC_SIM_CA);
#if (FEATURE_ON == MBB_DLOAD)
    huawei_smem_info *smem_data = NULL;
#endif

    /* 初始化本模块全局变量 */
    (void)memset(&g_plmn_info_stru, 0, sizeof(g_plmn_info_stru));
    (void)memset(&g_product_config_info, 0, sizeof(g_product_config_info));

#if (FEATURE_ON == MBB_DLOAD)
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        carrier_adapter_log(LOG_INFO, "carrier_adapter_init: smem_data is NULL");
        return ret;
    }

    if (SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag)
    {
        /*升级模式，屏蔽nv模块的启动*/
        carrier_adapter_log(LOG_INFO, "entry update not init carrier_adapter !");
        return ret;
    }
#endif /* MBB_DLOAD */

    /* 初始化系统锁 */
    wake_lock_init(&carrier_adapter_lock, WAKE_LOCK_SUSPEND, CA_MODULE_NAME);

    /* 信号量初始化函数 */
    sema_init(&carrier_adapter_sem, 0);

    /* 创建随卡匹配NV 切换任务 */
    ret = carrier_adapter_task_init();
    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "running carrier adapter task failed");
        return ret;
    }

    /*注册icc回调函数，用来获取SIM 卡的PLMN 信息*/
    ret = bsp_icc_event_register(channel_id, \
        (read_cb_func)carrier_adapter_icc_cb, NULL, NULL, NULL);
    if (BSP_OK != ret)
    {
        carrier_adapter_log(LOG_ERR, "icc event register failed.");
        return BSP_ERROR;
    }

    carrier_adapter_log(LOG_INFO, "Init OK");
    return BSP_OK;
}

module_init(carrier_adapter_init);

