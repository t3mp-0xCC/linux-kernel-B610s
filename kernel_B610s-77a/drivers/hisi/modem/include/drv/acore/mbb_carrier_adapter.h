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



#ifndef __MBB_CARRIER_ADAPTER_H__
#define __MBB_CARRIER_ADAPTER_H__

#include "product_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define PLMN_SIZE_MAX               (8)
#define MAXPLMN_ARRAY               (64)
#define CA_CUST_PATH_MAX_LENGTH     (64)
#define CA_COMMON_PATH_MAX_LENGTH   (64)

/* 配置文件的xml路径 */
#define CONFIG_FILE_PATH      "/system/ca_cust_nv/ca_config_file.xml"

/* OEMINFO分区的PLMN，分别存表示正常分区和bk分区的PLMN */
typedef struct
{
    char plmn[PLMN_SIZE_MAX];
    char plmn_bk[PLMN_SIZE_MAX];
} CA_OEM_INFO_STRU;

/* PLMN和运营商定制NV XML文件对应列表数据结构 */
typedef struct
{
    int plmn_name;
    int carrier_name;
    char plmn_str[PLMN_SIZE_MAX];
    char cust_nv_path[CA_CUST_PATH_MAX_LENGTH];
} PLMN_NV_MATCH_INFO_STRU;

typedef struct
{
    unsigned int product_id;
    unsigned int plmn_cnt;
    char common_nv_path[CA_COMMON_PATH_MAX_LENGTH];
    PLMN_NV_MATCH_INFO_STRU ca_support_sim_info[MAXPLMN_ARRAY];
} PRODUCT_CONFIG_INFO_STRU;

extern PRODUCT_CONFIG_INFO_STRU g_product_config_info;
extern int plmn_get_carrier_name(int *out, const char *plmn);
extern int decode_carrier_config_file(char *src_path);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif /* __MBB_CARRIER_ADAPTER_H__ */

