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
#include <linux/i2c.h>
#include <linux/slab.h>
#include "mbb_client_codec.h"

#if ( FEATURE_ON == MBB_CLIENT_CODEC )
static struct nau8814 *nau8814_handle = NULL;
static REGISTER_SETTING_T sRESET_CMD = {0x00,0x0000};
static REGISTER_SETTING_T sNAU8814_CODEC_SETTING[] =
{
    {0x01,0x001d},
    {0x02,0x0015},
    {0x03,0x00ED},
    {0x04,0x0118},
    {0x06,0x0000},
    {0x07,0x000A},
    {0x0E,0x01B8},
    {0x2d,0x00b0},
    {0x36,0x003f}
};
static unsigned int sNUM_OF_NA8814_REG = 
    sizeof(sNAU8814_CODEC_SETTING) / sizeof(REGISTER_SETTING_T);

/******************************************************************************
Function:     nau8814_init
Description: save nua8814 codec i2c handle 
Input:         struct nau8814 *nau8814
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
static int nau8814_init(struct nau8814 *nau8814)
{
    nau8814_handle = nau8814;
    return 0;
}
/******************************************************************************
Function:    nau8814_release
Description:release nua8814 codec i2c handle 
Input:         struct nau8814 *nau8814
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
static void nau8814_release(void)
{
    nau8814_handle = NULL;
    return;
}
/******************************************************************************
Function:    nau8814_i2c_read
Description:read nua8814 codec from i2c interface 
Input:         void *io_data, void *read_data
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
static int nau8814_i2c_read(void *io_data, void *read_data)
{
    struct i2c_client* i2c = (struct i2c_client*)io_data;
    REGISTER_SETTING_T* reg_data = (REGISTER_SETTING_T*)read_data;
    int ret = -1;
    /*retry 3 times if failed*/
    int retry = 3;
    struct i2c_msg xfer[2];
    unsigned char addr = 0;
    unsigned short data = 0;

    if ( ( NULL == i2c ) || ( NULL == reg_data ) )
    {
        printk("%s:%d illegal input i2c =%d reg_data = %d\n",
               __FUNCTION__, __LINE__, i2c, reg_data);
        return ret;
    }
    addr = (unsigned char)(((reg_data->reg_addr) << 1) & (0xFE));

    /* Write register */
    xfer[0].addr = i2c->addr;
    xfer[0].flags = 0;
    xfer[0].len = 1;
    xfer[0].buf = &addr;/*lint !e64*/

    /* Read data */
    xfer[1].addr = i2c->addr;
    xfer[1].flags = I2C_M_RD;
    xfer[1].len = sizeof(unsigned short);
    xfer[1].buf = (unsigned char*)&data;/*lint !e64*/
    while ( retry-- )
    {
        ret = i2c_transfer(i2c->adapter, xfer, 2);
        if (ret == 2)
        {
            data = ntohs(data);
            (*reg_data).data = data;
            ret = 0;
            break;
        }
        else if (ret >= 0)
        {
            printk("%s:%d i2c_transfer ret = %d\n",
                   __FUNCTION__, __LINE__, ret);
            ret = -EIO;
        }
    }

    return ret;
}
/******************************************************************************
Function:    nau8814_i2c_write
Description:write nua8814 codec from i2c interface 
Input:         void *io_data, void *write_data
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
static int nau8814_i2c_write(void *io_data, void *write_data)
{
    int ret = -1;
    struct i2c_client *i2c = io_data;
    REGISTER_SETTING_T *reg_data = write_data;
    unsigned short msg = 0;
    /*retry 3 times if failed*/
    int retry = 3;
    
    if ( ( NULL == i2c ) || ( NULL == reg_data ) )
    {
        printk("%s:%d illegal input i2c =%d reg_data = %d\n",
            __FUNCTION__, __LINE__, i2c, reg_data);
        return ret;
    }
    
    msg = (unsigned short)((reg_data->reg_addr << 9) | (reg_data->data));
    msg = ntohs(msg);
    //msg = ((unsigned char)(reg_data->reg_addr << 1) | ( ( reg_data->data >> 8 ) & 0x01 )) 
    //            | ((unsigned char)( reg_data->data & 0xFF ) << 8);
    
    
    while ( retry-- )
    {
        ret = i2c_master_send(i2c, (char*)&msg, sizeof(unsigned short));
        if (ret == 2)
        {
            ret = 0;
            break;
        }
        if ( 0 != ret )
        {
            printk("%s:%d i2c_master_send failed with %d\n",
                __FUNCTION__, __LINE__, ret);
        }
    }
    return ret;
}
/******************************************************************************
Function:    nau8814_i2c_probe
Description:probe nua8814 codec i2c driver 
Input:        struct i2c_client *i2c, const struct i2c_device_id *id
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
static int nau8814_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
    struct nau8814* nau8814;
    int ret;
    if ( NULL != nau8814_handle)
    {
        printk(KERN_ERR"%s %d: already attached,return\n", __FUNCTION__, __LINE__);
        return 0;
    }
    nau8814 = kzalloc(sizeof(struct nau8814), GFP_KERNEL);/*lint !e516*/
    if (nau8814 == NULL)
    {
        ret = -ENOMEM;
        goto err;
    }

    nau8814->io_data = i2c;
    nau8814->read_dev = nau8814_i2c_read;
    nau8814->write_dev = nau8814_i2c_write;
    i2c_set_clientdata(i2c, nau8814);

    ret = nau8814_init(nau8814);
    if (ret != 0)
    { 
        goto struct_err; 
    }

    return 0;

struct_err:
    kfree(nau8814);
err:
    return ret;
}
/******************************************************************************
Function:    nau8814_i2c_remove
Description:remove nua8814 codec i2c driver 
Input:        struct i2c_client *i2c
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
static int nau8814_i2c_remove(struct i2c_client *i2c)
{
    struct nau8814* nau8814 = i2c_get_clientdata(i2c);
    kfree(nau8814);
    nau8814_release();
    return 0;
}
/******************************************************************************
Function:    nau8814_i2c_detect
Description:detect nua8814 codec i2c driver 
Input:        struct i2c_adapter *adapter
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
int nau8814_i2c_detect(struct i2c_client *i2c, struct i2c_board_info *info)
{
    strlcpy(info->type, "NAU8814", I2C_NAME_SIZE);
    return 0;
}
static const struct i2c_device_id nau8814_i2c_id[] = {
    { "NAU8814", 0 },
    { }
};
unsigned short nau8814_i2c_address[] = 
{
    NAU8814_CODEC_ADDR,
    I2C_CLIENT_END
};
static struct i2c_driver nau8814_i2c_driver =
{
    .class = I2C_CLASS_HWMON,
    .driver = {
        .name = "CODEC_NAU8814",
        .owner = THIS_MODULE,
    },
    .probe    = nau8814_i2c_probe,
    .remove   = nau8814_i2c_remove,
    .id_table = nau8814_i2c_id,
    .detect = nau8814_i2c_detect,
    .address_list = nau8814_i2c_address,
};
/******************************************************************************
Function:    nau8814_module_init
Description:init nua8814 codec i2c driver 
Input:        void
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
static int __init nau8814_module_init(void)
{
    int ret = -ENODEV;

    ret = i2c_add_driver(&nau8814_i2c_driver);

    if (ret != 0)
    {
        printk(KERN_ERR"%s:%d nau8814_module_init:Failed to register I2C driver: %d\n\n",
            __FUNCTION__, __LINE__, ret);
    }

    return ret;
}
module_init(nau8814_module_init);
/******************************************************************************
Function:    nau8814_module_exit
Description:delete nua8814 codec i2c driver 
Input:        void
Output:       NONE
Return:        void
Others:        NONE
******************************************************************************/
static void __exit nau8814_module_exit(void)
{
    i2c_del_driver(&nau8814_i2c_driver);
}
module_exit(nau8814_module_exit);
/******************************************************************************
Function:    nau8814_codec_init
Description:send register data to nua8814 codec from i2c driver 
Input:        void *io_data, unsigned char codec_type
Output:       NONE
Return:        int, 0 for success
Others:        NONE
******************************************************************************/
static int nau8814_codec_init(void *io_data, unsigned char codec_type)
{
    int ret = -1;
    unsigned int index = 0;
    struct nau8814 *nau8814= io_data;
    
    if ( NULL == nau8814 )
    {
        printk("%s:%d illegal input nau8814 =%d\n",
               __FUNCTION__, __LINE__, nau8814);
        return ret;
    }
    
    switch (codec_type)
    {
        case 0:
        {
            ret = nau8814->write_dev(nau8814->io_data, 
                &sRESET_CMD);
            if ( 0 != ret )
            {
                printk("%s:%d write to I2C failed on reset\n", 
                    __FUNCTION__, __LINE__);
            }
            break;
        }
        case 1:
        {
            for ( index = 0; index < sNUM_OF_NA8814_REG; index++)
            {
                ret = nau8814->write_dev(nau8814->io_data, 
                    &sNAU8814_CODEC_SETTING[index]);
                if ( 0 != ret )
                {
                    printk("%s:%d write to I2C failed on index %d\n", 
                        __FUNCTION__, __LINE__, index);
                    break;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return ret;
}
/******************************************************************************
Function:    nau8814_proc_cmd
Description:interface function which is exposed to AT module to config codec by i2c 
Input:        CODEC_INFO_T *codec_cmd
Output:      NONE
Return:        int, 0 for success
Others:       NONE
******************************************************************************/
int nau8814_proc_cmd(CODEC_INFO_T *codec_cmd)
{
    int ret = -1;
    CODEC_COMMAND_ID_E cmd_id = codec_cmd->cmd_id;
    REGISTER_SETTING_T *codec_data = codec_cmd->codec_data;
    
    if ( NULL == nau8814_handle )
    {
        printk("%s:%d nau8814_handle is NULL\n", __FUNCTION__, __LINE__);
        return ret;
    }

    switch (cmd_id)
    {
        case CODEC_POW_CMD:
        {
            ret = nau8814_codec_init(nau8814_handle, codec_cmd->codec_type);
            break;
        }
        case CODEC_SETREG_CMD:
        {
            ret = nau8814_handle->write_dev(nau8814_handle->io_data, codec_data);
            break;
        }
        case CODEC_GETREG_CMD:
        {
            ret = nau8814_handle->read_dev(nau8814_handle->io_data, codec_data);
            break;
        }
        default:
        {
            printk("%s:%d cmd %d does not support\n", __FUNCTION__, __LINE__, cmd_id);
            break;
        }
    }

    return ret;
}
#endif
