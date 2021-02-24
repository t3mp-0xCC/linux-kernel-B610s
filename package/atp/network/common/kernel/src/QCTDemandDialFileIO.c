

/*****************************************************************************
1 头文件包含
*****************************************************************************/
#include "QCTDemandDialFileIO.h"
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kallsyms.h>
#include <asm/uaccess.h>
#include <asm-generic/errno-base.h>
#include <linux/huawei_netlink.h>
#include <linux/if.h>

#define DEVICE_ID_RNIC 11   //此值发生变化请同时修改xt_REDIRECT.c中此值的宏定义
#define RNIC_DIALRPT_DOWN 0x0601
#define RNIC_DIALRPT_UP 0x0600
#define DEC 10
#define WLAN_DEV_NAME_LEN   64

char g_wlan1_name[IFNAMSIZ] = {0};
char g_wlan2_name[IFNAMSIZ] = {0};
char g_wlan3_name[IFNAMSIZ] = {0};
char g_wlan4_name[IFNAMSIZ] = {0};

static const struct file_operations g_stOnDemandFileOps        =
{
    .owner      = THIS_MODULE,
    .write       = RNIC_WriteOnDemandFile,
    .read        = RNIC_ReadOnDemandFile,
};

static const struct file_operations g_stIdleTimerOutFileOps    =
{
    .owner      = THIS_MODULE,
    .write       = RNIC_WriteIdleTimerOutFile,
    .read        = RNIC_ReadIdleTimerOutFile,
};

static const struct file_operations g_stDialEventReportFileOps =
{
    .owner      = THIS_MODULE,
    .write       = RNIC_WriteDialEventReportFile,
    .read        = RNIC_ReadDialEventReportFile,
};
static const struct file_operations g_stDialLogSwitchFileOps =
{
    .owner      = THIS_MODULE,
    .write      = RNIC_WriteLogSwitchFile,
    .read       = RNIC_ReadLogSwitchFile,
};
static const struct file_operations g_stFluxFlagFileOps    =
{
    .owner      = THIS_MODULE,
    .write      = RNIC_WriteFluxFlagFile,
    .read       = RNIC_ReadFluxFlagFile,
};
RNIC_DIAL_MODE_STRU  g_DialMode;//全局数据
RNIC_FLUX_TIMER  g_flux_timer;
void rnic_flux_timer_handler(unsigned long lparam)
{
    if(g_DialMode.is_flow)
    {
        send_rnic_connect_event_to_app();
        g_DialMode.is_flow = false;
    }
    mod_timer(&g_flux_timer.timer, jiffies + ONDEMAND_CONNECT_FLUX_CHECK_TIME * HZ);
}

void rnic_flux_timer_init(void)
{     
        init_timer(&g_flux_timer.timer);
        g_flux_timer.is_timer_init = true;
        g_flux_timer.timer.function = rnic_flux_timer_handler;  
}

void rnic_flux_timer_start(void)
{
        mod_timer(&g_flux_timer.timer, jiffies + ONDEMAND_CONNECT_FLUX_CHECK_TIME * HZ);
}

void rnic_flux_timer_delete(void)
{
        del_timer(&g_flux_timer.timer);
        g_flux_timer.is_timer_init = false;
}


/*****************************************************************************
3 函数实现
*****************************************************************************/

VOS_UINT32 RNIC_TransferStringToInt(VOS_CHAR *pcString)
{
    VOS_UINT32 ulStrLen = 0;
    VOS_UINT32 i = 0;
    VOS_UINT32 ulRst = 0;

    ulStrLen = 0;
    ulRst = 0;

    ulStrLen = strlen(pcString);

    for ( i = 0; i < ulStrLen; i++ )
    {
        if ( (pcString[i] >= '0') && (pcString[i] <= '9') )
        {
            ulRst = (ulRst * DEC) + (pcString[i] - '0');
        }
    }

    return ulRst;
}

/*字符串与整数转换*/
typedef unsigned int dsat_num_item_type;
typedef  unsigned char      byte;  
typedef enum
{                       
  ATOI_OK,              /*  conversion complete             */
  ATOI_NO_ARG,          /*  nothing to convert              */
  ATOI_OUT_OF_RANGE     /*  character or value out of range */
} atoi_enum_type;
#define MAX_VAL_NUM_ITEM 0xFFFFFFFF /*  Max value of a numeric AT parm     */
#define  UPCASE( c ) ( ((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c) )
/*****************************************************************************
 函 数 名  : dsatutil_atoi
 功能描述  : 实现字符串安进制转换为整数
 输入参数  : val_arg_ptr ----- 指针用于存储转换结果
             s----- 原始的字符串
             r------进制
 输出参数  : 无
 返 回 值  : 成功或失败


    修改内容   : 新生成函数

*****************************************************************************/
atoi_enum_type dsatutil_atoi
(
    dsat_num_item_type *val_arg_ptr,     
    const byte *s,     
    unsigned int r    
)
{
    atoi_enum_type err_ret = ATOI_NO_ARG;
    byte c;
    dsat_num_item_type val, val_lim, dig_lim;
    
    val = 0;
    val_lim = (dsat_num_item_type) ((unsigned int)MAX_VAL_NUM_ITEM / r);
    dig_lim = (dsat_num_item_type) ((unsigned int)MAX_VAL_NUM_ITEM % r);

    if (NULL == val_arg_ptr || NULL == s)
    {
        return ATOI_NO_ARG;
    }
    while ( (c = *s++) != '\0')
    {
        if (c != ' ')
        {
            c = (byte) UPCASE (c);
            if (c >= '0' && c <= '9')
            {
                c -= '0';
            }
            else if (c >= 'A')
            {
                c -= 'A' - DEC;
            }
            else
            {
                break;
            }
            
            if (c >= r || val > val_lim
                || (val == val_lim && c > dig_lim))
            {
                err_ret = ATOI_OUT_OF_RANGE;  /*  char code too large */
                break;
            }
            else
            {
                err_ret = ATOI_OK;            /*  arg found: OK so far*/
                val = (dsat_num_item_type) (val * r + c);
            }
        }
        *val_arg_ptr =  val;
    }
  
    return err_ret;
}

/*****************************************************************************
 函 数 名  : RNIC_PraseWlanDevName
 功能描述  : 解析wlan的设备名
 输入参数  : char* namebuf, int len
 输出参数  : 无
 返 回 值  : 
*****************************************************************************/
void RNIC_PraseWlanDevName(char* namebuf)
{
    char* file_name = NULL;
    char* file_list = NULL;
    char *tmp_list = NULL;
    char temp_buf[WLAN_DEV_NAME_LEN] = {0};
    PS_MEM_SET(g_wlan1_name, 0x00, IFNAMSIZ);
    PS_MEM_SET(g_wlan2_name, 0x00, IFNAMSIZ);
    PS_MEM_SET(g_wlan3_name, 0x00, IFNAMSIZ);
    PS_MEM_SET(g_wlan4_name, 0x00, IFNAMSIZ);
    if (NULL  == namebuf)
    {
        return;
    }

    snprintf(temp_buf, WLAN_DEV_NAME_LEN, "%s", namebuf);
    file_list =  temp_buf;
    file_name = file_list;
    tmp_list = strstr(file_list, ",");
    if (NULL != tmp_list)
    {
        file_list = tmp_list;
        *file_list++ = 0;
         snprintf(g_wlan1_name, IFNAMSIZ, "%s", file_name);
    }
    else
    {
        snprintf(g_wlan1_name, IFNAMSIZ, "%s", file_name);
        return;
    }
    file_name = file_list;
    tmp_list = strstr(file_list, ",");
    if (NULL != tmp_list)
    {
        file_list = tmp_list;
        *file_list++ = 0;
        snprintf(g_wlan2_name, IFNAMSIZ, "%s", file_name);

    }
    else
    {
        snprintf(g_wlan2_name, IFNAMSIZ, "%s", file_name);
        return;
    }
    file_name = file_list;
    tmp_list = strstr(file_list, ",");
    if (NULL != tmp_list)
    {
        file_list = tmp_list;
        *file_list++ = 0;
        snprintf(g_wlan3_name, IFNAMSIZ, "%s", file_name);
    }
    else
    {
        snprintf(g_wlan3_name, IFNAMSIZ, "%s", file_name);
        return;
    }
    file_name = file_list;
    snprintf(g_wlan4_name, IFNAMSIZ, "%s", file_name);
    return;
}


ssize_t RNIC_ReadOnDemandFile(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    VOS_CHAR  acModeTemp[RNIC_ONDEMAND_FILE_LEN] = {0};
    VOS_INT32 lRlst = 0;

    if (*ppos > 0)
    {
        RNIC_ERROR_LOG("RNIC_ReadOnDemandFile:ppos is err!");
        return 0;
    }

    PS_MEM_SET(acModeTemp, 0x00, RNIC_ONDEMAND_FILE_LEN);
    snprintf((VOS_CHAR *)acModeTemp, RNIC_ONDEMAND_FILE_LEN, "%lu", g_DialMode.enDialMode);

    len = PS_MIN(len, strlen(acModeTemp));

    /*拷贝内核空间数据到用户空间上面*/
    lRlst = copy_to_user(buf,(VOS_VOID *)acModeTemp, len);

    if (lRlst < 0)
    {
        RNIC_ERROR_LOG("RNIC_ReadOnDemandFile:copy_to_user fail!");
        return -EPERM;
    }

    *ppos += len;

    return len;
}


ssize_t RNIC_WriteOnDemandFile(struct file *file, const char __user *buf, size_t len,loff_t *ppos)
{
    VOS_CHAR  acModeTemp[RNIC_ONDEMAND_FILE_LEN] = {0};
    VOS_INT32 lRlst = 0;

    PS_MEM_SET(acModeTemp, 0x00, RNIC_ONDEMAND_FILE_LEN);

    if (len >= RNIC_ONDEMAND_FILE_LEN)
    {
        RNIC_ERROR_LOG("RNIC_WriteOnDemandFile:len is %d!\n", len);
        return -ENOSPC;
    }

    /*拷贝用户空间数据到内核空间上面*/
    lRlst = copy_from_user((VOS_VOID *)acModeTemp, (VOS_VOID *)buf, len);

    if (lRlst > 0)
    {
        RNIC_ERROR_LOG("RNIC_WriteOnDemandFile:copy_from_user ERR!");
        return -EFAULT;
    }

    acModeTemp[len] = '\0';

    g_DialMode.enDialMode = RNIC_TransferStringToInt(acModeTemp);
    printk(KERN_ERR"RNIC_WriteOnDemandFile:OnDemand=%lu\n",g_DialMode.enDialMode);//fangxiaozhi for warning
        switch((RNIC_ONDEMAND_TYPE_ENUM)g_DialMode.enDialMode)
        {
                case ONDEMAND_DISABLE:
                    {
                        

                        if(g_flux_timer.is_timer_init)
                        {
                         rnic_flux_timer_delete();
                        }
                         g_DialMode.is_flow = false;
                    }
                break;
                case ONDEMAND_DISCONN:
                    {
  
                        if(g_flux_timer.is_timer_init)
                        {
                         rnic_flux_timer_delete();
                        }
                         g_DialMode.is_flow = false;
                    }
                break;
                case ONDEMAND_CONNECT:
                    {
                        g_DialMode.is_flow = false;
                        if(!g_flux_timer.is_timer_init)
                        {
                        rnic_flux_timer_init();
                        }
                        rnic_flux_timer_start(); 
                    }
               
                break;
                default:
               printk(KERN_ERR"%sInvalid arguments g_sta_flux.wifi_ondemand_flag =%lu\n",\
               __func__,g_DialMode.enDialMode  );//fangxiaozhi for warning

        }

    return len;
}


VOS_UINT32 RNIC_InitOnDemandFile(struct proc_dir_entry *pstParentFileDirEntry)
{
    struct proc_dir_entry *pstOnDemandEntry = NULL;

    if (VOS_NULL_PTR == pstParentFileDirEntry)
    {
        /*创建OnDemand虚拟文件*/
        pstOnDemandEntry = proc_create("dial/ondemand",
                                                          RNIC_VFILE_CRT_LEVEL,
                                                          pstParentFileDirEntry,
                                                          &g_stOnDemandFileOps);

        if (VOS_NULL_PTR == pstOnDemandEntry)
        {
            RNIC_WARNING_LOG("RNIC_OnDemandVf_Init: proc_create Return NULL");
            return VOS_ERR;
        }
    }
    else
    {
        /*创建OnDemand虚拟文件*/
        pstOnDemandEntry = proc_create("ondemand",
                                                          RNIC_VFILE_CRT_LEVEL,
                                                          pstParentFileDirEntry,
                                                          &g_stOnDemandFileOps);

        if (VOS_NULL_PTR == pstOnDemandEntry)
        {
            RNIC_WARNING_LOG("RNIC_OnDemandVf_Init: proc_create Return NULL");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


ssize_t RNIC_ReadIdleTimerOutFile(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    VOS_CHAR  acModeTemp[RNIC_IDLETIMEROUT_FILE_LEN] = {0};
    VOS_INT32 lRlst = 0;

    if (*ppos > 0)
    {
        RNIC_ERROR_LOG("RNIC_ReadIdleTimerOutFile:ppos is err!");
        return 0;
    }

    PS_MEM_SET(acModeTemp, 0x00, RNIC_IDLETIMEROUT_FILE_LEN);

    snprintf((VOS_CHAR *)acModeTemp, RNIC_IDLETIMEROUT_FILE_LEN, "%d", g_DialMode.disconnect_time);

    len = PS_MIN(len, strlen(acModeTemp));

    /*拷贝内核空间数据到用户空间上面*/
    lRlst = copy_to_user(buf,(VOS_VOID *)acModeTemp, len);

    if (lRlst < 0)
    {
        RNIC_ERROR_LOG("RNIC_ReadIdleTimerOutFile:copy_to_user fail!");
        return -EPERM;
    }

    *ppos += len;

    return len;
}



ssize_t RNIC_WriteIdleTimerOutFile(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{

    VOS_CHAR  acModeTemp[RNIC_IDLETIMEROUT_FILE_LEN] = {0};
    VOS_INT32 lRlst = 0;

    unsigned int tmp_disconnect_time = 0;
 
    PS_MEM_SET(acModeTemp, 0x00, RNIC_IDLETIMEROUT_FILE_LEN);
    if (len >= RNIC_IDLETIMEROUT_FILE_LEN)
    {
        RNIC_ERROR_LOG("RNIC_WriteIdleTimerOutFile:len is %d!\n", len);
        return -ENOSPC;
    }

    /*拷贝用户空间数据到内核空间上面*/
    lRlst = copy_from_user((VOS_VOID *)acModeTemp, (VOS_VOID *)buf, len);

    if (lRlst > 0)
    {
        RNIC_ERROR_LOG("RNIC_WriteOnDemandFile:copy_from_user ERR!");
        return -EFAULT;
    }
    acModeTemp[len] = '\0';

    lRlst = dsatutil_atoi(&tmp_disconnect_time, acModeTemp, DEC);
    if (ATOI_OK != lRlst)
    {
        return len;
    }
    if(0 != tmp_disconnect_time)
    {   
        g_DialMode.disconnect_time = tmp_disconnect_time;  
        printk(KERN_ERR"don't allow to set RNICTimerOut=0\n"); 
    }

    return len;
}


VOS_UINT32 RNIC_InitIdleTimerOutFile(struct proc_dir_entry *pstParentFileDirEntry)
{
    struct proc_dir_entry *pstIdleTimeOutEntry = NULL;

    if (VOS_NULL_PTR == pstParentFileDirEntry )
    {
        /*创建IdleTimeOut虚拟文件*/
        pstIdleTimeOutEntry = proc_create("dial/idle_timeout",
                                                          RNIC_VFILE_CRT_LEVEL,
                                                          pstParentFileDirEntry,
                                                          &g_stIdleTimerOutFileOps);

        if (VOS_NULL_PTR == pstIdleTimeOutEntry)
        {
            RNIC_ERROR_LOG("RNIC_InitIdleTimerOutFile:pstIdleTimeOutEntry is null!");
            return VOS_ERR;
        }
    }
    else
    {
        /*创建IdleTimeOut虚拟文件*/
        pstIdleTimeOutEntry = proc_create("idle_timeout",
                                                          RNIC_VFILE_CRT_LEVEL,
                                                          pstParentFileDirEntry,
                                                          &g_stIdleTimerOutFileOps);

        if (VOS_NULL_PTR == pstIdleTimeOutEntry)
        {
            RNIC_ERROR_LOG("RNIC_InitIdleTimerOutFile:pstIdleTimeOutEntry is null!");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}
/*****************************************************************************
 函 数 名  : RNIC_ReadFluxFlagFile
 功能描述  : ideltimerout虚拟文件读实现
 输入参数  : file --- 文件句柄
             buf  --- 用户空间
             ppos --- 文件偏移，参数未使用
 输出参数  : 无
 返 回 值  : 成功或失败

    修改内容   : 新生成函数

*****************************************************************************/
ssize_t RNIC_ReadFluxFlagFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                             len, //fangxiaozhi for warning
    loff_t                             *ppos
)
{
    VOS_CHAR                            acModeTemp[RNIC_ONDEMAND_FILE_LEN];
    VOS_INT32                           lRlst;

    if (*ppos > 0)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC "RNIC_ReadFluxFlagFile:ppos is err!");//fangxiaozhi for warning
        return 0;
    }

    PS_MEM_SET(acModeTemp, 0x00, RNIC_ONDEMAND_FILE_LEN);
    sprintf((VOS_CHAR *)acModeTemp, "%d", g_DialMode.is_flow);
    printk(KERN_ERR"RNIC_ReadFluxFlagFile:FluxFlag=%d\n",g_DialMode.is_flow);
    g_DialMode.is_flow = false;
    len  = PS_MIN(len, strlen(acModeTemp));

    /*拷贝内核空间数据到用户空间上面*/
    lRlst = copy_to_user(buf,(VOS_VOID *)acModeTemp, len);

    if (lRlst < 0)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_ReadFluxFlagFile:copy_to_user fail!");//fangxiaozhi for warning
        return -EPERM;
    }

    *ppos += len;

    return len;
}



ssize_t RNIC_WriteFluxFlagFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
)
{
    //RNIC_DIAL_MODE_STRU                 stDialMode;//fangxiaozhi for warning
    VOS_CHAR                            acModeTemp[RNIC_ONDEMAND_FILE_LEN];
    VOS_INT32                           lRlst;
	unsigned int tmp = 0;//fangxiaozhi for warning
	
    PS_MEM_SET(acModeTemp, 0x00, RNIC_ONDEMAND_FILE_LEN);
    if (len >= RNIC_ONDEMAND_FILE_LEN)
    {
        //RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_WriteFluxFlagFile:len is!", len);//fangxiaozhi for warning
        return -ENOSPC;
    }

    /*拷贝用户空间数据到内核空间上面*/
    lRlst = copy_from_user((VOS_VOID *)acModeTemp, (VOS_VOID *)buf, len);

    if (lRlst > 0)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC "RNIC_WriteFluxFlagFile:copy_from_user ERR!");//fangxiaozhi for warning
        return -EFAULT;
    }
    acModeTemp[len] = '\0';
    //unsigned int tmp = 0;//fangxiaozhi for warning
     dsatutil_atoi(&tmp,acModeTemp,DEC);
    if(1 == tmp)
    {   
     send_rnic_disconnect_event_to_app();
    }

    return len;
}

/*****************************************************************************
 函 数 名  : RNIC_InitFluxFlagFile
 功能描述  : ideltimerout虚拟文件初始化
 输入参数  : proc_dir_entry *pstParentFileDirEntry -- 父目录的入口
 输出参数  : 无
 返 回 值  : 初始化结果

*****************************************************************************/
VOS_UINT32 RNIC_InitFluxFlagFile(struct proc_dir_entry *pstParentFileDirEntry)
{
    struct proc_dir_entry              *pstFluxFlagEntry;

    if (VOS_NULL_PTR == pstParentFileDirEntry )
    {
        /*创建IdleTimeOut虚拟文件*/
              pstFluxFlagEntry                 = proc_create("dial/flux",
                                                         RNIC_VFILE_CRT_LEVEL,
                                                         pstParentFileDirEntry,
                                                         &g_stFluxFlagFileOps);

        if (VOS_NULL_PTR == pstFluxFlagEntry)
        {
            RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_InitFluxFlagFile:g_stFluxFlagFileOps is null!");//fangxiaozhi for warning

            return VOS_ERR;
        }
    }
    else
    {
        /*创建IdleTimeOut虚拟文件*/
              pstFluxFlagEntry                 = proc_create("flux",
                                                         RNIC_VFILE_CRT_LEVEL,
                                                         pstParentFileDirEntry,
                                                         &g_stFluxFlagFileOps);

        if (VOS_NULL_PTR == pstFluxFlagEntry)
        {
            RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_InitFluxFlagFile:pstFluxFlagEntry is null!");//fangxiaozhi for warning

            return VOS_ERR;
        }
    }

    return VOS_OK;
}

ssize_t RNIC_ReadLogSwitchFile(
    struct file                        *file,
    char __user                        *buf,
    size_t                             len, //fangxiaozhi for warning
    loff_t                             *ppos
)
{
    VOS_CHAR                            acModeTemp[RNIC_ONDEMAND_FILE_LEN];
    VOS_INT32                           lRlst;

    if (*ppos > 0)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC "RNIC_ReadLogSwitchFile:ppos is err!");//fangxiaozhi for warning
        return 0;
    }

    PS_MEM_SET(acModeTemp, 0x00, RNIC_ONDEMAND_FILE_LEN);
    sprintf((VOS_CHAR *)acModeTemp, "%d", g_DialMode.is_dump_rmnet_skb);

    len  = PS_MIN(len, strlen(acModeTemp));

    /*拷贝内核空间数据到用户空间上面*/
    lRlst = copy_to_user(buf,(VOS_VOID *)acModeTemp, len);

    if (lRlst < 0)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_ReadLogSwitchFile:copy_to_user fail!");//fangxiaozhi for warning
        return -EPERM;
    }

    *ppos += len;

    return len;
}



ssize_t RNIC_WriteLogSwitchFile(
    struct file                        *file,
    const char __user                  *buf,
    size_t                              len,
    loff_t                             *ppos
)
{

    //RNIC_DIAL_MODE_STRU                 stDialMode;//fangxiaozhi for warning
    VOS_CHAR                            acModeTemp[RNIC_ONDEMAND_FILE_LEN];
    VOS_INT32                           lRlst;

    PS_MEM_SET(acModeTemp, 0x00, RNIC_ONDEMAND_FILE_LEN);
    if (len >= RNIC_ONDEMAND_FILE_LEN)
    {
        //RNIC_ERROR_LOG1(ACPU_PID_RNIC "RNIC_WriteLogSwitchFile:len is!", len);//fangxiaozhi for warning
        return -ENOSPC;
    }

    /*拷贝用户空间数据到内核空间上面*/
    lRlst = copy_from_user((VOS_VOID *)acModeTemp, (VOS_VOID *)buf, len);

    if (lRlst > 0)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_WriteLogSwitchFile:copy_from_user ERR!");//fangxiaozhi for warning
        return -EFAULT;
    }
    acModeTemp[len] = '\0';

    g_DialMode.is_dump_rmnet_skb = RNIC_TransferStringToInt(acModeTemp); 

    return len;
}

VOS_UINT32 RNIC_InitLogSwitchFile(struct proc_dir_entry *pstParentFileDirEntry)
{
    struct proc_dir_entry              *pstLogSwitchEntry;

    if (VOS_NULL_PTR == pstParentFileDirEntry )
    {
        /*创建IdleTimeOut虚拟文件*/
        pstLogSwitchEntry                 = proc_create("dial/log_switch",
                                                         RNIC_VFILE_CRT_LEVEL,
                                                         pstParentFileDirEntry,
                                                         &g_stDialLogSwitchFileOps);

        if (VOS_NULL_PTR == pstLogSwitchEntry)
        {
            RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_InitLogSwitchFile:pstIdleTimeOutEntry is null!");//fangxiaozhi for warning

            return VOS_ERR;
        }
    }
    else
    {
        /*创建pstLogSwitchEntry虚拟文件*/
        pstLogSwitchEntry                 = proc_create("log_switch",
                                                         RNIC_VFILE_CRT_LEVEL,
                                                         pstParentFileDirEntry,
                                                         &g_stDialLogSwitchFileOps);

        if (VOS_NULL_PTR == pstLogSwitchEntry)
        {
            RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_InitLogSwitchFile:pstLogSwitchEntry is null!");//fangxiaozhi for warning

            return VOS_ERR;
        }
    }

    return VOS_OK;
}


ssize_t RNIC_ReadDialEventReportFile(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
    VOS_CHAR   acModeTemp[RNIC_EVENTFLAG_FILE_LEN] = {0};
    VOS_INT32  lRlst = 0;

    if (*ppos > 0)
    {
        RNIC_ERROR_LOG("RNIC_ReadDialEventReportFile:ppos is err!");
        return 0;
    }

    PS_MEM_SET(acModeTemp, 0x00, RNIC_EVENTFLAG_FILE_LEN);

    snprintf((VOS_CHAR *)acModeTemp, RNIC_EVENTFLAG_FILE_LEN, "%lu", g_DialMode.enEventReportFlag);

    len = PS_MIN(len, strlen(acModeTemp));

    /*拷贝内核空间数据到用户空间上面*/
    lRlst = copy_to_user(buf,(VOS_VOID *)acModeTemp, len);

    if (lRlst < 0)
    {
        RNIC_ERROR_LOG("RNIC_ReadIdleTimerOutFile:copy_to_user fail!");
        return -EPERM;
    }

    *ppos += len;

    return len;
}



ssize_t RNIC_WriteDialEventReportFile(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{

    VOS_CHAR  acModeTemp[RNIC_EVENTFLAG_FILE_LEN] = {0};
    VOS_INT32  lRlst = 0;

    PS_MEM_SET(acModeTemp, 0x00, RNIC_EVENTFLAG_FILE_LEN);

    if (len >= RNIC_EVENTFLAG_FILE_LEN)
    {
        RNIC_ERROR_LOG("RNIC_WriteDialEventReportFile:len is %d!\n", len);
        return -ENOSPC;
    }

    /*拷贝用户空间数据到内核空间上面*/
    lRlst = copy_from_user((VOS_VOID *)acModeTemp, (VOS_VOID *)buf, len);

    if (lRlst > 0)
    {
        RNIC_ERROR_LOG("RNIC_WriteDialEventReportFile:copy_from_user ERR!");
        return -EFAULT;
    }

    acModeTemp[len] = '\0';

    g_DialMode.enEventReportFlag = RNIC_TransferStringToInt(acModeTemp);

    return len;
}

VOS_UINT32 RNIC_InitDialEventReportFile(struct proc_dir_entry *pstParentFileDirEntry)
{
    struct proc_dir_entry *pstDialEventReportEntry = NULL;

    if (VOS_NULL_PTR == pstParentFileDirEntry)
    {
        /*创建dial_event_report虚拟文件*/
        pstDialEventReportEntry = proc_create("dial/dial_event_report",
                                                              RNIC_VFILE_CRT_LEVEL,
                                                              pstParentFileDirEntry,
                                                              &g_stDialEventReportFileOps);

        if (VOS_NULL_PTR == pstDialEventReportEntry)
        {
            RNIC_ERROR_LOG("RNIC_InitDialEventReportFile:pstDialEventReportEntry is null!");
            return VOS_ERR;
        }
    }
    else
    {
        /*创建dial_event_report虚拟文件*/
        pstDialEventReportEntry = proc_create("dial_event_report",
                                                              RNIC_VFILE_CRT_LEVEL,
                                                              pstParentFileDirEntry,
                                                              &g_stDialEventReportFileOps);

        if (VOS_NULL_PTR == pstDialEventReportEntry)
        {
            RNIC_ERROR_LOG("RNIC_InitDialEventReportFile:pstDialEventReportEntry is null!");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}


VOS_UINT32 RNIC_InitDemandDialFile(VOS_VOID)
{
    struct proc_dir_entry              *stProcDemandDialDir = NULL;
    VOS_UINT32                          ulRlst = 0;
    struct path                         path;
    int                                 error = 0;

    memset(&path, 0, sizeof(struct path));
    /*初始化拨号模式全局数据*/
    g_DialMode.enDialMode = AT_RNIC_DIAL_MODE_DEMAND_DISCONNECT;
    g_DialMode.enEventReportFlag = RNIC_FORBID_EVENT_REPORT ;
    g_DialMode.disconnect_time = DEFULT_RNIC_DISCONNECT_TIME;
    g_DialMode.is_flow = false;
   g_DialMode.is_dump_rmnet_skb = false;
    g_flux_timer.is_timer_init = false;
    rnic_flux_timer_init();
    error = kern_path("/proc/dial", LOOKUP_FOLLOW, &path);
    if (error)
    {
        stProcDemandDialDir = proc_mkdir("dial", VOS_NULL_PTR);
        if (VOS_NULL_PTR == stProcDemandDialDir)
        {
            RNIC_ERROR_LOG("RNIC_InitDemandDialFile:proc_mkdir Fail!");
            return VOS_ERR;
        }
    }
    else
    {
        stProcDemandDialDir = VOS_NULL_PTR;
        RNIC_NORMAL_LOG("RNIC_InitDemandDialFile:kern_path proc/dial is exist!");
    }

    ulRlst = RNIC_InitOnDemandFile(stProcDemandDialDir);

    if (VOS_ERR == ulRlst)
    {
        RNIC_ERROR_LOG("RNIC_InitDialEventReportFile:RNIC_InitOnDemandFile err!");
        return VOS_ERR;
    }

    ulRlst = RNIC_InitIdleTimerOutFile(stProcDemandDialDir);

    if (VOS_ERR == ulRlst)
    {
        RNIC_ERROR_LOG("RNIC_InitDialEventReportFile:RNIC_InitIdleTimerOutFile err!");
        return VOS_ERR;
    }

    ulRlst = RNIC_InitDialEventReportFile(stProcDemandDialDir);

    if (VOS_ERR == ulRlst)
    {
        RNIC_ERROR_LOG("RNIC_InitDialEventReportFile:RNIC_InitDialEventReportFile err!");
        return VOS_ERR;
    }
    ulRlst = RNIC_InitLogSwitchFile(stProcDemandDialDir);

    if (VOS_ERR == ulRlst)
    {

        RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_InitLogSwitchFile:RNIC_InitLogSwitchFile err!");//fangxiaozhi for warning

        return VOS_ERR;
    }
    ulRlst = RNIC_InitFluxFlagFile(stProcDemandDialDir);

    if (VOS_ERR == ulRlst)
    {

        RNIC_ERROR_LOG(ACPU_PID_RNIC"RNIC_InitFluxFlagFile:RNIC_InitFluxFlagFile err!");//fangxiaozhi for warning

        return VOS_ERR;
    }

    return VOS_OK;
}

void send_rnic_connect_event_to_app(void)
{

    nl_dev_type syswatch_rnic_connect = 
    {
        .device_id = DEVICE_ID_RNIC,
        .value = RNIC_DIALRPT_UP,
		.desc = NULL,
    };
    
    syswatch_nl_send( syswatch_rnic_connect );
    printk(KERN_ERR"syswatch_rnic_connect timeout send event to web sdk\n");
}
void send_rnic_disconnect_event_to_app(void)
{
    if(g_DialMode.enEventReportFlag == RNIC_ALLOW_EVENT_REPORT)
        {

           nl_dev_type syswatch_rnic_disconnect = 
            {
            .device_id = DEVICE_ID_RNIC,
            .value = RNIC_DIALRPT_DOWN,
            .desc = NULL,
            };
           syswatch_nl_send( syswatch_rnic_disconnect );

		   printk(KERN_ERR"syswatch_rnic_disconnect timeout send event to web sdk\n");
        }
    else
        {
          printk(KERN_ERR"time out but dont allow send event\n");
        }

}

