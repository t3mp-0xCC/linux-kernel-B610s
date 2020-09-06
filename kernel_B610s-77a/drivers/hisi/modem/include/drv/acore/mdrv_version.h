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



#ifndef _MDRV_ACORE_VERSION_H_
#define _MDRV_ACORE_VERSION_H_

#ifdef _cplusplus
extern "C"
{
#endif


#include "product_config.h"

#define BUILD_DATE_LEN  12
#define BUILD_TIME_LEN  12



/*memVersionCtrl接口操作类型*/
#define VERIONREADMODE                  0
#define VERIONWRITEMODE                1



typedef enum{
	 BOARD_TYPE_BBIT    = 0,
	 BOARD_TYPE_SFT,
	 BOARD_TYPE_ASIC,
	 BOARD_TYPE_MAX
}BOARD_ACTUAL_TYPE_E;

typedef enum{
     PV500_CHIP             = 0,
     V7R1_CHIP              = 1,
     PV500_PILOT_CHIP,
     V7R1_PILOT_CHIP,
     V7R2_CHIP              = 5,
     V8R1_PILOT_CHIP        = 0x8,
}BSP_CHIP_TYPE_E;

typedef struct  tagMPRODUCT_INFO_S
{
	unsigned char *  productname;  /*芯片名称 */ /* BSP*/
	unsigned int   productnamelen;
	BSP_CHIP_TYPE_E echiptype;  /* PV500_CHIP V7R1_CHIP? */
	unsigned int  platforminfo ;     /* FPGA ASIC?*/
	BOARD_ACTUAL_TYPE_E eboardatype;   /*BBIT SFT ASIC FOR */
} MPRODUCT_INFO_S;

/* 芯片Modem 的软件版本信息*/
typedef  struct  tagMSW_VER_INFO_S
{
	unsigned short  ulVVerNO;
	unsigned short  ulRVerNO;
	unsigned short  ulCVerNO;
	unsigned short  ulBVerNO;
	unsigned short  ulSpcNO;
	unsigned short  ulCustomNOv;
	unsigned int    ulProductNo;
	unsigned char  acBuildDate[BUILD_DATE_LEN];
	unsigned char  acBuildTime[BUILD_TIME_LEN];
} MSW_VER_INFO_S;

typedef  struct  tagMHW_VER_INFO_S
{
	int  hwindex;  /* HKADC检测到的硬件ID号(板级）*/
	unsigned int  hwidsub;  /* HKADC检测到的子版本号*/
	unsigned char *  hwfullver;  /* 硬件版本信息 MBB使用 */
	unsigned int  hwfullverlen;
	unsigned char *  hwname;  /* 外部产品名称  MBB使用 CPE? */
	unsigned int  hwnamelen;
	unsigned char *  hwinname;  /* 内部产品名称  MBB使用*/
	unsigned int  hwinnamelen;
} MHW_VER_INFO_S;

typedef struct  tagMODEM_VER_INFO_S
{
	MPRODUCT_INFO_S  stproductinfo;
	MSW_VER_INFO_S  stswverinfo;
	MHW_VER_INFO_S  sthwverinfo;
} MODEM_VER_INFO_S;

/*组件类型*/
#if (FEATURE_OFF == MBB_COMMON)

typedef enum
{
	VER_BOOTLOAD = 0,
	VER_BOOTROM = 1,
	VER_NV = 2,
	VER_VXWORKS = 3,
	VER_DSP = 4,
	VER_PRODUCT_ID = 5,
	VER_WBBP = 6,
	VER_PS = 7,
	VER_OAM = 8,
	VER_GBBP = 9,
	VER_SOC = 10,
	VER_HARDWARE = 11,
	VER_SOFTWARE = 12,
	VER_MEDIA = 13,
	VER_APP = 14,
	VER_ASIC = 15,
	VER_RF = 16,
	VER_PMU = 17,
	VER_PDM = 18,
	VER_PRODUCT_INNER_ID = 19,
	VER_INFO_NUM = 20
} COMP_TYPE_E;

#else
typedef enum
{
    VER_BOOTLOAD = 0,
    VER_BOOTROM =1,
    VER_NV =2 ,
    VER_VXWORKS =3,
    VER_DSP =4 ,
    VER_PRODUCT_ID =5 ,
    VER_WBBP =6 ,
    VER_PS =7,
    VER_OAM =8,
    VER_GBBP =9 ,
    VER_SOC =10,
    VER_HARDWARE =11,
    VER_SOFTWARE =12,
    VER_MEDIA =13,
    VER_APP =14,
    VER_ASIC =15,
    VER_RF =16,
    VER_PMU =17,
    VER_PDM = 18,
    VER_PRODUCT_INNER_ID = 19,
    VER_HARDWARE_NO_SUB = 20,/*新增版本号子事件20*/
    VER_INFO_NUM =21
}COMP_TYPE_E;
#endif /* MBB_COMMON */
/*****************************************************************************
 函 数 名  : mdrv_ver_get_memberinfo
 功能描述  : 查询所有组件的版本号。
 输入参数  : ppVersionInfo：待保存的版本信息地址。
 输出参数  : ucLength：待返回的数据的字节数。
 返 回 值  : 0:  操作成功；
             -1：操作失败。
*****************************************************************************/
int mdrv_ver_get_memberinfo(void ** ppVersionInfo, unsigned int * ulLength);




/*****************************************************************************
 函 数 名  : mdrv_ver_get_info
 功能描述  : 返回modem版本信息
****************************************************************************/
const MODEM_VER_INFO_S * mdrv_ver_get_info(void);
#if (FEATURE_ON == MBB_COMMON)
/*****************************************************************************
* 函 数 名  : bsp_version_get_baseline
*
* 功能描述  : 获取基线版本
*
* 输入参数  : 无
* 输出参数  :
*
 返 回 值  : 基线版本字符串
* 修改记录  :
*
*****************************************************************************/
extern char * bsp_version_get_baseline(void);

/*****************************************************************************
* 函 数 名   : mdrv_dload_get_productname
* 功能描述   : 获取产品对外名称
* 输入参数   : char *pProductName  : 字符串缓冲区
*     unsigned int ulLength   : 字符串长度
* 输出参数   : char *pProductName   : 字符串缓冲区
* 返 回 值   : -1 -- 失败
                0 -- 成功
* 其它说明   :  AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
extern int mdrv_dload_get_productname(char * pProductName, unsigned int ulLength);

/*****************************************************************************
* 函 数 名   : mdrv_dload_getisover
* 功能描述   : 获取后台版本号
* 输入参数   : char *pVersionInfo  : 字符串缓冲区
*     unsigned int ulLength   : 字符串长度
* 输出参数  : char *pVersionInfo   	: 字符串缓冲区
* 返 回 值  : -1 -- 失败
                  0 -- 成功
* 其它说明   :  AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
extern int mdrv_dload_getisover(char *pVersionInfo, unsigned int ulLength);

/*****************************************************************************
* 函 数 名   : mdrv_dload_getsoftver
* 功能描述   : 获取软件版本号
* 输入参数  : BSP_S8 *str : 字符串缓冲区
*     BSP_S32 len   : 字符串长度

* 输出参数   : BSP_S8 *str   : 字符串缓冲区
* 返 回 值   : -1 -- 失败
                       0 -- 成功
* 其它说明   :  AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
extern int mdrv_dload_getsoftver(char *str, unsigned int len);

/*****************************************************************
* 函 数 名   : mdrv_dload_normal_reboot
* 功能描述  : 产品线新增,用来在^godload和^reset AT命令
                      处理时重启单板,延迟2.5s重启
* 输入参数   : 无
* 输出参数   : 无
* 返 回 值   : 无
* 其它说明  : AT模块调用
*              正常模式支持
*             下载模式支持
******************************************************************/
extern void mdrv_dload_normal_reboot(void);


extern char * bsp_version_get_build_date_time(void);

/*****************************************************************************
* 函 数 名  : BSP_HwGetHwVersion
* 功能描述  : 获取硬件版本名称
* 输入参数  : char* pHwVersion,字符串指针，保证不小于32字节
* 输出参数  : 无
* 返 回 值  : 无
* 修改记录  :
*****************************************************************************/
int BSP_HwGetHwVersion (char* pFullHwVersion, unsigned int ulLength);

/*****************************************************************************
 函 数 名  : BSP_MNTN_GetProductIdInter
 功能描述  : 产品名称完整版本读接口。
 输入参数  : pProductIdInter：为调用者待保存返回的产品名称完整版本的内存首地址；
             ulLength       ：为调用者待保存返回的产品名称完整版本的内存大小；
 输出参数  : 无。
 返 回 值  : 0:  操作成功；
             -1：操作失败。
*****************************************************************************/
int	BSP_MNTN_GetProductIdInter(char * pProductIdInter, unsigned int ulLength);

/*****************************************************************
* 函 数 名  	: BSP_DLOAD_BaselineVer
* 功能描述  	: 查询海思基线版本
* 输入参数  	:

* 输出参数  	:
* 返 回 值  	:

* 其它说明  : AT模块调用
*             
******************************************************************/
int BSP_DLOAD_BaselineVer(char *str, unsigned int len);
extern char * bsp_version_get_firmware(void);
#endif

#ifdef BSP_CONFIG_BOARD_SOLUTION
/*模块子产品形态对外使用枚举*/
typedef enum{
    MDRV_VER_TYPE_M2M = 0,  /*模块m2m产品形态*/
    MDRV_VER_TYPE_CE,  /*模块ce产品形态*/
    MDRV_VER_TYPE_CAR,  /*模块车载产品形态*/

    MDRV_VER_TYPE_INVALID  /*无效产品形态*/
}MDRV_VER_SOLUTION_TYPE;

/*****************************************************************************
 函 数 名  : mdrv_ver_get_solution_type
 功能描述  : 对外接口,获取模块子产品形态(M2M/CE/车载)
 输入参数  : void
 输出参数  : void
 返 回 值  : 模块子产品形态
****************************************************************************/
MDRV_VER_SOLUTION_TYPE mdrv_ver_get_solution_type(void);
#endif
#if (FEATURE_ON == MBB_DLOAD)
extern signed int huawei_set_upinfo(char* pData);
extern void huawei_get_upinfo_times(unsigned int* times);
extern signed int huawei_get_spec_upinfo(char* pData, unsigned int pDataLen, unsigned int num);
extern int mdrv_dload_set_datalock_state(void);
#endif



#ifdef _cplusplus
}
#endif
#endif

