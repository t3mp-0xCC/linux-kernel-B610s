/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "usb_config.h"

#ifdef USB_TYPEC_CTRL

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/i2c/pmbus.h>
#include "usb_debug.h"
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include "product_config.h"
#include <linux/wait.h>
#include "usb_typec_driver.h"

#define TYPEC_USB_LINE_REMOVE    0
#define TYPEC_USB_LINE_INSERT    1

/*regulator 3.0v  for typec chip*/
#define TYPEC_REGULATOR_VOLT 3000000
/*chip default address for I2C interface*/
#define TYPEC_TI_DEVICE  0x47
#define TYPEC_PERL_DEVICE  0x1D
#define TYPEC_FAIRCHILD_DEVICE 0x21
#define TYPEC_NXP_DEVICE 0x1e

#define TYPEC_TI_ID 0x30
#define TYPEC_PERL_ID 0x0
#define TYPEC_FC_ID 0x12
#define TYPEC_NXP_ID 0xB

/*reg ,for reg read or write */
#define TYPEC_REG_0  0x00
#define TYPEC_REG_1  0x01
#define TYPEC_REG_2  0x02
#define TYPEC_REG_3  0x03
#define TYPEC_REG_4  0x04
#define TYPEC_REG_5  0x05
#define TYPEC_REG_6  0x06
#define TYPEC_REG_7  0x07
#define TYPEC_REG_8  0x08
#define TYPEC_REG_9  0x09
#define TYPEC_REG_10 0x0a
#define TYPEC_REG_11 0x0b
#define TYPEC_REG_12 0x0c
#define TYPEC_REG_13 0x0d
#define TYPEC_REG_14 0x0e
#define TYPEC_REG_15 0x0f
#define TYPEC_REG_16 0x10
#define TYPEC_REG_17 0x11
#define TYPEC_REG_18 0x12
#define TYPEC_REG_19 0x13
/*chip all regs number*/
#define PERL_MAX_REG 0x04
#define TI_MAX_REG 0x0f
#define FAIR_MAX_REG 0x1F
#define NXP_MAX_REG 0x50
#define TYPEC_MAX_REGS 0x60
#define TYPEC_BUF_LEN 20
#define TYPEC_MAX_DATALEN 256
#define TYPEC_REG_NUM_1 1
#define TYPEC_REG_NUM_2 2
#define TYPEC_REG_MAX_NUM 0x16
#define TYPEC_GPIO_ATTR 2

/*default support chip:ti ,perl,fairchild typec*/
#define TYPEC_SUPPORT_CHIPS 4

#define TYPEC_ERROR_1 ( - 1)
#define TYPEC_ERROR_2 ( - 2)
#define BITS_1 1
#define BITS_2 2
#define BITS_3 3
#define BITS_4 4
/*current typec mode define*/

#define TYPEC_PERL_DFP 0x01
#define TYPEC_TI_DFP 0x01
#define TYPEC_FC_DFP 0x02
#define TYPEC_NXP_DFP 0x02


/* define for typec try sink func, reg and value is supply from CHIP PRODUCTOR, no detail in datasheet*/
#define TYPEC_REG_43H 0x43
#define TYPEC_REG_4CH 0x4c
#define TYPEC_NUM_40H 0x40
#define TYPEC_NUM_34H 0x34
#define TYPEC_NUM_11H 0x11
#define TYPEC_NUM_15H 0x15
#define TYPEC_NUM_3aH 0x3a
#define TYPEC_NUM_30H 0x30
#define TYPEC_NUM_C0H 0xC0
#define TYPEC_NUM_24H 0x30
/*try sink wait delay ms*/
#define TYPEC_TRY_SINK_DELAY_300 300
#define TYPEC_TRY_SINK_DELAY_150 150

/*get typec mode*/
#define TYPEC_PERL_UFP 0x02
#define TYPEC_TI_UFP 0x02
#define TYPEC_FC_UFP 0x01
#define TYPEC_NXP_UFP 0x01

/* try sink process, id line signal will be swtich on and off, which should be ignore*/
int g_irq_avail = true;
/*cc status*/
typedef enum __usbc_cc_stauts
{
    CC_NO_CONNECT,
    CC1_CONNECT,
    CC2_CONNECT,
    CC_ERROR,
} USBC_CC_STATUS;

/*typec host/device mode,DRP:otg. DFP:host, UFP: device ACC:accessory mode*/


typedef struct _hw_usbc_driver
{
    struct class*    usbc_class;
    struct device*   usbc_dev;
    struct i2c_client* usbc_i2c_client;/*use I2C API to read write usb typec reg*/
    /*attr,<num,value>,num is gpio number,value is the default value */
    int gpio[TYPEC_GPIO_ATTR];
    int gpio_ss_sel_en[TYPEC_GPIO_ATTR];
    int gpio_ss_oen[TYPEC_GPIO_ATTR];
    USBC_TYPE type;
    int reg_max;
    void* pivate;
} usb_typec_driver;

/*
PERL chip CC register info
*/
typedef struct _pistat
{
    char stub1;
    char stub2;
    char stub3;
    unsigned char cc: BITS_2;
    unsigned char attach_mode: BITS_3;
    unsigned char charging_current_det: BITS_2;
    unsigned char vbus_det: BITS_1;
} pistat;

/*TI chip CC register info*/

typedef struct _tustat
{
    char stub1;
    char stub2;
    char stub3;
    char stub4;
    char stub5;
    char stub6;
    char stub7;
    char stub8;
    unsigned char cable: BITS_1;
    unsigned char acc: BITS_3;
    unsigned char set_curr: BITS_2;
    unsigned char curr: BITS_2;
    unsigned char reserved: BITS_4;
    unsigned char int_ss: BITS_1;
    unsigned char cc: BITS_1;
    unsigned char attach_mode: BITS_2;
} tustat;

/*FAIRCHILD chip CC register info*/

typedef struct _fustat
{
    char stub0;
    char stub1;
    char stub2;
    char stub3;
    char stub4;
    char stub5;
    char stub6;
    char stub7;
    char stub8;
    char stub9;
    char stub10;
    char stub11;
    char stub12;
    char stub13;
    char stub14;
    char stub15;
    char stub16;
    unsigned char attach: BITS_1;
    unsigned char bc_lvl: BITS_2;
    unsigned char vbus: BITS_1;
    unsigned char cc: BITS_2;
    unsigned char reserved: BITS_2;
    unsigned char acc: BITS_3;
    unsigned char attach_mode: BITS_2;
    unsigned char reserved1: BITS_3;
    char stub19;
} fustat;

/*NXP chip CC register info*/

typedef struct _nxpstat
{
    char stub0;
    char stub1;
    char stub2;
    char stub3;
    unsigned char cc:BITS_2;
    unsigned char attach_mode:BITS_3;
    unsigned char rp:BITS_2;
    unsigned char vbus:BITS_1;
    char stub5;
    char stub6;
    char stub7;
    char stub8;
    char stub9;
    unsigned char cable: BITS_2;
    unsigned char reserved1: BITS_3;
    unsigned char reserved2: BITS_3;
    char stub11;
    char stub12;
    char stub13;
    char stub14;
    char stub15;
    char stub16;
    char stub17;
    char stub18;
    char stub19;
} nxpstat;


/*global struct for usb typec*/
struct _hw_usbc_driver g_hw_usbc_driver[TYPEC_SUPPORT_CHIPS];
static pistat s_pi_stat;
static tustat s_tu_stat;
static fustat s_fu_stat;
static nxpstat s_nxp_stat;
static int    g_curr_idx = -1;/*record the current valid chip type index*/
static struct regulator* g_typc_cc = NULL;
static int s_cc_status = CC_ERROR;
/*��USB ����ʱ��Ҫͬ��*/
static DECLARE_WAIT_QUEUE_HEAD(typec_wait);

/*
perl chip
*/
#define PERL_ID_REG 0x01
#define PERL_CONTROL_REG 0x02
#define PERL_INT_REG 0X03
#define PERL_CC_STATUS_REG 0x04
#define PERL_MODE_DRP 0x04

/*ti chip*/
#define TI_ID_REG 0X01
#define TI_CONTROL_REG 0x08
#define TI_STATUS_REG 0x09
#define TI_MODE_REG 0x0a
#define TI_MODE_DRP 0x30

/*fairchild chip*/
#define FC_ID_REG 0x01
#define FC_MODE_REG 0x02
#define FC_CONTROL_REG 0x03
#define FC_MANUAL_REG 0x04
#define FC_RESET_REG 0x05
#define FC_MASK_REG 0x10
#define FC_STATUS_REG 0x11
#define FC_TYPE_REG 0x12
#define FC_INT_REG 0x13
#define FC_MODE_DRP 0x10
/*nxp chip*/
#define NXP_READ_MULTI_BYTES_FLAG 0x80
#define NXP_MODE_REG 0x02
#define NXP_MODE_DRP 0x04

/*cable recognize*/
#define NXP_CABLE_AVAIL 0x00
#define TI_CABLE_AVAIL 0x0
#define NXP_VCONN_EN 0x43
#define NXP_VCONN_AVAIL 0xe0
/*
typec switch on/off function
*/
int power_on_type_cc(void);
int power_off_type_cc(void);
static int usbc_i2c_read(usb_typec_driver* usbc, char* reg, int num);
static int usbc_i2c_write(usb_typec_driver* usbc, char value, int num);

/*byte write tyepc chip func*/
void usbc_write_device_register_byte( int addr, int value);
/* get typec mode status func*/
int usbc_get_current_mode(void);
/*****************************************************************************
��������   usbc_get_cc_status
��������:  ��ȡ��ǰCC ����״̬
�����������
����ֵ��   0:    δ���룬1��CC1���룬2 CC2���룬3��ERROR
*****************************************************************************/
int usbc_get_cc_status(void)
{
    return s_cc_status;
}
/*****************************************************************************
��������   usbc_get_mode_state
��������:  ͨ������ʱ��ȡ�Ĵ�����ȡ��ǰ�Ƿ�DFPģʽ
���������

����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
int usbc_check_host_mode(void)
{
    usb_typec_driver* usbc_driver = NULL;
    char buffer[TYPEC_MAX_REGS] = {0};
    int max_reg_num = 0;
    int i = 0;
    int ret = 0;
    int mode = false;

    DBG_I(MBB_TYPECREG, "usbc_get_mode_state enter.");

    if (-1 == g_curr_idx)
    {
        return mode;
    }

    usbc_driver = &(g_hw_usbc_driver[g_curr_idx]);
    max_reg_num = usbc_driver->reg_max;

    ret = usbc_i2c_read(usbc_driver, buffer, max_reg_num);
    if (0 != ret)
    {
        DBG_E(MBB_TYPECREG, "read err \n");
        return ret;
    }

    /* control the switch, throuth the CC status*/
    if (TYPEC_PERL_DEVICE == usbc_driver->usbc_i2c_client->addr)
    {
        usbc_driver->pivate = (pistat*)buffer;//PIоƬ
        if(TYPEC_PERL_DFP == ((pistat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  true;
        }
    }
    /*
    for TI chip , the CC status reg bit only one,
    0 is for cc1, 1 is for cc2
    */
    else if (TYPEC_TI_DEVICE == usbc_driver->usbc_i2c_client->addr)
    {
        usbc_driver->pivate = (tustat*)buffer;
        if ((TYPEC_TI_DFP == ((tustat*)(usbc_driver->pivate))->attach_mode) &&
            (TI_CABLE_AVAIL == ((tustat*)(usbc_driver->pivate))->cable))
        {
            mode =  true;
        }

    }
    else if (TYPEC_FAIRCHILD_DEVICE == usbc_driver->usbc_i2c_client->addr)
    {
        usbc_driver->pivate = (fustat*)buffer;
        if(TYPEC_FC_DFP == ((fustat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  true;
        }
    }
    else
    {
        if (0 == usbc_i2c_write(usbc_driver, NXP_VCONN_AVAIL, NXP_VCONN_EN))
        {
            ret = usbc_i2c_read(usbc_driver, buffer, max_reg_num);
            if (0 == ret)
            {
                usbc_driver->pivate = (nxpstat*)buffer;

                if ((TYPEC_NXP_DFP == ((nxpstat*)(usbc_driver->pivate))->attach_mode) &&
                    (NXP_CABLE_AVAIL == ((nxpstat*)(usbc_driver->pivate))->cable))
                {
                    mode =  true;
                }
            }
        }
    }

    return mode;
}

/*****************************************************************************
��������   usbc_i2c_read
��������:  ��װi2c_master_recv�����type C�Ĵ���������
���������usbc ,ȫ�ֱ�������¼��ǰdriver��Ϣ
           * reg ��������ز��������ڶ�ȡ�ļĴ���ֵ
            num����Ҫ��ȡ�ļĴ�������
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
static int usbc_i2c_read(usb_typec_driver* usbc, char* reg, int num)
{
    int ret = 0;
    char  addr = 0;

    if ((NULL == usbc) || (NULL == usbc->usbc_i2c_client))
    {
        DBG_E(MBB_TYPECREG, "usbc/%p OR usbc_i2c_client doesn't exist@~\n", usbc);
        return TYPEC_ERROR_1;
    }
    if (TYPEC_PERL_DEVICE == usbc->usbc_i2c_client->addr)
    {
        /*��оƬ�Ĵ���*/
        ret = i2c_master_recv(usbc->usbc_i2c_client, reg, num);
    }
    else
    {
        if(TYPEC_NXP_DEVICE == usbc->usbc_i2c_client->addr)
        {
            addr |= NXP_READ_MULTI_BYTES_FLAG;/*for nxp chip,the high bit is for read multi bytes*/
        }
        ret = i2c_master_send(usbc->usbc_i2c_client, &addr, TYPEC_REG_NUM_1);
        if (TYPEC_REG_NUM_1 != ret)
        {
            DBG_E(MBB_TYPECREG, "i2c_master_send err %d\n", ret);
            return TYPEC_ERROR_1;
        }
        /*��оƬ�Ĵ���*/
        ret = i2c_master_recv(usbc->usbc_i2c_client, reg, num);
    }
    if (num != ret)
    {
        DBG_E(MBB_TYPECREG, "i2c_master_recv err %d\n", ret);
        return TYPEC_ERROR_1;
    }
    else
    {
        return 0;
    }
}

/*****************************************************************************
��������   usbc_reg_store
��������:  ��װi2c_master_send�����type C�Ĵ���д����
���������usbcȫ�ֱ�������¼��ǰdriver��Ϣ
            value д��Ĵ�����ֵ
            numҪд��ļĴ���
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
static int usbc_i2c_write(usb_typec_driver* usbc, char value, int num)
{
    int ret = 0;
    char regs[TYPEC_MAX_REGS] = {0};

    if ((NULL == usbc) || (NULL == usbc->usbc_i2c_client))
    {
        DBG_E(MBB_TYPECREG, "usbc/%p OR usbc_i2c_client doesn't exist@~\n", usbc);
        return TYPEC_ERROR_1;
    }
    /*��prelоƬ�Ĵ���*/
    if (TYPEC_PERL_DEVICE == usbc->usbc_i2c_client->addr)
    {
        /*�Ĵ���Ȩ��У�飬ֻ��2�żĴ�����д*/
        if (TYPEC_REG_NUM_2 != num)
        {
            DBG_E(MBB_TYPECREG, "only 0x02 can be writed\n");
            return TYPEC_ERROR_1;
        }
        /*i2c_master_send����ֱ��д2�żĴ�������Ҫ 1,2��һ��д��
          �����ȶ���1�żĴ���*/
        if (0 != usbc_i2c_read(usbc, regs, TYPEC_REG_NUM_1))
        {
            DBG_E(MBB_TYPECREG, "the read before write err\n");
            return TYPEC_ERROR_1;
        }
        /*����2�żĴ�����ֵ*/
        regs[TYPEC_REG_1] = value;
        ret = i2c_master_send(usbc->usbc_i2c_client, regs, TYPEC_REG_NUM_2);
        if (TYPEC_REG_NUM_2 != ret)
        {
            DBG_E(MBB_TYPECREG, "i2c_master_send err %d\n", ret);
            return TYPEC_ERROR_1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        regs[TYPEC_REG_0] = num;
        regs[TYPEC_REG_1] = value;
        ret = i2c_master_send(usbc->usbc_i2c_client, regs, TYPEC_REG_NUM_2);
        if (TYPEC_REG_NUM_2 != ret)
        {
            DBG_E(MBB_TYPECREG, "i2c_master_send err %d\n", ret);
            return TYPEC_ERROR_1;
        }
        else
        {
            return 0;
        }
    }

}

/*****************************************************************************
��������   usbc_i2c_set_ss
��������:  ͨ����������ӷ�������ss�ܽ�
���������CC,��ǰCC״̬

����ֵ����

���뷽��    SS_SEL_EN     SS_OEN
CC1          L               L    An to Bn
CC2          H               L    An to Cn
X            X               H    Bn and Cn pins are high-Z
*****************************************************************************/

void usbc_i2c_set_ss(int cc)
{
    usb_typec_driver* usbc_driver = &(g_hw_usbc_driver[g_curr_idx]);
    int ss_sel_en = usbc_driver->gpio_ss_sel_en[0];
    int ss_oen = usbc_driver->gpio_ss_oen[0];
    DBG_I(MBB_TYPECREG, "cc=%x \n", cc);
    if (CC_NO_CONNECT == cc )
    {
        //gpio_direction_output(ss_sel_en, 1);
        gpio_direction_output(ss_oen, 1);
    }
    else if (CC1_CONNECT == cc )
    {
        gpio_direction_output(ss_sel_en, 0);
        gpio_direction_output(ss_oen, 0);
    }
    else if (CC2_CONNECT == cc )
    {
        gpio_direction_output(ss_sel_en, 1);
        gpio_direction_output(ss_oen, 0);
    }
    else
    {
        DBG_T(MBB_TYPECREG, "addr=%x \n", usbc_driver->usbc_i2c_client->addr);
        //gpio_direction_output(ss_sel_en, 1);
        gpio_direction_output(ss_oen, 1);

    }
    s_cc_status = cc;
}
/*****************************************************************************
��������   usbc_i2c_hotplug
��������:  ͨ������ʱ���¶�ȡ�Ĵ���״̬
���������action��1 typec���룬2 typec�Ƴ�

����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
int usbc_i2c_hotplug(int action)
{
    usb_typec_driver* usbc_driver = NULL;
    char buffer[TYPEC_MAX_REGS] = {0};
    int max_reg_num = 0;
    int i = 0;
    int ret = 0;
    int cc = CC_NO_CONNECT;

    DBG_I(MBB_TYPECREG, "action:%d before wait!", action);
    /*�ȴ���typec̽�����*/
    wait_event_interruptible(typec_wait, (g_curr_idx >= 0));
    usbc_driver = &(g_hw_usbc_driver[g_curr_idx]);
    max_reg_num = usbc_driver->reg_max;
    DBG_I(MBB_TYPECREG, "addr=%x \n,action:%d", usbc_driver->usbc_i2c_client->addr, action);

    switch (action)
    {
        case TYPEC_USB_LINE_INSERT:
        {
            /*�������ϵ�*/
            ret = power_on_type_cc();
            if (0 != ret)
            {
                DBG_E(MBB_TYPECREG, "power on typec  usb failed!\n");
                return ret;
            }
            ret = usbc_i2c_read(usbc_driver, buffer, max_reg_num);
            if (0 != ret)
            {
                DBG_E(MBB_TYPECREG, "read err \n");
                return ret;
            }

            /* control the switch, throuth the CC status*/
            if (TYPEC_PERL_DEVICE == usbc_driver->usbc_i2c_client->addr)
            {
                usbc_driver->pivate = (pistat*)buffer;//PIоƬ
                usbc_i2c_set_ss(((pistat*)(usbc_driver->pivate))->cc);
            }
            /*
            for TI chip , the CC status reg bit only one, 
            0 is for cc1, 1 is for cc2
            */
            else if (TYPEC_TI_DEVICE == usbc_driver->usbc_i2c_client->addr)
            {
                usbc_driver->pivate = (tustat*)buffer;
                if ( 0 == ((tustat*)(usbc_driver->pivate))->cc)
                {
                    cc = CC1_CONNECT;
                }
                else if ( 1 == ((tustat*)(usbc_driver->pivate))->cc)
                {
                    cc = CC2_CONNECT;
                }
                usbc_i2c_set_ss(cc);
            }
            else if (TYPEC_FAIRCHILD_DEVICE == usbc_driver->usbc_i2c_client->addr)
            {
                usbc_driver->pivate = (fustat*)buffer;
                usbc_i2c_set_ss(((fustat*)(usbc_driver->pivate))->cc);
            }
            else
            {
                usbc_driver->pivate = (nxpstat*)buffer;
                usbc_i2c_set_ss(((nxpstat*)(usbc_driver->pivate))->cc);
            }
            break;
        }
        case TYPEC_USB_LINE_REMOVE:
        {
            /*init try sink register*/
            usbc_try_sink_init();
            /*�������µ�*/
            ret = power_off_type_cc();
            if (0 != ret)
            {
                DBG_E(MBB_TYPECREG, "power off typec usb failed!\n");
            }
            usbc_i2c_set_ss(CC_NO_CONNECT);
            break;
        }
        default:
        {
            DBG_E(MBB_TYPECREG, "usbc_i2c_hotplug action:%d error!\n", action);
            break;
        }
    }
    return 0;
}

/*****************************************************************************
��������   usbc_reg_show
��������:  ͨ��sysfs��register
���������pdev
            attr
            buff
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
static ssize_t usbc_reg_show(struct device* pdev, struct device_attribute* attr, char* buf)
{

    usb_typec_driver* usbc_driver = dev_get_drvdata(pdev);
    char buffer[TYPEC_MAX_REGS] = {0};
    int max_reg_num = 0;
    int i = 0;

    DBG_T(MBB_TYPECREG, "addr=%x \n", usbc_driver->usbc_i2c_client->addr);

    max_reg_num = usbc_driver->reg_max;
    /*read all the register info*/
    if (0 == usbc_i2c_read(usbc_driver, buffer, max_reg_num))
    {
        for (i = 0; i < max_reg_num; i++)
        {
            DBG_T(MBB_TYPECREG, "read index 0%xH = 0x%02X \n", i, buffer[i]);
        }
    }
    else
    {
        DBG_T(MBB_TYPECREG, "read err \n");
    }
    return 0;
}

/*****************************************************************************
��������   usbc_reg_store
��������:  ͨ��sysfsдregister
���������pdev
            attr
            buff
            size
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
ssize_t usbc_reg_store(struct device* pdev, struct device_attribute* attr,
                        const char* buff, size_t size)
{
    usb_typec_driver* usbc_driver = dev_get_drvdata(pdev);
    char reg = 0;
    char buf[USB_NUM_256] = {0};
    int i = 0;
    char* cur_str = buf;
    int reg_num = 0;
    int reg_value = 0;
    int buf_size = 0;
    int ret = 0;

    buf_size = min(size, (sizeof(buf) - 1));
    /*�ȿ��������������*/
    strlcpy(buf, buff, buf_size);
    /*�����ո�*/
    while (' ' == *cur_str)
    {
        cur_str++;
        i++;
        if (TYPEC_MAX_DATALEN <= i)
        {
            DBG_E(MBB_TYPECREG, "too much ' ' err \n");
            return size;
        }
    }
    /*�������Ĵ�����ַ*/
    if (0 == sscanf(cur_str, "%4d", &reg_num))
    {
        DBG_E(MBB_TYPECREG, "get reg_num err \n");
        return size;
    }
    DBG_T(MBB_TYPECREG, " reg_num is %d \n", reg_num);

    i = 0;
    /*�ҵ�������*/
    while (cur_str)
    {
        if ( ',' == *cur_str)
        {
            cur_str++;
            break;
        }
        cur_str++;
        i++;
        if (TYPEC_MAX_DATALEN  <= i)
        {
            DBG_E(MBB_TYPECREG, " no ',' err \n");
            return size;
        }
    }

    i = 0;
    /*�����ո�*/
    while (' ' == *cur_str)
    {
        cur_str++;
        i++;
        if (TYPEC_MAX_DATALEN  <= i)
        {
            DBG_E(MBB_TYPECREG, " no reg value  err \n");
            return size;
        }
    }
    if (0 == sscanf(cur_str, "%4x", &reg_value))
    {
        DBG_E(MBB_TYPECREG, "get reg_value ' ' err \n");
        return size;
    }
    reg = (char)reg_value;
    DBG_T(MBB_TYPECREG, " reg value is %x \n", reg_value);
    ret = usbc_i2c_write(usbc_driver, reg_value, reg_num);

    if (0 != ret)
    {
        DBG_E(MBB_TYPECREG, "usbc_reg_store set failed \n");
    }
    return size;
}

static DEVICE_ATTR(usbc_reg, S_IRUGO | S_IWUSR, usbc_reg_show, usbc_reg_store);

/*****************************************************************************
��������   usbc_help_show
��������:  help for write/read register
���������pdev
            attr
            buff
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
static ssize_t usbc_help_show(struct device* pdev, struct device_attribute* attr, char* buf)
{
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|cat usbc_reg can print all the chip reg value---   ----------|\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------------|\n");

    DBG_T(MBB_DEBUG, "|-------------------------------------------------------------|\n");
    DBG_T(MBB_DEBUG, "|echo string as (reg_num reg_value) to usbc_reg:--------------|\n");
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------------|\n");
    return 0;
}

static DEVICE_ATTR(usbc_help, S_IRUGO, usbc_help_show, NULL);

static struct device_attribute* usbc_attributes[] =
{
    &dev_attr_usbc_reg,
    &dev_attr_usbc_help,
    NULL,
};
/*****************************************************************************
��������   usbc_create_sysfile
��������:  �����豸�ļ�
���������hw_usbc��index ��ǰ��Ч��chip������Ϣ
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/

static int usbc_create_sysfile(usb_typec_driver* hw_usbc, int index)
{
    int error = 0;
    struct device_attribute* attr = NULL;
    struct device_attribute** attrs = usbc_attributes;

    char class_name[TYPEC_BUF_LEN] = {0};
    char dev_name[TYPEC_BUF_LEN] = {0};

    usb_typec_driver* usbc = NULL;

    usbc = hw_usbc + index;

    snprintf(class_name, TYPEC_BUF_LEN, "typec%d", index);

    snprintf(dev_name, TYPEC_BUF_LEN, "dev%d", index);

    usbc->usbc_class = class_create(THIS_MODULE, class_name);
    if (NULL == usbc->usbc_class)
    {
        DBG_E(MBB_TYPECREG, "usbc_class create err.\n");
        return TYPEC_ERROR_1;
    }
    usbc->usbc_dev = device_create(usbc->usbc_class, NULL,
                                   MKDEV(0, 0), NULL, dev_name);
    if (NULL == usbc->usbc_dev)
    {
        DBG_E(MBB_TYPECREG, "usbc_dev create err.\n");
        return TYPEC_ERROR_2;
    }
    while ((attr = *attrs++))
    {
        error = device_create_file(usbc->usbc_dev, attr);
        if (0 != error)
        {
            DBG_E(MBB_TYPECREG, "attrs create err.\n");
            continue;
        }
    }
    dev_set_drvdata(usbc->usbc_dev, usbc);
    return error;
}
/*****************************************************************************
��������   usbc_set_init_mode
��������:  typec��ʼ������chip����ģʽΪDRP
���������usbc��ǰ��Ч��chip������Ϣ
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
void usbc_set_init_mode(usb_typec_driver* usbc)
{

    int addr = 0;
    int reg = 0;
    int value = 0;
    int ret = 0;

    if(NULL == usbc)
    {
        return;
    }

    addr = usbc->usbc_i2c_client->addr;

    if(TYPEC_FAIRCHILD_DEVICE == addr)
    {
        reg = FC_MODE_REG;
        value = FC_MODE_DRP;
    }
    else if(TYPEC_PERL_DEVICE == addr)
    {
        reg = PERL_CONTROL_REG;
        value = PERL_MODE_DRP;
    }
    else if(TYPEC_TI_DEVICE == addr)
    {
        reg = TI_MODE_REG;
        value = TI_MODE_DRP;
    }
    else
    {
        reg = NXP_MODE_REG;
        value = NXP_MODE_DRP;
    }
    ret = usbc_i2c_write(usbc, value, reg);

    if (0 != ret)
    {
        DBG_T(MBB_TYPECREG, "TYPEC init DRP MODE set failed \n");
    }
    /*init try sink register*/
    usbc_try_sink_init();
}
/*****************************************************************************
��������   usbc_write_device_register_byte
��������:  д�Ĵ�������֧��I2C�ӿڣ���typecоƬ֧�ֵ�byteд��
���������addr��д��Ĵ�����ַ��value��д������
����ֵ��   ��
*****************************************************************************/

void usbc_write_device_register_byte( int addr, int value)
{
    usb_typec_driver* usbc = NULL;
    char buffer[TYPEC_REG_NUM_2] = {0};
    int ret = 0;

    DBG_I(MBB_TYPECREG, "usbc_write_device_register_byte enter!");
    if (0 <= g_curr_idx)
    {
        usbc = &(g_hw_usbc_driver[g_curr_idx]);
        /*доƬ�Ĵ���*/
        buffer[0] = addr;
        buffer[1] = value;
        ret = i2c_master_send(usbc->usbc_i2c_client, buffer, TYPEC_REG_NUM_2);
        if (TYPEC_REG_NUM_2 != ret)
        {
            DBG_E(MBB_TYPECREG, "i2c_master_send err %d\n", ret);
            return;
        }  
    }

}
/*****************************************************************************
��������   usbc_try_sink_init
��������:  ����chip productor ���飬TRY sink ��ʼ���Ĵ�������
�����������
����ֵ��   ��
*****************************************************************************/

void usbc_try_sink_init(void)
{

    usb_typec_driver* usbc_driver = NULL;

    /*check current typec driver is probed*/
    if (-1 == g_curr_idx)
    {
        return;
    }

    usbc_driver = &(g_hw_usbc_driver[g_curr_idx]);
    if (TYPEC_TI_DEVICE == usbc_driver->usbc_i2c_client->addr)
    {
        /* restore the TYPEC chip status :clear the try sink bit*/
        usbc_write_device_register_byte(TYPEC_REG_10, TYPEC_NUM_30H);
    }
    else if (TYPEC_NXP_DEVICE == usbc_driver->usbc_i2c_client->addr)
    {
        usbc_write_device_register_byte(TYPEC_REG_43H, TYPEC_NUM_C0H);
        usbc_write_device_register_byte(TYPEC_REG_4CH, TYPEC_NUM_24H);
        usbc_write_device_register_byte(TYPEC_REG_2, TYPEC_NUM_15H);
    }

}
/*****************************************************************************
��������   usbc_get_current_mode
��������:  ͨ������ʱ��ȡ�Ĵ�����ȡ��ǰΪDFPģʽ��UFPģʽ��
                         �����޽��롣
�����������
����ֵ��   -1:    �޽��룬0��UFPģʽ��1:DFPģʽ
*****************************************************************************/
int usbc_get_current_mode(void)
{
    usb_typec_driver* usbc_driver = NULL;
    char buffer[TYPEC_MAX_REGS] = {0};
    int max_reg_num = 0;
    int ret = 0;
    int mode = TYPEC_UNATTACH;

    DBG_I(MBB_TYPECREG, "usbc_get_mode_state enter.");

    /*check current typec driver is probed*/
    if (-1 == g_curr_idx)
    {

        DBG_T(MBB_TYPECREG, "g_curr_idx is -1.");
        return mode;
    }

    usbc_driver = &(g_hw_usbc_driver[g_curr_idx]);
    max_reg_num = usbc_driver->reg_max;

    /*get the register buffer*/
    ret = usbc_i2c_read(usbc_driver, buffer, max_reg_num);
    if (0 != ret)
    {
        DBG_E(MBB_TYPECREG, "read err \n");
        return mode;
    }

    /*  get the current attach mode according the datasheet*/
    if (TYPEC_PERL_DEVICE == usbc_driver->usbc_i2c_client->addr)
    {
        usbc_driver->pivate = (pistat*)buffer;//PIоƬ
        if (TYPEC_PERL_DFP == ((pistat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  TYPEC_DFP;
        }
        else if (TYPEC_PERL_UFP == ((pistat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  TYPEC_UFP;
        }
    }
    else if (TYPEC_TI_DEVICE == usbc_driver->usbc_i2c_client->addr)
    {
        usbc_driver->pivate = (tustat*)buffer;
        if (TYPEC_TI_DFP == ((tustat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  TYPEC_DFP;
        }
        else if (TYPEC_TI_UFP == ((tustat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  TYPEC_UFP;
        }

    }
    else if (TYPEC_FAIRCHILD_DEVICE == usbc_driver->usbc_i2c_client->addr)
    {
        usbc_driver->pivate = (fustat*)buffer;
        if (TYPEC_FC_DFP == ((fustat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  TYPEC_DFP;
        }
        else  if (TYPEC_FC_UFP == ((fustat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  TYPEC_UFP;
        }
    }
    else
    {
        usbc_driver->pivate = (nxpstat*)buffer;

        if (TYPEC_NXP_DFP == ((nxpstat*)(usbc_driver->pivate))->attach_mode)
        {
            mode =  TYPEC_DFP;
        }
        else if (TYPEC_NXP_UFP == ((nxpstat*)(usbc_driver->pivate))->attach_mode)
        {
            mode = TYPEC_UFP;
        }
    }
    return mode;
}
/*****************************************************************************
��������   usbc_try_sink_attach
��������:  ����chip productor ���飬ִ��try sink������ǿ��ʶ��ΪUFPģʽ
�����������
����ֵ��   ��
*****************************************************************************/

int usbc_try_sink_attach(void)
{
    usb_typec_driver* usbc = NULL;
    int ret = TYPEC_UNATTACH;
    int addr = 0;

    DBG_T(MBB_TYPECREG, "test_try_sink!");
    
    if (0 <= g_curr_idx)
    {

        usbc = &(g_hw_usbc_driver[g_curr_idx]);
        if (NULL == usbc)
        {
            return ret;
        }

        addr = usbc->usbc_i2c_client->addr;

        g_irq_avail = false;
        if (TYPEC_TI_DEVICE == addr)
        {
            /*set the try sink bit, try to work on device mode : for B-TYPEC cable for MAC PC*/
            usbc_write_device_register_byte(TYPEC_REG_10, TYPEC_NUM_3aH);
            /*300ms for cc detect reset and DFP is attached*/
            msleep(TYPEC_TRY_SINK_DELAY_300);
            ret = usbc_get_current_mode();
            if ( TYPEC_UFP == ret)
            {
                DBG_T(MBB_TYPECREG, "TYPEC test_try_sink UFP is OK \n");
            }
            else
            {
                /*try sink failed, it is TYPEC cable for excharger, so restore the try sink status*/
                usbc_write_device_register_byte(TYPEC_REG_10, TYPEC_NUM_30H);
                /*300ms for cc detect reset and UFP is attached*/
                msleep(TYPEC_TRY_SINK_DELAY_300);
                ret = TYPEC_DFP;
            }
        }
        else if (TYPEC_NXP_DEVICE == addr)
        {
            /*set the try sink bit, try to work on device mode : for B-TYPEC cable for MAC PC*/
            usbc_write_device_register_byte(TYPEC_REG_43H, TYPEC_NUM_40H);
            usbc_write_device_register_byte(TYPEC_REG_4CH, TYPEC_NUM_34H);
            usbc_write_device_register_byte(TYPEC_REG_2, TYPEC_NUM_11H);
            /* 150ms for DFP is attached, try sink need 100ms, 50ms for DFP attach OK
             Maybe,the Rp is detected, but the DFP device is not attached, so the VBUS
             is not supply. In this case, the try sink is failed.*/
            msleep(TYPEC_TRY_SINK_DELAY_150);
            ret = usbc_get_current_mode();
            if ( TYPEC_UFP == ret)
            {
                DBG_T(MBB_TYPECREG, "TYPEC test_try_sink UFP is OK \n");
            }
            else
            {
               /*try sink failed, it is TYPEC cable for excharger, so restore the try sink status*/
                usbc_try_sink_init();
                /* 300ms for cc detect reset,*/
                msleep(TYPEC_TRY_SINK_DELAY_300);
                ret = usbc_get_current_mode();
            }
        }
        else
        {
            ret = TYPEC_DFP;
        }
        g_irq_avail = true;

    }
    return ret;
}
/*****************************************************************************
��������   usbc_get_irq_flag
��������:  try sink�����У�����id�ж��ݲ�����
�����������
����ֵ��   ��ǰid�ж��Ƿ���Ҫ����
*****************************************************************************/

int usbc_get_irq_flag()
{
    return g_irq_avail;
}

/*****************************************************************************
��������   usbc_probe
��������:  ƥ�䵽����ʱ��ϵͳ����
���������client
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
static int usbc_probe(struct i2c_client* client,
                        const struct i2c_device_id* id)
{
    int ret = 0;
    static int s_index = 0;
    int gpio_num = 0;
    int gpio_value = 0;
    char regs[TYPEC_MAX_REGS] = {0};
    usb_typec_driver* usbc_drv = NULL;

    /*device have probed,*/
    if (0 <= g_curr_idx)
    {
        DBG_T(MBB_TYPECREG, "%s probe: have probed!\n", __FUNCTION__);
        return 0 ;
    }

    DBG_I(MBB_TYPECREG, "%s probe: name =%20s!\n", __FUNCTION__, id->name);
    if (0 == i2c_check_functionality(client->adapter,
                                 I2C_FUNC_SMBUS_BYTE_DATA))
    {
        DBG_E(MBB_TYPECREG, "%s: SMBUS_BYTE_DATA unsupported\n", __func__);
        //return - EIO;
    }

    usbc_drv = &g_hw_usbc_driver[s_index];
    usbc_drv->usbc_i2c_client = client;

    /*check current chip type*/
    if (TYPEC_PERL_DEVICE == usbc_drv->usbc_i2c_client->addr)
    {
        usbc_drv->pivate = &s_pi_stat;
        usbc_drv->reg_max = PERL_MAX_REG;
    }
    else if (TYPEC_TI_DEVICE == usbc_drv->usbc_i2c_client->addr)
    {
        usbc_drv->pivate = &s_tu_stat;
        usbc_drv->reg_max = TI_MAX_REG;
    }
    else if (TYPEC_FAIRCHILD_DEVICE == usbc_drv->usbc_i2c_client->addr)
    {
        usbc_drv->pivate = &s_fu_stat;
        usbc_drv->reg_max = FAIR_MAX_REG;
    }
    else
    {
        usbc_drv->pivate = &s_nxp_stat;
        usbc_drv->reg_max = NXP_MAX_REG;
    }

    DBG_I(MBB_TYPECREG, "s_index=%d,client=%x,addr=%x,name=%20s\n", s_index, (unsigned int)client, client->addr, client->name);

    /*GPIO_3_02��enable type CоƬ*/
    ret = of_property_read_u32_array(client->dev.of_node, "gpio_en", usbc_drv->gpio, TYPEC_GPIO_ATTR);
    if (0 != ret)
    {
        DBG_E(MBB_TYPECREG, "device tree information value gpio_en get fail\n");
        return - ENODEV;
    }
    gpio_num = usbc_drv->gpio[0];
    gpio_value = usbc_drv->gpio[1];

    DBG_I(MBB_TYPECREG, "device tree information gpio %d, default value =%d\n", gpio_num, gpio_value);

    if (0 == gpio_is_valid(gpio_num))
    {
        DBG_E(MBB_TYPECREG, "gpio is Failed %d\n", gpio_num);
        return ( - ENODEV);
    }
    ret = gpio_request(gpio_num, "usbc_enable");
    if (0 != ret)
    {
        DBG_T(MBB_TYPECREG, "gpio %%d have request \n");
    }
    /*ʹ�ܹܽ�Ĭ���Ѿ��򿪣�����ȷ��GPIO�Ѿ�����*/
    gpio_direction_output(gpio_num, gpio_value);
    DBG_I(MBB_TYPECREG, "gpio %d push %d \n", gpio_num, gpio_value);

    /*GPIO_10_06,Type-c���ؿ���*/
    ret = of_property_read_u32_array(client->dev.of_node, "ss_sel_en", usbc_drv->gpio_ss_sel_en, TYPEC_GPIO_ATTR);
    if (0 != ret)
    {
        DBG_E(MBB_TYPECREG, "device tree information value gpio_en get fail\n");
        return - ENODEV;
    }
    gpio_num = usbc_drv->gpio_ss_sel_en[0];
    gpio_value = usbc_drv->gpio_ss_sel_en[1];

    DBG_I(MBB_TYPECREG, "device tree information gpio %d, default value =%d\n", gpio_num, gpio_value);

    if (0 == gpio_is_valid(gpio_num))
    {
        DBG_E(MBB_TYPECREG, "gpio is Failed %d\n", gpio_num);
        return ( - ENODEV);
    }
    ret = gpio_request(gpio_num, "ss_sel_en");
    if (0 != ret)
    {
        DBG_T(MBB_TYPECREG, "gpio %%d have request \n");
    }

    /*GPIO_11_01,Type-c���ؿ���*/
    ret = of_property_read_u32_array(client->dev.of_node, "ss_oen", usbc_drv->gpio_ss_oen, TYPEC_GPIO_ATTR);
    if (0 != ret)
    {
        DBG_E(MBB_TYPECREG, "device tree information value gpio_en get fail\n");
        return - ENODEV;
    }
    gpio_num = usbc_drv->gpio_ss_oen[0];
    gpio_value = usbc_drv->gpio_ss_oen[1];

    DBG_I(MBB_TYPECREG, "device tree information gpio %d, default value =%d\n", gpio_num, gpio_value);

    if (0 == gpio_is_valid(gpio_num))
    {
        DBG_E(MBB_TYPECREG, "gpio is Failed %d\n", gpio_num);
        return ( - ENODEV);
    }
    ret = gpio_request(gpio_num, "ss_oen");
    if (0 != ret)
    {
        DBG_T(MBB_TYPECREG, "gpio %%d have request \n");
    }
    /*̽����Чtypecc i2c �豸*/
    if (0 == usbc_i2c_read(usbc_drv, regs, usbc_drv->reg_max))
    {
        g_curr_idx = s_index;
        /*��sysfs�д����ڵ㣬��������type C�Ĵ���*/
        usbc_create_sysfile(g_hw_usbc_driver, g_curr_idx);
        DBG_I(MBB_TYPECREG, "start set init mode \n");
        usbc_set_init_mode(usbc_drv);
        wake_up(&typec_wait);
    }
    else
    {
        DBG_T(MBB_TYPECREG, "device not present\n");
    }
    s_index++;
    return 0;
}


/*****************************************************************************
��������   usbc_remove
��������:  type cc�������ϵ�
�����������
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/

int power_on_type_cc(void)
{
    if (0 != IS_ERR(g_typc_cc))
    {
        DBG_E(MBB_TYPECREG,"g_typc_cc error:%d!/n", (int)PTR_ERR(g_typc_cc));
        return TYPEC_ERROR_1;
    }

    return regulator_enable(g_typc_cc);
}
/*****************************************************************************
��������   usbc_remove
��������:  type cc�������µ�
�����������
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/

int power_off_type_cc(void)
{
    if (0 != IS_ERR(g_typc_cc))
    {
        DBG_E(MBB_TYPECREG,"g_typc_cc error:%d!/n", (int)PTR_ERR(g_typc_cc));
        return TYPEC_ERROR_1;
    }

    return regulator_disable(g_typc_cc);
}


/*****************************************************************************
��������   usbc_remove
��������:  �����Ƴ������������ڰ��ϣ��������಻���Ƴ�
���������client
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/
static int usbc_remove(struct i2c_client* client)
{
    struct device_attribute* attr = NULL;
    struct device_attribute** attrs = usbc_attributes;


    usb_typec_driver* usbc = NULL;

    if ((0 > g_curr_idx) || (TYPEC_SUPPORT_CHIPS <= g_curr_idx))
    {
        return 0;
    }

    usbc = &g_hw_usbc_driver[g_curr_idx];

    if (NULL == usbc)
    {
        return 0;
    }
    while ((attr = *attrs++))
    {
        device_remove_file(usbc->usbc_dev, attr);
    }

    return 0;
}
static const struct i2c_device_id usbc_id[] =
{
    { .name = "PI5USB30216", },
    { .name = "TUSB320", },
    { .name = "FUSB310A", },
    { .name = "PTN5150H", },
    {},
};

/*��ƥ������*/
#ifdef CONFIG_OF
static const struct of_device_id usbc_match[] =
{
    { .compatible = "ti,usbc-i2c",},
    { .compatible = "peri,usbc-i2c",},
    { .compatible = "fairchild,usbc-i2c",},
    { .compatible = "nxp,usbc-i2c",},
    { },
};
MODULE_DEVICE_TABLE(of, usbc_match);
#endif
static struct i2c_driver usbc_driver =
{
    .driver    = {
        .name           = "usbc-i2c",
        .owner          = THIS_MODULE,
#ifdef CONFIG_OF
        .of_match_table = of_match_ptr(usbc_match),
#endif
    },
    .probe          = usbc_probe,
    .remove         = usbc_remove,
    .id_table       = usbc_id,
};


static int __init usbc_init(void)
{
    g_typc_cc = regulator_get(NULL, "typec-vcc");
    if (0 != IS_ERR(g_typc_cc))
    {
        DBG_E(MBB_TYPECREG,"g_typc_cc regulator_get error:%d!/n", (int)PTR_ERR(g_typc_cc));
        return TYPEC_ERROR_1;  
    }
    if(0 != regulator_set_voltage(g_typc_cc,TYPEC_REGULATOR_VOLT,TYPEC_REGULATOR_VOLT))
    {
        DBG_E(MBB_TYPECREG,"g_typc_cc regulator_set_voltage error/n");
        return TYPEC_ERROR_1;  
    }
    g_curr_idx = -1;

    return i2c_add_driver(&usbc_driver);
}
module_init(usbc_init);

static void __exit usbc_exit(void)
{
    g_curr_idx = -1;
    return i2c_del_driver(&usbc_driver);
}
module_exit(usbc_exit);


MODULE_AUTHOR("Huawei MBB");
MODULE_DESCRIPTION("USB TypeCC driver");
MODULE_LICENSE("GPL v2");

#else
/*****************************************************************************
��������   usbc_i2c_hotplug
��������:  �Բ�֧��i2c typec�豸�Ľ��д�׮�������ݴ���
�����������
            attr
            buff
����ֵ��   0:    ִ�гɹ�
*****************************************************************************/

int usbc_i2c_hotplug(int action)
{
    /*stub*/
    return 0;
}
int usbc_check_host_mode(void)
{
    /*stub*/
    return 0;
}

int usbc_try_sink_attach(void)
{
    /*stub*/
    return 0;
}
int usbc_get_irq_flag()
{
    return 1;
}
#endif

