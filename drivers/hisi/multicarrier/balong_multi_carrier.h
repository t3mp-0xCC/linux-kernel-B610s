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


#ifndef BALONG_MULTI_CARRIER
#define BALONG_MULTI_CARRIER

#if (MBB_MULTI_CARRIER == FEATURE_ON)
#include "mbb_multicarrier.h"
#define MTC_PER_CARRIER_MAX_PLMN 1
#define MTC_PER_IMAGE_MAX_CARRIER 50
#define MTC_IMAGE_MAX 5
#else
#define MTC_PER_CARRIER_MAX_PLMN 5
#define MTC_PER_IMAGE_MAX_CARRIER 5
#define MTC_IMAGE_MAX 5
#endif /* MBB_MULTI_CARRIER */

#define MTC_S_NAME_MAX 15

#define MTC_BUF_MAX_LENGTH 0x300

#define MTC_DEST_IMAGE_PATH 40
#define MTC_DEST_CARRIER_XML_PATH 40

#define MTC_L_NAME_MAX 25
#if (MBB_MULTI_CARRIER == FEATURE_ON)
#else
#define MTC_PLMN_MAX  8
#endif

#define MTC_IMAGE_VER_MAX  28
#define MTC_SOFT_VER_MAX  28


#define MTC_FILE_PATH_MAX  120

#if (MBB_MULTI_CARRIER == FEATURE_ON)
#define BALONG_MT_CARRIER_CUR_VER_FILE         "/online/mtc/mt_carrier_current_info"
#define BALONG_MT_CARRIER_UPDATA_STATUS_FILE   "/online/mtc/mtc_updata_status"
#define BALONG_MT_CARRIER_PLMN_CONFIG_FILE        "/online/mtc/Image_Xml_Config.xml"
#define MTC_BODYSAR_PLMN_LIST_NAME   ("/online/mtc/bodysar_plmn_list.bin")
#else
#define BALONG_MT_CARRIER_CUR_VER_FILE         "/mtc/mt_carrier_current_info"
#define BALONG_MT_CARRIER_UPDATA_STATUS_FILE   "/mtc/mtc_updata_status"
#define BALONG_MT_CARRIER_PLMN_CONFIG_FILE        "/mtc/Image_Xml_Config.xml"
#endif

#define BALONG_MT_CARRIER_MTD_PARTITION_NAME   "multicarrier"
#define BALONG_MT_CARRIER_MOUNT_POINT          "/mtc"

#if (MBB_MULTI_CARRIER == FEATURE_ON)
#else
typedef enum
{
    MTC_DEFAULT_SET,
    MTC_DEFAULT_RUN,
}MTC_DFT_OPTION;

typedef enum
{
    MTC_QUERY_DEFAULT,
    MTC_QUERY_RUNNING,
    MTC_QUERY_SUPPORT,
}MTC_QUERY_TYPE;
#endif

typedef enum
{
    MTC_GEN_ID = 0x00,
    MTC_VERIZON_ID = 0x01,
    MTC_SPRINT_ID = 0x02,
    MTC_ATT_ID = 0x03,
    MTC_VODA_ID = 0x04,
    MTC_DT_ID = 0x05,
    MTC_ORANGE_ID  = 0x06,
    MTC_TELEF_ID  = 0x07,
    MTC_TIM_ID = 0x08,
    MTC_DOCOMO_ID  = 0x09,
    MTC_CHT_ID  = 0x0A,
    MTC_CHM_ID  = 0x0B,
    MTC_CHU_ID  = 0x0C,
    MTC_KDDI_ID  = 0x0D,
    MTC_TELSTRA_ID  = 0x0E,
    MTC_US_TMO_ID =  0x0F,
    MTC_SWSCOM_ID  = 0x10,
    MTC_YOTA_ID  = 0x11,
    MTC_GEN_IPV4V6_ID  = 0x12,
    MTC_VDF_IPV4V6_ID  = 0x13,
    MTC_RES3_ID  = 0x14,
    MTC_SUPPORT_ID ,
    MTC_INVALID_ID = 0xFF,
}MTC_ID_TYPE;

typedef  struct
{
    /* plmn名称*/
    char        plmn[MTC_PLMN_MAX];
    /*plmn的短名字*/
    char        s_name[MTC_S_NAME_MAX];
    /*plmn的长名字*/
    char        l_name[MTC_L_NAME_MAX];
}mt_carrier_plmn_info;


typedef  struct
{
    /*运营商的ID MTC_GEN_ID = 0x00,MTC_VODA_ID = 0x04,*/
    int       mt_carrier_id;
    /*此mt_image_index号中的carrier index*/
    int       mt_carrier_index;
    /*xml版本信息*/
    int       mt_carrier_xml_version;
    /*xml版本信息*/
    int       mt_carrier_support_plmn_cnt;

    mt_carrier_plmn_info *mt_carrier_support_plmn_array[MTC_PER_CARRIER_MAX_PLMN];

}mt_carrier_info;

typedef  struct
{
    /*当前的运行的plmn*/
    char        mt_cur_running_plmn[MTC_PLMN_MAX];
    /*要切换到的plmn*/
    char        mt_dest_to_plmn[MTC_PLMN_MAX];
    /*要切换到的image路径名*/
    char        mt_dest_image_path[MTC_DEST_IMAGE_PATH];
    /*plmn的短名字*/
    char        mt_dest_carrier_xml_path[MTC_DEST_CARRIER_XML_PATH];
    /*表示镜像是否要切换*/
    int         mt_image_updata_status;
    /*表示次carrier_xml是否要切换*/
    int         mt_carrier_xml_updata_status;
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /* 默认运营商的plmn */
    char        mt_default_plmn[MTC_PLMN_MAX];
    /* 默认运营商的image路径 */
    char        mt_default_image_path[MTC_DEST_IMAGE_PATH];
    /* 默认运营商的xml路径 */
    char        mt_default_carrier_xml_path[MTC_DEST_CARRIER_XML_PATH];
    /* 通用运营商的plmn */
    char        mt_general_plmn[MTC_PLMN_MAX];
    /* 通用运营商的image路径 */
    char        mt_general_image_path[MTC_DEST_IMAGE_PATH];
    /* 通用运营商的xml路径 */
    char        mt_general_carrier_xml_path[MTC_DEST_CARRIER_XML_PATH];
    /* 待切换到的运营商，切换失败时使用(切换顺序为用户设定或sim卡触发的运营商->默认运营商->通用运营商) */
    mtc_dest_carrier_types mt_dest_carrier_type;
#endif /* MBB_MULTI_CARRIER */
}mt_carrier_updata_stutus;

typedef  struct
{
    /*此镜像编号*/
    int       mt_image_index;
    /*此image的版本信息*/
    char       mt_image_version[MTC_IMAGE_VER_MAX];
    /*此image中有多少个carrier*/
    int       mt_carrier_cnt;
    mt_carrier_info *mt_carrier_info_array[MTC_PER_IMAGE_MAX_CARRIER];
}mt_image_info;

typedef  struct
{
    /*此软件包的版本信息*/
    char       mt_software_version[MTC_SOFT_VER_MAX];
    /*此image中有多少个image*/
    int       mt_image_cnt;
    mt_image_info *mt_image_info_array[MTC_IMAGE_MAX];
}mt_info;

typedef  struct
{
    /*此image的序号*/
    int       mt_image_index;
    char      mt_image_version[MTC_IMAGE_VER_MAX];
    /*运营商的ID MTC_GEN_ID = 0x00,MTC_VODA_ID = 0x04,*/
    int       mt_carrier_id;
    /*此mt_image_index号中的carrier index*/
    int       mt_carrier_index;
    /*xml版本信息*/
    int       mt_carrier_xml_version;
    mt_carrier_plmn_info mt_carrier_plmn_info;
}mt_current_select_infor;

/**/
typedef  struct
{
    /* info of carrier*/
    mt_current_select_infor  carrier_info;
    /*info of default*/
    mt_current_select_infor  defalt_info;
#if (FEATURE_ON == MBB_MULTI_CARRIER)
    /* 通用运营商信息 */
    mt_current_select_infor  general_info;
#endif /* MBB_MULTI_CARRIER */
}mtc_cur_info;



#define MT_TRACE_WARNING		0x00000008
#define MT_TRACE_INFO		    0x00000010
#define MT_TRACE_ALWAYS		    0x00000020
extern unsigned int mt_carrier_trace_mask;
#define mt_carrier_trace(msk, fmt, ...) do { \
	if (mt_carrier_trace_mask & (msk)) \
		printk(fmt "\n", ##__VA_ARGS__); \
} while (0)

#define START_BOARD_SUPPORT_PLMN_LABEL  "<board_support_plmn>"
#define END_BOARD_SUPPORT_PLMN_LABEL    "</board_support_plmn>"
#define START_BOARD_SUPPORT_PLMN_NAME_LABEL  "<plmn_name>"
#define END_BOARD_SUPPORT_PLMN_NAME_LABEL    "</plmn_name>"
#define START_BOARD_SUPPORT_S_NAME_LABEL  "<s_name>"
#define END_BOARD_SUPPORT_S_NAME_LABEL    "</s_name>"
#define START_BOARD_SUPPORT_L_NAME_LABEL  "<l_name>"
#define END_BOARD_SUPPORT_L_NAME_LABEL    "</l_name>"
#define START_MT_CARRIER_INDEX_LABEL  "<mt_carrier_index index="
#define END_MT_CARRIER_INDEX_LABEL    "</mt_carrier_index>"
#define MT_CARRIER_XML_VERSION_LABEL    "<xml_version>"
#define START_MT_CARRIER_ID_LABEL    "<mt_carrier_id id="
#define START_MT_IMAGE_ID_LABEL    "<image_id id="
#define END_MT_IMAGE_ID_LABEL    "</image_id>"
#define START_MT_IMAGE_VERSION_LABEL    "<image_version>"
#define END_MT_IMAGE_VERSION_LABEL    "</image_version>"
#define START_MT_SOFTWARE_VERSION_LABEL    "<mt_software_version>"
#define END_MT_SOFTWARE_VERSION_LABEL    "</mt_software_version>"
#define END_MT_XML_CONFIG_LABEL    "</Image_Xml_Config>"


#if (MBB_MULTI_CARRIER == FEATURE_ON)
#define OEM_NAME_LEN_MAX 10
#define MAX_HARDWARE_LEN            (31)
#define MTC_FEATURE_VERSION    ("02")
typedef enum
{
    OEM_GEN_ID = 0x00,
    OEM_DELL_ID = 0x01,	
    OEM_SONY_ID = 0x02,
    OEM_HP_ID = 0x03,
    OEM_ACER_ID = 0x04,
    OEM_LENOVO_ID = 0x05,
    OEM_SAMSUNG_ID  = 0x06,
    OEM_INTEL_ID  = 0x07,
    OEM_TOSHIBA_ID  = 0x08,
    OEM_ASUS_ID = 0x09,	
    OEM_RESERVE_ID = 0x0A,
    OEM_RESERVE1_ID = 0x0B,
    OEM_RESERVE2_ID = 0x0C,
    OEM_RESERVE3_ID = 0x0D,
    OEM_RESERVE4_ID = 0x0E,
    OEM_RESERVE5_ID = 0x0F,
    OEM_RESERVE6_ID = 0x10,
    OEM_RESERVE7_ID = 0x11,
    OEM_RESERVE8_ID = 0x12,
    OEM_RESERVE9_ID = 0x13,
    OEM_SUPPORT_ID ,
    OEM_INVALID_ID = 0xFF,
}pc_vendor_oem_type;

typedef void (*mtc_ver_get_func)(char *version, unsigned char  size);
typedef void (*mtc_op_func)(void);
/*****************************************************************************
 函 数 名  : huawei_mtc_get_plmn_from_oem
 功能描述  : 从 OMEINFO中获取plmn
 输入参数  : 
 输出参数  : 无
 返 回 值  : TRUE:获取成功
             FALSE:获取失败
*****************************************************************************/
bool huawei_mtc_get_plmn_from_oem(char *buffer,unsigned int size);
/*****************************************************************************
 函 数 名  : huawei_mtc_set_plmn_from_oem
 功能描述  : 向 OMEINFO中写入plmn
 输入参数  : 
 输出参数  : 无
 返 回 值  : TRUE:更新成功
             FALSE:更新失败
*****************************************************************************/
bool huawei_mtc_set_plmn_from_oem(char *buffer, unsigned int buf_len);
/*****************************************************************************
 函 数 名  : huawei_mtc_get_package_version
 功能描述  : 获取整包版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:输入数据区的大小
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void huawei_mtc_get_package_version(char *version, unsigned char  size);
/*****************************************************************************
 函 数 名  : huawei_mtc_get_oem_version
 功能描述  : 获取OEM版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:数据空间大小
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void huawei_mtc_get_oem_version(char *version,unsigned char  size);
/*****************************************************************************
 函 数 名  : huawei_mtc_get_image_version
 功能描述  : 获取IMAGE版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:数据空间大小
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void huawei_mtc_get_image_version(char *version,unsigned char  size);
/*****************************************************************************
 函 数 名  : huawei_mtc_get_image_version
 功能描述  : 获取MTC的feature版本号接口
 输入参数  : char *version  用来存储版本号的地址,size:数据空间大小
 输出参数  : 无
 返 回 值  : 无
*****************************************************************************/
void huawei_mtc_get_feature_version(char *version,unsigned char  size);
/*****************************************************************************
 函 数 名  : huawei_mtc_sim_switch_cb
 功能描述  : SIM卡初始化后向A核发icc消息的回调函数
 输入参数  : NULL
 输出参数  : 无
 返 回 值  : 0:成功  -1:失败
*****************************************************************************/
signed int huawei_mtc_sim_switch_cb(void);
/*****************************************************************************
 函 数 名  : huawei_mtc_auto_switch_entry
 功能描述  : SIM卡自动切换总入口
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :
*****************************************************************************/
void huawei_mtc_auto_switch_entry(void);
/*****************************************************************************
 函 数 名  : huawei_mtc_free_mem
 功能描述  : 用来释放申请的内存
 输入参数  : 无
 输出参数  : 无
 返 回 值  :无
*****************************************************************************/
void huawei_mtc_free_mem(void);

#endif /* MBB_MULTI_CARRIER */

#endif
