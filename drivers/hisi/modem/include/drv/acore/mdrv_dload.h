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



#ifndef _MDRV_DLOAD_H_
#define _MDRV_DLOAD_H_

#ifdef _cplusplus
extern "C"
{
#endif


/* ����ģʽö�� */
typedef enum tagDLOAD_MODE_E
{
    DLOAD_MODE_DOWNLOAD = 0,
    DLOAD_MODE_NORMAL,
    DLOAD_MODE_DATA,
    DLOAD_MODE_MAX
}DLOAD_MODE_E;

typedef  struct  tagDLOAD_VER_INFO_S
{
        unsigned char*  productname;
        unsigned int  productnamelen;
        unsigned char*  softwarever;
        unsigned int  softwareverlen;
        unsigned char*  isover;
        unsigned int  isoverlen;
        unsigned char*  dloadver;
        unsigned int  dloadverlen;
} DLOAD_VER_INFO_S;


/*****************************************************************************
* �� �� ��  : mdrv_dload_set_curmode
*
* ��������  : ���õ�ǰ����ģʽ
*
* �������  : void
* �������  : DLOAD_MODE_NORMAL     :����ģʽ
*             DLOAD_MODE_DATA       :����ģʽ
*             DLOAD_MODE_DOWNLOAD   :����ģʽ
*
* �� �� ֵ  : ��
*
* ����˵��  : ATģ�����
*             ����ģʽ֧�ֵ���:
*                 BSP_DLOAD_GetCurMode(DLOAD_MODE_DOWNLOAD)
*             ����ģʽ֧�ֵ���:
*                 BSP_DLOAD_GetCurMode(DLOAD_MODE_DATA)
*
*****************************************************************************/
void mdrv_dload_set_curmode(DLOAD_MODE_E eDloadMode);
/*****************************************************************************
* �� �� ��  : BSP_DLOAD_GetCurMode
*
* ��������  : ��ȡ��ǰ����ģʽ
*
* �������  : void
* �������  : ��
*
* �� �� ֵ  : DLOAD_MODE_NORMAL     :����ģʽ
*             DLOAD_MODE_DATA       :����ģʽ
*             DLOAD_MODE_DOWNLOAD   :����ģʽ
*
* ����˵��  : ��
*
*****************************************************************************/
DLOAD_MODE_E mdrv_dload_get_curmode(void);
/*****************************************************************************
* �� �� ��  : mdrv_dload_set_softload
*
* ��������  : ����������־
*
* �������  : BSP_BOOL bSoftLoad  :
*             BSP_FALSE :��vxWorks����
*             BSP_TRUE  :��bootrom����
* �������  : ��
*
* �� �� ֵ  : ��
*
* ����˵��  : ��
*
*****************************************************************************/
void mdrv_dload_set_softload (int bSoftLoad);


/*****************************************************************************
* �� �� ��  : mdrv_dload_get_softload
*
* ��������  : ��ȡ������־
*
* �������  : void
* �������  : ��
*
* �� �� ֵ  : BSP_TRUE  :��bootrom����
*             BSP_FALSE :��vxWorks����
*
* ����˵��  : ��
*
*****************************************************************************/
int mdrv_dload_get_softload (void);    

/*****************************************************************************
* �� �� ��  : mdrv_dload_get_info
*
* ��������  : ���������汾��Ϣ
*
* �������  : ��
* �������  : ��
*
* �� �� ֵ  : ��Ϊ�� ���������汾��Ϣ
                    Ϊ�� ����ʧ��
*
* ����˵��  : ��
*
*****************************************************************************/
DLOAD_VER_INFO_S * mdrv_dload_get_info(void);

#if (FEATURE_ON == MBB_DLOAD)
/*****************************************************************************
* �� �� ��  : BSP_DLOAD_GetWebuiVer
*
* ��������  : ��ȡwebui�汾���ַ���
*
* �������  : char *str   :�ַ���������
*             unsigned int len   :�ַ�������
* �������  : char *str   :�ַ���������
*
* �� �� ֵ  : DLOAD_ERROR   :��������Ƿ�
*             ����          :�����ַ�������
*
* ����˵��  : ATģ�����
*             ����ģʽ֧��
*             ����ģʽ֧��
*
*****************************************************************************/
int mdrv_dload_getwebuiver(char *pVersionInfo,unsigned int ulLength);
/*****************************************************************************
* �� �� ��  : mdrv_dload_get_dloadver
* ��������  : ��ѯ����Э��汾�š�����ϢBSP�̶�дΪ2.0��
* �������  : char *str  :�ַ���������
*             unsigned int len      :�ַ�������
* �������  : char *str   :�ַ���������
* �� �� ֵ  : -1 -- ʧ��
*                   0 -- �ɹ�
* ����˵��  : ATģ�����
*             ����ģʽ֧��
*             ����ģʽ֧��
*****************************************************************************/
int mdrv_dload_get_dloadver(char *str, unsigned int len);

/*****************************************************************************
* �� �� ��   : mdrv_dload_getsoftver
* ��������   : ��ȡ����汾��
* �������   : BSP_S8 *str   : �ַ���������
*     BSP_S32 len   : �ַ�������
* �������  : BSP_S8 *str   : �ַ���������
* �� �� ֵ  : -1 -- ʧ��
             0 -- �ɹ�
* ����˵��   :  ATģ�����
*             ����ģʽ֧��
*             ����ģʽ֧��
*****************************************************************************/
int mdrv_dload_getsoftver(char *str, unsigned int len);

/*****************************************************************************
* �� �� ��   : mdrv_dload_getisover
* ��������   : ��ȡ��̨�汾��
* �������   : char *pVersionInfo   : �ַ���������
*     unsigned int ulLength   : �ַ�������
* �������   : char *pVersionInfo   : �ַ���������
* �� �� ֵ   : -1 -- ʧ��
                  0 -- �ɹ�
* ����˵��   :  ATģ�����
*             ����ģʽ֧��
*             ����ģʽ֧��
*****************************************************************************/
int mdrv_dload_getisover(char *pVersionInfo, unsigned int ulLength);

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
int mdrv_dload_get_productname(char * pProductName, unsigned int ulLength);

/*****************************************************************************
* �� �� ��   : mdrv_dload_get_dloadtype
* ��������   : ��ȡ����ģʽ
* �������   : void
* �������   : ��
* �� �� ֵ   : 0 -- ����ģʽ
                            1 -- ����ģʽ
* ����˵��   :  ATģ�����
*             ����ģʽ֧��
*             ����ģʽ֧��
*****************************************************************************/
int mdrv_dload_get_dloadtype(void);

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
void mdrv_dload_normal_reboot(void);

#endif

#ifdef _cplusplus
}
#endif
#endif

