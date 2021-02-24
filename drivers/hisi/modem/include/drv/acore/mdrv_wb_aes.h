
#ifndef __MDRV_WB_AES_H__
#define __MDRV_WB_AE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

void wb_aes_decrypt(const u8* input, u8* output);
extern int wb_aes_decrypt_cbc(
		const u8* iv,
        const u8* input, u32 in_len,
        u8* output, u32* out_len
        );
#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif/*end for __MDRV_WB_AE_H__*/

