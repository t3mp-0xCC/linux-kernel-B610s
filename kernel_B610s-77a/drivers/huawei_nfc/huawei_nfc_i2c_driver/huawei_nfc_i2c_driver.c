

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/init.h>
#include <mdrv_nfc.h>
#include "drv_m24sr.h"
#include "libndefuri.h"
#include "libndefwifi.h"
#include "drv_m24sr.h"
#include "libtagtype4.h"

/*----------------------------------------------*
 * 枚举定义                                     *
 *----------------------------------------------*/
 /*打印等级控制*/
enum NFC_DEBUG_LEVEL
{
    HUAWEI_NFC_ERR,           /* error conditions */
    HUAWEI_NFC_WARNING,       /* warning conditions */
    HUAWEI_NFC_NOTICE,        /* normal but significant condition */
    HUAWEI_NFC_INFO,          /* informational */
    HUAWEI_NFC_DEBUG,         /* debug-level messages */
    HUAWEI_NFC_MAX,
};

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define NFC_ERROR(fmt,args...)          nfc_print_message(HUAWEI_NFC_ERR,    fmt,## args)
#define NFC_WARNING(fmt,args...)        nfc_print_message(HUAWEI_NFC_WARNING,fmt,## args)
#define NFC_NOTICE(fmt,args...)         nfc_print_message(HUAWEI_NFC_NOTICE, fmt,## args)
#define NFC_INFO(fmt,args...)           nfc_print_message(HUAWEI_NFC_INFO,   fmt,## args)
#define NFC_DEBUG(fmt,args...)          nfc_print_message(HUAWEI_NFC_DEBUG,  fmt,## args)

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
/*NFC LOG打印控制，默认打印ERR级别LOG*/
/*echo 5 > /sys/module/huawei_nfc_i2c_driver/parameters/g_nfc_log_level 打开所有打印*/
static int32_t g_nfc_log_level = HUAWEI_NFC_MAX;

module_param_named(
    g_nfc_log_level, g_nfc_log_level, int, S_IRUGO | S_IWUSR | S_IWGRP
);

static int32_t g_nfc_init = FALSE;
static int32_t g_nfc_write_flag = FALSE;
/*创建sysfs使用*/
static struct class* g_nfc_class = NULL;
/*操作I2C设备的句柄*/
struct i2c_client* g_huawei_nfc_client = NULL;
/*NFC设备驱动*/
struct huawei_nfc_device* g_huawei_nfc_device = NULL;

/*----------------------------------------------*
 * 结构体定义                                   *
 *----------------------------------------------*/
#if (FEATURE_ON == MBB_NFC_PARSE_URL)
/*URL键值表，用来解析URL*/
static URI_StringDef URI_Table[URI_RFU] =
{
    {URI_ID_0x01, URI_ID_0x01_STRING},
    {URI_ID_0x01, URI_ID_0x01_STRING},
    {URI_ID_0x02, URI_ID_0x02_STRING},
    {URI_ID_0x03, URI_ID_0x03_STRING},
    {URI_ID_0x04, URI_ID_0x04_STRING},
    {URI_ID_0x05, URI_ID_0x05_STRING},
    {URI_ID_0x06, URI_ID_0x06_STRING},
    {URI_ID_0x07, URI_ID_0x07_STRING},
    {URI_ID_0x08, URI_ID_0x08_STRING},
    {URI_ID_0x09, URI_ID_0x09_STRING},
    {URI_ID_0x0A, URI_ID_0x0A_STRING},
    {URI_ID_0x0B, URI_ID_0x0B_STRING},
    {URI_ID_0x0C, URI_ID_0x0C_STRING},
    {URI_ID_0x0D, URI_ID_0x0D_STRING},
    {URI_ID_0x0E, URI_ID_0x0E_STRING},
    {URI_ID_0x0F, URI_ID_0x01_STRING},
    {URI_ID_0x10, URI_ID_0x10_STRING},
    {URI_ID_0x11, URI_ID_0x11_STRING},
    {URI_ID_0x12, URI_ID_0x12_STRING},
    {URI_ID_0x13, URI_ID_0x13_STRING},
    {URI_ID_0x14, URI_ID_0x14_STRING},
    {URI_ID_0x15, URI_ID_0x15_STRING},
    {URI_ID_0x16, URI_ID_0x16_STRING},
    {URI_ID_0x17, URI_ID_0x17_STRING},
    {URI_ID_0x18, URI_ID_0x18_STRING},
    {URI_ID_0x19, URI_ID_0x19_STRING},
    {URI_ID_0x1A, URI_ID_0x1A_STRING},
    {URI_ID_0x1B, URI_ID_0x1B_STRING},
    {URI_ID_0x1C, URI_ID_0x1C_STRING},
    {URI_ID_0x1D, URI_ID_0x1D_STRING},
    {URI_ID_0x1E, URI_ID_0x1E_STRING},
    {URI_ID_0x1F, URI_ID_0x1F_STRING},
    {URI_ID_0x20, URI_ID_0x20_STRING},
    {URI_ID_0x21, URI_ID_0x21_STRING},
    {URI_ID_0x22, URI_ID_0x22_STRING},
    {URI_ID_0x23, URI_ID_0x23_STRING},
};
 #endif
/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : nfc_print_message
 功能描述  : NFC 驱动模块LOG打印
 输入参数  : int print_level
             const char *fmt...
 输出参数  : NA
 返 回 值  : vprintk返回值
 调用函数  : va_start vprintk va_end
 被调函数  : NFC_ERROR NFC_WARNING NFC_NOTICE NFC_INFO HUAWEI_NFC_DEBUG
*****************************************************************************/
asmlinkage int nfc_print_message(int print_level, const char* fmt, ...)
{
    va_list args;
    int rc = 0;

    /*小于g_chg_log_level级别的LOG可以被打印*/
    if ((uint32_t)print_level <= g_nfc_log_level)
    {
        va_start(args, fmt);
        rc = vprintk(fmt, args);
        va_end(args);
    }

    return rc;
}

/*****************************************************************************
 函 数 名  : huawei_nfc_is_initialized
 功能描述  : 判断NFC芯片是否初始化完成
 输入参数  : NA
 输出参数  : NA
 返 回 值  : NFC芯片初始化状态
 调用函数  : NA
 被调函数  : NA
*****************************************************************************/
int32_t huawei_nfc_is_initialized(void)
{
    return g_nfc_init;
}

/*****************************************************************************
 函 数 名  : huawei_nfc_i2c_read_wifi_url_token
 功能描述  : 从NFC读取wifi和URL信息
 输入参数  : 无
 输出参数  : sWifiUrlToken *pToken 存储读取到的wifi和URL信息
 返 回 值  : 0     :函数执行失败
                  非零 :写入的字节数
 调用函数  : TT4_ReadWifiURLToken
 被调函数  : NA
*****************************************************************************/
static uint16_t huawei_nfc_i2c_read_wifi_url_token(sWifiUrlToken* pToken)
{
    uint16_t ret = ERROR;

    if(NULL == pToken)
    {
        NFC_ERROR("HUAWEI_NFC: invalid params!\n");
        return - EINVAL;
    }

    ret = TT4_ReadWifiURLToken(pToken);

    if ( SUCCESS != ret )
    {
        NFC_ERROR("HUAWEI_NFC: nfc read ndef failed!\n");
    }

    return ret;
}

/*****************************************************************************
 函 数 名  : huawei_nfc_i2c_write_wifi_url_token
 功能描述  : 写入WIFI+URL 数据到NDEF File
 输入参数  : sWifiUrlToken *pToken 包含WIFI的SSID+PWD,URL的信息。
 输出参数  : 无
 返 回 值  : 0     :函数执行失败
             非零  :写入的字节数
 调用函数  : TT4_WriteWifiURLToken
 被调函数  : NA
*****************************************************************************/
static uint16_t huawei_nfc_i2c_write_wifi_url_token(sWifiUrlToken* pToken)
{
    uint16_t ret = ERROR;

    if(NULL == pToken)
    {
        NFC_ERROR("HUAWEI_NFC: invalid params!\n");
        return -EINVAL;
    }

    if(TRUE != g_nfc_write_flag)
    {
        ret = TT4_WriteWifiURLToken(pToken);

        if ( SUCCESS != ret )
        {
            NFC_ERROR("HUAWEI_NFC: nfc write ndef failed!\n");
        }
    }
    else
    {
        NFC_ERROR("HUAWEI_NFC: g_nfc_write_flag is true, do not write wifi token!\n");
    }

    return ret;
}
#if (FEATURE_ON == MBB_NFC_PARSE_URL)
/*****************************************************************************
 函 数 名  : huawei_nfc_parse_url
 功能描述  : 将URL分解为protocol + message 两部分
 输入参数  : char *pURL
 输出参数  : sURI_Info *pToken
 返 回 值  : NA
 调用函数  : NA
 被调函数  : NA
*****************************************************************************/
static void huawei_nfc_parse_url(char* pURL, sURI_Info* pToken)
{
    uint32_t nID = 0;
    char* pTemp = NULL;

    if(NULL == pToken)
    {
        NFC_ERROR("HUAWEI_NFC: invalid params!\n");
        return;
    }

    for (nID = 0; nID < URI_RFU; nID++)
    {
        if (!strncmp( pURL, URI_Table[nID].aString, strlen(URI_Table[nID].aString)))
        {
            strncpy(pToken->protocol,
                URI_Table[nID].aString, NFC_URL_PROTOCOL_MAX_SIZE);
            pTemp = pURL + strlen(URI_Table[nID].aString);
            strncpy(pToken->URI_Message, pTemp, NFC_URL_MESSAGE_MAX_SIZE);
            return;
        }
    }
    NFC_ERROR("HUAWEI_NFC:ERROR (%s) does not mached any string!\n", pURL);
}
#endif
/*****************************************************************************
 函 数 名  : huawei_nfc_show
 功能描述  : 读取NFC数据
 输入参数  : struct device *dev,
		     struct device_attribute *attr
 输出参数  : char *buf
 返 回 值  : 负数   :函数执行失败
             非负   :函数执行成功，返回读到的字节数。
 调用函数  : huawei_nfc_i2c_read_wifi_url_token
 被调函数  : NA
*****************************************************************************/
static ssize_t huawei_nfc_show(struct device* dev,
                                struct device_attribute* attr,
                                char* buf)
{
    uint16_t         ret  = 0;
    sWifiUrlToken    token;
    huawei_nfc_token nfc_token;

    if(FALSE == huawei_nfc_is_initialized())
    {
        NFC_ERROR("HUAWEI_NFC: nfc chip have not initialized yet!\n");
        return -EAGAIN;
    }

    memset(&token, '\0', sizeof(sWifiUrlToken));
    memset(&nfc_token, '\0', sizeof(huawei_nfc_token));
   /*读取WiFi和URL信息*/
    ret = huawei_nfc_i2c_read_wifi_url_token(&token);

    if (SUCCESS != ret)
    {
        NFC_ERROR("HUAWEI_NFC:ERROR read ndef (%d)!\n", ret);
        return -EFAULT;
    }

    strncpy(nfc_token.NetworkSSID, token.WifiToken.NetworkSSID,
        HUAWEI_NFC_MAX_SSID_SIZE);
    strncpy(nfc_token.NetworkKey, token.WifiToken.NetworkKey,
        HUAWEI_NFC_MAX_PWD_SIZE);
    strncpy(nfc_token.NetSecType, token.WifiToken.AuthenticationType,
        HUAWEI_NFC_MAX_SEC_SIZE);
    strncpy(nfc_token.URL, token.URLToken.protocol,
        HUAWEI_NFC_MAX_URL_SIZE);
    strncat(nfc_token.URL, token.URLToken.URI_Message,
        (HUAWEI_NFC_MAX_URL_SIZE - strlen(token.URLToken.protocol)));
    memcpy(buf, (char*)&nfc_token, sizeof(huawei_nfc_token));

    return sizeof(huawei_nfc_token);
}
/*****************************************************************************
 函 数 名  : huawei_nfc_store
 功能描述  : 写入NFC数据
 输入参数  : struct device *dev,
            struct device_attribute *attr
 输出参数  : char *buf
 返 回 值  : 负数     :函数执行失败
            非负     :函数执行成功，返回写入的字节数
 调用函数  : huawei_nfc_i2c_write_wifi_url_token
 被调函数  : NA
*****************************************************************************/
static ssize_t huawei_nfc_store(struct device* dev,
                                struct device_attribute* attr,
                                const char* buf,
                                size_t count)
{
    uint16_t         ret = 0;
    sWifiUrlToken    token ;
    huawei_nfc_token nfc_token ;

    if(FALSE == huawei_nfc_is_initialized())
    {
        NFC_ERROR("HUAWEI_NFC: nfc chip have not initialized yet!\n");
        return -EAGAIN;
    }

    if(NULL == buf)
    {
        NFC_ERROR("HUAWEI_NFC: buf is null!\n");
        return -EINVAL;
    }

    if(count > sizeof(huawei_nfc_token))
    {
        NFC_ERROR("HUAWEI_NFC:invalid input params!\n");
        return -EINVAL;
    }

    memset(&token, '\0', sizeof(sWifiUrlToken));
    memset(&nfc_token, '\0', sizeof(huawei_nfc_token));
    memcpy(&nfc_token, buf, sizeof(huawei_nfc_token));

    /*检测数据长度的有效性*/
    if ( (strlen(nfc_token.NetworkSSID) > HUAWEI_NFC_MAX_SSID_SIZE)
         || (strlen(nfc_token.NetworkKey) >  HUAWEI_NFC_MAX_PWD_SIZE)
         || (strlen(nfc_token.URL) > HUAWEI_NFC_MAX_URL_SIZE)
         || (strlen(nfc_token.NetSecType) > HUAWEI_NFC_MAX_SEC_SIZE))
    {
        NFC_ERROR("HUAWEI_NFC:nfc_token length is error!\n");
        return -EINVAL;
    }
    else
    {
        strncpy(token.WifiToken.NetworkSSID, nfc_token.NetworkSSID,
            HUAWEI_NFC_MAX_SSID_SIZE);
        strncpy(token.WifiToken.NetworkKey, nfc_token.NetworkKey,
            HUAWEI_NFC_MAX_PWD_SIZE);
        strncpy(token.WifiToken.AuthenticationType, nfc_token.NetSecType,
            HUAWEI_NFC_MAX_SEC_SIZE);
#if (FEATURE_ON == MBB_NFC_PARSE_URL)
        huawei_nfc_parse_url(nfc_token.URL,&token.URLToken);
#else
        /*URL为固定值，在驱动中写死*/
        strncpy(token.URLToken.protocol, URI_ID_0x04_STRING,
            NFC_URL_PROTOCOL_MAX_SIZE);
        strncpy(token.URLToken.URI_Message,
            HUAWEI_NFC_URL_MESSAGE, NFC_URL_MESSAGE_MAX_SIZE);
#endif
    }

    /*写WIFI和URL信息到NDEF*/
    ret = huawei_nfc_i2c_write_wifi_url_token(&token);

    if (SUCCESS != ret)
    {
        NFC_ERROR("HUAWEI_NFC:ERROR write ndef (%d)!\n", ret);
        return -EFAULT;
    }

    NFC_INFO("HUAWEI_NFC:write successfully!\n");

    return count;
}

static DEVICE_ATTR(nfc_dev, S_IRUGO | S_IWUSR, huawei_nfc_show, huawei_nfc_store);


/*****************************************************************************
 函 数 名  : huawei_nfc_write_info
 功能描述  : 此接口用于AT接口调用，写入NFC相关信息
 输入参数  : huawei_nfc_token *nfc_info NFC信息结构体
 输出参数  : NA
 返 回 值  : ERROR    :函数执行失败
             SUCCESS  :函数执行成功
 调用函数  : huawei_nfc_i2c_write_wifi_url_token
 被调函数  : AT模块
*****************************************************************************/
unsigned short huawei_nfc_write_info(huawei_nfc_token *nfc_info)
{
    sWifiUrlToken token;
    uint16_t      ret = 0;

    if(NULL == nfc_info)
    {
        NFC_ERROR("HUAWEI_NFC:ERROR nfc_info is null!\n");
        return ERROR;
    }

    if((strlen(nfc_info->NetworkSSID) > HUAWEI_NFC_MAX_SSID_SIZE) || 
        (strlen(nfc_info->NetworkKey) > HUAWEI_NFC_MAX_PWD_SIZE))
    {
        NFC_ERROR("HUAWEI_NFC:ERROR nfc_info size is error!\n");
        return ERROR;
    }

    if(FALSE == huawei_nfc_is_initialized())
    {
        NFC_ERROR("HUAWEI_NFC: nfc chip have not initialized yet!\n");
        return ERROR;
    }

    g_nfc_write_flag = FALSE;    /*写之前置为FALSE*/

    memset(&token, '\0', sizeof(sWifiUrlToken));

    strncpy(token.WifiToken.NetworkSSID, nfc_info->NetworkSSID,
            HUAWEI_NFC_MAX_SSID_SIZE);
    strncpy(token.WifiToken.NetworkKey, nfc_info->NetworkKey,
            HUAWEI_NFC_MAX_PWD_SIZE);

    strncpy(token.WifiToken.AuthenticationType, nfc_info->NetSecType,
            HUAWEI_NFC_MAX_SEC_SIZE);
    /*URL为固定值，在驱动中写死*/
    strncpy(token.URLToken.protocol, URI_ID_0x04_STRING,
        NFC_URL_PROTOCOL_MAX_SIZE);
    strncpy(token.URLToken.URI_Message,
        HUAWEI_NFC_URL_MESSAGE, NFC_URL_MESSAGE_MAX_SIZE);

    /*写WIFI和URL信息到NDEF*/
    ret = huawei_nfc_i2c_write_wifi_url_token(&token);

    g_nfc_write_flag = TRUE;    /*写之后置为TRUE*/

    if (SUCCESS != ret)
    {
        NFC_ERROR("HUAWEI_NFC:ERROR write ndef file failed!\n");
        return ERROR;
    }

    return SUCCESS;
}

/*****************************************************************************
 函 数 名  : huawei_nfc_read_info
 功能描述  : 此接口用于AT接口调用，读取NFC相关信息
 输入参数  : huawei_nfc_token *nfc_info NFC信息结构体
 输出参数  : NA
 返 回 值  : ERROR    :函数执行失败
             SUCCESS  :函数执行成功
 调用函数  : huawei_nfc_i2c_read_wifi_url_token
 被调函数  : AT模块
*****************************************************************************/
unsigned short huawei_nfc_read_info(huawei_nfc_token *nfc_info)
{
    sWifiUrlToken    token;
    huawei_nfc_token nfc_token;
    uint16_t         ret = 0;

    if(NULL == nfc_info)
    {
        NFC_ERROR("HUAWEI_NFC:ERROR nfc_info is null!\n");
        return ERROR;
    }

    if(FALSE == huawei_nfc_is_initialized())
    {
        NFC_ERROR("HUAWEI_NFC: nfc chip have not initialized yet!\n");
        return ERROR;
    }

    memset(&token, '\0', sizeof(sWifiUrlToken));
    memset(&nfc_token, '\0', sizeof(huawei_nfc_token));

    ret = huawei_nfc_i2c_read_wifi_url_token(&token);

    if (SUCCESS != ret)
    {
        NFC_ERROR("HUAWEI_NFC:ERROR read ndef file failed!\n");
        return ERROR;
    }

    strncpy(nfc_token.NetworkSSID, token.WifiToken.NetworkSSID,
        HUAWEI_NFC_MAX_SSID_SIZE);
    strncpy(nfc_token.NetworkKey, token.WifiToken.NetworkKey,
        HUAWEI_NFC_MAX_PWD_SIZE);
    memcpy((void *)nfc_info, (void *)&nfc_token, sizeof(huawei_nfc_token));

    return SUCCESS;
}
/*****************************************************************************
 函 数 名  : huawei_nfc_rf_enable
 功能描述  : 控制nfc打开关闭
 输入参数  : rf_enable rf_enable值
 输出参数  : NA
 返 回 值  : 非0    :函数执行失败
             0      :函数执行成功
 调用函数  : M24SR_RFEnable
 被调函数  : huawei_nfc_enable_store
*****************************************************************************/
int huawei_nfc_rf_enable(unsigned char rf_enable)
{
    uint16_t ret = 0;

    if((HUAWEI_NFC_DISABLE_VALUE != rf_enable) && (HUAWEI_NFC_ENABLE_VALUE != rf_enable))
    {
        NFC_ERROR("HUAWEI_NFC:huawei_nfc_rf_enable input parameter is error!\n");
        return -1;
    }

    ret = M24SR_RFEnable(rf_enable);

    if((M24SR_ERROR_I2CTIMEOUT == ret) || (M24SR_ERROR_CRC == ret))
    {
        NFC_ERROR("HUAWEI_NFC:M24SR_RFEnable failed!\n");
        return -1;
    }

    ret = M24SR_Deselect();

    if(M24SR_ACTION_COMPLETED != ret)
    {
        NFC_ERROR("HUAWEI_NFC:M24SR_Deselect failed!\n");
        return -1;
    }

    return 0;
}

/*****************************************************************************
 函 数 名  : huawei_nfc_get_rf_enable_status
 功能描述  : 获取RF ENABLE状态
 输入参数  : NA
 输出参数  : NA
 返 回 值  : 0-NFC RF READ关闭，1-NFC RF READ打开，-1-获取状态失败
 调用函数  : M24SR_ReadBinary/M24SR_KillSession/M24SR_SelectApplication
 被调函数  : huawei_nfc_enable_show
*****************************************************************************/

int huawei_nfc_get_rf_enable_status(void)
{
    uint16_t status = 0;
    unsigned char enable_status = 0;
    int ret = 0;

    status = M24SR_KillSession();

    if(M24SR_ACTION_COMPLETED != status)
    {
        printk(KERN_ERR "[%s : %d] : M24SR_KillSession failed!!!\n",__func__,__LINE__);
        return -1;
    }

    (void)M24SR_SelectApplication();
    (void)M24SR_SelectSystemfile();

    status = M24SR_ReadBinary ( HUAWEI_NFC_RF_DISABLE_OFFSET , HUAWEI_NFC_READ_RF_DISABLE_BYTES , &enable_status );

    if((M24SR_ERROR_I2CTIMEOUT == status) || (M24SR_ERROR_CRC == status))
    {
        printk(KERN_ERR "M24SR_ReadBinary failed!!!\n");
        return -1;
    }

    status = M24SR_Deselect();

    if(M24SR_ACTION_COMPLETED != status)
    {
        printk(KERN_ERR "M24SR_Deselect failed!!!\n");
        return -1;
    }

    if(HUAWEI_NFC_DISABLE_VALUE == enable_status)
    {
        ret = 0;
    }
    else if(HUAWEI_NFC_ENABLE_VALUE == enable_status)
    {
        ret = 1;
    }
    else
    {
        printk(KERN_ERR "enable_status is other value!!!\n");
        ret = -1;
    }

    return ret;
}
/*****************************************************************************
 函 数 名  : huawei_nfc_enable_show
 功能描述  : 读取NFC使能状态
 输入参数  : struct device *dev,
            struct device_attribute *attr
 输出参数  : char *buf
 返 回 值  : 返回读到的字节数
 调用函数  : huawei_nfc_get_rf_enable_status
 被调函数  : NA
*****************************************************************************/
static ssize_t huawei_nfc_enable_show(struct device* dev,
                                                    struct device_attribute* attr,
                                                    char* buf)
{
    int nfc_enable_status;

    nfc_enable_status = huawei_nfc_get_rf_enable_status();

    if(-1 == nfc_enable_status)
    {
        printk(KERN_ERR "huawei_nfc_get_rf_enable_status error!!!\n");
        return -1;
    }

    if(NULL == buf)
    {
        printk(KERN_ERR "buf is null!!!\n");
        return -1;
    }

    return snprintf(buf, HUWAEI_NFC_ENABLE_BUFF_SIZE, "%u\n", nfc_enable_status);
}

/*****************************************************************************
 函 数 名  : huawei_nfc_enable_store
 功能描述  : 控制NFC打开或者关闭
 输入参数  : struct device *dev,
            struct device_attribute *attr
 输出参数  : char *buf
 返 回 值  : 返回写入的字节数
 调用函数  : huawei_nfc_rf_enable
 被调函数  : NA
*****************************************************************************/
static ssize_t huawei_nfc_enable_store(struct device* dev,
                                                   struct device_attribute* attr,
                                                   const char* buf,
                                                   size_t count)
{
    unsigned long status;
    ssize_t ret = -EINVAL;
    int nfc_enable_result;

    ret = kstrtoul(buf, 10, &status);

    if(ret)
    {
        printk(KERN_ERR "kstrtoul error!\n");
        return ret;
    }

    if(HUAWEI_NFC_ENABLE_OFF == status)
    {
        nfc_enable_result = huawei_nfc_rf_enable(HUAWEI_NFC_DISABLE_VALUE);

        if(-1 == nfc_enable_result)
        {
            printk(KERN_ERR "huawei_nfc_enable_store : enable off error!\n");
            return -1;
        }
    }
    else if(HUAWEI_NFC_ENABLE_ON == status)
    {
        nfc_enable_result = huawei_nfc_rf_enable(HUAWEI_NFC_ENABLE_VALUE);

        if(-1 == nfc_enable_result)
        {
            printk(KERN_ERR "huawei_nfc_enable_store : enable on error!\n");
            return -1;
        }
    }
    else
    {
        printk(KERN_ERR "huawei_nfc_enable_store : input value is error!\n");
    }
    return count;
}

static DEVICE_ATTR(nfc_enable, S_IRUGO | S_IWUSR, huawei_nfc_enable_show, huawei_nfc_enable_store);

/*****************************************************************************
 函 数 名  : huawei_nfc_get_handle
 功能描述  : 获取操作NFC的句柄
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 操作句柄
 调用函数  : NA
 被调函数  : NA
*****************************************************************************/
struct i2c_client* huawei_nfc_get_handle(void)
{
    return g_huawei_nfc_client;
}

/*****************************************************************************
 函 数 名  : huawei_nfc_probe
 功能描述  : NFC 驱动加载函数
 输入参数  : struct i2c_client* client
             const struct i2c_device_id* id
 输出参数  : NA
 返 回 值  : 0-驱动加载成功，非0-驱动加载失败
 调用函数  : NA
 被调函数  : NA
*****************************************************************************/
static int huawei_nfc_probe(struct i2c_client* client, const struct i2c_device_id* id)
{
    int32_t ret = 0;
    u16 init_result = ERROR;
    struct huawei_nfc_device* nfc_device = NULL;
    struct device_node* node = client->dev.of_node;

    dev_info(&client->dev, "HUAWEI_NFC:%s:***huawei_nfc_probe***\n", __func__);

    if (!node)
    {
        dev_err(&client->dev,
            "HUAWEI_NFC:%s: device tree information missing\n", __func__);
        return - ENODEV;
    }

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) //判断适配器能力
    {
        dev_err(&client->dev, "HUAWEI_NFC:%s: SMBUS_BYTE_DATA unsupported\n", __func__);
        return - EIO;
    }

    nfc_device = devm_kzalloc(&client->dev, sizeof(struct huawei_nfc_device), GFP_KERNEL);

    if (!nfc_device)
    {
        dev_err(&client->dev, "HUAWEI_NFC:%s: devm_kzalloc failed\n", __func__);
        return - ENOMEM;
    }

    memset(nfc_device, 0x0, sizeof(struct huawei_nfc_device));

    nfc_device->nfc_i2c_client = client;

    i2c_set_clientdata(client, nfc_device);

    g_huawei_nfc_client = nfc_device->nfc_i2c_client;
    g_huawei_nfc_device = nfc_device;

    /*Create nfc device class.--> /sys/class/huawei_nfc.*/
    g_nfc_class = class_create(THIS_MODULE, "huawei_nfc");
    if(IS_ERR(g_nfc_class))
    {
        ret = PTR_ERR(g_nfc_class);
        NFC_ERROR("HUAWEI_NFC: Create class failed!\n");
        goto err_create_class;
    }

    /*Create NFC device --> /sys/class/huawei_nfc/nfc.*/
    nfc_device->nfc_dev = device_create(g_nfc_class, NULL, MKDEV(0, 0), "%s", "nfc");
    if (IS_ERR(nfc_device->nfc_dev))
    {
        ret = PTR_ERR(nfc_device->nfc_dev);
        NFC_ERROR("HUAWEI_NFC: Create device failed!\n");
        goto  err_create_device;
    }

    /*Create NFC sysfs device node --> /sys/class/huawei_nfc/nfc/nfc_dev.*/
    ret = device_create_file(nfc_device->nfc_dev, &dev_attr_nfc_dev);
    if (ret)
    {
        NFC_ERROR("HUAWEI_NFC: Create sysfs attributes failed!\n");
        goto err_create_sysfs_nfc_dev;
    }

    /*Create NFC sysfs device node --> /sys/class/huawei_nfc/nfc/nfc_enable.*/
    ret = device_create_file(nfc_device->nfc_dev, &dev_attr_nfc_enable);
    if (ret)
    {
        NFC_ERROR("HUAWEI_NFC: Create sysfs attributes failed!\n");
        goto err_create_sysfs_nfc_enable;
    }

    NFC_DEBUG("HUAWEI_NFC: Init NFC start!\n");

    /*Init NFC chip.*/
    init_result = TT4_Init();

    if(SUCCESS == init_result)
    {
        g_nfc_init = TRUE;
        NFC_DEBUG("HUAWEI_NFC: Init NFC finish!\n");
    }
    else
    {
        NFC_ERROR("HUAWEI_NFC: Init NFC ERROR!\n");
    }

    return 0;
err_create_sysfs_nfc_enable:
    device_remove_file(nfc_device->nfc_dev, &dev_attr_nfc_dev);
err_create_sysfs_nfc_dev:
    device_destroy(g_nfc_class, MKDEV(0, 0));
err_create_device:
    class_destroy(g_nfc_class);
err_create_class:
    devm_kfree(&client->dev, nfc_device);
    nfc_device = NULL;
    return ret;
}

static const struct i2c_device_id nfc_device_id[] =
{
    {
        .name = NFC_IC_DRVNAME,
    },

    {

    },
};

static const struct of_device_id nfc_device_match[] =
{
    {
        .compatible = NFC_COMPATIBLE_NAME, 
    },

    {

    },
};


static struct i2c_driver huawei_nfc_i2c_driver =
{
    .driver    = 
    {
        .name           = NFC_IC_DRVNAME,
        .owner          = THIS_MODULE,
        .of_match_table = nfc_device_match,
    },

    .probe          = huawei_nfc_probe,   /*i2c_client和i2c_driver匹配时调用*/
    .id_table       = nfc_device_id,
};

static int __init nfc_init(void)
{
    return i2c_add_driver(&huawei_nfc_i2c_driver);
}
module_init(nfc_init);

static void __exit nfc_exit(void)
{
    i2c_del_driver(&huawei_nfc_i2c_driver);
    return;
}
module_exit(nfc_exit);

MODULE_DESCRIPTION(NFC_IC_DRVNAME "NFC");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("i2c:" NFC_IC_DRVNAME);
