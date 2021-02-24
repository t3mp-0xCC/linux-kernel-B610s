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



#ifndef _MDRV_USB_H_
#define _MDRV_USB_H_


#include "product_config.h"

#ifdef _cplusplus
extern "C"
{
#endif

#ifdef __KERNEL__
#include <linux/notifier.h>
#endif

/* IOCTL CMD ���� */
#define ACM_IOCTL_SET_WRITE_CB      0x7F001000
#define ACM_IOCTL_SET_READ_CB       0x7F001001
#define ACM_IOCTL_SET_EVT_CB        0x7F001002
#define ACM_IOCTL_SET_FREE_CB       0x7F001003

#define ACM_IOCTL_WRITE_ASYNC       0x7F001010
#define ACM_IOCTL_GET_RD_BUFF       0x7F001011
#define ACM_IOCTL_RETURN_BUFF       0x7F001012
#define ACM_IOCTL_RELLOC_READ_BUFF  0x7F001013
#define ACM_IOCTL_SEND_BUFF_CAN_DMA 0x7F001014

#define ACM_IOCTL_IS_IMPORT_DONE    0x7F001020
#define ACM_IOCTL_WRITE_DO_COPY     0x7F001021

#define ACM_MODEM_IOCTL_SET_MSC_READ_CB 0x7F001030
#define ACM_MODEM_IOCTL_MSC_WRITE_CMD   0x7F001031
#define ACM_MODEM_IOCTL_SET_REL_IND_CB  0x7F001032

/* �첽�����շ��ṹ */
typedef struct tagACM_WR_ASYNC_INFO
{
    char *pVirAddr;
    char *pPhyAddr;
    unsigned int u32Size;
    void* pDrvPriv;
}ACM_WR_ASYNC_INFO;

/* ACM�豸�¼����� */
typedef enum tagACM_EVT_E
{
    ACM_EVT_DEV_SUSPEND = 0,        /* �豸�����Խ��ж�д(��Ҫ�����¼��ص�������״̬) */
    ACM_EVT_DEV_READY = 1,          /* �豸���Խ��ж�д(��Ҫ�����¼��ص�������״̬) */
    ACM_EVT_DEV_BOTTOM
}ACM_EVT_E;

typedef enum tagACM_IOCTL_FLOW_CONTROL_E
{
    ACM_IOCTL_FLOW_CONTROL_DISABLE = 0,      /* resume receiving data from ACM port */
    ACM_IOCTL_FLOW_CONTROL_ENABLE      /* stop receiving data from ACM port */
}ACM_IOCTL_FLOW_CONTROL_E;

/* ��buffer��Ϣ */
typedef struct tagACM_READ_BUFF_INFO
{
    unsigned int u32BuffSize;
    unsigned int u32BuffNum;
}ACM_READ_BUFF_INFO;


typedef void (*ACM_WRITE_DONE_CB_T)(char *pVirAddr, char *pPhyAddr, int size);
typedef void (*ACM_READ_DONE_CB_T)(void);
typedef void (*ACM_EVENT_CB_T)(ACM_EVT_E evt);
typedef void (*ACM_FREE_CB_T)(char* buf);
typedef void (*ACM_MODEM_REL_IND_CB_T)(unsigned int bEnable);

typedef void (*USB_UDI_ENABLE_CB_T)(void);
typedef void (*USB_UDI_DISABLE_CB_T)(void);


/* �豸ö�����˿ڸ��� */
#define DYNAMIC_PID_MAX_PORT_NUM        17
#define USB_NUM_19    19
typedef struct 
{
    unsigned long nv_status;
    unsigned char first_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char second_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char third_config_port[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char reserved_1[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char reserved_2[DYNAMIC_PID_MAX_PORT_NUM];
    unsigned char reserved_3[USB_NUM_19];
} DRV_HUAWEI_MULTI_CONFIG_PORT_TYPE;

#if (FEATURE_ON == MBB_USB_SOLUTION)
/*****************************************************************************
��������   usb_port_config_get
��������:  ��ѯconfig��Ӧ�Ķ˿�ֵ
��������� 
��������� 
����ֵ��   0:    ��ѯ�ɹ�
           ��������ѯʧ��
*****************************************************************************/
unsigned char* usb_port_config_get(unsigned char index);
#define DRV_USB_GET_CONFIG(index) usb_port_config_get(index)
/*****************************************************************************
��������   BSP_USB_PortTypeQuery
��������:  ��ѯ��ǰ���豸ö�ٵĶ˿���ֵ̬
��������� stDynamicPidType  �˿���̬
��������� stDynamicPidType  �˿���̬
����ֵ��   0:    ��ѯ�ɹ�
           ��������ѯʧ��
*****************************************************************************/
unsigned int BSP_USB_PortTypeQuery(DRV_HUAWEI_MULTI_CONFIG_PORT_TYPE *pstDynamicPidType);
#define DRV_SET_PORT_QUIRY(a)   BSP_USB_PortTypeQuery(a)
#endif


/*****************************************************************************
 *  �� �� ��  : mdrv_usb_reg_enablecb
 *  ��������  : Э��ջע��USBʹ��֪ͨ�ص�����
 *  �������  : pFunc: USBʹ�ܻص�����ָ��
 *
 *  �������  : ��
 *  �� �� ֵ  :  0	       �����ɹ���
 *                           ����	����ʧ�ܡ�
 *
 ******************************************************************************/
unsigned int mdrv_usb_reg_enablecb(USB_UDI_ENABLE_CB_T pFunc);

/*****************************************************************************
 *  �� �� ��  : mdrv_usb_reg_disablecb
 *  ��������  : Э��ջע��USBȥʹ��֪ͨ�ص�����
 *  �������  : pFunc: USBȥʹ�ܻص�����ָ��
 *
 *  �������  : ��
 *  �� �� ֵ  :  0	       �����ɹ���
 *                           ����	����ʧ�ܡ�
 *
 ******************************************************************************/
unsigned int mdrv_usb_reg_disablecb(USB_UDI_DISABLE_CB_T pFunc);

#define USB_CAPABILITY_THREE BSP_USB_CapabilityThree
#define DRV_GET_USB_SPEED(a)   BSP_USB_GetSpeed(a)
#if (FEATURE_ON == MBB_USB)
#if (FEATURE_ON == MBB_USB_TYPEC)
#define DRV_GET_TYPEC_STATUS   usbc_get_cc_status
/*****************************************************************************
��������   usbc_get_cc_status
��������:  ��ȡ��ǰCC ����״̬
�����������
����ֵ��   0:    δ���룬1��CC1���룬2 CC2���룬3��ERROR
*****************************************************************************/
int usbc_get_cc_status(void);
#endif
/*****************************************************************************
��������   BSP_USB_GetSpeed
��������:  �ṩ���ϲ��ѯ�豸USB���ʽӿ�
��������� char *buf
����ֵ��   0:    ��ѯ�ɹ�
           ��������ѯʧ��
*****************************************************************************/
int BSP_USB_GetSpeed(unsigned char * buf);
/*****************************************************************************
��������   BSP_USB_CapabilityThree
��������:  ��ѯ�豸USB����ģʽ�ӿ�
��������� char *
����ֵ��   
*****************************************************************************/
int BSP_USB_CapabilityThree(void);

/*****************************************************************************
 �� �� ��  : BSP_USB_GetPortMode
 ��������  : ��ȡ�˿���̬ģʽ�����ضԽ�����
 �������  : ��
 �������  :
 �� �� ֵ  :
*****************************************************************************/
extern unsigned char BSP_USB_GetPortMode(char*PsBuffer, unsigned long*Length );
#define DRV_GET_PORT_MODE(PsBuffer, Length)    BSP_USB_GetPortMode(PsBuffer,Length)
/*****************************************************************************
 �� �� ��  : BSP_USB_GetDiagModeValue
 ��������  : ����豸���͡�
 �������  : �ޡ�
 �������  : ucDialmode:  0 - ʹ��Modem����; 1 - ʹ��NDIS����; 2 - Modem��NDIS����
              ucCdcSpec:   0 - Modem/NDIS������CDC�淶; 1 - Modem����CDC�淶;
                           2 - NDIS����CDC�淶;         3 - Modem/NDIS������CDC�淶
 �� �� ֵ  : VOS_OK/VOS_ERR
*****************************************************************************/
extern int BSP_USB_GetDiagModeValue(unsigned char *pucDialmode,
                             unsigned char *pucCdcSpec);

#define DRV_GET_DIAG_MODE_VALUE(pucDialmode, pucCdcSpec)    \
                             BSP_USB_GetDiagModeValue(pucDialmode, pucCdcSpec)

/*****************************************************************************
��������   BSP_USB_MBIMSetMode
��������:  �ṩ���ϲ��л�MBIM�˿ڹ���ģʽ�Ľӿ�
��������� switch_idex   ��Ҫ�л��Ķ˿���̬
�������:  ��
����ֵ��   ��
*****************************************************************************/
void BSP_USB_MBIMSetMode(int switch_idex);
#define DRV_USB_MBIM_SET_MODE(switch_idex) BSP_USB_MBIMSetMode(switch_idex)

/*****************************************************************************
��������   BSP_USB_MBIMGetMode
��������:  �ṩ���ϲ���MBIM�˿ڹ���ģʽ�Ľӿ�
��������� ��
�������:  ��
����ֵ��   ��
*****************************************************************************/           
unsigned int BSP_USB_MBIMGetMode(void);
#define DRV_USB_MBIM_GET_MODE() BSP_USB_MBIMGetMode() 

#else
/*****************************************************************************
��������   BSP_USB_GetSpeed
��������:  �ṩ���ϲ��ѯ�豸USB���ʽӿ�
��������� char *buf
����ֵ��   0:    ��ѯ�ɹ�
           ��������ѯʧ��
*****************************************************************************/
static INLINE int BSP_USB_GetSpeed(unsigned char * buf)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return ERROR;
}
/*****************************************************************************
��������   BSP_USB_CapabilityThree
��������:  ��ѯ�豸USB����ģʽ�ӿ�
��������� char *
����ֵ��   
*****************************************************************************/
static INLINE int BSP_USB_CapabilityThree(void)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return ERROR;
}

/*****************************************************************************
 �� �� ��  : BSP_USB_GetPortMode
 ��������  : ��ȡ�˿���̬ģʽ�����ضԽ����󣬴�׮��
 �������  : ��
 �������  :
 �� �� ֵ  :
*****************************************************************************/
static inline unsigned char DRV_GET_PORT_MODE(char*PsBuffer, unsigned int*Length)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}

static inline int DRV_GET_DIAG_MODE_VALUE(unsigned char *pucDialmode,
                                          unsigned char *pucCdcSpec)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}

static inline void DRV_USB_MBIM_SET_MODE(int switch_idex)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}

static inline unsigned int DRV_USB_MBIM_GET_MODE(void)
{
    pr_err("%s stub!\n",__FUNCTION__);
    return 0;
}


#endif


#if (FEATURE_ON == MBB_USB)
typedef enum tagHW_USB_CB_E
{
    HWUSBCB_PM_REMOTE_WAKEUP_CTRL = 0,
    HWUSBCB_MAX , //����������
}HW_USB_CB_E;

/*Զ�̻��ѿ��ؿ��ƺ���ָ������*/
typedef void (*Pm_Remote_Wakeup_Ctrl_Func)(unsigned int);

typedef void* HW_USB_CB_FUNCPTR;
void mdrv_hw_usb_register_cb(HW_USB_CB_E type, HW_USB_CB_FUNCPTR func);

#endif /*#if (FEATURE_ON == MBB_USB)*/
#ifdef _cplusplus
}
#endif
#endif

