

/*lint -save -e7 -e10 -e63 -e64  -e115 -e132 -e533 -e539*/
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <product_config.h>
#include <asm/uaccess.h>
#include <product_nv_id.h>
#include <bsp_nvim.h>

/*宏定义 节点名称*/
#define DRV_PRODUCTTYPE_PROC_FILE "producttype"


/*全局变量 节点可操作的数据结构*/
static struct proc_dir_entry *g_product_type_proc_file = NULL;

/*************************************************************************
* 函数名     :  drv_product_type_proc_read
* 功能描述   :  查询产品名称
* 输入参数   :  void
* 输出参数   :  buffer : 从内核将版本号传给用户态
* 返回值     :  VER_RET_OK : 操作成功
*               VER_RET_ERROR : 操作失败
**************************************************************************/
static ssize_t drv_product_type_proc_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    void __user *buf_usr = (void __user *)buffer;
    static size_t  len_left = 0;
    size_t  len;

    int ret = 0;
    /*产品类型长度 */
   
    /*存放产品类型 字符串*/
  

    UINT8 ProductType[2] = {0};

    ret = bsp_nvm_read(NV_ITEM_SKU_TYPE, ProductType, sizeof(ProductType));
    
    if (0 != ret)
    {
        pr_err("drv_product_type_proc_read : NVM_Read NV  is failed.\n");
        return -1;
    }
    
    

    /*第一次读取*/
    if (0 == *offset)
    {
        len_left = sizeof(ProductType);
    }
    
    len      = (length > len_left) ? (len_left) : length;
    /*读取完毕*/
    if( 0 == len_left )
    {
        return 0;
    }

    if (!access_ok(VERIFY_WRITE, buf_usr, len))
    {
        pr_err("%s: Verify user buffer write fail.\n", __FUNCTION__);
        return -1;
    }

    if(copy_to_user(buf_usr, ProductType + (sizeof(ProductType) - len_left), len))
    {
        pr_err("%s: copy_to_user failed, nothing copied\n", __FUNCTION__);
        return -1;
    }

    *offset  += len;
    len_left -= len;

    return len;

}

/*节点的操作函数数据结构*/
static struct file_operations drv_product_type_proc_ops = {
    .read  = drv_product_type_proc_read,
};

/*************************************************************************
* 函数名     :  create_product_type_proc_file
* 功能描述   :  创建节点
* 输入参数   :  void
* 输出参数   :  void
* 返回值     :  void
**************************************************************************/
static void create_product_type_proc_file(void)
{
    struct proc_dir_entry *p = NULL;
    g_product_type_proc_file = proc_create(DRV_PRODUCTTYPE_PROC_FILE,
                                /*权限设置*/0444,  p, &drv_product_type_proc_ops);
        
    if(NULL == g_product_type_proc_file)    
    {		
        pr_err("producttype_drv.c:Error:Could not initialize /proc/%s\n",DRV_PRODUCTTYPE_PROC_FILE);    
    }
}

/*************************************************************************
* 函数名     :  remove_drv_version_proc_file
* 功能描述   :  删除节点
* 输入参数   :  void
* 输出参数   :  void
* 返回值     :  void
**************************************************************************/
static void remove_product_type_proc_file(void)
{
    remove_proc_entry(DRV_PRODUCTTYPE_PROC_FILE, NULL);
}

static int __init product_type_drv_init(void)
{
    create_product_type_proc_file();
    return 0;
}

static void __exit product_type_drv_exit(void)
{
    remove_product_type_proc_file();
}

module_init(product_type_drv_init);

module_exit(product_type_drv_exit);

/*lint -restore +e7 +e10 +e63 +e64  +e115 +e132 +e533 +e539*/
