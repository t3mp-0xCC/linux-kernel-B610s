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


#include "usb_platform_comm.h"
#include "usb_debug.h"
#ifdef MBB_USB_UNITARY_Q   
#include "usb_nv_get.h"
#include "usb_smem.h"
ssize_t huawei_nv_write(nv_data_stru_t* nv_str);
#else
#include "bsp_nvim.h"
#include "mdrv_nvim.h"
#endif
/*******************************************************************************
*   Prototype:      boolean get_usb_config_smem( usb_port_info *usb_port )
*   Description:    从共享内存中获取端口形态
*   Input:          usb_port_info *usb_port  从VENDOR1中获取的端口形态
*   Output:         NA 
*   Return Value:  TRUE :获取成功
                         FALSE:获取失败
*   Date:           2014/2/27
*   Modification:   Created function
*******************************************************************************/
#ifdef USB_NV
#ifdef MBB_USB_UNITARY_Q   
static USB_INT usb_nv_smem_read(USB_UINT16 usID, USB_PVOID pItem, USB_UINT ulLength)
{
    mbb_usb_nv_info_st *usb_nv_info = NULL;
    huawei_vendor2_smem_info *smem_data = NULL;

    smem_data = usb_get_smem_info_vendor2();
    if (NULL == smem_data)
    {
        DBG_E(MBB_USB_NV,"smem alloc fail!\n");
        return MBB_USB_ERROR;
    }
    usb_nv_info = (mbb_usb_nv_info_st *)(smem_data->usb_smem_data);
    switch(usID)
    {
        case USB_NV_SERIAL_NUM_ID:
            memcpy(pItem,&(usb_nv_info->stUSBNvSnSupp),ulLength);
            break;
        case USB_NV_PID_UNIFICATION_ID:
            memcpy(pItem,&(usb_nv_info->pid_info),ulLength);
            break;
        case USB_NV_PORT_INFO_ID:
            memcpy(pItem,&(usb_nv_info->dev_profile_info),ulLength);
            break;
        case USB_NV_MASS_DYNAMIC_NAME:
            memcpy(pItem,&(usb_nv_info->mass_dynamic_name),ulLength);
            break;
         case USB_NV_DYNAMIC_INFO_NAME:
            memcpy(pItem,&(usb_nv_info->port_dynamic_name),ulLength);
            break;
         case USB_NV_PROT_OFFSET_ID:
            memcpy(pItem,&(usb_nv_info->stDevProtOfsInfo),ulLength);
            break;
         case USB_NV_FEATURE_WWAN:
            memcpy(pItem,&(usb_nv_info->feature_wwan),ulLength);
            break;
         case USB_NV_WINBLUE_PRF_ID:
            memcpy(pItem,&(usb_nv_info->winblue_profile),ulLength);
            DBG_T(MBB_USB_NV, "Winblue: profile nv val: %s, %u, %u, %u, %u, %u, %u\n", 
              usb_nv_info->winblue_profile.InterfaceName, 
              usb_nv_info->winblue_profile.MBIMEnable,
              usb_nv_info->winblue_profile.CdRom, 
              usb_nv_info->winblue_profile.TCard, 
              usb_nv_info->winblue_profile.MaxPDPSession, 
              usb_nv_info->winblue_profile.IPV4MTU, 
              usb_nv_info->winblue_profile.IPV6MTU);
              break;
         case USB_NV_NET_SPEED_ID:
            memcpy(pItem,&(usb_nv_info->net_speed_info),ulLength);
            break;
         case USB_NV_USB_PRIVATE_INFO:
            memcpy(pItem,&(usb_nv_info->usb_log_ctl),ulLength);
            break;
		 case USB_NV_HILINK_MODEM_MODE:
			pr_info("%s: USB_NV_USB_PRIVATE_INFO \n", __func__);
            memcpy(pItem,&(usb_nv_info->hilink_mode_flag),ulLength);
            break;
         default:
            break;
            
    }

    return MBB_USB_OK; 
}
#endif
#endif
/*****************************************************************************
* 函 数 名  : usb_read_nv
*
* 功能描述  : A核读NV接口函数
*
* 输入参数  : usID -> NV ID
              *pItem -> Save NV data buffer
              ulLength -> buffer length
* 输出参数  : 无
*
* 返 回 值  : 0:    操作成功
*                       -1: 操作失败
*
* 其它说明  : 无
*
*****************************************************************************/
USB_INT usb_read_nv(USB_UINT16 usID, USB_PVOID pItem, USB_UINT ulLength)
{
#ifdef USB_NV

#ifdef MBB_USB_UNITARY_Q   
    return usb_nv_smem_read(usID,pItem,ulLength);
#else
    return NVM_Read(usID, pItem, ulLength);
#endif

#else
    DBG_I(MBB_USB_NV, "no usb nv\n");
    return MBB_USB_OK; 
#endif

}


#ifdef USB_NV
#ifdef MBB_USB_UNITARY_Q
typedef struct
{
    TYPE_OP_INFO type;
    usb_nv_item  nv_id;
} usb_nv_data;

TYPE_OP_INFO nv_write_support[] = 
{
    {TYPE_NV_DEBUG_MODEM,USB_NV_USB_PRIVATE_INFO},
};


USB_INT usb_write_nv(USB_UINT16 usID, USB_PVOID pItem, USB_UINT ulLength)
{
    nv_data_stru_t* nv_str ;
    ssize_t ret = 0;
    int i = 0;
    TYPE_OP_INFO write_type ;
    DBG_T(MBB_USB_NV, "enter usb nv_write !\n" );
    if (NULL == pItem  || ulLength > NV_MAX_USER_BUFF_LEN)
    {
        DBG_T(MBB_USB_NV, " data point is NULL  or len beyond 1024!\n" );
        return -1;
    }


	for (i = 0; i < sizeof(nv_write_support)/sizeof(TYPE_OP_INFO); i++)
    {
        if ( usID == nv_write_support[i].nv_id )
        {
            write_type = nv_write_support[i].type;
            goto write_nv;
        }
    }
    
    return -1;
    
write_nv:
    nv_str = kmalloc(sizeof(nv_data_stru_t), GFP_KERNEL);
    if (!nv_str)
    {
        return -1;
    }
    nv_str->type = write_type;
    nv_str->len = ulLength;
    memset(nv_str->data, 0, sizeof(nv_str->data));
    memcpy(nv_str->data , pItem , ulLength);
    ret = huawei_nv_write(nv_str);
    if (ret < 0 )
    {
        DBG_T(MBB_USB_NV, "usb nv_write  failed!\n" );
        kfree(nv_str);
        return -1;
    }
    DBG_T(MBB_USB_NV, " nv type %d write success\n" , write_type);
    kfree(nv_str);

    return MBB_USB_OK;
}
#else/*MBB_USB_UNITARY_Q*/
USB_INT usb_write_nv(USB_UINT16 usID, USB_PVOID pItem, USB_UINT ulLength)
{
    return NVM_Write(usID, pItem, ulLength);
}
#endif/*MBB_USB_UNITARY_Q*/

#else/*USB_NV*/

USB_INT usb_write_nv(USB_UINT16 usID, USB_PVOID pItem, USB_UINT ulLength)
{
    DBG_T(MBB_USB_NV, "no USB nv\n");
    return MBB_USB_OK;
}
#endif/*USB_NV*/

