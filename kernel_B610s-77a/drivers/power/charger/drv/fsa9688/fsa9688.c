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

/**********************问题单修改记录******************************************
日    期              修改人         问题单号           修改内容

******************************************************************************/

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <asm/irq.h>
#include "fsa9688.h"

struct mutex hvdcp_detect_lock;

static int gpio = -1;
static struct i2c_client *this_client = NULL;
static struct work_struct   g_intb_work;
static int g_intb_irq_flag = 0xFB25DC69;  /* 共享中断时需要判断是否是自己需要的中断 */
/******************************************************************************
  Function      fsa9688_write_reg
  Description   对I2C设备寄存器指定地址写入数值
  Input         reg   :寄存器地址
  Output        val :待写入的值
  Return        0     :函数执行成功
                -1    :函数执行失败
  Others        N/A
******************************************************************************/
static int fsa9688_write_reg(uint8_t reg, uint8_t val)
{
    int ret = -1;
    if (NULL == this_client)
    {
        ret = (- ERR_NO_DEV);
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client=NULL!!! ret=%d\n", __func__, ret);
        return ret;
    }

    if ( (FSA9688_REG_MIN > reg) || (FSA9688_REG_MAX < reg) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: reg error!!! reg=%d\n", __func__, reg);
        return SWITCH_ERR;
    }

    /*val 取值范围为0--FF,不用判定合法性*/

    ret = i2c_smbus_write_byte_data(this_client, reg, val);
    if (ret < 0)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: i2c write error!!! ret=%d\n", __func__, ret);
        return SWITCH_ERR;
    }

    return SWITCH_OK;
}

/******************************************************************************
  Function      fsa9688_read_reg
  Description   读取I2C设备指定寄存器地址的值
  Input         reg   :寄存器地址pval:读取返回指针
  Output        val :待写入的值
  Return        0     :函数执行成功
                -1    :函数执行失败
  Others        N/A
******************************************************************************/
static int32_t fsa9688_read_reg(uint8_t reg, uint8_t *pval)
{
    if (NULL == this_client)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client=NULL!!!\n", __func__);
        return (- ERR_NO_DEV);
    }

    if (NULL == pval)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: pval=NULL!!!\n", __func__);
        return SWITCH_ERR;
    }

    if ( (FSA9688_REG_MIN > reg) || (FSA9688_REG_MAX < reg) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: reg error!!!\n", __func__);
        return SWITCH_ERR;
    }

    *pval = i2c_smbus_read_byte_data(this_client, reg);
    if (*pval < 0)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: i2c read error!!!\n", __func__);
        return SWITCH_ERR;
    }

    return SWITCH_OK;
}

/******************************************************************************
  Function      fsa9688_test_read_reg
  Description   读取fsa9688设备指定寄存器地址的值
  Input         reg   :寄存器地址pval:读取返回指针
  Output        NA
  Return        寄存器值
  Others        N/A
******************************************************************************/
uint8_t fsa9688_test_read_reg(uint8_t reg)
{
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    if ( (FSA9688_REG_MIN > reg) || (FSA9688_REG_MAX < reg) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: reg error!!! reg=%d\n", __func__, reg);
        return SWITCH_ERR;
    }

    ret = fsa9688_read_reg(reg, &reg_val);
    if (SWITCH_OK != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Read Reg[0x%02x] Error with 0x%x!\n", reg, ret, 0);
        return SWITCH_ERR;
    }

    chg_print_level_message(CHG_MSG_ERR,
                "Reg[0x%02x] = 0x%02x.\n", reg, reg_val, 0);
    return reg_val;
}

/******************************************************************************
  Function      fsa9688_dump_all_reg
  Description   读取fsa9688 所有寄存器
  Input         void
  Output        NA
  Return        0     :函数执行成功
                -1    :函数执行失败
  Others        N/A
******************************************************************************/
#define NEW_LINE_NUM    (4)
int32_t fsa9688_dump_all_reg(void)
{
    int32_t ret = 0;
    uint8_t reg_val = 0;
    uint32_t i = 0;

    if (NULL == this_client)
    {
        ret = (- ERR_NO_DEV);
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client=NULL!!! ret=%d\n", __func__, ret);
        return ret;
    }

    /*读取0x00--0x0f*/
    for(i = 1; i <= FSA9688_REG_DEVICE_TYPE_4; i++)
    {
        ret = fsa9688_read_reg(i, &reg_val);
        if (ret < 0)
        {
            chg_print_level_message(CHG_MSG_ERR, "Read Reg[0x%02x] Error with 0x%x!\n", i, reg_val);
            return SWITCH_ERR;
        }
        else
        {
            chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x ", i, reg_val);
            if (0 == (i % NEW_LINE_NUM))
            {
                chg_print_level_message(CHG_MSG_ERR, "\r\n", 0, 0, 0);
            }
        }
    }

    chg_print_level_message(CHG_MSG_ERR, "\n", 0, 0, 0);

    ret = fsa9688_read_reg(FSA9688_REG_RESET, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", FSA9688_REG_RESET, reg_val);

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_STATUS, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", FSA9688_REG_ACCP_STATUS, reg_val);

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_CNTL, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", FSA9688_REG_ACCP_CNTL, reg_val);

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_CMD, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", FSA9688_REG_ACCP_CMD, reg_val);

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_INT1, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", FSA9688_REG_ACCP_INT1, reg_val);

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_INT2, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", FSA9688_REG_ACCP_INT2, reg_val);

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_INT1_MASK, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", FSA9688_REG_ACCP_INT1_MASK, reg_val);

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_INT2_MASK, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", FSA9688_REG_ACCP_INT2_MASK, reg_val);

    return SWITCH_OK;
}

/******************************************************************************
  Function      fsa9688_dump_all_slave_reg
  Description   读取adapter所有寄存器
  Input         void
  Output        NA
  Return        0     :函数执行成功
                -1    :函数执行失败
  Others        N/A
******************************************************************************/
int32_t fsa9688_dump_all_slave_reg(void)
{
    int32_t ret;
    uint8_t reg_val = 0;

    if (NULL == this_client)
    {
        ret = (- ERR_NO_DEV);
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client=NULL!!! ret=%d\n", __func__, ret);
        return ret;
    }

    ret = fsa9688_read_reg(SLAVE_REG_DVCTYPE, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_DVCTYPE, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_SPEC_VER, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_SPEC_VER, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_SCNTL, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_SCNTL, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_SSTAT, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_SSTAT, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_ID_OUT0, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_ID_OUT0, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_CAPABILITIES, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_CAPABILITIES, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_DISCRETE_CAPABILITIES, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_DISCRETE_CAPABILITIES, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_ADAPTER_STATUS, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_ADAPTER_STATUS, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_VOUT_STATUS, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_VOUT_STATUS, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_OUTPUT_CONTROL, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_OUTPUT_CONTROL, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_VOUT_CONFIG, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_VOUT_CONFIG, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_DISCRETE_VOUT_0, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_DISCRETE_VOUT_0, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_DISCRETE_VOUT_1, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_DISCRETE_VOUT_1, reg_val);

    ret = fsa9688_read_reg(SLAVE_REG_DISCRETE_VOUT_2, &reg_val);
    chg_print_level_message(CHG_MSG_ERR, "Reg[0x%02x]=0x%02x \r\n", SLAVE_REG_DISCRETE_VOUT_2, reg_val);

    return SWITCH_OK;
}

/****************************************************************************
  Function:     fsa9688_is_hvdcp_support
  Description:  检测充电芯片是否支持HVDCP
  Input:        void
  Output:       NA
  Return:       执行结果SWITCH_OK:支持SWITCH_ERR:不支持
***************************************************************************/
static int32_t fsa9688_is_hvdcp_support(void)
{
    uint8_t reg_val = 0;
    int32_t ret = 0;
    if (NULL == this_client)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client is null.\n", __func__);
        return SWITCH_ERR;
    }

    ret = fsa9688_read_reg(FSA9688_REG_DEVICE_ID, &reg_val);
    if (SWITCH_OK != ret )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: read FSA9688_REG_DEVICE_ID err.\n", __func__);
        return SWITCH_ERR;
    }

    if ( FSA9688_VERSION_ID != (reg_val & FSA9688_VERSION_ID) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:no fsa9688,no support fcp, reg[0x1]=%d.\n", __func__, reg_val);
        return SWITCH_ERR;
    }
    else
    {
        return SWITCH_OK;
    }
}

/******************************************************************************
  Function      fsa9688_is_hvdcp_charger
  Description   检测是否已识别到FCP adapter
  Input         void
  Output        NA
  Return        SWITCH_OK     :fcp adapter
                SWITCH_ERR    :not fcp adapter
  Others        N/A
******************************************************************************/
static int32_t fsa9688_is_hvdcp_charger(void)
{
    int32_t ret = 0;
    uint8_t reg_val = 0;
    ret = fsa9688_is_hvdcp_support();
    if ( SWITCH_OK != ret)
    {
        return SWITCH_ERR;
    }

    ret = fsa9688_read_reg(FSA9688_REG_DEVICE_TYPE_4, &reg_val);
    if (ret < 0)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: read FSA9688_REG_DEVICE_TYPE_4 error!reg:%d\n", __func__,reg_val);
        return SWITCH_ERR;
    }

    if (FSA9688_ACCP_ADP_DETECTED == (reg_val & FSA9688_ACCP_ADP_DETECTED) )
    {
        return SWITCH_OK;
    }

    return SWITCH_ERR;
}

/******************************************************************************
  Function      hvdcp_get_charger_type
  Description   获取充电器类型
  Input         void
  Output        NA
  Return        charger typer
  Others        N/A
******************************************************************************/
chg_chgr_type_t  fsa9688_get_charger_type(void)
{
    chg_chgr_type_t charger_type = CHG_CHGR_INVALID;

    uint8_t reg_val = 0;
    int32_t ret = 0;
    if (NULL == this_client)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client=NULL! charger_type set to NONE\n", __func__);
        return charger_type;
    }

    ret = fsa9688_read_reg(FSA9688_REG_DEVICE_TYPE_1, &reg_val);
    if (ret < 0)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: read REG[%d] erro, val = %d, charger_type set to NONE\n",
            __func__, FSA9688_REG_DEVICE_TYPE_1, reg_val);
        return charger_type;
    }

    if ( FSA9688_USB_OTG_DETECTED == (reg_val & FSA9688_USB_OTG_MASK) )
    {
        charger_type = CHG_USB_OTG_CRADLE;
    }
    else if ( FSA9688_DCP_DETECTED == (reg_val & FSA9688_DCP_MASK) )
    {
        charger_type = CHG_WALL_CHGR;
    }
    else if ( FSA9688_CDP_DETECTED == (reg_val & FSA9688_CDP_MASK) )
    {
        charger_type = CHG_USB_HOST_PC;
    }
    else if ( FSA9688_SDP_DETECTED == (reg_val & FSA9688_SDP_MASK) )
    {
        charger_type = CHG_USB_HOST_PC;
    }
    else
    {
        charger_type = CHG_CHGR_INVALID;
    }

    ret = fsa9688_is_hvdcp_charger();
    if ( (CHG_WALL_CHGR == charger_type) && (SWITCH_OK == ret) )
    {
        /*if is HVDCP ,report hvdcp,because when we detect fcp last time ,FSA9688_REG_DEVICE_TYPE_4 will be set */
        charger_type = CHG_HVDCP_CHGR;
        chg_print_level_message(CHG_MSG_INFO, "%s:update charger type by device type4, charger type is:%d\n",__func__,charger_type);
    }

    return charger_type;
}

/******************************************************************************
  Function      accp_adapter_reg_read
  Description   读取适配器寄存器
  Input         reg   :寄存器地址
  Output        *pval :用于将寄存器的值写入该整型数据所在地址
  Return        0     :函数执行成功
                -1    :函数执行失败
  Others        N/A
******************************************************************************/
static int32_t fsa9688_adapter_reg_read(uint8_t reg, uint8_t* val)
{
    uint8_t reg_val = 0;
    int32_t ret = 0;

    if (NULL == val)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: val is NULL.\n", __func__);
        return SWITCH_ERR;
    }

    if (reg > FSA9688_SLAVE_REG_MAX)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: reg invalid reg=%d.\n", __func__, reg);
        return SWITCH_ERR;
    }

    fsa9688_write_reg(FSA9688_REG_ACCP_CMD, FCP_CMD_SBRRD);
    fsa9688_write_reg(FSA9688_REG_ACCP_ADDR, reg);
    fsa9688_write_reg(FSA9688_REG_ACCP_CNTL, FSA9688_ACCP_PROTL_ENANBLED | FSA9688_SNDCMD_EN);
    mdelay(DEALY_50MS);

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_INT1, &reg_val);
    if (SWITCH_OK != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:read reg[0x59] err ret=%d.\n", __func__, ret);
        return SWITCH_ERR;
    }

    if ( FSA9688_CRCPAR_INT == (reg_val & FSA9688_CRCPAR_INT) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:crc or par err by not responding,reg[0x59]=%d.\n", __func__, reg_val);
        return SWITCH_ERR;
    }

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_INT2, &reg_val);
    if (SWITCH_OK != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:read reg[0x59] err ret=%d.\n", __func__, ret);
        return SWITCH_ERR;
    }

    if ( 0 != (reg_val & (FSA9688_CRCRX_INT | FSA9688_PARRX_INT)) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:crc or par err from slave,reg[0x5a]=%d.\n", __func__, reg_val);
        return SWITCH_ERR;
    }

    ret = fsa9688_read_reg(FSA9688_REG_ACCP_DATA, val);

    return ret;
}

/******************************************************************************
  Function      fsa9688_test_read_adapter_reg
  Description   读取fsa9688 slave(adapter)设备指定寄存器地址的值
  Input         reg   :寄存器地址pval:读取返回指针
  Output        NA
  Return        寄存器值
  Others        N/A
******************************************************************************/
uint8_t fsa9688_test_read_adapter_reg(uint8_t reg)
{
    uint8_t reg_val = 0;
    int32_t ret     = 0;

    if (reg > FSA9688_SLAVE_REG_MAX)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: reg invalid reg=%d.\n", __func__, reg);
        return SWITCH_ERR;
    }

    ret = fsa9688_adapter_reg_read(reg, &reg_val);
    if (SWITCH_OK != ret)
    {
        chg_print_level_message(CHG_MSG_ERR,
            "Read Reg[0x%02x] Error with 0x%x!\n", reg, ret, 0);
        return SWITCH_ERR;
    }

    chg_print_level_message(CHG_MSG_ERR,
        "Reg[0x%02x] = 0x%02x.\n", reg, reg_val, 0);
    return reg_val;
}

/******************************************************************************
  Function      fsa9688_adapter_reg_write
  Description   写适配器寄存器
  Input         reg   :寄存器地址
  Output        val :待写入的值
  Return        0     :函数执行成功
                SWITCH_ERR    :函数执行失败
  Others        N/A
******************************************************************************/
static int32_t fsa9688_adapter_reg_write(uint8_t reg, uint8_t val)
{
    uint8_t reg_val = 0;
    int32_t ret = 0;
    if (FSA9688_SLAVE_REG_MAX < reg)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:reg err!reg =%d.\n", __func__, reg);
        return SWITCH_ERR;
    }
    /*val 取值范围0x00--0xff，不做校验*/

    fsa9688_write_reg(FSA9688_REG_ACCP_CMD, FCP_CMD_SBRWR);
    fsa9688_write_reg(FSA9688_REG_ACCP_ADDR, reg);
    fsa9688_write_reg(FSA9688_REG_ACCP_DATA, val);
    fsa9688_write_reg(FSA9688_REG_ACCP_CNTL, FSA9688_ACCP_PROTL_ENANBLED | FSA9688_SNDCMD_EN);
    mdelay(DEALY_50MS);

   /*only show err,not deal with*/
    ret = fsa9688_read_reg(FSA9688_REG_ACCP_INT1, &reg_val);
    if (ret != SWITCH_OK)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:read reg[0x59] err!=%d.\n", __func__, reg_val);
        return SWITCH_ERR;
    }

    if (reg_val & FSA9688_CRCPAR_INT)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:crc or par err by not responding,reg[0x59]=%d.\n", __func__, reg_val);
        return SWITCH_ERR;
    }
    ret = fsa9688_read_reg(FSA9688_REG_ACCP_INT1, &reg_val);

    if (SWITCH_OK != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:read reg[0x5a] err!=%d.\n", __func__, reg_val);
        return SWITCH_ERR;
    }

    if ( 0 != (reg_val & (FSA9688_CRCRX_INT | FSA9688_PARRX_INT)) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:crc or par err from slave,reg[0x5a]=%d.\n", __func__, reg_val);
        return SWITCH_ERR;
    }

    return SWITCH_OK;
}

/******************************************************************************
  Function      fsa9688_get_supported_max_vol
  Description   获取accp adapter支持输出的最大电压
  Input         返回电压值指针
  Output        vol(5V/9V/12V)
  Return        0     :获取成功
                -1    :获取失败
  Others        N/A
******************************************************************************/
static int32_t fsa9688_get_supported_max_vol(uint8_t *vol)
{
    uint8_t reg_val = 0;
    uint8_t output_vol = 0;
    chg_chgr_type_t charger_type = CHG_CHGR_INVALID;

    if (NULL == this_client)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client is null.\n", __func__);
        return SWITCH_ERR;
    }

    charger_type = fsa9688_get_charger_type();
    if (CHG_HVDCP_CHGR != charger_type)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: the charger is not hvdcp,not supported.\n", __func__);
        return SWITCH_ERR;
    }

    /*get adapter vol list number,exclude 5V*/
    if (0 != fsa9688_adapter_reg_read(SLAVE_REG_DISCRETE_CAPABILITIES, &reg_val) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: read reg[0x21] err.\n", __func__);
        return SWITCH_ERR;
    }

    /*currently,fcp only support three out vol config(5v/9v/12v)*/
    if (reg_val > 2)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: vout list support err, reg[0x21] = %d.\n", __func__, reg_val);
        return SWITCH_ERR;
    }

    /*get max out vol value*/
    if (0 != fsa9688_adapter_reg_read(SLAVE_REG_DISCRETE_OUT_V(reg_val), &output_vol) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: read reg %d err.\n", __func__, SLAVE_REG_DISCRETE_OUT_V(output_vol));
        return SWITCH_ERR;
    }

    *vol = output_vol / HVDCP_VOL_SETP;
    chg_print_level_message(CHG_MSG_INFO, "%s: get adapter max out vol = %d,num= %d.\n", __func__, output_vol,output_vol);
    return SWITCH_OK;
}

/****************************************************************************
  Function:     fsa9688_convert_to_hvdcp_vol
  Description:  将待设置的电压枚举成电压值
  Input:        NA
  Output:       NA
  Return:        0: success
                -1: fail
***************************************************************************/
static uint8_t fsa9688_convert_to_hvdcp_vol(chg_hvdcp_type_value vol_set)
{
    switch (vol_set)
    {
        case CHG_HVDCP_5V:
        {
            vol_set = HVDCP_OUTPUT_VOL_5V;
            break;
        }
        case CHG_HVDCP_9V:
        {
            vol_set = HVDCP_OUTPUT_VOL_9V;
            break;
        }
        case CHG_HVDCP_12V:
        {
            vol_set = HVDCP_OUTPUT_VOL_12V;
            break;
        }
        default:
        {
            vol_set = HVDCP_OUTPUT_VOL_5V;
            break;
        }

    }
    return vol_set;
}

/****************************************************************************
  Function:     chg_get_hvdcp_adapter_max_power
  Description:  获取HVDCP adapter最大输出功率
  Input:        pow_val:功率值指针
  Output:       pow_val:功率值
  Return:       执行结果0:成功-1:失败
***************************************************************************/
int32_t fsa9688_get_hvdcp_adapter_max_power(uint8_t *pow_val)
{
    int32_t ret = 0;
    if ( (NULL == this_client) || (NULL == pow_val) )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client or pow_val is null.\n", __func__);
        return SWITCH_ERR;
    }

    /*read max power*/
    ret = fsa9688_adapter_reg_read(SLAVE_REG_MAX_PWR, pow_val);
    if (0 == ret)
    {
        chg_print_level_message(CHG_MSG_INFO, "%s: max power reg[0x22] = %d.\n", __func__, *pow_val);
        return SWITCH_OK;
    }
    chg_print_level_message(CHG_MSG_INFO, "%s: read SLAVE_REG_MAX_PWR err.\n", __func__);
    return SWITCH_ERR;
}

/****************************************************************************
  Function:     fsa9688_reset_reg
  Description:  检测充电芯片是否支持HVDCP
  Input:        void
  Output:       NA
  Return:       执行结果-1:执行失败；0:执行ok
***************************************************************************/
static int32_t fsa9688_reset_reg(void)
{
    uint8_t reg_val = 0;
    int32_t ret = 0;
    if (NULL == this_client)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:this_client is null.\n", __func__);
        return SWITCH_ERR;
    }

    ret = fsa9688_read_reg(FSA9688_REG_RESET, &reg_val);
    if ( ret < 0 )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:read FSA9688_REG_RESET fail,.\n", __func__);
        return SWITCH_ERR;
    }
    else
    {
        reg_val |= FSA9688_RESET;
        ret = fsa9688_write_reg(FSA9688_REG_RESET, reg_val);
        if (ret < 0)
        {
            chg_print_level_message(CHG_MSG_ERR, "%s:write FSA9688_REG_RESET fail.\n", __func__);
            return SWITCH_ERR;
        }

        return SWITCH_OK;
    }
}

/****************************************************************************
  Function:     chg_get_hvdcp_adapter_status
  Description:  获取DVDCP充电器状态
  Input:        adp_status,状态值指针
  Output:       adp_status
  Return:       执行结果0:成功-1:失败
***************************************************************************/
int32_t fsa9688_get_hvdcp_adapter_status (uint16_t *adp_status)
{
    uint8_t val = 0;
    int32_t ret = 0;

    if (NULL == adp_status)
    {
        return SWITCH_ERR;
    }

    ret = fsa9688_adapter_reg_read(SLAVE_REG_ADAPTER_STATUS, &val);
    if (0 != ret)
    {
        return SWITCH_ERR;
    }

    if( SLAVE_ADAPTER_OVLT == (val & SLAVE_ADAPTER_OVLT))
    {
        *adp_status = SLAVE_ADAPTER_OVLT;
        return 0;
    }

    if( SLAVE_ADAPTER_OCURRENT == (val & SLAVE_ADAPTER_OCURRENT))
    {
        *adp_status = SLAVE_ADAPTER_OCURRENT;
        return SWITCH_OK;
    }

    if( SLAVE_ADAPTER_OTEMP == (val & SLAVE_ADAPTER_OTEMP))
    {
        *adp_status = SLAVE_ADAPTER_OTEMP;
        return SWITCH_OK;
    }

    /*normal status*/
    *adp_status = 0;
    return SWITCH_OK;
}

#ifdef CONFIG_OF
static const struct of_device_id switch_fsa9688_ids[] =
{
    { .compatible = "hisilicon,fairchild_fsa9688" },
    {},
};
MODULE_DEVICE_TABLE(of, switch_fsa9688_ids);
#endif

/****************************************************************************
  Function:     fsa9688_irq_handler
  Description:  INTB中断处理函数
  Input:        irq，dev_id
  Output:       NA
  Return:       中断处理结果
***************************************************************************/
static irqreturn_t fsa9688_irq_handler(int irq, void *dev_id)
{
    int *pintb_flag = NULL;

    if(NULL == dev_id)    
    {
        return IRQ_NONE;    
    }
    pintb_flag = (int *)dev_id;

    if(g_intb_irq_flag != *pintb_flag )    
    {
        return IRQ_NONE;
    }    

    if ( gpio < 0 )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:gpio err!\n", __func__);
        return IRQ_NONE;
    }

    schedule_work(&g_intb_work);

    return IRQ_HANDLED;
}
/****************************************************************************
  Function:     fsa9688_intb_work
  Description:  INTB中断work函数
  Input:        work
  Output:       void
  Return:       执行结果0:成功-1:失败
***************************************************************************/
static void fsa9688_intb_work(struct work_struct *work)
{
    uint8_t reg_val = 0;
    int32_t ret = 0;
    if (NULL == this_client)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:this_client is null.\n", __func__);
        return;
    }

    mutex_lock(&hvdcp_detect_lock);
    /*读取中断寄存器，自动清除*/
    ret = fsa9688_read_reg(FSA9688_REG_INTERRUPT, &reg_val);
    if ( ret < 0 )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: read FSA9688_REG_INTERRUPT error!!! reg_ctl=%d.\n", __func__, reg_val);
    }
    mutex_unlock(&hvdcp_detect_lock);

    return;
}

/****************************************************************************
  Function:     fsa9688_reg_init
  Description:  fsa9688寄存器初始化
  Input:        void
  Output:       NA
  Return:       寄存器初始化结果SWITCH_ERR:失败；SWITCH_ERR:成功
***************************************************************************/
int32_t fsa9688_reg_init(void)
{
    int32_t ret = 0;
    uint8_t reg_ctl = 0;

    /*reset all reg*/
    if (SWITCH_OK != fsa9688_reset_reg() )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: fsa9688_reset_reg error!!\r\n", __func__);
        return SWITCH_ERR;
    }

    /* clear INT MASK (enable intb)*/
    ret = fsa9688_read_reg(FSA9688_REG_CONTROL1, &reg_ctl);
    if ( ret < 0 )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: read FSA9688_REG_CONTROL1 error!!! ret=%d.\n", __func__, ret);
        return SWITCH_ERR;
    }

    reg_ctl &= (~FSA9688_INT_MASK);
    ret = fsa9688_write_reg(FSA9688_REG_CONTROL1, reg_ctl);
    if ( ret != SWITCH_OK )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: write FSA9688_REG_CONTROL1 error!!! ret=%d.\n", __func__, ret);
        return SWITCH_ERR;
    }

    /*mask accp int*/
    ret = fsa9688_write_reg(FSA9688_REG_ACCP_INT1_MASK, FSA9688_MASK_ACCP_INT);
    if ( ret < 0 )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: write FSA9688_REG_ACCP_INT1_MASK error!!! ret=%d", __func__, ret);
        return SWITCH_ERR;
    }
    ret = fsa9688_write_reg(FSA9688_REG_ACCP_INT2_MASK, FSA9688_MASK_ACCP_INT);
    if ( ret < 0 )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: write FSA9688_REG_ACCP_INT2_MASK error!!! ret=%d", __func__, ret);
        return SWITCH_ERR;
    }

    /*Read FSA9688_REG_CONTROL2 reg*/
    ret = fsa9688_read_reg(FSA9688_REG_CONTROL2, &reg_ctl);
    if ( ret < 0 )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: read FSA9688_REG_CONTROL2 error!!! ret=%d.\n", __func__, ret);
        return SWITCH_ERR;
    }

    /*enable ACCP auto dedect*/
    reg_ctl |= (FSA9688_ACCP_BLOCK_EN | FSA9688_ACCP_AUTO_PROTL_EN | FSA9688_ACCP_OSC_EN);
    ret = fsa9688_write_reg(FSA9688_REG_CONTROL2, reg_ctl);
    if ( ret != SWITCH_OK )
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: write FSA9688_REG_CONTROL2 error!!! ret=%d.\n", __func__, ret);
        return SWITCH_ERR;
    }

    /*初始化完成后，执行一次detach*/
    ret = fsa9688_write_reg(FSA9688_REG_DETACH_CONTROL, 1);
    if (ret != SWITCH_OK)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: write FSA9688_REG_DETACH_CONTROL error!!! ret=%d.\n", __func__, ret);
        return SWITCH_ERR;
    }

    return SWITCH_OK;
}

/******************************************************************************
  Function      chg_get_hvdcp_type
  Description   获取高压充电器类型
  Input         void
  Output        NA
  Return        CHG_HVDCP_5V :5V高压充电器
                CHG_HVDCP_9V :9V高压充电器
                CHG_HVDCP_12V:12V高压充电器
                CHG_HVDCP_INVALID:非高压充电器
  Others        N/A
******************************************************************************/
chg_hvdcp_type_value chg_get_hvdcp_type(void)
{
    chg_chgr_type_t charger_type = CHG_CHGR_INVALID;
    uint8_t max_vol = 0;
    int32_t ret = 0;

    charger_type = fsa9688_get_charger_type();

    if (CHG_HVDCP_CHGR != charger_type)
    {
        return CHG_HVDCP_INVALID;
    }

    ret = fsa9688_get_supported_max_vol(&max_vol);
    if (SWITCH_OK != ret)
    {
        return CHG_HVDCP_INVALID;
    }

    if (HVDCP_OUTPUT_VOL_12V <= max_vol)
    {
        return CHG_HVDCP_12V;
    }
    else if (HVDCP_OUTPUT_VOL_9V <= max_vol)
    {
        return CHG_HVDCP_9V;
    }
    else if (HVDCP_OUTPUT_VOL_5V <= max_vol)
    {
        return CHG_HVDCP_5V;
    }
    else
    {
        return CHG_HVDCP_INVALID;
    }
}

/****************************************************************************
  Function:     chg_set_hvdcp_adpter_vol
  Description:  set fcp adapter output vol
  Input:        NA
  Output:       NA
  Return:        0: success
                -1: fail
***************************************************************************/
boolean chg_set_hvdcp_adpter_vol(chg_hvdcp_type_value vol_set)
{
    uint8_t val = 0;
    uint8_t vol = 0;
    uint32_t i = 0;
    int32_t ret = 0;
    chg_chgr_type_t charger_type = CHG_CHGR_INVALID;

    if(NULL == this_client)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: this_client is null.\n", __func__);
        return FALSE;
    }

    charger_type = fsa9688_get_charger_type();
    if (CHG_HVDCP_CHGR != charger_type)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: the charger is not hvdcp,not supported.\n", __func__);
        return FALSE;
    }

    if ( (CHG_HVDCP_5V == vol_set ) || (CHG_HVDCP_9V == vol_set ) || (CHG_HVDCP_12V == vol_set ) )
    {
        
        vol = fsa9688_convert_to_hvdcp_vol(vol_set);
        vol = vol * HVDCP_VOL_SETP;
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: the vol value err.\n", __func__);
        return FALSE;
    }

    /*使能adapter输出电压设置*/
    ret = fsa9688_adapter_reg_write(SLAVE_REG_OUTPUT_CONTROL, HVDCP_SLAVE_SET_VOUT);
    if (SWITCH_OK != ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: enable adapter_vol_set err.\n", __func__);
        return FALSE;
    }

    /*try 3 times if write fail */
    for (i = 0; i < HVDCP_RETRY_MAX_TIMES; i++)
    {
        fsa9688_adapter_reg_write(SLAVE_REG_VOUT_CONFIG, vol);
        fsa9688_adapter_reg_read(SLAVE_REG_VOUT_CONFIG, &val);
        chg_print_level_message(CHG_MSG_INFO, "%s: vout config reg[0x2c] = %d.\n", __func__, val);
        if (val == vol)
        {
            break;
        }
    }

    /*if try 3 times return fail */
    if (HVDCP_RETRY_MAX_TIMES == i)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s:out vol config err, reg[0x2c] = %d.\n", __func__, val);
        return FALSE;
    }

    ret = fsa9688_adapter_reg_write(SLAVE_REG_OUTPUT_CONTROL, HVDCP_SLAVE_SET_VOUT);
    if (SWITCH_OK == ret)
    {
        chg_print_level_message(CHG_MSG_INFO, "fcp adapter output vol set ok.\n");
        return TRUE;
    }
    else
    {
        chg_print_level_message(CHG_MSG_ERR, "fcp adapter output vol set fail.\n");
        return FALSE;
    }
}

static int fsa9688_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret = 0;
    struct device_node *node = NULL;
    int gpio_value = 0;

    if (NULL == client )
    {
        printk("%s: client or id is NULL!!!\n", __func__);
        goto err_device_node;
    }

    node = client->dev.of_node;
    if ( NULL == node)
    {
        printk("%s: node is NULL!!!\n", __func__);
        goto err_device_node;
    }

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        printk("%s: i2c_check_functionality error!!!\n", __func__);
        ret = (- ERR_NO_DEV);
        goto err_i2c_check_functionality;
    }

    mutex_init(&hvdcp_detect_lock);

    of_property_read_u32(node, "gpio-intb", &gpio);
    if (-1 == gpio)
    {
        printk("%s: gpio init error!!!\n", __func__);
        goto  err_gpio_set;
    }

    client->irq = gpio_to_irq(gpio);

    if (client->irq < 0)
    {
        printk("%s: gpio_to_irq error!!! , client->irq=%d.\n", __func__, client->irq);
        ret = (- ERR_GPIO_TO_IRQ);
        goto err_gpio_set;
    }

    ret = gpio_request(gpio, "fsa9688_int");
    if (ret < 0)
    {
        printk("%s: gpio_request error!!! ret=%d.\n", __func__, ret);
        ret = (- ERR_GPIO_REQUEST);
        goto err_gpio_set;
    }

    ret = gpio_direction_input(gpio);
    if (ret < 0)
    {
        printk("%s: gpio_direction_input error!!! ret=%d.\n", __func__, ret);
        ret = (- ERR_GPIO_DIRECTION_INPUT);
        goto err_gpio_set;
    }

    this_client = client;
    /* interrupt register */
    INIT_WORK(&g_intb_work, fsa9688_intb_work);
    gpio_value = gpio_get_value(gpio);
    /*如果是电平，进行充电器中断寄存器读取*/
    if (0 == gpio)
    {
        schedule_work(&g_intb_work);
    }

    /*上电后设置为下降沿触发*/
    ret = request_irq(client->irq,
               fsa9688_irq_handler,
               IRQF_SHARED | IRQF_NO_SUSPEND | IRQF_TRIGGER_FALLING,
               "fsa9688_int", (void*)(&g_intb_irq_flag));

    if (ret < 0)
    {
        printk("%s: request_irq error!!! ret=%d.\n", __func__, ret);
        ret = (- ERR_REQUEST_THREADED_IRQ);
        goto err_gpio_set;
    }

    ret = fsa9688_reg_init();
    if (ret != SWITCH_OK)
    {
        printk("%s: fsa9688_reg_init error!!! ret=%d.\n", __func__, ret);
        goto err_fsa9688_reg_int;
    }
    
    printk("%s: ------end. ret = %d.\n", __func__, ret);
    return ret;

err_gpio_set:
    gpio_free(gpio);

err_fsa9688_reg_int:
err_device_node:
err_i2c_check_functionality:
    this_client = NULL;

    printk("%s: ------FAIL!!! end. ret = %d.\n", __func__, ret);
    return ret;
}

static int fsa9688_remove(struct i2c_client *client)
{
    free_irq(client->irq, client);
    gpio_free(gpio);
    return 0;
}

static const struct i2c_device_id fsa9688_i2c_id[] =
{
    { "fsa9688", 0 },
    { }
};

static struct i2c_driver fsa9688_driver =
{
    .driver =
    {
        .name = "fairchild_fsa9688",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(switch_fsa9688_ids),
    },
    .probe    = fsa9688_probe,
    .remove   = fsa9688_remove,
    .id_table = fsa9688_i2c_id,
};

static __init int fsa9688_init(void)
{
    int ret = 0;
    chg_print_level_message(CHG_MSG_INFO, "%s: ------entry.\n", __func__);

    ret = i2c_add_driver(&fsa9688_driver);
    if(ret)
    {
        chg_print_level_message(CHG_MSG_ERR, "%s: i2c_add_driver error!!!\n", __func__);
    }
    chg_print_level_message(CHG_MSG_INFO, "%s: ------end.\n", __func__);
    return ret;
}

static __exit void fsa9688_exit(void)
{
    i2c_del_driver(&fsa9688_driver);
}

module_init(fsa9688_init);
module_exit(fsa9688_exit);

MODULE_AUTHOR("mobile@huawei.com>");
MODULE_DESCRIPTION("I2C bus driver for FSA9688 USB Accesory Detection Switch");
MODULE_LICENSE("GPL v2");
