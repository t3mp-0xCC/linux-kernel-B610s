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


#ifndef __BSP_VERSION_H__
#define __BSP_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__OS_VXWORKS__) || defined(__OS_RTOSCK__) || defined(__KERNEL__))
#include <mdrv_version.h>
#endif
#include "product_config.h"
#include <osl_types.h>
#include <drv_nv_def.h>
#include <bsp_om.h>
#include <bsp_shared_ddr.h>

#ifndef VER_ERROR
#define VER_ERROR (-1)
#endif

#ifndef VER_OK
#define VER_OK 0
#endif

#define  ver_print_info(fmt, ...)  (bsp_trace(BSP_LOG_LEVEL_INFO,  BSP_MODU_VERSION, "[version]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))
#define  ver_print_error(fmt, ...) (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_VERSION, "[version]: <%s> "fmt, __FUNCTION__, ##__VA_ARGS__))

#define HW_VER_INVALID                          (BSP_U32)0XFFFFFFFF
#define HW_VER_UDP_MASK				(BSP_U32)0XFF000000 	/*UDP单板掩码*/
#define HW_VER_UDP_UNMASK			(BSP_U32)(~HW_VER_UDP_MASK) /*UDP掩码取反*/

#define HW_VER_V711_UDP				(BSP_U32)0X71000000 /*V711 UDP*/
#define HW_VER_V750_UDP				(BSP_U32)0X75000000 /*V7R5 UDP*/
#define HW_VER_V722_UDP				(BSP_U32)0X72000000 /*V722 UDP*/
#define HW_VER_K3V5_UDP				(BSP_U32)0X35000000 /*K3V5 UDP*/
#define HW_VER_PXXX                                 (BSP_U32)0XFF000000 /*P532*/

/* CPE的硬件版本号定义 */
#define HW_VER_PRODUCT_CPE          (BSP_U32)0X00040000
#define HW_VER_PRODUCT_B610s_76a    (BSP_U32)0X00040000
#define HW_VER_PRODUCT_B618s_22d    (BSP_U32)0X00040001
#define HW_VER_PRODUCT_B610s_77a    (BSP_U32)0X00040002
#define HW_VER_PRODUCT_B618s_65d    (BSP_U32)0X00040003
#define HW_VER_PRODUCT_KD11         (BSP_U32)0X00040006
#define HW_VER_PRODUCT_KD15         (BSP_U32)0X00040021
#define HW_VER_PRODUCT_B612s_51d    (BSP_U32)0X00040009
#define HW_VER_PRODUCT_B715s_23c    (BSP_U32)0X00040012
#define HW_VER_PRODUCT_B612s_25d    (BSP_U32)0X00040015
#define HW_VER_PRODUCT_B610s_79a    (BSP_U32)0X00040018

/* E5产品硬件版本号定义 */
#define HW_VER_PRODUCT_DC04         (BSP_U32)0X00000001
#define HW_VER_PRODUCT_SB03         (BSP_U32)0X00000003

typedef enum
{
	CHIP_P531 = 0x0530,
	CHIP_P532 = 0x0532,
	CHIP_K3V3 = 0x3630,
	CHIP_K3V5 = 0x3650,
	CHIP_K3V6 = 0x3660,
	CHIP_V8R5 = 0x6250,
	CHIP_V711 = 0x6921,
	CHIP_V722 = 0x6932,
	CHIP_V750 = 0x6950
}VERSION_CHIP_TYPE_E;

typedef enum
{
	PLAT_ASIC= 0x0,
	PLAT_FPGA = 0xa,
	PLAT_EMU = 0xe
}VERSION_PLAT_TYPE_E;

typedef enum{
	 BSP_BOARD_TYPE_BBIT    = 0,
	 BSP_BOARD_TYPE_SFT,
	 BSP_BOARD_TYPE_ASIC,
	 BSP_BOARD_TYPE_SOC,
	 BSP_BOARD_TYPE_MAX
}VERSION_BOARD_TYPE_E;

typedef enum
{
	DALLAS_BBIT = 0x1,
	V722_BBIT = 0x2,
	CHICAGO_BBIT = 0x3
}VERSION_BBIT_TYPE_E;

typedef enum
{
	PRODUCT_MBB= 0x0,
	PRODUCT_PHONE = 0x1,
	PRODUCT_ERROR = 0x2
}VERSION_PRODUCT_TYPE_E;

typedef enum
{
	PRODUCT_AUSTIN = 0x3650,
	PRODUCT_CHICAGO = 0x3660,
	PRODUCT_DALLAS = 0x6250,
	PRODUCT_722 = 0x6932,
	PRODUCT_750 = 0x6950,
	PRODUCT_NOT_SUPPORT = 0xFFFF
}VERSION_PRODUCT_NAME_E;

typedef struct
{
	u32 board_id;                    /*硬件版本号，通过hkadc读取。NV和dts在用*/
	u32 board_id_udp_masked;         /*屏蔽扣板信息的硬件版本号。ioshare在用*/
    u32 chip_version;                /*芯片版本号*/
    u16 chip_type;                   /*芯片类型，如CHIP_V711=0x6921*/
	u8  plat_type;                   /*平台类型，如asic/proting/emu*/
	u8  board_type;                  /*平台类型，如BBIT SOC ASIC SFT*/
	u8  bbit_type;                   /*bbit 平台，如dalass bbit/722 BBIT/chicago bbit*/
	u8  product_type;                /*产品类型，如MBB/PHONE*/
    u16 product_name;                /*产品名称，如PRODUCT_722，将722 porting/bbit/sft/udp统一归类为722*/
/*在版本号信息结构中添加一项，用于表示当前单板从HKADC中读取的真实
  硬件子版本号： NV通过board_id无法同时支持多个硬件子版本。*/
#if (FEATURE_ON == MBB_COMMON)
    u8  real_hwIdSub;
#endif
    u32 reserved;
}BSP_VERSION_INFO_S;
#define VERSION_ISO_MAX_LEN  128
#define VERSION_WEBUI_MAX_LEN  128



/*****************************************************************************
*                                                                                                                               *
*            以下提供给version_balong.c(a/c)                                                          *
*                                                                                                                               *
******************************************************************************/

#if defined(__OS_VXWORKS__) || defined(__OS_RTOSCK__)
#define StrParamType (unsigned int)
#define MemparamType (int)
#else
#define StrParamType (int)
#define MemparamType (unsigned int)
#endif

#define DLOAD_VERSION      "2.0"                   /* 下载协议版本 */
#define VERSION_MAX_LEN 32

/*C31B030基线中删除此宏，但C核adp_version.c中增加的代码需要。*/
#if (FEATURE_ON == MBB_COMMON)
#define HW_VER_PRODUCT_TYPE_MASK    (BSP_U32)0x00FF0000
#define HW_VER_PRODUCT_TYPE_OS      16
#endif

/*MBB的产品，通常有Ver.A/B/C 3个硬件子版本。*/
#if (FEATURE_ON == MBB_COMMON)
#define HW_MBB_SUB_VER_NR  (3)
#endif

#ifdef BSP_CONFIG_BOARD_SOLUTION
/*产品形态(包括产品子形态)枚举,共享内存存放值*/
typedef enum{
    PRODUCT_TYPE_M2M = 0xFAAFFA01,  /*模块m2m产品形态*/
    PRODUCT_TYPE_CE = 0xFAAFFA02,  /*模块ce产品形态*/
    PRODUCT_TYPE_CAR = 0xFAAFFA03,  /*模块车载产品形态*/

    PRODUCT_TYPE_INVALID = 0xFAAFFAAF  /*无效产品形态魔数*/
}SOLUTION_PRODUCT_TYPE;
/*产品形态与硬件ID对照表结构体*/
typedef struct _SOLUTION_PRODUCT_INFO_T{
    SOLUTION_PRODUCT_TYPE product_type;
    unsigned int product_id;
}SOLUTION_PRODUCT_INFO_T;
#endif

/*****************************************************************************
*                                                                                                                               *
*            以下提供给version.c(fastboot)                                                       *
*                                                                                                                               *
******************************************************************************/

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/(sizeof((a)[0])))
#endif

#if (FEATURE_ON == MBB_COMMON)
/*****************************************************************************
* 函 数 名   : bsp_version_get_hardware_no_subver
*
* 功能描述   : 获取硬件版本号
*
* 输入参数   : 无
* 输出参数   : 硬件版本号字符串指针
*
* 返 回 值   : 0获取成功
             -1获取失败
*
* 修改记录  :
*
*****************************************************************************/
char * bsp_version_get_hardware_no_subver(void);
#endif

typedef struct
{
    u16 vol_low;
    u16 vol_high;
}voltage_range;



/*****************************************************************************
*                                                                                                                               *
*            以下提供给adp_version.c(a/c)                                                              *
*                                                                                                                               *
******************************************************************************/

#ifndef isdigit
#define isdigit(c)      (((c) >= '0') && ((c) <= '9'))
#endif

#define CHIP_TYPE_MASK 0xffff0000
#define PLAT_TYPE_MASK 0x0000f000

#define MAX_VER_SECTION 8
#define VER_PART_LEN 3
#define VERC_PART_LEN 2

#ifndef VER_MAX_LENGTH
#define VER_MAX_LENGTH                  30
#endif

#ifdef BSP_CONFIG_BOARD_SOLUTION
/*****************************************************************************
* 函数  : bsp_get_solution_type
* 功能  : 获取模块子产品形态(独立对外接口)
      fastboot中该接口通过遍历对照表查找
      A/C/M各内核中使用时直接读取共享内存
* 输入  : void
* 输出  : void
* 返回  : MBB_PRODUCT_TYPE
*****************************************************************************/
SOLUTION_PRODUCT_TYPE bsp_get_solution_type(void);
#endif

#if (FEATURE_ON == MBB_COMMON)
/*****************************************************************************
* 函数  : mbb_version_get_board_type
* 功能  : 产品线获取产品硬件ID使用接口
                     产品线新增代码中不能使用上面的海思接口
* 输入  : void
* 输出  : void
* 返回  : BOARD_TYPE_E
*****************************************************************************/
u32 mbb_version_get_board_type(void);

#if (FEATURE_ON == MBB_DLOAD)
/*****************************************************************************
* 函 数 名  : bsp_get_web_version
*
* 功能描述  : 获取webui_version
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : webui_version字符串的指针
* 修改记录  :
*
*****************************************************************************/
void bsp_get_web_version(char *str,int len);


void bsp_get_iso_version(char *str, int len);
#endif

#endif

typedef struct
{
    unsigned char CompId;                                           /* 组件号：参见COMP_TYPE */
    unsigned char CompVer[VER_MAX_LENGTH+1];         /* 最大版本长度 30 字符+ \0 */
}VERSIONINFO;



/*****************************************************************************
*                                                                                                                               *
*            以下提供给virtual boardid功能                                                          *
*                                                                                                                               *
******************************************************************************/

#define VIRTUAL_BOARDID_SET_OK      0x12345000
#define VIRTUAL_BOARDID_NO_SET      0x12345001
#define VIRTUAL_BOARDID_CMD_NULL    0x12345002
#define VIRTUAL_BOARDID_ERR_FORMAT  0x12345003
#define VIRTUAL_BOARDID_NV_NOBURN   0x12345004
#define VIRTUAL_BOARDID_SET_FLAG    0x12345005

#define MISC_VERSION_OFFSET 100

typedef struct  {
    unsigned err_code;
    unsigned virtual_boardid;
    unsigned timestamp;
    unsigned set_ok_flag;
} misc_ptn_version_info;

typedef enum {
	VIRTUAL_BOARDID_MISC_OK = 0,
	VIRTUAL_BOARDID_MISC_ERROR,
}virtual_boardid_misc_return_type;



/*****************************************************************************
*                                                                                                                               *
*            以下为对外头文件声明                                                              *
*                                                                                                                               *
******************************************************************************/

char * bsp_version_get_hardware(void);
char * bsp_version_get_product_inner_name(void);
char * bsp_version_get_product_out_name(void);

char * bsp_version_get_build_date_time(void);
char * bsp_version_get_chip(void);
char * bsp_version_get_firmware(void);
char * bsp_version_get_release(void);

int bsp_version_acore_init(void);
int bsp_version_ccore_init(void);
void bsp_version_ddr_init(void);
int bsp_version_init(void);
void mdrv_ver_init(void);

VERSION_CHIP_TYPE_E bsp_version_get_chip_type(void);
const BSP_VERSION_INFO_S* bsp_get_version_info(void);

int bsp_version_debug(void);

void update_version_boardid(void);
void set_virtual_boardid(char *virtual_boardid);
void show_virtual_boardid(void);
void clear_virtual_boardid(void);


#if (FEATURE_ON == MBB_DLOAD)
/*****************************************************************************
* 函数	: huawei_set_update_info
* 功能	: set update info 
* 输入	: void
* 输出	: void
* 返回	: void
*
* 其它       : 无
*
*****************************************************************************/
void huawei_set_update_info(char* str);

/*****************************************************************************
* 函数	: huawei_get_update_info_times
* 功能	: get update info times
* 输入	: NA 
* 输出	: times: num of upgrade 
* 返回	: void 
*
* 其它       : 无
*
*****************************************************************************/
void huawei_get_update_info_times(s32* times);

/*****************************************************************************
* 函数	: huawei_get_spec_num_upinfo
* 功能	: get info of once update 
* 输入	: num : index of upgrade
* 输出	: void
* 返回	: 0获取成功/-1获取失败
*
* 其它       : 无
*
*****************************************************************************/
s32 huawei_get_spec_num_upinfo(char* str, s32 str_len, s32 num);
#endif
#ifdef __cplusplus
}
#endif

#endif

