
#ifndef _HW_KICONCOVERT_H
#define _HW_KICONCOVERT_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define CONVT_OK                      0
#define CONVT_ERROR                   (-1)

#define ATP_ENCODING_STR_MAX_LEN_D          (32)
typedef struct NlsIoctParms
{
    int type;
    int status;
    const char *pcInputStr;
    int inputLen;
    char acInputEncoding[ATP_ENCODING_STR_MAX_LEN_D];
    char acOutputEncoding[ATP_ENCODING_STR_MAX_LEN_D];
    int outputLen;
    char *pcOutputStr;
} NLS_IOCTL_PARMS_ST;


typedef struct ConvtIoctParms
{
    char *pcString;
    char *pcBuf;
    unsigned int   ulType;
    unsigned int   ulStrLen;
    unsigned int   ulOffset;  
    int   lResult; 
} CONVT_IOCTL_PARMS_ST;


#define CONVERT_PROC "convert"
#define IOCTL_CONVERT_MAGIC             'G'
#define IOCTL_ICON_CONVERT  _IOWR(IOCTL_CONVERT_MAGIC, 10, CONVT_IOCTL_PARMS_ST)


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif
