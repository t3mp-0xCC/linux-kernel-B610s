

/**********************���ⵥ�޸ļ�¼******************************************
��    ��              �޸���         ���ⵥ��           �޸�����
 2012.4.19          zhangfei                            ���power_supply�ײ�ʵ��
******************************************************************************/

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/notifier.h>
#include "dwc3_otg_charger.h"
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/gpio.h>


#define CHARGER_IC_ENABLE_GPIO               (GPIO_5_7)

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef uint8_t     boolean;

struct i2c_client *g_i2c_client = NULL;

struct chargeIC_chip *g_chip = NULL;

/*BQ24196��I2C�Ĵ�����ַ*/
#define I2C_CHARGER_IC_ADDR  0x6B

int chg_debug_flag = 0;

int chg_debug_set(int value)
{
    chg_debug_flag = value;

    return 0;
}



/*******************************����ʵ��***************************************/


/******************************************************************************
  Function      chargeIC_i2c_read
  Description   ��ȡI2C�豸ָ���Ĵ�����ַ��ֵ
  Input         reg   :�Ĵ�����ַ
  Output        *pval :���ڽ��Ĵ�����ֵд��������������ڵ�ַ
  Return        0     :����ִ�гɹ�
                -1    :����ִ��ʧ��
  Others        N/A
******************************************************************************/
static int chargeIC_i2c_read(uint8_t reg, uint8_t *pval)
{
    int ret = 0;

    if(NULL == g_i2c_client)
    {
        pr_err("%s failed due to g_i2c_client doesn't exist@~\n", __FUNCTION__);
        return -1;
    }


    ret = i2c_smbus_read_byte_data(g_i2c_client, reg);

    if(ret < 0)
    {
        pr_err("%s fail to i2c_smbus_read_byte_data reg=0x%02X, ret=%d\n",
            __FUNCTION__, reg, ret);
        return -1;
    }
    else
    {
        *pval = ret;
        //pr_info("read(0x%02X)=0x%02X\n", reg, *pval);
        return 0;
    }
}

/******************************************************************************
  Function      chargeIC_i2c_write
  Description   ��I2C�豸�Ĵ���ָ����ַд����ֵ
  Input         reg  : �Ĵ�����ַ
                val  : ϣ��д�������Ĵ�����ַ��ֵ
  Output        N/A
  Return        0    : ����ִ�гɹ�
                -1   : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
static int chargeIC_i2c_write(uint8_t reg, uint8_t val)
{
    int32_t ret = 0;
    if(NULL == g_i2c_client)
    {
        pr_err("%s failed due to g_i2c_client doesn't exist@~\n", __FUNCTION__);
        return -1;
    }


    ret = i2c_smbus_write_byte_data(g_i2c_client, reg, val);
    if(0 == ret)
    {
        //pr_info("write(0x%02X)=0x%02X\n", reg, val);
        return 0;
    }
    else
    {
        pr_err("%s fail to i2c_smbus_write_byte_data reg=0x%02X, ret=%d\n",
            __FUNCTION__, reg, ret);
        return -1;
    }
}
/*test for read reg value*/
int chg_get_chip_reg_value(uint8_t reg)
{
    uint8_t value = 0;
    
    chargeIC_i2c_read(reg, (uint8_t *)&value);

    return value;
}

struct i2c_board_info  i2c_charger_info __initdata=
{
	.type="i2c_charger",
	.addr=I2C_CHARGER_IC_ADDR,
};

static int __init i2c_charger_init(void)
{
    /*charge used bus-1*/
	return i2c_register_board_info(0,&i2c_charger_info,1);
}

boolean this_kthread_should_stop(void)
{
    if (kthread_should_stop())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
/******************************************************************************
  Function      chg_set_chip_cd
  Description   ʹ�ܳ��
  Input         �������ֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/

void chg_set_chip_cd(boolean enable)
{
    if (!gpio_is_valid(CHARGER_IC_ENABLE_GPIO))         /* */
    {
        printk( "CHG_PLT: set charger enable failed!!\r\n ");
    }
    if (gpio_request(CHARGER_IC_ENABLE_GPIO, "chip_cd"))         /* */
    {
        printk( "CHG_PLT:request charger enable failed!!\r\n ");
    }

    if(TRUE == enable)
    {
        gpio_direction_output(CHARGER_IC_ENABLE_GPIO,TRUE);
        printk( "CHG_PLT:set charger enable success!\r\n ");
    }
    else
    {
        gpio_direction_output(CHARGER_IC_ENABLE_GPIO,FALSE);
        printk("CHG_PLT:set charger enable success!\r\n ");
    }
}



/******************************************************************************
  Function      chg_set_supply_limit
  Description   �趨�����������
  Input         �������ֵ
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_supply_limit(uint32_t lmt_val)
{
    uint8_t reg_val = 0;
    int32_t ret     = 0;
    boolean op_result = TRUE;

    if ((CHG_IINPUT_LIMIT_3000MA < lmt_val)
        || (CHG_IINPUT_LIMIT_100MA > lmt_val))
    {
        printk("CHG_DRV: chg_set_supply_limit Para invalid!\n");
        return FALSE;
    }

    ret = chargeIC_i2c_read(BQ24192_INPUT_CTL_REG, &reg_val);
    if (0 != ret)
    {
        printk("CHG_DRV: ERROR READING BQ24192_INPUT_CTL_REG!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }

    /*����Ĵ���ԭʼֵ*/
    reg_val &= ~(BQ24192_IINLIMIT_MASK);

    /*�����������ֵ�ԼĴ�����������*/
    switch (lmt_val)
    {
        case CHG_IINPUT_LIMIT_3000MA:
        {
            reg_val |= BQ24192_IINLIMIT_3000;

            break;
        }
        case CHG_IINPUT_LIMIT_2000MA:
        {
            reg_val |= BQ24192_IINLIMIT_2000;

            break;
        }
        case CHG_IINPUT_LIMIT_1500MA:
        {
            reg_val |= BQ24192_IINLIMIT_1500;

            break;
        }
        case CHG_IINPUT_LIMIT_1200MA:
        {
            reg_val |= BQ24192_IINLIMIT_1200;

            break;
        }
        case CHG_IINPUT_LIMIT_900MA:
        {
            reg_val |= BQ24192_IINLIMIT_900;

            break;
        }
        case CHG_IINPUT_LIMIT_500MA:
        {
            reg_val |= BQ24192_IINLIMIT_500;

            break;
        }
        case CHG_IINPUT_LIMIT_150MA:
        {
            reg_val |= BQ24192_IINLIMIT_150;

            break;
        }
        default:
        {
            reg_val |= BQ24192_IINLIMIT_100;

            break;
        }

    }

    ret = chargeIC_i2c_write(BQ24192_INPUT_CTL_REG, reg_val);
    if (0 != ret)
    {
        printk("CHG_DRV: ERROR SETTING CHARGING SUPPLY CURRENT LIMIT!\n");
        return FALSE;
    }
    else
    {
        return op_result;
    }
}


/******************************************************************************
  Function      chg_set_charge_enable
  Description   �趨�Ƿ�ʹ�ܳ��
  Input         en_val:TRUE   ����ʹ�ܳ��
                       FALSE  ������ֹ���
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
boolean chg_set_charge_enable(boolean enable)
{
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    ret = chargeIC_i2c_read(BQ24192_POWER_ON_CFG_REG, &reg_val);
    if (0 != ret)
    {
        printk("CHG_DRV: ERROR READING chg_set_charge_enable!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }

    reg_val &= ~(BQ24192_CHG_CFG_MASK);

    if (TRUE == enable)
    {
        reg_val |= BQ24192_CHG_CFG_BAT;
    }
    else
    {
        reg_val |= BQ24192_CHG_CFG_DIS;
    }

    ret = chargeIC_i2c_write(BQ24192_POWER_ON_CFG_REG, reg_val);
    if (0 != ret)
    {
        printk("CHG_DRV: ERROR SETTING CHARGING ENABLE VALUE!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
/******************************************************************************
  Function      chg_set_tmr_rst
  Description   ���IC��ʱι��
  Input         
                
  Output        N/A
  Return        TRUE      : ����ִ�гɹ�
                FALSE     : ����ִ��ʧ��
  Others        N/A
******************************************************************************/
int chg_set_tmr_rst(void)
{
    uint8_t reg     = 0;
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    ret = chargeIC_i2c_read(BQ24192_POWER_ON_CFG_REG, &reg_val);
    if (0 != ret)
    {
        printk( "CHG_DRV: ERROR READING chg_set_tmr_rst!\n");
        return FALSE;
    }
    else
    {
        /*do nothing*/
    }

    reg_val |= BQ24192_TMR_RST;

    ret = chargeIC_i2c_write(BQ24192_POWER_ON_CFG_REG, reg_val);
    if (0 != ret)
    {
        printk( "CHG_DRV: ERROR KICKING WATCHDOG!\n");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*��������ID*/
struct task_struct *g_chgTaskID = NULL;

int chg_mini_task( void  *task_para )
{
    while(FALSE == this_kthread_should_stop())
    {
        msleep(5000);
        /*feed watchdog*/
        chg_set_tmr_rst();
        if(chg_debug_flag)
        {
            printk("CHG_DRV: feed watchdog\n !");
        }
    }

    return 0;
}


int chg_chip_BQ24192_init(void)
{

    int32_t ret_code = 0;
    static int s_chg_chip_init_flag = 0;
    printk("CHG_DRV: chg_chip_init \n ");

    if(0 == s_chg_chip_init_flag)
    {
        s_chg_chip_init_flag = 1;
    }
    else
    {
        return TRUE;
    }
    
    /*���оƬʹ��*/
    chg_set_chip_cd(TRUE);

    /*�رճ��ʹ��*/
    if (FALSE == chg_set_charge_enable(FALSE))
    {
        /*If error occured, set 0 bit of ret_code.*/
        ret_code |= (1 << 0);
    }

    /*������������*/
    else if (FALSE == chg_set_supply_limit(1500))
    {
        /*If error occured, set 2 bit of ret_code.*/
        ret_code |= (1 << 1);
    }
    
    /*�Գ��оƬ�����߹�����.*/
    else if (FALSE == chg_set_tmr_rst())
    {
        /*If error occured, set 12 bit of ret_code.*/
        ret_code |= (1 << 2);
    }
    else
    {
        printk("CHG_DRV: CHARGE INIT SUCCESS!\n");
    }

    if (0 != ret_code)
    {
        printk("CHG_DRV: CHARGE INIT: ERRNO %d!\n",ret_code);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

static u8 g_usbotg_value = 0;
int chg_set_usb_otg(void)
{
    u8 value;
    int ret;

    ret = chargeIC_i2c_read(BQ24192_POWER_ON_CFG_REG, &value);
    if(ret){
        return -EIO;
    }

    g_usbotg_value = value & (BQ24192_CHG_CFG_MASK);

    value &= ~(BQ24192_CHG_CFG_MASK);
    value |= (BQ24192_CHG_CFG_OTG);

    ret = chargeIC_i2c_write(BQ24192_POWER_ON_CFG_REG, value);
    if(ret){
        return -EIO;
    }

    printk(KERN_DEBUG"%s line %d:value=%#x\n",__FUNCTION__,__LINE__,value);

    return 0;
}
EXPORT_SYMBOL(chg_set_usb_otg);

int chg_clr_usb_otg(void)
{
    u8 value;
    int ret;

    ret = chargeIC_i2c_read(BQ24192_POWER_ON_CFG_REG, &value);
    if(ret){
        return -EIO;
    }

    value &= ~(BQ24192_CHG_CFG_MASK);
    value |= g_usbotg_value;

    ret = chargeIC_i2c_write(BQ24192_POWER_ON_CFG_REG, value);
    if(ret){
        return -EIO;
    }

    printk(KERN_DEBUG"%s line %d:value=%#x\n",__FUNCTION__,__LINE__,value);

    return 0;
}
EXPORT_SYMBOL(chg_clr_usb_otg);

static int chargeIC_probe(struct i2c_client *client,
                const struct i2c_device_id *id)
{
    struct chargeIC_chip *chip;
    struct device_node *node = client->dev.of_node;
    int pn_rev, devreg_rev;
    uint8_t dev_id;
    int32_t ret = 0;

    dev_info(&client->dev, "%s: Begin:*******chargeIC_probe*********\n", __func__);

    if (!node) {
        dev_err(&client->dev, "%s: device tree information missing\n", __func__);
        //return - ENODEV;
    }

    if (!i2c_check_functionality(client->adapter,
            I2C_FUNC_SMBUS_BYTE_DATA)) {
        dev_err(&client->dev, "%s: SMBUS_BYTE_DATA unsupported\n", __func__);
        return - EIO;
    }

    chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&client->dev, "%s: devm_kzalloc failed\n", __func__);
        return - ENOMEM;
    }

    chip->client = client;

    g_i2c_client = chip->client;
    g_chip = chip;

    i2c_set_clientdata(client, chip);

    ret = chargeIC_i2c_read(I2C_CHARGER_IC_ADDR, &dev_id);
    if (0 != ret)
    {
        dev_err(&client->dev, "failed to chargeIC_i2c_read\n");
    }

    pn_rev = (dev_id & DEV_ID_PN_MASK) >> DEV_ID_PN_SHIFT;
    devreg_rev = (dev_id & DEV_ID_DEV_REG_MASK)
            >> DEV_ID_DEV_REG_SHIFT;

    dev_info(&client->dev, "%s: chargeIC probed successfully, pn_rev=%d, devreg_rev=%d\n",
        __func__, pn_rev, devreg_rev);

    //charge current limmit
    /******************************************/

    /*charge chip init*/
    chg_chip_BQ24192_init();
    
    /* ��������߳� */
    g_chgTaskID = kthread_run( chg_mini_task, NULL, "chg_mini_task");
    if ( IS_ERR(g_chgTaskID) )
    {
        printk(" %s: Thread create error!!!\r\n ", __func__);
    }
    else
    {
        printk(" %s: Thread create success!!!\r\n ", __func__);
    }
    /******************************************/

    dev_info(&client->dev, "%s: g_i2c_client->addr %d\n", __func__, g_i2c_client->addr);

    return 0;
}

static int chargeIC_remove(struct i2c_client *client)
{
    return 0;
}

static const struct i2c_device_id chargeIC_id[] = {
    { .name = "i2c_charger", },
    {},
};

static const struct of_device_id chargeIC_match[] = {
    { .compatible = "ti,i2c_charger", },
    { },
};

static struct i2c_driver chargeIC_driver = {
    .driver    = {
        .name           = "i2c_charger",
        .owner          = THIS_MODULE,
        .of_match_table = chargeIC_match,
    },
    .probe          = chargeIC_probe,
    .remove         = chargeIC_remove,
    .id_table       = chargeIC_id,
};



static int __init chargeIC_init(void)
{
    return i2c_add_driver(&chargeIC_driver);
}
module_init(chargeIC_init);

static void __exit chargeIC_exit(void)
{
    return i2c_del_driver(&chargeIC_driver);
}
module_exit(chargeIC_exit);

postcore_initcall(i2c_charger_init);

MODULE_DESCRIPTION("i2c_charger");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("i2c:i2c_charger");
