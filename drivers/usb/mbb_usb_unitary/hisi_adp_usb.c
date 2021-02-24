/*lint -save -e19 -e123 -e537 -e713*/
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
 
#include <linux/usb/bsp_usb.h>
#include "usb_vendor.h"
#if (FEATURE_ON == MBB_USB)
#include "mbb_usb_adp.h"
#include "mdrv_nvim.h"
#include "bsp_nvim.h"
#include "drv_nv_id.h"
#endif
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/module.h>
#include "mdrv.h"

typedef struct
{
    usb_enum_done_cb_t enum_done_cbs[USB_ENUM_DONE_CB_BOTTEM];
    USB_UDI_ENABLE_CB_T udi_enable_cb[USB_ENABLE_CB_MAX];
    USB_UDI_DISABLE_CB_T udi_disable_cb[USB_ENABLE_CB_MAX];
}USB_CTX_S;


static unsigned g_usb_enum_done_cur = 0;
static unsigned g_usb_enum_dis_cur = 0;
#if (FEATURE_ON == MBB_USB)
#define MAX_EPIN_NUM            15
#define MAX_EPOUT_NUM           15
#endif

static struct notifier_block gs_adp_usb_nb;
static struct notifier_block *gs_adp_usb_nb_ptr = NULL;
static int g_usb_enum_done_notify_complete = 0;
static int g_usb_disable_notify_complete = 0;
static USB_CTX_S g_usb_ctx = {{0},{0},{0}};

USB_UDI_ENABLE_CB_T gs_usb_notifier = 0;

#if (FEATURE_ON == MBB_USB)

const unsigned char gDevProfileSupported[] = 
{
    USB_IF_PROTOCOL_3G_DIAG, 
#if (FEATURE_OFF == MBB_USB_E5)
    USB_IF_PROTOCOL_MODEM,
    USB_IF_PROTOCOL_3G_MODEM,
#endif
    USB_IF_PROTOCOL_CTRL, 
    USB_IF_PROTOCOL_PCUI,
    USB_IF_PROTOCOL_DIAG,
    USB_IF_PROTOCOL_3G_GPS,
    USB_IF_PROTOCOL_GPS,
    USB_IF_PROTOCOL_COMM_B,
    USB_IF_PROTOCOL_SHEEL_A,
    USB_IF_PROTOCOL_SHEEL_B,
    USB_IF_PROTOCOL_BLUETOOTH,
    USB_IF_PROTOCOL_NCM,
    USB_IF_PROTOCOL_CDROM,
#if (FEATURE_ON == MBB_USB_SD)    
    USB_IF_PROTOCOL_SDRAM,
#endif
    USB_IF_PROTOCOL_ADB,
};
#endif

/*****************************************************************************
 �� �� ��  : BSP_USB_SetPid
 ��������  :
 �������  : ��
 �������  : ��
 �� �� ֵ  : ��
*****************************************************************************/
int BSP_USB_SetPid(unsigned char u2diagValue)
{
    return OK;
}

/*****************************************************************************
��������   BSP_USB_PortTypeQuery
��������:  ��ѯ��ǰ���豸ö�ٵĶ˿���ֵ̬
��������� stDynamicPidType  �˿���̬
��������� stDynamicPidType  �˿���̬
����ֵ��   0:    ��ѯ�ɹ�
           ��������ѯʧ��
*****************************************************************************/
#if (FEATURE_ON == MBB_USB_SOLUTION)
unsigned int BSP_USB_PortTypeQuery(DRV_HUAWEI_MULTI_CONFIG_PORT_TYPE *pstDynamicPidType)
{
    unsigned  int ret = 0;
    if (NULL != pstDynamicPidType)
    {
        memset(pstDynamicPidType, 0, sizeof(DRV_HUAWEI_MULTI_CONFIG_PORT_TYPE));
        ret = NVM_Read(NV_ID_DRV_USB_MULTI_CONFIG_PORT_INFO, (void*)pstDynamicPidType,
                            sizeof(DRV_HUAWEI_MULTI_CONFIG_PORT_TYPE));
    }
   
    return ret;
}
#else
unsigned int BSP_USB_PortTypeQuery(DRV_DYNAMIC_PID_TYPE_STRU *pstDynamicPidType)
{
#if (FEATURE_ON == MBB_USB)
#if(FEATURE_ON == MBB_USB_E5)
    return 1;
#endif
    unsigned  int ret = 0;
    if (NULL != pstDynamicPidType)
    {
        memset(pstDynamicPidType, 0, sizeof(DRV_DYNAMIC_PID_TYPE_STRU));
        ret = NVM_Read(NV_ID_DRV_USB_DYNAMIC_PID_TYPE_PARAM, (void*)pstDynamicPidType,sizeof(DRV_DYNAMIC_PID_TYPE_STRU));
    }
   
    return ret;
#else
    return OK;
#endif	
}
#endif

/*****************************************************************************
��������   BSP_USB_PortTypeValidCheck
��������:  �ṩ���ϲ��ѯ�豸�˿���̬���úϷ��Խӿ�
           1���˿�Ϊ��֧�����ͣ�2������PCUI�ڣ�3�����ظ��˿ڣ�4���˵���������16��
           5����һ���豸��ΪMASS��
��������� pucPortType  �˿���̬����
           ulPortNum    �˿���̬����
����ֵ��   0:    �˿���̬�Ϸ�
           �������˿���̬�Ƿ�
*****************************************************************************/
unsigned int BSP_USB_PortTypeValidCheck(unsigned char *pucPortType, unsigned long ulPortNum)
{
#if (FEATURE_ON == MBB_USB)
#if(FEATURE_ON == MBB_USB_E5)
    return 1;
#endif
    uint32_t cnt = 0, type = 0, dev_loc = 0;
    uint32_t ep_in_cnt = 0, ep_out_cnt = 0;
    bool pcui_present =  false;
    uint8_t port_exist[256] = {0};

    printk(KERN_ERR "BSP_USB_PortTypeValidCheck");

    if ((NULL == pucPortType) || (0 == ulPortNum) || (DYNAMIC_PID_MAX_PORT_NUM < ulPortNum))
    {
        printk(KERN_ERR "%s: invalid param buf[%p] num[%ld]\n", __FUNCTION__, pucPortType, ulPortNum);
        return 1;
    }

    type = pucPortType[0];

    /* ����һ���豸����ΪMASS�豸����豸 */
    if ((USB_IF_PROTOCOL_CDROM == type) || (USB_IF_PROTOCOL_SDRAM == type) ||
        (USB_IF_PROTOCOL_VOID == type))
    {
        printk(KERN_ERR "%s: First device is Mass Storage device!\r\n", __FUNCTION__);
        return 1;
    }

#ifdef CONFIG_BALONG_RNDIS        // Hilink��̬��֧��Setport
    return 1;
#else
    /* ����л�����豸��̬ */
    for (cnt = 0; cnt < ulPortNum; cnt++)
    {
        type = pucPortType[cnt];

        if (0 < port_exist[type])
        {
            printk(KERN_ERR "%s: Port type repeat\n", __FUNCTION__);
            return 1;
        }

#if (FEATURE_ON == MBB_USB_E5)
        /* E5��Ʒ���ܴ�Modem�豸 */
        if ((USB_IF_PROTOCOL_3G_MODEM == type) || (USB_IF_PROTOCOL_MODEM == type))
        {
            printk(KERN_ERR "%s: There is MODEM with E5!\r\n", __FUNCTION__);
            return 1;
        }
#endif
        /* ���PCUI�˿��Ƿ���� */
        if ((USB_IF_PROTOCOL_3G_PCUI == type) || (USB_IF_PROTOCOL_PCUI == type))
        {
            pcui_present = true;
        }

        /* ����豸��̬�Ƿ���Ч֧�� */
        for (dev_loc = 0; dev_loc < sizeof(gDevProfileSupported); dev_loc++)
        {
            if (type == gDevProfileSupported[dev_loc])
            {
                break;
            }
        }

        if (dev_loc == sizeof(gDevProfileSupported))
        {
            printk(KERN_ERR "%s: Port type isn't supported by current device!\r\n", __FUNCTION__);
            printk(KERN_ERR "%s: dev_loc = [%d], type = [%d]\r\n", __FUNCTION__, dev_loc, type);
            return 1;
        }

        /* ͳ���ض��豸��̬��Ҫ��USB�˵���Ŀ */
        if ((USB_IF_PROTOCOL_NCM  == type) || (USB_IF_PROTOCOL_MODEM   == type) ||
            (USB_IF_PROTOCOL_NDIS == type) || (USB_IF_PROTOCOL_RNDIS   == type) ||
            (USB_IF_PROTOCOL_PCSC == type) || (USB_IF_PROTOCOL_3G_NDIS == type) ||
            (USB_IF_PROTOCOL_3G_MODEM == type))
        {
            ep_in_cnt  += 2;    /* Bulk IN + Interrupt IN */
            ep_out_cnt += 1;    /* Bulk OUT */
        }
        else
        {
            ep_in_cnt  += 1;    /* Bulk IN */
            ep_out_cnt += 1;    /* Bulk OUT */
        }

        port_exist[type]++;
    }

    /* ��PCUI�˿ڣ�����Ϊ�Ƿ� */
    if (false == pcui_present)
    {
        printk(KERN_ERR "%s: No PCUI!\r\n", __FUNCTION__);
        return 1;
    }

    /* �ж���Ҫ��USB�˵����Ƿ񳬹�USB IP��֧�� */
    if ((ep_in_cnt > MAX_EPIN_NUM) || (ep_out_cnt > MAX_EPOUT_NUM))
    {
        printk(KERN_ERR "%s: Too many ports-ep_in_cnt[%d], ep_out_cnt[%d]\n", __FUNCTION__,
                ep_in_cnt, ep_out_cnt);
        return 1;
    }
    return 0;
#endif /*#ifdef MBB_USB_RNDIS*/
    /* �豸��̬�������� */
#else	
    return OK;
#endif
}

/*****************************************************************************
��������   BSP_USB_GetAvailabePortType
��������:  �ṩ���ϲ��ѯ��ǰ�豸֧�ֶ˿���̬�б�ӿ�
��������� ulPortMax    Э��ջ֧�����˿���̬����
�������:  pucPortType  ֧�ֵĶ˿���̬�б�
           pulPortNum   ֧�ֵĶ˿���̬����
����ֵ��   0:    ��ȡ�˿���̬�б�ɹ�
           ��������ȡ�˿���̬�б�ʧ��
*****************************************************************************/
unsigned int BSP_USB_GetAvailabePortType(unsigned char *pucPortType,
                            unsigned long *pulPortNum, unsigned long ulPortMax)
{
#if (FEATURE_ON == MBB_USB)
    unsigned int i = 0;
    unsigned int cnt = 0;
    unsigned int j = 0;

    if ((NULL == pucPortType) || (NULL == pulPortNum) || (0 == ulPortMax))
    {
        return 1;
    }  

    cnt = sizeof(gDevProfileSupported) / sizeof(char);
    
    printk(KERN_ERR "aTm support  fds\n");
    if (cnt > ulPortMax)
    {
        return 1;
    }

    for (i = 0; i < cnt; i++)
    {
        pucPortType[j++] = gDevProfileSupported[i];
    }

    *pulPortNum = j;
#endif  
    return OK;
}

/*****************************************************************************
 �� �� ��  : BSP_USB_ATProcessRewind2Cmd
 ��������  : rewind2 CMD ����
 �������  : pData������
 �������  : ��
 �� �� ֵ  : false(0):����ʧ��
             tool(1):����ɹ�
*****************************************************************************/
int BSP_USB_ATProcessRewind2Cmd(unsigned char *pData)
{
    return OK;
}

/*****************************************************************************
 �� �� ��  : BSP_USB_GetDiagModeValue
 ��������  : ����豸����
 �������  : ��
 �������  : ucDialmode:  0 - ʹ��Modem����; 1 - ʹ��NDIS����; 2 - Modem��NDIS����
              ucCdcSpec:   0 - Modem/NDIS������CDC�淶; 1 - Modem����CDC�淶;
                           2 - NDIS����CDC�淶;         3 - Modem/NDIS������CDC�淶
 �� �� ֵ  : VOS_OK/VOS_ERR
*****************************************************************************/
int BSP_USB_GetDiagModeValue(unsigned char *pucDialmode,
                             unsigned char *pucCdcSpec)
{
#if (FEATURE_ON == MBB_USB)
    unsigned int ret = 1;

    printk(KERN_ERR" BSP_USB_GetDiagModeValue \n");

    ret = Query_USB_PortType(pucDialmode,pucCdcSpec);
    if ( 0 != ret )
    {
        return 1; 
    }
#endif    
    return 0;
}

/*****************************************************************************
 �� �� ��  :     BSP_USB_GetPortMode
 ��������  :  ��ȡ�˿���̬ģʽ�����ضԽ����󣬴�׮��
 �������  :  PsBuffer   �����ѯ���Ķ˿����Ƽ��˿��ϱ���˳��
                            Length     ��¼*PsBuffer���ַ����ĳ���
 �������  : 
 �� �� ֵ  :      �ɹ�����0��ʧ�ܷ���1
*****************************************************************************/
unsigned char BSP_USB_GetPortMode(char*PsBuffer, unsigned long*Length )
{
#if (FEATURE_ON == MBB_USB)
    printk(KERN_ERR"BSP_USB_GetPortMode Enter!\r\n");
    unsigned char ret = 1;

    ret = Check_EnablePortName(PsBuffer,Length);
    if( 0 != ret )
    {
        return 1;
    }
#endif
    return 0;
}

/*****************************************************************************
 �� �� ��  : BSP_USB_GetU2diagDefaultValue
 ��������  : ��ö˿�Ĭ��u2diagֵ
 �������  : ��
 �������  : ��
 ����ֵ��   u2diagֵ

*****************************************************************************/
BSP_U32 BSP_USB_GetU2diagDefaultValue(void)
{
    return 0;
}
#ifdef  MBB_USB_RESERVED

void MNTN_ERRLOG_REG_FUNC(MNTN_ERRLOGREGFUN pRegFunc)
{
    return ;
}
#endif 
/*****************************************************************************
 �� �� ��  : BSP_USB_UdiagValueCheck
 ��������  : ���ӿ����ڼ��NV����USB��ֵ̬�ĺϷ���
 �������  : ��
 �������  : ��
 �� �� ֵ  : 0��OK
             -1��ERROR
*****************************************************************************/
int BSP_USB_UdiagValueCheck(unsigned long DiagValue)
{
    return OK;
}

/*****************************************************************************
 �� �� ��  : BSP_USB_GetLinuxSysType
 ��������  : ���ӿ����ڼ��PC���Ƿ�ΪLinux���Թ��Linux��̨���β���ʧ�ܵ�����.
 �������  : �ޡ�
 �������  : �ޡ�
 �� �� ֵ  : 0��Linux��
            -1����Linux��
*****************************************************************************/
int BSP_USB_GetLinuxSysType(void)
{
    return -1;
}

/********************************************************
����˵���� ���ص�ǰ�豸�б���֧��(sel=1)���߲�֧��(sel=0)PCSC���豸��ֵ̬
��������:
���������sel
          0: ͨ������dev_type���ص�ǰ����PCSC���豸��ֵ̬
          1��ͨ������dev_type���ص�ǰ��PCSC���豸��ֵ̬
���������dev_type д����Ҫ���豸��ֵ̬�����û����д��ֵ��
          NV�д洢���豸��ֵ̬
���������pulDevType ��ulCurDevType��Ӧ���豸��ֵ̬�����û�з���ֵ1��
����ֵ��
          0����ѯ����Ӧ���豸��ֵ̬��
          1��û�в�ѯ����Ӧ���豸��ֵ̬��
********************************************************/
int BSP_USB_PcscInfoSet(unsigned int  ulSel, unsigned int  ulCurDevType, unsigned int *pulDevType)
{
    return 0;
}
//#endif

/********************************************************
����˵����Э��ջע��USBʹ��֪ͨ�ص�����
��������:
���������pFunc: USBʹ�ܻص�����ָ��
�����������
�����������
����ֵ  ��0���ɹ�
          1��ʧ��
********************************************************/
unsigned int BSP_USB_RegUdiEnableCB(USB_UDI_ENABLE_CB_T pFunc)
{
    if (g_usb_enum_done_cur >= USB_ENABLE_CB_MAX)
    {
        printk("mdrv_usb_reg_enablecb error:0x%x", (unsigned)pFunc);
        return (unsigned int)(-1);
    }

    g_usb_ctx.udi_enable_cb[g_usb_enum_done_cur] = pFunc;
    g_usb_enum_done_cur++;

	if (g_usb_enum_done_notify_complete)
    {
    	if (pFunc){
			gs_usb_notifier = pFunc;
        	pFunc();
    	}
    }
	gs_usb_notifier = 0;
    return 0;
}

/********************************************************
����˵����Э��ջע��USBȥʹ��֪ͨ�ص�����
��������:
���������pFunc: USBȥʹ�ܻص�����ָ��
�����������
�����������
����ֵ  ��0���ɹ�
          1��ʧ��
********************************************************/
unsigned int BSP_USB_RegUdiDisableCB(USB_UDI_DISABLE_CB_T pFunc)
{
    if (g_usb_enum_dis_cur >= USB_ENABLE_CB_MAX)
    {
        printk("mdrv_usb_reg_disablecb error:0x%x", (unsigned)pFunc);
        return (unsigned int)(-1);
    }

    g_usb_ctx.udi_disable_cb[g_usb_enum_dis_cur] = pFunc;
    g_usb_enum_dis_cur++;

    return 0;
}

#if (FEATURE_ON == MBB_USB)
/*****************************************************************************
* �� �� ��  : BSP_USB_MBIMSetMode
*
* ��������  : ���õ���ģʽֵ
*
* �������  : switch_idex ---����ģʽֵ
* �������  :
*
* �� �� ֵ  : ��
*
* ����˵��  :
*
*****************************************************************************/
USB_VOID BSP_USB_MBIMSetMode(USB_INT switch_idex)
{
#ifdef USB_CPE
#if (FEATURE_ON == MBB_BUILD_DEBUG) && defined(USB_PORT_AUTH)
    return usb_mode_ctrl(switch_idex);
#endif
#else
    return ncm_set_mbim_mode(switch_idex);
#endif
}

/*****************************************************************************
* �� �� ��  : BSP_USB_MBIMSetMode
*
* ��������  : ��ȡ��ǰ����ģʽֵ
*
* �������  : ��
* �������  :��ǰ����ģʽֵ
*
* �� �� ֵ  : ��
*
* ����˵��  :
*
*****************************************************************************/
USB_UINT BSP_USB_MBIMGetMode(USB_VOID)
{
#ifdef USB_CPE
    return 0;
#else
    return ncm_get_mbim_mode();
#endif
}
#endif

void gs_usb_notifier_cb_show(void)
{
	if (gs_usb_notifier){
		printk("gs_usb_notifier : %pS \n", gs_usb_notifier);
	}else{
		printk("gs_usb_notifier : NULL \n");
	}		
}
EXPORT_SYMBOL(gs_usb_notifier_cb_show);

static int gs_usb_adp_notifier_cb(struct notifier_block *nb,
            unsigned long event, void *priv)
{
    int loop;

    switch (event) {

    case USB_BALONG_DEVICE_INSERT:
        g_usb_disable_notify_complete = 0;
        break;
    case USB_BALONG_ENUM_DONE:
        /* enum done */
        g_usb_disable_notify_complete = 0;
        if (!g_usb_enum_done_notify_complete) {
            for(loop = 0; loop < USB_ENUM_DONE_CB_BOTTEM; loop++)
            {
				if(g_usb_ctx.enum_done_cbs[loop]){
					gs_usb_notifier = g_usb_ctx.enum_done_cbs[loop];
                    g_usb_ctx.enum_done_cbs[loop]();
				}
				
            }

            for(loop = 0; loop < USB_ENABLE_CB_MAX; loop++)
            {
                if(g_usb_ctx.udi_enable_cb[loop]){
					gs_usb_notifier = g_usb_ctx.udi_enable_cb[loop];
                    g_usb_ctx.udi_enable_cb[loop]();
                }
            }
        }
        g_usb_enum_done_notify_complete = 1;
		gs_usb_notifier = 0;
        break;
    case USB_BALONG_DEVICE_DISABLE:
    case USB_BALONG_DEVICE_REMOVE:
        /* notify other cb */
        g_usb_enum_done_notify_complete = 0;
        if (!g_usb_disable_notify_complete) {
            for(loop = 0; loop < USB_ENABLE_CB_MAX; loop++)
            {
                if(g_usb_ctx.udi_disable_cb[loop])
                    g_usb_ctx.udi_disable_cb[loop]();
            }
            g_usb_disable_notify_complete = 1;
        }
        break;
    default:
        break;
    }
    return 0;
}

#ifdef CONFIG_USB_SUPPORT
int __init adp_usb_init(void)
{
    /* we just regist once, and don't unregist any more */
    if (!gs_adp_usb_nb_ptr) {
        gs_adp_usb_nb_ptr = &gs_adp_usb_nb;
        gs_adp_usb_nb.priority = USB_NOTIF_PRIO_ADP;
        gs_adp_usb_nb.notifier_call = gs_usb_adp_notifier_cb;
        bsp_usb_register_notify(gs_adp_usb_nb_ptr);
    }
    return 0;
}
module_init(adp_usb_init);
#endif
/*lint -restore*/
