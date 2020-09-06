#include <linux/nls.h>

//#define KNSL_DEBUG printk
#define KNSL_DEBUG(x...)

// For linux kernel 2.6.21, which has not defined nls_nullsize
#ifndef nls_nullsize
#define nls_nullsize(x ...) 1
#endif

static int HwNlsGetTargetLen(const NLS_IOCTL_PARMS_ST *pstParams, const struct nls_table *src_codepage,
            const struct nls_table *dst_codepage, int *bSuccess)
{
    int i;
    int charlen;
    int outlen;
    wchar_t tmp_wchar;
    int src_bound;
    const unsigned char* pucSrc;

    char tmp[NLS_MAX_CHARSET_SIZE];

    (*bSuccess) = 0;

    KNSL_DEBUG("Calculating converted length now ...\n");

    outlen = 0;
    pucSrc = (const unsigned char*)pstParams->pcInputStr;
    for (i = 0; i < pstParams->inputLen;)
    {
        KNSL_DEBUG("Calculating [%x] with idx [%d] now ...\n", pucSrc[i], i);

        // Convert each char to wchar
        src_bound = (pstParams->inputLen - i);
        if (src_bound > NLS_MAX_CHARSET_SIZE)
        {
            src_bound = NLS_MAX_CHARSET_SIZE;
        }
        charlen = src_codepage->char2uni(pucSrc + i, src_bound, &tmp_wchar);
        KNSL_DEBUG("char2uni return %d.\n", charlen);
        if (charlen > 0)
        {
            i += charlen;
        }
        else
        {
            i++;
            tmp_wchar = 0x003f;
            (*bSuccess) = 1;
            KNSL_DEBUG("char2uni failed return %d for the %dth input.\n", charlen, i);
        }

        // Convert wchar to target encoding
        charlen = dst_codepage->uni2char(tmp_wchar, tmp, NLS_MAX_CHARSET_SIZE);
        KNSL_DEBUG("uni2char return %d.\n", charlen);
        if (charlen > 0)
        {
            outlen += charlen;
        }
        else
        {
		    KNSL_DEBUG("uni2char failed return %d for the %dth output, %dth input %d.\n", charlen, outlen, i);
            outlen++;
        }
    }

    outlen += nls_nullsize(dst_codepage);

    KNSL_DEBUG("Final converted length is [%d], status [%d].\n", outlen, (*bSuccess));
    return outlen;
}

static int HwNlsConvert(NLS_IOCTL_PARMS_ST *pstParams, const struct nls_table *src_codepage,
            const struct nls_table *dst_codepage)
{
    int i;
    int charlen;
    int outlen;
    wchar_t tmp_wchar;
    int src_bound;
    const unsigned char *pucSrc;
    unsigned char *pucDst;
    unsigned char *pucDstPtr;

    // Malloc
    pucDst = kmalloc(pstParams->outputLen, GFP_KERNEL);
    if (NULL == pucDst)
    {
        return -1;
    }

    KNSL_DEBUG("Do real converting with target memory length %d now ...\n", pstParams->outputLen);

    // Try convert
    pucDstPtr = pucDst;
    outlen = 0;
    pucSrc = (const unsigned char *)pstParams->pcInputStr;
    for (i = 0; (i < pstParams->inputLen) && (outlen < pstParams->outputLen);)
    {
        KNSL_DEBUG("Converting [%x] with idx [%d] now ...\n", pucSrc[i], i);
   
        // Convert each char to wchar
        src_bound = (pstParams->inputLen - i);
        if (src_bound > NLS_MAX_CHARSET_SIZE)
        {
            src_bound = NLS_MAX_CHARSET_SIZE;
        }
        charlen = src_codepage->char2uni(pucSrc + i, src_bound, &tmp_wchar);
        KNSL_DEBUG("char2uni return %d.\n", charlen);
        if (charlen > 0)
		{
            i += charlen;
        }
		else
		{
            i++;
            tmp_wchar = 0x003f;
            KNSL_DEBUG("char2uni failed return %d for the %dth input.\n", charlen, i);
        }

        // Convert wchar to target encoding
        charlen = dst_codepage->uni2char(tmp_wchar, pucDstPtr, NLS_MAX_CHARSET_SIZE);
        KNSL_DEBUG("uni2char return %d.\n", charlen);
        if (charlen > 0)
		{
		    pucDstPtr += charlen;
		    outlen += charlen;
        }
        else
        {
            KNSL_DEBUG("uni2char failed return %d for the %dth output, %dth input %d.\n", charlen, outlen, i);
        }
    }

    // Fill with null terminated
    charlen = nls_nullsize(dst_codepage);
    for (i = 0; (i < charlen) && (outlen < pstParams->outputLen); i++)
    {
        *pucDstPtr = 0;
        outlen++;
    }

    // Copy to user
    if(copy_to_user(pstParams->pcOutputStr, pucDst, outlen))
    {
        kfree(pucDst);
        return -1;
    }

    KNSL_DEBUG("Final converted length is [%d], input length is [%d].\n", outlen, pstParams->outputLen);
    pstParams->outputLen = outlen - charlen;

    kfree(pucDst);
    return CONVT_OK;
}

static int HwNlsIoctlProc(NLS_IOCTL_PARMS_ST *pstParams)
{
    struct nls_table *src_codepage;
    struct nls_table *dst_codepage;
    int success;

    if (NULL == pstParams->pcInputStr)
    {
        return -EINVAL;
    }

    src_codepage = load_nls(pstParams->acInputEncoding);
    dst_codepage = load_nls(pstParams->acOutputEncoding);

    if ((NULL == src_codepage) || (NULL == dst_codepage))
    {
        return -EINVAL;
    }

    // Try to get converted length, and check if can be converted successfully.
    if (0 != pstParams->type)
    {
        success = 0;
        pstParams->outputLen = HwNlsGetTargetLen(pstParams, src_codepage, dst_codepage, &success);
        pstParams->status = success;
        return success;
    }

    if ((pstParams->outputLen <= 0) || (NULL == pstParams->pcOutputStr))
    {
        return -EINVAL;
    }

    // Do convert
    return HwNlsConvert(pstParams, src_codepage, dst_codepage);
}

static int HwNlsConvertIoCtl(unsigned long arg)
{
    int                     iRet;
    NLS_IOCTL_PARMS_ST      stCtrlParms;
    char                    *pcInputBuf;
    char                    *pcTmpStr;

    KNSL_DEBUG("HwNlsConvertIoCtl start now ...arg =%x\n",arg);

    iRet = copy_from_user((void*)&stCtrlParms, (void*)arg, sizeof(stCtrlParms));
    if (0 != iRet)
    {
        KNSL_DEBUG("\n copy_from_user iRet =%x\n",iRet);
        return CONVT_ERROR;
    }

    // Copy input string
    pcTmpStr = (char *)kmalloc((stCtrlParms.inputLen + 1), GFP_KERNEL);
    if (NULL == pcTmpStr)
    {
        return CONVT_ERROR;
    }
    iRet = copy_from_user((void*)pcTmpStr, (void*)stCtrlParms.pcInputStr, stCtrlParms.inputLen);
    if (0 != iRet)
    {
        kfree(pcTmpStr);
        return CONVT_ERROR;
    }
    pcTmpStr[stCtrlParms.inputLen] = '\0';
    pcInputBuf = (char *)stCtrlParms.pcInputStr;
    stCtrlParms.pcInputStr = pcTmpStr;

    KNSL_DEBUG("hwnls get input from user space: acInputEncoding [%s], pcInputStr [%s], inputLen [%d], acOutputEncoding [%s], outputLen [%d] type [%d].\n",
               stCtrlParms.acInputEncoding, stCtrlParms.pcInputStr, stCtrlParms.inputLen,
               stCtrlParms.acOutputEncoding, stCtrlParms.outputLen, stCtrlParms.type);

    iRet = HwNlsIoctlProc(&stCtrlParms);
    if (0 != iRet)
    {
        kfree(pcTmpStr);
        return iRet;
    }

    stCtrlParms.pcInputStr = pcInputBuf;    // Restore
    if(copy_to_user((NLS_IOCTL_PARMS_ST*)arg, &stCtrlParms, sizeof(NLS_IOCTL_PARMS_ST)))
    {
        kfree(pcTmpStr);
        return -1;
    }
    kfree(pcTmpStr);

    KNSL_DEBUG("HwNlsConvertIoCtl finished with %d.\n", iRet);
    return CONVT_OK;
}

