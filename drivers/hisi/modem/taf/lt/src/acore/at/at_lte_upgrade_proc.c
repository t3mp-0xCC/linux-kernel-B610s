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

/******************************************************************************
 *//* PROJECT   : MSP
 *//* SUBSYSTEM : MSP
 *//* MODULE    : AT
 *//* OWNER     :
 *//******************************************************************************
 */
/*lint --e{537,322,7,958,732,813,718,746,734}*/
#include "msp_upgrade_proc.h"
#include "at_lte_common.h"
#include "ATCmdProc.h"
#include "ScInterface.h"
#include "mdrv.h"
#include <osm.h>
/*lint -e767 原因:Log打印*/
#define    THIS_FILE_ID        MSP_FILE_ID_AT_LTE_UPGRADE_PROC_C
/*lint +e767 */
VOS_UINT32 g_atNvBackupFlag = 0;
#include "mbb_config.h"
#include "bsp_sram.h"
extern TAF_UINT32 At_SendCmdMsg (TAF_UINT8 ucIndex,TAF_UINT8* pData, TAF_UINT16 usLen,TAF_UINT8 ucType);


/*bootrom的版本查询*/
VOS_UINT32 atQryBootRomVer(VOS_UINT8 ucClientId)
{
    return AT_ERROR;
}

#include "msp_nv_id.h"
#include "drv_nv_id.h"



#define AT_NV_IMEI_LEN              15

VOS_UINT32 At_GetNvRevertState(VOS_VOID)
{
    VOS_UINT32 ret = (VOS_UINT32)-1;
    VOS_UINT16 resume_flag = 0;

    ret = NVM_Read(NV_ID_DRV_RESUME_FLAG,&resume_flag,sizeof(VOS_UINT16)); /*to do*/
    if(ret)
    {
        return ret;
    }
    if(0 != resume_flag)
    {
        return ((VOS_UINT32)-1);
    }
    return 0;
}

VOS_UINT32 At_GetNvAuthorityType(VOS_UINT32 * pdata)
{
    VOS_UINT32 ulRst;
    VOS_UINT8 *pucData = NULL;
    if(pdata==NULL)
    {
        return 0;
    }
    pucData = (VOS_UINT8 *)pdata;
    ulRst = NVM_Read(NV_ID_MSP_AUTHORITY_TYPE, pdata, sizeof(VOS_UINT32));

    if(ulRst != 0)
    {
        pucData[0] = 1;
        pucData[1] = 0;
        pucData[2] = 0;
        pucData[3] = 0;
        return ulRst;
    }

    return  0;
}

VOS_UINT32 At_GetNvAuthorityVer(VOS_UINT32 * pdata)
{
    VOS_UINT32 ulRst;

    if(pdata == NULL)
    {
        return ((VOS_UINT32)-1);
    }

    ulRst = NVM_Read(NV_ID_MSP_AUTHORITY_VER, pdata, sizeof(VOS_UINT32));

    if(ulRst != 0)
    {
        *pdata = 0;
    }

    return 0;
}



VOS_UINT32 At_GetImei(VOS_CHAR szimei [ 16 ])
{
    VOS_UINT32  ret;
    VOS_UINT32  uslen = 0;
    VOS_UINT32  subscript = 0;
    VOS_CHAR   checkdata = 0;
    VOS_CHAR   auctemp[AT_NV_IMEI_LEN+1] = {0};

    uslen = AT_NV_IMEI_LEN+1;

    ret = NVM_Read(0, auctemp, uslen);

    if(ret != 0)
    {
        return ret;
    }
    else
    {
        for (subscript = 0; subscript < (AT_NV_IMEI_LEN - 1); subscript += 2)
        {
            checkdata += (VOS_CHAR)(((auctemp[subscript])
                           +((auctemp[subscript + 1] * 2) / 10))
                           +((auctemp[subscript + 1] * 2) % 10));
        }
        checkdata = (10 - (checkdata%10)) % 10;

        for (subscript = 0; subscript < uslen; subscript++)
        {
            *(szimei + subscript) = *(auctemp + subscript) + 0x30; /*字符转换*/
        }

        szimei[AT_NV_IMEI_LEN - 1] = checkdata + 0x30;
        szimei[AT_NV_IMEI_LEN] = 0;
    }

    return 0;
}

VOS_UINT32 atQryDLoadVer(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    VOS_UINT8* pData = NULL;
    gstAtSendData.usBufLen = 0;

    pData = (VOS_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, (DYNAMIC_MEM_PT), UPGRADE_STR_BUF_SIZE);
    if(NULL == pData)
    {
        CmdErrProc(ucClientId, ERR_MSP_MALLOC_FAILUE, 0, NULL);
        return AT_ERROR;
    }

    MSP_MEMSET(pData,0,UPGRADE_STR_BUF_SIZE);
    ulRst = mdrv_dload_get_dloadver((char *)pData,UPGRADE_STR_BUF_SIZE);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        VOS_MemFree(WUEPS_PID_AT, pData);
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "%s",
                                           pData
                                         );
    
    VOS_MemFree(WUEPS_PID_AT, pData);

    return AT_OK;
}

#define UPDATE_INFO_BUF_SIZE        2048
#define VER_TIME_LEN                32
#define MIN_RECORD_TIMES            1
#define AT_SUP_PARA_NUM             3
#define MAX_RECORD_TIMES            10
/* the struct is used to save the upinfo from onekey tool */
typedef struct _rec_tool_upinfo_
{
    VOS_UINT8 time_info[VER_TIME_LEN];  
    VOS_UINT8 ver_info[VER_TIME_LEN];
    VOS_UINT32 tooltype;   
}update_info_st;

/* the AT is used to save the upinfo from onekey tool, 
    such as "AT^SUPINFO ="2014/03/17 19:21:33","11.220.00.00.168",1" */
VOS_UINT32 atSetUpInfo(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    update_info_st upinfo = {{0}, {0}, 0};
    gstAtSendData.usBufLen = 0;
    
    if(AT_SUP_PARA_NUM != gucAtParaIndex)
    {
        (VOS_VOID)vos_printf(" AtSetupinfo para num err = %d\n", gucAtParaIndex);
        return AT_ERROR;
    }
    if((gastAtParaList[0].usParaLen > VER_TIME_LEN - 1) 
        || (gastAtParaList[1].usParaLen > VER_TIME_LEN - 1))
    {
        (VOS_VOID)vos_printf("AtSetupinfo para length err\n");
        return AT_ERROR;        
    }

    MSP_MEMCPY((VOS_CHAR *)upinfo.time_info, gastAtParaList[0].aucPara, gastAtParaList[0].usParaLen);
    MSP_MEMCPY((VOS_CHAR *)upinfo.ver_info, gastAtParaList[1].aucPara, gastAtParaList[1].usParaLen);
    upinfo.tooltype = gastAtParaList[2].ulParaValue;
    (VOS_VOID)vos_printf("update info: %s,%s,%d\n", upinfo.time_info, upinfo.ver_info, upinfo.tooltype);
    /* set upinfo flag */
    ulRst = huawei_set_upinfo((VOS_CHAR *)&upinfo);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        (VOS_VOID)vos_printf(" AtSetupinfo set upinfo err\n");
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }

    return AT_OK;
}

/* the AT is used to get the upinfo times in the current board, 
    such as "AT^CHKUPINFO?" */
VOS_UINT32 atQryUpInfo(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    unsigned int upinfo_times = 0;
    gstAtSendData.usBufLen = 0;
    
    (void)huawei_get_upinfo_times(&upinfo_times);
    
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,(VOS_CHAR* )(pgucAtSndCodeAddr), 
        (VOS_CHAR* )(pgucAtSndCodeAddr + gstAtSendData.usBufLen), "%d", upinfo_times);
    return ulRst;

}

/* the AT is used to get the upinfo of the spec times in the current board, 
    for example "AT^CHKUPINFO=5" */
VOS_UINT32 atSetUpInfoNum(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    VOS_UINT8* pData = NULL;
    VOS_UINT32 upinfo_num = 0;
    gstAtSendData.usBufLen = 0;

    if(1 != gucAtParaIndex)
    {
        (VOS_VOID)vos_printf("atSetUpInfoNum para num err = %d\n", gucAtParaIndex);
        return AT_ERROR;
    }
    if(gastAtParaList[0].ulParaValue > MAX_RECORD_TIMES || gastAtParaList[0].ulParaValue < MIN_RECORD_TIMES)
    {
        (VOS_VOID)vos_printf("atSetUpInfoNum ulParaValue err = %d\n", gastAtParaList[0].usParaLen);
        return AT_ERROR;
    }

    upinfo_num = gastAtParaList[0].ulParaValue;
    (VOS_VOID)vos_printf(" atSetUpInfoNum num is  = %d \n", upinfo_num);
    
    pData = VOS_MemAlloc(WUEPS_PID_AT, (DYNAMIC_MEM_PT), UPDATE_INFO_BUF_SIZE);
    if(NULL == pData)
    {
        CmdErrProc(ucClientId, ERR_MSP_MALLOC_FAILUE, 0, NULL);
        return AT_ERROR;
    }
    MSP_MEMSET(pData,0, UPDATE_INFO_BUF_SIZE);
    
    ulRst = huawei_get_spec_upinfo(pData, UPDATE_INFO_BUF_SIZE, upinfo_num);
    if(ulRst != AT_SUCCESS)
    {
        VOS_MemFree(WUEPS_PID_AT, pData);
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,(VOS_CHAR* )(pgucAtSndCodeAddr), 
        (VOS_CHAR* )(pgucAtSndCodeAddr + gstAtSendData.usBufLen), "%s", pData);
    
    VOS_MemFree(WUEPS_PID_AT, pData);
    return AT_OK;    
}

/*本命令用于at^dloadinfo=?的测试命令*/
VOS_UINT32 atQryDloadInfoTestOk(VOS_UINT8 ucClientId)
{
    return AT_OK;
}

/*本命令用户查询单板信息，用于返回单板和后台版本号、产品型号名称、下载类型信息*/
VOS_UINT32 atQryDLoadInfo(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    VOS_UINT8* pData = NULL;
    const char* web_version = "WEBUI_00.00.00.000.03";
    gstAtSendData.usBufLen = 0;

    pData = (VOS_UINT8 *)VOS_MemAlloc(WUEPS_PID_AT, (DYNAMIC_MEM_PT), UPGRADE_STR_BUF_SIZE);
    if(NULL == pData)
    {
        CmdErrProc(ucClientId, ERR_MSP_MALLOC_FAILUE, 0, NULL);
        return AT_ERROR;
    }

    /*获取软件版本号字符串*/
    MSP_MEMSET(pData,0, UPGRADE_STR_BUF_SIZE);
    ulRst = mdrv_dload_getsoftver((char *)pData, UPGRADE_STR_BUF_SIZE);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        VOS_MemFree(WUEPS_PID_AT, pData);
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)(pgucAtSndCodeAddr),
                                            (VOS_CHAR*)(pgucAtSndCodeAddr + gstAtSendData.usBufLen),
                                            "swver:%s\r\n",
                                            pData
                                          );

    /*后台版本号字符串 */
    MSP_MEMSET(pData,0, UPGRADE_STR_BUF_SIZE);
    ulRst = mdrv_dload_getisover((char *)pData, UPGRADE_STR_BUF_SIZE);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        VOS_MemFree(WUEPS_PID_AT, pData);
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }

    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)(pgucAtSndCodeAddr),
                                            (VOS_CHAR*)(pgucAtSndCodeAddr+ gstAtSendData.usBufLen),
                                            "\r\nisover:%s\r\n",
                                            pData
                                          );

    /*webui版本号字符串, 默认MBB_DLOAD宏开启后，支持webui字段上报，不合入安全升级v1.3时，此字段返回为空 */
    MSP_MEMSET(pData,0, UPGRADE_STR_BUF_SIZE);
    ulRst = mdrv_dload_getwebuiver((char *)pData,UPGRADE_STR_BUF_SIZE);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        VOS_MemFree(WUEPS_PID_AT, pData);
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }

    if(!strncmp(pData, web_version, strlen(web_version)))
    {
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)(pgucAtSndCodeAddr),
                                                (VOS_CHAR*)(pgucAtSndCodeAddr+ gstAtSendData.usBufLen),
                                                "\r\nwebuiver:\r\n"
                                              );
    }
    else
    {
        gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)(pgucAtSndCodeAddr),
                                                (VOS_CHAR*)(pgucAtSndCodeAddr+ gstAtSendData.usBufLen),
                                                "\r\nwebuiver:%s\r\n",
                                                pData
                                              );
    }
    /*产品名称字符串*/
    MSP_MEMSET(pData,0, UPGRADE_STR_BUF_SIZE);
    ulRst = mdrv_dload_get_productname((char *)pData, UPGRADE_STR_BUF_SIZE);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        VOS_MemFree(WUEPS_PID_AT, pData);
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }
    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)(pgucAtSndCodeAddr),
                                            (VOS_CHAR*)(pgucAtSndCodeAddr+ gstAtSendData.usBufLen),
                                            "\r\nproduct name:%s\r\n",
                                            pData
                                          );
    /*获取下载类型*/
    MSP_MEMSET(pData,0, UPGRADE_STR_BUF_SIZE);
    ulRst = mdrv_dload_get_dloadtype();
    if((ulRst != 0)&&(ulRst != 1))
    {
        VOS_MemFree(WUEPS_PID_AT, pData);
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }
    *pData = ulRst + '0';
    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)(pgucAtSndCodeAddr),
                                            (VOS_CHAR*)(pgucAtSndCodeAddr+ gstAtSendData.usBufLen),
                                            "\r\ndload type:%s",
                                            pData
                                          );
    VOS_MemFree(WUEPS_PID_AT, pData);

    return AT_OK;
}

/*NV备份命令*/
VOS_UINT32 atSetNVBackup(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    gstAtSendData.usBufLen = 0;
    /* 参数检查 ,无参数*/
    if(AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    ulRst = NVM_UpgradeBackup(EN_NVM_BACKUP_FILE);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }
    g_atNvBackupFlag ++;
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "%d",
                                           ulRst
                                         );
    return AT_OK;
}

/*NV恢复命令*/
VOS_UINT32 atSetNVRestore(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    gstAtSendData.usBufLen = 0;
        /* 参数检查 ,无参数*/
    if(AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    ulRst = NVM_UpgradeRestore();
    if(ulRst != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "%d",
                                           ulRst
                                         );

    return AT_OK;
}

VOS_UINT32 atQryAuthorityVer(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    VOS_UINT32 ulAuthorityVer = 0;


    ulRst = At_GetNvAuthorityVer(&ulAuthorityVer);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }
 
    ulAuthorityVer = 2;




    gstAtSendData.usBufLen = 0;
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                                (VOS_CHAR*)pgucAtSndCodeAddr,
                                                "%d",
                                                ulAuthorityVer
                                                );
    return AT_OK;
}

VOS_UINT32 atQryAuthorityID(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst;
    VOS_UINT8 pData[NV_ITEM_IMEI_SIZE] = {0};
    gstAtSendData.usBufLen = 0;


    ulRst = At_GetImei((VOS_CHAR *)pData);
    if(ulRst != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "%s,0", //目前默认通过imei 号鉴权，如果有需要根据海思的机制再做修改
                                           pData
                                         );
    return AT_OK;
}

//切换到下载模式命令单板重启后将进入下载模式
VOS_UINT32 atSetGodLoad(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    /* 参数检查 ,无参数*/
    if(AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    mdrv_dload_set_curmode(DLOAD_MODE_DOWNLOAD);
    mdrv_dload_set_softload(true);    /* 设置共享内存魔术字，下次进入下载模式 */

    /* 模拟at^cfun=4的处理 */
    ulRst = At_SendCmdMsg (AT_CLIENT_TAB_APP_INDEX ,(unsigned char*)"at+cfun=4\r\n", strlen("at+cfun=4\r\n"), 0);
    (VOS_VOID)vos_printf("######godload restart notice protocal######, %d\n", ulRst);
    mdrv_dload_normal_reboot();

    return AT_OK;
}

// *****************************************************************************
// 函数名称: atSetReset
// 功能描述: 单板重启命令 "^RESET"
//
// 参数说明:
//   ulIndex [in] 用户索引
//
// 返 回 值:
//    TODO: ...
//
// 调用要求: TODO: ...
// 调用举例: TODO: ...

//  2.日    期   : 2015年01月26日

// *****************************************************************************
VOS_UINT32 atSetReset(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    /* 参数检查 ,无参数*/
    if(AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    if (NULL == smem_data)
    {
        ulRst = AT_ERROR;
        return ulRst;
    }
    if(SMEM_SWITCH_PUCI_FLAG_NUM == smem_data->smem_switch_pcui_flag)
    {
        smem_data->smem_switch_pcui_flag = 0;
    }
    ulRst = At_SendCmdMsg (AT_CLIENT_TAB_APP_INDEX ,(unsigned char*)"at+cfun=4\r\n", strlen("at+cfun=4\r\n"), 0);

    (VOS_VOID)vos_printf("######restart notice protocal######, %d\n", ulRst);

    mdrv_dload_normal_reboot();

    return AT_OK;
}

//自动恢复状态查询
VOS_UINT32 atSetNVRstSTTS(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;
    gstAtSendData.usBufLen = 0;
        /* 参数检查 ,无参数*/
        if(AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
        {
            return AT_ERROR;
        }
    ulRst = At_GetNvRevertState();
    if(ulRst != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "%d",
                                           ulRst
                                         );
    return AT_OK;

}

VOS_UINT32 atSetNVFactoryRestore(VOS_UINT8 ucClientId)
{
    VOS_UINT32 ulRst = AT_OK;

    /*因为MODEN拨号会下发该命令，所以在moden口不进行NV恢复，直接返回OK*/
    if ( AT_MODEM_USER == gastAtClientTab[ucClientId].UserType )
    {
        return AT_OK;
    }

    ulRst = NVM_RevertFNV();
    if(ulRst != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, ulRst, 0, NULL);
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                           (VOS_CHAR *)pgucAtSndCodeAddr,
                                           (VOS_CHAR*)pgucAtSndCodeAddr,
                                           "\r%d\r",
                                           ulRst
                                         );
    /* E5通知APP恢复用户设置  */
    AT_PhSendRestoreFactParm();
										 
    return AT_OK;
}


VOS_UINT32 atSetNVFactoryBack(VOS_UINT8 ucClientId)
{

    VOS_UINT32                          ulRst;
    if (AT_CMD_OPT_SET_CMD_NO_PARA != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    ulRst = NVM_BackUpFNV();
    if(ulRst != ERR_MSP_SUCCESS)
    {
        printk("atSetNVFactoryBack:nv Backup faile! %d", ulRst);

        CmdErrProc(ucClientId, ulRst, 0, NULL);

        return AT_ERROR;
    }

    ulRst = SC_COMM_Backup();
    if(ulRst != ERR_MSP_SUCCESS)
    {
        printk("atSetNVFactoryBack:SC Backup faile! %d", ulRst);

        CmdErrProc(ucClientId, ulRst, 0, NULL);

        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32  AT_SetLteSdloadPara(VOS_UINT8 ucClientId)
{
    FTM_RD_SDLOAD_REQ_STRU stSdloadSetReq = {0};
    VOS_UINT32 ulRst;

    ulRst = atSendFtmDataMsg(MSP_SYS_FTM_PID, ID_MSG_FTM_SET_SDLOAD_REQ, ucClientId, (VOS_VOID*)(&stSdloadSetReq),
        sizeof(stSdloadSetReq));

    if(AT_SUCCESS == ulRst)
    {
        gastAtClientTab[ucClientId].CmdCurrentOpt = AT_CMD_SDLOAD_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

/* BEGIN: To_Be_Deleted_V7R2 */
VOS_UINT32 atQryBootRomVerCnf(VOS_UINT8 ucClientId,VOS_VOID *pMsgBlock)
{
    FTM_RD_BOOTROMVER_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_RD_BOOTROMVER_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
	{
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
		return AT_OK;
	}

    if(pstCnf->ulErrCode != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
		return AT_OK;
    }
    else
    {
        gstLAtSendData.usBufLen = (VOS_UINT16)At_sprintf(
                                                    AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)(pgucLAtSndCodeAddr),
                                                    (VOS_CHAR *)(pgucLAtSndCodeAddr + gstLAtSendData.usBufLen),
                                                    "\r\n%s\r\n",
                                                    pstCnf->szSendStr
                                                    );

        CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);
    }
    return AT_OK;
}
VOS_UINT32 atQryDLoadVerCnf(VOS_UINT8 ucClientId,VOS_VOID *pMsgBlock)
{
    FTM_RD_DLOADVER_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_RD_DLOADVER_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
	{
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
		return AT_OK;
	}

    if(pstCnf->ulErrCode != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
		return AT_OK;
    }
    else
    {
        gstLAtSendData.usBufLen = (VOS_UINT16)At_sprintf(
              AT_CMD_MAX_LEN,
              (VOS_CHAR *)(pgucLAtSndCodeAddr),
              (VOS_CHAR *)(pgucLAtSndCodeAddr + gstLAtSendData.usBufLen),
              "\r\n%s\r\n",
              pstCnf->cVer
              );

	    CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);
    }

    return AT_OK;
}

VOS_UINT32 atQryDLoadInfoCnf(VOS_UINT8 ucClientId, VOS_VOID *pMsgBlock)
{
    FTM_RD_DLOADINFO_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_RD_DLOADINFO_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
	{
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
		return AT_OK;
	}
    if(pstCnf->ulErrCode != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
		return AT_OK;
    }
    else
    {
        gstLAtSendData.usBufLen = 0;
        gstLAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)(pgucLAtSndCodeAddr),
                                                (VOS_CHAR*)(pgucLAtSndCodeAddr + gstLAtSendData.usBufLen),
                                                "\r\nswver:%s\r\n",
                                                pstCnf->szSendSW
                                              );

        gstLAtSendData.usBufLen += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)(pgucLAtSndCodeAddr),
                                                (VOS_CHAR*)(pgucLAtSndCodeAddr+ gstLAtSendData.usBufLen),
                                                "\r\nisover:%s\r\n",
                                                pstCnf->szSendISO
                                              );
        gstLAtSendData.usBufLen += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)(pgucLAtSndCodeAddr),
                                                (VOS_CHAR*)(pgucLAtSndCodeAddr+ gstLAtSendData.usBufLen),
                                                "\r\nproduct name:%s\r\n",
                                                pstCnf->szSendProductID
                                              );
        gstLAtSendData.usBufLen += (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                (VOS_CHAR *)(pgucLAtSndCodeAddr),
                                                (VOS_CHAR*)(pgucLAtSndCodeAddr+ gstLAtSendData.usBufLen),
                                                "\r\ndload type:%s\r\n",
                                                pstCnf->szSendDloadType
                                              );
        CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);
    }
    return AT_OK;
}

VOS_UINT32 atSetNVBackupCnf(VOS_UINT8 ucClientId,VOS_VOID *pMsgBlock)
{
    FTM_SET_NVBACKUP_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_SET_NVBACKUP_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
	{
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
		return AT_OK;
	}
    if(pstCnf->ulErrCode != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
		return AT_OK;
    }
    else
    {
        g_atNvBackupFlag++;

        gstLAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucLAtSndCodeAddr,
                                               (VOS_CHAR*)pgucLAtSndCodeAddr,
                                               "\r\n%d\r\n",
                                               pstCnf->ulRetValue
                                             );

        CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);
    }
    return AT_OK;
}

VOS_UINT32 atSetNVRestoreCnf(VOS_UINT8 ucClientId,VOS_VOID *pMsgBlock)
{
    FTM_SET_NVRESTORE_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_SET_NVRESTORE_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
	{
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
		return AT_OK;
	}
    if(pstCnf->ulErrCode != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
		return AT_OK;
    }
    else
    {
        gstLAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucLAtSndCodeAddr,
                                               (VOS_CHAR*)pgucLAtSndCodeAddr,
                                               "\r\n%d\r\n",
                                               pstCnf->ulRetValue
                                             );

        CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);
    }
    return AT_OK;
}

VOS_UINT32 atQryAuthorityVerCnf(VOS_UINT8 ucClientId,VOS_VOID *pMsgBlock)
{
    FTM_RD_AUTHORITYVER_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_RD_AUTHORITYVER_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
	{
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
		return AT_OK;
	}
    if(pstCnf->ulErrCode != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
		return AT_OK;
    }
    else
    {
        gstLAtSendData.usBufLen = 0;
        gstLAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                                    (VOS_CHAR *)pgucLAtSndCodeAddr,
                                                    (VOS_CHAR*)pgucLAtSndCodeAddr,
                                                    "\r\n%s\r\n",
                                                    pstCnf->szAuthorityVer
                                                    );
        CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);
    }
    return AT_OK;
}

VOS_UINT32 atQryAuthorityIDCnf(VOS_UINT8 ucClientId,VOS_VOID *pMsgBlock)
{
    FTM_RD_AUTHORITYID_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;

    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_RD_AUTHORITYID_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
	{
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
		return AT_OK;
	}
    if(pstCnf->ulErrCode != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
		return AT_OK;
    }
    else
    {
        gstLAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR*)pgucLAtSndCodeAddr,
                                               (VOS_CHAR*)pgucLAtSndCodeAddr,
                                               "\r\n%s,%s\r\n",
                                               pstCnf->szAuthorityID,pstCnf->szAuthorityType
                                             );

	    CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);
    }
    return AT_OK;
}

VOS_UINT32 atSetNVRstSTTSCnf(VOS_UINT8 ucClientId,VOS_VOID *pMsgBlock)
{
    FTM_SET_NVRSTSTTS_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;
/*  VOS_BOOL bRet = TRUE;
 */
    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_SET_NVRSTSTTS_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
	{
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
		return AT_OK;
	}

    if(pstCnf->ulErrCode != ERR_MSP_SUCCESS)
    {
        CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);
		return AT_OK;
    }
    else
    {
        gstLAtSendData.usBufLen = (VOS_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                               (VOS_CHAR *)pgucLAtSndCodeAddr,
                                               (VOS_CHAR*)pgucLAtSndCodeAddr,
                                               "\r\n%s\r\n",
                                               pstCnf->szNVRst
                                             );
        CmdErrProc(ucClientId, pstCnf->ulErrCode, gstLAtSendData.usBufLen, pgucLAtSndCodeAddr);
    }
    return AT_OK;
}

VOS_UINT32 atSetSdloadCnf(VOS_UINT8 ucClientId,VOS_VOID *pMsgBlock)
{
    FTM_SET_SDLOAD_CNF_STRU *pstCnf = NULL;
    OS_MSG_STRU*pEvent = NULL;
    gstLAtSendData.usBufLen = 0;
/*  VOS_BOOL bRet = TRUE;
 */
    pEvent = (OS_MSG_STRU*)(((MsgBlock*)pMsgBlock)->aucValue);
	pstCnf = (FTM_SET_SDLOAD_CNF_STRU *)pEvent->ulParam1;

    if(NULL == pstCnf)
    {
    	CmdErrProc(ucClientId, ERR_MSP_FAILURE, 0, NULL);
    	return AT_OK;
    }

    CmdErrProc(ucClientId, pstCnf->ulErrCode, 0, NULL);

    return AT_OK;
}
/* END: To_Be_Deleted_V7R2 */

