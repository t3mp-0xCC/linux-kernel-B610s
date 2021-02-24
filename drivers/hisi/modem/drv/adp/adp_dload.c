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


#include "product_config.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wakelock.h>
#include "mdrv_sysboot.h"
#include "bsp_sram.h"
#include "bsp_version.h"
#include "../onoff/power_exchange.h"
#include "mdrv_version.h"

#include "mdrv_dload.h"

/*lint --e{715,818}*/

/*****************************************************************************
* 函 数 名  : mdrv_dload_set_curmode
*
* 功能描述  : 设置当前工作模式
*
* 输入参数  : void
* 输出参数  : DLOAD_MODE_NORMAL     :正常模式
*             DLOAD_MODE_DATA       :数据模式
*             DLOAD_MODE_DOWNLOAD   :下载模式
*
* 返 回 值  : 无
*
* 其它说明  : AT模块调用
*             正常模式支持的有:
*                 BSP_DLOAD_GetCurMode(DLOAD_MODE_DOWNLOAD)
*             下载模式支持的有:
*                 BSP_DLOAD_GetCurMode(DLOAD_MODE_DATA)
*
*****************************************************************************/
void mdrv_dload_set_curmode(DLOAD_MODE_E eDloadMode)
{
	return ;
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetCurMode
*
* 功能描述  : 获取当前工作模式
*
* 输入参数  : void
* 输出参数  : 无
*
* 返 回 值  : DLOAD_MODE_NORMAL     :正常模式
*             DLOAD_MODE_DATA       :数据模式
*             DLOAD_MODE_DOWNLOAD   :下载模式
*
* 其它说明  : 无
*
*****************************************************************************/
DLOAD_MODE_E mdrv_dload_get_curmode(void)
{
    return DLOAD_MODE_NORMAL;
}

/*****************************************************************************
* 函 数 名  : mdrv_dload_set_softload
*
* 功能描述  : 设置启动标志
*
* 输入参数  : BSP_BOOL bSoftLoad  :
*             BSP_FALSE :从vxWorks启动
*             BSP_TRUE  :从bootrom启动
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 其它说明  : 无
*
*****************************************************************************/
void mdrv_dload_set_softload (int bSoftLoad)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        ver_print_error("Dload smem_data malloc fail!\n");
        return;
    }
    
    if(BSP_TRUE == bSoftLoad)
    {
        smem_data->smem_dload_flag = SMEM_DLOAD_FLAG_NUM;
    }
    else
    {
        smem_data->smem_dload_flag = 0;
    }
    
    if(SMEM_SWITCH_PUCI_FLAG_NUM == smem_data->smem_switch_pcui_flag)
    {
        smem_data->smem_switch_pcui_flag = 0;
    }

    return;
}

/*****************************************************************************
* 函 数 名  : mdrv_dload_get_softload
*
* 功能描述  : 获取启动标志
*
* 输入参数  : void
* 输出参数  : 无
*
* 返 回 值  : BSP_TRUE  :从bootrom启动
*             BSP_FALSE :从vxWorks启动
*
* 其它说明  : 无
*
*****************************************************************************/
int mdrv_dload_get_softload (void)
{
	return 0;
}

/*****************************************************************************
* 函 数 名  : mdrv_dload_get_info
*
* 功能描述  : 返回升级版本信息
*
* 输入参数  : 无
* 输出参数  : 无
*
* 返 回 值  : 不为空 返回升级版本信息
                    为空 返回失败
*
* 其它说明  : 无
*
*****************************************************************************/
DLOAD_VER_INFO_S * mdrv_dload_get_info(void)
{
	return (void*)0;
}

/*****************************************************************************
* 函 数 名  : mdrv_dload_get_dloadver
*
* 功能描述  : 查询下载协议版本号。该信息BSP固定写为2.0。
*
* 输入参数  : char *str  :字符串缓冲区
*             unsigned int len      :字符串长度
* 输出参数  : char *str   :字符串缓冲区
*
* 返 回 值  : -1 -- 失败
*                   0 -- 成功
*
* 其它说明  : AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
int mdrv_dload_get_dloadver(char *str, unsigned int len)
{
    ver_print_info("getDloadVersion\r\n");

    if((NULL == str) || ((UINT32)len < sizeof(DLOAD_VERSION)))
    {
        ver_print_error("param error!\n");
        return VER_ERROR;
    }

    memset(str, 0x0, (UINT32)len);
    memcpy(str, DLOAD_VERSION, sizeof(DLOAD_VERSION));

    return VER_OK;
}

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
int mdrv_dload_getsoftver(char *str, unsigned int len)
{
    static bool b_geted = false;
    static char* software_ver = NULL;

    if((NULL == str) || (len == 0))
    {
        ver_print_error("param error!\n");
        return VER_ERROR;
    }

    if(!b_geted)
    {
        b_geted=true;
        software_ver = bsp_version_get_firmware();
    }

    if(!software_ver)
    {
        ver_print_error("get software version failed !\n");
        return VER_ERROR;
    }

    /*lint -save -e732 -e713*/
    strncpy(str,software_ver,len);
    /*lint -restore*/
    return VER_OK;

}

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
int mdrv_dload_getisover(char *pVersionInfo, unsigned int ulLength)
{
    static bool b_geted = false;
    static char iso_ver[VERSION_ISO_MAX_LEN];

    if((VERSION_ISO_MAX_LEN > ulLength) || (NULL == pVersionInfo))
    {
        ver_print_error("pVersionInfo = %d, ulLength = %d error.\n",(int)pVersionInfo,(int)ulLength);
        return VER_ERROR;
    }

    if(!b_geted)
    {
        b_geted = true;
        bsp_get_iso_version(iso_ver, VERSION_ISO_MAX_LEN);
    }

    if('\0' == iso_ver[0])
    {
        ver_print_error("get iso version fail.\n");
        return VER_ERROR;
    }

    /*lint -save -e732 -e713 */
    strncpy(pVersionInfo,iso_ver,StrParamType(ulLength));
    /*lint -restore*/
    return VER_OK;
}

/*****************************************************************************
* 函 数 名   : mdrv_dload_get_productname
* 功能描述   : 获取产品对外名称
* 输入参数   : char *pProductName  : 字符串缓冲区
*     unsigned int ulLength   : 字符串长度
* 输出参数   : char *pProductName    : 字符串缓冲区
* 返 回 值   : -1 -- 失败
                0 -- 成功
* 其它说明   :  AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
int mdrv_dload_get_productname(char * pProductName, unsigned int ulLength)
{
    static bool b_geted = false;
    static char* pout_name = NULL;

    if((BSP_NULL == pProductName) || (0 == ulLength))
    {
        return VER_ERROR;
    }

    if(!b_geted)
    {
        b_geted=true;
        pout_name = bsp_version_get_product_out_name();
    }

    if(!pout_name)
    {
        ver_print_error("get product out name failed !\n");
        return VER_ERROR;
    }

    /*lint -save -e732  -e713*/
    strncpy(pProductName, pout_name, StrParamType(ulLength));
    /*lint -restore*/
    return VER_OK;
}

/*****************************************************************************
* 函 数 名   : mdrv_dload_get_dloadtype
* 功能描述   : 获取升级模式
* 输入参数   : void
* 输出参数   : 无
* 返 回 值   : 0 -- 正常模式
               1 -- 升级模式
* 其它说明   :  AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
int mdrv_dload_get_dloadtype(void)
{
    if(DLOAD_MODE_NORMAL == mdrv_dload_get_curmode())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*****************************************************************************
* 函 数 名  : mdrv_dload_getwebuiver
*
* 功能描述  : 获取webui版本号字符串
*
* 输入参数  : char *str   :字符串缓冲区
*             unsigned int len   :字符串长度
* 输出参数  : char *str   :字符串缓冲区
*
* 返 回 值  : DLOAD_ERROR   :输入参数非法
*             其它          :返回字符串长度
*
* 其它说明  : AT模块调用
*             正常模式支持
*             下载模式支持
*
*****************************************************************************/
int mdrv_dload_getwebuiver(char *pVersionInfo,unsigned int ulLength)
{
    static char webui_ver[VERSION_WEBUI_MAX_LEN] = {0};

    if(VERSION_WEBUI_MAX_LEN > ulLength || NULL == pVersionInfo)
    {
        ver_print_error("pVersionInfo = %d, ulLength = %d error.\n",(int)pVersionInfo,(int)ulLength);
        return ERROR;
	}

    (void)bsp_get_web_version(webui_ver, VERSION_WEBUI_MAX_LEN);

    /*lint -save -e732 -e713 */
    strncpy(pVersionInfo, webui_ver, strlen(webui_ver));
    /*lint -restore*/
    return OK; 
}

/* set 2.5s delay before restart */
static struct delayed_work restart_process;
static bool restart_workqueue_create_flag = false;
static struct workqueue_struct* restart_workqueue = NULL;
/*wakelock超时锁,重启过程中防止单板休眠*/
static struct wake_lock dload_reset_lock;
#define HUAWEI_UPDATE_START_DELAY_TIME  2500
#define DLOAD_RESET_LOCK_TIMEOUT    10000

/*****************************************************************
* 函 数 名   : restart_func
* 功能描述   : MBB 调用海思重启接口，延时2.5s
* 输入参数   : 工作队列
* 输出参数   : 无
* 返回值   : void
******************************************************************/
static void restart_func(struct work_struct* data)
{
    /* 调用海思reboot接口 */
    printk(KERN_ERR "######ready to restart!######\n"); 
    mdrv_sysboot_restart();
    return;
}

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
void mdrv_dload_normal_reboot(void)
{
    power_on_wdt_cnt_set();

    /* set 2.5s delay before restart */
    if(false == restart_workqueue_create_flag)
    {
        /* 初始化wakelock锁 */
        wake_lock_init(&dload_reset_lock , WAKE_LOCK_SUSPEND, "dload_reset_lock");
        restart_workqueue = create_workqueue("huaweirestart");
        if(NULL == restart_workqueue)
        {
            printk(KERN_ERR "restart create_workqueue err and restart now!\n");
            mdrv_sysboot_restart();
            return;
        }
        INIT_DELAYED_WORK(&restart_process, restart_func);

        restart_workqueue_create_flag = true;
    }
    else
    {
        printk(KERN_ERR "restart create_workqueue have created!\n");
    }

    /*升级reset过程中不允许单板休眠,时间为10s*/
    wake_lock_timeout(&dload_reset_lock, (long)msecs_to_jiffies(DLOAD_RESET_LOCK_TIMEOUT));
    queue_delayed_work(restart_workqueue, &restart_process, 
                        msecs_to_jiffies(HUAWEI_UPDATE_START_DELAY_TIME));
    return;
}


