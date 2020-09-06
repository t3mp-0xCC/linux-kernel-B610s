
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
 
/* linux 系统头文件 */
#ifdef WLAN_AT_API_APP_BUILD
    #include <unistd.h>
    #include <sys/wait.h>
#else
    #include <linux/module.h>
    #include <linux/init.h>
    #include <linux/moduleparam.h>
    #include <linux/kernel.h>
    #include "wlan_if.h"
#endif

#include "wlan_at.h"
#include "wlan_utils.h"

/* 对应的wifi芯片操作对象 */
STATIC WLAN_CHIP_OPS* g_wlan_ops[WLAN_CHIP_MAX] = {NULL};

/* 默认wt使用第一个芯片校准，当外部条件触发后，比如cradle插入等，需要修改该值逻辑 */
STATIC int32 g_cur_chip = -1;

#ifdef FEATURE_HUAWEI_MBB_BCM4352
extern WLAN_CHIP_OPS bcm4352_ops; 
#endif

#ifdef FEATURE_HUAWEI_MBB_BCM4356
extern WLAN_CHIP_OPS bcm4356_ops; 
#endif

#ifdef FEATURE_HUAWEI_MBB_BCM4360
extern WLAN_CHIP_OPS bcm4360_ops; 
#endif

#ifdef FEATURE_HUAWEI_MBB_BCM43217
extern WLAN_CHIP_OPS bcm43217_ops; 
#endif
/* 判断索引是否有效 */
#define CHIP_IDX_IS_VALID(idx) ((WLAN_CHIP_MAX > idx) && (idx > chipstub) && (NULL != g_wlan_ops[idx]))
/* 索引有效的断言 */
#define ASSERT_CHIP_IDX_VALID(idx, ret)  do { \
    if ((WLAN_CHIP_MAX <= idx) || (idx <= chipstub))\
    {\
        PLAT_WLAN_ERR("chip index valid = %d", idx); \
        return ret;\
    }\
    if (NULL == g_wlan_ops[idx])\
    {\
        PLAT_WLAN_ERR("chip index %d valid = NULL", idx); \
        return ret;\
    }\
}while(0)

#define WIFI_MODE_LEN (2)          /* wifi mode占用字节数 */
#define CHN_POW_LEN (2)            /* 2.4G信道或者发射功率占用的字节数 */
#define CHANN_LEN_5G  (6)          /* 5G占用的字节数 */
#define CHAR_BIT_SIZE (8)       /* 一字节包含8位 */
/* 检查天线掩码中是否包含天线索引信息 */
#define WLAN_GROUP_CHECK(mask, grp) /*bit(7)=1, bit(0-6)为子网掩码；bit(7)=0, bit(0-6)为数值*/ \
        (((mask) & 0x80) ? ((grp) == ((mask) & 0x7F)) : ((mask) & (1 << (grp)))) /* 包含检查 */


/* 平台提供的接口，判断是否产测模式 */
extern int bsp_get_factory_mode(void);

//////////////////////////////////////////////////////////////////////////
/*(1)^WIENABLE 设置WiFi模块使能 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
 功能描述  : 用于wifi 进入测试模式，正常模式，关闭wifi
 输入参数  :  0  关闭
              1  打开正常模式
              2  打开测试模式
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiEnable(WLAN_AT_WIENABLE_TYPE onoff)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiEnable, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiEnable(onoff);
}
/*****************************************************************************
 函数名称  : WLAN_AT_WIENABLE_TYPE WlanATGetWifiEnable()
 功能描述  : 获取当前的WiFi模块使能状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0  关闭
             1  正常模式(信令模式)
             2  测试模式(非信令模式)
 其他说明  : 
*****************************************************************************/
WLAN_AT_WIENABLE_TYPE WlanATGetWifiEnable(void)
{ 
    static int is_mp_start = 0;
    if (!is_mp_start)
    {
        is_mp_start = 1;
        (void)wlan_set_log_flag(WLAN_LOG((WLAN_LOG_STDOUT | WLAN_LOG_FILE), WLAN_LOG_DRV_LOW));
    }

    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_WIENABLE_OFF);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiEnable, AT_WIENABLE_OFF);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiEnable();
}

//////////////////////////////////////////////////////////////////////////
/*(2)^WIMODE 设置WiFi模式参数 目前均为单模式测试*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
 功能描述  : 设置WiFi AP支持的制式
 输入参数  : 0,  CW模式
             1,  802.11a制式
             2,  802.11b制式
             3,  802.11g制式
             4,  802.11n制式
             5,  802.11ac制式
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiMode(WLAN_AT_WIMODE_TYPE mode)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiMode, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiMode(mode);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 获取当前WiFi支持的制式
             当前模式，以字符串形式返回eg: 2
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiMode(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiMode, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiMode(strBuf);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 获取WiFi芯片支持的所有协议模式
             支持的所有模式，以字符串形式返回eg: 2,3,4
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiModeSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiModeSupport, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiModeSupport(strBuf);
}

//////////////////////////////////////////////////////////////////////////
/*(3)^WIBAND 设置WiFi带宽参数 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiBand(WLAN_AT_WIBAND_TYPE width)
 功能描述  : 用于设置wifi带宽
 输入参数  : 0 20M
             1 40M
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 只有在n模式下才可以设置40M带宽
*****************************************************************************/
int32 WlanATSetWifiBand(WLAN_AT_WIBAND_TYPE band)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiBand, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiBand(band);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  :获取当前带宽配置 
            当前带宽，以字符串形式返回eg: 0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiBand(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiBand, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiBand(strBuf);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  :获取WiFi支持的带宽配置 
            支持带宽，以字符串形式返回eg: 0,1
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiBandSupport(WLAN_AT_BUFFER_STRU *strBuf)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiBandSupport, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiBandSupport(strBuf);
}

//////////////////////////////////////////////////////////////////////////
/*(4)^WIFREQ 设置WiFi频点 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 设置WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiFreq, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiFreq(pFreq);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
 功能描述  : 获取WiFi频点
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiFreq(WLAN_AT_WIFREQ_STRU *pFreq)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiFreq, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiFreq(pFreq);
}

//////////////////////////////////////////////////////////////////////////
/*(5)^WIDATARATE 设置和查询当前WiFi速率集速率
  WiFi速率，单位为0.01Mb/s，取值范围为0～65535 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiDataRate(uint32 rate)
 功能描述  : 设置WiFi发射速率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiDataRate(uint32 rate)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiDataRate, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiDataRate(rate);
}
/*****************************************************************************
 函数名称  : uint32 WlanATGetWifiDataRate()
 功能描述  : 查询当前WiFi速率设置
 输入参数  : NA
 输出参数  : NA
 返 回 值  : wifi速率
 其他说明  : 
*****************************************************************************/
uint32 WlanATGetWifiDataRate(void)
{ 
    ASSERT_CHIP_IDX_VALID(g_cur_chip, 0);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiDataRate, 0);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiDataRate();
}

//////////////////////////////////////////////////////////////////////////
/*(6)^WIPOW 来设置WiFi发射功率 
   WiFi发射功率，单位为0.01dBm，取值范围为 -32768～32767 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiPOW(int32 power_dBm_percent)
 功能描述  : 设置WiFi发射功率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiPOW(int32 power_dBm_percent)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiPOW, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiPOW(power_dBm_percent);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiPOW()
 功能描述  : 获取WiFi当前发射功率
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiPOW(void)
{ 
    ASSERT_CHIP_IDX_VALID(g_cur_chip, 0);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiPOW, 0);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiPOW();
}

//////////////////////////////////////////////////////////////////////////
/*(7)^WITX 来设置WiFi发射机开关 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
 功能描述  : 打开或关闭wifi发射机
 输入参数  : 0 关闭
             1 打开
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiTX(WLAN_AT_FEATURE_TYPE onoff)
{  
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiTX, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiTX(onoff);
}

/*****************************************************************************
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATGetWifiTX()
 功能描述  : 查询当前WiFi发射机状态信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0 关闭发射机
             1 打开发射机
 其他说明  : 
*****************************************************************************/
WLAN_AT_FEATURE_TYPE WlanATGetWifiTX(void)
{ 
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_FEATURE_DISABLE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiTX, AT_FEATURE_DISABLE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiTX();
}

//////////////////////////////////////////////////////////////////////////
/*(8)^WIRX 设置WiFi接收机开关 */
//////////////////////////////////////////////////////////////////////////
int32 WlanATSetWifiRX(WLAN_AT_WIRX_STRU *params)
{  
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiRX, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiRX(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiRX(WLAN_AT_WIRX_STRU *params)
 功能描述  : 获取wifi接收机的状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiRX(WLAN_AT_WIRX_STRU *params)
{ 
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiRX, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiRX(params);
}

//////////////////////////////////////////////////////////////////////////
/*(9)^WIRPCKG 查询WiFi接收机误包码，上报接收到的包的数量*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiRPCKG(int32 flag)
 功能描述  : 清除Wifi接收统计包为零
 输入参数  : 0 清除wifi统计包
             非0 无效参数
 输出参数  : NA
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiRPCKG(int32 flag)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiRPCKG, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiRPCKG(flag);
}

/*****************************************************************************
 函数名称  : int32 WlanATSetWifiRPCKG(int32 flag)
 功能描述  : 查询WiFi接收机误包码，上报接收到的包的数量
 输入参数  : WLAN_AT_WIRPCKG_STRU *params
 输出参数  : uint16 good_result; //单板接收到的好包数，取值范围为0~65535
             uint16 bad_result;  //单板接收到的坏包数，取值范围为0~65535
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiRPCKG(WLAN_AT_WIRPCKG_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiRPCKG, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiRPCKG(params);
}

/*===========================================================================
 (10)^WIINFO 查询WiFi的相关信息
===========================================================================*/
/*****************************************************************************
 函数名称  : wlan_at_get_wifi_info
 功能描述  : 根据NV buf解析生成Info数据
 输入参数  : WLAN_AT_WIINFO_STRU *params: 查询参数
             char *buf: 查询的buffer;  int len: buffer的大小
 输出参数  : params->member.content: 组合字符串
 返 回 值  : 0 成功
             1 失败
 其他说明  : 
*****************************************************************************/
int32 wlan_at_get_wifi_info(WLAN_AT_WIINFO_STRU *params, const uint8 *buf, int buf_len)
{
    char *strBuf = NULL;
    int buf_idx = 0, str_idx = 0, str_len = 0;
    const uint8 *grpBuf = NULL;
    const uint8 *modeBuf = NULL;
    const uint8 *valBuf = NULL;
    uint8 chip_idx = 0, chip_count = 0, chip_id = 0, chip_len = 0;
    uint8 grp_mask = 0, grp_idx = 0, grp_len = 0;
    uint8 mode_idx = 0, mode_bit_idx = 0;
    uint8 val_len = 0, chn_idx = 0, chn_bit_idx = 0;
    
    const int DST_CHIP_ID = g_cur_chip;
    static const uint8 s_channel_24G[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14}; /* 2.4G 信道编号，第一位0为占位符 */
    static const uint8 s_channel_5G[] = {0,7,8,9,11,12,16,34,36,38,40,42,44,46,48,52,56,60,64   /* 5G编号 */
                                        ,100,104,108,112,116,120,124,128,132,136,140,149     /* 5G编号 */
                                        ,153,157,161,165,183,184,185,187,188,189,192,196};   /* 5G编号 */
    static const char *s_wifi_mode[] = {"a","b","g","n","n4","ac","ac4","ac8","ac16"}; /*wifi接入模式*/ 
    
    /* ++++++++++++++++++++ 有效性检查 +++++++++++++++++++++++ */
    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(buf, AT_RETURN_FAILURE);

    PLAT_WLAN_INFO("buf_len = %d, type = %d, group = %d, cur_chip = (%d)%s", buf_len
        , (int)params->type, (int)params->member.group, DST_CHIP_ID, wlan_chip_type2name(DST_CHIP_ID));
    if ((0 >= buf_len) || (0 > params->member.group))
    {
        PLAT_WLAN_ERR("Invalid param! buffer len = %d, group = %d", buf_len, (int)params->member.group);
        return AT_RETURN_FAILURE;
    }

    /*+++++++++++++++++++++++++++ 数据解析生成 ++++++++++++++++++++++++++++++*/
    str_len = sizeof(params->member.content);
    memset(params->member.content, 0, str_len);
    str_idx = 0;
    strBuf = (char *)params->member.content;

    /* 芯片头处理 */
    chip_idx = 0;
    /*版本号检查*/
    buf_idx = 0;
    if (1 != buf[buf_idx]) /*当前只支持版本号为1*/
    {
        PLAT_WLAN_ERR("Invalid version = %d", (int)buf[buf_idx]);
        return AT_RETURN_FAILURE;
    }
    buf_idx++;
    /* 记录芯片数量 */
    chip_count =  buf[buf_idx++];
    while((buf_idx < buf_len) && (chip_idx < chip_count))
    {
        chip_idx++;  /* 记录遍历芯片的数量 */
        chip_id =  buf[buf_idx++];   /* 芯片类型 */
        chip_len = buf[buf_idx++];   /* 芯片包含数据的长度 */

        /* 控制log打印 */
        PLAT_WLAN_DETAIL(WLAN_LOG_DRV_HI
                    , "chip_len = %d, cur_chip = (%d)%s", chip_len, chip_id, wlan_chip_type2name(chip_id));

        /* 芯片类型检查 */
        if (buf_idx + chip_len > buf_len)
        {
            PLAT_WLAN_ERR("chip_idx + chip_len = %d > len = %d", (int)(buf_idx + chip_len), buf_len);
            return AT_RETURN_FAILURE;
        }

        grpBuf = &buf[buf_idx];
        buf_idx += chip_len;    /*跳转到下一个芯片数据*/
        if (DST_CHIP_ID != chip_id)
        {
            continue;
        }

        /* 查找天线记录 */
        grp_idx = 0;
        grp_mask = grpBuf[grp_idx++];
        grp_len = grpBuf[grp_idx++];
        while ((grp_idx + grp_len) <= chip_len) /* 天线包含数据的有效性检查 */
        {
            /* 控制log打印 */
            PLAT_WLAN_DETAIL(WLAN_LOG_DRV_HI, "grp_len = %d, grp_mask = 0x%x", grp_len, grp_mask);

            /* 天线数据有效性检查 */
            if ((WIFI_MODE_LEN + CHN_POW_LEN) > grp_len)
            {
                PLAT_WLAN_ERR("grp_len = %d < %d", grp_len, (WIFI_MODE_LEN + CHN_POW_LEN));
                return AT_RETURN_FAILURE;
            }

            /*生成天线数据*/
            if (WLAN_GROUP_CHECK(grp_mask, params->member.group))
            {
                modeBuf = &grpBuf[grp_idx];
                valBuf = &modeBuf[WIFI_MODE_LEN];
                if ((AT_WIINFO_POWER == params->type)   /*power占两个字节*/
                    || (0 == (valBuf[0] & 0x01))        /* 低位为0表示2.4G，占2个字节 */
                    )
                {
                    val_len = CHN_POW_LEN;
                }
                else 
                {
                    val_len = CHANN_LEN_5G;
                }
                while ((modeBuf + WIFI_MODE_LEN + val_len) <= (grpBuf + grp_idx + grp_len)) 
                {
                    /* 控制log打印 */
                    PLAT_WLAN_DETAIL(WLAN_LOG_DRV_HI, "mode_mask = 0x%04X, val_len = %d, value = 0x%04X"
                                        , *((short *)modeBuf), val_len, *((short *)valBuf));

                    for (mode_idx = 0; mode_idx < WIFI_MODE_LEN; mode_idx++)
                    {
                        for (mode_bit_idx = 0; mode_bit_idx < CHAR_BIT_SIZE; mode_bit_idx++)
                        {
                            if ((0 == (modeBuf[mode_idx] & (1 << mode_bit_idx)))
                                || (ARRAY_SIZE(s_wifi_mode) <= ((mode_idx * CHAR_BIT_SIZE) + mode_bit_idx)))
                            {
                                continue;
                            }
                            
                            /* 记录WiFi接入模式 */
                            str_idx += snprintf(&strBuf[str_idx], (str_len - str_idx), "%s"
                                                , s_wifi_mode[(mode_idx * CHAR_BIT_SIZE) + mode_bit_idx]);
                            /* 记录对应数据 */
                            switch (params->type)
                            {
                                case AT_WIINFO_POWER:
                                {
                                    str_idx += snprintf(&strBuf[str_idx], (str_len - str_idx), ",%d"
                                                        , (int)(*((unsigned short *)valBuf)));
                                    break;
                                }
                                case AT_WIINFO_FREQ:
                                {
                                    str_idx += snprintf(&strBuf[str_idx], (str_len - str_idx), ",%d"
                                                        , (valBuf[0] & 0x01));
                                    break;
                                }
                                case AT_WIINFO_CHANNEL:
                                {
                                    for (chn_idx = 0; chn_idx < val_len; chn_idx++)
                                    {
                                        for (chn_bit_idx = 0; chn_bit_idx < CHAR_BIT_SIZE; chn_bit_idx++)
                                        {
                                            if (((0 == chn_idx) && (0 == chn_bit_idx)) /* 跳过第一位 */
                                                || (0 == (valBuf[chn_idx] & (1 << chn_bit_idx))))
                                            {
                                                continue;
                                            }
                                            if (0 == (valBuf[0] &0x01))
                                            {
                                                /* 2.4G处理 */
                                                if (ARRAY_SIZE(s_channel_24G) <= ((chn_idx * CHAR_BIT_SIZE) + chn_bit_idx))
                                                {
                                                    PLAT_WLAN_ERR("bit = %d is SET!", ((chn_idx * CHAR_BIT_SIZE) + chn_bit_idx));
                                                    continue;
                                                }
                                                str_idx += snprintf(&strBuf[str_idx], (str_len - str_idx), ",%d"
                                                                , (int)s_channel_24G[(chn_idx * CHAR_BIT_SIZE) + chn_bit_idx]);
                                            }
                                            else
                                            {
                                                /* 5G处理 */
                                                if (ARRAY_SIZE(s_channel_5G) <= ((chn_idx * CHAR_BIT_SIZE) + chn_bit_idx))
                                                {
                                                    PLAT_WLAN_ERR("bit = %d is SET!", ((chn_idx * CHAR_BIT_SIZE) + chn_bit_idx));
                                                    continue;
                                                }
                                                str_idx += snprintf(&strBuf[str_idx], (str_len - str_idx), ",%d"
                                                                , (int)s_channel_5G[(chn_idx * CHAR_BIT_SIZE) + chn_bit_idx]);
                                            }
                                        }
                                    }
                                    break;
                                }
                                default:
                                {
                                    PLAT_WLAN_ERR("Error grp_type = %d", params->type);
                                    return AT_RETURN_FAILURE;
                                }
                            }
                            
                            /* 下一条处理数据 */
                            if ((str_idx + 1) >= str_len)
                            {
                                PLAT_WLAN_ERR("Error str_len = %d is not enough!", str_len);
                                return AT_RETURN_FAILURE;
                            }
                            strBuf[str_idx++] = '\0';  /* 一行数据结束，用'\0'分隔 */
                        }
                    }

                    /* 查询下一个数据 */
                    modeBuf += (WIFI_MODE_LEN + val_len);
                    valBuf = &modeBuf[WIFI_MODE_LEN];
                    if ((AT_WIINFO_POWER == params->type)   /*power占两个字节*/
                        || (0 == (valBuf[0] & 0x01))        /* 低位为0表示2.4G，占2个字节 */
                        )
                    {
                        val_len = CHN_POW_LEN;
                    }
                    else 
                    {
                        val_len = CHANN_LEN_5G;
                    }
                }
                
                /* 天线数据有效性检查 */
                if ((modeBuf - grpBuf) != (grp_idx + grp_len))
                {
                    PLAT_WLAN_ERR("grp_len = %d != %d", (int)(modeBuf - grpBuf), (grp_idx + grp_len));
                    return AT_RETURN_FAILURE;
                }
            }
            
            /* 查询下一组天线数据 */
            grp_idx += grp_len;
            grp_mask = grpBuf[grp_idx++];
            grp_len = grpBuf[grp_idx++];
        }
    }

    /* 检查是否有写入数据 */
    if (0 == strlen(strBuf))
    {
        PLAT_WLAN_ERR("No chip DATA!!");
        return AT_RETURN_FAILURE;
    }
    
    return AT_RETURN_SUCCESS;
}
/*****************************************************************************
 函数名称  : ATGetWifiInfo
 功能描述  : 查询WiFi的相关信息(内部接口)
 输入参数  : NA
 输出参数  : NA
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATGetWifiInfo(WLAN_AT_WIINFO_STRU *params)
{
#ifdef WLAN_NV_READ_WIINFO
    static int s_len_channel = 0;
    static uint8 *s_nv_channel = NULL;
    static int s_len_power = 0;
    static uint8 *s_nv_power = NULL;

    int *p_len = NULL;
    uint8 **p_nv_buffer = NULL;
    int32 ret = 0;
#endif /*WLAN_NV_READ_WIINFO*/

    ASSERT_NULL_POINTER(params, AT_RETURN_FAILURE);
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);

    PLAT_WLAN_INFO("Enter [group=%d,type=%d]", (int)params->member.group, params->type);

    /* NV解析方式，处理失败则用非NV方式处理 */
#ifdef WLAN_NV_READ_WIINFO
    switch (params->type)
    {
        case AT_WIINFO_CHANNEL:    /*支持的信道号*/
        case AT_WIINFO_FREQ:       /*支持的频段*/
        {
            p_len = &s_len_channel;
            p_nv_buffer = &s_nv_channel;
            break;
        }
        case AT_WIINFO_POWER:       /*支持的目标功率*/
        {
            p_len = &s_len_power;
            p_nv_buffer = &s_nv_power;
            break;
        }
        default:
        {
            PLAT_WLAN_ERR("Error type = %d", params->type);
            break;
        }
    }
    /* 获取nv缓存数据 */
    if ((NULL != p_nv_buffer) && (NULL == *p_nv_buffer))
    {
        *p_nv_buffer = (uint8 *)WLAN_NV_READ_WIINFO(params->type, p_len);
    }
    /* nv数据解析 */
    if ((NULL != p_nv_buffer) && (NULL != *p_nv_buffer))
    {
        ret = wlan_at_get_wifi_info(params, *p_nv_buffer, *p_len);
        if (AT_RETURN_SUCCESS == ret)
        {
            return ret;
        }            
    }
#endif /*WLAN_NV_READ_WIINFO*/

    /* 采用非NV的解析方式 */
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiInfo, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiInfo(params);
}

//////////////////////////////////////////////////////////////////////////
/*(11)^WIPLATFORM 查询WiFi方案平台供应商信息 */
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform()
 功能描述  : 查询WiFi方案平台供应商信息
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
WLAN_AT_WIPLATFORM_TYPE WlanATGetWifiPlatform(void)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiPlatform, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiPlatform();
}

//////////////////////////////////////////////////////////////////////////
/*(12)^TSELRF 查询设置单板的WiFi射频通路*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetTSELRF(uint32 group)
 功能描述  : 设置天线，非多通路传0
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATSetTSELRF(uint32 group)
{  
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetTSELRF, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetTSELRF(group);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 支持的天线索引序列，以字符串形式返回eg: 0,1,2,3
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetTSELRFSupport(WLAN_AT_BUFFER_STRU *strBuf)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetTSELRFSupport, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetTSELRFSupport(strBuf);
}

//////////////////////////////////////////////////////////////////////////
/*(13)^WiPARANGE设置、读取WiFi PA的增益情况*/
//////////////////////////////////////////////////////////////////////////
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiParange(WLAN_AT_WiPARANGE_TYPE pa_type)
 功能描述  : 设置WiFi PA的增益情况
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATSetWifiParange(WLAN_AT_WiPARANGE_TYPE pa_type)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiParange, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiParange(pa_type);
}

/*****************************************************************************
 函数名称  : WLAN_AT_WiPARANGE_TYPE WlanATGetWifiParange()
 功能描述  : 读取WiFi PA的增益情况
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
WLAN_AT_WiPARANGE_TYPE WlanATGetWifiParange(void)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_WiPARANGE_BUTT);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiParange, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiParange();
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiParangeSupport(WLAN_AT_BUFFER_STRU *strBuf)
 功能描述  : 支持的pa模式序列，以字符串形式返回eg: l,h
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NA
 其他说明  : 
*****************************************************************************/
int32 WlanATGetWifiParangeSupport(WLAN_AT_BUFFER_STRU *strBuf)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiParangeSupport, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiParangeSupport(strBuf);
}


/*===========================================================================
 (14)^WICALTEMP设置、读取WiFi的温度补偿值
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalTemp(WLAN_AT_WICALTEMP_STRU *params)
 功能描述  : 设置WiFi的温度补偿值
 输入参数  : NA
 输出参数  : params:温度补偿参数
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATGetWifiCalTemp(WLAN_AT_WICALTEMP_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalTemp, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalTemp(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalTemp(WLAN_AT_WICALTEMP_STRU *params)
 功能描述  : 设置WiFi的温度补偿值
 输入参数  : params:温度补偿参数
 输出参数  : NA
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATSetWifiCalTemp(WLAN_AT_WICALTEMP_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCalTemp, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCalTemp(params);
}

/*===========================================================================
 (15)^WICALDATA设置、读取指定类型的WiFi补偿数据
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalData(WLAN_AT_WICALDATA_STRU * params)
 功能描述  : 指定类型的WiFi补偿数据
 输入参数  : NA
 输出参数  : params:补偿数据
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATGetWifiCalData(WLAN_AT_WICALDATA_STRU * params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip,AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalData,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalData(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATSetWifiCalData(WLAN_AT_WICALDATA_STRU *params)
 功能描述  : 指定类型的WiFi补偿数据
 输入参数  : params:补偿数据
 输出参数  : NA
 返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
int32 WlanATSetWifiCalData(WLAN_AT_WICALDATA_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCalData, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCalData(params);
}

/*===========================================================================
 (16)^WICAL设置、读取校准的启动状态，是否支持补偿
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiCal(WLAN_AT_FEATURE_TYPE onoff)
 功能描述  : 设置校准的启动状态
 输入参数  : onoff:0,结束校准；1,启动校准
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATSetWifiCal(WLAN_AT_FEATURE_TYPE onoff)
{  
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCal, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCal(onoff);
}

/*****************************************************************************
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATGetWifiCal(void)
 功能描述  : 读取校准的启动状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
WLAN_AT_FEATURE_TYPE WlanATGetWifiCal(void)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_FEATURE_DISABLE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCal, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCal();
}

/*****************************************************************************
 函数名称  : WLAN_AT_FEATURE_TYPE WlanATGetWifiCalSupport(void)
 功能描述  : 是否支持校准
 输入参数  : NA
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
WLAN_AT_FEATURE_TYPE WlanATGetWifiCalSupport(void)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_FEATURE_DISABLE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalSupport, AT_FEATURE_DISABLE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalSupport();
}

/*===========================================================================
 (17)^WICALFREQ 设置、查询频率补偿值
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATSetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
 功能描述  : 设置频率补偿
 输入参数  : params:补偿参数
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATSetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCalFreq, AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCalFreq(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
 功能描述  : 设置频率补偿
 输入参数  : NA
 输出参数  : params:补偿参数
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATGetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalFreq,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalFreq(params);
}

/*===========================================================================
 (18)^WICALPOW 设置、查询功率补偿值
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalFreq(WLAN_AT_WICALFREQ_STRU *params)
 功能描述  : 设置功率补偿
 输入参数  : NA
 输出参数  : params:补偿参数
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATSetWifiCalPOW(WLAN_AT_WICALPOW_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifiCalPOW,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifiCalPOW(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifiCalPOW(WLAN_AT_WICALPOW_STRU *params)
 功能描述  : 校准发射功率时，查询对应值
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int32
*****************************************************************************/
int32 WlanATGetWifiCalPOW(WLAN_AT_WICALPOW_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifiCalPOW,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifiCalPOW(params);
}

/*===========================================================================
 (19)^WIPAVARS2G 设置、查询功2.4G的wifi射频参数
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATSetWifi2GPavars(WLAN_AT_WICALFREQ_STRU *params)
 功能描述  : 设置2.4G的wifi射频参数
 输入参数  : 2.4G射频校准的值
 输出参数  : NA
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATSetWifi2GPavars(WLAN_AT_PAVARS2G_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifi2GPavars,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifi2GPavars(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifi2GPavars(WLAN_AT_WICALPOW_STRU *params)
 功能描述  : 读取2.4G的wifi射频参数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int32
*****************************************************************************/
int32 WlanATGetWifi2GPavars(WLAN_AT_BUFFER_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifi2GPavars,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifi2GPavars(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetInitWifi2GPavars(WLAN_AT_WICALPOW_STRU *params)
 功能描述  : 读取2.4G初始的wifi射频参数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int32
*****************************************************************************/
int32 WlanATGetInitWifi2GPavars(WLAN_AT_BUFFER_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetInitWifi2GPavars,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetInitWifi2GPavars(params);
}

/*===========================================================================
 (20)^WIPAVARS5G 设置、查询功5G的wifi射频参数
===========================================================================*/
/*****************************************************************************
 函数名称  : int32 WlanATSetWifi5GPavars(WLAN_AT_WICALFREQ_STRU *params)
 功能描述  : 设置5G的wifi射频参数
 输入参数  : NA
 输出参数  : params
 返 回 值  : WLAN_AT_FEATURE_TYPE
*****************************************************************************/
int32 WlanATSetWifi5GPavars(WLAN_AT_PAVARS5G_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATSetWifi5GPavars,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATSetWifi5GPavars(params);
}

/*****************************************************************************
 函数名称  : int32 WlanATGetWifi5GPavars(WLAN_AT_WICALPOW_STRU *params)
 功能描述  : 读取5G的wifi射频参数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int32
*****************************************************************************/
int32 WlanATGetWifi5GPavars(WLAN_AT_BUFFER_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetWifi5GPavars,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetWifi5GPavars(params);
}
/*****************************************************************************
 函数名称  : int32 WlanATGetInitWifi5GPavars(WLAN_AT_WICALPOW_STRU *params)
 功能描述  : 读取5G初始的wifi射频参数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int32
*****************************************************************************/
int32 WlanATGetInitWifi5GPavars(WLAN_AT_BUFFER_STRU *params)
{   
    ASSERT_CHIP_IDX_VALID(g_cur_chip, AT_RETURN_FAILURE);
    ASSERT_NULL_POINTER(g_wlan_ops[g_cur_chip]->WlanATGetInitWifi5GPavars,AT_RETURN_FAILURE);
    return g_wlan_ops[g_cur_chip]->WlanATGetInitWifi5GPavars(params);
}

EXPORT_SYMBOL(WlanATSetWifi2GPavars);
EXPORT_SYMBOL(WlanATGetWifi2GPavars);
EXPORT_SYMBOL(WlanATSetWifi5GPavars);
EXPORT_SYMBOL(WlanATGetWifi5GPavars);
EXPORT_SYMBOL(WlanATGetInitWifi2GPavars);
EXPORT_SYMBOL(WlanATGetInitWifi5GPavars);

/***********************************************************************************
Function:          wlan_at_switch_chip
Description:    wt 模块初始化
Calls:
Input:           
Output:            NA
Return:            NULL or node
                 
************************************************************************************/
void wlan_at_switch_chip(int chip_type)
{
    PLAT_WLAN_INFO("switch chip from (%d)%s to (%d)%s \n"
        , g_cur_chip, wlan_chip_type2name(g_cur_chip), chip_type, wlan_chip_type2name(chip_type));
    g_cur_chip = (int32)chip_type;
}
EXPORT_SYMBOL(wlan_at_switch_chip);


/*****************************************************************************
函数名称  : wlan_at_reg_chip
功能描述  : 芯片注册函数
输入参数  : NA
输出参数  : NA
返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
WLAN_AT_RETURN_TYPE wlan_at_reg_chip(int32 chip_type, WLAN_CHIP_OPS *chip_ops)
{
    /* 0:产线模式；1:正常使用模式，默认正常模式, 读取nv36，判断是否产线模式，暂未使用 */
#ifdef MBB_WIFI_CHECK_FACTORY_MODE
    int wlan_mode = 1; 

    wlan_mode = bsp_get_factory_mode();
    PLAT_WLAN_INFO("factory_mode = %d",wlan_mode);
    if (1 == wlan_mode)
    {
        /* 正常模式，返回 */
        return AT_RETURN_FAILURE;
    }
#endif /* MBB_WIFI_CHECK_FACTORY_MODE */

    if (chip_type < 0 || chip_type >= (int)ARRAY_SIZE(g_wlan_ops))
    {
        PLAT_WLAN_ERR("chip idx valid = %d", chip_type);
        return AT_RETURN_FAILURE;
    }

    PLAT_WLAN_INFO("chip %d = (%s) reg %p to %p"
        , chip_type, wlan_chip_type2name(chip_type), g_wlan_ops[chip_type], chip_ops);
    g_wlan_ops[chip_type] = chip_ops;
    if (!CHIP_IDX_IS_VALID(g_cur_chip))
    {
        g_cur_chip = chip_type;
    }
    PLAT_WLAN_INFO("Exit, g_cur_chip= %d\n",g_cur_chip);
    return AT_RETURN_SUCCESS;
}
EXPORT_SYMBOL(wlan_at_reg_chip);

/*****************************************************************************
函数名称  : wlan_at_unreg_chip
功能描述  : 芯片去注册函数
输入参数  : NA
输出参数  : NA
返 回 值  : WLAN_AT_RETURN_TYPE
*****************************************************************************/
WLAN_AT_RETURN_TYPE wlan_at_unreg_chip(int32 chip_type)
{
    ASSERT_CHIP_IDX_VALID(chip_type, AT_RETURN_SUCCESS);
    
    PLAT_WLAN_INFO("Exit, chip %d = (%s) unreg from %p"
        , chip_type, wlan_chip_type2name(chip_type), g_wlan_ops[chip_type]);
    g_wlan_ops[chip_type] = NULL;
    
    return AT_RETURN_SUCCESS;
}
EXPORT_SYMBOL(wlan_at_unreg_chip);

/*****************************************************************************
 函数名称  : wlan_wt_init
 功能描述  : wlan at root 初始化函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
static int __init wlan_wt_init(void)
{
    int i = 0;

#ifdef FEATURE_HUAWEI_MBB_BCM4352
    wlan_at_reg_chip(bcm4352, &bcm4352_ops);
    g_cur_chip = bcm4352;        
#endif

#ifdef FEATURE_HUAWEI_MBB_BCM4356
    wlan_at_reg_chip(bcm4356, &bcm4356_ops);
    g_cur_chip = bcm4356;        
#endif

#ifdef FEATURE_HUAWEI_MBB_BCM4360
    wlan_at_reg_chip(bcm4360, &bcm4360_ops);
    g_cur_chip = bcm4360;        
#endif

#ifdef FEATURE_HUAWEI_MBB_BCM43217
    wlan_at_reg_chip(bcm43217, &bcm43217_ops);
    g_cur_chip = bcm43217;
#endif

    PLAT_WLAN_INFO("enter curr_chip %d = (%s)", g_cur_chip, wlan_chip_type2name(g_cur_chip));
   
    for (i = 0; i < (int)ARRAY_SIZE(g_wlan_ops); i++)
    {
        if (NULL != g_wlan_ops[i])
        {
            PLAT_WLAN_INFO("chip %d = (%s) reg to %p", i, wlan_chip_type2name(i), g_wlan_ops[i]);
        }
    }

    PLAT_WLAN_INFO("exit");
    return AT_RETURN_SUCCESS;
}

 /*****************************************************************************
 函数名称  : wlan_wt_exit
 功能描述  : wlan at root 去初始化函数
 输入参数  : NA
 输出参数  : NA
 返 回 值  : int
*****************************************************************************/
static void __exit wlan_wt_exit(void)
{
    PLAT_WLAN_INFO("enter, curr_chip = %d", g_cur_chip);
    memset(g_wlan_ops, 0x0, sizeof(g_wlan_ops));
    g_cur_chip = -1;
    PLAT_WLAN_INFO("exit");
}

module_init(wlan_wt_init); /*lint !e529*/
module_exit(wlan_wt_exit); /*lint !e529*/

MODULE_AUTHOR("Huawei");
MODULE_DESCRIPTION("Huawei WT");
MODULE_LICENSE("GPL");

//////////////////////////////////////////////////////////////////////////
EXPORT_SYMBOL(WlanATSetWifiEnable);
EXPORT_SYMBOL(WlanATGetWifiEnable);
EXPORT_SYMBOL(WlanATSetWifiMode);
EXPORT_SYMBOL(WlanATGetWifiMode);
EXPORT_SYMBOL(WlanATGetWifiModeSupport);
EXPORT_SYMBOL(WlanATSetWifiBand);
EXPORT_SYMBOL(WlanATGetWifiBand);
EXPORT_SYMBOL(WlanATGetWifiBandSupport);
EXPORT_SYMBOL(WlanATSetWifiFreq);
EXPORT_SYMBOL(WlanATGetWifiFreq);
EXPORT_SYMBOL(WlanATSetWifiDataRate);
EXPORT_SYMBOL(WlanATGetWifiDataRate);
EXPORT_SYMBOL(WlanATSetWifiPOW);
EXPORT_SYMBOL(WlanATGetWifiPOW);
EXPORT_SYMBOL(WlanATSetWifiTX);
EXPORT_SYMBOL(WlanATGetWifiTX);
EXPORT_SYMBOL(WlanATSetWifiRX);
EXPORT_SYMBOL(WlanATGetWifiRX);
EXPORT_SYMBOL(WlanATSetWifiRPCKG);
EXPORT_SYMBOL(WlanATGetWifiRPCKG);
EXPORT_SYMBOL(WlanATGetWifiPlatform);
EXPORT_SYMBOL(WlanATSetTSELRF);
EXPORT_SYMBOL(WlanATGetTSELRFSupport);
EXPORT_SYMBOL(WlanATSetWifiParange);
EXPORT_SYMBOL(WlanATGetWifiParange);
EXPORT_SYMBOL(WlanATGetWifiParangeSupport);
EXPORT_SYMBOL(WlanATSetWifiCalTemp);
EXPORT_SYMBOL(WlanATGetWifiCalTemp);
EXPORT_SYMBOL(WlanATSetWifiCalData);
EXPORT_SYMBOL(WlanATSetWifiCal);
EXPORT_SYMBOL(WlanATGetWifiCal);
EXPORT_SYMBOL(WlanATGetWifiCalSupport);
EXPORT_SYMBOL(WlanATSetWifiCalPOW);
EXPORT_SYMBOL(WlanATGetWifiCalFreq);

