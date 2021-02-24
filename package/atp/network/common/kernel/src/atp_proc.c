
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>
#include <linux/if.h>
#include <asm/uaccess.h>
#ifdef CONFIG_ATP_PRSITE 
#include "atp_prsite.h"
#endif

#ifdef CONFIG_ATP_GUESTWIFI_ACCESS_UI 
#include "atp_guestwifiaccessui.h"
#endif

#define ATP_PROC_DEBUG(msg, ...)    printk(KERN_DEBUG " [%s] [%d] [%s] "msg"\r\n", __FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
#define ATP_PROC_ERROR(msg, ...)    printk(KERN_ERR " [%s] [%d] [%s] "msg"\r\n", __FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
#define ATP_PROC_SAFE_PTR( ptr)     ( (ptr) ? (ptr) : "NULL")
#define ATP_PROC_FILE_VALUE_LENGTH             8  
#define ATP_PROC_DIR_NAME               "app_proc"
#define ATP_PROC_DIR                    "/proc/"ATP_PROC_DIR_NAME
static struct proc_dir_entry *g_AtpProcDirEntry = NULL;
typedef int (*ProcFileRead)(struct file *fp, char *buffer, size_t len, loff_t *offset);
typedef int (*ProcFileWrite)(struct file *fp, const char *userBuf, size_t len, loff_t *off); 

int atp_proc_create_file(struct proc_dir_entry *dir_entry,
                         char *file,
                         struct proc_dir_entry **file_entry,
                         struct file_operations *pfile_proc_fops)
{
    ATP_PROC_DEBUG("will create file: %s", ATP_PROC_SAFE_PTR(file));

    if (NULL == pfile_proc_fops)
    {        
        ATP_PROC_DEBUG("pfile_proc_fops is NULL");
        return -1;    
    }

    if (NULL != dir_entry)
    {        
        *file_entry = proc_create(file, 0664, dir_entry, pfile_proc_fops);
        if (NULL != *file_entry)
        {
            return 0;
        }
        else
        {
            ATP_PROC_DEBUG("create_proc_entry failed");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("dir_entry null");
        return -1;
    }
}

#ifdef CONFIG_ATP_PRSITE 
#define ATP_PROC_PRSITE_DIR_NAME        "prsite"
#define ATP_PROC_PRSITE_DIR             ATP_PROC_DIR"/prsite"
#define ATP_PROC_PRSITE_ADDR_FILE       "addr"
#define ATP_PROC_PRSITE_MASK_FILE       "mask"
#define ATP_PROC_PRSITE_ACTIONMODE_FILE "actionmode"
#define ATP_PROC_PRSITE_LOCATION_FILE   "location"
#define ATP_PROC_PRSITE_RAND_FILE       "rand"
#define ATP_PROC_PRSITE_TIMES_FILE      "times"


//multimac
/* 多用户强制门户开关文件 */
#define ATP_PROC_PRSITE_MULTIUSER_ENABLE_FILE        "multiuserenable"
/* 多用户强制门户删除某个mac文件 */
#define ATP_PROC_PRSITE_MULTIUSER_MACLIST_FILE       "delmac"

#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) 
#define ATP_PROC_PRSITE_LOCATION_GUESTWIFI_FILE   "locationguestwifi"
#endif

#define ATP_PROC_PRSITE_DATA_ADDR       0
#define ATP_PROC_PRSITE_DATA_MASK       1
#define ATP_PROC_PRSITE_DATA_ACTIONMODE 2
#define ATP_PROC_PRSITE_DATA_LOCATION   3
#define ATP_PROC_PRSITE_DATA_RAND       4
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) 
#define ATP_PROC_PRSITE_DATA_LOCATION_GUESTWIFI   5
#endif

//multimac
/* 多用户强制门户开关分类号，续接上面的定义 */
#define ATP_PROC_PRSITE_DATA_MULTIUSERENABLE        6
/* 多用户强制门户删除某个mac分类号 */
#define ATP_PROC_PRSITE_DATA_MULTIUSERMACLIST       7

#define ATP_PROC_PRSITE_DATA_TIMES       8

#define PRSITE_BUFF_LEN_MAX             64

static struct proc_dir_entry *g_AtpProcPrsiteDirEntry = NULL;
static struct proc_dir_entry *g_AtpProcPrsiteAddrEntry = NULL;
static struct proc_dir_entry *g_AtpProcPrsiteMaskEntry = NULL;
static struct proc_dir_entry *g_AtpProcPrsiteActionModeEntry = NULL;
static struct proc_dir_entry *g_AtpProcPrsiteLocationEntry = NULL;
static struct proc_dir_entry *g_AtpProcPrsiteRandEntry = NULL;
static struct proc_dir_entry *g_AtpProcPrsiteTimeEntry = NULL;


//multimac
/* 多用户强制门户开关文件处理入口 */
static struct proc_dir_entry *g_AtpProcPrsiteMultiUserEnableEntry  = NULL;
/* 多用户强制门户删除某个mac文件处理入口 */
static struct proc_dir_entry *g_AtpProcPrsiteMultiUserMaclistEntry = NULL;

#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) 
static struct proc_dir_entry *g_AtpProcPrsiteLocationGuestwifiEntry = NULL;
#endif

extern struct prsite_url_info g_stPrsiteUrlInfo;
extern int g_prsitenum ;
static ssize_t atp_proc_prsite_read(struct file *fp, char *buffer, size_t len, loff_t *offset, int datatype) 
{
    static int finished = 0;
    int        ret = 0;
    char       tempbuff[HTTP_URL_MAX] = {0};

    if(*offset >0) 
    {
        ATP_PROC_DEBUG("atp_proc_prsite_read already finished.\n");
        return 0;
    }        

    switch (datatype)
    {
        case ATP_PROC_PRSITE_DATA_ADDR:
             snprintf(tempbuff, sizeof(tempbuff), "%d", g_stPrsiteUrlInfo.ul_lan_addr);
             break;
        case ATP_PROC_PRSITE_DATA_MASK:
             snprintf(tempbuff, sizeof(tempbuff), "%d", g_stPrsiteUrlInfo.ul_lan_mask);
             break;
        case ATP_PROC_PRSITE_DATA_ACTIONMODE:
             snprintf(tempbuff, sizeof(tempbuff), "%d", g_stPrsiteUrlInfo.lEnable);
             break;
        case ATP_PROC_PRSITE_DATA_LOCATION:
             snprintf(tempbuff, sizeof(tempbuff), "%s", g_stPrsiteUrlInfo.ac_stb_url);
             break;
		case ATP_PROC_PRSITE_DATA_RAND:
			 ret = showrandall(buffer,len);
             *offset += ret;
			 return ret;
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) 
        case ATP_PROC_PRSITE_DATA_LOCATION_GUESTWIFI:
            snprintf(tempbuff, sizeof(tempbuff), "%s", g_stPrsiteUrlInfo.ac_stb_url_guestwifi);
            break;
#endif
        //multimac
        case ATP_PROC_PRSITE_DATA_MULTIUSERENABLE:
            //获取多用户强制门户开关
            snprintf(tempbuff, sizeof(tempbuff), "%d", g_stPrsiteUrlInfo.lMultiuserEnable);
            break;
        case ATP_PROC_PRSITE_DATA_MULTIUSERMACLIST:
            //获取多用户强制门MAC表
            showmaclist();
            return 0;
		case ATP_PROC_PRSITE_DATA_TIMES:
            //当前重定向次数
			snprintf(tempbuff, sizeof(tempbuff), "%d", g_prsitenum);			
        default:
             break;
    }
    ret = strlen(tempbuff) + 1;
    if(copy_to_user(buffer, tempbuff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",tempbuff);
        return -1;
    }
    *offset += ret;
    ATP_PROC_DEBUG("atp_proc_prsite_read Read: %s\n", tempbuff);    
    return ret;
}

static ssize_t atp_proc_prsite_write(struct file *fp, const char *userBuf, size_t len, loff_t *off, int datatype) 
{    
    char temp_data[PRSITE_BUFF_LEN_MAX] = {0};
    int  ret = 0;

    if ((NULL == userBuf) || (len > (PRSITE_BUFF_LEN_MAX - 1)))
    {
        ATP_PROC_ERROR("buffer is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, len))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }
    temp_data[PRSITE_BUFF_LEN_MAX-1] = 0;
    ret = strlen(temp_data);

    switch (datatype)
    {
        case ATP_PROC_PRSITE_DATA_ADDR:
             g_stPrsiteUrlInfo.ul_lan_addr = simple_strtoul(temp_data, NULL, 0);
             break;
        case ATP_PROC_PRSITE_DATA_MASK:
             g_stPrsiteUrlInfo.ul_lan_mask = simple_strtoul(temp_data, NULL, 0);
             break;
        case ATP_PROC_PRSITE_DATA_ACTIONMODE:
             g_stPrsiteUrlInfo.lEnable = simple_strtoul(temp_data, NULL, 0);
			 if(g_stPrsiteUrlInfo.lEnable == 0)
             {
                 delrandall();
             }
			 g_prsitenum = 0;
             break;
        case ATP_PROC_PRSITE_DATA_LOCATION:
             snprintf(g_stPrsiteUrlInfo.ac_stb_url, sizeof(g_stPrsiteUrlInfo.ac_stb_url), 
                      "%s", 
                      temp_data);
             break;
		case ATP_PROC_PRSITE_DATA_RAND:
			 delrandbyid(temp_data);
			 break;
        //multimac
        case ATP_PROC_PRSITE_DATA_MULTIUSERENABLE:
            //设置多用户强制门户开关
            g_stPrsiteUrlInfo.lMultiuserEnable = simple_strtoul(temp_data, NULL, 0);
            break;
        case ATP_PROC_PRSITE_DATA_MULTIUSERMACLIST:
            //删除多用户强制门MAC表中的某一项
            delmaclistbymac(temp_data);
            break;
        case ATP_PROC_PRSITE_DATA_TIMES:
		    g_prsitenum = simple_strtoul(temp_data, NULL, 10);
		    break;
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) 
        case ATP_PROC_PRSITE_DATA_LOCATION_GUESTWIFI:
            snprintf(g_stPrsiteUrlInfo.ac_stb_url_guestwifi, sizeof(g_stPrsiteUrlInfo.ac_stb_url_guestwifi), 
                    "%s", 
                    temp_data);
            break;
#endif
        default:
             break;
    }

    /* mac地址不能打log */
    if (ATP_PROC_PRSITE_DATA_MULTIUSERMACLIST != datatype)
    {
        ATP_PROC_ERROR("atp_proc_prsite_write: %s,  g_stPrsiteUrlInfo.lEnable:%d\n", temp_data,  g_stPrsiteUrlInfo.lEnable);    
    }
    return ret;
}

static ssize_t atp_proc_prsite_addr_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{ 
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_addr_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_ADDR);
    return ret;
}
static ssize_t atp_proc_prsite_mask_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_mask_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_MASK);
    return ret;
}
static ssize_t atp_proc_prsite_actionmode_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_actionmode_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_ACTIONMODE);
    return ret;
}
static ssize_t atp_proc_prsite_location_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_location_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_LOCATION);
    return ret;
}

static ssize_t atp_proc_prsite_time_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_time_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_TIMES);
    return ret;
}


static ssize_t atp_proc_prsite_Rand_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_Rand_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_RAND);
    return ret;
}

//multimac
/* 多用户强制门户开关文件读取接口 */
static ssize_t atp_proc_prsite_multi_user_enable_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_multi_user_enable_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_MULTIUSERENABLE);
    return ret;
}
/* 多用户强制门户删除某个mac文件读取接口 */
static ssize_t atp_proc_prsite_multi_user_maclist_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_multi_user_maclist_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_MULTIUSERMACLIST);
    return ret;
}

static ssize_t atp_proc_prsite_addr_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_addr_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_ADDR);
    return ret;
}
static ssize_t atp_proc_prsite_mask_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_mask_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_MASK);
    return ret;
}
static ssize_t atp_proc_prsite_actionmode_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_actionmode_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_ACTIONMODE);
    return ret;
}

static ssize_t atp_proc_prsite_time_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_time_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_TIMES);
    return ret;
}

static ssize_t atp_proc_prsite_location_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_location_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_LOCATION);
    return ret;
}
static ssize_t atp_proc_prsite_Rand_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;
	
    ATP_PROC_DEBUG("atp_proc_prsite_location_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_RAND);
    return ret;
}
//multimac
/* 多用户强制门户开关文件写接口 */
static ssize_t atp_proc_prsite_multi_user_enable_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_multi_user_enable_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_MULTIUSERENABLE);
    return ret;
}
/* 多用户强制门户删除某个mac文件写接口 */
static ssize_t atp_proc_prsite_multi_user_maclist_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_multi_user_maclist_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_MULTIUSERMACLIST);
    return ret;
}

#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) 
static ssize_t atp_proc_prsite_location_guestwifi_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_location_guestwifi_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_LOCATION_GUESTWIFI);
    return ret;
}

static ssize_t atp_proc_prsite_location_guestwifi_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_location_guestwifi_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_LOCATION_GUESTWIFI);
    return ret;
}


#endif

struct file_operations g_file_proc_fops_prsite_actionmode = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_actionmode_read,
                           .write = atp_proc_prsite_actionmode_write,                           
                           }; 
						   
struct file_operations g_file_proc_fops_prsite_time = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_time_read,
                           .write = atp_proc_prsite_time_write,                           
                           };						   
						   

struct file_operations g_file_proc_fops_prsite_addr = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_addr_read,
                           .write = atp_proc_prsite_addr_write,                           
                           }; 

struct file_operations g_file_proc_fops_prsite_mask = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_mask_read,
                           .write = atp_proc_prsite_mask_write,                           
                           }; 

struct file_operations g_file_proc_fops_prsite_location = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_location_read,
                           .write = atp_proc_prsite_location_write,                           
                           }; 

struct file_operations g_file_proc_fops_prsite_Rand = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_Rand_read,
                           .write = atp_proc_prsite_Rand_write,                           
                           }; 

//multimac
/* 多用户强制门户开关文件操作总入口 */
struct file_operations g_file_proc_fops_prsite_multi_user_enable = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_multi_user_enable_read,
                           .write = atp_proc_prsite_multi_user_enable_write,                           
                           }; 
/* 多用户强制门户删除某个mac文件操作总入口 */
struct file_operations g_file_proc_fops_prsite_multi_user_maclist = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_multi_user_maclist_read,
                           .write = atp_proc_prsite_multi_user_maclist_write,                           
                           }; 

#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03)                                                      
struct file_operations g_file_proc_fops_prsite_location_guestwifi = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_location_guestwifi_read,
                           .write = atp_proc_prsite_location_guestwifi_write,                           
                           };

#endif

int atp_proc_create_prsite(void)
{
    int error = 0;
    struct path proc_path;

    error = kern_path(ATP_PROC_PRSITE_DIR, LOOKUP_FOLLOW, &proc_path);
    if (error)
    {
        ATP_PROC_DEBUG("%s NOT exist !!!\n", ATP_PROC_PRSITE_DIR);
        g_AtpProcPrsiteDirEntry = proc_mkdir(ATP_PROC_PRSITE_DIR_NAME, g_AtpProcDirEntry);
        if (NULL == g_AtpProcPrsiteDirEntry)
        {
            ATP_PROC_ERROR("proc_mkdir failed ");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("%s already exist !\n", ATP_PROC_PRSITE_DIR);
        return 0;
    }

    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_ADDR_FILE,
                         &g_AtpProcPrsiteAddrEntry,
                         &g_file_proc_fops_prsite_addr);
    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_MASK_FILE,
                         &g_AtpProcPrsiteMaskEntry,
                         &g_file_proc_fops_prsite_mask);
    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_ACTIONMODE_FILE,
                         &g_AtpProcPrsiteActionModeEntry,
                         &g_file_proc_fops_prsite_actionmode);
    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_LOCATION_FILE,
                         &g_AtpProcPrsiteLocationEntry,
                         &g_file_proc_fops_prsite_location);

    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_TIMES_FILE,
                         &g_AtpProcPrsiteTimeEntry,
                         &g_file_proc_fops_prsite_time);

    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_RAND_FILE,
                         &g_AtpProcPrsiteRandEntry,
                         &g_file_proc_fops_prsite_Rand);

    //multimac
    /* 多用户强制门户开关文件创建 */
    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_MULTIUSER_ENABLE_FILE,
                         &g_AtpProcPrsiteMultiUserEnableEntry,
                         &g_file_proc_fops_prsite_multi_user_enable);
    /* 多用户强制门户删除某个mac文件创建 */
    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_MULTIUSER_MACLIST_FILE,
                         &g_AtpProcPrsiteMultiUserMaclistEntry,
                         &g_file_proc_fops_prsite_multi_user_maclist);

#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03) 
    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_LOCATION_GUESTWIFI_FILE,
                         &g_AtpProcPrsiteLocationGuestwifiEntry,
                         &g_file_proc_fops_prsite_location_guestwifi);
#endif

    return 0;
}
#endif

#ifndef FEATURE_ON
#define FEATURE_ON      0
#endif
#ifndef MBB_CTF_COMMON
#define MBB_CTF_COMMON  FEATURE_ON + 1
#endif
#ifndef MBB_FEATURE_FASTIP
#define MBB_FEATURE_FASTIP FEATURE_ON + 1
#endif
#ifndef MBB_FEATURE_ETH_WAN_MIRROR
#define MBB_FEATURE_ETH_WAN_MIRROR  FEATURE_ON + 1
#endif

#if ((MBB_CTF_COMMON == FEATURE_ON) || (MBB_FEATURE_FASTIP == FEATURE_ON)) 
#define ATP_PROC_CTF_DIR_NAME           "ctf"
#define ATP_PROC_CTF_DIR                ATP_PROC_DIR"/ctf"
#define ATP_PROC_CTF_SWITCH_FILE        "ctf_switch_route"
#define ATP_PROC_CTF_MIRROR_FILE        "ctf_switch_mirror"
#define ATP_CTF_SWITCH_ROUTE_BIT        0    /* 这一位用于 路由控制CTF的开关*/
#define ATP_CTF_WAN_MIRROR_BIT          1    /* 这一位用于 镜像抓包控制CTF的开关*/
#define ATP_CTF_FILE_LENGTH             8  /* ifctf的文件内容最大长度*/
#define ATP_CTF_DECIMAL                 10
static struct proc_dir_entry *g_AtpProcCtfDirEntry = NULL;
static struct proc_dir_entry *g_AtpProcCtfSwitchFileEntry = NULL;
#if (MBB_CTF_COMMON == FEATURE_ON)
extern unsigned int g_ifctf;
#endif
#if (MBB_FEATURE_FASTIP == FEATURE_ON)
extern unsigned int fastip_run_flag;
#endif
void atp_proc_set_special_bit(unsigned int * flags, int pos, int value)
{
    if (0 == value)
    {
        (*flags) &= ~(0x1 << pos);
    }
    else
    {
        (*flags) |= (0x1 << pos);
    }
}

int atp_proc_get_special_bit(unsigned int flags, int pos)
{
    return ((flags >> pos) & 0x1);
}

static ssize_t atp_proc_ctf_read(struct file *fp, char *buffer, size_t len, loff_t *offset, int flag)
{
    int        ret = 0;
    char       tempbuff[ATP_CTF_FILE_LENGTH] = {0};
    
    if(*offset >0) 
    {
        ATP_PROC_DEBUG("atp_proc_ctf_read already finished.\n");
        return 0;
    }
    
#if (MBB_CTF_COMMON == FEATURE_ON)
    snprintf(tempbuff, sizeof(tempbuff), "%d\n", atp_proc_get_special_bit(g_ifctf, flag));
#endif
#if (MBB_FEATURE_FASTIP == FEATURE_ON)
    snprintf(tempbuff, sizeof(tempbuff), "%d\n", atp_proc_get_special_bit(fastip_run_flag, flag));
#endif

    ret = strlen(tempbuff) + 1;
    if(copy_to_user(buffer, tempbuff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",tempbuff);
        return -1;
    }
    *offset += ret;
    
    ATP_PROC_DEBUG("tempbuff %s", tempbuff);

    return ret;
}

static ssize_t atp_proc_ctf_write(struct file *fp, const char *userBuf, size_t len, loff_t *off, int flag)
{
    char temp_data[ATP_CTF_FILE_LENGTH] = {0};
    int  set_value = 0;
    int  ret = 0;
    
    if ((NULL == userBuf) || (len > sizeof(temp_data) - 1))
    {
        ATP_PROC_ERROR("userBuf is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, (sizeof(temp_data) - 1)))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }

    ret = strlen(temp_data);
    set_value = simple_strtol(temp_data, NULL, ATP_CTF_DECIMAL); 
    
#if (MBB_CTF_COMMON == FEATURE_ON)
    ATP_PROC_DEBUG("user data is %s, g_ifctf is %u", temp_data, g_ifctf);
    atp_proc_set_special_bit(&g_ifctf, flag, set_value);
    ATP_PROC_DEBUG("write g_ifctf %u", g_ifctf);
#endif
    
#if (MBB_FEATURE_FASTIP == FEATURE_ON)
    ATP_PROC_DEBUG("user data is %s, fastip_run_flag is %u", temp_data, fastip_run_flag);
    atp_proc_set_special_bit(&fastip_run_flag, flag, set_value);
    ATP_PROC_DEBUG("fastip_run_flag %u", fastip_run_flag);
#endif
    
    return ret;
}

static ssize_t atp_proc_switch_ctf_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int    ret = 0;    

    ATP_PROC_DEBUG("now read for router_ctf ...");

    ret = atp_proc_ctf_read(fp,buffer,len,offset,ATP_CTF_SWITCH_ROUTE_BIT);

    return ret;
}

static ssize_t atp_proc_mirror_ctf_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int    ret = 0;    

    ATP_PROC_DEBUG("now read for mirror_ctf ...");

    ret = atp_proc_ctf_read(fp,buffer,len,offset,ATP_CTF_WAN_MIRROR_BIT);

    return ret;
}

static ssize_t atp_proc_switch_ctf_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int    ret = 0;    

    ATP_PROC_DEBUG("now write for router_ctf ...");

    ret = atp_proc_ctf_write(fp,userBuf,len,off,ATP_CTF_SWITCH_ROUTE_BIT);

    return ret;
}

#if (MBB_FEATURE_ETH_WAN_MIRROR == FEATURE_ON)
static struct proc_dir_entry *g_AtpProcCtfMirrorFileEntry = NULL;

static ssize_t atp_proc_mirror_ctf_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int    ret = 0;    

    ATP_PROC_DEBUG("now write for mirror_ctf ...");

    ret = atp_proc_ctf_write(fp,userBuf,len,off,ATP_CTF_WAN_MIRROR_BIT);

    return ret;
}

void set_ctf_wan_mirror_flags(int value)
{
#if (MBB_CTF_COMMON == FEATURE_ON)
    atp_proc_set_special_bit(&g_ifctf, ATP_CTF_WAN_MIRROR_BIT, value);
#endif
#if (MBB_FEATURE_FASTIP == FEATURE_ON)
    atp_proc_set_special_bit(&fastip_run_flag, ATP_CTF_WAN_MIRROR_BIT, value);
#endif
}

struct file_operations g_file_proc_fops_ctf_mirror = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_mirror_ctf_read,
                           .write = atp_proc_mirror_ctf_write,                           
                           }; 

#endif
struct file_operations g_file_proc_fops_ctf_route = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_switch_ctf_read,
                           .write = atp_proc_switch_ctf_write,                           
                           }; 

int atp_proc_create_ctf(void)
{
    int error = 0;
    struct path proc_path;

    error = kern_path(ATP_PROC_CTF_DIR, LOOKUP_FOLLOW, &proc_path);
    if (error)
    {
        ATP_PROC_DEBUG("%s NOT exist !!!\n", ATP_PROC_CTF_DIR);
        g_AtpProcCtfDirEntry = proc_mkdir(ATP_PROC_CTF_DIR_NAME, g_AtpProcDirEntry);
        if (NULL == g_AtpProcCtfDirEntry)
        {
            ATP_PROC_ERROR("proc_mkdir failed for ctf");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("%s already exist !\n", ATP_PROC_CTF_DIR);
        return 0;
    }

    atp_proc_create_file(g_AtpProcCtfDirEntry,
                         ATP_PROC_CTF_SWITCH_FILE,
                         &g_AtpProcCtfSwitchFileEntry,
                         &g_file_proc_fops_ctf_route);
#if (MBB_FEATURE_ETH_WAN_MIRROR == FEATURE_ON)
    atp_proc_create_file(g_AtpProcCtfDirEntry,
                         ATP_PROC_CTF_MIRROR_FILE,
                         &g_AtpProcCtfMirrorFileEntry,
                         &g_file_proc_fops_ctf_mirror);
#endif
    
    return 0;
}
#endif

#ifdef CONFIG_ATP_ONDEMAND_DIAL
#define ATP_PROC_DIAL_DIR_NAME           "dial"
#define ATP_PROC_DIAL_DIR                ATP_PROC_DIR"/dial"
#define ATP_PROC_DIAL_WLAN_NAME_FILE        "wlan_dev_name"
#define ATP_PROC_DIAL_ETH_ISLAN_FILE        "eth_is_lan"
#define ATP_PROC_DIAL_REPORT_TIMEOUT_FILE   "report_timeout"

#define WLAN_DEV_NAME_LEN   64
static struct proc_dir_entry *g_AtpProcDialDirEntry = NULL;
static struct proc_dir_entry *g_AtpProcDialWlanNameFileEntry = NULL;
static struct proc_dir_entry *g_AtpProcDialEthIsLanFileEntry = NULL;
static struct proc_dir_entry *g_AtpProcDialReportTimeoutFileEntry = NULL;

/*记录WLAN设置名*/
char g_dial_wlan_name[WLAN_DEV_NAME_LEN] = {0};
/*记录eth0是否为lan设备*/
char g_dial_eth_is_lan[IFNAMSIZ] = {0};
/*REDIRECT上报间隔时间*/
int g_report_time = 5;
extern void RNIC_PraseWlanDevName(char* namebuf);

static ssize_t atp_proc_dial_wlan_name_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{ 
    int ret = 0;
    char       temp_buff[WLAN_DEV_NAME_LEN] = {0};

    if(*offset > 0) 
    {
        ATP_PROC_DEBUG("atp_proc_dial_wlan_name_readalready finished. \n");
        return 0;
    }
    
    ATP_PROC_DEBUG("atp_proc_dial_wlan_name_read \n");

    snprintf(temp_buff, sizeof(temp_buff), "%s", g_dial_wlan_name);

    ret = strlen(temp_buff) + 1;
    if(copy_to_user(buffer, temp_buff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",temp_buff);
        return -1;
    }
    *offset += len;
	return ret;
}
static ssize_t atp_proc_dial_wlan_name_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;
    char temp_data[WLAN_DEV_NAME_LEN] = {0};

    ATP_PROC_DEBUG("atp_proc_dial_wlan_name_write \n");

    if ((NULL == userBuf) || (len > (WLAN_DEV_NAME_LEN - 1)))
    {
        ATP_PROC_ERROR("buffer is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, len))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }
    temp_data[WLAN_DEV_NAME_LEN-1] = 0;
    ret = strlen(temp_data);

    snprintf(g_dial_wlan_name, sizeof(g_dial_wlan_name), "%s", temp_data);
    RNIC_PraseWlanDevName((char *)g_dial_wlan_name);
    return ret;
}



static ssize_t atp_proc_dial_report_timeout_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{ 
    int        ret = 0;
    char       temp_buff[IFNAMSIZ] = {0};
    
    if(*offset > 0) 
    {
        ATP_PROC_DEBUG("atp_proc_dial_report_timeout_read finished.\n");
        return 0;
    }
    
    ATP_PROC_DEBUG("atp_proc_dial_report_timeout_read \n");

    snprintf(temp_buff, sizeof(temp_buff), "%d", g_report_time);

    ret = strlen(temp_buff) + 1;
    if(copy_to_user(buffer, temp_buff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",temp_buff);
        return -1;
    }
    *offset += ret;
    return ret;
}
static ssize_t atp_proc_dial_report_timeout_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;
    char temp_data[IFNAMSIZ] = {0};

    ATP_PROC_DEBUG("atp_proc_dial_eth_lan_write \n");

    if ((NULL == userBuf) || (len > (IFNAMSIZ - 1)))
    {
        ATP_PROC_ERROR("buffer is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, len))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }

    temp_data[IFNAMSIZ-1] = 0;

    ret = strlen(temp_data);
	
	g_report_time = simple_strtol(temp_data, NULL, 10); 
	
//    snprintf(g_report_time, sizeof(g_report_time), "%s", temp_data);
    return ret;
}





static ssize_t atp_proc_dial_eth_lan_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{ 
    int        ret = 0;
    char       temp_buff[IFNAMSIZ] = {0};
    
    if(*offset > 0) 
    {
        ATP_PROC_DEBUG("atp_proc_dial_eth_lan_read finished.\n");
        return 0;
    }
    
    ATP_PROC_DEBUG("atp_proc_dial_eth_lan_read \n");

    snprintf(temp_buff, sizeof(temp_buff), "%s", g_dial_eth_is_lan);

    ret = strlen(temp_buff) + 1;
    if(copy_to_user(buffer, temp_buff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",temp_buff);
        return -1;
    }
    *offset += ret;
    return ret;
}
static ssize_t atp_proc_dial_eth_lan_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;
    char temp_data[IFNAMSIZ] = {0};

    ATP_PROC_DEBUG("atp_proc_dial_eth_lan_write \n");

    if ((NULL == userBuf) || (len > (IFNAMSIZ - 1)))
    {
        ATP_PROC_ERROR("buffer is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, len))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }

    temp_data[IFNAMSIZ-1] = 0;

    ret = strlen(temp_data);

    snprintf(g_dial_eth_is_lan, sizeof(g_dial_eth_is_lan), "%s", temp_data);
    return ret;
}
struct file_operations g_file_proc_fops_dial_wlan_name = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_dial_wlan_name_read,
                           .write = atp_proc_dial_wlan_name_write,                           
                           }; 
struct file_operations g_file_proc_fops_dial_eth_lan = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_dial_eth_lan_read,
                           .write = atp_proc_dial_eth_lan_write,                           
                           }; 

struct file_operations g_file_proc_fops_report_timeout = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_dial_report_timeout_read,
                           .write = atp_proc_dial_report_timeout_write,                           
                           }; 

int atp_proc_create_dial(void)
{
    int error = 0;
    struct path proc_path;

    error = kern_path(ATP_PROC_DIAL_DIR, LOOKUP_FOLLOW, &proc_path);
    if (error)
    {
        ATP_PROC_DEBUG("%s NOT exist !!!\n", ATP_PROC_DIAL_DIR);
        g_AtpProcDialDirEntry = proc_mkdir(ATP_PROC_DIAL_DIR_NAME, g_AtpProcDirEntry);
        if (NULL == g_AtpProcDialDirEntry)
        {
            ATP_PROC_ERROR("proc_mkdir failed for ctf");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("%s already exist !\n", ATP_PROC_DIAL_DIR);
        return 0;
    }

    atp_proc_create_file(g_AtpProcDialDirEntry,
                         ATP_PROC_DIAL_WLAN_NAME_FILE,
                         &g_AtpProcDialWlanNameFileEntry,
                         &g_file_proc_fops_dial_wlan_name);
    atp_proc_create_file(g_AtpProcDialDirEntry,
                         ATP_PROC_DIAL_ETH_ISLAN_FILE,
                         &g_AtpProcDialEthIsLanFileEntry,
                         &g_file_proc_fops_dial_eth_lan);  

    atp_proc_create_file(g_AtpProcDialDirEntry,
                         ATP_PROC_DIAL_REPORT_TIMEOUT_FILE,
                         &g_AtpProcDialReportTimeoutFileEntry,
                         &g_file_proc_fops_report_timeout);  
    return 0;
}
#endif

#ifdef CONFIG_ATP_DNSCACHE
#define ATP_PROC_DNS_CACHE_DIR_NAME           "dns"
#define ATP_PROC_DNS_CACHE_DIR                ATP_PROC_DIR"/dns"
#define ATP_PROC_DNS_MATCH_FILE        "urlfilter"
#define ATP_PROC_DNS_MATCH_SWITCH_FILE        "dnscacheswitch"
#define ATP_PROC_DNS_CACHE_LIST_FILE        "dnscachelist"
#define ATP_PROC_DNS_CACHE_TYPE_FILE          "dnscachetype"
static struct proc_dir_entry *g_AtpProcDnsDirEntry = NULL;
static struct proc_dir_entry *g_AtpProcDnsMatchFileEntry = NULL;
static struct proc_dir_entry *g_AtpProcDnsCacheSwitchFileEntry = NULL;
static struct proc_dir_entry *g_AtpProcDnsCacheListFileEntry = NULL;
static struct proc_dir_entry *g_AtpProcDnsCacheTypeEntry = NULL;
extern int proc_dnscache_match_read(struct file *fp, char *buffer, size_t len, loff_t *offset);
extern int proc_dnscache_match_write(struct file *fp, const char *userBuf, size_t len, loff_t *off);
extern int proc_dnscache_list_read(struct file *fp, char *buffer, size_t len, loff_t *offset);
extern int g_dnscacheswitch;
extern int g_filtertype;

static ssize_t atp_proc_dns_match_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{ 
    int ret = 0;
    
    if(*offset > 0) 
    {
        ATP_PROC_DEBUG("atp_proc_dns_match_read finished.\n");
        return 0;
    }
    
    ret = proc_dnscache_match_read(fp, buffer, len, offset);
    if (0 > ret)
    {
        ATP_PROC_ERROR("copy_to_user failed\n");
    }
    *offset += ret;
    return ret;
}
static ssize_t atp_proc_dns_match_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_dns_match_write \n");

    if (NULL == userBuf)
    {
        ATP_PROC_ERROR("buffer is NULL");
        return -1;
    }

    ret = proc_dnscache_match_write(fp, userBuf, len, off);
    if (0 > ret)
    {
        ATP_PROC_ERROR("copy_from_user failed\n");
    }
    
    return ret;
}
static ssize_t atp_proc_dns_cache_swtich_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{ 
    int        ret = 0;
    char       temp_buff[ATP_PROC_FILE_VALUE_LENGTH] = {0};

    if(*offset >0) 
    {
        ATP_PROC_DEBUG("atp_proc_dns_cache_swtich_read finished.\n");
        return 0;
    }
    
    ATP_PROC_DEBUG("atp_proc_prsite_addr_read \n");

    snprintf(temp_buff, sizeof(temp_buff), "%d", g_dnscacheswitch);

    ret = strlen(temp_buff) + 1;
    if(copy_to_user(buffer, temp_buff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",temp_buff);
        return -1;
    }
    *offset += ret;
    return ret;
}
static ssize_t atp_proc_dns_cache_swtich_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;
    char temp_data[ATP_PROC_FILE_VALUE_LENGTH] = {0};
    int tmpswitch = 0;

    ATP_PROC_DEBUG("atp_proc_dns_cache_swtich_write \n");

    if ((NULL == userBuf) || (len > (ATP_PROC_FILE_VALUE_LENGTH - 1)))
    {
        ATP_PROC_ERROR("buffer is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, len))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }
    temp_data[ATP_PROC_FILE_VALUE_LENGTH-1] = 0;
    ret = strlen(temp_data);
    tmpswitch = simple_strtoul(temp_data,NULL,10);

    if ((tmpswitch == 2) || (tmpswitch == 1) || (tmpswitch == 0))
    {
        g_dnscacheswitch = tmpswitch;
    }
    return ret;
}
static ssize_t atp_proc_dns_cache_list_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{ 
    int ret = 0;
    
    if(*offset > 0) 
    {
        ATP_PROC_DEBUG("atp_proc_dns_cache_list_read finished. Read: %s\n", buffer);
        return 0;
    }
    
    ATP_PROC_DEBUG("atp_proc_dns_cache_list_read \n");

    ret = proc_dnscache_list_read(fp, buffer, len, offset);
    *offset += ret;
    return ret;
}


static ssize_t atp_proc_dns_cache_type_read(struct file* fp, char* buffer, size_t len, loff_t* offset)
{
    int        ret = 0;
	char       temp_buff[ATP_PROC_FILE_VALUE_LENGTH] = {0};
    
    if (*offset >0)
    {
        ATP_PROC_DEBUG("atp_proc_dns_cache_brinfo_read finished.\n");
        return 0;
    }
    
    ATP_PROC_DEBUG("atp_proc_dns_cache_type_read \n");
    
    if (g_filtertype)
    {
        snprintf(temp_buff, sizeof(temp_buff), "white \n");
    }
    else
    {
        snprintf(temp_buff, sizeof(temp_buff), "black \n");
    }
	
	ret = strlen(temp_buff) + 1;

	if(copy_to_user(buffer, temp_buff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",temp_buff);
        return -1;
    }	
    *offset += ret;
    return ret;
}

static ssize_t atp_proc_dns_cache_type_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;
    char temp_data[ATP_PROC_FILE_VALUE_LENGTH] = {0};
    int tmpswitch = 0;

    ATP_PROC_DEBUG("atp_proc_dns_cache_type_write \n");

    if ((NULL == userBuf) || (len > (ATP_PROC_FILE_VALUE_LENGTH - 1)))
    {
        ATP_PROC_ERROR("buffer is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, len))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }
    temp_data[ATP_PROC_FILE_VALUE_LENGTH-1] = 0;
    ret = strlen(temp_data);
    tmpswitch = simple_strtoul(temp_data,NULL,10);

    if ((tmpswitch == 1) || (tmpswitch == 0))
    {
        g_filtertype = tmpswitch;
    }
	
    return ret;
}



struct file_operations g_file_proc_fops_dns_match = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_dns_match_read,
                           .write = atp_proc_dns_match_write,                           
                           }; 
struct file_operations g_file_proc_fops_dns_cache_swtich = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_dns_cache_swtich_read,
                           .write = atp_proc_dns_cache_swtich_write,                           
                           }; 
struct file_operations g_file_proc_fops_dns_cache_list = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_dns_cache_list_read,
                           .write = NULL,                           
                           }; 
struct file_operations g_file_proc_fops_dns_cache_type = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_dns_cache_type_read,
                           .write = atp_proc_dns_cache_type_write,                           
                           }; 

int atp_proc_create_dnscache(void)
{
    int error = 0;
    struct path proc_path;

    error = kern_path(ATP_PROC_DNS_CACHE_DIR, LOOKUP_FOLLOW, &proc_path);
    if (error)
    {
        ATP_PROC_DEBUG("%s NOT exist !!!\n", ATP_PROC_DNS_CACHE_DIR);
        g_AtpProcDnsDirEntry = proc_mkdir(ATP_PROC_DNS_CACHE_DIR_NAME, g_AtpProcDirEntry);
        if (NULL == g_AtpProcDnsDirEntry)
        {
            ATP_PROC_ERROR("proc_mkdir failed for ctf");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("%s already exist !\n", ATP_PROC_DNS_CACHE_DIR);
        return 0;
    }

    atp_proc_create_file(g_AtpProcDnsDirEntry,
                         ATP_PROC_DNS_MATCH_FILE,
                         &g_AtpProcDnsMatchFileEntry,
                         &g_file_proc_fops_dns_match);
    atp_proc_create_file(g_AtpProcDnsDirEntry,
                         ATP_PROC_DNS_MATCH_SWITCH_FILE,
                         &g_AtpProcDnsCacheSwitchFileEntry,
                         &g_file_proc_fops_dns_cache_swtich);
    atp_proc_create_file(g_AtpProcDnsDirEntry,
                         ATP_PROC_DNS_CACHE_LIST_FILE,
                         &g_AtpProcDnsCacheListFileEntry,
                         &g_file_proc_fops_dns_cache_list);
    
    atp_proc_create_file(g_AtpProcDnsDirEntry,
                         ATP_PROC_DNS_CACHE_TYPE_FILE,
                         &g_AtpProcDnsCacheTypeEntry,
                         &g_file_proc_fops_dns_cache_type);	
    return 0;
}
#endif

#ifdef CONFIG_ATP_GUESTWIFI_ACCESS_UI

#define ATP_PROC_GUESTWIFIACCESSUI_DIR_NAME        "guestwifiaccessui"
#define ATP_PROC_GUESTWIFIACCESSUI_DIR             ATP_PROC_DIR"/guestwifiaccessui"
#define ATP_PROC_GUESTWIFIACCESSUI_IFACCESS_FILE   "ifaccess"
extern void procaddifnamelist(char* pinfo);
extern int showifnamelist(char *buffer);


static struct proc_dir_entry* g_AtpProcGuestwifiAccessuiDirEntry = NULL;
static struct proc_dir_entry* g_AtpProcGuestwifiAccessuiFileEntry = NULL;



static ssize_t atp_proc_guestwifiaccessui_ifaccess_read(struct file* fp, char* buffer, size_t len, loff_t* offset)
{
    int        ret = 0;
    char       temp_buff[GUESTWIFI_BUFF_LEN_MAX] = {0};
    
    if (*offset > 0)
    {
        ATP_PROC_DEBUG("atp_proc_guestwifiaccessui_ifaccess_read already finished.\n");
        return 0;
    }
    
    showifnamelist(temp_buff);

    ret = strlen(temp_buff) + 1;

    if(copy_to_user(buffer, temp_buff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",temp_buff);
        return -1;
    }
    *offset += ret;
    return ret;
}
static ssize_t atp_proc_guestwifiaccessui_ifaccess_write(struct file* fp, const char* userBuf, size_t len, loff_t* off)
{
    int  ret = 0;
    char temp_data[GUESTWIFI_BUFF_LEN_MAX] = {0};


    ATP_PROC_DEBUG("atp_proc_guestwifiaccessui_ifaccess_write \n");

    if ((NULL == userBuf) || (len > (GUESTWIFI_BUFF_LEN_MAX - 1)))
    {
        ATP_PROC_ERROR("buffer is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, len))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }

    temp_data[GUESTWIFI_BUFF_LEN_MAX - 1] = 0;

    procaddifnamelist(temp_data);

    return ret;
}


struct file_operations g_file_proc_fops_guestwifiaccessui_ifaccess =
{
    .owner = THIS_MODULE,
    .read = atp_proc_guestwifiaccessui_ifaccess_read,
    .write = atp_proc_guestwifiaccessui_ifaccess_write,
};




int atp_proc_create_guestwifiaccessui()
{
    int error = 0;
    struct path proc_path;

    error = kern_path(ATP_PROC_GUESTWIFIACCESSUI_DIR, LOOKUP_FOLLOW, &proc_path);
    if (error)
    {
        ATP_PROC_DEBUG("%s NOT exist !!!\n", ATP_PROC_GUESTWIFIACCESSUI_DIR);
        g_AtpProcGuestwifiAccessuiDirEntry = proc_mkdir(ATP_PROC_GUESTWIFIACCESSUI_DIR_NAME, g_AtpProcDirEntry);
        if (NULL == g_AtpProcGuestwifiAccessuiDirEntry)
        {
            ATP_PROC_ERROR("proc_mkdir failed for secssidaccessui");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("%s already exist !\n", ATP_PROC_GUESTWIFIACCESSUI_DIR);
        return 0;
    }

    atp_proc_create_file(g_AtpProcGuestwifiAccessuiDirEntry,
                         ATP_PROC_GUESTWIFIACCESSUI_IFACCESS_FILE,
                         &g_AtpProcGuestwifiAccessuiFileEntry,
                         &g_file_proc_fops_guestwifiaccessui_ifaccess);			 

    return 0;

}
#endif


int atp_proc_init(void)
{
    int error = 0;
    struct path proc_path;

    error = kern_path(ATP_PROC_DIR, LOOKUP_FOLLOW, &proc_path);
    if (error)
    {
        ATP_PROC_DEBUG("%s NOT exist !\n", ATP_PROC_DIR);
        g_AtpProcDirEntry = proc_mkdir(ATP_PROC_DIR_NAME, NULL);
        if (NULL == g_AtpProcDirEntry)
        {
            ATP_PROC_ERROR("proc_mkdir failed ");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("%s already exist !\n", ATP_PROC_DIR);
    }
#ifdef CONFIG_ATP_PRSITE
    atp_proc_create_prsite();
#endif
#if ((MBB_CTF_COMMON == FEATURE_ON) || (MBB_FEATURE_FASTIP == FEATURE_ON))
    atp_proc_create_ctf();
#endif
#ifdef CONFIG_ATP_ONDEMAND_DIAL
    atp_proc_create_dial();
#endif
#ifdef CONFIG_ATP_DNSCACHE
    atp_proc_create_dnscache();
#endif
#ifdef CONFIG_ATP_GUESTWIFI_ACCESS_UI
    atp_proc_create_guestwifiaccessui();
#endif
    return 0;
}


void atp_proc_deinit(void)
{
#ifdef CONFIG_ATP_PRSITE
    remove_proc_entry(ATP_PROC_PRSITE_ADDR_FILE, g_AtpProcPrsiteAddrEntry);
    remove_proc_entry(ATP_PROC_PRSITE_MASK_FILE, g_AtpProcPrsiteMaskEntry);
    remove_proc_entry(ATP_PROC_PRSITE_ACTIONMODE_FILE, g_AtpProcPrsiteActionModeEntry);
    remove_proc_entry(ATP_PROC_PRSITE_LOCATION_FILE, g_AtpProcPrsiteLocationEntry);    
    remove_proc_entry(ATP_PROC_PRSITE_RAND_FILE, g_AtpProcPrsiteRandEntry);  
    remove_proc_entry(ATP_PROC_PRSITE_TIMES_FILE, g_AtpProcPrsiteTimeEntry);  	
#if defined(CONFIG_ATP_GUESTWIFI_ACCESS_UI) && defined(BSP_CONFIG_BOARD_E5_SB03)
    remove_proc_entry(ATP_PROC_PRSITE_LOCATION_GUESTWIFI_FILE, g_AtpProcPrsiteLocationGuestwifiEntry);
#endif
    remove_proc_entry(ATP_PROC_PRSITE_DIR_NAME, g_AtpProcPrsiteDirEntry);
#endif
#if ((MBB_CTF_COMMON == FEATURE_ON) || (MBB_FEATURE_FASTIP == FEATURE_ON))
    remove_proc_entry(ATP_PROC_CTF_SWITCH_FILE, g_AtpProcCtfSwitchFileEntry);
#if (MBB_FEATURE_ETH_WAN_MIRROR == FEATURE_ON)
    remove_proc_entry(ATP_PROC_CTF_MIRROR_FILE, g_AtpProcCtfMirrorFileEntry);
#endif
#endif
#ifdef CONFIG_ATP_ONDEMAND_DIAL
    remove_proc_entry(ATP_PROC_DIAL_WLAN_NAME_FILE, g_AtpProcDialWlanNameFileEntry);
    remove_proc_entry(ATP_PROC_DIAL_ETH_ISLAN_FILE, g_AtpProcDialEthIsLanFileEntry);
#endif
    remove_proc_entry(ATP_PROC_DIR_NAME, g_AtpProcDirEntry);
#ifdef CONFIG_ATP_GUESTWIFI_ACCESS_UI
    remove_proc_entry(ATP_PROC_GUESTWIFIACCESSUI_IFACCESS_FILE, g_AtpProcGuestwifiAccessuiFileEntry);
#endif

}

module_init(atp_proc_init)
module_exit(atp_proc_deinit)
MODULE_LICENSE("GPL");

