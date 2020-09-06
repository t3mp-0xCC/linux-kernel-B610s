



#include <linux/errno.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/swap.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "bsp_ipc.h"
#include "bsp_icc.h"
#include "bsp_sram.h"
#include "mdrv_version.h"
#include <mbb_common.h>
#include <linux/platform_device.h> 
#if ( FEATURE_ON == MBB_MLOG )
#include <linux/mlog_lib.h>
#include <linux/string.h>
#endif
#include "mdrv_ipc_enum.h"
#include <hi_gpio.h>
#include <linux/gpio.h>
struct mbb_common_struct mbb_common_config;

#if (FEATURE_ON == MBB_MEM_CHECK)
#define MEM_CHECK_THREAD_NAME    "mem_check"  /*扫描线程名称*/
#define MEM_CHECK_INTERVAL_TIME   200   /*mem check扫描间隔时间*/
#define MEM_CHECK_WATER_LINE    (8 * 1024)  /*门限大小为8M,此处单位为KB*/
#define DROP_CACHE_PATH   "/proc/sys/vm/drop_caches"
#define MEM_CHECK_KSIZE(x) ((x) << (PAGE_SHIFT - 10))  /*内核计算KB算法*/

/********************************************************
*函数名   : mem_check_get_free_size
*函数功能 : 获取free size
*输入参数 : 无
*输出参数 : 无
*返回值   : free size
*修改历史 :
********************************************************/
static unsigned int mem_check_get_free_size(void)
{
    struct sysinfo m_info = {0};

    /*调用内核接口获取meminfo*/
    si_meminfo(&m_info);

    return MEM_CHECK_KSIZE(m_info.freeram);
}

/********************************************************
*函数名   : mem_check_drop_cache
*函数功能 : 释放page cache
*输入参数 : 无
*输出参数 : 无
*返回值   : 0/-1
*修改历史 :
********************************************************/
static int mem_check_drop_cache(void)
{
    int ret = 0;
    size_t s_ret = -1;
    struct file *filp = NULL;
    mm_segment_t old_fs = {0};
    char *pcbuf = "1";

    /* 改变内存空间访问权限 */
    old_fs = get_fs();
    set_fs(KERNEL_DS);

    /*打开/proc/sys/vm/drop_caches*/
    filp = filp_open(DROP_CACHE_PATH, O_RDWR, 0777);
    if (IS_ERR(filp))
    {
        printk(KERN_ERR "[mem_check_drop_cache]: open %s failed.\n", DROP_CACHE_PATH);
        set_fs(old_fs);
        return -1;
    }

    /*写文件*/
    filp->f_pos = 0;
    s_ret = vfs_write(filp, pcbuf, strlen(pcbuf), &(filp->f_pos));
    if(0 >= s_ret)
    {
        printk(KERN_ERR "[mem_check_drop_cache]: write %s failed, ret=0x%x.\n", DROP_CACHE_PATH, s_ret);
        ret = -1;
    }

    set_fs(old_fs);
    filp_close(filp, NULL);
 
    return ret;
}

/********************************************************
*函数名   : mem_check_thread_fun
*函数功能 : mem check线程主函数
*输入参数 : void
*输出参数 : 无
*返回值   : 0/-1
*修改历史 :
********************************************************/
static int mem_check_thread_fun(void)
{
    unsigned int free_size = 0;

    /*线程主循环*/
    while(1)
    {
        /*休眠200ms*/
        msleep(MEM_CHECK_INTERVAL_TIME);

        /*获取free size*/
        free_size = mem_check_get_free_size();
        if(free_size > MEM_CHECK_WATER_LINE)
        {
            /*free size未到门限值直接返回*/
            continue;
        }

        /*打开/proc/sys/vm/drop_caches释放page cache*/
        (void)mem_check_drop_cache();
    }

    return 0;
}
#endif

/*#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
extern void dwc_otg_enter_suspend(void);
extern void dwc_otg_exit_suspend(void);
#endif*/
#if ( FEATURE_ON == MBB_MLOG )
typedef struct
{
    uint32 one_class_temp_start_time;
    uint32 one_class_temp_last_time;
    uint32 two_class_temp_start_time;
    uint32 two_class_temp_last_time;
    uint32 dropline_class_temp_start_time;
    uint32 dropline_class_temp_last_time;
} ThermStatTime_t;

ThermStatTime_t  g_ThermStatTime = {0, 0, 0, 0, 0, 0};

typedef enum _THERM_STATE_TYPE
{
    THERM_STATE_NORMAL = 0,
    THERM_STATE_LIMIT_1 = 1 ,  /*20M*/
    THERM_STATE_LIMIT_2 = 2 ,  /*2M*/
    THERM_STATE_EMERG_1 = 3 ,  /*fly mode*/
    THERM_STATE_EMERG_2 = 4 ,  /*shutdown*/
    THERM_STATE_MAX = 0xFFFF,  /*stop*/
} THERM_STATE_TYPE;

THERM_STATE_TYPE current_therm_state = THERM_STATE_MAX;
THERM_STATE_TYPE pre_therm_state = THERM_STATE_MAX;

#define SIX_MINUTE_TICKS   (HZ * 60 * 6)

void  mstat_thermal_protection(char* p);

#endif
#if ( FEATURE_ON == MBB_MLOG )
/********************************************************
*函数名   : mbb_common_mlog_recv
*函数功能 : 接收C核需要发到mobile log模块的日志
*输入参数 : u32 channel_id , u32 len, void* context
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2014-2-18 徐超 初版作成
********************************************************/
static int mbb_common_mlog_recv( u32 channel_id , u32 len, void* context )
{
    int read_len = 0;
    stMbbCommonMlog mlogbuf;
    mlogbuf.len = 0;
    mlogbuf.loglv = 0;

    read_len = bsp_icc_read(channel_id, (u8*)&mlogbuf, len);
    if((read_len > len) || (read_len < 0))
    {
        printk(KERN_ERR "[Mbb Common Drv] mlogbuf.len =(%d),read len(%d),len(%d), msg: %s.\n", mlogbuf.len, read_len,len, mlogbuf.msg);
        return -1;
    }

    printk(KERN_INFO "[Mbb Common Drv] module=(%s),mlogbuf.len =(%d),read len(%d),len(%d),lv=(%d),msg: %s.\n",
        mlogbuf.module, mlogbuf.len, read_len, len, mlogbuf.loglv, mlogbuf.msg);
#if (FEATURE_ON == MBB_MLOG)
    mlog_print(mlogbuf.module, mlogbuf.loglv, "%s", mlogbuf.msg);
#endif
#if ( FEATURE_ON == MBB_MLOG)
    mstat_thermal_protection(mlogbuf.module);
#endif
    return 0;
}
#endif

#if ( FEATURE_ON == MBB_MLOG)
void  mstat_thermal_protection(char* p)
{
    char* tp_sign = "TP_CLASS_" ;
    char* tp_class = NULL;
    char* tp_string = p ;
    unsigned int last_times = 0;
    unsigned int tp_class_max_size = 0 ;

    if ( NULL == p)
    {
        return;
    }

    if ( 0 != strncmp(tp_string, tp_sign, strlen(tp_sign)))
    {
        return;
    }

    tp_class = p + strlen(tp_sign) ;
    tp_class_max_size = MLOG_MAX_MD_LEN - strlen(tp_sign) - 1;

    printk("%s tp_class_max_size= %d  %s %d \n" , tp_class, tp_class_max_size, __FUNCTION__, __LINE__);
    if (0 == strncmp(tp_class, "ONE", min(strlen(tp_sign), tp_class_max_size )))
    {
        current_therm_state = THERM_STATE_LIMIT_1;
        if (pre_therm_state != current_therm_state)
        {
            mlog_set_statis_info("one_class_temp_prot_count", 1);
            g_ThermStatTime.one_class_temp_start_time = jiffies;
        }
    }

    if (0 == strncmp(tp_class, "TWO", min(strlen(tp_sign), tp_class_max_size )))
    {
        current_therm_state = THERM_STATE_LIMIT_2;
        if (pre_therm_state != current_therm_state)
        {
            mlog_set_statis_info("two_class_temp_prot_count", 1);
            g_ThermStatTime.two_class_temp_start_time = jiffies;
        }
    }

    if (0 == strncmp(tp_class, "DROPLINE", min(strlen(tp_sign), tp_class_max_size)))
    {
        current_therm_state = THERM_STATE_EMERG_1;
        if (pre_therm_state != current_therm_state)
        {
            mlog_set_statis_info("dropline_class_temp_prot_count", 1);
            g_ThermStatTime.dropline_class_temp_start_time = jiffies;
        }
    }

    if (0 == strncmp(tp_class, "NORMAL", min(strlen(tp_sign), tp_class_max_size)))
    {
        current_therm_state = THERM_STATE_NORMAL;
    }

    if (0 == strncmp(tp_class, "POWEROFF", min(strlen(tp_sign), tp_class_max_size)))
    {
        current_therm_state = THERM_STATE_EMERG_2;
    }

    if (pre_therm_state != current_therm_state)
    {
        if (THERM_STATE_LIMIT_1 == pre_therm_state)
        {
            /*获取持续时间*/
            g_ThermStatTime.one_class_temp_last_time = \
                    g_ThermStatTime.one_class_temp_last_time \
                    + jiffies - g_ThermStatTime.one_class_temp_start_time;

            /*持续时间转换成6分钟*/
            last_times =  g_ThermStatTime.one_class_temp_last_time / SIX_MINUTE_TICKS;

            /*保存剩余的ticks*/
            g_ThermStatTime.one_class_temp_last_time = \
                    g_ThermStatTime.one_class_temp_last_time % SIX_MINUTE_TICKS;

            /*写入统计信息*/
            mlog_set_statis_info("one_class_temp_prot_time", last_times);
        }
        else if (THERM_STATE_LIMIT_2 == pre_therm_state)
        {
            /*获取持续时间*/
            g_ThermStatTime.two_class_temp_last_time = \
                    g_ThermStatTime.two_class_temp_last_time \
                    + jiffies - g_ThermStatTime.two_class_temp_start_time;

            /*持续时间转换成6分钟*/
            last_times = g_ThermStatTime.two_class_temp_last_time / SIX_MINUTE_TICKS;
            mlog_set_statis_info("two_class_temp_prot_time", last_times);

            /*保存剩余的ticks*/
            g_ThermStatTime.two_class_temp_last_time = \
                    g_ThermStatTime.two_class_temp_last_time % SIX_MINUTE_TICKS;

            /*写入统计信息*/
            mlog_set_statis_info("two_class_temp_prot_time", last_times);

        }
        else if (THERM_STATE_EMERG_1 == pre_therm_state)
        {
            /*获取持续时间*/
            g_ThermStatTime.dropline_class_temp_last_time = \
                    g_ThermStatTime.dropline_class_temp_last_time \
                    + jiffies - g_ThermStatTime.dropline_class_temp_start_time;

            /*持续时间转换成6分钟*/
            last_times = g_ThermStatTime.dropline_class_temp_last_time / SIX_MINUTE_TICKS;

            /*保存剩余的ticks*/
            g_ThermStatTime.dropline_class_temp_last_time = \
                    g_ThermStatTime.dropline_class_temp_last_time % SIX_MINUTE_TICKS;
            mlog_set_statis_info("dropline_class_temp_prot_time", last_times);
        }
    }
    pre_therm_state = current_therm_state;
}
#endif


/********************************************************
*函数名   : mbb_common_ipc_send
*函数功能 : 向C核发送消息
*输入参数 : u32 flag
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2013-11-25 徐超 初版作成
********************************************************/
void mbb_common_ipc_send(u32 flag)
{
    SRAM_SMALL_SECTIONS * sram_mem = ((SRAM_SMALL_SECTIONS * )(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS));
    sram_mem->SRAM_MBB_COMMON = flag;
    bsp_ipc_int_send(IPC_CORE_CCORE,IPC_CCPU_INT_SRC_ACPU_MBB_COMM);
}

#ifdef BSP_CONFIG_BOARD_SOLUTION
/*solution_type节点操作函数*/
/********************************************************
*函数名   : proc_solution_type_read
*函数功能 : solution_type节点读操作函数
        用来获取模块子产品形态
*输入参数 :
*输出参数 :
*返回值   : 成功返回读取数据的长度
            否则返回0
********************************************************/
#define SOLUTION_TYPE_LEN   2
static ssize_t proc_solution_type_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    unsigned long ret = 0;
    ssize_t len = 0;
    unsigned char type_buf[SOLUTION_TYPE_LEN] = {0};  /*存放产品形态类型的数组*/
    MDRV_VER_SOLUTION_TYPE solution_type = MDRV_VER_TYPE_INVALID;

    /*入参判断*/
    if((NULL == buffer) || (count < SOLUTION_TYPE_LEN))
    {
        printk(KERN_ERR "[solution_type_proc_read]: Input para error, count = %d.\n", count);
        return 0;
    }

    if(0 == *ppos)
    {
        /*初始化*/
        memset(type_buf, 0, SOLUTION_TYPE_LEN);
        /*调用接口返回实际的子产品形态*/
        solution_type = mdrv_ver_get_solution_type();
        (void)snprintf(type_buf, SOLUTION_TYPE_LEN, "%d", solution_type);
        len = (ssize_t)(strnlen(type_buf, SOLUTION_TYPE_LEN) + 1);

        /*将子形态字符值返回带出*/
        ret = copy_to_user(buffer, type_buf, len);
        if(ret)
        {
            printk(KERN_ERR "[solution_type_proc_read]: copy to user failed, ret = 0x%x.\n", ret);
            return 0;
        }

        *ppos += len;
        return len;
    }
    else
    {
        /*支持cat,内容仅2个字节,第二次读直接返回0*/
        return 0;
    }
}

/********************************************************
*函数名   : proc_solution_type_write
*函数功能 : solution_type节点写操作函数
        打桩预留
*输入参数 :
*输出参数 :
*返回值   :
********************************************************/
static ssize_t proc_solution_type_write(struct file *file, const char __user *buffer, 
                        size_t count, loff_t *ppos)
{
    return 0;
}

static const struct file_operations g_file_solution_type ={
    .owner    = THIS_MODULE,
    .read     = proc_solution_type_read,
    .write    = proc_solution_type_write,
};
#endif

#if (FEATURE_ON == MBB_VBUS_POWER_CTRL)
#define VBUS_CLASS_NODE          "vbus"
#define VBUS_POWER_GPIO_NAME     "mbb_vbus_power"
#define VBUS_OVERFLOW_GPIO_NAME  "mbb_vbus_overflow"
int g_vbus_irq_flag = 0xBB9527;  /* 共享中断时需要判断是否是自己需要的中断 */

/********************************************************
*函数名   : vbus_overflow_isr
*函数功能 : vbus过流后关闭vbus供电
*输入参数 : int irq, void *dev_id
*输出参数 : 无
*返回值   :IRQ_NONE表示中断非此gpio产生，
                      IRQ_HANDLED表示中断处理成功
*修改历史 :
********************************************************/
irqreturn_t vbus_overflow_isr(int irq, void *dev_id)
{
    int *pvbus_flag = NULL;
    if(NULL == dev_id)
    {
        return IRQ_NONE;
    }
    
    pvbus_flag = (int *)dev_id;
    if(g_vbus_irq_flag != *pvbus_flag )
    {
        return IRQ_NONE;
    }
    printk("vbus_overflow_isr:%d vbus over flow 0x%x!!\n", __LINE__, *pvbus_flag);
    /* vbus发生过流，关闭vbus供电 */
    gpio_direction_output(VBUS_POWER_CTRL, 0);
    return IRQ_HANDLED;
}

/******************************************************************************
*Function:           vbus_power_status_show
*Description:        提供给应用的节点，读取vbus上下电情况
*Calls:              
*Input:              
*Output:             NA
*Return:             字符串长度
*Others:             NA
******************************************************************************/
static ssize_t vbus_power_status_show(struct class *class, 
                    struct class_attribute *attr, char *buf)
{
    if (NULL == buf || NULL == class || NULL == attr)
    {
        printk("vbus_power_status_show:the buf or class or attr is null.\n");
        return -ENOMEM;
    }

    /* 将vbus供电情况输出到节点中 */
    return snprintf(buf, 3, "%u\n", gpio_get_value(VBUS_POWER_CTRL));
}

/******************************************************************************
*Function:           vbus_power_status_store
*Description:        提供给应用的节点，操作vbus上下电
*Calls:              
*Input:              
*Output:             NA
*Return:             字符串长度
*Others:             NA
******************************************************************************/
static ssize_t vbus_power_status_store(struct class *class,
        struct class_attribute *attr, const char *buf, size_t size)
{
    unsigned vbus_state = 0;
    ssize_t ret = -EINVAL;

    if( NULL == class || NULL == attr || NULL == buf )
    {
        printk("vbus_power_status_store:%d class or attr or buf is NULL !\n", __LINE__);
        return -EINVAL;
    }

    /* 将写入节点的字符转化成数字 */
    ret = kstrtoul(buf, 10, &vbus_state);
    if (0 != ret)
    {
        printk("vbus_power_status_store:%d You must write 0 or 1 !\n", __LINE__);
        return ret;
    }

    /* gpio设置的时候只支持0和1 */
    if( 0 == vbus_state || 1 == vbus_state)
    {
        printk("vbus_power_status_store:%d  %u!\n", __LINE__, vbus_state);
        gpio_direction_output(VBUS_POWER_CTRL, vbus_state);
    }
    else
    {
        printk("vbus_power_status_store:%d You must write 0 or 1 !\n", __LINE__);
    }
    return size;
}

static struct class* vbus_class = NULL;
static CLASS_ATTR(power_status, S_IRUGO | S_IWUSR, vbus_power_status_show, vbus_power_status_store);
static struct class_attribute *vbus_attributes[] = {
        &class_attr_power_status,   /* power_status */
};

/******************************************************************************
*Function:           vbus_gpio_init
*Description:        初始化用到的gpio
*Calls:              vbus_create_class_files
*Input:              
*Output:             NA
*Return:             成功返回0 失败返回-错误码
*Others:             NA
******************************************************************************/
int vbus_gpio_init(void)
{
    int ret = -EPERM;
    
    ret = gpio_request(VBUS_POWER_CTRL, VBUS_POWER_GPIO_NAME);
    if(0 != ret) 
    {
        printk(KERN_ERR "%s: request gpio %s FAIL!\n", __func__, VBUS_POWER_GPIO_NAME);
        return -EPERM;
    }
    
    ret = gpio_request(VBUS_OVERFLOW_IRQ, VBUS_OVERFLOW_GPIO_NAME);
    if(0 != ret) 
    {
        printk(KERN_ERR "%s: request gpio %s FAIL!\n", __func__, VBUS_OVERFLOW_GPIO_NAME);
        return -EPERM;
    }

    (void)gpio_direction_input(VBUS_OVERFLOW_IRQ);

    ret = request_irq((unsigned int)gpio_to_irq(VBUS_OVERFLOW_IRQ), vbus_overflow_isr, 
                    IRQF_NO_SUSPEND | IRQF_SHARED | IRQF_TRIGGER_FALLING, VBUS_OVERFLOW_GPIO_NAME, (void*)(&g_vbus_irq_flag));
    if (0 > ret)
    {
        printk(KERN_ERR"%s: %s  request_irq  failed.\n", __func__, VBUS_OVERFLOW_GPIO_NAME);
        return -EPERM;
    }
    return 0;
}

/******************************************************************************
*Function:           vbus_create_class_files
*Description:        创建跟应用交互的sys/class/目录节点
*Calls:              class_create  /  class_create_file
*Input:              
*Output:             NA
*Return:             0: success, 非0: failed
*Others:             NA
******************************************************************************/
static int vbus_create_class_files(void)
{
    int ret = -EPERM;
    int i = 0;
    int attrs_num = 0;

    struct class_attribute** vbus_attrs = vbus_attributes;

    vbus_class = class_create(THIS_MODULE, VBUS_CLASS_NODE);
    if (IS_ERR(vbus_class))
    { 
        printk("Error: Failed to create class.");
        return PTR_ERR(vbus_class); 
    }

    attrs_num = ARRAY_SIZE(vbus_attributes);
    for(i = 0; i < attrs_num; i++)
    {
        ret = class_create_file(vbus_class, vbus_attrs[i]);
        if (ret < 0)
        {
            (void)class_destroy(vbus_class);
            printk("Failed to create file, %d", i);
            return ret;
        }
    }

    /* 初始化vbus控制的gpio */
    ret = vbus_gpio_init();
    return ret;
}
/******************************************************************************
*Function:           vbus_destory_class_files
*Description:        删除跟应用交互的sys/class/目录节点
*Calls:              class_remove_file
*Input:              
*Output:             NA
*Return:             
*Others:             NA
******************************************************************************/
static void vbus_destory_class_files(void)
{
    int i = 0;
    int attrs_num = 0;
    struct class_attribute** vbus_attrs = vbus_attributes;

    if(NULL == vbus_class)
    {
        return;
    }
    
    attrs_num = ARRAY_SIZE(vbus_attributes);
    for(i = 0; i < attrs_num; i++)
    {
        (void)class_remove_file(vbus_class, vbus_attrs[i]);
    }

    (void)class_destroy(vbus_class);
    return;
}
#endif

/********************************************************
*函数名   : Mbb_Common_Probe
*函数功能 : 
*输入参数 : struct platform_device *dev
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2013-11-25 徐超 初版作成
********************************************************/
static int Mbb_Common_Probe(struct platform_device *dev)
{

    int ret = 0;
#ifdef BSP_CONFIG_BOARD_SOLUTION
    struct proc_dir_entry *solution_type = NULL; /*模块solution_type节点,用来查询子产品形态*/
#endif
#if ( FEATURE_ON == MBB_MLOG )
    u32 channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_MLOG; /* 用于接收C核的mobile log消息 */
#endif

    printk(KERN_ERR "\r\n [Mbb Common Drv] Mbb_Common_Probe!\n");

    if(NULL == dev )
    {
        printk(KERN_ERR "\r\n [Mbb Common Drv]Mbb_Common_Probe: dev is NULL!\r\n");
        return -1;
    }

#ifdef BSP_CONFIG_BOARD_SOLUTION
    /*模块创建proc节点,用来查询模块子产品形态(M2M/CE/车载)*/
    solution_type = proc_create("solution_type", S_IFREG | S_IRUGO, NULL, &g_file_solution_type);
    if (NULL == solution_type)
    {
        /*节点创建失败后继续执行*/
        printk(KERN_ERR "[Mbb_Common_Probe]: create solution_type procfile error.\n");
    }
#endif

#if (FEATURE_ON == MBB_MEM_CHECK)
    /*创建线程轮询ddr free大小,小于阈值则释放cache*/
    (void)kthread_run(mem_check_thread_fun, NULL, MEM_CHECK_THREAD_NAME);
#endif

#if ( FEATURE_ON == MBB_MLOG )
    /* 注册用于接收C核的mobile log消息的函数 */
    ret = bsp_icc_event_register(channel_id, (read_cb_func)mbb_common_mlog_recv, NULL, NULL, NULL);
    if(ret != 0)
    {
        printk(KERN_ERR "\r\n [Mbb Common Drv] bsp_icc_event_register error，ret=%d.\n", ret);
    }
#endif

#if (FEATURE_ON == MBB_VBUS_POWER_CTRL)
    /* 创建vbus上下电控制的节点 */
    ret = vbus_create_class_files();
    if(ret != 0)
    {
        printk(KERN_ERR "\r\n [Mbb Common Drv] vbus_create_class_files error，ret=%d.\n", ret);
    }
#endif
    return ret;
}
/********************************************************
*函数名   : Mbb_Common_Remove
*函数功能 : 
*输入参数 : 
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2013-11-25 徐超 初版作成
********************************************************/
static int Mbb_Common_Remove(struct platform_device *dev)
{
    int ret = 0;

    printk(KERN_ERR "\r\n [Mbb Common Drv] Mbb_Common_Remove!\n");
    
    if(NULL == dev )
    {
        printk(KERN_ERR "\r\n [Mbb Common Drv]Mbb_Common_Remove: dev is NULL!\r\n");
        return -1;
    }
    
#if (FEATURE_ON == MBB_VBUS_POWER_CTRL)
    vbus_destory_class_files();
#endif
    return ret;
}

#ifdef CONFIG_PM
static int Mbb_Common_Prepare(struct device *pdev)
{
    return 0;
}

static void Mbb_Common_Complete(struct device *pdev)
{
    return ;
}

/********************************************************
*函数名   : Mbb_Common_Suspend
*函数功能 : Mbb_Common函数，用于处理待机休眠时的一些事务
*输入参数 : 
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2013-11-25 徐超 初版作成
********************************************************/
static int Mbb_Common_Suspend(struct device *dev)
{
    int ret = 0;

    printk(KERN_INFO "\r\n [Mbb Common Drv] Mbb_Common_Suspend!\n");
    
    if(NULL == dev )
    {
        printk(KERN_ERR "\r\n [Mbb Common Drv]Mbb_Common_Suspend: dev is NULL!\r\n");
        return -1;
    }
/*#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
    dwc_otg_enter_suspend();
#endif */

    /* 将A核要进入待机的消息通知C核*/
    mbb_common_ipc_send(MBB_SUSPEND);
    return ret;
}

/********************************************************
*函数名   : Mbb_Common_Resume
*函数功能 : Mbb_Common函数，用于处理待机唤醒时的一些事务
*输入参数 : 
*输出参数 : 无
*返回值   : 执行成功返回0，失败返回非0值
*修改历史 :
*           2013-11-25 徐超 初版作成
********************************************************/
static int Mbb_Common_Resume(struct device *dev)
{
    int ret = 0;

    printk(KERN_INFO "\r\n [Mbb Common Drv] Mbb_Common_Resume!\n");
    
    if(NULL == dev )
    {
        printk(KERN_ERR "\r\n [Mbb Common Drv]Mbb_Common_Resume: dev is NULL!\r\n");
        return -1;
    }
    /* 将A核被唤醒的消息通知C核*/
/*#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
    dwc_otg_exit_suspend();
#endif*/

    mbb_common_ipc_send(MBB_RESUME);
    return ret;
}

const struct dev_pm_ops Mbb_Common_dev_Pm_Ops =
{
    .suspend    =   Mbb_Common_Suspend,
    .resume     =   Mbb_Common_Resume,
    .prepare    =   Mbb_Common_Prepare,
    .complete   =   Mbb_Common_Complete,
};

#define MBB_COMMON_DEV_PM_OPS (&Mbb_Common_dev_Pm_Ops)

#else

#define MBB_COMMON_DEV_PM_OPS NULL

#endif

static struct platform_driver Mbb_Common_drv = {
    .probe = Mbb_Common_Probe,
    .remove = Mbb_Common_Remove,
    .driver = {  
        .name  = "Mbb_Common",  
        .owner = THIS_MODULE, 
        .pm    = MBB_COMMON_DEV_PM_OPS,
    },  
};

static struct platform_device Mbb_Common_dev = {
    .name           = "Mbb_Common",
    .id             = 1,
    .dev ={
        .platform_data = &mbb_common_config,
    },
};

static int __init mbb_common_init(void)
{
    int ret = 0;
    
    ret = platform_device_register(&Mbb_Common_dev);
    if (ret < 0)
    {
        printk(KERN_ERR "\r\n [Mbb Common Drv] platform_device_register [Mbb Common Module] failed.\r\n");
        return ret;
    }
    
    ret = platform_driver_register(&Mbb_Common_drv);
    if (0 > ret)
    {
        platform_device_unregister(&Mbb_Common_dev);
        printk(KERN_ERR "\r\n [Mbb Common Drv] platform_driver_register [Mbb Common Module] failed.\r\n");
        return ret;
    }
    return ret;
}

static void __exit mbb_common_exit(void)
{
    platform_driver_unregister(&Mbb_Common_drv);

    platform_device_unregister(&Mbb_Common_dev);
}

module_init(mbb_common_init);
module_exit(mbb_common_exit);

MODULE_AUTHOR("MBB.Huawei Device");
MODULE_DESCRIPTION("Mbb Common Driver");
MODULE_LICENSE("GPL");

