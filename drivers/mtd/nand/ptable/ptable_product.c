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



#ifdef __cplusplus
extern "C"
{
#endif

#include "ptable_def.h"
#include "ptable_com.h"

/*----------------------- |  0 byte
 |"pTableHead"            |
 *----------------------- | 16 byte (partition head flag string)
 | the property of table  |
 *----------------------- | 20 byte (partition head flag string)
 |"V7R2_FPGA" (example.)  |
 *----------------------- | 48 byte (partition table version name)
 | <partition info>       |
 |  (size 32byte)         |
 *----------------------- | 96 byte
 | < partition info >     |
 |  (size 32byte)         |
 |------------------------| 144 byte
 :    .............       :
 :    .............       :
 |------------------------| 48 x N byte
 | < partition info >     |
 |  (size 32byte)         |
 |------------------------| 48 x (N+1) byte
 | "T"  (table end flag)  |
 |                        |
 |------------------------| */

//#pragma GCC push_options
//#pragma GCC optimize("O0")

char ptable_tail[PTABLE_TAIL_SIZE] = {PTABLE_TAIL_STR};
/***********************************************************************
 * FLASH�ռ����(��Ʒ�߷�����)
 ***********************************************************************/
#if (FEATURE_ON == MBB_DLOAD)
/*kernel entry Address*/
#define MBB_PTABLE_KERNEL_ENTRY             (PTABLE_FASTBOOT_ENTRY + MBB_PTABLE_FASTBOOT_LEN)
/*table partition begin*/
/*
 * Size of ptable.bin is set 2KB, so max number of partitions is 42, if you have to use more than 42 partitions, you must modify somewhere,
 * e.g. fastboot(load ptable, burn ptable.bin to nand), t32 script(nandwrite.cmm).
 */
struct ST_PART_TBL ptable_product[PTABLE_PARTITION_MAX] =
{
    /*name*/         /*offset*/                 /*loadsize*/ /*capacity*/              /*loadaddr*/                /*entry*/                   /*type*/          /*property*/                                  /*count*/
#if (0 != MBB_PTABLE_M3BOOT_LEN)
    {"m3boot"       ,MBB_PTABLE_M3BOOT_START    ,0x00000000  ,MBB_PTABLE_M3BOOT_LEN    ,0x00000000                 ,0x00000000                ,IMAGE_M3BOOT     ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_FASTBOOT_LEN)
    {"fastboot"     ,MBB_PTABLE_FASTBOOT_START  ,0x00000000  ,MBB_PTABLE_FASTBOOT_LEN  ,PTABLE_FASTBOOT_ENTRY      ,PTABLE_FASTBOOT_ENTRY     ,IMAGE_FASTBOOT   ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_NVBACK_LEN)
    {"nvbacklte"    ,MBB_PTABLE_NVBACK_START    ,0x00000000  ,MBB_PTABLE_NVBACK_LEN    ,0x00000000                 ,0x00000000                ,IMAGE_NVBACKLTE  ,DATA_NORMAL | PTABLE_PROTECTED | PTABLE_MOUNT_MTD   ,0},
#endif
#if (0 != MBB_PTABLE_NVDEFAULT_LEN)
    {"nvdefault"    ,MBB_PTABLE_NVDEFAULT_START ,0x00000000  ,MBB_PTABLE_NVDEFAULT_LEN ,0x00000000                 ,0x00000000                ,IMAGE_NVFACTORY  ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_NVIMAGE_LEN)
    {"nvimg"        ,MBB_PTABLE_NVIMAGE_START   ,0x00000000  ,MBB_PTABLE_NVIMAGE_LEN   ,0x00000000                 ,0x00000000                ,IMAGE_NVIMG      ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_NVDLOAD_LEN)
    {"nvdload"      ,MBB_PTABLE_NVDLOAD_START   ,0x00000000  ,MBB_PTABLE_NVDLOAD_LEN   ,0x00000000                 ,0x00000000                ,IMAGE_NVDLD      ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_NVUPDATE_LEN)
    {"nvupdate"     ,MBB_PTABLE_NVUPDATE_START  ,0x00000000  ,MBB_PTABLE_NVUPDATE_LEN  ,0x00000000                 ,0x00000000                ,IMAGE_NVDLD   ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_NVSYS_LEN)
    {"nvsys"         ,MBB_PTABLE_NVSYS_START    ,0x00000000  ,MBB_PTABLE_NVSYS_LEN     ,0x00000000                 ,0x00000000                ,IMAGE_NVSYS      ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_OEMINFO_LEN)
    {"oeminfo"      ,MBB_PTABLE_OEMINFO_START   ,0x00000000  ,MBB_PTABLE_OEMINFO_LEN   ,0x00000000                 ,0x00000000                ,IMAGE_OEMINFO    ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_KERNEL_LEN)
    {"kernel"       ,MBB_PTABLE_KERNEL_START    ,0x00000000  ,MBB_PTABLE_KERNEL_LEN    ,MBB_PTABLE_KERNEL_ENTRY    ,MBB_PTABLE_KERNEL_ENTRY   ,IMAGE_KERNER     ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_KERNELBK_LEN)
    {"kernelbk"     ,MBB_PTABLE_KERNELBK_START  ,0x00000000  ,MBB_PTABLE_KERNELBK_LEN  ,MBB_PTABLE_KERNEL_ENTRY    ,MBB_PTABLE_KERNEL_ENTRY   ,IMAGE_KERNELBK   ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_DTIMG_LEN)
    {"dts"       ,MBB_PTABLE_DTIMG_START    ,0x00000000  ,MBB_PTABLE_DTIMG_LEN    ,0x00000000                 ,0x00000000                ,IMAGE_DTS     ,DATA_NORMAL | PTABLE_MOUNT_MTD                       ,0},
#endif
#if (0 != MBB_PTABLE_DTIMGBK_LEN)
    {"dtsbk"     ,MBB_PTABLE_DTIMGBK_START     ,0x00000000  ,MBB_PTABLE_DTIMGBK_LEN    ,0x00000000                 ,0x00000000                ,IMAGE_DTIMGBK      ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_LOGO_LEN)
    {"logo"         ,MBB_PTABLE_LOGO_START      ,0x00000000  ,MBB_PTABLE_LOGO_LEN      ,0x00000000                 ,0x00000000                ,IMAGE_LOGO       ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_M3IMAGE_LEN)
    {"m3image"      ,MBB_PTABLE_M3IMAGE_START   ,0x00000000  ,MBB_PTABLE_M3IMAGE_LEN   ,0x00000000                 ,0x00000000                ,IMAGE_M3IMAGE    ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_M3IMAGEBK_LEN)
    {"m3imagebk"    ,MBB_PTABLE_M3IMAGEBK_START ,0x00000000  ,MBB_PTABLE_M3IMAGEBK_LEN ,0x00000000                 ,0x00000000                ,IMAGE_M3IMAGEBK  ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_DSP_LEN)
    {"dsp"          ,MBB_PTABLE_DSP_START       ,0x00000000  ,MBB_PTABLE_DSP_LEN       ,0x00000000                 ,0x00000000                ,IMAGE_DSP        ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_HIFI_LEN)
    {"hifi"         ,MBB_PTABLE_HIFI_START     ,0x00000000  ,MBB_PTABLE_HIFI_LEN       ,0x00000000                 ,0x00000000                ,IMAGE_HIFI       ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_MISC_LEN)
    {"misc"         ,MBB_PTABLE_MISC_START      ,0x00000000  ,MBB_PTABLE_MISC_LEN      ,0x00000000                 ,0x00000000                ,IMAGE_MISC       ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_VXWORK_LEN)
    {"vxworks"      ,MBB_PTABLE_VXWORK_START    ,0x00000000  ,MBB_PTABLE_VXWORK_LEN    ,PTABLE_MODEM_CCORE_ENTRY ,PTABLE_MODEM_CCORE_ENTRY,IMAGE_CCORE      ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0}, 
#endif
#if (0 != MBB_PTABLE_RESERVE1_LEN)
    {"wbdata"     ,MBB_PTABLE_RESERVE1_START  ,0x00000000  ,MBB_PTABLE_RESERVE1_LEN  ,0x00000000                 ,0x00000000                ,IMAGE_RESERVE1   ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_RESERVE2_LEN)
    {"reserve2"     ,MBB_PTABLE_RESERVE2_START  ,0x00000000  ,MBB_PTABLE_RESERVE2_LEN  ,0x00000000                 ,0x00000000                ,IMAGE_RESERVE2   ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_RESERVE3_LEN)
    {"reserve3"     ,MBB_PTABLE_RESERVE3_START  ,0x00000000  ,MBB_PTABLE_RESERVE3_LEN  ,0x00000000                 ,0x00000000                ,IMAGE_RESERVE3   ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,0},
#endif
#if (0 != MBB_PTABLE_FOTA_LEN)
    {"fota"        ,MBB_PTABLE_FOTA_START   ,0x00000000  ,MBB_PTABLE_FOTA_LEN   ,0x00000000                 ,0x00000000                ,IMAGE_FOTA      ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_OM_LEN)
    {"om"           ,MBB_PTABLE_OM_START        ,0x00000000  ,MBB_PTABLE_OM_LEN        ,0x00000000                 ,0x00000000                ,IMAGE_OM         ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,0},
#endif
#if (0 != MBB_PTABLE_WEBSDK_LEN)
#if (FEATURE_ON == MBB_DLOAD_JFFS2)
    {"app"          ,MBB_PTABLE_WEBSDK_START    ,0x00000000  ,MBB_PTABLE_WEBSDK_LEN    ,0x00000000                 ,0x00000000                ,IMAGE_WEBSDK     ,DATA_NORMAL | PTABLE_MOUNT_MTD                       ,0},
#else
    {"app"          ,MBB_PTABLE_WEBSDK_START    ,0x00000000  ,MBB_PTABLE_WEBSDK_LEN    ,0x00000000                 ,0x00000000                ,IMAGE_WEBSDK     ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,0},
#endif
#endif
#if (0 != MBB_PTABLE_WEBUI_LEN)
#if (FEATURE_ON == MBB_DLOAD_JFFS2)
/*webui partition will suport jffs2 filesystem*/
    {"webui"        ,MBB_PTABLE_WEBUI_START     ,0x00000000  ,MBB_PTABLE_WEBUI_LEN     ,0x00000000                 ,0x00000000                ,IMAGE_WEBUI      ,DATA_NORMAL | PTABLE_MOUNT_MTD                       ,0},
#else
    {"webui"        ,MBB_PTABLE_WEBUI_START     ,0x00000000  ,MBB_PTABLE_WEBUI_LEN     ,0x00000000                 ,0x00000000                ,IMAGE_WEBUI      ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,0},
#endif
#endif
#if (0 != MBB_PTABLE_SYSTEM_LEN)
#if (FEATURE_ON == MBB_DLOAD_JFFS2)
    {"system"       ,MBB_PTABLE_SYSTEM_START    ,0x00000000  ,MBB_PTABLE_SYSTEM_LEN    ,0x00000000                 ,0x00000000                ,IMAGE_SYSTEM     ,DATA_NORMAL | PTABLE_MOUNT_MTD                       ,0},
#else
    {"system"       ,MBB_PTABLE_SYSTEM_START    ,0x00000000  ,MBB_PTABLE_SYSTEM_LEN    ,0x00000000                 ,0x00000000                ,IMAGE_SYSTEM     ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,0},
#endif
#endif
/*change usrdata and online partitions sequence in stick and hilink product currently*/
#if (0 != MBB_PTABLE_USERDATA_LEN)
    {"userdata"     ,MBB_PTABLE_USERDATA_START  ,0x00000000  ,MBB_PTABLE_USERDATA_LEN  ,0x00000000                 ,0x00000000                ,IMAGE_USERDATA   ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,0},
#endif
#if (0 != MBB_PTABLE_BBOU_LEN)
    {"online"       ,MBB_PTABLE_BBOU_START      ,0x00000000  ,MBB_PTABLE_BBOU_LEN      ,0x00000000                 ,0x00000000                ,IMAGE_BBOU       ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,0},
#endif
#if (0 != MBB_PTABLE_MMC_LEN)
    {"cdromiso"     ,MBB_PTABLE_MMC_START       ,0x00000000  ,MBB_PTABLE_MMC_LEN       ,0x00000000                 ,0x00000000                ,IMAGE_MMC        ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,0},
#endif
#if (0 != MBB_PTABLE_CACHE_LEN)
    {"cache"        ,MBB_PTABLE_CACHE_START     ,0x00000000  ,MBB_PTABLE_CACHE_LEN     ,0x00000000                 ,0x00000000                ,IMAGE_CACHE      ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif 
#if (0 != MBB_PTABLE_RECVA_LEN)
    {"recovery-a"   ,MBB_PTABLE_RECVA_START     ,0x00000000  ,MBB_PTABLE_RECVA_LEN     ,0x00000000                 ,0x00000000                ,IMAGE_RECOVERY_A ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#if (0 != MBB_PTABLE_RECVB_LEN)
    {"recovery-b"   ,MBB_PTABLE_RECVB_START     ,0x00000000  ,MBB_PTABLE_RECVB_LEN     ,0x00000000                 ,0x00000000                ,IMAGE_RECOVERY_B ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#ifdef BSP_CONFIG_BOARD_CPE
#if (0 != MBB_PTABLE_COREDUMP_LEN)
    {"coredump"        ,MBB_PTABLE_COREDUMP_START   ,0x00000000  ,MBB_PTABLE_COREDUMP_LEN   ,0x00000000                 ,0x00000000                ,IMAGE_COREDUMP      ,DATA_NORMAL | PTABLE_MOUNT_MTD                      ,0},
#endif
#endif
    {PTABLE_END_STR ,0x00000000                 ,0x00000000  ,0x00000000               ,0x00000000                 ,0x00000000                ,IMAGE_PART_TOP   ,DATA_YAFFS                                          ,0}
};
#else
/*table partition begin*/
struct ST_PART_TBL ptable_product[PTABLE_PARTITION_MAX] =
{
    /*name*/        /*offset*/      /*loadsize*/    /*capacity*/   /*loadaddr*/   /*entry*/     /*type*/            /*property*/
#if (PTABLE_M3_BOOT_LEN != 0)
    {PTABLE_M3BOOT_NM       ,PTABLE_M3_BOOT_START   , 0x00000000,PTABLE_M3_BOOT_LEN,  0x00000000,  0x00000000   ,IMAGE_M3BOOT ,DATA_NORMAL | PTABLE_MOUNT_MTD ,   0},
#endif
#if (PTABLE_FASTBOOT_LEN != 0)
    {PTABLE_FASTBOOT_NM     ,PTABLE_FASTBOOT_START    , 0x00000000,PTABLE_FASTBOOT_LEN,  PTABLE_FASTBOOT_ENTRY,  PTABLE_FASTBOOT_ENTRY   ,IMAGE_FASTBOOT  ,DATA_NORMAL | PTABLE_MOUNT_MTD   ,   0},
#endif
#if (PTABLE_NVFACTORY_LEN != 0)
    {PTABLE_NVDEFAULT_NM    ,PTABLE_NVFACTORY_START   , 0x00000000   ,PTABLE_NVFACTORY_LEN,   0x00000000,  0x00000000   ,IMAGE_NVFACTORY   ,DATA_NORMAL | PTABLE_MOUNT_MTD    ,   0},
#endif
#if (PTABLE_NV_LTE_LEN != 0)
    {PTABLE_NVBACK_NM    ,PTABLE_NV_LTE_START   , 0x00000000,PTABLE_NV_LTE_LEN,  0x00000000,  0x00000000   ,IMAGE_NVBACKLTE ,DATA_NORMAL | PTABLE_PROTECTED | PTABLE_MOUNT_MTD   ,   0},
#endif
#if (PTABLE_NVIMG_LEN != 0)
    {PTABLE_NVIMG_NM        ,PTABLE_NVIMG_START   , 0x00000000  ,PTABLE_NVIMG_LEN,  0x00000000,  0x00000000 ,IMAGE_NVIMG   ,DATA_NORMAL   | PTABLE_MOUNT_MTD  ,   0},
#endif
#if (PTABLE_NVDLD_LEN != 0)
    {PTABLE_NVDLOAD_NM      ,PTABLE_NVDLD_START   , 0x00000000   ,PTABLE_NVDLD_LEN,  0x00000000,  0x00000000   ,IMAGE_NVDLD   ,DATA_NORMAL | PTABLE_MOUNT_MTD     ,   0},
#endif
#if (PTABLE_SEC_STORAGE_LEN != 0)
    {PTABLE_SEC_STORAGE_NM  ,PTABLE_SEC_STORAGE_START, 0x00000000   ,PTABLE_SEC_STORAGE_LEN,  0x00000000,  0x00000000   ,IMAGE_SEC_STORAGE   ,DATA_YAFFS | PTABLE_MOUNT_MTD     ,   0},
#endif
#if (PTABLE_M3IMAGE_LEN != 0)
    {PTABLE_M3IMG_NM      ,PTABLE_M3IMAGE_START   ,0x00000000   ,PTABLE_M3IMAGE_LEN,   0x00000000,  0x00000000   ,IMAGE_M3IMAGE    ,DATA_NORMAL | PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_RECOVERY_A_LEN != 0)
    {PTABLE_RECOVERYA_NM   ,PTABLE_RECOVERY_A_START   ,0x00000000   ,PTABLE_RECOVERY_A_LEN,   0x00000000,  0x00000000   ,IMAGE_RECOVERY_A     ,DATA_NORMAL | PTABLE_MOUNT_MTD    ,   0},
#endif
#if (PTABLE_RECOVERY_B_LEN != 0)
    {PTABLE_RECOVERYB_NM   ,PTABLE_RECOVERY_B_START   ,0x00000000   ,PTABLE_RECOVERY_B_LEN,   0x00000000,  0x00000000   ,IMAGE_RECOVERY_B     ,DATA_NORMAL | PTABLE_MOUNT_MTD     ,   0},
#endif
#if (PTABLE_MISC_LEN != 0)
    {PTABLE_MISC_NM         ,PTABLE_MISC_START   ,0x00000000   ,PTABLE_MISC_LEN,   0x00000000,  0x00000000   ,IMAGE_MISC    ,DATA_NORMAL | PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_DTS_LEN != 0)
    {PTABLE_DTS_NM          ,PTABLE_DTS_START   ,0x00000000   ,PTABLE_DTS_LEN,   0x00000000,  0x00000000   ,IMAGE_DTS    ,DATA_NORMAL | PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_TEEOS_LEN != 0)
    {PTABLE_TEEOS_NM       ,PTABLE_TEEOS_START ,0x00000000   ,PTABLE_TEEOS_LEN, 0x00000000, 0x00000000, IMAGE_TEEOS    ,DATA_NORMAL | PTABLE_MOUNT_MTD,   0},
#endif
#if (PTABLE_BOOTIMAGE_LEN != 0)
    {PTABLE_KERNEL_NM       ,PTABLE_BOOTIMAGE_START ,0x00000000   ,PTABLE_BOOTIMAGE_LEN, (PTABLE_FASTBOOT_ENTRY + PRODUCT_CFG_FLASH_FASTBOOT_LEN), (PTABLE_FASTBOOT_ENTRY + PRODUCT_CFG_FLASH_FASTBOOT_LEN), IMAGE_KERNEL    ,DATA_NORMAL | PTABLE_MOUNT_MTD,   0},
#endif
#if (PTABLE_BOOTIMAGEBAK_LEN != 0)
    {PTABLE_KERNELBK_NM       ,PTABLE_BOOTIMAGEBAK_START ,0x00000000   ,PTABLE_BOOTIMAGEBAK_LEN, (PTABLE_FASTBOOT_ENTRY + PRODUCT_CFG_FLASH_FASTBOOT_LEN), (PTABLE_FASTBOOT_ENTRY + PRODUCT_CFG_FLASH_FASTBOOT_LEN), IMAGE_KERNELBK    ,DATA_NORMAL | PTABLE_MOUNT_MTD,   0},
#endif
#if (PTABLE_MODEM_FW_LEN != 0)
        {PTABLE_MODEM_FW_NM     ,PTABLE_MODEM_FW_START   ,0x00000000   ,PTABLE_MODEM_FW_LEN,   0x00000000,  0x00000000   ,IMAGE_MODEM_FW    ,DATA_YAFFS | PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_YAFFS_ONLINE_LEN != 0)
    {PTABLE_ONLINE_NM       ,PTABLE_YAFFS_ONLINE_START   ,0x00000000   ,PTABLE_YAFFS_ONLINE_LEN,   0x00000000,  0x00000000   ,IMAGE_ONLINE, DATA_YAFFS | PTABLE_MOUNT_MTD      ,   0},
#endif

#if (PTABLE_HIFI_LEN != 0)
    {PTABLE_HIFI_NM          ,PTABLE_HIFI_START ,0x00000000   ,PTABLE_HIFI_LEN,   0x00000000,  0x00000000   ,IMAGE_HIFI    ,DATA_NORMAL | PTABLE_MOUNT_MTD,   0},
#endif
#if (PTABLE_OEMINFO_LEN != 0)
    {PTABLE_OEMINFO_NM      ,PTABLE_OEMINFO_START   ,0x00000000 ,PTABLE_OEMINFO_LEN,   0x00000000,  0x00000000   ,IMAGE_OEMINFO    ,DATA_NORMAL | PTABLE_MOUNT_MTD,   0},
#endif
#if (PTABLE_RESERVE1_LEN != 0)
    {PTABLE_RESERVE1_NM      ,PTABLE_RESERVE1_START   ,0x00000000   ,PTABLE_RESERVE1_LEN,   0x00000000,  0x00000000   ,IMAGE_RESERVE1     ,DATA_NORMAL | PTABLE_MOUNT_MTD                       ,   0},
#endif
#if (PTABLE_RESERVE2_LEN != 0)
    {PTABLE_RESERVE2_NM      ,PTABLE_RESERVE2_START   ,0x00000000   ,PTABLE_RESERVE2_LEN,   0x00000000,  0x00000000   ,IMAGE_RESERVE2     ,DATA_NORMAL | PTABLE_MOUNT_MTD                       ,   0},
#endif
#if (PTABLE_RESERVE3_LEN != 0)
    {PTABLE_RESERVE3_NM      ,PTABLE_RESERVE3_START   ,0x00000000   ,PTABLE_RESERVE3_LEN,  0x00000000,  0x00000000   ,IMAGE_RESERVE3     ,DATA_YAFFS | PTABLE_MOUNT_MTD                       ,   0},
#endif
#if (PTABLE_OM_LEN != 0)
    {PTABLE_OM_NM           ,PTABLE_OM_START   ,0x00000000   ,PTABLE_OM_LEN,   0x00000000,  0x00000000   ,IMAGE_OM    ,DATA_YAFFS | PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_YAFFS_APP_LEN != 0)
    {PTABLE_APP_NM          ,PTABLE_YAFFS_APP_START  ,0x00000000   ,PTABLE_YAFFS_APP_LEN,   0x00000000,  0x00000000   ,IMAGE_APP    ,DATA_YAFFS |  PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_YAFFS_WEBUI_LEN != 0)
    {PTABLE_WEBUI_NM        ,PTABLE_YAFFS_WEBUI_START   ,0x00000000   ,PTABLE_YAFFS_WEBUI_LEN,   0x00000000,  0x00000000   ,IMAGE_WEBUI    ,DATA_YAFFS | PTABLE_MOUNT_MTD      ,   0},
#endif
#if (PTABLE_YAFFS_SYSIMG_LEN != 0)
    {PTABLE_SYSTEM_NM       ,PTABLE_YAFFS_SYSIMG_START   ,0x00000000   ,PTABLE_YAFFS_SYSIMG_LEN,   0x00000000,  0x00000000   ,IMAGE_SYSTEM    ,DATA_YAFFS | PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_YAFFS_USERDATA_LEN != 0)
    {PTABLE_USERDATA_NM     ,PTABLE_YAFFS_USERDATA_START   ,0x00000000   ,PTABLE_YAFFS_USERDATA_LEN,   0x00000000,  0x00000000   ,IMAGE_USERDATA    ,DATA_YAFFS | PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_LOGO_LEN != 0)
    {PTABLE_LOGO_NM       ,PTABLE_LOGO_START ,0x00000000   ,PTABLE_LOGO_LEN, 0x00000000, 0x00000000, IMAGE_LOGO, DATA_NORMAL | PTABLE_MOUNT_MTD,   0},
#endif
#if (PTABLE_CACHE_LEN != 0)
    {PTABLE_CACHE_NM        ,PTABLE_CACHE_START   ,0x00000000   ,PTABLE_CACHE_LEN,   0x00000000,  0x00000000   ,IMAGE_CACHE    ,DATA_NORMAL | PTABLE_MOUNT_MTD       ,   0},
#endif
#if (PTABLE_MULTI_CARRIER_LEN != 0)
    {PTABLE_MULTI_CARRIER_NM   ,PTABLE_MULTI_CARRIER_START   ,0x00000000   ,PTABLE_MULTI_CARRIER_LEN,   0x00000000,  0x00000000   ,IMAGE_MULTI_CARRIER     ,DATA_YAFFS | PTABLE_MOUNT_MTD     ,   0},
#endif

#if (PTABLE_ISO_LEN != 0)
    {PTABLE_CDROMISO_NM     ,PTABLE_ISO_START   ,0x00000000   ,PTABLE_ISO_LEN,   0x00000000,  0x00000000   ,IMAGE_CDROMISO    ,DATA_YAFFS | PTABLE_MOUNT_MTD       ,   0},
#endif
    {PTABLE_END_STR ,0x00000000   ,0x00000000   ,0x00000000,   0x00000000,  0x00000000   ,IMAGE_PART_TOP  ,DATA_YAFFS                        ,   0}
};
#endif /*MBB_DLOAD*/

struct ST_PTABLE_HEAD ptable_head =
{
	PTABLE_HEAD_STR,
	0x80000000,
	PTABLE_VER_ID_STR,
	PTABLE_VER_STR
};
//#pragma GCC pop_options

#ifdef __cplusplus
}
#endif

