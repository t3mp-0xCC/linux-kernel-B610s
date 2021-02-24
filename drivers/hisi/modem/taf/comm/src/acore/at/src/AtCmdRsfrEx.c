

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "ATCmdProc.h"

#include <linux/fs.h>
#include <asm/uaccess.h>



/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_AT_CMDRSFREX_C

#define APP_FILE_NAME_MAXLEN 32
#define REF_NUMBER 234
#define FILE_SUM 14
#define MLOG_HEAD_MAX_LOG 100
#define MLOG_BASE64_BUF_LOG 2000
#define MLOG_DATA_BUF_LOG  (MLOG_BASE64_BUF_LOG/2)
#define SEND_DATA_MAX_LOG (MLOG_BASE64_BUF_LOG + MLOG_HEAD_MAX_LOG)

#define MLOG_LOG_SYSBOL_CFG   "mlogcfg"
#define PARTION_SHARE_STR "[SHARE]"

//文件索引
typedef enum
{
    FLIE_MLOG = 0,
    FILE_BITFLIP = 1,
    FILE_MEMTEST = 2,
    FILE_STAT,
    FILE_MINI_DUMP_0,
    FILE_MINI_DUMP_1,
    FILE_MINI_DUMP_2,
    FILE_HARDWARE_TEST,
    FILE_CBT,
    FILE_WIFI_XML,
    FILE_UPDATE_LOG_A,
    FILE_UPDATE_LOG_B,
    FLIE_INDEX_END
}file_name_index;

struct app_flie_manage_type
{
    VOS_UINT8 datafile_name[APP_FILE_NAME_MAXLEN];   /*文件名字  例如“simlock”*/
    VOS_UINT32 ref;                                  /*参考号，请参考AT文档rsfr*/
    VOS_CHAR *file_path ;                            /*文件路径*/
    VOS_UINT8 file_index ;                           /*文件的索引*/
};

//可以在A5读取的文件列表
struct app_flie_manage_type appfile_own_partion_name_tab[FILE_SUM] = 
{
    { "LOGSAVE_MLOG", REF_NUMBER, "/mobilelog/mobilelogger.log", FLIE_MLOG },
    { "BITFLIP.TAR.GZ", 527, "/mobilelog/bitflip.tar.gz", FILE_BITFLIP },
    { "MEMTEST.TAR.GZ", 927, "/mobilelog/memtest.tar.gz", FILE_MEMTEST },
    { "LOGSAVE_STAT", REF_NUMBER, "/mobilelog/mobile_stat_log.xml", FILE_STAT },
    { "LOGSAVE_DUMP_0", REF_NUMBER, "/modem_log/log/dump_00.bin", FILE_MINI_DUMP_0 },
    { "LOGSAVE_DUMP_1", REF_NUMBER, "/modem_log/log/dump_01.bin", FILE_MINI_DUMP_1 },
    { "LOGSAVE_DUMP_2", REF_NUMBER, "/modem_log/log/dump_02.bin", FILE_MINI_DUMP_2 },
    { "LOGSAVE_HARDWARE_TEST", REF_NUMBER, "/mobilelog/hardwaretest.tar.gz", FILE_HARDWARE_TEST },
    { "LOGSAVE_CBT", REF_NUMBER, "/root/lte_log/ltelogfile_1.lpd", FILE_CBT },
    { "LOGSAVE_WIFI_XML", REF_NUMBER, "/app/config/netplatform/wifi_config.xml", FILE_WIFI_XML },
    { "LOGSAVE_UPDATE_LOG_A", REF_NUMBER, "/data/huawei_dload/dload_logA.txt", FILE_UPDATE_LOG_A },
    { "LOGSAVE_UPDATE_LOG_B", REF_NUMBER, "/data/huawei_dload/dload_logB.txt", FILE_UPDATE_LOG_B }
};

struct app_flie_manage_type appfile_share_partion_name_tab[FILE_SUM] = 
{
    { "LOGSAVE_MLOG", REF_NUMBER, "/online/mobilelog/mobilelogger.log", FLIE_MLOG },
    { "BITFLIP.TAR.GZ", 527, "/online/bitflip.tar.gz", FILE_BITFLIP },
    { "MEMTEST.TAR.GZ", 927, "/online/memtest.tar.gz", FILE_MEMTEST },
    { "LOGSAVE_STAT", REF_NUMBER, "/online/mobilelog/mobile_stat_log.xml", FILE_STAT },
    { "LOGSAVE_DUMP_0", REF_NUMBER, "/modem_log/log/dump_00.bin", FILE_MINI_DUMP_0 },
    { "LOGSAVE_DUMP_1", REF_NUMBER, "/modem_log/log/dump_01.bin", FILE_MINI_DUMP_1 },
    { "LOGSAVE_DUMP_2", REF_NUMBER, "/modem_log/log/dump_02.bin", FILE_MINI_DUMP_2 },
    { "LOGSAVE_HARDWARE_TEST", REF_NUMBER, "/online/hardwaretest.tar.gz", FILE_HARDWARE_TEST },
    { "LOGSAVE_CBT", REF_NUMBER, "/root/lte_log/ltelogfile_1.lpd", FILE_CBT },
    { "LOGSAVE_WIFI_XML", REF_NUMBER, "/app/config/netplatform/wifi_config.xml", FILE_WIFI_XML },
    { "LOGSAVE_UPDATE_LOG_A", REF_NUMBER, "/data/huawei_dload/dload_logA.txt", FILE_UPDATE_LOG_A },
    { "LOGSAVE_UPDATE_LOG_B", REF_NUMBER, "/data/huawei_dload/dload_logB.txt", FILE_UPDATE_LOG_B }
};

VOS_UCHAR g_mlog_buf64[SEND_DATA_MAX_LOG];
VOS_UCHAR g_mlog_buf[MLOG_DATA_BUF_LOG + 1];
 
struct app_flie_manage_type appfile_name_tab[FILE_SUM] = {0};
static int data_index[FILE_SUM] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1};
static int g_appfile_name_init = 0;

/*****************************************************************************
  3 函数实现
*****************************************************************************/
extern VOS_VOID AtBase64Encode(const VOS_VOID* pdata,const VOS_UINT32 data_size,VOS_VOID* out_pcode);

VOS_CHAR *AT_strstr(const VOS_CHAR *strSrc, const VOS_CHAR *str)   
{   
    //assert(strSrc != NULL && str != NULL);  
    if( VOS_NULL_PTR == strSrc ||  VOS_NULL_PTR == str )
    {
        return VOS_NULL_PTR;
    }    
    const VOS_CHAR *s = strSrc;   
    const VOS_CHAR *t = str;   
    for (; *strSrc != '\0'; ++strSrc)   
    {   
        for (s = strSrc, t = str; *t != '\0' && *s == *t; ++s, ++t)
        {
            VOS_NULL_PTR;
        }
        if (*t == '\0')
        {
            return (VOS_CHAR *) strSrc;
        }
    }   
    return VOS_NULL_PTR;   
}

VOS_INT32 AT_atoi(VOS_CHAR *str)
{
    VOS_INT32 ret_int = 0;
    if( VOS_NULL_PTR == str )
    {
        return 0;
    }
    
    while(*str >= '0' && *str <= '9')
    {
        ret_int = ret_int * 10 + (*str - '0');
        str++;
    }
    if(*str == '\0')
    {
        return ret_int;
    }
    else
    {
        return 0;
    }
} 

VOS_INT32 mlog_use_share_partion()
{
    VOS_UINT32 file_size = 0;
    VOS_INT32 ret = 0;
    VOS_INT32 i = 0;
    VOS_UCHAR g_mlog_buf[MLOG_DATA_BUF_LOG];
    VOS_UCHAR * str = VOS_NULL_PTR;
    
    struct file *filp = VOS_NULL_PTR;
    struct inode *inode = VOS_NULL_PTR;
    loff_t *pos;
    mm_segment_t old_fs;
    
    filp = filp_open(MLOG_LOG_SYSBOL_CFG, O_RDONLY , 0);
    if( VOS_NULL_PTR == filp || IS_ERR(filp))
    {
        return VOS_ERROR;
    }
    
    //获取文件的大小
    inode = filp->f_dentry->d_inode;
    file_size = inode->i_size;
    pos = &(filp->f_pos);
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_read(filp, g_mlog_buf, file_size, pos);
    if(ret <= 0)
    {
        return VOS_ERROR;
    }
    set_fs(old_fs);
    filp_close(filp, NULL);

    str = g_mlog_buf;
    str = AT_strstr(str, PARTION_SHARE_STR);
    if( VOS_NULL_PTR == str)
    {
        return VOS_ERROR;
    }
    str = AT_strstr(str, "=");
    if( VOS_NULL_PTR == str)
    {
        return VOS_ERROR;
    }
    str++;
    if(*str == '0')
    {
        return VOS_ERROR;
    }
    else if(*str == '1')
    {
        return VOS_OK;
    }
    
}

VOS_UINT32 AT_RsfrExParse(VOS_CHAR *cName, VOS_UINT8 * file_name_index)
{
    VOS_UINT8 i = 0;
    if ( VOS_NULL_PTR == cName || VOS_NULL_PTR == file_name_index )
    {
        return VOS_ERR;
    }
    for(i = 0; i < FILE_SUM; i++)
    {
        if(ERR_MSP_SUCCESS == AT_STRCMP(cName, appfile_own_partion_name_tab[i].datafile_name))
        {
            *file_name_index = i;
            return VOS_OK;
        }
    }
    return VOS_ERR;
}

VOS_UINT32 AT_RsfrExData(VOS_UINT8  f_index, VOS_CHAR *cSubName)
{
    VOS_UINT32 file_size = 0;
    VOS_UINT32 data_length = MLOG_DATA_BUF_LOG ;
    VOS_UINT32 buf_index = 0;
    VOS_UINT32 buf_index_max = 0;
    VOS_UINT32 buf_index_rem = 0;
    VOS_INT32 ret = 0;
    VOS_CHAR *read_file_path = VOS_NULL_PTR;
    
    struct file *filp = VOS_NULL_PTR;
    struct inode *inode = VOS_NULL_PTR;
    loff_t *pos;
    mm_segment_t old_fs;
    
    if(f_index >= FILE_SUM)
    {
        return VOS_ERR;
    }
    if (0 == g_appfile_name_init)
    {
        if(VOS_OK == mlog_use_share_partion())
        {
            VOS_MemCpy(appfile_name_tab, appfile_share_partion_name_tab, sizeof(appfile_name_tab));
        }
        else
        {
            VOS_MemCpy(appfile_name_tab, appfile_own_partion_name_tab, sizeof(appfile_name_tab));
        }
        g_appfile_name_init = 1;
    }
    
    if( f_index != appfile_name_tab[f_index].file_index)
    {
        return VOS_ERR;
    }
    
    read_file_path = appfile_name_tab[f_index].file_path;

    VOS_MemSet(g_mlog_buf64, 0, sizeof(g_mlog_buf64));
    VOS_MemSet(g_mlog_buf, 0, sizeof(g_mlog_buf));
    
    filp = filp_open(read_file_path, O_RDONLY , 0);
    if( VOS_NULL_PTR == filp || IS_ERR(filp))
    {
        return VOS_ERR;
    }
    
    //获取文件的大小
    inode = filp->f_dentry->d_inode;
    file_size = inode->i_size;
    //计算拆分数据总包
    buf_index_max = file_size / MLOG_DATA_BUF_LOG;
    buf_index_rem = file_size % MLOG_DATA_BUF_LOG;
    if ( 0 != buf_index_rem)
    {
        buf_index_max++;
    }
    buf_index = AT_atoi(cSubName);
    if ( buf_index > buf_index_max )
    {
        filp_close(filp, NULL); 
        return VOS_ERR;
    }
    if ( 0 == buf_index )
    {
        buf_index = data_index[f_index];
    }
    else
    {
        data_index[f_index] = buf_index;
    }
    if ( buf_index == buf_index_max )
    {
        if ( 0 != buf_index_rem)
        {
            data_length = buf_index_rem ;
        }
    }
    filp->f_pos  = (loff_t)(buf_index - 1) * MLOG_DATA_BUF_LOG;
    pos = &(filp->f_pos);
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_read(filp, g_mlog_buf, data_length, pos);

    if(ret <= 0)
    {
        set_fs(old_fs);
        filp_close(filp, NULL); 
        return VOS_ERR;
    }
    
    AtBase64Encode(g_mlog_buf, data_length, g_mlog_buf64);
    
    gstAtSendData.usBufLen = (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                (VOS_CHAR *)pgucAtSndCodeAddr,
                                "^RSFR:\"%s\",\"%s\",%d,%d,%d,",
                                appfile_name_tab[f_index].datafile_name,
                                "",
                                appfile_name_tab[f_index].ref,
                                buf_index_max,
                                buf_index
                                );

    gstAtSendData.usBufLen += (VOS_UINT16)At_sprintf(AT_CMD_MAX_LEN, (VOS_CHAR *)pgucAtSndCodeAddr,
                                (VOS_CHAR *)pgucAtSndCodeAddr + gstAtSendData.usBufLen,
                                "\"%s\"", g_mlog_buf64);
    
    data_index[f_index]++;
    
    if ( data_index[f_index] > (int)buf_index_max)
    {
        data_index[f_index] = 1;
    }
    
    set_fs(old_fs);
    filp_close(filp, NULL); 
    return VOS_OK;
}
