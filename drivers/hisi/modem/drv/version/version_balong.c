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



/*lint --e{537}*/
/*lint -save -e958*/

#include <linux/module.h>
#include <linux/string.h>
#include <drv_nv_id.h>
#include <bsp_nvim.h>
#include <bsp_version.h>
#include <linux/mtd/flash_huawei_dload.h>

/*
*--------------------------------------------------------------*
* �궨��                                                       *
*--------------------------------------------------------------*
*/
#define HUAWEI_DLOAD_CDROM_VERSION_LEN           128
#define HUAWEI_DLOAD_NAME_MAX_LEN    256
/* iso_info_flag_type�ṹ����sizeof()����ռ���ʵ����ռ�ռ�Ĳ�ֵ */
#define DIFFERENCE 3

PRODUCT_INFO_NV_STRU huawei_product_info = {0};
NV_SW_VER_STRU nv_sw_ver={0};

/*****************************************************************************
* ����  : mbb_ver_get_hw_sub_id
* ����  : ��ĳЩ��Ʒ�ϣ���Ҫͬʱ֧�ֶ��Ӳ���Ӱ汾����Ҫ��AT^VERSION�ܷ�����ȷ��
         Ӳ���Ӱ汾�š��������fastboot�ж�ȡ������ʵӲ���Ӱ汾�ţ�����hwIdSubֵ��
* ����  : void
* ���  : huawei_product_info.hwIdSub ����HKADC�Ӱ汾��ˢ��Ӳ���Ӱ汾����Ϣ
*****************************************************************************/
void mbb_ver_get_hw_sub_id(void)
{    
    u8 real_hwIdSub = 0;
    u32 board_id    = 0;
    /*��ȡ�����ڴ�����ʵ��Ӳ���Ӱ汾��*/
    real_hwIdSub = bsp_get_version_info()->real_hwIdSub;
    board_id     = bsp_get_version_info()->board_id;


    switch(board_id)
    {
        case HW_VER_PRODUCT_DC04:
        {
            /*���Ӳ���Ӱ汾��Ϊ0�ĵ��������⴦��*/
            if (0 == real_hwIdSub)
            {
                huawei_product_info.hwIdSub = real_hwIdSub;
            }
            else
            {
                huawei_product_info.hwIdSub = ((real_hwIdSub - 1) % HW_MBB_SUB_VER_NR);
            }
            break;
        }
        case HW_VER_PRODUCT_B610s_77a:
        {
            /* ���B610s-77a���汾�Ź̶�Ϊver.A */
            huawei_product_info.hwIdSub = 0;
            break;
        }
        /*Ĭ��һ����Ʒ3���Ӱ汾�ţ����Զ�3ȡ�ࡣ*/
        default:
        {
            huawei_product_info.hwIdSub = (real_hwIdSub % HW_MBB_SUB_VER_NR);
            break;
        }
    }
}

/*****************************************************************************
* �� �� ��     : bsp_version_productinfo_init
*
* ��������  : ��NV(0xD115)��ֵ��huawei_product_info
*
* ����˵��  : ��
*
*����ֵ : VER_ERROR��ʾ��ȡ�쳣
*                   VER_OK��ʾ��ȡ����
*****************************************************************************/
static __inline__ int bsp_version_productinfo_init(void)
{
    u32 ret = 0;
    u32 hw_ver = bsp_get_version_info()->board_id;

    /*lint -save -e26 -e119*/
    ret = bsp_nvm_read(NV_ID_DRV_NV_VERSION_INIT,(u8*)&huawei_product_info,sizeof(PRODUCT_INFO_NV_STRU));
    if(NV_OK != ret)
    {
        ver_print_error(" bsp_nvm_read failed!\n");
        return VER_ERROR;
    }

    if(hw_ver!=huawei_product_info.index){
        ver_print_error(" product id(0x%x) is not defined in xml!\n",hw_ver);
        huawei_product_info.index = hw_ver;
        /*��ȡ��Ʒ��ͬ��Ӳ���汾*/
        huawei_product_info.hwIdSub = hw_ver & 0xff;
        /*����HKADC�Ӱ汾��ֵˢ��hwIdSub.*/
        mbb_ver_get_hw_sub_id();
        return VER_ERROR;
    }
    /*lint -restore*/

    return VER_OK;
}

/*****************************************************************************
* �� �� ��  	: bsp_version_acore_init
*
* ��������  : acore�汾�ų�ʼ��
*
* ����˵��  : ��
*
*����ֵ : ��
*****************************************************************************/
int bsp_version_acore_init(void)
{
    int ret = VER_ERROR;
    memset((void *)(&huawei_product_info), 0, sizeof(PRODUCT_INFO_NV_STRU));

    ret = bsp_version_productinfo_init();
    if(VER_ERROR==ret)
        ver_print_error("bsp_version_productinfo_init fail! ");
    else
        ver_print_error("bsp version acore init OK!\n");

    mdrv_ver_init();
    return ret;
}



/*****************************************************************************
*                                                                                                                               *
*            ���½ӿ��ṩ��mdrv�ӿڵ���                                                  *
*                                                                                                                               *
******************************************************************************/



/*****************************************************************************
* �� ��	: bsp_version_get_hardware
* �� ��	: ��ȡӲ���汾��(Ӳ���汾����+ Ver.+Ӳ���Ӱ汾��+A)
*****************************************************************************/
char * bsp_version_get_hardware(void)
{
    u32 len;
    static bool b_geted=false;
    static char hardware_version[VERSION_MAX_LEN];
    char hardware_sub_ver = 0;

    if((HW_VER_INVALID == huawei_product_info.index))
    {
        ver_print_error("huawei_product_info.index = HW_VER_INVALID\n");
        return NULL;
    }

    if(!b_geted){
        /*lint -save -e18 -e718 -e746*/
        len = (unsigned int)(strlen(huawei_product_info.hwVer) + strlen(" Ver.X"));
        mbb_ver_get_hw_sub_id();
        hardware_sub_ver = (char)huawei_product_info.hwIdSub+'A';
        (void)memset((void *)hardware_version, 0, MemparamType(len));
        /*lint -restore*/
        /* coverity[secure_coding] */
        strncat(strncat(hardware_version, huawei_product_info.hwVer, strlen(huawei_product_info.hwVer)), " Ver.", strlen(" Ver."));
        *((hardware_version + len) - 1) = hardware_sub_ver;
        *(hardware_version + len) = 0;
        b_geted=true;
    }

    return (char *)hardware_version;
}

/*****************************************************************************
* �� ��	: bsp_get_product_inner_name
* �� ��	: ��ȡ�ڲ���Ʒ����(�ڲ���Ʒ��+ �ڲ���Ʒ��plus)
*****************************************************************************/
char * bsp_version_get_product_inner_name(void)
{
    unsigned int len;
    static bool b_geted=false;
    static char product_inner_name[VERSION_MAX_LEN];

    if( HW_VER_INVALID == huawei_product_info.index)
    {
        ver_print_error("huawei_product_info.index = HW_VER_INVALID\n");
        return NULL;
    }

    if(!b_geted){
        len = (unsigned int)(strlen(huawei_product_info.name)+ strlen(huawei_product_info.namePlus));
        (void)memset((void*)product_inner_name, 0, MemparamType(len));

        /* coverity[secure_coding] */
        strncat(strncat(product_inner_name,huawei_product_info.name, strlen(huawei_product_info.name)), huawei_product_info.namePlus, strlen(huawei_product_info.namePlus));
        *(product_inner_name+len) = 0;
        b_geted=true;
    }

    return  (char*)product_inner_name;
}

/*****************************************************************************
* �� ��	: bsp_get_product_out_name
* �� ��	: ��ȡ�ⲿ��Ʒ����
*****************************************************************************/
char * bsp_version_get_product_out_name(void)
{
    if(HW_VER_INVALID == huawei_product_info.index)
    {
        ver_print_error("huawei_product_info.index = HW_VER_INVALID\n");
        return NULL;
    }

    return (char *)huawei_product_info.productId;
}



/*****************************************************************************
*                                                                                                                               *
*            ���½ӿ��ṩ��dumpģ�����                                                 *
*                                                                                                                               *
******************************************************************************/



/*****************************************************************************
* �� ��	: bsp_get_build_date_time
* �� ��	: ��ȡ�������ں�ʱ��
*****************************************************************************/
char * bsp_version_get_build_date_time(void)
{
	static char * build_date   = __DATE__ ", " __TIME__;
	return build_date;
}

/*****************************************************************************
* �� ��	: bsp_get_chip_version
* �� ��	: ��ȡоƬ�汾��
*****************************************************************************/
char * bsp_version_get_chip(void)
{
	return (char *)PRODUCT_CFG_CHIP_SOLUTION_NAME;
}

/*****************************************************************************
* �� ��	: bsp_get_firmware_version
* �� ��	: ��ȡ����汾��
*****************************************************************************/
char * bsp_version_get_firmware(void)
{
    u32 ret = VER_OK;

    /*lint -save -e26 -e119*/
    ret=bsp_nvm_read(NV_ID_DRV_NV_DRV_VERSION_REPLACE_I, (u8 *)&nv_sw_ver, sizeof(NV_SW_VER_STRU));
    if(NV_OK != ret){
        ver_print_error("get NV_SW_VERSION_REPLACE failed!\n");
        return (char*)PRODUCT_DLOAD_SOFTWARE_VER;
    }
    else if(0 == nv_sw_ver.nvStatus){
        return (char*)PRODUCT_DLOAD_SOFTWARE_VER;
    }
    /*lint -restore*/

    return (char *)nv_sw_ver.nv_version_info;
}

#define PAGE_SIZE_MAX 4096
s32 huawei_get_spec_num_upinfo(char* str, s32 str_len, s32 num)
{
    rec_updinfo_st rec_info = {0};
    char *format_str = "%s%s\r\n\r\n%s%s\r\n\r\n%s%s\r\n\r\n%s%d\r\n\r\n%s%d";
    
    if(NULL == str)
    {
        return VER_ERROR;
    }
    /* get update info from oeminfo area */
    (void)flash_get_share_region_info( RGN_RECORD_UPINFO_FLAG, 
                        (void *)(&rec_info), sizeof(rec_updinfo_st));

    if(DLOAD_RECORD_UPDATE_MAGIC != rec_info.magic_num)
    {
        /* oeminfo is empty, return */
        return VER_ERROR;
    }
    if(num > rec_info.upinfo_record_times)
    {
        /* para err */
        return VER_ERROR;
    }
    snprintf(str, str_len, format_str, 
                "bswver: ", 
                rec_info.upswver_info[num - 1].bswver,
                "aswver: ",
                rec_info.upswver_info[num - 1].aswver,
                "uptime: ",
                rec_info.upswver_info[num - 1].uptime,
                "uptype: ",
                rec_info.upswver_info[num - 1].uptype,
                "upstatus: ",
                rec_info.upswver_info[num - 1].upstatus
            );
    return VER_OK;
    
}

void huawei_get_update_info_times(s32* times)
{
    rec_updinfo_st rec_info = {0};
    if(NULL == times)
    {
        return;
    }

    /* get update info from oeminfo area */
    (void)flash_get_share_region_info(RGN_RECORD_UPINFO_FLAG, 
                        (void *)(&rec_info), sizeof(rec_updinfo_st));
    if(DLOAD_RECORD_UPDATE_MAGIC != rec_info.magic_num)
    {
        /* oeminfo is empty, return 0 */
        *times = 0;
    }
    else
    {
        *times = rec_info.upinfo_record_times;
    }
    return;
}

void huawei_set_update_info(char* str)
{
    rec_updinfo_st rec_info = {0};
    update_info_st update_info = {{0}, {0}, 0};
    uint8 i = 0;
    uint32 cur_times = 0;
    int8* firmware_ver = NULL;
    int8 ret_buf[PAGE_SIZE_MAX] = {0};
    if(NULL == str)
    {
        ver_print_error("str is null\n");
        return;
    }
    memcpy(&update_info, str, sizeof(update_info_st));
    /* get update info from oeminfo area */
    (void)flash_get_share_region_info( RGN_RECORD_UPINFO_FLAG, 
                        (void *)(&rec_info), sizeof(rec_updinfo_st));

    /* check the magic */
    if(DLOAD_RECORD_UPDATE_MAGIC != rec_info.magic_num)
    {
        rec_info.total_times = 1;
        rec_info.upinfo_record_times = 1;
        cur_times = rec_info.upinfo_record_times - 1;
        ver_print_error("rec_info.magic_num is not magic\n");
    }
    else
    {
        /* if the current update status is 1, update this struct */
        if(UPDATE_BEGIN_STATUS == rec_info.upswver_info[rec_info.upinfo_record_times - 1].upstatus)
        {
            cur_times = rec_info.upinfo_record_times - 1;  // 4 = 5 - 1
            ver_print_error("repeat at ,times is %d\n", rec_info.upinfo_record_times);
        }
        else
        {
            if(RECORD_MAX_TIMES <= rec_info.upinfo_record_times)
            {
                /* << ,set the back info to front */
                for(i = 0; i < RECORD_MAX_TIMES - 1; i++)
                {
                    rec_info.upswver_info[i] = rec_info.upswver_info[i + 1];
                }
                (void)memset(&rec_info.upswver_info[RECORD_MAX_TIMES - 1], 0, sizeof(updswver_info_st));
                rec_info.upinfo_record_times = RECORD_MAX_TIMES;
                cur_times = RECORD_MAX_TIMES - 1;   /* set cur_times -> 10 - 1 */
            }
            else
            {
                cur_times = rec_info.upinfo_record_times;
                rec_info.upinfo_record_times++;
                ver_print_error("update at ,times is %d\n", rec_info.upinfo_record_times);
            }
        }
    }

    /* update the target software version */
    (void)strncpy(rec_info.upswver_info[cur_times].aswver, update_info.ver_info, VERSION_LENGTH - 1);

    /* update the current software version */
    firmware_ver = bsp_version_get_firmware();
    if(NULL != firmware_ver)
    {
        ver_print_error("the before version :%s\n", rec_info.upswver_info[cur_times].bswver);
        (void)memset(rec_info.upswver_info[cur_times].bswver, 0, VERSION_LENGTH);
        (void)strncpy(rec_info.upswver_info[cur_times].bswver, firmware_ver, strlen(firmware_ver));
        ver_print_error("the current version :%s\n", rec_info.upswver_info[cur_times].bswver);
    }

    /* update the current upgrade UTC time */
    (void)strncpy(rec_info.upswver_info[cur_times].uptime, update_info.time_info, VERSION_LENGTH - 1);
    
    /* update the tools type */
    rec_info.upswver_info[cur_times].uptype = update_info.tooltype;

    /* update the record times */
    rec_info.upswver_info[cur_times].upstatus = UPDATE_BEGIN_STATUS;
    
    /* update the magic number */
    rec_info.magic_num = DLOAD_RECORD_UPDATE_MAGIC;
    memcpy(ret_buf, &rec_info, sizeof(rec_updinfo_st));

    (void)flash_update_share_region_info(RGN_RECORD_UPINFO_FLAG, (void *)ret_buf, sizeof(rec_updinfo_st));
    return;
}

/*****************************************************************************
* �� �� : bsp_get_webui_version
* �� �� : ��ȡwebui_version
* �� �� : 
           str:����webui�汾��
           len:��Ҫ��ȡ��webui�汾�ų���
* �� �� : ��
* �� �� : ��
* ˵ �� :��ȡ����ָ�벻�����ͷ�
*****************************************************************************/
void bsp_get_web_version(char *str,int len)
{
    unsigned int   verLen = 0;
    unsigned char   bufTmp[WEBUI_VER_BUF_LEN] = {0};
    webver_buffer_s  *webver_info = NULL;
    char  webui_ver[WEBUI_VER_BUF_LEN] = "WEBUI_00.00.00.000.03";

    /*����ж�*/
    if(NULL == str || len < WEBUI_VER_BUF_LEN)
    {
        ver_print_error("Input parameter error!.\r\n");
        return ;
    }
    
    /*��ȡ��������汾��Ϣ�ַ�����Ϣ*/ 
    verLen = WEBUI_VER_BUF_LEN;
    memset(bufTmp, 0, WEBUI_VER_BUF_LEN);

    /* ��ȡISO�汾�� */
    /*lint -e429*/
    if (true == flash_get_share_region_info(RGN_WEBHD_FLAG,bufTmp,verLen))
    {
        webver_info = (webver_buffer_s*)bufTmp;
        if (webver_info->magic_number == WEBUI_VER_FLAG_MAGIC)
        {
            memset(str, 0, len);
            snprintf((char*)str, WEBUI_VER_BUF_LEN, "%s", webver_info->version_buffer);
            ver_print_error("flash_get_share_region_info ture. \r\n");
        }
        else
        {
            memset(str, 0, len);
            snprintf((char*)str, WEBUI_VER_BUF_LEN, "%s", webui_ver);
            ver_print_error("flash_get_share_region_info ture iso null.\r\n");
        }
    }
    else
    {
        memset(str, 0, len);
        snprintf((char*)str, WEBUI_VER_BUF_LEN, "%s", webui_ver);
        ver_print_error("flash_get_share_region_info false.\r\n");
    }
    return;    
}


void bsp_get_iso_version(char *str, int len)
{
    unsigned int   verLen = 0;
    unsigned char   bufTmp[HUAWEI_DLOAD_CDROM_VERSION_LEN] = {0};
    iso_info_flag_type  *iso_version_info = NULL;
    char  iso_ver[HUAWEI_DLOAD_CDROM_VERSION_LEN] = "00.00.00.000.00";

    /*����ж�*/
    if(NULL == str || len < HUAWEI_DLOAD_CDROM_VERSION_LEN)
    {
        ver_print_error("Input parameter error!.\r\n");
        return ;
    }
    
    /*��ȡ��������汾��Ϣ�ַ�����Ϣ*/ 
    verLen = HUAWEI_DLOAD_CDROM_VERSION_LEN;
    memset(bufTmp, 0, HUAWEI_DLOAD_CDROM_VERSION_LEN);

    /* ��ȡISO�汾�� */
    /*lint -e429*/
    if (true == flash_get_share_region_info(RGN_ISOHD_FLAG,bufTmp,verLen))
    {
        iso_version_info = (iso_info_flag_type*)bufTmp;
        if (iso_version_info->in_use_flag == FLASH_OEM_REGION_IN_USE_FLAG 
            && iso_version_info->iso_info_magic == DLOAD_ISO_VERSION_MAGIC)
        {
            memset(str, 0, len);
            /*  �˴�Ϊ3�Ǹ���iso_info_flag_type�ṹ��ԭ������*/
            snprintf((char*)str, HUAWEI_DLOAD_CDROM_VERSION_LEN, "%s", \
                    bufTmp + sizeof(iso_info_flag_type) - DIFFERENCE);
             ver_print_error("flash_get_share_region_info ture. \r\n");
        }
        else
        {
            memset(str, 0, len);
            snprintf((char*)str, HUAWEI_DLOAD_CDROM_VERSION_LEN, "%s", iso_ver);
            ver_print_error("flash_get_share_region_info ture iso null.\r\n");
        }
    }
    else
    {
        memset(str, 0, len);
        snprintf((char*)str, HUAWEI_DLOAD_CDROM_VERSION_LEN, "%s", iso_ver);
        ver_print_error("flash_get_share_region_info false.\r\n");
    }

    return ;
}

/*****************************************************************************
* �� ��	: bsp_version_get_release
* �� ��	: ��ȡȫ�汾��
*****************************************************************************/
char * bsp_version_get_release(void)
{
    return PRODUCT_FULL_VERSION_STR;
}


/*****************************************************************************
* ����  : mbb_version_get_board_type
* ����  : ��Ʒ�߻�ȡ��ƷӲ��IDʹ�ýӿ�
                     ��Ʒ�����������в���ʹ������ĺ�˼�ӿ�
* ����  : void
* ���  : void
* ����  : BOARD_TYPE_E
*****************************************************************************/
u32 mbb_version_get_board_type(void)
{
    unsigned int type = 0xffffffff;
    type = bsp_get_version_info()->board_id;

    if(HW_VER_INVALID == type)
    {
        ver_print_error("[mbb_version_get_board_type]:get hw version failed!hw_ver:0x%x\n", type);
    }

    return type;
}


/*****************************************************************************
* �� �� ��  	: bsp_get_version_info
*
* ��������  : Ϊ����ģ���ṩ���ְ汾����Ϣ
*
* ����˵��  : ��
*
*����ֵ : version�Ĺ����ڴ����ݽṹ��ĵ�ַ
*****************************************************************************/
const BSP_VERSION_INFO_S* bsp_get_version_info(void)
{
	return (BSP_VERSION_INFO_S *)(SHM_BASE_ADDR + SHM_OFFSET_VERSION);
}

/*****************************************************************************
* �� ��	: bsp_version_debug
* �� ��	: ���ڵ��Բ鿴�汾�������Ϣ
*****************************************************************************/
int bsp_version_debug(void)
{
    if(huawei_product_info.index == HW_VER_INVALID){
        ver_print_error("huawei_product_info.index = HW_VER_INVALID\n");
        return VER_ERROR;
    }

    ver_print_error("\n\n1 . the element value of nv(0xd115):\n");
    ver_print_error("Hardware index :0x%x\n",huawei_product_info.index);
    ver_print_error("hw_Sub_ver     :0x%x\n",huawei_product_info.hwIdSub);
    ver_print_error("Inner name     :%s\n",huawei_product_info.name);
    ver_print_error("name plus      :%s\n",huawei_product_info.namePlus);
    ver_print_error("HardWare ver   :%s\n",huawei_product_info.hwVer);
    ver_print_error("DLOAD_ID       :%s\n",huawei_product_info.dloadId);
    ver_print_error("Out name       :%s\n",huawei_product_info.productId);

    ver_print_error("\n\n2 . the element value of nv(0xc362):\n");
    ver_print_error("nvStatus       :0x%x\n",nv_sw_ver.nvStatus);
    ver_print_error("nv_version_info:%s\n",nv_sw_ver.nv_version_info);

    ver_print_error("\n\n3 . get from func(bsp_version_get_xxx):\n");
    ver_print_error("HardWare ver   :%s\n",bsp_version_get_hardware());
    ver_print_error("Inner name     :%s\n",bsp_version_get_product_inner_name());
    ver_print_error("Out name       :%s\n",bsp_version_get_product_out_name());
    ver_print_error("Build_time     :%s\n",bsp_version_get_build_date_time());
    ver_print_error("Chip_ver       :%s\n",bsp_version_get_chip());
    ver_print_error("Firmware       :%s\n",bsp_version_get_firmware());
    ver_print_error("Release_ver    :%s\n",bsp_version_get_release());

    ver_print_error("\n\n4 . get from bsp_get_version_info:\n");
    ver_print_error("board_id            :0x%x\n",bsp_get_version_info()->board_id);
    ver_print_error("Real SubID     :0x%x\n",bsp_get_version_info()->real_hwIdSub);
    ver_print_error("board_id_udp_masked :0x%x\n",bsp_get_version_info()->board_id_udp_masked);
    ver_print_error("chip_version        :0x%x\n",bsp_get_version_info()->chip_version);
    ver_print_error("chip_type           :0x%x\n",bsp_get_version_info()->chip_type);
    ver_print_error("plat_type           :0x%x  (0:asic a:fpga e:emu)\n",bsp_get_version_info()->plat_type);
    ver_print_error("board_type(for drv) :0x%x  (0:bbit 1:sft 2:asic 3:soc)\n",bsp_get_version_info()->board_type);
    ver_print_error("board_type(for mdrv):0x%x  (0:bbit 1:sft 2:asic)\n",(BOARD_ACTUAL_TYPE_E)bsp_get_version_info()->board_type);
    ver_print_error("bbit_type           :0x%x  (1:dallas 2:722 3:chicago)\n",bsp_get_version_info()->bbit_type);
    ver_print_error("product_type        :0x%x  (0:mbb 1:phone)\n",bsp_get_version_info()->product_type);
    ver_print_error("product_name        :0x%x\n",bsp_get_version_info()->product_name);

    return VER_OK;
}

/*ע��:����nvģ���ʼ��֮��*/
module_init(bsp_version_acore_init);

EXPORT_SYMBOL_GPL(huawei_product_info);
EXPORT_SYMBOL_GPL(bsp_version_acore_init);
EXPORT_SYMBOL_GPL(bsp_version_get_hardware);
EXPORT_SYMBOL_GPL(bsp_version_get_product_out_name);
EXPORT_SYMBOL_GPL(bsp_version_get_product_inner_name);
EXPORT_SYMBOL_GPL(bsp_get_version_info);
EXPORT_SYMBOL_GPL(bsp_version_debug);

/*lint -restore*/

/*****************************************************************************
* �� �� ��  : bsp_version_get_baseline
*
* ��������  : ��ȡ��˼���߰汾��
*
* �������  : ��
* �������  :
*
 �� �� ֵ  : ����汾���ַ���ָ��
* �޸ļ�¼  :
*
*****************************************************************************/
char * bsp_version_get_baseline(void)
{
      /* ����V7R2���߰汾 */
    return (char*)PRODUCT_VERSION_STR;
}
