

#ifndef __MDRV_NFC__H__
#define __MDRV_NFC__H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define NFC_IC_DRVNAME                      ("m24sr02")
#define NFC_COMPATIBLE_NAME                 ("st,m24sr02")
#define HUAWEI_NFC_URL_MESSAGE              ("play.google.com/store/apps/details?id=com.huawei.mw\0")

#define TRUE   (1)
#define FALSE  (0)

#define HUAWEI_NFC_MAX_SSID_SIZE        (32)
#define HUAWEI_NFC_MAX_PWD_SIZE         (64)
#define HUAWEI_NFC_MAX_SEC_SIZE         (8)
#define HUAWEI_NFC_MAX_URL_SIZE         (64)

#define HUAWEI_NFC_ENABLE_ON            (1)
#define HUAWEI_NFC_ENABLE_OFF           (0)

#define HUAWEI_NFC_ENABLE_VALUE         (0x01)
#define HUAWEI_NFC_DISABLE_VALUE        (0x00)

#define HUWAEI_NFC_ENABLE_BUFF_SIZE     (2)

#define HUAWEI_NFC_RF_DISABLE_OFFSET     (0x0006)
#define HUAWEI_NFC_READ_RF_DISABLE_BYTES (0x01)


/*----------------------------------------------*
 * 结构体定义                                   *
 *----------------------------------------------*/
struct huawei_nfc_device
{
    struct class*    nfc_class;
    struct device*   nfc_dev;
    struct i2c_client* nfc_i2c_client;
    struct work_struct  work;
    int nfc_int_gpio;
    struct wake_lock nfc_wake_lock;
};

typedef struct 
{
    char NetworkSSID[HUAWEI_NFC_MAX_SSID_SIZE + 1];
    char NetworkKey[HUAWEI_NFC_MAX_PWD_SIZE + 1];
    char NetSecType[HUAWEI_NFC_MAX_SEC_SIZE + 1];
    char URL[HUAWEI_NFC_MAX_URL_SIZE + 1];
    char reserved;
}huawei_nfc_token;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
struct i2c_client* huawei_nfc_get_handle(void);
unsigned short huawei_nfc_write_info(huawei_nfc_token *nfc_info);
unsigned short huawei_nfc_read_info(huawei_nfc_token *nfc_info);
int huawei_nfc_rf_enable(unsigned char rf_enable);
int huawei_nfc_is_initialized(void);


 #endif /*__MDRV_NFC__H__*/
