

/*lint -save -e7 -e10 -e63 -e64  -e115 -e132 -e533 -e539*/
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <product_config.h>
#include <asm/uaccess.h>
#include <product_nv_id.h>
#include <bsp_nvim.h>

/*�궨�� �ڵ�����*/
#define DRV_PRODUCTTYPE_PROC_FILE "producttype"


/*ȫ�ֱ��� �ڵ�ɲ��������ݽṹ*/
static struct proc_dir_entry *g_product_type_proc_file = NULL;

/*************************************************************************
* ������     :  drv_product_type_proc_read
* ��������   :  ��ѯ��Ʒ����
* �������   :  void
* �������   :  buffer : ���ں˽��汾�Ŵ����û�̬
* ����ֵ     :  VER_RET_OK : �����ɹ�
*               VER_RET_ERROR : ����ʧ��
**************************************************************************/
static ssize_t drv_product_type_proc_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    void __user *buf_usr = (void __user *)buffer;
    static size_t  len_left = 0;
    size_t  len;

    int ret = 0;
    /*��Ʒ���ͳ��� */
   
    /*��Ų�Ʒ���� �ַ���*/
  

    UINT8 ProductType[2] = {0};

    ret = bsp_nvm_read(NV_ITEM_SKU_TYPE, ProductType, sizeof(ProductType));
    
    if (0 != ret)
    {
        pr_err("drv_product_type_proc_read : NVM_Read NV  is failed.\n");
        return -1;
    }
    
    

    /*��һ�ζ�ȡ*/
    if (0 == *offset)
    {
        len_left = sizeof(ProductType);
    }
    
    len      = (length > len_left) ? (len_left) : length;
    /*��ȡ���*/
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

/*�ڵ�Ĳ����������ݽṹ*/
static struct file_operations drv_product_type_proc_ops = {
    .read  = drv_product_type_proc_read,
};

/*************************************************************************
* ������     :  create_product_type_proc_file
* ��������   :  �����ڵ�
* �������   :  void
* �������   :  void
* ����ֵ     :  void
**************************************************************************/
static void create_product_type_proc_file(void)
{
    struct proc_dir_entry *p = NULL;
    g_product_type_proc_file = proc_create(DRV_PRODUCTTYPE_PROC_FILE,
                                /*Ȩ������*/0444,  p, &drv_product_type_proc_ops);
        
    if(NULL == g_product_type_proc_file)    
    {		
        pr_err("producttype_drv.c:Error:Could not initialize /proc/%s\n",DRV_PRODUCTTYPE_PROC_FILE);    
    }
}

/*************************************************************************
* ������     :  remove_drv_version_proc_file
* ��������   :  ɾ���ڵ�
* �������   :  void
* �������   :  void
* ����ֵ     :  void
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
