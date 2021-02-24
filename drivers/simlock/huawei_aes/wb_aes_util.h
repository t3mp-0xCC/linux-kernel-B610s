/*
 * wb_aes_util.h
 *
 */

#ifndef WB_AES_UTIL_H_
#define WB_AES_UTIL_H_

//#include<stdio.h>
//#include<string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define TB(tb,r,col,row,i)		(tb)[(r)<<(s+4)|(col)<<(s+2)|(row)<<s|(i)]
/*
 * 64-bit integer manipulation macros (little endian)
 */
#define GET_UINT64_LE(n,b,i)                 \
do{                                          \
    (n) = ( (u64) (b)[(i)    ]       )       \
        | ( (u64) (b)[(i) + 1] << 16 )       \
        | ( (u64) (b)[(i) + 2] << 32 )       \
        | ( (u64) (b)[(i) + 3] << 48 );      \
}while(0)

#define PUT_UINT64_LE(n,b,i)                  \
do{                                           \
    (b)[(i)    ] = (u16) ( (n)       );       \
    (b)[(i) + 1] = (u16) ( (n) >> 16 );       \
    (b)[(i) + 2] = (u16) ( (n) >> 32 );       \
    (b)[(i) + 3] = (u16) ( (n) >> 48 );       \
}while(0)

/*
 * 32-bit integer manipulation macros (little endian)
 */
#define GET_UINT32_LE(n,b,i)                    \
do{                                             \
    (n) = ( (u32) ((b)[(i)    ] & 0xff)       ) \
        | ( (u32) ((b)[(i) + 1] & 0xff) <<  8 ) \
        | ( (u32) ((b)[(i) + 2] & 0xff) << 16 ) \
        | ( (u32) ((b)[(i) + 3] & 0xff) << 24 );\
}while(0)

#define PUT_UINT32_LE(n,b,i)                 \
do{                                          \
    (b)[(i)    ] = (u8) ( (n)       );       \
    (b)[(i) + 1] = (u8) ( (n) >>  8 );       \
    (b)[(i) + 2] = (u8) ( (n) >> 16 );       \
    (b)[(i) + 3] = (u8) ( (n) >> 24 );       \
}while(0)


u16 ROTL(u16 x, int n, int m);
void xor_block_128(u8* tar, const u8* src);
void xor_block(u8* tar, const u8* src, u32 len);

#endif /* WB_AES_UTIL_H_ */
