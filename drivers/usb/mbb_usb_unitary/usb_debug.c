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
#include <linux/string.h>
#include "usb_debug.h"
#include "usb_platform_comm.h"

debug_mode_info debug_bank[MBB_LAST] = 
{
    [MBB_ALL]         = {U_ALL  ,"U_ALL"},  
    [MBB_DEBUG]       = {U_ALL  ,""},          
    [MBB_ACM]         = {U_ERROR,"U_ACM"},
    [MBB_NET]         = {U_ERROR,"U_NET"},
    [MBB_RNDIS]       = {U_ERROR,"U_RNDIS"},
    [MBB_ECM]         = {U_ERROR,"U_ECM"},
    [MBB_NCM]         = {U_ERROR,"U_NCM"},
    [MBB_CHARGER]     = {U_ERROR,"U_CHARGER"},
    [MBB_USB_NV]      = {U_ERROR,"U_NV"},
    [MBB_PNP]         = {U_ERROR,"U_PNP"},
    [MBB_EVENT]       = {U_ERROR,"U_EVENT"},
    [MBB_MASS]        = {U_ERROR,"U_MASS"},
    [MBB_OTG_CHARGER] = {U_ERROR,"U_OTG"},
    [MBB_USB_OTG]     = {U_ERROR,"U_OTG"},
    [MBB_HOTPLUG]     = {U_ERROR,"U_HOTPLUG"},
    [MBB_TYPECREG]    = {U_ERROR,"U_TYPECREG"},
};
EXPORT_SYMBOL(debug_bank);

USB_CHAR *lev_name[U_ALL+1] = 
{
    "TRACE",
    "ERROR",
    "WARN ",
    "INFO ",
    "ALL  "
};
EXPORT_SYMBOL(lev_name);

/*****************************************************************
Parameters    :  USB_VOID
Return        :    
Description   :  debug��ʼ��
*****************************************************************/
USB_VOID usb_debug_init(USB_VOID)
{
    USB_INT i = 0;
    DBG_SET_GLOBAL_LEVEL(U_ALL);
    {
        DBG_SET_LEVEL(MBB_ALL, U_ALL);
        for( i = 2; i < MBB_LAST; i++)
        {
            DBG_SET_LEVEL(i, U_ERROR);
        }
    }
}

/*****************************************************************
Parameters    :  type
Return        :    
Description   :  ���ͼ��
*****************************************************************/
USB_VOID valid_type(enum DEBUG_MODULES type)
{
    UNUSED_VAR(type);
}
EXPORT_SYMBOL_GPL(valid_type);

/*****************************************************************
Parameters    :  module
              lev
Return        :    
Description   :  ��ӡ����USB��log��ʹ�ô�log���������󣬻���ֺ���
                 ��log�����ʹ��
*****************************************************************/
USB_VOID usb_all_module_logs(USB_INT all_or_none)
{
    USB_INT i = 0;
    if(all_or_none)
    {
        DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
        DBG_T(MBB_DEBUG, "|          YOU WILL OUTPUT ALL USB DEBUG INFO!!!        |\n");
        DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
        for( i = 0; i < MBB_LAST; i++)
        {
            DBG_SET_LEVEL(i, U_ALL);
        }
    }
    else
    {
        DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
        DBG_T(MBB_DEBUG, "|            YOU CLOSED  ALL USB DEBUG INFO!!!          |\n");
        DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
        for( i = 0; i < MBB_LAST; i++)
        {
            DBG_SET_LEVEL(i, U_TRACE);
        }
    }
}

/*****************************************************************
Parameters    :  module
              lev
Return        :    
Description   :  ģ�鼶������
*****************************************************************/
USB_VOID usb_debug_lev_set(USB_INT module, USB_INT lev)
{
    DBG_SET_LEVEL(module, lev);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(module,    "|        module name %s-- %d set lev to %s-- %d         |\n",
          debug_bank[module].module_name, module, lev_name[lev], lev);
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
}

USB_VOID usb_debug_show_module_lev(USB_VOID)
{
    USB_INT i = 0;
    /*MBB_ALL & MBB_DEBUG do not need show the lever*/
    for ( i = 2; i < MBB_LAST; i++)
    {
        DBG_T(MBB_DEBUG, "|%15.15s(%-2d)---- lev: %-10.10s |\n",
              debug_bank[i].module_name, i, lev_name[debug_bank[i].lev]);
    }
}
/*****************************************************************
Parameters    :   None
Return        :    
Description   :  ģ�鼶�����ð���
*****************************************************************/
USB_VOID usb_debug_help(USB_VOID)
{
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|you can change the lev of the module to output it's log|\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|      use: dbg_lev_set(USB_INT module, USB_INT lev)    |\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|if you want to output all of the logs in module MASS   |\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|      you can input : ---- ecall dbg_lev_set 10 4      |\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|if you want to output all of the logs about usb        |\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|     you can input : ---- ecall usb_all_module_logs 1  |\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|if you use this, there will be a lage of logs to output|\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|---------------------|\n");
    DBG_T(MBB_DEBUG, "|the module is:       |\n");
    DBG_T(MBB_DEBUG, "|MBB_DEBUG:-------%-4d|\n",(USB_INT)MBB_DEBUG);
    DBG_T(MBB_DEBUG, "|MBB_ACM:---------%-4d|\n",(USB_INT)MBB_ACM);
    DBG_T(MBB_DEBUG, "|MBB_NET:---------%-4d|\n",(USB_INT)MBB_NET);
    DBG_T(MBB_DEBUG, "|MBB_RNDIS:-------%-4d|\n",(USB_INT)MBB_RNDIS);
    DBG_T(MBB_DEBUG, "|MBB_ECM:---------%-4d|\n",(USB_INT)MBB_ECM);
    DBG_T(MBB_DEBUG, "|MBB_NCM:---------%-4d|\n",(USB_INT)MBB_NCM);    
    DBG_T(MBB_DEBUG, "|MBB_CHARGER:-----%-4d|\n",(USB_INT)MBB_CHARGER);
    DBG_T(MBB_DEBUG, "|MBB_USB_NV:------%-4d|\n",(USB_INT)MBB_USB_NV);
    DBG_T(MBB_DEBUG, "|MBB_PNP:---------%-4d|\n",(USB_INT)MBB_PNP);
    DBG_T(MBB_DEBUG, "|MBB_EVENT:-------%-4d|\n",(USB_INT)MBB_EVENT);
    DBG_T(MBB_DEBUG, "|MBB_MASS:--------%-4d|\n",(USB_INT)MBB_MASS);
    DBG_T(MBB_DEBUG, "|MBB_OTG_CHARGER:-%-4d|\n",(USB_INT)MBB_OTG_CHARGER);
    DBG_T(MBB_DEBUG, "|---------------------|\n");
    DBG_T(MBB_DEBUG, "|the lev is:          |\n");
    DBG_T(MBB_DEBUG, "|U_TRACE:---------%-4d|\n",(USB_INT)U_TRACE);
    DBG_T(MBB_DEBUG, "|U_ERROR:---------%-4d|\n",(USB_INT)U_ERROR);
    DBG_T(MBB_DEBUG, "|U_WARN:----------%-4d|\n",(USB_INT)U_WARN);
    DBG_T(MBB_DEBUG, "|U_INFO:----------%-4d|\n",(USB_INT)U_INFO);
    DBG_T(MBB_DEBUG, "|---------------------|\n");
}

/*****************************************************************
Parameters    :   None
Return        :    
Description   :  LOG����
*****************************************************************/
USB_VOID usb_debug_test(USB_VOID)
{
    DBG_T(MBB_ACM, "MBB_ACM-----ERR");
    DBG_W(MBB_ACM, "MBB_ACM-----WARING");
    DBG_I(MBB_ACM, "MBB_ACM-----INFO");
    DBG_T(MBB_NET, "MBB_NET-----ERR");
    DBG_W(MBB_NET, "MBB_NET-----WARING");
    DBG_I(MBB_NET, "MBB_NET-----INFO");
    DBG_I(MBB_DEBUG, "U_ALL:%d\n",(USB_INT)U_ALL);
}

USB_VOID usb_MemHexDump(const USB_PUINT8 pbuf, USB_UINT size)
{
    USB_UINT i, row;
    USB_UINT8 line[USB_NUM_16 * USB_NUM_5];
    USB_UINT left = size;
    USB_UINT per_row = USB_NUM_16;
    USB_PUINT8 buf = pbuf;
    if (!buf)
    {
        return;
    }
    printk("Dump buffer %p size %u:\n", buf, size);    

#define TO_CHAR(a)      ((a) > 9 ? ((a) - 10 + 'A') : (a) + '0')
#define IS_PRINTABLE(a) ((a) > 31 && (a) < 127)

    for (row = 0; left; row++)
    {
        memset(line, ' ', sizeof(line));

        for (i = 0; (i < per_row) && left; i++, left--, buf++)
        {
            USB_UINT8 val = *buf;

            /* The HEX value */
            line[(i * 3)] = TO_CHAR(val >> 4);
            line[(i * 3) + 1] = TO_CHAR(val & 0x0F);

            /* The print char */
            line[(per_row * 3) + 2 + i] = IS_PRINTABLE(val) ? val : '.';
        }

        line[(per_row * 3) + 2 + per_row] = '\0';

        printk("[%4u]: %s\n", row * per_row, line);
    }
#undef TO_CHAR
#undef IS_PRINTABLE
    return;
}

