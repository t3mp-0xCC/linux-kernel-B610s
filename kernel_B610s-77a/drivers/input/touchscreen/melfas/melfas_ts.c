/*********************************************************************
 *
 * Melfas MCS6000 Touchscreen Controller Driver
 *
 *********************************************************************/

/*********************************************************************
 * drivers/input/touchscreen/melfas_ts.c
 *
 * Copyright (C) 2010 Melfas, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation,and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *********************************************************************/
 

#include <linux/module.h>
#include <linux/delay.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include "melfas_ts.h"
#include "bsp_sram.h"

#ifdef CONFIG_OF
#include <linux/of_address.h>
#include <linux/of.h>
#endif


#if (FEATURE_ON == MBB_TP_FIRMWARE_UPDATE)
#include "bsp_memmap.h"
#include "ios_ao_drv_macro.h"
#include "ios_pd_drv_macro.h"

#define READ_REGISTER_ULONG(addr) (*(volatile UINT32 *)(addr))
#define WRITE_REGISTER_ULONG(addr,val) (*(volatile UINT32 *)(addr) = (val))
#define INREG32(x)          READ_REGISTER_ULONG(x)
#define OUTREG32(x, y)      WRITE_REGISTER_ULONG((x), (UINT32)(y))
#define SETREG32(x, y)      OUTREG32((x), INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32((x), INREG32(x)&~(y))
#define SETBITVALUE32(addr, mask, value)  OUTREG32((addr), (INREG32(addr)&(~(mask))) | ((value)&(mask)))
#endif
#define MELFAS_MAX_TOUCH      2
/* for TP firmware*/
#if (FEATURE_ON == MBB_TP_FIRMWARE_UPDATE)
#define FW_VERSION_E          0x06 //Check Version
#define FW_VERSION_O          0x05
#else
#define FW_VERSION            0x00 //Check Version
#endif
#define HW_VERSION            0x00 //Check Version



#define TS_MAX_X_COORD        320 //Check resolution
#define TS_MAX_Y_COORD        240 //Check resolution




#define TS_MAX_Z_TOUCH        255

/*weight 应该是压力值 */
#define TS_MAX_W_TOUCH        30


#define TS_READ_EVENT_PACKET_SIZE     0x0F
#define TS_READ_START_ADDR            0x10
#define TS_READ_VERSION_ADDR          0xF0 //Start read H/W, S/W Version
#define TS_READ_REGS_LEN              66
#define TS_SINGAL_POINT_LEN           6

#define TOUCH_TYPE_NONE         0
#define TOUCH_TYPE_SCREEN       1
#define TOUCH_TYPE_KEY          2

/*i2c touch读取 尝试最大次数*/
#define I2C_RETRY_CNT           10

/*why */
#define SET_DOWNLOAD_BY_GPIO    1
#define ESD_DETECTED            0

#define PRESS_KEY               1
#define RELEASE_KEY             0
#define DEBUG_PRINT             1

#define TS_READ_ONCE            1
/*I2C read or write markers*/
#define I2C_MSG_NUM    2
#define I2C_READ    1
#define I2C_WRITE    0


static struct workqueue_struct *melfas_ts_wq = NULL;

static int melfas_ts_debug_mask = 0;
#define TP_DEBUG(args...) do { \
if(melfas_ts_debug_mask) { \
        printk( args); \
    } \
}while(0)

int melfas_ts_debug_set(int mask)
{
    melfas_ts_debug_mask = mask;
    return 0;
}



#define melfas_ldo_en_index (0)
#define melfas_int_en_index (1)
#define melfas_rst_en_index (2)
#define melfas_platform_data_length (3)

#define melfas_scl_index (0)
#define melfas_sda_index (1)
#define melfas_gpio_i2c_length (2)
#define melfas_rev_mess_length (6)
struct melfas_touch_platform_data ts_platform_data = {0} ;

#if SET_DOWNLOAD_BY_GPIO
#include <melfas_download.h>
/*暂时使用E5382的资源，后续根据实际情况更新*/
#if (FEATURE_ON == MBB_TP_FIRMWARE_UPDATE)
//#include "MCH_HE5380(O)_R03_V05_bin.c"
#include "MCH_HE5382(E)_R40_V06_bin.c"
#include "MCH_HE5382(O)_R30_V05_bin.c"
#endif

#endif // SET_DOWNLOAD_BY_GPIO

//该结构体用于保存TP触摸坐标信息
struct muti_touch_info
{
    int action;
    int fingerX;
    int fingerY;
    int width;
    int strength;
};

struct melfas_ts_data
{
    uint16_t addr;
    struct i2c_client *client;
    struct input_dev *input_dev;
    struct work_struct  work;
 
    uint32_t flags;
    int (*power)(int on);
    #ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend early_suspend;
    #endif
    struct melfas_touch_platform_data *platform_data;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];

int mach_num = 7,ver_num = 0;
void get_tp_version()
{
    printk("machine is 0x%x,version = 0x%x \r\n",mach_num,ver_num);
}


#if (FEATURE_ON == MBB_TP_FIRMWARE_UPDATE)
void set_pin_i2c_config()
{

    /*i2c1_scl管脚复用配置*/
    CLR_IOS_I2C1_CTRL2_2;
    SET_IOS_I2C1_CTRL2_1;
    CLR_IOS_GPIO5_2_CTRL1_1;
    /*i2c1_scl管脚上下拉配置*/
    NASET_IOS_PD_IOM_CTRL67;

    /*i2c1_sda管脚复用配置*/
    CLR_IOS_I2C1_CTRL2_2;
    SET_IOS_I2C1_CTRL2_1;
    CLR_IOS_GPIO5_3_CTRL1_1;
    /*i2c1_sda管脚上下拉配置*/
    NASET_IOS_PD_IOM_CTRL68;

}

void set_pin_gpio_config()
{
    /*i2c1 scl管脚复用配置*/
    
    CLR_IOS_I2C1_CTRL2_1;
    SET_IOS_GPIO5_2_CTRL1_1;
    /*i2c1_scl管脚上下拉配置*/
    NASET_IOS_PD_IOM_CTRL67;

    /*i2c1_sda管脚复用配置*/
    
    CLR_IOS_I2C1_CTRL2_1;
    SET_IOS_GPIO5_3_CTRL1_1;
    /*i2c1_sda管脚上下拉配置*/
    NASET_IOS_PD_IOM_CTRL68;

}
static void melfas_write_download_mode_signal(void)
{
    int i;
    uint8_t enter_code[14] = { 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1 };
    int value;
    printk("melfas_write_download_mode_signal : start !\n");
  
    for(i = 0; i < 14; i ++)
    {
        if(enter_code[i])
        {           
            gpio_set_value(ts_platform_data.gpio_int, 1);

        }
        else
        {
            gpio_set_value(ts_platform_data.gpio_int, 0);
        }
        
        gpio_set_value(ts_platform_data.gpio_sck, 1);
        udelay(600);
        gpio_set_value(ts_platform_data.gpio_sck, 0);
        gpio_set_value(ts_platform_data.gpio_int, 0);

        udelay(600);
    }
    gpio_set_value(ts_platform_data.gpio_sck, 1);

    udelay(100);
    gpio_set_value(ts_platform_data.gpio_int, 1);
}

static int melfas_enter_download_mode(struct melfas_ts_data *ts)
{
    uint8_t buf[TS_READ_REGS_LEN] = {0};
    int nRet = MCSDL_RET_ENTER_DOWNLOAD_MODE_FAILED;
    int ret = 0;

    printk("melfas_enter_download_mode 1 \n");

    gpio_set_value(ts_platform_data.gpio_en,0); //tp_vdd
    
    set_pin_gpio_config();      //scl cda->gpio    
    
    /*Touch FW  grade 具体产品具体配置*/
    gpio_request(ts_platform_data.gpio_sda,"TP_SDA");
    gpio_request(ts_platform_data.gpio_sck,"TP_SCL");
    gpio_request(ts_platform_data.gpio_int,"melfas_int");

    gpio_direction_output(ts_platform_data.gpio_sda,0); //SDA
    gpio_direction_output(ts_platform_data.gpio_sck,0); //sCL
    gpio_direction_output(ts_platform_data.gpio_int,0);

    gpio_set_value(ts_platform_data.gpio_reset, 0); //rst

    mdelay(90);

    gpio_set_value(ts_platform_data.gpio_en,1); //tp_vdd
   
    
    gpio_set_value(ts_platform_data.gpio_reset, 1);//rst
    
    
    gpio_set_value(ts_platform_data.gpio_sda, 1);//SDA

    mdelay(25);
    
    melfas_write_download_mode_signal();
    mdelay(2);
 
    set_pin_i2c_config();
    gpio_free(ts_platform_data.gpio_int);
    mdelay(20);
    printk("melfas_enter_download_mode 6\n");

    ts->client->addr = 0x7D;
    ret = i2c_master_recv(ts->client, buf, 1);
    pr_err("%s returns - ret : 0x%x\n", __func__, buf[0]); 

    if(ret != 1 || buf[0] != MCSDL_I2C_SLAVE_READY_STATUS)
    {
        pr_err("%s returns - ret : 0x%x & buf[0] : 0x%x\n", __func__, ret, buf[0]);
        goto MCSDL_ENTER_DOWNLOAD_MODE_FINISH; 
    }
    
    nRet = MCSDL_RET_SUCCESS;

MCSDL_ENTER_DOWNLOAD_MODE_FINISH:
    return nRet;
}

static int melfas_i2c_prepare_erase_flash(struct melfas_ts_data *ts)
{
    int nRet = MCSDL_RET_PREPARE_ERASE_FLASH_FAILED;
    int i = 0;
    int bRet = 0;

    uint8_t i2c_buffer[4] = { MCSDL_ISP_CMD_ERASE_TIMING,
                          MCSDL_ISP_ERASE_TIMING_VALUE_0,
                          MCSDL_ISP_ERASE_TIMING_VALUE_1,
                          MCSDL_ISP_ERASE_TIMING_VALUE_2   };
    uint8_t ucTemp;

    for(i = 0; i < 4; i ++)
    {
        bRet = i2c_master_send(ts->client, &i2c_buffer[i], 1);
        if(bRet != 1)
        {
            goto MCSDL_I2C_PREPARE_ERASE_FLASH_FINISH;
        }
        udelay(15);
    }
    udelay(500);
    bRet = i2c_master_recv(ts->client, &ucTemp, 1);
    if(bRet == 1 && ucTemp == MCSDL_ISP_ACK_PREPARE_ERASE_DONE)
    {
        nRet = MCSDL_RET_SUCCESS;
    }

MCSDL_I2C_PREPARE_ERASE_FLASH_FINISH :
    return nRet;
}

static int melfas_i2c_erase_flash(struct melfas_ts_data *ts)
{
    int nRet = MCSDL_RET_ERASE_FLASH_FAILED;
    uint8_t i = 0;
    int bRet = 0;
    uint8_t i2c_buffer[1] = {MCSDL_ISP_CMD_ERASE};
    uint8_t ucTemp;

    for(i = 0; i < 1; i ++)
    {
        bRet = i2c_master_send(ts->client, &i2c_buffer[i], 1);
        if(bRet != 1)
        {
            goto MCSDL_I2C_ERASE_FLASH_FINISH;
        }
        udelay(15);
    }
    mdelay(45);
    bRet = i2c_master_recv(ts->client, &ucTemp, 1);
    if(bRet == 1 && ucTemp == MCSDL_ISP_ACK_ERASE_DONE)
    {
        nRet = MCSDL_RET_SUCCESS;
    }

 MCSDL_I2C_ERASE_FLASH_FINISH :
    return nRet;
}

static int melfas_i2c_read_flash(struct melfas_ts_data *ts, uint8_t *pBuffer, uint16_t nAddr_start, uint8_t cLength)
{
    int nRet = MCSDL_RET_READ_FLASH_FAILED;
    int i = 0;
    int bRet = 0;
    uint8_t cmd[4];
    uint8_t ucTemp;

    cmd[0] = MCSDL_ISP_CMD_READ_FLASH;
    cmd[1] = (uint8_t)((nAddr_start >> 8) & 0xFF);
    cmd[2] = (uint8_t)((nAddr_start) & 0xFF);
    cmd[3] = cLength;

    for(i = 0; i < 4; i ++)
    {
        bRet = i2c_master_send(ts->client, &cmd[i], 1);
        udelay(15);
        if(bRet != 1)
        {
            goto MCSDL_I2C_READ_FLASH_FINISH;
        }
    }

    bRet = i2c_master_recv(ts->client, &ucTemp, 1);
    if(bRet != 1 || ucTemp != MCSDL_MDS_ACK_READ_FLASH)
    {
        goto MCSDL_I2C_READ_FLASH_FINISH;
    }

    for(i = 0; i < (int)cmd[3]; i ++)
    {
        udelay(100);
        bRet = i2c_master_recv(ts->client, pBuffer++, 1);
        if(bRet != 1 && i != (int)(cmd[3] - 1))
        {
            goto MCSDL_I2C_READ_FLASH_FINISH;
        }
    }
    nRet = MCSDL_RET_SUCCESS;

MCSDL_I2C_READ_FLASH_FINISH:
    return nRet;
}

static int melfas_i2c_prepare_program(struct melfas_ts_data *ts)
{
    int nRet = MCSDL_RET_PREPARE_PROGRAM_FAILED;

    int i = 0;
    int bRet = 0;
    uint8_t i2c_buffer[4] = { MCSDL_ISP_CMD_PROGRAM_TIMING,
                            MCSDL_ISP_PROGRAM_TIMING_VALUE_0,
                            MCSDL_ISP_PROGRAM_TIMING_VALUE_1,
                            MCSDL_ISP_PROGRAM_TIMING_VALUE_2};
    uint8_t ucTemp = 0;
    for(i = 0; i < 4; i ++)
    {
        bRet = i2c_master_send(ts->client, &i2c_buffer[i], 1);
        if(bRet != 1)
        {
            goto MCSDL_I2C_PREPARE_PROGRAM_FINISH;
        }
        udelay(15);
    }
    udelay(500);

    bRet = i2c_master_recv(ts->client, &ucTemp, 1);
    if(bRet != 1 || ucTemp != MCSDL_I2C_ACK_PREPARE_PROGRAM)
    {
        goto MCSDL_I2C_PREPARE_PROGRAM_FINISH;
    }
    udelay(100);
    nRet = MCSDL_RET_SUCCESS;
MCSDL_I2C_PREPARE_PROGRAM_FINISH:
    return nRet;
}

static int melfas_i2c_program_flash(struct melfas_ts_data *ts, uint8_t *pData, uint16_t nAddr_start, uint8_t cLength)
{
    int nRet = MCSDL_RET_PROGRAM_FLASH_FAILED;
    int i = 0;
    int bRet = 0;
    uint8_t cData;
    uint8_t cmd[4];

    cmd[0] = MCSDL_ISP_CMD_PROGRAM_FLASH;
    cmd[1] = (uint8_t)((nAddr_start >> 8) & 0xFF);
    cmd[2] = (uint8_t)((nAddr_start) & 0xFF);
    cmd[3] = cLength;
  
    for(i = 0; i < 4; i ++)
    {
        bRet = i2c_master_send(ts->client, &cmd[i], 1);
        udelay(15);
        if(bRet != 1)
        {
            goto MCSDL_I2C_PROGRAM_FLASH_FINISH;
        }
    }

    bRet = i2c_master_recv(ts->client, &cData, 1);
    if(bRet != 1 || cData != MCSDL_MDS_ACK_PROGRAM_FLASH)
    {
        goto  MCSDL_I2C_PROGRAM_FLASH_FINISH;
    }

    udelay(150);
    for(i = 0; i < (int)cmd[3]; i += 2)
    {
        bRet = i2c_master_send(ts->client, &pData[i + 1], 1);
        if(bRet != 1)
        {
            goto MCSDL_I2C_PROGRAM_FLASH_FINISH;
        }
        udelay(100);
        bRet = i2c_master_send(ts->client, &pData[i], 1);
        if(bRet != 1)
        {
            goto MCSDL_I2C_PROGRAM_FLASH_FINISH;
        }
        udelay(150);
    }
    nRet = MCSDL_RET_SUCCESS;

MCSDL_I2C_PROGRAM_FLASH_FINISH:
    return nRet;
}

static int melfas_download(struct melfas_ts_data *ts, const uint8_t *pData, const uint16_t nLength)
{
    int i = 0;
    int nRet = 0;
    uint8_t cLength = 0;
    uint16_t nStart_address = 0;
    uint8_t buffer[MELFAS_TRANSFER_LENGTH];
    uint8_t *pOriginal_data;

    pr_err("Starting download...\n");

    nRet = melfas_enter_download_mode(ts);
    if(nRet != MCSDL_RET_SUCCESS)
    {
        goto MCSDL_DOWNLOAD_FINISH;
    }
    mdelay(1);

    pr_err("Checking module revision...\n");
    
    pOriginal_data = (uint8_t *)pData;
    nRet = melfas_i2c_read_flash(ts, buffer, MCSDL_ADDR_MODULE_REVISION, 4);
    if(nRet != MCSDL_RET_SUCCESS)
    {
        goto MCSDL_DOWNLOAD_FINISH;
    }
    if((pOriginal_data[MCSDL_ADDR_MODULE_REVISION + 1] != buffer[1])
        || (pOriginal_data[MCSDL_ADDR_MODULE_REVISION + 2] != buffer[2]))
    {
        nRet = MCSDL_RET_WRONG_MODULE_REVISION;
        goto MCSDL_DOWNLOAD_FINISH;
    }
    mdelay(1);

    pr_err("Erasing...\n");

    nRet = melfas_i2c_prepare_erase_flash(ts);
    if(nRet != MCSDL_RET_SUCCESS)
    {
        goto MCSDL_DOWNLOAD_FINISH;
    }
    mdelay(1);
    nRet = melfas_i2c_erase_flash(ts);
    if(nRet != MCSDL_RET_SUCCESS)
    {
        goto MCSDL_DOWNLOAD_FINISH;
    }
    mdelay(1); 

    pr_err("Verify Erasing...\n");

    nRet = melfas_i2c_read_flash(ts, buffer, 0x00, 16);
    if(nRet != MCSDL_RET_SUCCESS)
    {
        goto MCSDL_DOWNLOAD_FINISH;
    }
    for(i = 0; i < 16; i ++)
    {
        if(buffer[i] != 0xFF)
        {
            nRet = MCSDL_RET_ERASE_VERIFY_FAILED;
            goto MCSDL_DOWNLOAD_FINISH;
        }
    }
    mdelay(1);

    pr_err("Preoaring Program...\n");

    nRet = melfas_i2c_prepare_program(ts);
    if(nRet != MCSDL_RET_SUCCESS)
    {
        goto MCSDL_DOWNLOAD_FINISH;
    }
    mdelay(1);

    pr_err("Program flash...\n");

    pOriginal_data = (uint8_t *)pData;

    nStart_address = 0;
    cLength = MELFAS_TRANSFER_LENGTH;
    for(nStart_address = 0; nStart_address < nLength; nStart_address += cLength)
    {
        //pr_err("#");
        if((nLength - nStart_address) < MELFAS_TRANSFER_LENGTH)
        {
            cLength = (uint8_t)(nLength - nStart_address);
            cLength += (cLength % 2);
        }
        nRet = melfas_i2c_program_flash(ts, pOriginal_data, nStart_address, cLength);
        if(nRet != MCSDL_RET_SUCCESS)
        {
            pr_err("\nProgram flash failed position : 0x%x / nRet: 0x%x", nStart_address, nRet);
            goto MCSDL_DOWNLOAD_FINISH;
        }
        pOriginal_data += cLength;
        udelay(500);
    }
    
    pr_err("\n");
    pr_err("Verify flash...");

    pOriginal_data = (uint8_t *)pData;

    nStart_address = 0;

    cLength = MELFAS_TRANSFER_LENGTH;

    for(nStart_address = 0; nStart_address < nLength; nStart_address += cLength)
    {
    
        if((nLength - nStart_address) < MELFAS_TRANSFER_LENGTH)
        {
            cLength = (uint8_t)(nLength - nStart_address);
            cLength += (cLength % 2);
        }
        nRet = melfas_i2c_read_flash(ts, buffer, nStart_address, cLength);
        for(i = 0; i < (int)cLength; i ++)
        {
            if(buffer[i] != pOriginal_data[i])
            {
                pr_err("\n [Error] Address : 0x%04x : 0x%02x - 0x%02x\n", nStart_address, pOriginal_data[i], buffer[i]);
                nRet = MCSDL_RET_PROGRAM_VERIFY_FAILED;
                goto MCSDL_DOWNLOAD_FINISH;
            }
        }
        pOriginal_data += cLength;
        udelay(500);
    }
    
    pr_err("\n");
    nRet = MCSDL_RET_SUCCESS;

MCSDL_DOWNLOAD_FINISH:
    mdelay(1);

    buffer[0] = MCSDL_ISP_CMD_RESET;
    i2c_master_send(ts->client, buffer, 1);
    ts->client->addr = 0x23;
    gpio_free(ts_platform_data.gpio_int);

    mdelay(180);
    return nRet;
}
static void melfas_print_result(int nRet)
{
    if( nRet == MCSDL_RET_SUCCESS ){

    pr_err(" MELFAS Firmware downloading SUCCESS.\n");

    }
    else
    {
    pr_err(" MELFAS Firmware downloading FAILED  :  ");

     switch( nRet ){

            case MCSDL_RET_SUCCESS                      :   pr_err("MCSDL_RET_SUCCESS\n" );                 	break;
            case MCSDL_RET_ENTER_DOWNLOAD_MODE_FAILED   :   pr_err("MCSDL_RET_ENTER_ISP_MODE_FAILED\n" );      break;
            case MCSDL_RET_ERASE_FLASH_FAILED           :   pr_err("MCSDL_RET_ERASE_FLASH_FAILED\n" );         break;
            case MCSDL_RET_READ_FLASH_FAILED            :   pr_err("MCSDL_RET_READ_FLASH_FAILED\n" );         	break;
            case MCSDL_RET_READ_EEPROM_FAILED           :   pr_err("MCSDL_RET_READ_EEPROM_FAILED\n" );         break;
            case MCSDL_RET_READ_INFORMAION_FAILED       :   pr_err("MCSDL_RET_READ_INFORMAION_FAILED\n" );     break;
            case MCSDL_RET_PROGRAM_FLASH_FAILED         :   pr_err("MCSDL_RET_PROGRAM_FLASH_FAILED\n" );      	break;
            case MCSDL_RET_PROGRAM_EEPROM_FAILED        :   pr_err("MCSDL_RET_PROGRAM_EEPROM_FAILED\n" );      break;
            case MCSDL_RET_PREPARE_PROGRAM_FAILED       :   pr_err("MCSDL_RET_PROGRAM_INFORMAION_FAILED\n" );  break;
            case MCSDL_RET_PROGRAM_VERIFY_FAILED        :   pr_err("MCSDL_RET_PROGRAM_VERIFY_FAILED\n" );      break;

            case MCSDL_RET_WRONG_MODE_ERROR             :   pr_err("MCSDL_RET_WRONG_MODE_ERROR\n" );         	 break;
            case MCSDL_RET_WRONG_SLAVE_SELECTION_ERROR  :   pr_err("MCSDL_RET_WRONG_SLAVE_SELECTION_ERROR\n" ); break;
            case MCSDL_RET_COMMUNICATION_FAILED         :   pr_err("MCSDL_RET_COMMUNICATION_FAILED\n" );      	 break;
            case MCSDL_RET_READING_HEXFILE_FAILED       :   pr_err("MCSDL_RET_READING_HEXFILE_FAILED\n" );      break;
            case MCSDL_RET_WRONG_PARAMETER              :   pr_err("MCSDL_RET_WRONG_PARAMETER\n" );      		 break;
            case MCSDL_RET_FILE_ACCESS_FAILED           :   pr_err("MCSDL_RET_FILE_ACCESS_FAILED\n" );      	 break;
            case MCSDL_RET_MELLOC_FAILED                :   pr_err("MCSDL_RET_MELLOC_FAILED\n" );      		 break;
            case MCSDL_RET_WRONG_MODULE_REVISION        :   pr_err("MCSDL_RET_WRONG_MODULE_REVISION\n" );       break;

            default                                     :pr_err("UNKNOWN ERROR. [0x%02X].\n", nRet );      	 break;
        }

        pr_err("\n");
    }

}
#endif

static int melfas_init_panel(struct melfas_ts_data *ts)
{
    int buf = 0x00;
    int ret = 0;

    ret = i2c_master_send(ts->client, &buf, 1);
    return ret;
}

int melfas_data_receive(const struct i2c_client *client, const char *buf, int count, int offset )
{
    int ret = 0;
    struct i2c_msg msg[I2C_MSG_NUM] = {0};
    uint8_t buf0[I2C_MSG_NUM] = {0};

    buf0[0] = offset;    /*the slave register offset*/
    /*config the sending message*/
    msg[0].addr = client->addr;
    msg[0].flags = I2C_WRITE;    /*operation type */
    msg[0].len = 1;             /*the first msg is only include register offset*/
    msg[0].buf = buf0;          /*register offset*/
    msg[1].addr = client->addr;
    msg[1].flags = I2C_READ;    /*operation type */
    msg[1].len = count;         /*read byte number*/
    msg[1].buf = (char *)buf;   /*read data buf */

    /*read data*/
   /*ret means the message num send*/
    ret = i2c_transfer(client->adapter, msg, I2C_MSG_NUM);
    if (ret < 0) 
    {
        printk(KERN_ERR "TP:i2c_transfer failed\n");
        return ret;
    }   
    return ret;
}

static void melfas_ts_work_func(struct work_struct *work)
{
    struct melfas_ts_data *ts = container_of(work, struct melfas_ts_data, work);
    int ret = 0, i;
    uint8_t buf[TS_SINGAL_POINT_LEN + 1] = {0};
    uint8_t read_num = 0;
    uint8_t touchAction = 0, touchType = 0, fingerID = 0;

    

    int exchange_fingerX = 0, exchange_fingerY = 0;

#if DEBUG_PRINT
    TP_DEBUG(KERN_ERR "melfas_ts_work_func\n");
#endif 



    /******************************************************
    Simple send transaction:
    S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P
    Simple recv transaction:
    S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P
    *******************************************************/
#if 0
    buf[0] = TS_READ_EVENT_PACKET_SIZE;
    ret = i2c_master_send(ts->client, buf, 1);
#endif

#if DEBUG_PRINT
    TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);
#endif

    if (ret >= 0)
    {
#if 0
#if TS_READ_ONCE
        ret = i2c_master_recv(ts->client, buf, 13);
#else
        ret = i2c_master_recv(ts->client, buf, 1);
#endif
#endif
        /*only process one point data，1+6 byte */
        ret = melfas_data_receive(ts->client, buf, TS_SINGAL_POINT_LEN + 1, TS_READ_EVENT_PACKET_SIZE);
#if DEBUG_PRINT
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv packet_size [%d]\n", buf[0]);

#if TS_READ_ONCE
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv InputEvent_information [%d]\n", buf[1]); 
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv xy_coordi [%d]\n", buf[2]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv x_coordi [%d]\n", buf[3]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv y_coordi [%d]\n", buf[4]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv width [%d]\n", buf[5]);
        TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv strength [%d]\n", buf[6]);

#endif
#endif
        if (ret >= 0)
        {
            read_num = buf[0];

#if ESD_DETECTED    
            if((read_num & 0x80))
            {
#if DEBUG_PRINT
                printk(KERN_ERR "***** ESD Detected status ****\n", ret);
#endif            
                //Need CE or VDD Control for TSP reset
                enable_irq(ts->client->irq);
                return;
            }
#endif //ESD_DETECTED           
        }
        /*only process one point data，six byte */
        if(read_num > TS_SINGAL_POINT_LEN)
        {
            read_num = TS_SINGAL_POINT_LEN;
        }
    }
    else // ret < 0
    {
#if DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
#endif

        enable_irq(ts->client->irq);

        return ;
    }
  
#if !(TS_READ_ONCE)
    buf[0] = TS_READ_START_ADDR;
    ret = i2c_master_send(ts->client, buf, 1);
#if DEBUG_PRINT
    TP_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);
#endif
    if (ret >= 0)
    {
        ret = i2c_master_recv(ts->client, buf, read_num);
#if DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]\n", ret);
#endif
        if (ret >= 0)
            break; // i2c success
    }

    if(ret < 0) 
    {
        printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
       enable_irq(ts->client->irq);
        return ;
    }
    else // MIP (Melfas Interface Protocol)
#endif
    {
#if TS_READ_ONCE
        for(i = 1; i < read_num + 1; i = i + 6)
#else
        for(i = 0; i < read_num; i = i + 6)
#endif
        {

            touchAction = ((buf[i] & 0x80) == 0x80);
           
            touchType = (buf[i] & 0x60) >> 5;
            fingerID = (buf[i] & 0x0F) - 1;

#if DEBUG_PRINT
            TP_DEBUG(KERN_ERR "melfas_ts_work_func: touchAction : %d, touchType: %d, fingerID: %d\n", touchAction, touchType, fingerID);
#endif

            if(touchType == TOUCH_TYPE_NONE)
            {
            }
            else if(touchType == TOUCH_TYPE_SCREEN)
            {
                input_report_key(ts->input_dev, BTN_TOUCH, touchAction ? PRESS_KEY : RELEASE_KEY);
                g_Mtouch_info[fingerID].action = touchAction;
                g_Mtouch_info[fingerID].fingerX = (buf[i + 1] & 0x0F) << 8 | buf[i + 2];
                g_Mtouch_info[fingerID].fingerY = (buf[i + 1] & 0xF0) << 4 | buf[i + 3];
                g_Mtouch_info[fingerID].width = buf[i + 4];
                g_Mtouch_info[fingerID].strength = buf[i + 5];

                exchange_fingerX = g_Mtouch_info[fingerID].fingerX;
                exchange_fingerY = g_Mtouch_info[fingerID].fingerY;
                /*按照硬件设计 进行旋转*/
                g_Mtouch_info[fingerID].fingerX = TS_MAX_X_COORD - exchange_fingerY;
                g_Mtouch_info[fingerID].fingerY = exchange_fingerX;
                input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, fingerID);
                input_report_abs(ts->input_dev, ABS_MT_POSITION_X, g_Mtouch_info[fingerID].fingerX);
                input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, g_Mtouch_info[fingerID].fingerY);
                input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR,  g_Mtouch_info[fingerID].strength);
                input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR,g_Mtouch_info[fingerID].width);
                /*保证每个点的准确性和完整性*/
                input_mt_sync(ts->input_dev);
                input_sync(ts->input_dev);
#if DEBUG_PRINT
                TP_DEBUG(KERN_ERR "melfas_ts_work_func: Touch ID: %d, x: %d, y: %d, z: %d w: %d\n",
                        i, g_Mtouch_info[fingerID].fingerX, g_Mtouch_info[fingerID].fingerY, g_Mtouch_info[fingerID].strength, g_Mtouch_info[fingerID].width);
#endif
            }
            else if(touchType == TOUCH_TYPE_KEY)
            {
                if (fingerID == 0x1)
                    input_report_key(ts->input_dev, KEY_MENU, touchAction ? PRESS_KEY : RELEASE_KEY);
                if (fingerID == 0x2)
                    input_report_key(ts->input_dev, KEY_HOME, touchAction ? PRESS_KEY : RELEASE_KEY);
                if (fingerID == 0x3)
                    input_report_key(ts->input_dev, KEY_BACK, touchAction ? PRESS_KEY : RELEASE_KEY);
                if (fingerID == 0x4)
                    input_report_key(ts->input_dev, KEY_SEARCH, touchAction ? PRESS_KEY : RELEASE_KEY);


                input_sync(ts->input_dev);
#if DEBUG_PRINT
                TP_DEBUG(KERN_ERR "melfas_ts_work_func: keyID : %d, keyState: %d\n", fingerID, touchAction);
#endif
            }
        }
    }


     enable_irq(ts->client->irq);

}

static irqreturn_t melfas_ts_irq_handler(int irq, void *handle)
{
    struct melfas_ts_data *ts = (struct melfas_ts_data *)handle;
    unsigned int ucdata = 0;
#if DEBUG_PRINT
    TP_DEBUG(KERN_ERR "melfas_ts_irq_handler\n");
#endif

  
    disable_irq_nosync(ts->client->irq);

    ucdata = gpio_int_state_get((unsigned int)(ts->platform_data->gpio_int));
    if (0 == ucdata)
    {
        printk(KERN_ERR "%s not gpio%d interrupt.\n", 
            __func__,ts->platform_data->gpio_int);
        return IRQ_NONE;/*lint !e82 !e110 !e533*/
    }


    //schedule_work(&ts->work);
  queue_work(melfas_ts_wq, &ts->work);/*lint !e534*/


    return IRQ_HANDLED;
}


static int __init melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct melfas_ts_data *ts = NULL;

    int ret = 0, i;


    uint8_t buf[melfas_rev_mess_length] = {0};

    u32 reg_addr[melfas_platform_data_length] = {0};
    u32 gpio_i2c[melfas_gpio_i2c_length] = {0};
    
    /*这个之前获取一下*/

    if (NULL == client )    
    {   
        printk("%s: client or id is NULL!!!\n", __func__); 
        return  -ENODEV;   
    }
    struct device_node *node = client->dev.of_node;

    if(NULL == node )
    {

        printk(KERN_ERR, "[%s:%d] device tree information missing\n", __func__,__LINE__);
        return - ENODEV;
    }



    ret = of_property_read_u32_array(node, "gpios", reg_addr, melfas_platform_data_length); 
    if(ret)
    {
        printk(KERN_ERR, "[%s:%d] device tree information value get fail\n", __func__,__LINE__);
        return - ENODEV;
    }
    ts_platform_data.gpio_en    = reg_addr[melfas_ldo_en_index];
    ts_platform_data.gpio_int   = reg_addr[melfas_int_en_index];
    ts_platform_data.gpio_reset = reg_addr[melfas_rst_en_index];

    ret = of_property_read_u32_array(node,"i2c_gpio",gpio_i2c,melfas_gpio_i2c_length);
    if(ret)
    {
        printk(KERN_ERR, "[%s:%d] device tree information value i2c_gpio_num get fail\n", __func__,__LINE__);
        return - ENODEV;
    }
    ts_platform_data.gpio_sck  = gpio_i2c[melfas_scl_index];
    ts_platform_data.gpio_sda  = gpio_i2c[melfas_sda_index];
    
  
    struct melfas_touch_platform_data *ts_platformdata = &ts_platform_data;


#if DEBUG_PRINT
    printk(KERN_ERR "melfas_ts_probe now ...\n");
    
#endif

    
    if(NULL == ts_platformdata)
    {
        printk(KERN_ERR "Could not find platform data!!\n");
        return ( - EINVAL);
    }

    //申请并配置LDO_EN使用的GPIO

    ret = gpio_request((unsigned int)(ts_platformdata->gpio_en), "melfas_en");
    if (ret)
    {
        printk(KERN_ERR "%s: Failed to get LDO_EN gpio %d. Code: %d.",
            __func__, ts_platformdata->gpio_en, ret);
        return ret;
    }

    gpio_direction_output((unsigned int)(ts_platformdata->gpio_en),1);/*lint !e534*/

    
    msleep(MELFAS_POWER_UP_TIME);//wait the touch IC power up

    //申请并配置RESET使用的GPIO
    ret = gpio_request((unsigned int)(ts_platformdata->gpio_reset), "melfas_reset");
    if (ret)
    {
        printk(KERN_ERR "%s: Failed to get RESET gpio %d. Code: %d.",
            __func__, ts_platformdata->gpio_reset, ret);
        return ret;
    }
    
    gpio_direction_output((unsigned int)(ts_platformdata->gpio_reset),1);/*lint !e534*/
   
    
    msleep(MELFAS_TP_UNRESET_TIME);//wait the touch IC power up
 

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        printk(KERN_ERR "melfas_ts_probe: need I2C_FUNC_I2C\n");
        ret = -ENODEV;
        goto err_check_functionality_failed;
    }


    ts = devm_kzalloc(&client->dev, sizeof(*ts), GFP_KERNEL);
    if (!ts) {
        printk(KERN_ERR "melfas_ts_probe: failed to create a state of melfas-ts\n");
        ret = -ENOMEM;
        goto err_input_dev_alloc_failed;
    }


    
    ts->platform_data = (struct melfas_touch_platform_data *)&ts_platform_data;
    
    melfas_ts_wq = create_singlethread_workqueue("melfas_ts_wq");
    if (NULL == melfas_ts_wq)
    {
        printk(KERN_ERR "Could not create work queue melfas_ts_wq: no memory!!\n");
        ret = - ENOMEM;
        goto  error_wq_creat_failed; 
    }

    INIT_WORK(&ts->work, melfas_ts_work_func);

    ts->client = client;
    i2c_set_clientdata(client, ts);

    ret = i2c_master_send(ts->client, &buf, 1);

#if DEBUG_PRINT
    printk(KERN_ERR "melfas_ts_probe: i2c_master_send() [%d], Add[%d]\n", ret, ts->client->addr);
#endif

#if SET_DOWNLOAD_BY_GPIO
    buf[0] = TS_READ_VERSION_ADDR;
    printk(KERN_INFO "melfas_ts_probe: start to read version buf0 = 0x%x\n",buf[0]);
    for (i = 0; i < I2C_RETRY_CNT; i++)
    {
        ret = i2c_master_send(ts->client, buf, 1);
        if (ret >= 0)
        {
            ret = i2c_master_recv(ts->client, buf, 6);//dong.hu

            if (ret >= 0)
            {
                break; // i2c success
            }
        }
    }

    if(i == I2C_RETRY_CNT) //VERSION READ Fail
    {
        //ret = mcsdl_download_binary_file();
        printk(KERN_INFO "melfas_ts_probe: VERSION READ FAIL ,buf5= 0x%x\n",buf[5]);
        mach_num = buf[5];
#if (FEATURE_ON == MBB_TP_FIRMWARE_UPDATE)
/*TP version 获取失败时，不升级FW*/
#else
        mcsdl_download_binary_data();
#endif
    }
    else
    {
        printk(KERN_INFO "melfas_ts_probe: buf0= 0x%x ,buf3=0x%x,buf5=0x%x \n",buf[0],buf[3],buf[5]);
        mach_num = buf[5];
        ver_num = buf[3];
#if (FEATURE_ON == MBB_TP_FIRMWARE_UPDATE)
        if (buf[0] == HW_VERSION && buf[5] == 0x06)
        {
            if(buf[3] < FW_VERSION_E)
            {
                 printk( KERN_ERR "melfas_ts_probe: 3: MELFAS_E_binary \n");
                 ret = melfas_download(ts, (const uint8_t *)MELFAS_E_binary, (const uint16_t)MELFAS_E_binary_nLength);
                 melfas_print_result(ret);
            }
        }
        else if (buf[0] == HW_VERSION && buf[5] == 0x00)
        {
            if(buf[3] < FW_VERSION_O)
            { 
                printk(KERN_ERR "melfas_ts_probe:4: MELFAS_O_binary   \n");
                ret = melfas_download(ts, (const uint8_t *)MELFAS_O_binary, (const uint16_t)MELFAS_O_binary_nLength);
                melfas_print_result(ret);
            }
        }
         /*这个地方代码写的有点问题，没有结束分支*/
        else
        {
              printk(KERN_ERR "not support this touchscreen chip manufcature\n");

        }
#else
        if (buf[0] == HW_VERSION && buf[1] < FW_VERSION)
        {
            //ret = mcsdl_download_binary_file();
            mcsdl_download_binary_data();
        }
#endif
 
    }
#endif // SET_DOWNLOAD_BY_GPIO

    ts->input_dev = input_allocate_device();
    if (!ts->input_dev)
    {
        printk(KERN_ERR "melfas_ts_probe: Not enough memory\n");
        ret = -ENOMEM;
        goto err_input_dev_alloc_failed;
    }

    ts->input_dev->name = "melfas-ts" ;
    ts->input_dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY) | BIT_MASK(EV_SYN);
    ts->input_dev->keybit[BIT_WORD(KEY_MENU)] |= BIT_MASK(KEY_MENU);
    ts->input_dev->keybit[BIT_WORD(KEY_HOME)] |= BIT_MASK(KEY_HOME);
    ts->input_dev->keybit[BIT_WORD(KEY_BACK)] |= BIT_MASK(KEY_BACK);
    ts->input_dev->keybit[BIT_WORD(KEY_SEARCH)] |= BIT_MASK(KEY_SEARCH);
    __set_bit(BTN_TOUCH, ts->input_dev->keybit);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, TS_MAX_X_COORD, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, TS_MAX_Y_COORD, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, TS_MAX_Z_TOUCH, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, MELFAS_MAX_TOUCH - 1, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, TS_MAX_W_TOUCH, 0, 0);

    ret = input_register_device(ts->input_dev);
    if (ret)
    {
        printk(KERN_ERR "melfas_ts_probe: Failed to register device\n");
        ret = -ENOMEM;
        goto err_input_register_device_failed;
    }


   
    ret = gpio_request((unsigned int)(ts->platform_data->gpio_int), "melfas_int");
    if (ret) 
    {
        printk(KERN_ERR "%s: Failed to get int gpio %d. Code: %d.",
            __func__, ts->platform_data->gpio_int, ret);
        return ret;
    }
    ts->client->irq = gpio_to_irq(ts->platform_data->gpio_int);
    
   
    
    ret = gpio_direction_input((unsigned int)(ts->platform_data->gpio_int));/*lint !e534*/ 
    if(ret)
    {
         printk(KERN_ERR "%s: Failed to set gpio_direction_input.",
            __func__,ret);
    }
    if (ts->client->irq)
    {
#if DEBUG_PRINT
        printk(KERN_ERR "melfas_ts_probe: trying to request irq: %s-%d\n", ts->client->name, client->irq);
#endif
        ret = request_irq(client->irq, melfas_ts_irq_handler, IRQF_NO_SUSPEND | IRQF_SHARED |IRQF_TRIGGER_FALLING/*IRQF_TRIGGER_LOW*/, ts->client->name, ts);
        if (ret > 0)
        {
            printk(KERN_ERR "melfas_ts_probe: Can't allocate irq %d, ret %d\n", client->irq, ret);
            ret = -EBUSY;
            goto err_request_irq;
        }
    }
#if DEBUG_PRINT
    printk(KERN_ERR "melfas_ts_probe: enable irq and the irq is %d\n", client->irq);
#endif
    //schedule_work(&ts->work);
    queue_work(melfas_ts_wq, &ts->work);/*lint !e534*/

    for (i = 0; i < MELFAS_MAX_TOUCH; i++)  /* _SUPPORT_MULTITOUCH_ */
    {   
        g_Mtouch_info[i].strength = -1;
    }

#if DEBUG_PRINT
    printk(KERN_ERR "melfas_ts_probe: succeed to register input device\n");
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
    ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 1;
    ts->early_suspend.suspend = melfas_ts_early_suspend;
    ts->early_suspend.resume = melfas_ts_late_resume;
    register_early_suspend(&ts->early_suspend);
#endif

#if DEBUG_PRINT
    printk(KERN_INFO "melfas_ts_probe: Start touchscreen. name: %s, irq: %d\n", ts->client->name, ts->client->irq);
#endif
    return 0;

err_request_irq:
    printk(KERN_ERR "melfas-ts: err_request_irq failed\n");
    free_irq(client->irq, ts);
err_input_register_device_failed:
    printk(KERN_ERR "melfas-ts: err_input_register_device failed\n");
    input_free_device(ts->input_dev);
err_input_dev_alloc_failed:
    printk(KERN_ERR "melfas-ts: err_input_dev_alloc failed\n");
err_alloc_data_failed:
    printk(KERN_ERR "melfas-ts: err_alloc_data failed\n");
error_wq_creat_failed:
    printk(KERN_ERR "melfas-ts: error_wq_creat failed\n");
err_detect_failed:
    printk(KERN_ERR "melfas-ts: err_detect failed\n");
    kfree(ts);
err_check_functionality_failed:
    printk(KERN_ERR "melfas-ts: err_check_functionality failed\n");

    return ret;
}

static int __exit melfas_ts_remove(struct i2c_client *client)
{
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    if(NULL == ts)
    {
        return -ENODEV;
    }
    #ifdef CONFIG_HAS_EARLYSUSPEND    
    unregister_early_suspend(&ts->early_suspend);
    #endif
    free_irq(client->irq, ts);
    input_unregister_device(ts->input_dev);
    kfree(ts);
    return 0;
}

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
    int ret,i;  
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    for (i = 0; i < MELFAS_MAX_TOUCH ; i++)
    {
        g_Mtouch_info[i].strength = -1;
        g_Mtouch_info[i].fingerX = 0;
        g_Mtouch_info[i].fingerY = 0;
        g_Mtouch_info[i].width = 0;
    }
    
    if(NULL == ts)
    {
        return -ENODEV;
    }

    disable_irq(client->irq);
 
    
    ret = cancel_work_sync(&ts->work);
    if (ret) /* if work was pending disable-count is now 2 */
    {
        
        enable_irq(client->irq);
      
    }

   
    gpio_set_value((unsigned int)(ts->platform_data->gpio_reset), 0);
    
    printk(KERN_ERR "melfas_ts_suspend: success!!\n");
   
    return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
    struct melfas_ts_data *ts = i2c_get_clientdata(client);

    
    if(NULL == ts)
    {
        return -ENODEV;
    }
    gpio_set_value((unsigned int)(ts->platform_data->gpio_reset), 1);
    msleep(MELFAS_TP_UNRESET_TIME);
    
    melfas_init_panel(ts);
    cancel_work_sync(&ts->work);


    queue_work(melfas_ts_wq, &ts->work);/*lint !e534*/
    enable_irq(client->irq);

    printk(KERN_ERR "melfas_ts_resume: success!!\n");
    return 0;
}


#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
    struct melfas_ts_data *ts;
    int ret = 0;
    ts = container_of(h, struct melfas_ts_data, early_suspend);
    ret = melfas_ts_suspend(ts->client, PMSG_SUSPEND);
    if(ret)
    {
        printk(KERN_ERR "melfas_ts_suspend failed!!!\n");
    }
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
    struct melfas_ts_data *ts;
    int ret = 0;
    ts = container_of(h, struct melfas_ts_data, early_suspend);

    ret = melfas_ts_resume(ts->client);
    if(ret)
    {
        printk(KERN_ERR "melfas_ts_resume failed!!!\n");
    }
}
#endif


static const struct i2c_device_id melfas_ts_id[] =
{
    { MELFAS_TS_NAME, 0 },
    { }
};


#ifdef CONFIG_OF
static struct of_device_id melfas_gpios_of_match[] = {
    { .compatible = "melfas,melfas-ts", },
    { },
};
MODULE_DEVICE_TABLE(i2c, melfas_ts_id);
#endif
static struct i2c_driver melfas_ts_driver =
{
    .driver = {
    .name = MELFAS_TS_NAME,
    .owner = THIS_MODULE,
    .of_match_table = of_match_ptr(melfas_gpios_of_match),
    },
    .id_table = melfas_ts_id,
    .probe  = melfas_ts_probe,
    .remove = __exit_p(melfas_ts_remove),
#ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend = melfas_ts_suspend,
    .resume  = melfas_ts_resume,
#endif
};


static int __init melfas_ts_init(void)
{
 
    huawei_smem_info *smem_data = NULL;

    
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

    if (NULL == smem_data)
    {
        printk(KERN_ERR "nandc_mtd_dload_proc_deal:get smem_data error\n");
   
        return 0;
    }
   
    if(smem_data->smem_dload_flag == SMEM_DLOAD_FLAG_NUM)
    {
         printk(KERN_ERR "Need not start up TP in update process\n");
        return 0;
    }
 
    return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
 
    huawei_smem_info *smem_data = NULL;

    
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;

    if (NULL == smem_data)
    {
        printk(KERN_ERR "nandc_mtd_dload_proc_deal:get smem_data error\n");
       
        return;
    }
    if(smem_data->smem_dload_flag == SMEM_DLOAD_FLAG_NUM)
    {
        printk(KERN_ERR "Need not start up TP in update process\n");
        return ;
    }
 

    i2c_del_driver(&melfas_ts_driver);

    if (NULL != melfas_ts_wq)
    {
        destroy_workqueue(melfas_ts_wq);
    }

}

int touch_melfas_init(void)
{
    int ret = -1;
    ret = melfas_ts_init();
    if( 0 > ret )
    {
        printk(KERN_ERR "Melfas TP touch_driver_init fail!\n");
        return ret;
    }

    return 0;
}

void touch_melfas_exit(void)
{
    melfas_ts_exit();
}

MODULE_DESCRIPTION("Driver for Melfas MIP Touchscreen Controller");
MODULE_LICENSE("GPL");
/*
module_init(melfas_ts_init);
module_exit(melfas_ts_exit);
*/
