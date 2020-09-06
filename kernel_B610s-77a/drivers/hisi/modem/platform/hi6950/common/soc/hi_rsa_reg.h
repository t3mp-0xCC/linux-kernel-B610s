#ifndef __HI_RSA_REG_H__
#define __HI_RSA_REG_H__ 
#ifdef __cplusplus
extern "C" {
#endif
#define HI_RSA_BASE (0x90004000)
#define HI_RSA_OPERAND_E_REG (0x000)
#define HI_RSA_OPERAND_N_REG (0x080)
#define HI_RSA_OPERAND_C_REG (0x100)
#define HI_RSA_OPERAND_M_REG (0x180)
#define HI_RSA_RESULT_R_REG (0x200)
#define HI_RSA_N_LENGTH_REG (0x300)
#define HI_RSA_E_LENGTH_REG (0x304)
#define HI_RSA_MULT_EXP_MODULE_REG (0x308)
#define HI_RSA_FORCE_END_REG (0x30C)
#define HI_RSA_ROOTKEY_MODE_REG (0x310)
#define HI_RSA_RINT_REG (0x314)
#define HI_RSA_MINT_REG (0x318)
#define HI_RSA_INT_REG (0x31C)
#define HI_RSA_BUSY_REG (0x320)
#define HI_RSA_ERROR_REG (0x324)
#define HI_RSA_ST_REG (0x328)
#ifdef __cplusplus
}
#endif
#endif
