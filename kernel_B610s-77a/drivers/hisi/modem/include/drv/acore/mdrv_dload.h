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


/* 下载模式枚举 */
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
void mdrv_dload_set_curmode(DLOAD_MODE_E eDloadMode);
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
DLOAD_MODE_E mdrv_dload_get_curmode(void);
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
void mdrv_dload_set_softload (int bSoftLoad);


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
int mdrv_dload_get_softload (void);    

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
DLOAD_VER_INFO_S * mdrv_dload_get_info(void);

#if (FEATURE_ON == MBB_DLOAD)
/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetWebuiVer
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
int mdrv_dload_getwebuiver(char *pVersionInfo,unsigned int ulLength);
/*****************************************************************************
* 函 数 名  : mdrv_dload_get_dloadver
* 功能描述  : 查询下载协议版本号。该信息BSP固定写为2.0。
* 输入参数  : char *str  :字符串缓冲区
*             unsigned int len      :字符串长度
* 输出参数  : char *str   :字符串缓冲区
* 返 回 值  : -1 -- 失败
*                   0 -- 成功
* 其它说明  : AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
int mdrv_dload_get_dloadver(char *str, unsigned int len);

/*****************************************************************************
* 函 数 名   : mdrv_dload_getsoftver
* 功能描述   : 获取软件版本号
* 输入参数   : BSP_S8 *str   : 字符串缓冲区
*     BSP_S32 len   : 字符串长度
* 输出参数  : BSP_S8 *str   : 字符串缓冲区
* 返 回 值  : -1 -- 失败
             0 -- 成功
* 其它说明   :  AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
int mdrv_dload_getsoftver(char *str, unsigned int len);

/*****************************************************************************
* 函 数 名   : mdrv_dload_getisover
* 功能描述   : 获取后台版本号
* 输入参数   : char *pVersionInfo   : 字符串缓冲区
*     unsigned int ulLength   : 字符串长度
* 输出参数   : char *pVersionInfo   : 字符串缓冲区
* 返 回 值   : -1 -- 失败
                  0 -- 成功
* 其它说明   :  AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
int mdrv_dload_getisover(char *pVersionInfo, unsigned int ulLength);

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
int mdrv_dload_get_productname(char * pProductName, unsigned int ulLength);

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
int mdrv_dload_get_dloadtype(void);

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
void mdrv_dload_normal_reboot(void);

#endif

#ifdef _cplusplus
}
#endif
#endif

