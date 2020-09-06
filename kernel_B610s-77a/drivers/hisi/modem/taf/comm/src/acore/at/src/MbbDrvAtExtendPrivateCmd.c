

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <linux/syscalls.h>
#include <linux/statfs.h>
#include "AtTestParaCmd.h"
#include "ATCmdProc.h"

#include "product_nv_id.h"
#include "product_nv_def.h"
#include "bsp_version.h"
#include "mdrv_version.h"
#include "mdrv_chg.h"
#include "LPsNvInterface.h"


#include "mdrv_rfile_common.h"

/*lint -e767 -e960 修改人:罗建 107747;检视人:孙少华65952;原因:Log打印*/
#define    THIS_FILE_ID        PS_FILE_ID_AT_EXTENDPRIVATECMD_C
/*lint +e767 +e960 修改人:罗建 107747;检视人:sunshaohua*/



/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
/*测试项定义*/
char *hw_test_option[] =
{
    "ddr",
    "flash",
    "peripheral",
    "all",
     NULL
};

#define HW_TEST_DEV_NAME "/dev/hardwaretest"    /*硬件自检驱动节点*/
#define HARDWARE_TEST_MODE_SET  (3)
#define HW_DATA_LENGTH    (50)

typedef  struct
{
    unsigned int smem_hw_mode;             /* 自检模式*/
    unsigned int smem_hw_option;           /* 测试项选择*/
    unsigned int smem_reserved;            /* 保留*/
}at_hwtest_info;

#define AT_RSFR_RSFW_DATA_LEN   AT_MAX_CMD_LEN_FOR_M2M
#define OFFLINE_LOG_REF_0              201
#define OFFLINE_LOG_REF_1              202
#define UPDATE_LOG_REF_0              301
#define UPDATE_LOG_REF_1              302
#define SDT_FILE_PATH                  "/data/log/modemConfig/hids_ue_msg.cfg"
#define DUMP_FILE_PATH                 "/modem_log/Exc/"
#define OFFLINE_LOG_CNAME_DUMP          1  //dump
#define OFFLINE_LOG_CNAME_UPDATE      2 //flash log
#define OFFLINE_LOG_REF_BASE       203

#define OM_SD_LOG_PATH_MAX_LENGTH               (128)
#define OM_VENDOR_LOG_MAX_SIZE         (128 * 1024)

#define OM_FLASH_LOG_ROOT_PATH         "/online/log/balonglte"

#define OM_SD_LOG_SET_INFO_FILE         "/online/log/offlinelog.conf"

VOS_UINT32 g_EntryFlag = 0;
VOS_UINT32 g_usLength = 0;
VOS_UINT32 g_onlyonce = 0;
VOS_UINT32 g_usSeekLength = 0;
VOS_INT32 g_usCurrentSwitch = -1;
VOS_UINT32 g_packagenum = 0;
VOS_UINT32 g_packageitem = 0;
VOS_UINT32 g_SdataPackageCount = 0;
OM_FS_READ_CTRL_INFO                g_stOmFSReadCtrlInfo;
OM_FLASH_INFO_STRU              g_stFSFileInfo;
/*MBB新增的离线log控制信息*/
OM_SD_LOG_SET_INFO                g_stOmSDLogSetInfo;

/*文件打开类型*/
#define OM_SD_FILE_MODE                         (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)

VOS_UINT32 OM_FSReadData(
                  VOS_UINT8 *pucData, 
                  VOS_UINT32 *pulLen, 
                  VOS_UINT32 *FsTotalPack, 
                  VOS_UINT32 *FsCurPack,
                  VOS_UINT32 FileIndex,
                  VOS_UINT32 ucSubName);
VOS_UINT32 OM_FSgetLogFilePath(VOS_UINT32 ulIndex,VOS_CHAR *Path);
VOS_UINT32 OM_GetVendorLogFilePath
(
    VOS_UINT32 ulIndex,
    VOS_CHAR *Path, 
    VOS_UINT32 ucSubName
);

VOS_CHAR g_dumpfiname[2][256] = {"\0","\0"};   /* 保存dump的路径名称 */







/*****************************************************************************
  3 函数定义
*****************************************************************************/
/*lint -e64*/
/*****************************************************************************
 函 数 名  : At_TestMdonPara
 功能描述  : ^MDON测试命令
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :

 修改历史      :
*****************************************************************************/
VOS_UINT32 At_TestMdonPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (TAF_UINT16)VOS_sprintf((TAF_CHAR*)pgucAtSndCodeAddr,
                                "%s:(0-5)",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}
/*lint +e64*/


VOS_UINT32 At_TestMdslPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (TAF_UINT16)VOS_sprintf((TAF_CHAR*)pgucAtSndCodeAddr,
                                "%s:(0,1)",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}


VOS_UINT32 At_TestMdsfPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (TAF_UINT16)VOS_sprintf((TAF_CHAR*)pgucAtSndCodeAddr,
                                "%s:(0,1)",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName);

    return AT_OK;
}
/*lint +e64*/

/*****************************************************************************
 函 数 名  :  AT_TestSetMbimMode
 功能描述  : ^SETMODE命令的测试命令
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 AT_TestSetMbimMode(VOS_UINT8 ucIndex)
{
      /* 通道检查 */
    if (VOS_FALSE == AT_CheckHsicUser(ucIndex))
    {
        return AT_ERROR;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       (VOS_CHAR *)pgucAtSndCodeAddr,
                                       "%s: (0,1)",
                                       g_stParseContext[ucIndex].pstCmdElement->pszCmdName);
    return AT_OK;
}








#define AT_SPEED_NAME_LEN              (16)

VOS_UINT32 At_QrySpeed(VOS_UINT8 ucIndex)
{
    VOS_UINT32          ret;
    VOS_UCHAR            usbspeed[AT_SPEED_NAME_LEN]={'\0'};
    ret = DRV_GET_USB_SPEED(usbspeed);
    printk(KERN_ERR"%s\n",usbspeed);
    if(ret)
    {
        return AT_ERROR;
    }
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            (TAF_CHAR *)pgucAtSndCodeAddr,
                                            "^USBSPEED: %s", usbspeed);
    return AT_OK;
}




/*lint -e64 -e119*/
/*****************************************************************************
 函 数 名  : AT_QryMdonPara
 功能描述  : (AT^MDON)查询当前保存LOG的状态
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
*****************************************************************************/
VOS_UINT32 AT_QryMdonPara(VOS_UINT8 ucIndex)
{
    VOS_INT32 ulRet = 0;
    VOS_UINT32 ustate = 0; /*当前状态*/
    VOS_UINT8  Flag = 0;
    VOS_UINT8 ucmlogflag = 0; /*mlog标识*/
    ulRet = mlog_state_get(&ucmlogflag);
    if(VOS_OK != ulRet)
    {
        return AT_ERROR;
    }
    /*判断支持情况*/
    if(0 == ucmlogflag)
    {
        /*功能关闭*/
        ustate = 0;
    }
    else if(1 == ucmlogflag)
    {
        /*mlog功能开启*/
        ustate = 5;
    }
    else
    {
        return AT_ERROR;
    }
    ulRet = OM_SDGetStorageOrFlipFlag(2, &Flag);  /* 获取离线log开关 */
    if (VOS_OK != ulRet)
    {
        return AT_ERROR; 
    }
    if (1 == Flag)
    {
        ustate = 1;
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s:%d", g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            ustate);

    return AT_OK;
}
/*lint +e64 +e119*/



VOS_UINT32 AT_QryMdslPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32  ulRet = 0;
    VOS_UINT8  Flag = 0;

    ulRet = OM_SDGetStorageOrFlipFlag(0, &Flag);
    if (VOS_OK != ulRet)
    {
        return AT_ERROR; 
    }

    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s:%d", g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            Flag);

    return AT_OK;
}



VOS_UINT32 AT_QryMdStoragePara(VOS_UINT8 ucIndex)
{
    VOS_UINT32                          ucState     = 0;
    VOS_INT32                           ucNandState     = -1;
    VOS_INT32                           ucSdVolume = 0;

    /*获取FS 存储空间*/
    ucState = OM_FSGetFreeStorage( &ucNandState );
    if ( VOS_OK != ucState )
    {
        ucNandState = -1;
    }

    //获取SD卡存储空间
    /*ucState = OM_SDGetFreeStorage( &ucSdVolume );
    if ( VOS_OK != ucState )
    {
        ucSdVolume = -1;
    }*/
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s:%d,%d", g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            ucNandState,
                                            ucSdVolume);

    return AT_OK;
}



VOS_UINT32 AT_QryMdsfPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32  ulRet = 0;
    VOS_UINT8   Flag = 0;

    ulRet = OM_SDGetStorageOrFlipFlag(1, &Flag);
    if (VOS_OK != ulRet)
    {
        return AT_ERROR; 
    }
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                            (VOS_CHAR *)pgucAtSndCodeAddr,
                                            (VOS_CHAR*)pgucAtSndCodeAddr,
                                            "%s:%d", g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                            Flag);

    return AT_OK;
}


VOS_UINT32 OM_FSGetFreeStorage(VOS_INT32 *vol)
{
    VOS_UINT32 ulfreespace = 0;
    mm_segment_t sd_fs = 0; /*lint !e10 !e522*/
    struct statfs disk_info = {0}; /*lint !e121 !e129 !e529 !e438 !e533*/
    VOS_UINT32 ulMaxSpace = 32 * 1024;

    sd_fs = get_fs(); /*lint !e10 !e129*/
    set_fs(KERNEL_DS);  /*lint !e119 !e132*/

    /*若执行失败则返回error*/
    if (0  != sys_statfs("/online",&disk_info)) /*lint !e2 !e10 !e102*/
    {
        vos_printf("sys_statfs execute fail! \n");
        set_fs(sd_fs);
        return VOS_ERR;
    }

    ulfreespace = (VOS_UINT32)(disk_info.f_bfree * disk_info.f_bsize) / 1024; /*Byte shift to KB*/ /*lint !e10 !e101 !e129*/
    ulfreespace = ulfreespace * 9 / 10;/*保留10%*/ /*lint !e10 !e129*/
    vos_printf( "ulfreespace = %llu \n",ulfreespace); /*lint !e2 !e102 !e830*/

    if (ulfreespace > ulMaxSpace)
    {
        ulfreespace = ulMaxSpace; /*lint !e63*/
    } /*lint !e533*/
    
    *vol = (VOS_INT32)ulfreespace; /*lint !e10 !e129*/
    set_fs(sd_fs); /*lint !e132 !e578*/

    return VOS_OK; /*lint !e102*/
}

/*lint +e64 +e119*/

/*****************************************************************************
 函 数 名  : AT_SetMdonPara
 功能描述  :启动或关闭保存功能^MDON
 输入参数  : ucIndex
 输出参数  : 无
 返 回 值      : AT_OK:      设置成功
                AT_ERROR:   设置失败
 调用函数  :
 被调函数  :

 修改历史      :
*****************************************************************************/
VOS_UINT32 AT_SetMdonPara(VOS_UINT8 ucIndex)
{
    VOS_UINT32 ucTmpState = 0; /*本次设置的状态值*/
    VOS_UINT8 ucmlogstat = 0; /*待写入mlog状态值*/
    VOS_UINT32 ulRst = VOS_ERR;
    VOS_UINT8 ucUpdateFlag;
    VOS_INT ret = 0;
    static VOS_UINT8  init_Flag = 0;
    VOS_UINT32  ulCustomVersion = 0;
    /* -r代表每个文件的大小，以M为单位， -n代表文件数目 */
    const char* pcmd = "/app/bin/modemlogcat_lte -r 10 -n 8";
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }
    /* 参数过多 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }
    /* 参数内容只能为1个 */
    if (gastAtParaList[0].usParaLen != 1)
    {
        return AT_ERROR;
    }
    /*判断datalock是否解锁*/
    if(VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }

    ucTmpState = gastAtParaList[0].aucPara[0];
    /* ucTempState取值范围为'0'~'5' */
    if (('0' <= ucTmpState) && ('5' >= ucTmpState))
    {
        ucTmpState = (VOS_UINT32)(gastAtParaList[0].aucPara[0] - '0');
    }
    else
    {
        return AT_ERROR;
    }

    if ( VOS_OK != NV_ReadEx(MODEM_ID_0, en_NV_Item_SW_VERSION_FLAG, 
                            &ulCustomVersion, sizeof(ulCustomVersion)) )
    {
        return AT_ERROR;
    }
    /*根据传入的状态判断写入状态*/
    switch(ucTmpState)
    {
        case MDON_TYPE_ALL_CLOSE:
        {
            /*关闭所有状态*/
            /* 烧片版本无需重启,产线CBT工位要求 */
            ulRst = OM_SDSetStorageOrFlipFlag(2, MDON_TYPE_ALL_CLOSE);  /* 设置log存储开关 */
            if (VOS_OK != ulRst)
            {
                return AT_ERROR;
            }
            (void)VOS_TaskDelay(500);  /* 延时500ms */
            /* 只做端口模式切换, 不更新NV， 否则影响hids连接 */
            ulRst = PPM_LogPortSwitch(CPM_OM_PORT_TYPE_USB, VOS_FALSE);
            if (VOS_OK != ulRst)
            {
                return AT_ERROR;
            }
            if(!ulCustomVersion)
            {
                init_Flag = 0;
                return AT_OK;
            }
            else
            {
                /* release版本默认返回error，后续有需求再修改 */
                return AT_ERROR;
            }

            ucmlogstat = 0; /*lint !e527*/
            break;
        }
        case MDON_TYPE_OFFLINE_LOGSAVE:
        {
            /* 只做端口模式切换, 不更新NV， 否则影响hids连接 */
            ulRst = PPM_LogPortSwitch(CPM_OM_PORT_TYPE_VCOM, VOS_FALSE);
            if (VOS_OK != ulRst)
            {
                return AT_ERROR;
            }
            if(!ulCustomVersion)
            {
                if(init_Flag)
                {
                    return AT_OK;
                }
                init_Flag = 1;
                ulRst = OM_SDSetStorageOrFlipFlag(2, MDON_TYPE_OFFLINE_LOGSAVE);  /* 设置log存储开关 */
                if (VOS_OK == ulRst)
                {
                    ret = modemlogcat_run_cmd(pcmd);
                    if (!ret)
                    {
                        return AT_OK;
                    }
                    else
                    {
                        return AT_ERROR;
                    }
                }
                else
                {
                    return AT_ERROR;
                }
            }
            else
            {
                /* release版本默认返回error，后续有需求再修改 */
                return AT_ERROR;
            }

            break; /*lint !e527*/
        }
        case MDON_TYPE_OFFLINE_DUMPSAVE:
        case MDON_TYPE_OFFLINE_LOGEXPORT:
        case MDON_TYPE_OFFLINE_ALL:
        {
            /*离线log控制,此处仅关闭mlog*/
            ucmlogstat = 0;
            break;
        }
        case MDON_TYPE_OFFLINE_MLOG:
        {
            /*启动mlog*/
            ucmlogstat = 1;
            break;
        }
        default:
        {
            AT_ERR_LOG1("AT_SetMdonPara: not supported type, %d.\n", ucTmpState);
            return AT_ERROR;
        }
    }

    ulRst = VOS_OK;
    ulRst = (VOS_UINT32)mlog_state_set(ucmlogstat);
    if (VOS_OK == ulRst)
    {
        (void)VOS_TaskDelay(500);
        ucUpdateFlag = VOS_FALSE;
        ulRst = AT_FillAndSndAppReqMsg(gastAtClientTab[ucIndex].usClientId,
                                    gastAtClientTab[ucIndex].opId,
                                    DRV_AGENT_TMODE_SET_REQ,
                                    &ucUpdateFlag,
                                    sizeof(ucUpdateFlag),
                                    I0_WUEPS_PID_DRV_AGENT);
        if (VOS_OK != ulRst)
        {
            return AT_ERROR;    /* 此命令不需要等待C核回复，直接返回OK */
        }
        else
        {
            return AT_OK;
        }
    }
    else
    {
        return AT_ERROR;
    }

}

/*****************************************************************************
 Prototype    : modemlogcat_run_cmd
 Description  : 用来执行业务态离线log进程
 Input        : 要执行的进程名
 Output       : None
 Return Value :
 Calls        :
*****************************************************************************/
VOS_INT modemlogcat_run_cmd(const char *pcmd)
{
    VOS_INT  ret = 0;
    char *envp[] = {"HOME=/", "PATH=/app/bin:/system/bin:/sbin:/bin:/usr/sbin:/usr/bin", "LD_LIBRARY_PATH=/vendor/lib:/system/lib:/app/lib", NULL};
    char **argv = NULL;

    if (NULL == pcmd)
    {
        return -1;
    }

    argv = argv_split(0, pcmd, &ret);

    printk("ret =%d\n", ret);

    if (NULL == argv)
    {
        printk("!!!argv is NULL!!!\n");
        return -1;
    }

    if(ret <= 0)
    {
        argv_free(argv);
        printk("params=%d <= 0", ret);
        return -1;
    }

    ret = call_usermodehelper(argv[0], argv, envp, (int)UMH_WAIT_EXEC);
    argv_free(argv);
    printk("ret = %d, run cmd:%s\n", (int)ret,  pcmd);

    return ret;
}



VOS_UINT32 AT_SetMdslPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucRRCVer;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }

    /* 参数内容只能为1个 */
    if (gastAtParaList[0].usParaLen != 1)
    {
        return AT_ERROR;
    }

    ucRRCVer = gastAtParaList[0].aucPara[0];
    /* ucRRCVer取值范围为'0'~'1' */
    if (('0' <= ucRRCVer) && ('1' >= ucRRCVer))
    {
        ucRRCVer = gastAtParaList[0].aucPara[0] - '0';
    }
    else
    {
        return AT_ERROR;
    }
    ulRst = OM_SDSetStorageOrFlipFlag(0,ucRRCVer);
    if (VOS_OK == ulRst)
    {
        return AT_OK;    /* 此命令不需要等待C核回复，直接返回OK */
    }
    else
    {
        return AT_ERROR;
    }
}



VOS_UINT32 AT_SetMdStoragePara(VOS_UINT8 ucIndex) /*lint !e830*/
{
    VOS_UINT8 ucRRCVer = 0;
    mm_segment_t sd_fs = 0; /*lint !e10 !e522*/
    int retRm = 0;
    DRV_DIR_S *dirp = NULL;
    DRV_DIRENT_S *read_dirent = NULL;
    VOS_CHAR path[OM_SD_LOG_PATH_MAX_LENGTH] = {0};

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }

    /* 参数内容只能为1个 */
    if (gastAtParaList[0].usParaLen != 1)
    {
        return AT_ERROR;
    }

    ucRRCVer = gastAtParaList[0].aucPara[0];
    /* ucRRCVer取值范围为'0'~'1' */
    if (('0' <= ucRRCVer) && ('1' >= ucRRCVer))
    {
        ucRRCVer = gastAtParaList[0].aucPara[0] - '0';
    }
    else
    {
        return AT_ERROR;
    }
    if(1 == ucRRCVer)
    {
        return AT_ERROR;
    }
    if(0 == ucRRCVer)
    {
        dirp = mdrv_file_opendir(OM_FLASH_LOG_ROOT_PATH);
        if (NULL == dirp)
        {
            vos_printf("OM_FSReadData:open file path %s is null\n", OM_FLASH_LOG_ROOT_PATH); /*lint !e119*/
            return AT_ERROR;
        }

        /* 改变内存访问权限 */
        sd_fs = get_fs(); /*lint !e63*/
        set_fs(KERNEL_DS);

        while (1)
        {
            read_dirent = mdrv_file_readdir(dirp);
            if(NULL == read_dirent)
            {
                break;
            }

            VOS_sprintf(path, "%s/%s", OM_FLASH_LOG_ROOT_PATH, read_dirent->d_name);
            mdrv_file_remove(path);

        }

        retRm = mdrv_file_rmdir(OM_FLASH_LOG_ROOT_PATH);
        if (retRm != 0)
        {
            printk("mdrv_file_rmdir faild %d\n", retRm);
        }

        set_fs(sd_fs);
        return AT_OK;
    }

}/*lint !e533*/


VOS_UINT32 AT_SetMdsfPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8                           ucRRCVer;
    VOS_UINT32                          ulRst;

    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (gucAtParaIndex != 1)
    {
        return AT_ERROR;
    }

    /* 参数内容只能为1个 */
    if (gastAtParaList[0].usParaLen != 1)
    {
        return AT_ERROR;
    }

    ucRRCVer = gastAtParaList[0].aucPara[0];
    /* ucRRCVer取值范围为'0'~'1' */
    if (('0' <= ucRRCVer) && ('1' >= ucRRCVer))
    {
        ucRRCVer = gastAtParaList[0].aucPara[0] - '0';
    }
    else
    {
        return AT_ERROR;
    }
    ulRst = OM_SDSetStorageOrFlipFlag(1,ucRRCVer);
    if (VOS_OK == ulRst)
    {
        return AT_OK;    /* 此命令不需要等待C核回复，直接返回OK */
    }
    else
    {
        return AT_ERROR;
    }
}


VOS_UINT32 OM_SDSetStorageOrFlipFlag(VOS_UINT8 storageOrFlip, VOS_UINT8 flag)
{
    FILE*                       fp = NULL;
    VOS_INT32                   lReadSize = 0;
    mm_segment_t                sd_fs = 0; /*lint !e10 !e522*/

    /* 改变内存访问权限 */
    sd_fs = get_fs(); /*lint !e63*/
    set_fs(KERNEL_DS);

    /*读取开关配置文件*/
    fp = mdrv_file_open(OM_SD_LOG_SET_INFO_FILE, "rb");
    if (NULL == fp)
    {
        vos_printf("OM_SDSetStorageOrFlipFlag: The config file is not exsit, create it!\n"); /*lint !e119*/
        VOS_MemSet(&g_stOmSDLogSetInfo, 0, sizeof(OM_SD_LOG_SET_INFO));
    }
    else
    {
        lReadSize = mdrv_file_read((void *)&g_stOmSDLogSetInfo, 1, sizeof(OM_SD_LOG_SET_INFO), fp);
        if (sizeof(OM_SD_LOG_SET_INFO) != lReadSize)
        {
            vos_printf("OM_SDSetStorageOrFlipFlag: The config file read fail!\n"); /*lint !e119*/
            (void)mdrv_file_close(fp);
            set_fs(sd_fs);
            return VOS_ERR;
        }
    
        (void)mdrv_file_close(fp);
    }
    /*设置是否循环存储*/
    if ( 1 == storageOrFlip )
    {
        if ( g_stOmSDLogSetInfo.ucOfflineCleanStorage != flag)
        {
            g_stOmSDLogSetInfo.ucOfflineCleanStorage = flag;
        }
    }
    else if ( 0 == storageOrFlip )/*设置存储位置*/
    {
        if ( g_stOmSDLogSetInfo.ucStorageLocation != flag)
        {
            g_stOmSDLogSetInfo.ucStorageLocation = flag;
        }
    }
    else if ( 2 == storageOrFlip )/*设置存储LOG开关*/
    {
        if ( g_stOmSDLogSetInfo.ucOfflineSwitch != flag)
        {
            g_stOmSDLogSetInfo.ucOfflineSwitch = flag;
        }
    }

        /*写入开关配置文件*/
    fp = mdrv_file_open(OM_SD_LOG_SET_INFO_FILE, "wb");
    if (NULL == fp)
    {
        vos_printf("OM_SDSetStorageOrFlipFlag: The config file is not exsit!\n"); /*lint !e119*/
        set_fs(sd_fs);
        return VOS_ERR;
    }

    lReadSize = mdrv_file_write((void *)&g_stOmSDLogSetInfo, 1, sizeof(OM_SD_LOG_SET_INFO), fp);
    if (sizeof(OM_SD_LOG_SET_INFO) != lReadSize)
    {
        vos_printf("OM_SDSetStorageOrFlipFlag: The config file write fail!\n"); /*lint !e119*/
        (void)mdrv_file_close(fp);
        set_fs(sd_fs);
        return VOS_ERR;
    }

    (void)mdrv_file_close(fp);
    set_fs(sd_fs);
    return VOS_OK;
}



VOS_UINT32 OM_SDGetStorageOrFlipFlag(VOS_UINT8 storageOrFlip, VOS_UINT8 *flag)
{
    FILE*                       fp = DRV_FILE_NULL;
    VOS_INT32                   lReadSize = 0;
    mm_segment_t                sd_fs = 0; /*lint !e10 !e522*/

    /* 改变内存访问权限 */
    sd_fs = get_fs(); /*lint !e63*/
    set_fs(KERNEL_DS);

    /*读取配置文件*/
    fp = mdrv_file_open(OM_SD_LOG_SET_INFO_FILE, "r");
    if (NULL == fp)
    {
        vos_printf("OM_SDSetStorageOrFlipFlag: The config file is not exsit, return defualt!\n"); /*lint !e119*/
        /*默认是循环存储/Flash的方式*/
        *flag = 0;
        set_fs(sd_fs);
        return VOS_OK;
    }
    else
    {
        lReadSize = mdrv_file_read((VOS_CHAR*)&g_stOmSDLogSetInfo, 1, sizeof(OM_SD_LOG_SET_INFO), fp);
        if (sizeof(OM_SD_LOG_SET_INFO) != lReadSize)
        {
            vos_printf("OM_SDSetStorageOrFlipFlag: The config file read fail!\n"); /*lint !e119*/
            (void)mdrv_file_close(fp);
            set_fs(sd_fs);
            return VOS_ERR;
        }
        (void)mdrv_file_close(fp);
    }

    /*查询是否循环存储*/
    if ( 1 == storageOrFlip )
    {
        *flag = g_stOmSDLogSetInfo.ucOfflineCleanStorage;
    }
    else if ( 0 == storageOrFlip )/*查询存储位置*/
    {
        *flag = g_stOmSDLogSetInfo.ucStorageLocation;
    }
    else if ( 2 == storageOrFlip ) /* 查询存储LOG开关 */
    {
        *flag = g_stOmSDLogSetInfo.ucOfflineSwitch;
    }
    set_fs(sd_fs);
    return VOS_OK;
}









VOS_UINT32 At_SetHgmrPara(VOS_UINT8 ucIndex)
{
    VOS_UINT8 version_type = SOFTWARE_VERSION_MAX;
    VOS_CHAR  version_info[VERSION_MAX_LEN] = {0};
    VOS_UINT8 ulRst = AT_SUCCESS;
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数个数不为一个 */
    if (gucAtParaIndex != 1)
    {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    version_type = (VOS_UINT8)gastAtParaList[0].ulParaValue;

    switch(version_type)
    {
        /*查询基线软件版本号*/
        case BASE_SOFTWARE_VERSION:
        {
            /* 先打桩，后面添加具体的接口 */
           // PS_MEM_CPY(version_info, "V700R200C00B001", AT_STRLEN("V700R200C00B001"));
            ulRst = BSP_DLOAD_BaselineVer(version_info,VERSION_MAX_LEN);
            if (AT_SUCCESS != ulRst)
            {
                AT_ERR_LOG("At_SetHgmrPara: Get BaselineVer failed!");
                return AT_ERROR;
            }
            break;
        }
        /*查询平台软件版本号*/
        case PLATFORM_SOFTWARE_VERSION:
        {
           // PS_MEM_CPY(version_info, "21.090.00.00.000", AT_STRLEN("21.090.00.00.000"));
            ulRst = mdrv_dload_getsoftver(version_info,VERSION_MAX_LEN);
            if (AT_SUCCESS != ulRst)
            {
                AT_ERR_LOG("At_SetHgmrPara: GetSoftwareVer failed!");
                return AT_ERROR;
            }
            break;
        }
        /*查询产品软件版本号，目前不对PDU查询的版本号做处理*/
        case PRODUCT_SOFTWARE_VERSION:
            break;
        /*查询PC VENDOR 软件版本号，balong平台不支持*/
        case PC_VENDOR_SOFTWARE_VERSION:
        default:
            break;
    }
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                (TAF_CHAR *)pgucAtSndCodeAddr,
                                "%s:%s",
                                g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                version_info);
    return AT_OK;
    /*查询基线软件版本号*/
}


VOS_UINT32 At_QryHgmrPara(VOS_UINT8 ucIndex)
{
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 (TAF_CHAR *)pgucAtSndCodeAddr,
                                 "%s:%d,%d,%d",
                                 g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                 BASE_SOFTWARE_VERSION,
                                 PLATFORM_SOFTWARE_VERSION,
                                 PRODUCT_SOFTWARE_VERSION);
    return AT_OK;
}





VOS_UINT32 AT_QryHwTest(VOS_UINT8 ucIndex)
{
    VOS_UINT16 usLen = 0;
	usLen = (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   "%s:%d,%s\n",
                                   "^HWTEST", 0, hw_test_option[0]);
    
    usLen += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                   "%s:%d,%s\n",
                                   "^HWTEST", 1, hw_test_option[1]);
    
    usLen += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                   "%s:%d,%s\n",
                                   "^HWTEST", 2, hw_test_option[2]);
    usLen += (TAF_UINT16)At_sprintf( AT_CMD_MAX_LEN,
                                   (TAF_CHAR *)pgucAtSndCodeAddr,
                                   (TAF_CHAR *)pgucAtSndCodeAddr + usLen,
                                   "%s:%d,%s\n",
                                   "^HWTEST", 3, hw_test_option[3]);
 
    gstAtSendData.usBufLen = usLen;

    return AT_OK;
}


/*****************************************************************************
 函 数 名  : At_SetMbimMode
 功能描述  : AT^SETMODE=<value>（该命令用于模式切换）
 输入参数  : VOS_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
VOS_UINT32 At_SetMbimMode(VOS_UINT8 ucIndex)
{   
    int switch_mode = 0;
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 1)
    {
        return AT_ERROR;
    }
    
    /*参数长度不为1*/
    if (1 != gastAtParaList[0].usParaLen)
    {
        return AT_ERROR;
    }

    /*1表示需要切换为debug模式，0表示需要切换为NORMAL模式*/
    if('1' == gastAtParaList[0].aucPara[0])
    {
        switch_mode = 1;
       
    }
    else if('0' == gastAtParaList[0].aucPara[0])
    {
        switch_mode = 0;
    }
    else
    {

        return AT_ERROR;
    }
    
    DRV_USB_MBIM_SET_MODE(switch_mode); 
    return AT_OK;
}





/*****************************************************************************
 函 数 名  : At_QryMbimMode
 功能描述  : ^SETMODE查询命令处理函数
 输入参数  : TAF_UINT8 ucIndex
 输出参数  : 无
 返 回 值  : VOS_UINT32
 调用函数  :
 被调函数  :
*****************************************************************************/
TAF_UINT32 At_QryMbimMode(TAF_UINT8 ucIndex)
{
    VOS_UINT32 ulResult = 0;
    ulResult = DRV_USB_MBIM_GET_MODE();
    
    /* 0：NORMAL模式(即正常工作模式) 1：DEBUG模式(即调试模式)*/
    gstAtSendData.usBufLen = (TAF_UINT16)At_sprintf(AT_CMD_MAX_LEN,
                                              (TAF_CHAR *)pgucAtSndCodeAddr,
                                              (TAF_CHAR*)pgucAtSndCodeAddr,
                                              "%s: %d",
                                              g_stParseContext[ucIndex].pstCmdElement->pszCmdName,
                                              ulResult);
    return AT_OK;
}








VOS_UINT32 At_SetHwTest(VOS_UINT8 ucIndex)
{
    VOS_UINT8 hw_mode = 0, hw_option = 0;
    int ret = -1;
    int fd = -1;
    int hwtest_ftm_mod = -1;
    mm_segment_t fs_old = 0; /*lint !e10 !e522*/
    at_hwtest_info hwtest_info = {0, 0, 0};
    /* 烧片模式不支持*/
    ret = NV_ReadEx(MODEM_ID_0, NV_HUAWEI_FACTORY_CFG, 
                            (unsigned char *)(& hwtest_ftm_mod),sizeof( unsigned int));
    if(NV_OK != ret)
    {
        return AT_ERROR;
    }
    if(0 == hwtest_ftm_mod)
    {
        return AT_ERROR;
    }
    /* 参数检查 */
    if (AT_CMD_OPT_SET_PARA_CMD != g_stATParseCmd.ucCmdOptType)
    {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (gucAtParaIndex > 2)
    {
        return AT_ERROR;
    }

    /*参数长度不为1*/
    if (1 != gastAtParaList[0].usParaLen || 1 != gastAtParaList[1].usParaLen)
    {
        return AT_ERROR;
    }

    /*判断datalock是否解锁,兼容无AT白名单的产品*/
    if(VOS_TRUE == g_bAtDataLocked)
    {
        return AT_ERROR;
    }

    hw_mode = gastAtParaList[0].aucPara[0];

    /* hw_mode取值范围为'0'~'1' */
    if (('0' <= hw_mode) && ('1' >= hw_mode))
    {
        hw_mode = gastAtParaList[0].aucPara[0] - '0';
    }
    else
    {
        return AT_ERROR;
    }

    hw_option = gastAtParaList[1].aucPara[0];

    /* hw_mode取值范围为'0'~'3' */
    if (('0' <= hw_option) && ('3' >= hw_option))
    {
        hw_option = gastAtParaList[1].aucPara[0] - '0';
    }
    else
    {
        return AT_ERROR;
    }

    vos_printf("receive AT^HWTEST = %d,%d\n", hw_mode, hw_option); /*lint !e119*/

    hwtest_info.smem_hw_mode = hw_mode;
    hwtest_info.smem_hw_option = hw_option;

    /*备份内核访问权限 */
    fs_old = get_fs(); /*lint !e63*/
    
    /*改变内核访问权限 */
    set_fs(KERNEL_DS);

    fd = sys_open(HW_TEST_DEV_NAME, O_RDWR,0);
    if (fd < 0)
    {
        vos_printf( "AT^HWTEST: OPEN ERROR\n"); /*lint !e119*/
        return AT_ERROR;
    }
    ret = sys_ioctl(fd, HARDWARE_TEST_MODE_SET, (unsigned long)&hwtest_info);//set mode
    if (ret < 0)
    {
        vos_printf( "AT^HWTEST: IOCTL ERROR\n"); /*lint !e119*/
        sys_close(fd);
        return AT_ERROR;
    }
    sys_close(fd);

    /*恢复内核访问权限 */
    set_fs(fs_old);
    VOS_TaskDelay(300); /*wait 300 tick*/
    mdrv_dload_normal_reboot();

    return AT_OK;
}
























VOS_UINT32 OM_FSReadData(
                  VOS_UINT8 *pucData, 
                  VOS_UINT32 *pulLen, 
                  VOS_UINT32 *FsTotalPack, 
                  VOS_UINT32 *FsCurPack,
                  VOS_UINT32 FileIndex,
                  VOS_UINT32 ucSubName)
{
    FILE                             *fp = NULL;
    VOS_INT32                   iRet = 0;
    VOS_UINT32                  uFsTotalPack = 0;
    mm_segment_t                sd_fs = 0; /*lint !e10 !e522*/
    VOS_CHAR                    path[OM_SD_LOG_PATH_MAX_LENGTH] = {0};
    VOS_UINT32                  packet_len = *pulLen;
    DRV_DIR_S                   *dirp = NULL;
    DRV_DIRENT_S              *read_dirent = NULL;
    int max_file_id = 0;
    int min_file_id = 0;
    int temp_id = 0;

    /*首次读取初始化总数据包数及总文件长度*/
    if (0 == g_stOmFSReadCtrlInfo.isInited)
    {
        g_stOmFSReadCtrlInfo.ulTotalLength = 0;
        g_stOmFSReadCtrlInfo.ulFileCurId = 0;
        g_stOmFSReadCtrlInfo.ulCurPack = 0;

        if ( FILE_TYPE_FS == FileIndex )
        {
            dirp = mdrv_file_opendir(OM_FLASH_LOG_ROOT_PATH);
            if (NULL == dirp)
            {
                vos_printf("OM_FSReadData:open file path %s is null\n", OM_FLASH_LOG_ROOT_PATH); /*lint !e119*/
                return AT_ERROR;
            }
        }
        else if ( FILE_TYPE_VENDOR == FileIndex )
        {
            return AT_ERROR;
        }
        else
        {
            return AT_ERROR;
        }
        
        /* 改变内存访问权限 */
        sd_fs = get_fs(); /*lint !e63*/
        set_fs(KERNEL_DS);

        while (1)
        {
            read_dirent = mdrv_file_readdir(dirp);
            if(NULL == read_dirent)
            {
                break;
            }
            if(!sscanf(read_dirent->d_name, "ltelogfile_%05d", &temp_id))
            {
                continue;
            }
            //VOS_sprintf(path, "%s/LOG_%05d.lpd", OM_FLASH_LOG_ROOT_PATH, temp_id);
            VOS_sprintf(path, "%s/%s", OM_FLASH_LOG_ROOT_PATH, read_dirent->d_name);

            fp = mdrv_file_open(path, "r+");
            if (NULL == fp)
            {
                continue;
            }
            mdrv_file_seek(fp, 0, SEEK_END);
            iRet = mdrv_file_tell(fp);
            
            //iRet = OM_FSgetLogFileLength(path, FileIndex);
            if (iRet < sizeof(OM_UCMX_HEAD_INFO_STRU)) /*lint !e574*/
            {
                continue;
            }

            if (0 != g_stOmFSReadCtrlInfo.ulFileCurId)
            {
                iRet -= sizeof(OM_UCMX_HEAD_INFO_STRU);
            }

            if (temp_id > max_file_id)
            {
                max_file_id = temp_id;
            }
            
            if ((min_file_id > temp_id) || (0 == min_file_id))
            {
                min_file_id = temp_id;
            }

            g_stOmFSReadCtrlInfo.ulFileLength = iRet;
            uFsTotalPack += (g_stOmFSReadCtrlInfo.ulFileLength + packet_len -1) / packet_len;
            g_stOmFSReadCtrlInfo.ulTotalLength +=  g_stOmFSReadCtrlInfo.ulFileLength;
            g_stOmFSReadCtrlInfo.ulFileCurId++;
        
        }
        g_stFSFileInfo.ulFileMinId = min_file_id;
        g_stFSFileInfo.ulFileMaxId = max_file_id;

        kfree(read_dirent);
        if (NULL != fp)
        {
            mdrv_file_close(fp);
        }
        if (NULL != dirp)
        {
            (void)mdrv_file_closedir(dirp);
        }
        set_fs(sd_fs);
        if (0 == uFsTotalPack)
        {
            return VOS_ERR;
        }
        g_stOmFSReadCtrlInfo.ulTotalPack = uFsTotalPack;
        g_stOmFSReadCtrlInfo.ulFileCurId = 0;
        g_stOmFSReadCtrlInfo.ulDataInBuf = 0;
        g_stOmFSReadCtrlInfo.ulFileLength = 0;
        g_stOmFSReadCtrlInfo.isInited = 1;
    }

    VOS_sprintf(path, "%s/ltelogfile_%05d.lpd", OM_FLASH_LOG_ROOT_PATH,
        (g_stFSFileInfo.ulFileMinId + g_stOmFSReadCtrlInfo.ulFileCurId));
    /*如果是首次读取此文件则先初始化文件长度等*/
    if (0 == g_stOmFSReadCtrlInfo.ulFileLength)
    {
        //iRet = OM_FSgetLogFileLength(path, FileIndex);
        sd_fs = get_fs(); /*lint !e63*/
        set_fs(KERNEL_DS);
        fp = mdrv_file_open(path, "r+");
        if (NULL == fp)
        {
            set_fs(sd_fs);
            return VOS_ERR;
        }
        
        mdrv_file_seek(fp, 0, SEEK_END);
        iRet = mdrv_file_tell(fp);
        mdrv_file_close(fp);
        set_fs(sd_fs);
        if (iRet < sizeof(OM_UCMX_HEAD_INFO_STRU)) /*lint !e574*/
        {
            return VOS_ERR;
        }
//#if (FEATURE_ON == FEATURE_COMPRESS_WRITE_LOG_FILE)
        if (0 != g_stOmFSReadCtrlInfo.ulFileCurId) 
        {
            iRet -= sizeof(OM_UCMX_HEAD_INFO_STRU);
        }
//#endif
        g_stOmFSReadCtrlInfo.ulFileLength = iRet;

        g_stOmFSReadCtrlInfo.ulFileOffset = 0;
//#if (FEATURE_ON == FEATURE_COMPRESS_WRITE_LOG_FILE)
        /* LOG压缩算法开启，需要减去头文件信息 */
        if (0 != g_stOmFSReadCtrlInfo.ulFileCurId)
        {
            g_stOmFSReadCtrlInfo.ulFileOffset += sizeof(OM_UCMX_HEAD_INFO_STRU);
            vos_printf("COMPRESS:g_stOmFSReadCtrlInfo.ulFileOffset= %d \n",g_stOmFSReadCtrlInfo.ulFileOffset); /*lint !e119*/
        }
//#endif
    }
    /*读取文件内容到buf*/
    sd_fs = get_fs(); /*lint !e63*/
    set_fs(KERNEL_DS);
    fp = mdrv_file_open(path, "r+");
    if (NULL == fp)
    {
        vos_printf("SD_FS_OPEN(%s, \"r\") is NULL\n",path); /*lint !e119*/
        goto error1;
    }
    
    iRet = mdrv_file_seek(fp, g_stOmFSReadCtrlInfo.ulFileOffset , SEEK_SET);
    if(iRet < 0)
    {
        vos_printf("%s:%d:SD_FS_LSEEK is wrong\n",__FUNCTION__,__LINE__); /*lint !e119*/
        goto error2;
    }
    
    if (g_stOmFSReadCtrlInfo.ulFileLength < packet_len)
    {
        g_stOmFSReadCtrlInfo.ulDataInBuf = g_stOmFSReadCtrlInfo.ulFileLength;
    }
    else
    {
        g_stOmFSReadCtrlInfo.ulDataInBuf = packet_len;
    }
    
    iRet = mdrv_file_read(pucData,  sizeof(VOS_UINT8), g_stOmFSReadCtrlInfo.ulDataInBuf, fp);
    if(iRet != g_stOmFSReadCtrlInfo.ulDataInBuf)
    {
        vos_printf("%s:BSP_fread() is wrong\n",__FUNCTION__); /*lint !e119*/
        goto error2;
    }
    
    g_stOmFSReadCtrlInfo.ulFileLength -= g_stOmFSReadCtrlInfo.ulDataInBuf;
    g_stOmFSReadCtrlInfo.ulFileOffset += g_stOmFSReadCtrlInfo.ulDataInBuf;
    g_stOmFSReadCtrlInfo.ulCurPack ++;
    mdrv_file_close(fp);
    set_fs(sd_fs);

    *pulLen = g_stOmFSReadCtrlInfo.ulDataInBuf;
    *FsTotalPack = g_stOmFSReadCtrlInfo.ulTotalPack;
    *FsCurPack = g_stOmFSReadCtrlInfo.ulCurPack;

    if (0 == g_stOmFSReadCtrlInfo.ulFileLength)
    {
        g_stOmFSReadCtrlInfo.ulFileCurId ++;
    }

    if (g_stOmFSReadCtrlInfo.ulCurPack == g_stOmFSReadCtrlInfo.ulTotalPack)
    {
        g_stOmFSReadCtrlInfo.ulCurPack = 0;
        g_stOmFSReadCtrlInfo.ulFileCurId = 0;
        g_stOmFSReadCtrlInfo.ulTotalLength = 0;
        g_stOmFSReadCtrlInfo.ulTotalPack = 0;
        g_stOmFSReadCtrlInfo.isInited = 0;
        /*if ( FILE_TYPE_VENDOR == FileIndex )
        {
            OM_SetVendorLogStop(0);
        }*/
    }

    return VOS_OK;
error2:
    mdrv_file_close(fp);
error1:
    set_fs(sd_fs);
    vos_printf("%s:return VOS_ERR\n",__FUNCTION__); /*lint !e119*/
    return VOS_ERR;
}
/*lint +e64 +e119*/

/*****************************************************************************
 函 数 名  : Mbb_AT_QryRsfrLogFs
 功能描述  : 导出离线log
 输入参数  : ucIndex - 用户索引
 输出参数  : 无
 返 回 值  : 导出成功: AT_OK, 失败: AT_ERROR
 调用函数  : OM_FSReadData
             AtBase64Encode
 被调函数  : AT_SetRsfrPara
 修改历史  :
*****************************************************************************/
VOS_UINT32 Mbb_AT_QryRsfrLogFs(VOS_CHAR* cSubName, VOS_CHAR* cName)
{
    VOS_UINT32 ucSubName = 0;
    VOS_UINT32 buflenght = 0;


    VOS_UINT32 uFsdataCurPackNum = 0;
    VOS_UINT32 uFsdataTotalPack = 0;
    VOS_UINT32 uFileIndex = 0;
    VOS_UINT8 *pcRsfrSrcItem = NULL;
    VOS_UINT8 *pcRsfrDstItem = NULL;
    /*lint -e64*/
    (void)At_Auc2ul(cSubName,gastAtParaList[1].usParaLen,&ucSubName);   
    /*lint +e64*/
    if (ERR_MSP_SUCCESS == AT_STRCMP(cName, "LOGSAVE_FS"))
    {
        uFileIndex = FILE_TYPE_FS;
    }
    else if (ERR_MSP_SUCCESS == AT_STRCMP(cName, "LOGSAVE_VENDOR"))
    {
        //uFileIndex = FILE_TYPE_VENDOR;
    }

    pcRsfrSrcItem = (VOS_UINT8*)PS_MEM_ALLOC(WUEPS_PID_AT, AT_RSFR_RSFW_MAX_LEN);
    if (VOS_NULL_PTR == pcRsfrSrcItem )
    {
        return AT_ERROR;
    }

    PS_MEM_SET(pcRsfrSrcItem, 0, AT_RSFR_RSFW_MAX_LEN);

    buflenght = AT_RSFR_RSFW_DATA_LEN;
    if (VOS_OK != OM_FSReadData(pcRsfrSrcItem,&buflenght,
        &uFsdataTotalPack,&uFsdataCurPackNum, uFileIndex, ucSubName))
    {
        PS_MEM_FREE(WUEPS_PID_AT, pcRsfrSrcItem); /*lint !e830*/
        return AT_ERROR;
    }
    pcRsfrDstItem = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT,AT_RSFR_RSFW_MAX_LEN);
    if(pcRsfrDstItem == NULL)
    {
        PS_MEM_FREE(WUEPS_PID_AT, pcRsfrSrcItem);
        return AT_ERROR;
    }
    PS_MEM_SET(pcRsfrDstItem, 0, AT_RSFR_RSFW_MAX_LEN);

    /*对数据进行Base64编码*/
    AtBase64Encode(pcRsfrSrcItem, buflenght, pcRsfrDstItem);/*lint !e516*/

    /*lint -e64 -e119*/   
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                "^RSFR:\"%s\",\"%s\",%d,%d,%d,",
                                cName,cSubName,OFFLINE_LOG_REF_BASE,uFsdataTotalPack,uFsdataCurPackNum);
                                   
    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                "\"%s\",%s", pcRsfrDstItem,gaucAtCrLf);
    /*lint +e64 +e119*/
    PS_MEM_FREE(WUEPS_PID_AT, pcRsfrSrcItem);
    PS_MEM_FREE(WUEPS_PID_AT, pcRsfrDstItem);
    return AT_OK;
}





/******************************************************************************
                       MBB DRV AT list
******************************************************************************/
AT_PAR_CMD_ELEMENT_STRU g_astDrvAtPrivateCmdTbl[] = {

/* HUAWEI 移动宽带产品终端设备一键升级接口规范V1.07.doc 文档涉及的命令底软未归一: V700R001版本使用LTE的实现，V300R002版本使用GU模的实现 */
    {AT_CMD_SUPINFO,
    atSetUpInfo, AT_UPGRADE_TIME_5S, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
    (VOS_UINT8*)"^SUPINFO", (VOS_UINT8*)"(@time),(@ver),(1-10)"},

    {AT_CMD_CHKUPINFO,
    atSetUpInfoNum, AT_UPGRADE_TIME_5S, atQryUpInfo, AT_UPGRADE_TIME_5S, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_ERROR, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
    (VOS_UINT8*)"^CHKUPINFO", (VOS_UINT8*)"(1-10)"},

    {AT_CMD_USBSPEED,
    VOS_NULL_PTR,           AT_SET_PARA_TIME,   At_QrySpeed,          AT_QRY_PARA_TIME,   VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR,   AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL | CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^USBSPEED",    TAF_NULL_PTR},


    /*lint -e10 -e64 -e133*/
    /*离线log与mlog开关控制AT命令*/
    {AT_CMD_MDON,
    AT_SetMdonPara, AT_NOT_SET_TIME, AT_QryMdonPara, AT_QRY_PARA_TIME, At_TestMdonPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^MDON",   (TAF_UINT8*)"(0-5)"},
    /*lint +e10 +e64 +e133*/
    {AT_CMD_MDSL,
    AT_SetMdslPara, AT_NOT_SET_TIME, AT_QryMdslPara, AT_QRY_PARA_TIME, At_TestMdslPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^MDSL", (TAF_UINT8*)"(0,1)"},

    {AT_CMD_MDSTORAGE,
    AT_SetMdStoragePara, AT_NOT_SET_TIME, AT_QryMdStoragePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^MDSTORAGE", (TAF_UINT8*)"(0,1)"},

    {AT_CMD_MDSF,
    AT_SetMdsfPara, AT_NOT_SET_TIME, AT_QryMdsfPara, AT_QRY_PARA_TIME, At_TestMdsfPara, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^MDSF", (TAF_UINT8*)"(0,1)"},
    /*lint +e10 +e64 +e133*/


    {AT_CMD_HGMR,
    At_SetHgmrPara,     AT_NOT_SET_TIME,    At_QryHgmrPara,    AT_NOT_SET_TIME,    
    VOS_NULL_PTR ,    AT_NOT_SET_TIME,
    VOS_NULL_PTR,AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8*)"^HGMR",  (VOS_UINT8 *)"(1-4)"},

/*lint -e553 修改人:姚旿冬;检视人:陈博;原因:MBB_USB_PORT_AUTH为产品宏*/



    {AT_CMD_HWTEST,
    At_SetHwTest,     AT_SET_PARA_TIME,   AT_QryHwTest,       AT_QRY_PARA_TIME,   VOS_NULL_PTR,   AT_NOT_SET_TIME,
    VOS_NULL_PTR,        AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS,    CMD_TBL_NO_LIMITED,
    (VOS_UINT8 *)"^HWTEST",   (VOS_UINT8 *)"(0-1),(0-3)"},







}; /* g_astDrvAtPrivateCmdTbl */

VOS_UINT32 At_RegisterMbbDrvPrivateCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_astDrvAtPrivateCmdTbl, sizeof(g_astDrvAtPrivateCmdTbl)/sizeof(g_astDrvAtPrivateCmdTbl[0]));
}



