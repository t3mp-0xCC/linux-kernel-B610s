#ifndef __SOC_ONCHIP_H__
#define __SOC_ONCHIP_H__ 
#include "soc_memmap.h"
#define FEATURE_RSA_LOCAL 
#define M3_TCM_ADDR (0x10000000)
#define M3_TCM_SIZE (128*1024)
#define SRAM_ADDR (0x80800000)
#define SRAM_SIZE (64*1024)
#define M3_TCM_SHARE_DATA_SIZE sizeof(tOcrShareData)
#define M3_TCM_SHARE_DATA_ADDR (M3_TCM_ADDR + M3_TCM_SIZE - M3_TCM_SHARE_DATA_SIZE)
#define OCR_INITED_FLAG_ADDR (M3_TCM_ADDR + M3_TCM_SIZE - 4)
#define OCR_AUTO_ENUM_FLAG_ADDR (M3_TCM_ADDR + M3_TCM_SIZE - 8)
#define OCR_INITED_FLAG_VALUE (0x23456789)
#define OCR_UNINITED_FLAG_VALUE (0xA55A6789)
#define AUTO_ENUM_FLAG_VALUE 0x82822828
#define ROOT_CA_WITHOUT_MD5 
#define EFUSE_GROUP_INDEX_SECURE (31)
#define EFUSE_BIT_FLAG_SEC_EN ((0x1<<3)|(0x1<<4))
#define EFUSE_GRP_SECBOOT_ID (18)
#define KEY_OEMID_INDEX sizeof(KEY_STRUCT)
#define KEY_HWID_INDEX (KEY_OEMID_INDEX+0x04)
#define BL_LEN_INDEX (144 * 4)
#define ROOT_CA_INDEX (BL_LEN_INDEX + 4)
#define ROOT_CA_LEN (260+128)
#define OEM_CA_LEN (268+128)
#define IDIO_LEN 128
#define MAX_N_LENGTH 32
#define SHA256_HASH_SIZE 8
#define MD5_HASH_SIZE 4
#define VRL_TABLE_SIZE (0)
#define NAND_ARGS_FULL_AVAIL 0x3a
#define NAND_ARGS_PART_AVAIL 0x45
#define NANDC_USE_ONCHIPROM_PARAM 
#define NANDC_OOBLENGTH_8BITECCTYPE 
#ifndef __ASSEMBLY__
typedef struct
{
    unsigned short eLen;
    unsigned short nLen;
    unsigned int e[MAX_N_LENGTH];
    unsigned int n[MAX_N_LENGTH];
    unsigned int c[MAX_N_LENGTH];
}KEY_STRUCT;
struct nand_spec_real
{
 unsigned long page_size;
 unsigned long page_num_per_block;
 unsigned long addr_cycle;
 unsigned long ecc_type;
 unsigned long oob_size;
 unsigned long chip_size;
};
struct nand_spec_sram
{
    unsigned int page_size : 4;
    unsigned int ecc_type : 4;
    unsigned int addr_num : 4;
    unsigned int page_num_per_block : 4;
 unsigned int spare_size : 5;
 unsigned int chip_size : 4;
    unsigned int flag : 7;
};
typedef int (*idioIdentifyPtr)(unsigned int dataAddr, unsigned int dataLen, KEY_STRUCT *pKey, unsigned int* pIdio);
typedef int (*SHA256HashPtr)(unsigned int dataAddr, unsigned int dataLen, unsigned int* hash);
typedef int (*RSAPtr)(KEY_STRUCT *pKey, unsigned int* pIndata, unsigned int* pOutdata);
typedef int (*bsp_nand_get_spec_and_save_ptr)(volatile struct nand_spec_sram *spec_sram);
typedef int (*efuseReadPtr)(unsigned int group, unsigned int* pBuf, unsigned int num);
typedef void (*wdtRebootPtr)(unsigned int delay_ms);
typedef void (*printfPtr)(const void *pucBuffer);
typedef struct tagOcrShareData
{
    printfPtr printf;
    wdtRebootPtr wdtReboot;
    efuseReadPtr efuse_read;
    unsigned int emmc_base_addr;
 struct nand_spec_sram nandspec;
 bsp_nand_get_spec_and_save_ptr bsp_nand_get_spec_and_save;
    int errNo;
    SHA256HashPtr SHA256Hash;
    RSAPtr RSA;
    idioIdentifyPtr idioIdentify;
    int bSecEn;
    int bRootCaInited;
    KEY_STRUCT *pRootKey;
    unsigned int ulEnumFlag;
    unsigned int ulOcrInitedFlag;
}tOcrShareData;
#define M3_SRAM_SHARE_DATA_SIZE sizeof(tOcrShareData)
#define M3_SRAM_SHARE_DATA_ADDR (SRAM_ADDR + SRAM_SIZE - M3_SRAM_SHARE_DATA_SIZE)
#endif
#endif
