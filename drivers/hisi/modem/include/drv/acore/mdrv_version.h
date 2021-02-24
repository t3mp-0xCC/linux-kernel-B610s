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



/*memVersionCtrl�ӿڲ�������*/
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
	unsigned char *  productname;  /*оƬ���� */ /* BSP*/
	unsigned int   productnamelen;
	BSP_CHIP_TYPE_E echiptype;  /* PV500_CHIP V7R1_CHIP? */
	unsigned int  platforminfo ;     /* FPGA ASIC?*/
	BOARD_ACTUAL_TYPE_E eboardatype;   /*BBIT SFT ASIC FOR */
} MPRODUCT_INFO_S;

/* оƬModem ������汾��Ϣ*/
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
	int  hwindex;  /* HKADC��⵽��Ӳ��ID��(�弶��*/
	unsigned int  hwidsub;  /* HKADC��⵽���Ӱ汾��*/
	unsigned char *  hwfullver;  /* Ӳ���汾��Ϣ MBBʹ�� */
	unsigned int  hwfullverlen;
	unsigned char *  hwname;  /* �ⲿ��Ʒ����  MBBʹ�� CPE? */
	unsigned int  hwnamelen;
	unsigned char *  hwinname;  /* �ڲ���Ʒ����  MBBʹ��*/
	unsigned int  hwinnamelen;
} MHW_VER_INFO_S;

typedef struct  tagMODEM_VER_INFO_S
{
	MPRODUCT_INFO_S  stproductinfo;
	MSW_VER_INFO_S  stswverinfo;
	MHW_VER_INFO_S  sthwverinfo;
} MODEM_VER_INFO_S;

/*�������*/
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
    VER_HARDWARE_NO_SUB = 20,/*�����汾�����¼�20*/
    VER_INFO_NUM =21
}COMP_TYPE_E;
#endif /* MBB_COMMON */
/*****************************************************************************
 �� �� ��  : mdrv_ver_get_memberinfo
 ��������  : ��ѯ��������İ汾�š�
 �������  : ppVersionInfo��������İ汾��Ϣ��ַ��
 �������  : ucLength�������ص����ݵ��ֽ�����
 �� �� ֵ  : 0:  �����ɹ���
             -1������ʧ�ܡ�
*****************************************************************************/
int mdrv_ver_get_memberinfo(void ** ppVersionInfo, unsigned int * ulLength);




/*****************************************************************************
 �� �� ��  : mdrv_ver_get_info
 ��������  : ����modem�汾��Ϣ
****************************************************************************/
const MODEM_VER_INFO_S * mdrv_ver_get_info(void);
#if (FEATURE_ON == MBB_COMMON)
/*****************************************************************************
* �� �� ��  : bsp_version_get_baseline
*
* ��������  : ��ȡ���߰汾
*
* �������  : ��
* �������  :
*
 �� �� ֵ  : ���߰汾�ַ���
* �޸ļ�¼  :
*
*****************************************************************************/
extern char * bsp_version_get_baseline(void);

/*****************************************************************************
* �� �� ��   : mdrv_dload_get_productname
* ��������   : ��ȡ��Ʒ��������
* �������   : char *pProductName  : �ַ���������
*     unsigned int ulLength   : �ַ�������
* �������   : char *pProductName   : �ַ���������
* �� �� ֵ   : -1 -- ʧ��
                0 -- �ɹ�
* ����˵��   :  ATģ�����
*             ����ģʽ֧��
*             ����ģʽ֧��
*****************************************************************************/
extern int mdrv_dload_get_productname(char * pProductName, unsigned int ulLength);

/*****************************************************************************
* �� �� ��   : mdrv_dload_getisover
* ��������   : ��ȡ��̨�汾��
* �������   : char *pVersionInfo  : �ַ���������
*     unsigned int ulLength   : �ַ�������
* �������  : char *pVersionInfo   	: �ַ���������
* �� �� ֵ  : -1 -- ʧ��
                  0 -- �ɹ�
* ����˵��   :  ATģ�����
*             ����ģʽ֧��
*             ����ģʽ֧��
*****************************************************************************/
extern int mdrv_dload_getisover(char *pVersionInfo, unsigned int ulLength);

/*****************************************************************************
* �� �� ��   : mdrv_dload_getsoftver
* ��������   : ��ȡ����汾��
* �������  : BSP_S8 *str : �ַ���������
*     BSP_S32 len   : �ַ�������

* �������   : BSP_S8 *str   : �ַ���������
* �� �� ֵ   : -1 -- ʧ��
                       0 -- �ɹ�
* ����˵��   :  ATģ�����
*             ����ģʽ֧��
*             ����ģʽ֧��
*****************************************************************************/
extern int mdrv_dload_getsoftver(char *str, unsigned int len);

/*****************************************************************
* �� �� ��   : mdrv_dload_normal_reboot
* ��������  : ��Ʒ������,������^godload��^reset AT����
                      ����ʱ��������,�ӳ�2.5s����
* �������   : ��
* �������   : ��
* �� �� ֵ   : ��
* ����˵��  : ATģ�����
*              ����ģʽ֧��
*             ����ģʽ֧��
******************************************************************/
extern void mdrv_dload_normal_reboot(void);


extern char * bsp_version_get_build_date_time(void);

/*****************************************************************************
* �� �� ��  : BSP_HwGetHwVersion
* ��������  : ��ȡӲ���汾����
* �������  : char* pHwVersion,�ַ���ָ�룬��֤��С��32�ֽ�
* �������  : ��
* �� �� ֵ  : ��
* �޸ļ�¼  :
*****************************************************************************/
int BSP_HwGetHwVersion (char* pFullHwVersion, unsigned int ulLength);

/*****************************************************************************
 �� �� ��  : BSP_MNTN_GetProductIdInter
 ��������  : ��Ʒ���������汾���ӿڡ�
 �������  : pProductIdInter��Ϊ�����ߴ����淵�صĲ�Ʒ���������汾���ڴ��׵�ַ��
             ulLength       ��Ϊ�����ߴ����淵�صĲ�Ʒ���������汾���ڴ��С��
 �������  : �ޡ�
 �� �� ֵ  : 0:  �����ɹ���
             -1������ʧ�ܡ�
*****************************************************************************/
int	BSP_MNTN_GetProductIdInter(char * pProductIdInter, unsigned int ulLength);

/*****************************************************************
* �� �� ��  	: BSP_DLOAD_BaselineVer
* ��������  	: ��ѯ��˼���߰汾
* �������  	:

* �������  	:
* �� �� ֵ  	:

* ����˵��  : ATģ�����
*             
******************************************************************/
int BSP_DLOAD_BaselineVer(char *str, unsigned int len);
extern char * bsp_version_get_firmware(void);
#endif

#ifdef BSP_CONFIG_BOARD_SOLUTION
/*ģ���Ӳ�Ʒ��̬����ʹ��ö��*/
typedef enum{
    MDRV_VER_TYPE_M2M = 0,  /*ģ��m2m��Ʒ��̬*/
    MDRV_VER_TYPE_CE,  /*ģ��ce��Ʒ��̬*/
    MDRV_VER_TYPE_CAR,  /*ģ�鳵�ز�Ʒ��̬*/

    MDRV_VER_TYPE_INVALID  /*��Ч��Ʒ��̬*/
}MDRV_VER_SOLUTION_TYPE;

/*****************************************************************************
 �� �� ��  : mdrv_ver_get_solution_type
 ��������  : ����ӿ�,��ȡģ���Ӳ�Ʒ��̬(M2M/CE/����)
 �������  : void
 �������  : void
 �� �� ֵ  : ģ���Ӳ�Ʒ��̬
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

