/*
 * wb_aes_decrypt.h
 *
 */

#ifndef WB_AES_DECRYPT_H_
#define WB_AES_DECRYPT_H_

#include"wb_aes_util.h"

void wb_aes_decrypt(const u8* input, u8* output);
extern int wb_aes_decrypt_cbc(
		const u8* iv,
		const u8* input, u32 in_len,
		u8* output, u32* out_len
		);


#endif /* WB_AES_DECRYPT_H_ */
