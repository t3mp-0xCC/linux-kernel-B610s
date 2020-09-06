/*
 * wb_aes_encrypt.h
 *
 */

#ifndef WB_AES_ENCRYPT_H_
#define WB_AES_ENCRYPT_H_

#include"wb_aes_util.h"

void wb_aes_encrypt(const u8* input, u8* output);
extern int wb_aes_encrypt_cbc(
		const u8* iv,
		const u8* input, u32 in_len,
		u8* output, u32* out_len);

#endif /* WB_AES_ENCRYPT_H_ */
