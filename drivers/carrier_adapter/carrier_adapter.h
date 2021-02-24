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



#ifndef _CARRIER_ADAPTER_H_
#define _CARRIER_ADAPTER_H_

#include "product_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PLMN_SIZE_MAX               (8)
#define CA_CUST_PATH_MAX_LENGTH     (64)
#define RESERVE_SIZE                (3)
#define MAXPLMN_ARRAY               (64)
#define CA_COMMON_PATH_MAX_LENGTH   (64)
#define OTHERS_CARRIER_NAME         (999)

/* 与TAF_PLMN_ID_STRU保持一致 */
typedef struct
{
    unsigned int    Mcc;
    unsigned int    Mnc;
} TAF_PLMN_ID_STRU;

typedef struct
{
    TAF_PLMN_ID_STRU    stHplmn;
    unsigned char       ucHplmnMncLen;
    unsigned char       aucReserved[RESERVE_SIZE];
} CA_HPLMN_WITH_MNC_LEN_STRU;

/* 配置文件的xml路径 */
#define CONFIG_FILE_PATH      "/system/ca_cust_nv/ca_config_file.xml"

/* PLMN和运营商定制NV XML文件对应列表数据结构 */
typedef struct
{
    int plmn_name;
    int carrier_name;
    char plmn_str[PLMN_SIZE_MAX];
    char cust_nv_path[CA_CUST_PATH_MAX_LENGTH];
} PLMN_NV_MATCH_INFO_STRU;

/* OEMINFO分区的PLMN，分别存表示正常分区和bk分区的PLMN */
typedef struct
{
    char plmn[PLMN_SIZE_MAX];
    char plmn_bk[PLMN_SIZE_MAX];
} CA_OEM_INFO_STRU;

/******************* for debug log output ********************/
#ifdef MBB_BUILD_DEBUG

#define LOG_ERR     "[ERR]: "
#define LOG_WARN    "[WARN]: "
#define LOG_INFO    "[INFO]: "

#define carrier_adapter_log(lvl, fmt, ...) \
    printk("<CARRIER_ADAPTER>" lvl fmt "\n", ##__VA_ARGS__)

#else
#define carrier_adapter_log(lvl, fmt, ...)
#endif

/* 局部使用宏定义，消除魔鬼数字 */
#define CA_MODULE_NAME          "carrier_adapter"
#define CA_TSK_NAME             "carrier_adapter_task"

#define NV_SWITCH_WAIT_TIME     ((long)msecs_to_jiffies(5000))

#define FIRST_BIT_MASK          (0x0000000f)
#define SECOND_BIT_MASK         (0x00000f00)
#define THIRD_BIT_MASK          (0x000f0000)

#define FIRST_BIT_SHIFT         (0)
#define SECOND_BIT_SHIFT        (8)
#define THIRD_BIT_SHIFT         (16)

typedef struct
{
    unsigned int product_id;
    unsigned int plmn_cnt;
    char common_nv_path[CA_COMMON_PATH_MAX_LENGTH];
    PLMN_NV_MATCH_INFO_STRU ca_support_sim_info[MAXPLMN_ARRAY];
} PRODUCT_CONFIG_INFO_STRU;

#define START_PRODUCT_ID "<PRODUCT_ID="
#define END_PRODUCT_ID "</PRODUCT_ID>"
#define START_PLMN_ID "<PLMN_ID="
#define END_PLMN_ID "</PLMN_ID>"
#define START_CARRIER_NAME "<CARRIER_NAME>"
#define START_PLMN_STR "<PLMN_STR>"
#define END_PLMN_STR "</PLMN_STR>"
#define START_NV_PATH "<NV_PATH>"
#define END_NV_PATH "</NV_PATH>"
#define START_PLMN_CNT "<PLMN_CNT>"
#define START_COMMON_NV_PATH "<COMMON_NV_PATH>"
#define END_COMMON_NV_PATH "</COMMON_NV_PATH>"

/* 对NV数据nv.bin生成CRC校验码 */
extern unsigned int nv_make_ddr_crc(void);
 /*将最新数据写入各个分区*/
extern unsigned int nv_data_writeback(void);
/*重启接口函数*/
extern void bsp_drv_power_reboot(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif /* _CARRIER_ADAPTER_H_ */

