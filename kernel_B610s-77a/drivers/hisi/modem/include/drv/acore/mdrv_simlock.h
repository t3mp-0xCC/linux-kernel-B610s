
#ifndef __HUAWEI_SIMLOCK_H__
#define __HUAWEI_SIMLOCK_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (FEATURE_ON == MBB_SIMLOCK_FOUR)

#ifndef BSP_U8
typedef unsigned char       BSP_U8;
#endif

#ifndef BSP_U32
typedef unsigned int        BSP_U32;
#endif
#define DRV_HWLOCK_HASH_LEN    (32)
#define TAF_DRV_HWLOCK_HASH_MAX    (0xFF)
#define AT_AUTHVER_FIVE                (5)
#define HW_LOCK_OEM_TYPE    (1)
#define HW_LOCK_SIMLOCK_TYPE    (2)
#define HWLOCK_ENABLE_FLAG    (0)
#define DRV_OEM_SIMLOCK_ENABLE    (0X65)

#define PBKDF2_OUT_LEN    (32)
#define PBKDF2_CONSTANT_LEN    (4)
#define PBKDF2_SALT_LEN_MAX    (40)
#define SET_ITERATE_CNT    (10000)
#define SHA256_OUT_LEN    (32)
#define DRV_RANDOM_LEN    (32)
#define DRV_HWOCK_LEN_MAX    (32)
#define SALT_IV "E678442CE1B2915C"
#define DRV_ASE_LEN    (48)
#define YEAR_BASE    (1900)
#define DRV_PBKDF2_TIMES_RANDOM_LEN    (4)
extern int hw_lock_set_proc(char *lock_para, unsigned int para_len, unsigned int type);
extern int hw_lock_verify_proc(unsigned char *lock_para, unsigned int para_len, unsigned int type);
extern int cpufreq_dfs_set_profile(int profile);
#endif/*end for MBB_SIMLOCK_FOUR*/

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif/*end for __HUAWEI_SIMLOCK_H__*/

