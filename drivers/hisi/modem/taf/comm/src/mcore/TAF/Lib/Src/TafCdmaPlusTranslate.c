/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and 
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may 
* *    be used to endorse or promote products derived from this software 
* *    without specific prior written permission.
* 
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/*****************************************************************************
   1 ͷ�ļ�����
*****************************************************************************/
#include "TafCdmaPlusTranslate.h"
#include "PsCommonDef.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define THIS_FILE_ID PS_FILE_ID_TAF_CDMA_PLUS_TRANSLATE_C

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)

/*****************************************************************************
  2 ȫ�ֱ�������
*****************************************************************************/
/* ��ӵ�������������10649 */
TAF_COUNTRY_DIAL_CODE_INFO g_astTafCountryList[] =
{
    {13568, 14335, 86, 460, "00", "13", "0", VOS_FALSE, ""},/* China 13568*/

    {25600, 26111, 86, 460, "00", "13", "0", VOS_FALSE, ""},/* China 25600*/

    {13568, 14335, 86, 460, "00", "14", "0", VOS_FALSE, ""},/* China 13568*/

    {25600, 26111, 86, 460, "00", "14", "0", VOS_FALSE, ""},/* China 25600*/

    {13568, 14335, 86, 460, "00", "15", "0", VOS_FALSE, ""},/* China 13568*/

    {25600, 26111, 86, 460, "00", "15", "0", VOS_FALSE, ""},/* China 25600*/

    {13568, 14335, 86, 460, "00", "17", "0", VOS_FALSE, ""},/* China 13568*/

    {25600, 26111, 86, 460, "00", "17", "0", VOS_FALSE, ""},/* China 25600*/

    {13568, 14335, 86, 460, "00", "18", "0", VOS_FALSE, ""},/* China 13568*/

    {25600, 26111, 86, 460, "00", "18", "0", VOS_FALSE, ""},/* China 25600*/

    {13568, 14335, 86, 460, "00", "10649", "0", VOS_FALSE, ""},/* China 13568*/

    {25600, 26111, 86, 460, "00", "10649", "0", VOS_FALSE, ""},/* China 25600*/

    {12288, 13311, 81, 440, "010", "", "0", VOS_FALSE, ""}, /* Japan 12288*/

    {12288, 13311, 81, 441, "010", "", "0", VOS_FALSE, ""}, /* Japan */

    {2176, 2303, 82, 450, "00700", "", "0", VOS_FALSE, ""}, /* Korea (South) */

    {11296, 11311, 853, 455, "00", "", "0", VOS_FALSE, ""}, /* Macao/Macau : 11296 */
};



/*****************************************************************************
  3 ����ʵ��
*****************************************************************************/


TAF_COUNTRY_DIAL_CODE_INFO* TAF_GetCountryListAddr(
    VOS_VOID
)
{
    return (g_astTafCountryList);
}


VOS_INT32 TAF_ConvertHomeMccToSidRange(
    VOS_UINT16                          usHomeMcc
)
{
    VOS_INT32                           lSid;
    VOS_UINT32                          i;
    TAF_COUNTRY_DIAL_CODE_INFO         *pstCountryDailCodeInfo  = VOS_NULL_PTR;

    lSid                                = TAF_INVALID_SID;
    pstCountryDailCodeInfo = TAF_GetCountryListAddr();

    for (i = 0; i < TAF_COUNTRY_LIST_LEN; i++)
    {
        if (usHomeMcc == pstCountryDailCodeInfo[i].usMcc)
        {
            lSid    = (VOS_INT32)pstCountryDailCodeInfo[i].usSidRangeStart;

            return lSid;
        }
    }

    return lSid;
}

VOS_UINT16 TAF_GetMccOfCountryList(
    VOS_UINT16                         Index
)
{
    return (g_astTafCountryList[Index].usMcc);
}


VOS_UINT16 TAF_GetCcOfCountryList(
    VOS_UINT16                         Index
)
{
    return (g_astTafCountryList[Index].usCc);
}



VOS_UINT8* TAF_GetIpcOfCountryList(
    VOS_UINT16                         Index
)
{
    return (VOS_UINT8*)(g_astTafCountryList[Index].acInterPrefixCode);
}


VOS_UINT8* TAF_GetDpcOfCountryList(
    VOS_UINT16                         Index
)
{
    return (VOS_UINT8*)(g_astTafCountryList[Index].acDialPrefixCode);
}


VOS_UINT8* TAF_GetNehOfCountryList(
    VOS_UINT16                         Index
)
{
    return (VOS_UINT8*)(g_astTafCountryList[Index].acNumEntityHeader);
}


VOS_UINT8 TAF_GetNehLenOfCountryList(
    VOS_UINT16                         Index
)
{
    return (VOS_UINT8)VOS_StrLen((VOS_CHAR*)g_astTafCountryList[Index].acNumEntityHeader);
}


VOS_UINT8 TAF_GetIpcLenOfCountryList(
    VOS_UINT16                         Index
)
{
    return (VOS_UINT8)VOS_StrLen((VOS_CHAR*)g_astTafCountryList[Index].acInterPrefixCode);
}


VOS_UINT8 TAF_GetDpcLenOfCountryList(
    VOS_UINT16                         Index
)
{
    return (VOS_UINT8)VOS_StrLen((VOS_CHAR*)g_astTafCountryList[Index].acDialPrefixCode);
}


VOS_UINT8 TAF_GetCcLenOfCountryList(
    VOS_UINT16                         Index
)
{
    VOS_UINT8                           ucCcLen;
    VOS_UINT16                          usCountryListCc;
    VOS_INT8                            acCountryListCc[TAF_COUNTRY_CODE_MAX_LEN];

    usCountryListCc  =TAF_GetCcOfCountryList(Index);

    PS_MEM_SET(acCountryListCc, 0, sizeof(acCountryListCc));
    VOS_sprintf((VOS_CHAR*)acCountryListCc, "%d", usCountryListCc);
    ucCcLen          = (VOS_UINT8)VOS_StrLen((VOS_CHAR*)acCountryListCc);

    return ucCcLen;
}



VOS_VOID TAF_GetInfoInCountryList(
    VOS_INT32                           lSid,
    VOS_UINT16                         *pusMccFirstMatchEntryIndex,
    VOS_UINT16                         *pusMccMatchEntryNum
)
{
    VOS_UINT16                          i;
    VOS_UINT16                          usFirstMatchEntryIndex;
    VOS_UINT16                          usMatchEntryNum;
    TAF_COUNTRY_DIAL_CODE_INFO         *pstCountryDailCodeInfo  = VOS_NULL_PTR;

    usFirstMatchEntryIndex = 0;
    usMatchEntryNum        = 0;
    pstCountryDailCodeInfo = TAF_GetCountryListAddr();

    for (i = 0; i < TAF_COUNTRY_LIST_LEN; i++)
    {
        if ((lSid >= (VOS_INT32)pstCountryDailCodeInfo[i].usSidRangeStart)
         && (lSid <= (VOS_INT32)pstCountryDailCodeInfo[i].usSidRangeEnd))
        {
            if (0 == usMatchEntryNum)
            {
                usFirstMatchEntryIndex = i;
            }

            usMatchEntryNum = i - usFirstMatchEntryIndex + 1;
        }
    }

    if (0 != usMatchEntryNum)
    {
        *pusMccFirstMatchEntryIndex = usFirstMatchEntryIndex;
        *pusMccMatchEntryNum        = usMatchEntryNum;

    }

    return;
}


VOS_UINT32 TAF_ConvertASCIINumToUint(
    VOS_UINT8                           ucstartDigit,
    VOS_UINT8                           ucDigitsLen,
    VOS_UINT8                          *pucAsciiNum
)
{
    VOS_UINT8                           i;
    VOS_UINT8                           ucDigit;
    VOS_UINT32                          ulValue;

    ulValue = 0;
    ucDigit = 0;

    for (i = ucstartDigit; i < ucDigitsLen; i++)
    {
        ucDigit = (VOS_UINT8)(pucAsciiNum[i] - '0');

        ulValue = ulValue*10 + ucDigit;
    }

    return ulValue;

}


VOS_VOID TAF_InsertDigitsInNumHeader(
    VOS_UINT8                          *pucInsertDigits,
    VOS_UINT8                           ucInsertDigitsLen,
    VOS_UINT8                          *pucAsciiNum,
    VOS_UINT8                          *pucNumLen
)
{
    VOS_UINT8                          ucNumLen;

    ucNumLen    = *pucNumLen;

    if ((ucInsertDigitsLen + ucNumLen) > (TAF_MO_MAX_ASCII_NUM_LEN + 1))
    {
        return;
    }

    /*������� */
    PS_MEM_MOVE(&(pucAsciiNum[ucInsertDigitsLen]),
                &(pucAsciiNum[0]),
                (ucNumLen * sizeof(VOS_UINT8)));

    /*������� */
    PS_MEM_CPY(pucAsciiNum, pucInsertDigits, ucInsertDigitsLen);

    /*���º��볤�� */
    ucNumLen  += ucInsertDigitsLen;

    *pucNumLen = ucNumLen;

    return;
}


VOS_VOID TAF_SkipDigitsOfNumHeader(
    VOS_UINT8                          *pucAsciiNum,
    VOS_UINT8                          *pucNumLen,
    VOS_UINT8                           ucSkipDigitsLen
)
{
    VOS_UINT8                           ucNumLen;

    ucNumLen    = *pucNumLen;

    PS_MEM_MOVE(&(pucAsciiNum[0]),
                &(pucAsciiNum[ucSkipDigitsLen]),
                (ucNumLen - ucSkipDigitsLen) * sizeof(VOS_UINT8));

    /*ʣ��λ���� */
    PS_MEM_SET(&(pucAsciiNum[ucNumLen - ucSkipDigitsLen]), 0, ucSkipDigitsLen);

    /*���º��볤�� */
    ucNumLen   -= ucSkipDigitsLen;

    *pucNumLen  = ucNumLen;

    return;
}



VOS_UINT8 TAF_IsNumEntityHeaderInCountryList(
    VOS_UINT16                          usMccFirstMatchEntryIndex,
    VOS_UINT16                          usMccMatchEntryNum,
    VOS_UINT8                          *pucAsciiNum
)
{
    VOS_UINT16                          i;
    VOS_UINT32                          ulCalledNumEntityHeader;
    VOS_UINT32                          ulCountryListNumEntityHeader;
    VOS_UINT8                           ucNumEntityHeaderLen;
    VOS_UINT8                          *pucNumEntityHeader = VOS_NULL_PTR;

    /*��country list �У�һ��mccֻ���Ӧһ�� ipc �� dpc �������ܶ�Ӧ�������ʵ��ͷ  */

    /*��� ����ʵ��ͷ���Ƿ���country list ��*/
    for (i = usMccFirstMatchEntryIndex; i < (usMccFirstMatchEntryIndex + usMccMatchEntryNum); i++)
    {
        /*��ȡ ����ʵ��ͷ�� ��Ӧռ�ݵĳ��� */
        pucNumEntityHeader    = TAF_GetNehOfCountryList(i);

        ucNumEntityHeaderLen  = TAF_GetNehLenOfCountryList(i);

        if (0 == ucNumEntityHeaderLen)
        {
            return VOS_FALSE;
        }

        /*��ȡcountry list�У�ƥ��MCC�ļ�¼�У� ����ʵ��ͷ�� ��ֵ */
        ulCountryListNumEntityHeader = TAF_ConvertASCIINumToUint(0, ucNumEntityHeaderLen, (VOS_UINT8 *)pucNumEntityHeader);

        /*���ݺ���ʵ��ͷ����λ����������к����и�ֵ */
        ulCalledNumEntityHeader      = TAF_ConvertASCIINumToUint(0, ucNumEntityHeaderLen, pucAsciiNum);

        if (ulCountryListNumEntityHeader == ulCalledNumEntityHeader)
        {

            return VOS_TRUE;
        }
    }

    return VOS_FALSE;

}




VOS_UINT8 TAF_IsIpcAndCcInCountryList(
    VOS_UINT16                          usMccFirstMatchEntryIndex,
    VOS_UINT8                          *pucAsciiNum
)
{
    VOS_UINT32                          ulNumIpc;
    VOS_UINT16                          usNumCc;
    VOS_UINT32                          ulCountryListIpc;
    VOS_UINT8                           ucIpcLen;
    VOS_UINT16                          usCountryListCc;
    VOS_UINT8                           ucCcLen;

    VOS_UINT8                          *pucCountryListIpc = VOS_NULL_PTR;

    /*���ƥ�䣬����к���Ĺ���Ϊ: ipc + cc + ʣ����� ,
      country list�У�һ��mcc ֻ���Ӧһ�� ipp + cc   */

    /*��ȡcountry list�У� cc ��ֵ */
    usCountryListCc  =TAF_GetCcOfCountryList(usMccFirstMatchEntryIndex);

    /*����country list�У�ipc �� cc ��ֵ�ĳ��ȣ��жϺ��к����У�
      ������뿪ͷҲ��ipc + cc ��ɣ�ipc �� cc Ӧ��ռ�ݵ�λ��    */
    ucIpcLen         = TAF_GetIpcLenOfCountryList(usMccFirstMatchEntryIndex);

    ucCcLen          = TAF_GetCcLenOfCountryList(usMccFirstMatchEntryIndex);

    pucCountryListIpc = TAF_GetIpcOfCountryList(usMccFirstMatchEntryIndex);

    /*��ȡcountry list�У�ipc ��ֵ */
    ulCountryListIpc = TAF_ConvertASCIINumToUint(0, ucIpcLen, pucCountryListIpc);


    /* ����ipc��ccռ�ݵ�λ����������к����и�ֵ */
    ulNumIpc         = TAF_ConvertASCIINumToUint(0, ucIpcLen, pucAsciiNum);
    usNumCc          = (VOS_UINT16)TAF_ConvertASCIINumToUint(ucIpcLen, (ucCcLen + ucIpcLen), pucAsciiNum);

    /* ipc + cc ƥ�䣬 */
    if ((ulNumIpc == ulCountryListIpc)
     && (usNumCc  == usCountryListCc))
    {
        return VOS_TRUE;
    }
    else
    {
        return VOS_FALSE;
    }

}


VOS_UINT8 TAF_IsCcInCountryList(
    VOS_UINT16                          usMccFirstMatchEntryIndex,
    VOS_UINT8                          *pucAsciiNum
)
{
    VOS_UINT16                          usNumCc;
    VOS_UINT16                          usCountryListCc;
    VOS_UINT8                           ucCcLen;

    usCountryListCc     = TAF_GetCcOfCountryList(usMccFirstMatchEntryIndex);

    ucCcLen             = TAF_GetCcLenOfCountryList(usMccFirstMatchEntryIndex);

    usNumCc             = (VOS_UINT16)TAF_ConvertASCIINumToUint(0, ucCcLen, pucAsciiNum);

    if (usCountryListCc == usNumCc)
    {
        return VOS_TRUE;
    }
    else
    {
        return VOS_FALSE;
    }

}



VOS_UINT8 TAF_TranslatePlusToNationalNum(
    VOS_UINT16                          usMccFirstMatchEntryIndex,
    VOS_UINT16                          usMccMatchEntryNum,
    VOS_UINT8                          *pucCurASCIINum,
    VOS_UINT8                          *pucCurNumLen
)
{

    VOS_UINT8                           ucIpcLen;
    VOS_UINT8                           ucCcLen;
    VOS_UINT8                          *pucNumDpc = VOS_NULL_PTR;
    VOS_UINT8                           ucDpcLen;

    /*��⿪ͷ�ĺ����Ƿ���IPC+CC,����ǣ�ֱ������IPC + CC�������棬������ǣ��˳������벻���޸� */
    if (VOS_TRUE == TAF_IsIpcAndCcInCountryList(usMccFirstMatchEntryIndex, pucCurASCIINum))
    {
        /*���� ipc + cc λ */
        ucIpcLen         = TAF_GetIpcLenOfCountryList(usMccFirstMatchEntryIndex);
        ucCcLen          = TAF_GetCcLenOfCountryList(usMccFirstMatchEntryIndex);

        TAF_SkipDigitsOfNumHeader(pucCurASCIINum, pucCurNumLen, (VOS_UINT8)(ucIpcLen + ucCcLen));

    }
    else
    {
        return VOS_FALSE;
    }

    /*������ʵ���ͷ���Ƿ��ڹ����б��� */
    if (VOS_FALSE == TAF_IsNumEntityHeaderInCountryList(usMccFirstMatchEntryIndex, usMccMatchEntryNum, pucCurASCIINum))
    {

         /*����ʵ��ͷ������country list �У���dpc���뵽����ͷ�� */
         pucNumDpc   = TAF_GetDpcOfCountryList(usMccFirstMatchEntryIndex);
         ucDpcLen    = TAF_GetDpcLenOfCountryList(usMccFirstMatchEntryIndex);

         TAF_InsertDigitsInNumHeader(pucNumDpc, ucDpcLen, pucCurASCIINum, pucCurNumLen);
    }

    return VOS_TRUE;

}


VOS_VOID TAF_TranslatePlusToInternationalNum(
    VOS_UINT16                          usMccFirstMatchEntryIndex,
    VOS_UINT16                          usMccMatchEntryNum,
    VOS_UINT8                          *pucCurASCIINum,
    VOS_UINT8                          *pucCurNumLen,
    TAF_PLUS_TRANSLATE_DIAL_TYPE_ENUM_U8                    enDialType
)
{
    VOS_UINT8                           ucCcLen;
    VOS_UINT8                          *pucNumIpc = VOS_NULL_PTR;
    VOS_UINT8                           ucIpcLen;
    VOS_UINT8                          *pucNumDpc = VOS_NULL_PTR;
    VOS_UINT8                           ucDpcLen;

    if ((VOS_TRUE == TAF_IsCcInCountryList(usMccFirstMatchEntryIndex, pucCurASCIINum))
    &&  (TAF_PLUS_TRANSLATE_DIAL_TYPE_SMS != enDialType))
    {
        /*���, ���ӺŵĹ��ں��У����������� */
        ucCcLen = TAF_GetCcLenOfCountryList(usMccFirstMatchEntryIndex);

        TAF_SkipDigitsOfNumHeader(pucCurASCIINum, pucCurNumLen, ucCcLen);

        /*���������ں�������ִ��*/
        if (VOS_FALSE == TAF_IsNumEntityHeaderInCountryList(usMccFirstMatchEntryIndex, usMccMatchEntryNum, pucCurASCIINum))
        {
             /*����ʵ��ͷ������country list �У���dpc���뵽����ͷ�� */
             pucNumDpc   = TAF_GetDpcOfCountryList(usMccFirstMatchEntryIndex);
             ucDpcLen    = TAF_GetDpcLenOfCountryList(usMccFirstMatchEntryIndex);

             TAF_InsertDigitsInNumHeader(pucNumDpc, ucDpcLen, pucCurASCIINum, pucCurNumLen);
        }
    }
    else
    {
        /*��ͬ�����ӺŵĹ��ʺ� ������IPC�����պ����ʽ�� ipc + cc + ����ʵ�� */
        pucNumIpc   = TAF_GetIpcOfCountryList(usMccFirstMatchEntryIndex);
        ucIpcLen    = TAF_GetIpcLenOfCountryList(usMccFirstMatchEntryIndex);

        TAF_InsertDigitsInNumHeader(pucNumIpc, ucIpcLen, pucCurASCIINum, pucCurNumLen);
    }

    return;
}


VOS_VOID TAF_TranslatePlusToNum(
    VOS_INT32                                               lSid,
    TAF_PLUS_TRANSLATE_DIAL_TYPE_ENUM_U8                    enDialType,
    TAF_PLUS_TRANSLATE_NUM_TYPE_ENUM_U8                    *penNumType,
    VOS_UINT8                                              *pucAsciiNum,
    VOS_UINT8                                              *pucNumLen
)
{
    /**************************************************************************************************

    1�����й����У�����������ҵ���Ƕ���ҵ����ǰһ�׶εĴ�������У�������Ӻţ������Ӻ�ȥ����
    ͨ�����������͸�Ϊ������������ʾ֮ǰ������мӺţ�

    2�����ʺ������ʹ���:
    2.1��������к����е�Cc������б��У�Mcc��Ӧ��CC��ͬ�������ӺŵĹ��ں���,ȥ��CC���ٸ���neh
    �Ƿ�������б���Mcc��Ӧ����ƥ�䣬������ڲ�ƥ�䣬��Ҫ����npc
    (exp +8613888888888 ת��Ϊ 13888888888��+86 21 888888888 ת��Ϊ 021 888888888)

    2.2��������к����е�Cc������б��У�Mcc��Ӧ��CC��ͬ�������ʺ��У����պ��к���ṹ:"ipc" + "cc" + "neh" + "ʣ�����"
    (exp �����û������ձ��û� +81xxxxxxx ת��Ϊ 0081xxxxxxx)

    3�����ں������ʹ���
    3.1�������������Ϊ"ipc" + "cc" + "neh" + "ʣ�����"��ת��Ϊ"ipc/npc(�ж�ͬ2.1)"+"neh" + "ʣ�����"
    (exp 008613888888888 ת��Ϊ 0013888888888��008614888888888 ת��Ϊ 014888888888)

    3.2���������벻���ı�
    ****************************************************************************************************/


    VOS_UINT16                                              usMccFirstMatchEntryIndex;
    VOS_UINT16                                              usMccMatchEntryNum;
    VOS_UINT8                                               aucAsciiNum[TAF_MO_MAX_ASCII_NUM_LEN + 1];
    VOS_UINT8                                               ucNumLen;
    TAF_PLUS_TRANSLATE_NUM_TYPE_ENUM_U8                     enNumType;

    /*���mcc �Ƿ��� countrylist,����ƥ��ļ�¼�������Լ���һ����¼������ */
    usMccFirstMatchEntryIndex = 0;
    usMccMatchEntryNum        = 0;

    /*��ȡƥ���Sid��Ϣ */
    TAF_GetInfoInCountryList(lSid, &usMccFirstMatchEntryIndex, &usMccMatchEntryNum);

    /*û��ƥ���¼ */
    if (0 == usMccMatchEntryNum)
    {
        return;
    }


    /* ��ȡ���к��볤�� */
    ucNumLen                  = *pucNumLen;

    /* ��ȡ��������*/
    enNumType                 = *penNumType;

    /*��ȡ���к��� */
    PS_MEM_SET(aucAsciiNum, 0, sizeof(aucAsciiNum));
    PS_MEM_CPY(aucAsciiNum, pucAsciiNum, sizeof(aucAsciiNum));

    if (TAF_PLUS_TRANSLATE_NUM_INTERNATIONAL == enNumType)
    {

        /*���ʺ��д��� */
        TAF_TranslatePlusToInternationalNum(usMccFirstMatchEntryIndex, usMccMatchEntryNum, aucAsciiNum, &ucNumLen, enDialType);
    }
    else
    {
        if (VOS_FALSE == TAF_TranslatePlusToNationalNum(usMccFirstMatchEntryIndex, usMccMatchEntryNum, aucAsciiNum, &ucNumLen))
        {
            return;
        }
    }

    /*�����ɼӺ�ת�������Ӻ��Ѿ���ȥ�������к������͸���Ϊ���ں������� */
    enNumType                 = TAF_PLUS_TRANSLATE_NUM_NATIONAL;

    /*��ת����ĺ�����Ϣд�� */
    PS_MEM_CPY(pucAsciiNum, aucAsciiNum, sizeof(aucAsciiNum));
    *pucNumLen                = ucNumLen;
    *penNumType               = enNumType;

    return;

}

#if 0

VOS_UINT8 TAF_IsIpcInCountryList(
    VOS_UINT16                          usMccFirstMatchEntryIndex,
    VOS_UINT8                          *pucAsciiNum
)
{
    VOS_UINT32                          ulNumIpc;
    VOS_UINT32                          ulCountryListIpc;
    VOS_UINT8                           ucIpcLen;
    VOS_UINT8                          *pucCountryListIpc = VOS_NULL_PTR;

    pucCountryListIpc = TAF_GetIpcOfCountryList(usMccFirstMatchEntryIndex);

    ucIpcLen          = TAF_GetIpcLenOfCountryList(usMccFirstMatchEntryIndex);

    ulCountryListIpc  = TAF_ConvertASCIINumToUint(0, ucIpcLen, pucCountryListIpc);

    ulNumIpc          = TAF_ConvertASCIINumToUint(0, ucIpcLen, pucAsciiNum);

    if (ulCountryListIpc == ulNumIpc)
    {

        return VOS_TRUE;
    }
    else
    {
        return VOS_FALSE;
    }

}

VOS_UINT8 TAF_TranslateInternationalNumToPlus(
    VOS_UINT16                                              usMccFirstMatchEntryIndex,
    VOS_UINT8                                              *pucAsciiNum,
    VOS_UINT8                                              *pucNumLen,
    TAF_PLUS_TRANSLATE_DIAL_TYPE_ENUM_U8                    enDialType
)
{
    VOS_UINT8                                               ucIpcLen;
    VOS_UINT8                                               ucNumLen;

    /*ƥ����뿪ͷ��ipc�Ƿ��ڹ����б��� */
    if (VOS_TRUE == TAF_IsIpcInCountryList(usMccFirstMatchEntryIndex, pucAsciiNum))
    {
        /*ipc ƥ�䣬����ipcλ */
        ucIpcLen = TAF_GetIpcLenOfCountryList(usMccFirstMatchEntryIndex);

        TAF_SkipDigitsOfNumHeader(pucAsciiNum, pucNumLen, ucIpcLen);
    }

    /*���ź��� */
    if (TAF_PLUS_TRANSLATE_DIAL_TYPE_SMS == enDialType)
    {
        /*����Ӻ��ں���ͷ�� */
        if ((*pucNumLen) >= (TAF_MT_MAX_ASCII_NUM_LEN + 1))
        {
            return VOS_FALSE;
        }

        ucNumLen = *pucNumLen;

        /*������� */
        PS_MEM_MOVE(&(pucAsciiNum[1]),
                    &(pucAsciiNum[0]),
                    (ucNumLen * sizeof(VOS_UINT8)));

        pucAsciiNum[0] = '+';

        ucNumLen++;

        *pucNumLen = ucNumLen;

        return VOS_TRUE;
    }
    else
    {
        return VOS_FALSE;
    }

}


VOS_UINT8 TAF_TranslateNationalNumToPlus(
    VOS_UINT16                                              usMccFirstMatchEntryIndex,
    VOS_UINT8                                              *pucAsciiNum,
    VOS_UINT8                                              *pucNumLen,
    TAF_PLUS_TRANSLATE_NUM_TYPE_ENUM_U8                    *penNumType
)
{
    VOS_UINT8                                               ucIpcLen;

    if (VOS_TRUE == TAF_IsIpcInCountryList(usMccFirstMatchEntryIndex, pucAsciiNum))
    {
        /*ipc ƥ�䣬����ipcλ */
        ucIpcLen          = TAF_GetIpcLenOfCountryList(usMccFirstMatchEntryIndex);

        TAF_SkipDigitsOfNumHeader(pucAsciiNum, pucNumLen, ucIpcLen);

        /*���º�������Ϊ���ʺ��� */
        *penNumType = TAF_PLUS_TRANSLATE_NUM_INTERNATIONAL;

        return VOS_TRUE;
    }
    else
    {
        return VOS_FALSE;
    }

}


VOS_VOID TAF_TranslateNumToPlus(
    VOS_UINT16                                              usCurMcc,
    TAF_PLUS_TRANSLATE_DIAL_TYPE_ENUM_U8                    enDialType,
    TAF_PLUS_TRANSLATE_NUM_TYPE_ENUM_U8                    *penNumType,
    VOS_UINT8                                              *pucAsciiNum,
    VOS_UINT8                                              *pucNumLen
)
{
    /**************************************************************************************************

    1�����ʺ������ʹ���:
    1.1��"ipc" + "cc" +����,����ҵ��ת��Ϊ"cc" +����
    1.2��"ipc" + "cc" +����,����ҵ��ת��Ϊ"+" + "cc" + ����
    2�����ں������ʹ���:
    �κ�ҵ��"ipc" + "cc" +����,����ҵ��ת��Ϊ"cc" +����
    3���������벻���޸�
    ****************************************************************************************************/

    VOS_UINT16                                              usMccFirstMatchEntryIndex;
    VOS_UINT16                                              usMccMatchEntryNum;
    VOS_UINT8                                               aucAsciiNum[TAF_MT_MAX_ASCII_NUM_LEN + 1];
    VOS_UINT8                                               ucNumLen;
    TAF_PLUS_TRANSLATE_NUM_TYPE_ENUM_U8                     enNumType;

    /*���mcc �Ƿ��� countrylist,����ƥ��ļ�¼�������Լ���һ����¼������ */
    usMccFirstMatchEntryIndex = 0;
    usMccMatchEntryNum        = 0;

    /*��ȡƥ���MCC��Ϣ */
    TAF_GetMccInfoInCountryList(usCurMcc, &usMccFirstMatchEntryIndex, &usMccMatchEntryNum);

    /*û��ƥ���¼ */
    if (0 == usMccMatchEntryNum)
    {
        return;
    }

    /* ��ȡ���к��볤�� */
    ucNumLen  = *pucNumLen;

    /* ��ȡ��������*/
    enNumType = *penNumType;

    /*��ȡ���к��� */
    PS_MEM_SET(aucAsciiNum, 0, sizeof(aucAsciiNum));
    PS_MEM_CPY(aucAsciiNum, pucAsciiNum, sizeof(aucAsciiNum));

    if (TAF_PLUS_TRANSLATE_NUM_INTERNATIONAL == enNumType)
    {
        /*����ҵ�� */
        if (VOS_FALSE == TAF_TranslateInternationalNumToPlus(usMccFirstMatchEntryIndex, aucAsciiNum, &ucNumLen, enDialType))
        {
            return;
        }
    }
    else
    {
        /*����ҵ�� */
        if (VOS_FALSE == TAF_TranslateNationalNumToPlus(usMccFirstMatchEntryIndex, aucAsciiNum, &ucNumLen, &enNumType))
        {
            return;
        }
    }

    /*��ת����ĺ�����Ϣд�� */
    PS_MEM_CPY(pucAsciiNum, aucAsciiNum, sizeof(aucAsciiNum));
    *pucNumLen   = ucNumLen;
    *penNumType  = enNumType;

    return;
}


#endif


VOS_UINT32 TAF_QueryMccWithSid(
    VOS_INT32       lSid
)
{
    VOS_UINT32                          i;

    for (i = 0; i < TAF_COUNTRY_LIST_LEN; i++)
    {
        if ((lSid >= g_astTafCountryList[i].usSidRangeStart)
         && (lSid <= g_astTafCountryList[i].usSidRangeEnd))
        {
            return (VOS_UINT32)g_astTafCountryList[i].usMcc;
        }
    }

    return TAF_MCORE_INVALID_MCC;
}


#endif

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


