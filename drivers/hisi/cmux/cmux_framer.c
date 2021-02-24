

// Cmux common includes
#include "cmux_framer.h"
#include "cmux_cmd.h"
#include "cmux_api_ifc.h"
#include "cmux_tgt_osal.h"

#ifndef WIN32
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#endif /* WIN32 */

#if ( FEATURE_ON == MBB_HSUART_CMUX )
/* HUAWEI DRIVER DEV GROUP<m2m-1594,1593,1580><proper NSC response format><24-march>*/
unsigned char *g_nsc_frm = 0;
unsigned int g_nsc_received = 0;

#define NO_ACCEPT_FLAGS 0xFFFFFFFF
#define NO_REJECT_FLAGS 0x00000000

/*lint -e133 -e43*/
const struct framedetails UE_frame_details [] = {
    { CMUX_CTRL_SABM_FRAME, CMUX_CTRL_SABM_FRAME_LENGTH, 0, 0, 0, 0},
    { CMUX_CTRL_UA, CMUX_CTRL_UA_FRAME_LENGTH, (UE_SENT_DISC_CMD_UA_PENDING | DLC_ESTABLISHED), DLC_CLOSED, cmux_UE_rxed_UA, 0},
    { CMUX_CTRL_DM, CMUX_CTRL_DM_FRAME_LENGTH, UE_SENT_DISC_CMD_UA_PENDING, NO_REJECT_FLAGS, cmux_TE_UE_rxed_DM, 0},
    { CMUX_CTRL_DISC, CMUX_CTRL_DISC_FRAME_LENGTH, DLC_ESTABLISHED, NO_REJECT_FLAGS, 0, 0},
    { CMUX_CTRL_UIH, 0, 0, 0, 0, 0},
    { CMUX_MUX_DLC_NEGO_PARAM_CMD, CMUX_MUX_DLC_NEGO_PARAM_LENGTH, NO_ACCEPT_FLAGS, NO_REJECT_FLAGS, cmux_UE_rxed_DLCPN_cmd, 0},/*lint !e569*/
    { CMUX_MUX_DLC_NEGO_PARAM_RESP, CMUX_MUX_DLC_NEGO_PARAM_LENGTH, 0, 0, 0, 0},
    { CMUX_MUX_POWER_SAVING_CMD, CMUX_MUX_POWER_SAVING_LENGTH, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_PSC_cmd, 0},
    { CMUX_MUX_POWER_SAVING_RESP, CMUX_MUX_POWER_SAVING_LENGTH, 0, 0, 0, 0},
    { CMUX_MUX_CLOSE_CMD, CMUX_MUX_CLOSE_CMD_LENGTH, DLC_ESTABLISHED, NO_REJECT_FLAGS, cmux_UE_rxed_CLD_cmd, 0},
    { CMUX_MUX_CLOSE_RESP, CMUX_MUX_CLOSE_CMD_LENGTH, (UE_SENT_MUX_CLD_RESP_PENDING | DLC_ESTABLISHED), DLC_CLOSED, cmux_UE_rxed_CLD_RESP, 0},
    { CMUX_MUX_TEST_CMD, CMUX_MUX_TEST_CMD_LENGTH, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_Test_cmd, 0},
    { CMUX_MUX_TEST_CMD_RESP, CMUX_MUX_TEST_CMD_LENGTH, 0, 0, 0, 0},
    { CMUX_MUX_MODEM_STATUS_CMD, CMUX_MUX_MODEM_STATUS_CMD_LENGTH, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_MSC_cmd, 0},
    { CMUX_MUX_MODEM_STATUS_RESP, CMUX_MUX_MODEM_STATUS_CMD_LENGTH, 0, 0, 0, 0},
    { CMUX_MUX_NON_SUPORTED_RES, CMUX_MUX_NON_SUPORTED_RES_LENGTH, (DLC_ESTABLISHED | DLC_CLOSED), NO_REJECT_FLAGS, cmux_UE_NSC_handler, 0},
    { CMUX_MUX_SERVICE_NEGO_CMD, CMUX_MUX_SERVICE_NEGO_LENGTH, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_SNC_cmd, 0},
    { CMUX_MUX_SERVICE_NEGO_RESP, CMUX_MUX_SERVICE_NEGO_LENGTH, 0, 0, 0, 0},
    { CMUX_DATA_UIH, 0, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_UIHDATA, 0},
    { CMUX_FCON_CMD, CMUX_FCON_LENGTH, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_FCON_cmd, 0},
    { CMUX_FCON_RESP, CMUX_FCON_LENGTH, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_FCON_FCOFF_RESP, 0},
    { CMUX_FCOFF_CMD, CMUX_FCOFF_LENGTH, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_FCOFF_cmd, 0},
    { CMUX_FCOFF_RESP, CMUX_FCOFF_LENGTH, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_rxed_FCON_FCOFF_RESP, 0},
    { CMUX_RLS_CMD, 0, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_NSC_handler, 0},
    { CMUX_RLS_RESP, 0, 0, 0, 0, 0},
    { CMUX_RPN_CMD, 0, DLC_ESTABLISHED, DLC_CLOSED, cmux_UE_NSC_handler, 0},
    { CMUX_RPN_RESP, 0, 0, 0, 0, 0}
};
/*lint +e133 +e43*/

static const unsigned char crc_table [] = {
    0x00, 0x91, 0xE3, 0x72, 0x07, 0x96, 0xE4, 0x75,
    0x0E, 0x9F, 0xED, 0x7C, 0x09, 0x98, 0xEA, 0x7B,
    0x1C, 0x8D, 0xFF, 0x6E, 0x1B, 0x8A, 0xF8, 0x69,
    0x12, 0x83, 0xF1, 0x60, 0x15, 0x84, 0xF6, 0x67,
    0x38, 0xA9, 0xDB, 0x4A, 0x3F, 0xAE, 0xDC, 0x4D,
    0x36, 0xA7, 0xD5, 0x44, 0x31, 0xA0, 0xD2, 0x43,
    0x24, 0xB5, 0xC7, 0x56, 0x23, 0xB2, 0xC0, 0x51,
    0x2A, 0xBB, 0xC9, 0x58, 0x2D, 0xBC, 0xCE, 0x5F,
    0x70, 0xE1, 0x93, 0x02, 0x77, 0xE6, 0x94, 0x05,
    0x7E, 0xEF, 0x9D, 0x0C, 0x79, 0xE8, 0x9A, 0x0B,
    0x6C, 0xFD, 0x8F, 0x1E, 0x6B, 0xFA, 0x88, 0x19,
    0x62, 0xF3, 0x81, 0x10, 0x65, 0xF4, 0x86, 0x17,
    0x48, 0xD9, 0xAB, 0x3A, 0x4F, 0xDE, 0xAC, 0x3D,
    0x46, 0xD7, 0xA5, 0x34, 0x41, 0xD0, 0xA2, 0x33,
    0x54, 0xC5, 0xB7, 0x26, 0x53, 0xC2, 0xB0, 0x21,
    0x5A, 0xCB, 0xB9, 0x28, 0x5D, 0xCC, 0xBE, 0x2F,
    0xE0, 0x71, 0x03, 0x92, 0xE7, 0x76, 0x04, 0x95,
    0xEE, 0x7F, 0x0D, 0x9C, 0xE9, 0x78, 0x0A, 0x9B,
    0xFC, 0x6D, 0x1F, 0x8E, 0xFB, 0x6A, 0x18, 0x89,
    0xF2, 0x63, 0x11, 0x80, 0xF5, 0x64, 0x16, 0x87,
    0xD8, 0x49, 0x3B, 0xAA, 0xDF, 0x4E, 0x3C, 0xAD,
    0xD6, 0x47, 0x35, 0xA4, 0xD1, 0x40, 0x32, 0xA3,
    0xC4, 0x55, 0x27, 0xB6, 0xC3, 0x52, 0x20, 0xB1,
    0xCA, 0x5B, 0x29, 0xB8, 0xCD, 0x5C, 0x2E, 0xBF,
    0x90, 0x01, 0x73, 0xE2, 0x97, 0x06, 0x74, 0xE5,
    0x9E, 0x0F, 0x7D, 0xEC, 0x99, 0x08, 0x7A, 0xEB,
    0x8C, 0x1D, 0x6F, 0xFE, 0x8B, 0x1A, 0x68, 0xF9,
    0x82, 0x13, 0x61, 0xF0, 0x85, 0x14, 0x66, 0xF7,
    0xA8, 0x39, 0x4B, 0xDA, 0xAF, 0x3E, 0x4C, 0xDD,
    0xA6, 0x37, 0x45, 0xD4, 0xA1, 0x30, 0x42, 0xD3,
    0xB4, 0x25, 0x57, 0xC6, 0xB3, 0x22, 0x50, 0xC1,
    0xBA, 0x2B, 0x59, 0xC8, 0xBD, 0x2C, 0x5E, 0xCF
};


static unsigned char cmux_ts0710_compute_crc (const unsigned char *data, int len)
{
    unsigned char crc       = 0xFF;
    unsigned char crc_const = 0xFF;
    unsigned int i;

    for (i = 0; i < (unsigned int)len; i++)
    {
        crc = crc_table [crc ^ data [i]];
    }

    crc_const = crc_const - crc;
    return (crc_const);
}


static unsigned char cmux_ts0710_check_fcs (const unsigned char *data, int len,
                                            unsigned char cfcs)
{
    unsigned char crc = 0xFF;
    int i;

    for (i = 0; i < len; i++)
    {
        crc = crc_table [crc ^ data [i]];
    }

    crc = crc_table [crc ^ cfcs];

    if (crc == 0xcf)
    {
        return 0;
    }
    else
    {
        return 1;
    }
} /* cmux_ts0710_check_fcs */


int cmux_free_frame (struct frame *fr)
{
    if (NULL == fr)
    {
        return FAIL;
    }
    else
    {
        if (NULL != fr->uih_data)
        {
            cmux_free ((unsigned char *) fr->uih_data);
        }
        if (NULL != fr->raw_data)
        {
            cmux_free ((unsigned char *) fr->raw_data);
        }
        cmux_free ((unsigned char *) fr);
        /* HUAWEI DRIVER DEV GROUP<m2m-1638,1639,1641><Crash on wrong frame format,avoid dangling pointer>*/
        fr = NULL;
    }

    return PASS;
} /* cmux_free_frame */


int cmux_check_length (unsigned char *rawptr, struct frame *fr_temp)
{
    const struct framedetails *frame_details = UE_frame_details;
    int len_vl;
    
    if ((((*rawptr) & CMUX_EA_BIT_MASK) == (CMUX_EA_BIT_MASK))  )
    {
        if ((((*rawptr) >> 1) & 0x7F) != frame_details [fr_temp->f_type].frame_length)
        {
            return MUX_FRTYPE_ERR;
        }
        /* HUAWEI DRIVER DEV GROUP:M2M-1669: to send NSC if the length valud field is not correct april2 : begin */
        len_vl = (((*rawptr) >> 1) & 0x7F);
        
        if ((*(rawptr + (len_vl + 2))) != 0xF9)
        {
            return MUX_FRTYPE_ERR;
        }
        /* HUAWEI DRIVER DEV GROUP:M2M-1669: to send NSC if the length valud field is not correct april2 : end */
    }
    else
    {
        return MUX_FRTYPE_ERR;
    }
    return PASS;
} /* cmux_check_length */

/* HUAWEI DRIVER DEV GROUP:M2M-1669: to send NSC if the data field in snc and msc field is not correct april2 : begin */
int check_frame_data(struct cmuxlib *cm, unsigned char *rwptr,
                        void *cmd_struct, char cmd_type)
{
    struct snc_cmd*            snc;
    struct modem_status_cmd*   msc;
    int i;
    
    if (cmd_type == SNC_CMD)
    {
        snc = (struct snc_cmd*)cmd_struct;
        /* Service Value位中EA位必须为1，S1必须为0或1，S2-7必须为0 */
        if (snc->EA != 1)
        {
            return 0;
        }
        if (((snc->dlci) > MAX_DLC_TGT))
        {
            return 0;
        }
        if (!(((snc->s1) == 1) || (snc->s1 == 0)))
        {
            return 0;
        }
        if ((snc->s2 != 0) || (snc->s3 != 0) || (snc->s4 != 0) ||
            (snc->s5 != 0) || (snc->s6 != 0) || (snc->s7 != 0))
        {
            return 0;
        }
        /* Voice Codec Value位中EA位必须为1，V1-7必须为0 */
        if (CMUX_EA_BIT_MASK != ((*(rwptr + 1)) & CMUX_EA_BIT_MASK))
        {
            return 0;
        }
        for (i = 1; i <= 7; i++) //all other bits must be zero
        {
            if (0 != ((*(rwptr + 1)) >> i))
            {
                return 0;
            }
        }
        return 1;

    }
    if (cmd_type == MSC_CMD)
    {
        msc = (struct modem_status_cmd*)cmd_struct;
        if ((msc->dlci) > MAX_DLC_TGT)
        {
            return 0;
        }
        if (NULL == cm->dlc_db[msc->dlci])
        {
            return 0;
        }
        /* V.24 signals位中EA位必须为1，fc位必须为0或1，resrvd1-2位必须为0 */
        if (msc->EA != 1)
        {
            return 0;
        }
        if (!(((msc->fc) == 1) || ((msc->fc) == 0)))
        {
            return 0;
        }
        if ((msc->resrvd1 != 0) || (msc->resrvd2 != 0))
        {
            return 0;
        }
        return 1;

    }
    return 1;
}/* check_frame_data */
/* HUAWEI DRIVER DEV GROUP:M2M-1669: to send NSC if the data field in snc and msc field is not correct april2 : end */

unsigned int mux_frame_to_struct (unsigned char **vrawptr, 
                                    struct cmuxlib *cm, struct frame *fr_temp)
{
    int i;
    unsigned char *            rawptr;
    unsigned int rc            = PASS;
    struct dlc_param_neg *     dlcpn         = (struct dlc_param_neg *) NULL;
    struct modem_status_cmd *  msc           = (struct modem_status_cmd *) NULL;
    struct snc_cmd *           snc           = (struct snc_cmd *) NULL;
    const struct framedetails *frame_details = UE_frame_details;
    int chk_val;
    int len_vl;
    int test_len    = 0;
    int offset      = 0;
    /* HUAWEI DRIVER DEV GROUP<m2m-595><length validation to avoid crash in case of wrong length value><24-march>*/
    unsigned char len_temp;

    if (NULL == fr_temp || NULL == vrawptr)
    {
        return BASIC_FRAME_ERR;
    }
    if (NULL == *vrawptr)
    {
        return BASIC_FRAME_ERR;
    }

    rawptr            = *vrawptr;
    // we reset the value of uih-data, as it is this function responsibility to fill it
    fr_temp->uih_data = NULL;
    g_nsc_frm         = rawptr;
    do
    {
        for (i = DLC_PN_CMD; i < MAX_CMUX_FRAMES; i++)
        {
            if ((*rawptr == frame_details [i].frame_code))
            {
                fr_temp->f_type = i;
                /* 收到了NSC消息，停止流控定时器 */
                if (NSC == fr_temp->f_type)
                {
                    g_nsc_received = 1;
                }
                break;
            }
        }
        ++rawptr;
        switch (fr_temp->f_type)
        {
            case DLC_PN_CMD:
                len_temp = (((*rawptr) >> 1) & 0x7F);
                
                /* HUAWEI DRIVER DEV GROUP<m2m-595><length validation to avoid crash in case of wrong length value><24-march><Begin>*/
                if (len_temp != frame_details [fr_temp->f_type].frame_length)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                
                if ((*(rawptr + (len_temp + 2))) != 0xF9)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                /* HUAWEI DRIVER DEV GROUP<m2m-595><length validation to avoid crash in case of wrong length value><24-march><End>*/
                
                if (((*rawptr) & CMUX_EA_BIT_MASK) != (CMUX_EA_BIT_MASK))
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                
                fr_temp->uih_data = (void*) cmux_alloc (sizeof (struct dlc_param_neg));
                if (NULL == fr_temp->uih_data)
                {
                    rc = BASIC_FRAME_ERR;
                    break;
                }
                dlcpn = (struct dlc_param_neg*) fr_temp->uih_data;

                /* 00 00 D6 D5 D4 D3 D2 D1  通道号DLC */
                ++rawptr;
                if ((*rawptr > MAX_DLC_TGT) || (*rawptr == 0))
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                dlcpn->dlci = *rawptr;
                
                /* CL4 CL3 CL2 CL1 I4 I3 I2 I1  会聚类型 帧类型 */
                ++rawptr;
                if (*rawptr > 0)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                dlcpn->conv_layer = ((*rawptr) >> 4) & 0x0F;
                
                /* 00 00 P6 P5 P4 P3 P2 P1  优先级 */
                ++rawptr;
                dlcpn->priority = *rawptr & 0x3F;
                if (dlcpn->priority > MAX_PRIORITY)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                
                /* T8 T7 T6 T5 T4 T3 T2 T1  确认定时器T1 */
                ++rawptr;
                if (*rawptr > MAX_ACK_TIMER_T1)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                dlcpn->muxparams.ack_timer_t1 = *rawptr;

                /* N8  N7  N6  N5  N4  N3  N2  N1  
                   N16 N15 N14 N13 N12 N11 N10 N9  最大帧长N1*/
                ++rawptr;
                dlcpn->muxparams.frame_size_n1 = ((*rawptr) & 0xFF);
                ++rawptr;
                dlcpn->muxparams.frame_size_n1 |= (*rawptr) << 8;
                if (dlcpn->muxparams.frame_size_n1 > CMUX_MAXIMUM_FRAME_SIZE)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }

                /* NA8 NA7 NA6 NA5 NA4 NA3 NA2 NA1  最大重传次数N2 */
                ++rawptr;
                /* HUAWEI DRIVER DEV GROUP :M2M-1669:check retransmit n2 and return INVALID_FRAME_ERR: april 2 :begin */
                if (*rawptr > MAX_RETRANSMIT_N2)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                /* HUAWEI DRIVER DEV GROUP :M2M-1669:check retransmit n2 and return INVALID_FRAME_ERR: april 2 :end */
                dlcpn->muxparams.retransmit_n2 = *rawptr;

                /* 00 00 00 00 00 K3 K2 K1  错误恢复模式的窗口大小K */
                ++rawptr;
                if (*rawptr > MAX_WINDOW_K)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                dlcpn->muxparams.window_k = *rawptr;

                // copy dlcpn structure into uih_data void pointer
                fr_temp->uih_data = (void*) dlcpn;
                ++rawptr;
                *vrawptr = rawptr;
                rc       = PASS;
                break;

            case CLD_CMD:
                if ((((*rawptr) >> 1) & 0x7F) 
                    != frame_details [fr_temp->f_type].frame_length)
                {
                    // Invalid length error
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                /* HUAWEI DRIVER DEV GROUP: M2M-1669:to send NSC if the length valud field is not correct april2 : begin */
                len_vl = (((*rawptr) >> 1) & 0x7F);
                if ((*(rawptr + (len_vl + 2))) != 0xF9)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                if (!(((*rawptr) & CMUX_EA_BIT_MASK) == (CMUX_EA_BIT_MASK)))
                {
                    rc = INVALID_FRAME_ERR;
                    break;

                    // continue.. below..
                }
                *vrawptr = rawptr;
                rc       = PASS;
                break;
                /* HUAWEI DRIVER DEV GROUP:M2M-1669: to send NSC if the length valud field is not correct april2 : end */
            case TEST_CMD:
                test_len = (((*rawptr) >> 1) & 0x7F);
                if (CMUX_EA_BIT_MASK != ((*rawptr) & CMUX_EA_BIT_MASK))
                {
                    test_len = ((*(rawptr + 1)) << 7) | test_len;
                    offset = 1;
                }
                if (0xF9 != (*(rawptr + (test_len + 2 + offset))))
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                ++rawptr;
                fr_temp->uih_data = (void*) cmux_alloc (test_len);
                if (NULL == fr_temp->uih_data)
                {
                    rc = BASIC_FRAME_ERR;
                    break;
                }
                memcpy (fr_temp->uih_data, rawptr + offset, test_len);

                *vrawptr = rawptr;
                rc       = PASS;

                break;
            case PSC_CMD:
            case FCON_CMD:
            case FCOFF_CMD:
                rc = (unsigned int) cmux_check_length (rawptr, fr_temp);
                if (rc == PASS)
                {
                    *vrawptr = rawptr;
                }
                else
                {
                    rc = INVALID_FRAME_ERR;
                }
                break;
            case SNC_CMD:
                // do length validation before malloc to avoid crash.
                // otherwise the memory was being overwritten.
                /* HUAWEI DRIVER DEV GROUP<m2m-595><length validation to avoid crash in case of wrong length value><24-march><Begin>*/
                if ((((*rawptr) >> 1) & 0x7F) 
                    != frame_details [fr_temp->f_type].frame_length)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                /* HUAWEI DRIVER DEV GROUP: M2M-1669:to send NSC if the length valud field is not correct april2 : begin */
                len_vl = (((*rawptr) >> 1) & 0x7F);

                if ((*(rawptr + (len_vl + 2))) != 0xF9)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                /* HUAWEI DRIVER DEV GROUP:M2M-1669: to send NSC if the length valud field is not correct april2 : end */
                /* HUAWEI DRIVER DEV GROUP<m2m-595><length validation to avoid crash in case of wrong length value><24-march><End>*/
                fr_temp->uih_data = (void*) cmux_alloc (sizeof (struct snc_cmd));
                if (NULL == fr_temp->uih_data)
                {
                    rc = BASIC_FRAME_ERR;
                    break;
                }
                snc = (struct snc_cmd*) fr_temp->uih_data;
                // EA Bit is set and if length (in 1-byte) is not matching to expected OR if EA bit is NOT set, then error
                if (CMUX_EA_BIT_MASK != ((*rawptr) & CMUX_EA_BIT_MASK))
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                ++rawptr;
                if (CMUX_EA_BIT_MASK != ((*rawptr) & CMUX_EA_BIT_MASK))
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                if (0x01 != (((*rawptr) >> 1) & 0x01))
                {
                    rc = INVALID_FRAME_ERR;//mandatory that this bit is 1
                    break;
                }
                snc->dlci = ((*rawptr) >> 2) & 0x3F;
                if (MAX_DLC_TGT < snc->dlci)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                rawptr++;
                snc->EA           = (*rawptr) & 0x01;
                snc->s1           = ((*rawptr) >> 1);
                snc->s2           = ((*rawptr) >> 2);
                snc->s3           = ((*rawptr) >> 3);
                /* HUAWEI DRIVER DEV GROUP: M2M-1669:to to validate total snc data april2 : begin */
                snc->s4           = ((*rawptr) >> 4);
                snc->s5           = ((*rawptr) >> 5);
                snc->s6           = ((*rawptr) >> 6);
                snc->s7           = ((*rawptr) >> 7);

                chk_val = check_frame_data (cm, rawptr, (void*)snc, (char )SNC_CMD);
                if (chk_val == 0)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                
                /* HUAWEI DRIVER DEV GROUP: M2M-1669: to to validate total snc data april2 : end */
                fr_temp->uih_data = (void*) snc;
                *vrawptr          = rawptr;
                rc = PASS;
                break;
            case MSC_CMD:
                /* HUAWEI DRIVER DEV GROUP<m2m-595><length validation to avoid crash in case of wrong length value><24-march><Begin>*/
                if ((((*rawptr) >> 1) & 0x7F) 
                    != frame_details [fr_temp->f_type].frame_length)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                /* HUAWEI DRIVER DEV GROUP:M2M-1669: to send NSC if the length valud field is not correct april2 : begin */
                len_vl = (((*rawptr) >> 1) & 0x7F);
                if ((*(rawptr + (len_vl + 2))) != 0xF9)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                /* HUAWEI DRIVER DEV GROUP:M2M-1669: to send NSC if the length valud field is not correct april2 : end */
                /* HUAWEI DRIVER DEV GROUP<m2m-595><length validation to avoid crash in case of wrong length value><24-march><End>*/
                fr_temp->uih_data = (void*) cmux_alloc (sizeof (struct modem_status_cmd));
                if (NULL == fr_temp->uih_data)
                {
                    rc = BASIC_FRAME_ERR;
                    break;
                }
                msc = (struct modem_status_cmd*) fr_temp->uih_data;
                // EA Bit is set and if length (in 1-byte) is not matching to expected OR if EA bit is NOT set, then error
                if (((*rawptr) & CMUX_EA_BIT_MASK) != (CMUX_EA_BIT_MASK))
                {
                    // Invalid length error
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                ++rawptr;
                if (CMUX_EA_BIT_MASK != ((*rawptr) & CMUX_EA_BIT_MASK) )
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                if (0x01 != (((*rawptr) >> 1) & 0x01))
                {
                    rc = INVALID_FRAME_ERR;//mandatory that this bit is 1
                    break;
                }
                msc->dlci = ((*rawptr) >> 2) & 0x3F;
                if (MAX_DLC_TGT < msc->dlci)
                {
                    // pr_debug("DLC Nos not in the range\n");
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                rawptr++;
                msc->EA           = (*rawptr) & 0x01;
                msc->fc           = ((*rawptr) >> 1);
                msc->rtc          = ((*rawptr) >> 2);
                msc->rtr          = ((*rawptr) >> 3);
                /* HUAWEI DRIVER DEV GROUP: M2M-1669:to to validate total msc data april2 : begin */
                msc->resrvd1      = ((*rawptr) >> 4);
                msc->resrvd2      = ((*rawptr) >> 5);
                msc->ic           = ((*rawptr) >> 6);
                msc->dv           = ((*rawptr) >> 7);

                chk_val = check_frame_data (cm, rawptr, (void*)msc, (char )MSC_CMD);
                if (chk_val == 0)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }

                /* HUAWEI DRIVER DEV GROUP:M2M-1669: to to validate total msc data april2 : begin */
                fr_temp->uih_data = (void*) msc;
                *vrawptr          = rawptr;
                rc = PASS;
                break;
            case NSC:
                rc = (unsigned int) cmux_check_length (rawptr, fr_temp);
                if (rc == PASS)
                {
                    fr_temp->uih_data = (void*) cmux_alloc (frame_details [fr_temp->f_type].frame_length);
                    if (NULL == fr_temp->uih_data)
                    {
                        return BASIC_FRAME_ERR;
                    }
                    memcpy (fr_temp->uih_data, rawptr, frame_details [fr_temp->f_type].frame_length);
                    *vrawptr = rawptr;
                }
                else
                {
                    rc = BASIC_FRAME_ERR;
                }
                break;
                /* 针对Fcon和Fcoff响应，只对正确的响应做处理，错误的则丢弃 */
            case FCON_RESP:
            case FCOFF_RESP:
                rc = (unsigned int) cmux_check_length (rawptr, fr_temp);
                if (rc == PASS)
                {
                    *vrawptr = rawptr;
                }
                else
                {
                    rc = BASIC_FRAME_ERR;
                }
                break;
                /* 对下列命令的响应，不做回应，直接丢弃 */
            case DLC_PN_RESP:
            case CLD_RESP:
            case TEST_RESP:
            case PSC_RESP:
            case SNC_RESP:
            case MSC_RESP:
                rc = BASIC_FRAME_ERR;
                break;
            default:
                rc = INVALID_FRAME_ERR;
                break;
        }
    } while (0);

    if ((rc == FAIL) || (rc == BASIC_FRAME_ERR) || (rc == INVALID_FRAME_ERR))
    {
        if (fr_temp->uih_data != NULL)
        {
            cmux_free ((unsigned char *) fr_temp->uih_data);
            fr_temp->uih_data = NULL;
        }
        return rc;
    }
    else
    {
        return FRAME_OK;
    }
} /* mux_frame_to_struct */


unsigned short int cmux_basic_frame_length (unsigned char *ptr, unsigned int *offset)
{
    unsigned short int length   = 0;
    unsigned int l_offset = 0;

    /*把EA位顶掉，取后7个字节即为长度值*/
    length = (CMUX_LENGTH_MASK & ((*ptr) >> 1) );
    // Check for EA bit if EA is set returns length as one byte otherwise returns it as 2 bytes
    if (((*ptr) & CMUX_EA_BIT_MASK) == CMUX_EA_BIT_MASK)
    {
        length   = (((*ptr) >> 1) & CMUX_LENGTH_MASK);
        l_offset = 4;
    }
    else
    {
        l_offset = 5;

        length = ((*(ptr + 1)) << 7) | length;
    }
    /*offset表示的是长度字节结束的那个字节号*/
    *offset = l_offset;
    return length;
} /* cmux_basic_frame_length */


void cmux_fill_length (struct frame *fr_temp, unsigned int *offset, unsigned char **ptr)
{
    unsigned int l_offset = 0;
    unsigned char *rawptr;

    if (*ptr == NULL)
    {
        return;
    }
    rawptr = *ptr;
    if ((fr_temp->length) < 128)
    {
        *rawptr  = ( ((fr_temp->length) << 1) & 0xFE) | CMUX_EA_BIT_MASK;
        l_offset = 4;
    }
    else
    {
        *rawptr = (unsigned char) ( (fr_temp->length) << 1);
        rawptr++;
        *rawptr  = (unsigned char) ((fr_temp->length) >> 7);
        l_offset = 5;
    }
    *offset = l_offset;
    *ptr    = rawptr;
} /* cmux_fill_length */


unsigned int cmux_frame_to_struct (struct frame **fr, struct cmuxlib *cm,
                                unsigned char *rawptr, unsigned short raw_len)
{
    unsigned char *f_ptr   = (unsigned char *) NULL;
    struct frame * fr_temp = (struct frame *) NULL;
    unsigned int i, rc = 0;
    unsigned short int len;
    unsigned char l_fcs;
    unsigned char *data;
    unsigned int offset        = 0;
    const struct framedetails *frame_details = UE_frame_details;

    do
    {
        if (!fr)
        {
            rc = BASIC_FRAME_ERR;
            break;
        }
        fr_temp = *fr;
        if (fr_temp == NULL)
        {
            rc = NO_HEADER;
            break;
        }
        f_ptr     = rawptr; // back up the raw pointer
        g_nsc_frm = rawptr + 2;
        // check for starting flag 0xF9
        if ((*rawptr) != 0xF9)
        {
            rc = NO_HEADER;
            break;
        }
        // Check for length offset=4 for 1 byte length and 5 for 2 byte length
        len = cmux_basic_frame_length (rawptr + 3, &offset);

        if (len > MAX_CMUX_FRAME_SIZE (cm) || len > raw_len)
        {
            rc = BASIC_FRLEN_ERR;
            break;
        }
        // Length is approved !
        fr_temp->length = (unsigned short int) len;
        // if inframe length is with in buf len then check for fcs and f9
        // check for FCS
        if (*(rawptr + 2) != CMUX_DATA_UI)
        {
            l_fcs = cmux_ts0710_check_fcs (rawptr + 1, 
                                ((int) offset - 1), *(rawptr + len + offset));
        }
        else
        {
            l_fcs = cmux_ts0710_check_fcs (rawptr + 1, 
                        (int) ((offset + len) - 1), *(rawptr + len + offset));
        }
        if (l_fcs)
        {
            rc = FCS_ERR;
            break;
        }
        // check for End flag
        if (*(rawptr + len + offset + 1) != 0xF9)
        {
            rc = NO_TAIL;
            break;
        }

        // Upt to here rawptr points of SOF - F9, move ahead from here
        ++rawptr;  // Now points to Address field (DLC, CR, EA)

        fr_temp->dlc = ( ((*rawptr) >> 2) & CMUX_DLC_BIT_MASK);

        // Check for DLC
        // Check for EA bit
        fr_temp->addr_EA_bit = (*rawptr) & CMUX_EA_BIT_MASK;

        if (fr_temp->addr_EA_bit != 0x01)
        {
            rc = BASIC_FR_EA_ERR;
            break;
        }

        // check for CR bit
        fr_temp->add_CR_bit = ((*rawptr) >> 1) & 0x01;
        /* HUAWEI DRIVER DEV GROUP<m2m-1594><C/R bit validation><24-march><Begin>*/

        if (fr_temp->add_CR_bit != 0x01)
        {
            rc = BASIC_FR_CR_ERR;
            break;
        }

        /* HUAWEI DRIVER DEV GROUP<m2m-1594><C/R bit validation><24-march><End>*/

        ++rawptr;  // Now points to Control field (SABM,UIH..)
        fr_temp->f_type = NSC; // to make the default one as nsc
        // Extract frame types and accordingly switch
        for (i = 0; i < MAX_CMUX_FRAMES; i++)
        {
            if (*rawptr == CMUX_DATA_UI)
            {
                fr_temp->f_type = UIH_DATA;
                fr_temp->UI_bit = 1;
                break;
            }
            else
            {
                fr_temp->UI_bit = 0;
            }
            
            if (((*rawptr) & CMUX_PF_BIT_MASK) == 
                (frame_details [i].frame_code & CMUX_PF_BIT_MASK) )
            {
                // Extract P, F bit
                fr_temp->pf_bit = ((*rawptr) & (~CMUX_PF_BIT_MASK)) >> 4;
                if ((i == CTRL_SABM) || (i == CTRL_UA) ||
                    (i == CTRL_DM) || (i == CTRL_DISC))
                {
                    if (fr_temp->pf_bit == 0x00)
                    {
                        rc = BASIC_FR_EA_ERR;
                        return rc;
                    }
                }
            }

            if ((*rawptr) == (frame_details [i].frame_code) )
            {
                fr_temp->f_type = i;
                // Extract P, F bit
                fr_temp->pf_bit = ((*rawptr) & (~CMUX_PF_BIT_MASK)) >> 4;
                break;
            }
            //to consider the FF frame also
            if ((*rawptr) == 0XFF )   // In reception Dont-care the PF bit
            {
                fr_temp->f_type = UIH_DATA;
                // Extract P, F bit
                fr_temp->pf_bit = ((*rawptr) & (~CMUX_PF_BIT_MASK)) >> 4;
                break;
            }
        }
        
        if ( MAX_CMUX_FRAMES == i )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "there is an unrecoginized command %d\n", *rawptr);
            rc = BASIC_FRAME_ERR;
            break;
        }
        
        // Length check has passed, if we are here, so decided based on EA of length field
        if (offset == 4)
        {
            rawptr += 2;
        }
        else
        {
            rawptr += 3;
        }
        // PARKED the rawptr to DATA or MUX-frame

        if ((fr_temp->f_type == CTRL_SABM) || (fr_temp->f_type == CTRL_UA) ||
            (fr_temp->f_type == CTRL_DM) || (fr_temp->f_type == CTRL_DISC))
        {
            if (fr_temp->pf_bit == 0x00)
            {
                rc = BASIC_FR_EA_ERR;
            }
            break;
        }
        else if ((fr_temp->f_type == CTRL_UIH) || (fr_temp->f_type == UIH_DATA))
        {
            /* UIH帧携带Info长度是0，直接丢弃(含UI帧) */
            if (0 == fr_temp->length)
            {
                rc = BASIC_FRAME_ERR;
                break;
            }
            
            if (fr_temp->dlc == 0)
            {
                /* 携带控制消息的UI帧，返回NSC命令 */
                if (1 == fr_temp->UI_bit)
                {
                    rc = INVALID_FRAME_ERR;
                    break;
                }
                
                rc = mux_frame_to_struct (&rawptr, cm, fr_temp);
                break;
            }
            else    // UIH_DATA
            {
                fr_temp->f_type = UIH_DATA;
                /* 携带数据的UI帧，直接丢弃不做处理 */
                if (1 == fr_temp->UI_bit)
                {
                    rc = BASIC_FRAME_ERR;
                    break;
                }
                
                if (fr_temp->length)
                {
                    fr_temp->uih_data = (void *) cmux_alloc (fr_temp->length);
                    if (NULL == fr_temp->uih_data)
                    {
                        rc = FAIL;
                        break;
                    }
                    memcpy (fr_temp->uih_data, rawptr, fr_temp->length);
                }

                rawptr = fr_temp->length + rawptr;

                break;
            }
        }
        /* HUAWEI DRIVER DEV GROUP <M2M-1552> <Sending NSC response to unsupported commands><Begin>*/
        else
        {
            rc = INVALID_FRAME_ERR;
            break;
        }
        /* HUAWEI DRIVER DEV GROUP <M2M-1552> <Sending NSC response to unsupported commands><end>*/
    } while (0);

    if ((rc == FRAME_OK) && (fr_temp)) // j80025223: lint fix
    {
        fr_temp->raw_len  = raw_len;
        fr_temp->raw_data = cmux_alloc (fr_temp->raw_len);
        if (NULL == fr_temp->raw_data)
        {
            return FAIL;
        }
        if (f_ptr != NULL) // j80025223: lint fix
        {
            memcpy (fr_temp->raw_data, f_ptr, fr_temp->raw_len);
        }
    }

    return rc;
} /* cmux_frame_to_struct */


unsigned int mux_struct_to_frame (struct frame *fr_temp,
                                unsigned char **vrawptr, unsigned int *offset)
{
    unsigned char *rawptr = (unsigned char *) NULL;
    int len_val   = 0;
    const struct framedetails *frame_details = UE_frame_details;

    if (NULL == fr_temp || NULL == vrawptr || NULL == *vrawptr)
    {
        return BASIC_FRAME_ERR;
    }
    rawptr = *vrawptr;
    *rawptr = frame_details [CTRL_UIH].frame_code;
    ++rawptr;
    
    // Fill the raw pointer with Length field
    cmux_fill_length (fr_temp, offset, &rawptr);
    ++rawptr;
    
    // Fill the frame code
    *rawptr = frame_details [fr_temp->f_type].frame_code;
    ++rawptr;
    
    // Check for EA bit and fill the length field to the rawptr
    if (fr_temp->f_type == TEST_CMD || fr_temp->f_type == TEST_RESP)
    {
        if (TEST_LEN_LIMIT >= (fr_temp->length))
        {
            len_val = (fr_temp->length - 2);
            *rawptr  = (((len_val) << 1) & 0xFE) | CMUX_EA_BIT_MASK;
        }
        else
        {
            len_val = (fr_temp->length - 3);
            *rawptr = (unsigned char) ((len_val) << 1);
            rawptr++;
            *rawptr  = (unsigned char) ((len_val) >> 7);
        }
    }
    else
    {
        *rawptr = ((frame_details [fr_temp->f_type].frame_length) << 1) & 0xFE | 0x01;
    }

    *vrawptr = rawptr;
    return 0;
} /* mux_struct_to_frame */


unsigned int 
mux_nsc_struct_to_frame (struct frame *fr_temp, unsigned char **vrawptr)
{
    unsigned char *rawptr;

    if (NULL == fr_temp || NULL == vrawptr || NULL == *vrawptr)
    {
        return BASIC_FRAME_ERR;
    }

    rawptr = *vrawptr;

    if (fr_temp->length > 0)
    {
        /* HUAWEI DRIVER DEV GROUP<m2m-1594,1593,1580><proper NSC response format><24-march><Begin>*/
        *rawptr = *g_nsc_frm;
        /* HUAWEI DRIVER DEV GROUP<m2m-1594,1593,1580><proper NSC response format><24-march><End>*/
        rawptr = rawptr + 1;
    }

    *vrawptr = rawptr;
    return 0;
} /* mux_nsc_struct_to_frame */


unsigned int cmux_fill_header_struct_to_frame (struct frame *fr_temp,
                                unsigned char **vrawptr, unsigned int *offset)
{
    unsigned char *rawptr;
    const struct framedetails *frame_details = UE_frame_details;

    if (NULL == fr_temp)
    {
        return BASIC_FRAME_ERR;
    }

    if (NULL == vrawptr)
    {
        return BASIC_FRAME_ERR;
    }

    rawptr = *vrawptr;
    
    // Fill the raw pointer with control Frame code
    *rawptr = frame_details [fr_temp->f_type].frame_code;
    ++rawptr;
    
    // Fill the raw pointer with Length field one or two bytes
    cmux_fill_length (fr_temp, offset, &rawptr);

    ++rawptr;
    *vrawptr = rawptr;
    return 0;
} /* cmux_fill_header_struct_to_frame */


unsigned int mux_dlcpn_struct_to_frame (struct frame *fr_temp,
                                unsigned char **vrawptr, unsigned int *offset)
{
    struct dlc_param_neg *dlcpn;
    unsigned char *       rawptr;

    const struct framedetails *frame_details = UE_frame_details;

    if (NULL == fr_temp)
    {
        return BASIC_FRAME_ERR;
    }

    if (NULL == vrawptr)
    {
        return BASIC_FRAME_ERR;
    }

    rawptr = *vrawptr;

    *rawptr = frame_details [CTRL_UIH].frame_code;
    ++rawptr;
    
    // Fill the raw pointer with Length field
    cmux_fill_length (fr_temp, offset, &rawptr);
    ++rawptr;

    // Fill the Mux frame code
    *rawptr = frame_details [fr_temp->f_type].frame_code;
    ++rawptr;
    
    // Check for EA bit and fill the length field to the rawptr
    *rawptr = ((frame_details [fr_temp->f_type].frame_length) << 1) & 0xFE | 0x01;
    ++rawptr;
    
    // Fill the rawptr with DLC param nego parameters
    if (fr_temp->uih_data != NULL)
    {
        dlcpn = (struct dlc_param_neg *) fr_temp->uih_data;
        // Fill DLC
        *rawptr = (dlcpn->dlci) & CMUX_DLC_BIT_MASK;
        ++rawptr;
        // Fill convergence
        *rawptr = (dlcpn->conv_layer << 4) & 0xF0;
        ++rawptr;
        // Fill priority
        *rawptr = (dlcpn->priority) & CMUX_PRIORITY_BIT_MASK;
        ++rawptr;
        // Fill ack_timer to raw ptr
        *rawptr = dlcpn->muxparams.ack_timer_t1;
        ++rawptr;
        // Fill frame size
        *rawptr = (dlcpn->muxparams.frame_size_n1 & 0xFF);
        ++rawptr;
        *rawptr = (*rawptr) | ((dlcpn->muxparams.frame_size_n1 >> 8));
        ++rawptr;
        // Fill retransmit
        *rawptr = dlcpn->muxparams.retransmit_n2;
        ++rawptr;
        // Fill window_k to rawptr
        *rawptr = (dlcpn->muxparams.window_k) & CMUX_WINDOW_BIT_MASK;
        
        *vrawptr = rawptr;
    }
    return 0;
} /* mux_dlcpn_struct_to_frame */


unsigned int cmux_struct_to_frame (struct frame **fr, void *cmx)
{
    unsigned char *          vraw;
    unsigned char *          f_ptr, *ptr;
    unsigned short int len     = 0;
    unsigned int offset  = 0;
    struct frame *           fr_temp = (struct frame *) NULL;
    struct modem_status_cmd *msc;
    struct snc_cmd *         snc;
    struct cmuxlib *cm;

    if (NULL == fr || NULL == *fr || cmx == NULL)
    {
        return BASIC_FRAME_ERR;
    }

    cm      = (struct cmuxlib *) cmx;
    fr_temp = *fr;

    // conversion already done
    if (fr_temp->raw_data != NULL && fr_temp->raw_len != 0)
    {
        return PASS;
    }

    vraw = (unsigned char *) cmux_alloc ((MAX_CMUX_FRAME_SIZE (cm)));
    if (vraw == NULL)
    {
        return BASIC_FRAME_ERR;
    }

    ptr = vraw;
    // Fill the start flag to the local pointer
    *vraw = CMUX_FRAME_FLAG;
    ++vraw;
    f_ptr = vraw; // used for fcs

    // Fill the address field with DLC,EA and CR bit to the vraw
    *vraw = (((fr_temp->dlc) << 2) & 0xFC) | 
        (0x03 & ((fr_temp->addr_EA_bit & 0x01) | ((fr_temp->add_CR_bit) << 1)));
    ++vraw;
    
    // According to the frame type switch to the respective case
    switch (fr_temp->f_type)
    {
        case CTRL_SABM:
        case CTRL_UA:
        case CTRL_DM:
        case CTRL_DISC:
            cmux_fill_header_struct_to_frame (fr_temp, &vraw, &offset);
            break;
        case DLC_PN_CMD:
        case DLC_PN_RESP:
            if (fr_temp->dlc == 0)
            {
                mux_dlcpn_struct_to_frame (fr_temp, &vraw, &offset);
                ++vraw;
                break;
            }
            break;
        case CLD_CMD:
        case CLD_RESP:

            if (fr_temp->dlc == 0)
            {
                mux_struct_to_frame (fr_temp, &vraw, &offset);
                ++vraw;
                break;
            }
            break;
            
        case SNC_CMD:
        case SNC_RESP:
            if (fr_temp->dlc == 0)
            {
                mux_struct_to_frame (fr_temp, &vraw, &offset);
                ++vraw;
                if (NULL != fr_temp->uih_data)
                {
                    snc   = (struct  snc_cmd*) fr_temp->uih_data;
                    *vraw = (snc->dlci << 2 & 0xFC) | 0x03;
                    vraw++;
                    *vraw = snc->EA | snc->s1 << 1 | snc->s2 << 2 
                        | snc->s3 << 3 | snc->s4 << 4;
                    /* j80025223:UI Frame support Begin*/
                    vraw++;
                    *vraw |= 0x01;
                    /* j80025223:UI Frame support End*/
                }
                vraw++;
                break;
            }
            break;
        case PSC_CMD:
        case PSC_RESP:
            if (fr_temp->dlc == 0)
            {
                mux_struct_to_frame (fr_temp, &vraw, &offset);
                ++vraw;
                break;
            }
            break;

        case MSC_CMD:
        case MSC_RESP:
            if (fr_temp->dlc == 0)
            {
                mux_struct_to_frame (fr_temp, &vraw, &offset);
                ++vraw;
                if (NULL != fr_temp->uih_data)
                {
                    msc   = (struct  modem_status_cmd*) fr_temp->uih_data;
                    *vraw = (msc->dlci << 2 & 0xFC) | 0x03;
                    vraw++;
                    *vraw = msc->EA | msc->fc << 1 | msc->rtc << 2 
                        | msc->rtr << 3 | msc->ic << 6 | msc->dv << 7;
                }
                vraw++;
                break;
            }
            break;

        case TEST_CMD:
        case TEST_RESP:
            if (fr_temp->dlc == 0)
            {
                mux_struct_to_frame (fr_temp, &vraw, &offset);
                ++vraw;
                if (NULL != fr_temp->uih_data)
                {
                    if (TEST_LEN_LIMIT >= (fr_temp->length))
                    {
                        memcpy (vraw, fr_temp->uih_data, (fr_temp->length - 2));
                        vraw = vraw + (fr_temp->length - 2);
                    }
                    else
                    {
                        memcpy (vraw, fr_temp->uih_data, (fr_temp->length - 3));
                        vraw = vraw + (fr_temp->length - 3);
                    }
                }
                else
                {
                    vraw = vraw + (fr_temp->length - 2);
                }
            }
            break;

        case FCON_CMD:
        case FCON_RESP:
        case FCOFF_CMD:
        case FCOFF_RESP:
            if (fr_temp->dlc == 0)
            {
                mux_struct_to_frame (fr_temp, &vraw, &offset);
                ++vraw;
            }
            break;
        case NSC:
            if (fr_temp->dlc == 0)
            {
                mux_struct_to_frame (fr_temp, &vraw, &offset);
                ++vraw;
                mux_nsc_struct_to_frame (fr_temp, &vraw);
            }
            break;

        case UIH_DATA:
        {
            // DATA
            // Fill the UIH_DATA  code
            cmux_fill_header_struct_to_frame (fr_temp, &vraw, &offset);
            
            if (fr_temp->length > 0)
            {
                memcpy (vraw, fr_temp->uih_data, fr_temp->length);
            }
            vraw = vraw + fr_temp->length;
            break;
        }
        default:
            break;
    } /* switch */

    // Calculate FCS
    *vraw = cmux_ts0710_compute_crc ((f_ptr), ((int) offset - 1));
    ++vraw;
    // Fill the rawptr with End Flag
    *vraw = CMUX_FRAME_FLAG;
    // Copy the vraw pointer to the void raw_data pointer in the frame structure which will be to the serial line for transmission
    len = (unsigned short int) (vraw - ptr);

    fr_temp->raw_data = ptr;
    fr_temp->raw_len  = len + 1;

    return PASS;
} /* cmux_struct_to_frame */


int Cmux_get_default_cmux_frame (struct cmuxlib *cm, unsigned char dlc, 
                                enum fr_type ftype, struct frame **fr, 
                                unsigned char *data, unsigned int data_len)
{
    struct frame *            lfr = (struct frame *) NULL;
    struct dlc_param_neg *    dlcpnrxed, *dlcpn = (struct dlc_param_neg *) NULL;
    struct modem_status_cmd * mscgui, *msc = (struct modem_status_cmd *) NULL;
    struct snc_cmd *          sncgui, *snc = (struct snc_cmd *) NULL;
    struct non_supported_cmd_resp *nsc = (struct non_supported_cmd_resp *) NULL;
    int rc = PASS;

    const struct framedetails *frame_details = UE_frame_details;

    if (NULL == fr || ftype > MAX_CMUX_FRAMES)
    {
        return FAIL;
    }
    if (dlc > MAX_DLC_TGT)
    {
        if (ftype != CTRL_DM)
        {
            return FAIL;
        }
    }
    // Enters with assumption frame is allocated

    if (ftype == UIH_DATA || ftype == TEST_CMD || ftype == TEST_RESP)
    {
        if (NULL == data || data_len == 0)
        {
            return FAIL;
        }
    }
    
    if (ftype == MSC_CMD || ftype == MSC_RESP 
        || ftype == SNC_CMD || ftype == SNC_RESP
        || ftype == DLC_PN_CMD || ftype == DLC_PN_RESP)
    {
        if (NULL == data)
        {
            return FAIL;
        }
    }
    
    lfr = (struct frame *) cmux_alloc (sizeof (struct frame));

    if (lfr == NULL)
    {
        return FAIL; // malloc failure
    }

    do
    {
        switch (ftype)
        {
        case CTRL_SABM:
        case CTRL_UA:
        case CTRL_DM:
        case CTRL_DISC:
            lfr->length      = frame_details [ftype].frame_length;
            lfr->f_type      = ftype;
            lfr->dlc         = dlc;
            lfr->addr_EA_bit = 1;
            lfr->add_CR_bit  = 1;
            lfr->pf_bit      = 1;
            rc               = PASS;
            break;

        case MSC_CMD:
        case MSC_RESP:
            mscgui = (struct modem_status_cmd *) data;
            
            msc = (struct modem_status_cmd *) cmux_alloc (sizeof(struct modem_status_cmd));
            if (msc == NULL)
            {
                rc = FAIL;
                break;
            }
            lfr->dlc         = 0;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            lfr->f_type      = ftype;
            lfr->pf_bit      = 0;
            lfr->length      = frame_details [ftype].frame_length + 2;
            if (ftype == MSC_CMD)
            {
                msc->dlci = dlc;
            }
            else
            {
                msc->dlci = mscgui->dlci;
            }
            msc->dv       = mscgui->dv;
            msc->ic       = mscgui->ic;
            msc->EA       = 1;
            msc->fc       = mscgui->fc;
            msc->rtc      = mscgui->rtc;
            msc->rtr      = mscgui->rtr;
            lfr->uih_data = (void *) msc;
            rc            = PASS;
            break;
        case SNC_CMD:
        case SNC_RESP:
            sncgui = (struct snc_cmd *) data;
            
            snc    = (struct snc_cmd *) cmux_alloc (sizeof (struct snc_cmd));
            if (snc == NULL)
            {
                rc = FAIL;
                break;
            }
            lfr->dlc         = 0;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            lfr->f_type      = ftype;
            lfr->pf_bit      = 1;
            lfr->length      = frame_details [ftype].frame_length + 2;
            if (ftype == SNC_CMD)
            {
                snc->dlci = dlc;
            }
            else
            {
                snc->dlci = sncgui->dlci;
            }
            snc->EA       = 1;
            snc->s1       = sncgui->s1;
            snc->s2       = sncgui->s2;
            snc->s3       = sncgui->s3;
            snc->s4       = sncgui->s4;
            snc->s5       = sncgui->s5;
            snc->s6       = sncgui->s6;
            snc->s7       = sncgui->s7;
            lfr->uih_data = (void *) snc;
            rc            = PASS;
            break;
        case DLC_PN_CMD:
        case DLC_PN_RESP:
            dlcpnrxed = (struct dlc_param_neg *) data;
            
            dlcpn = (struct dlc_param_neg *) cmux_alloc (sizeof (struct dlc_param_neg));
            if (dlcpn == NULL)
            {
                rc = FAIL;
                break;
            }
            lfr->dlc         = 0;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            lfr->f_type      = ftype;
            lfr->pf_bit      = 1;
            lfr->length      = frame_details [ftype].frame_length + 2;
            dlcpn->priority   = DEFAULT_PRIORITY;
            dlcpn->conv_layer = CONVERGENCE_LAYER;

            dlcpn->dlci                    = dlcpnrxed->dlci;
            dlcpn->muxparams.ack_timer_t1  = dlcpnrxed->muxparams.ack_timer_t1;
            dlcpn->muxparams.frame_size_n1 = dlcpnrxed->muxparams.frame_size_n1;
            dlcpn->muxparams.retransmit_n2 = dlcpnrxed->muxparams.retransmit_n2;
            dlcpn->muxparams.window_k      = dlcpnrxed->muxparams.window_k;
            dlcpn->muxparams.res_timer_t2  = dlcpnrxed->muxparams.res_timer_t2;

            lfr->uih_data = (void *) dlcpn;
            rc            = PASS;
            break;

        case CLD_CMD:
        case CLD_RESP:
            lfr->dlc         = 0;
            lfr->f_type      = ftype;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            lfr->pf_bit      = 1;
            lfr->length      = frame_details [ftype].frame_length + 2;
            rc               = PASS;
            break;
        case TEST_CMD:
        case TEST_RESP:

            lfr->dlc         = 0;
            lfr->f_type      = ftype;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            lfr->pf_bit      = 1;
            if ((TEST_LEN_LIMIT - 2) >= data_len)/*lint !e574*/
            {
                lfr->length = (unsigned short) data_len + 2;
            }
            else
            {
                lfr->length = (unsigned short) data_len + 3;
            }
            lfr->uih_data    = (void *) cmux_alloc ((int) data_len);
            if (NULL == lfr->uih_data)
            {
                rc = FAIL;
                break;
            }
            
            memcpy (lfr->uih_data, data, (int)data_len);
            rc = PASS;
            break;
        case PSC_CMD:
        case PSC_RESP:
            lfr->dlc         = 0;
            lfr->f_type      = ftype;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->pf_bit = 1;
            }
            else
            {
                lfr->pf_bit = 0;
            }
            lfr->length = frame_details [ftype].frame_length + 2;
            rc          = PASS;
            break;

        case NSC:
            lfr->dlc         = 0;
            lfr->f_type      = ftype;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            lfr->pf_bit      = 1;
            lfr->length      = frame_details [ftype].frame_length + 2;
            nsc              = (struct non_supported_cmd_resp *) cmux_alloc (sizeof (struct non_supported_cmd_resp));
            if (NULL == nsc)
            {
                rc = FAIL;
                break;
            }
            lfr->uih_data = (void *) nsc;
            rc            = PASS;
            break;

        case FCON_RESP:
        case FCOFF_RESP:
            lfr->dlc         = 0;
            lfr->f_type      = ftype;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            lfr->pf_bit      = 1;
            lfr->length      = frame_details [ftype].frame_length + 2;
            rc               = PASS;
            break;
        case FCON_CMD:
        case FCOFF_CMD:
            lfr->dlc         = 0;
            lfr->f_type      = ftype;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }
            lfr->pf_bit      = 1;/*lint !e539 !e830*/
            lfr->length      = frame_details [ftype].frame_length + 2;
            rc               = PASS;
            break;
        case UIH_DATA:
            lfr->dlc         = dlc;
            lfr->f_type      = ftype;
            lfr->addr_EA_bit = 1;
            if (cm->cmux_status & CMUX_TEMODE)
            {
                lfr->add_CR_bit = 1;
            }
            else
            {
                lfr->add_CR_bit = 0;
            }

            lfr->uih_data = (void *) cmux_alloc ((int) data_len);
            if (NULL == lfr->uih_data)
            {
                rc = FAIL;
                break;
            }
            
            memcpy (lfr->uih_data, data, (int)data_len);
            lfr->length = (unsigned short) data_len;
            rc = PASS;
            break;
        } /* switch */
    } while (0);

    if (rc == FAIL)
    {
        if (lfr->uih_data != NULL)
        {
            cmux_free ((unsigned char *) lfr->uih_data);
            lfr->uih_data = NULL;
        }
        if (lfr != NULL)
        {
            cmux_free ((unsigned char *) lfr);
            lfr = NULL;
        }
    }
    else
    {
        *fr = lfr;
    }
    return rc;
} /* Cmux_get_default_cmux_frame */


int cmux_check_proper_frame (unsigned char *rawptr, struct cmuxlib *cm, 
                        unsigned short rawlen, struct frame_result *fr_result)
{
    unsigned int len, l_fcs, i, offset;
    struct frame_result *result;

    result = (struct frame_result *)fr_result;

    memset ((void *)result, 0, sizeof (struct frame_result));

    for (i = 0; i < rawlen; i++)
    {
        if (*(rawptr + i) != CMUX_FRAME_FLAG)
        {
            continue;
        }
        else
        {
            break;
        }
    }

    result->start_offset = (unsigned short) i;

    if (i >= rawlen)
    {
        result->start_flag = 0;
        result->err        = 1;
        CMUX_PRINT(CMUX_DEBUG_INFO, "start_flag wrong!\r\n");
        return -1;
    }

    // Surely atleast one byte is there..so check start flag
    // 1. Check start flag
    if (*(rawptr + i) == CMUX_FRAME_FLAG)
    {
        result->start_flag = 1;
        result->err        = 0;
    }
    else
    {
        result->start_flag = 0;
        result->err        = 1;
        CMUX_PRINT(CMUX_DEBUG_INFO, "start_flag wrong!\r\n");
        return -1;
    }
    /* HUAWEI DRIVER DEV GROUP:M2M-1669: check  ea bit of dlc before proceeding to data:April 2 :begin*/
    if (!(rawlen - i < 2))
    {
        if (((*(rawptr + result->start_offset + 1) & CMUX_EA_BIT_MASK)
            != CMUX_EA_BIT_MASK))
        {
            result->err        = 1;
            CMUX_PRINT(CMUX_DEBUG_INFO, "address wrong!\r\n");
            return -1;
        }
    }
    /* HUAWEI DRIVER DEV GROUP: M2M-1669:check  ea bit of dlc before proceeding to data:April 2 :begin*/
    // Now check if we have enough bytes for header width to even read..
    if (rawlen - i < 4)
    {
        return 0;
    }
    /* (rawptr + result->start_offset + 3)是长度域*/
    if (!((*(rawptr + result->start_offset + 3) & CMUX_EA_BIT_MASK) 
        == CMUX_EA_BIT_MASK))
    {
        if (rawlen - i < 5)
        {
            return 0;
        }
    }
    // 2. Check Length
    // Check for length offset=4 for 1 byte length and 5 for 2 byte length
    len = cmux_basic_frame_length (rawptr + result->start_offset + 3, &offset);

    if (len > (MAX_CMUX_FRAME_SIZE (cm) - (offset + 2)) /* offset 4/5 */)
    {
        result->length_flag = 0;
        result->err         = 1;
        CMUX_PRINT(CMUX_DEBUG_INFO, "length wrong!\r\n");
        return -1;
    }

    // if inframe length is with in buf len then check for fcs and f9
    // offset is 4 or 5 (according to EA bit, len-field), 2 is for FCS,f9
    /*(len + offset + 2)是一个完整帧长度*/
    if ((len + offset + 2) <= rawlen)
    {
        result->length_flag = 1;
        
        /*(rawptr + result->start_offset + 2)是contral域*/
        if ((*(rawptr + result->start_offset + 2)) != CMUX_DATA_UI)
        {
            l_fcs = cmux_ts0710_check_fcs (rawptr + result->start_offset + 1, 
                (int) offset - 1, *(rawptr + result->start_offset + len + offset));
        }
        else
        {
            l_fcs = cmux_ts0710_check_fcs (rawptr + result->start_offset + 1, 
                (int)((offset + len) - 1), 
                *(rawptr + result->start_offset + len + offset));
        }
        
        if (l_fcs)
        {
            result->fcs = 0;
            result->err = 1;
            CMUX_PRINT(CMUX_DEBUG_INFO, "fcs wrong!\r\n");
            return -1;
        }
        result->fcs = 1;

        // Move to End flag
        rawptr = rawptr + result->start_offset + len + offset + 1;

        // check for End flag
        if (*(rawptr) != CMUX_FRAME_FLAG)
        {
            result->end_flag = 0;
            result->err      = 1;
            CMUX_PRINT(CMUX_DEBUG_INFO, "end_flag wrong!\r\n");
            return -1;
        }
        result->end_flag   = 1;
        result->end_offset = (unsigned short) (result->start_offset + len + offset + 1);
        return PASS;
    }
    return PASS;
} /* cmux_check_proper_frame */


int Cmux_Input_frame_to_CMUX (struct cmuxlib *cm, unsigned char *buffer, 
                                                    unsigned short buffer_len)
{/*lint !e18 !e516*/
    unsigned short i;
    int count = -1;
    int rc    = 0;

    unsigned short offset;
    struct frame * fr;
    int rc_check = -1;
    unsigned char *rawptr [2];
    unsigned short rawptrlen [2];
    unsigned char buff [20];
    int k;

    struct frame_result fresult;
    CMUX_PRINT(CMUX_DEBUG_INFO, "cmux_hex_dump!\r\n");
    cmux_hex_dump(CMUX_DEBUG_INFO, buffer, buffer_len);
    // Entry validations
    if (NULL == cm)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cm or COM null error!\r\n");
        return -1;
    }

    if (NULL == buffer || 0 == buffer_len)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "buffer length error!\r\n");
        return -1;
    }
    CMUX_PRINT(CMUX_DEBUG_INFO, "buffer_len = %d!\r\n", buffer_len);
    CMUX_PRINT(CMUX_DEBUG_INFO, "pending_frame_idx = %d!\r\n", 
            cm->cmux_if.pending_frame_idx);
    // STEP 1: Consider Pending data if any
    // Before going to extract several frames from the buffer[], first check for pending frame
    // If Previous pending frame is there, then it will be appended in end, to see if the frame makes sense in parsing
    // Logic below fills rawptr[0,1], rawptrlen[0,1] respectively with index-0 filled means its an approved & merged proper pending frame

    if (cm->cmux_if.pending_frame_idx != 0)
    {
        // Now that there is a pending frame, check if there a pending counter part in received frame
        // Copy upto BASIC_FRAME_LEN of Pending-Buffer,.. from buffer_len
        // Check for frame correctness

        if (buffer_len >= MAX_CMUX_FRAME_SIZE (cm))
        {
            offset = MAX_CMUX_FRAME_SIZE (cm);  // Note: full buffer not copied
        }
        else
        {
            offset = buffer_len;
        }
        memcpy (cm->cmux_if.pending_frame + cm->cmux_if.pending_frame_idx, buffer, offset);

        // Always check.. Call cmux_check_proper_frame() only if MAX size is Hit !!
        //校验从pending_frame开始是不是一个完整的帧
        if ((cm->cmux_if.pending_frame_idx + offset) >= MAX_CMUX_FRAME_SIZE (cm))
        {
            /* HUAWEI DRIVER DEV GROUP<M2m 1679><to check for consequtivee F9> <Begin> */
            for(k = 0; k < (MAX_CMUX_FRAME_SIZE(cm)); k++)
            {
                if(*((cm->cmux_if.pending_frame) + k) == CMUX_FRAME_FLAG)
                {
                    if(*((cm->cmux_if.pending_frame) + k + 1) == CMUX_FRAME_FLAG)
                    {
                        continue;
                    }
                    break;
                }
            }
            cm->cmux_if.pending_frame = cm->cmux_if.pending_frame + k;
            /* HUAWEI DRIVER DEV GROUP<M2m 1679><to check for consequtivee F9> <END> */
            rc = cmux_check_proper_frame (cm->cmux_if.pending_frame, cm, 
                            MAX_CMUX_FRAME_SIZE (cm) - k, &(cm->cmux_if.result));
        }
        else
        {
            /* HUAWEI DRIVER DEV GROUP<M2m 1679><to check for consequtivee F9> <Begin> */
            for(k = 0; k < ((cm->cmux_if.pending_frame_idx) + offset); k++)
            {
                if(*((cm->cmux_if.pending_frame) + k) == CMUX_FRAME_FLAG)
                {
                    if(*((cm->cmux_if.pending_frame) + k + 1) == CMUX_FRAME_FLAG)
                    {
                        continue;
                    }
                    break;
                }
            }
            cm->cmux_if.pending_frame = cm->cmux_if.pending_frame + k;
            /* HUAWEI DRIVER DEV GROUP<M2m 1679><to check for consequtivee F9> <eND> */
            rc = cmux_check_proper_frame (cm->cmux_if.pending_frame, cm, 
            (cm->cmux_if.pending_frame_idx + offset - k), &(cm->cmux_if.result));
        }

        if (rc == -1)
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Not a valid frame\n");
            if (NULL != cm->cmux_if.pending_frame)
            {
                memset ((void*)(cm->cmux_if.pending_frame), 0, CMUX_PENDING_FRAME_SIZE);
            }
            cm->cmux_if.pending_frame_idx = 0;
            
            return 0;
        }

        // if correct then pending frame + consume cm->if.result.end_offset - cm->if.pending_frame_idx) + 1 from buffer[]
        if (cm->cmux_if.result.start_flag == 1 
            && cm->cmux_if.result.length_flag == 1 
            && cm->cmux_if.result.fcs == 1 
            && cm->cmux_if.result.end_flag == 1)
        {
            //有头有尾，取完整的帧
            // GOOD frame case
            rawptr [0]    = cm->cmux_if.pending_frame;
            rawptrlen [0] = cm->cmux_if.result.end_offset + 1;

            rawptr [1]    = buffer + ((cm->cmux_if.result.end_offset 
                                        - cm->cmux_if.pending_frame_idx + k) + 1);
            rawptrlen [1] = buffer_len - ((cm->cmux_if.result.end_offset 
                                        - cm->cmux_if.pending_frame_idx + k) + 1);
        }
        else if (cm->cmux_if.result.start_flag == 1 
            && cm->cmux_if.result.end_flag == 0 
            && cm->cmux_if.result.err == 0 
            && ((cm->cmux_if.pending_frame_idx + offset) < MAX_CMUX_FRAME_SIZE (cm)))
        {
            //有头无尾，接在后面
            // STILL PENDING case
            // Still pending, but cannot be dismissed as incorrect-frame
            // ie,. start flag is there, but no end flag..and no-error at the same time total pending data that was check has not cross MAX-CMUX frame size

            // No pending frame to process, but pending index gets an update
            cm->cmux_if.pending_frame_idx = cm->cmux_if.pending_frame_idx + offset - k;
            rawptr [0]                    = (unsigned char *) NULL;
            rawptrlen [0]                 = 0;

            // If control is here, it certainly means that offset == buffer_len AND less than MAX_CMUX_FRAME_SIZE,
            // and is FULLY consumed nothing to pending with nothing left to process in buffer
            rawptr [1]                    = (unsigned char *) NULL;
            rawptrlen [1]                 = 0;
            CMUX_PRINT(CMUX_DEBUG_INFO, "frame is added to pending frame!\r\n");
            return 0;
            // Not to worry that we overflow the pending buffer, code checks for accomodating ONLY up to MAX CMUX frame size and not beyond
        }
        else
        {
            //其他情形，全部送进STEP3处理
            // BY DEFAULT frame HAS to be GOOD or fall to PENDING, else we flush

            // if incorrect flush pending frame, go ahead with new  buffer
            // Eg: cases like this will fall here: (cm->cmux_if.result.start_flag == 0 || cm->cmux_if.result.err == 1) (OR)
            // start flag == 1 and end flag == 0 and No-error BUT pending_frame_idx + offset >= MAX_CMUX_FRAME_SIZE, then no way we store the pending frame anymore

            cm->cmux_if.pending_frame_idx = 0;
            memset ((void *)(cm->cmux_if.pending_frame), 0, CMUX_PENDING_FRAME_SIZE);

            rawptr [0]    = (unsigned char *) NULL; // No pending frame
            rawptrlen [0] = 0;

            rawptr [1]    = buffer; // Take input buffer only
            rawptrlen [1] = buffer_len;
        }
    }
    else
    {
        CMUX_PRINT(CMUX_DEBUG_INFO, "there is no pending frame\r\n");
        //没有pending_frame的情形，放在rawptr[1]里面检验
        rawptr [0]    = (unsigned char *) NULL;
        rawptrlen [0] = 0;

        rawptr [1]    = buffer;
        rawptrlen [1] = buffer_len;
    }

    // STEP 2: Send pending frame and new buffer frame now
    // rawptr[0] is for successful pending frame formation
    if (NULL != rawptr [0] && 0 != rawptrlen [0])
    {
        fr = (struct frame *) cmux_alloc (sizeof (struct frame));

        if (NULL == fr)
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "fr null error!\r\n");
            return -1;
        }

        /* HUAWEI DRIVER DEV GROUP <M2M-1552> <Sending NSC response to unsupported commands><begin>*/
        rc_check = cmux_frame_to_struct (&fr, cm, rawptr [0], rawptrlen [0]);
        
        //  HUAWEI DRIVER DEV GROUP: to send nsc for invalid frame
        if(rc_check == INVALID_FRAME_ERR)
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "rc_check is INVALID_FRAME_ERR\r\n");
            fr->f_type = NSC;
        }

        /* HUAWEI DRIVER DEV GROUP <M2M-1552> <Sending NSC response to unsupported commands><end>*/
        count = 1;

        // Give frame to command layer
        if((rc_check == INVALID_FRAME_ERR) || (rc_check == FRAME_OK))
        {
            (*(cm->frame_map.cmux_cmd_handler))((void *) cm, (void *) fr);
        }
        cmux_free_frame (fr);
        g_nsc_frm = NULL;

        cm->cmux_if.pending_frame_idx = 0;
        if (NULL != cm->cmux_if.pending_frame) // coverity fix
        {
            memset ((void*)(cm->cmux_if.pending_frame), 0, CMUX_PENDING_FRAME_SIZE);
        }
    }

    // STEP 3: Call series of cmux_frame_to_struct and get the framing done for buffer
    // rawptr[1] is for buffer
    CMUX_PRINT(CMUX_DEBUG_INFO, "rawptrlen [1] = %d\n", rawptrlen [1]);
    for (i = 0; i < (rawptrlen [1]); )
    {
        // Do a basic check for Start flag,..before calling cmux_check_proper_frame()
        if (rawptrlen [1] > 1)
        {
            if (!((*(rawptr [1] + i) == CMUX_FRAME_FLAG) 
                && (*(rawptr [1] + i + 1) != CMUX_FRAME_FLAG)))
            {
                i++;
                continue;
            }
        }
        else
        {
            if (*(rawptr [1] + i) != CMUX_FRAME_FLAG)
            {
                i++;
                continue;
            }
        }

        // Give only required length to check for frame
        if (rawptrlen [1] - i < MAX_CMUX_FRAME_SIZE (cm))
        {
            offset = rawptrlen [1] - i;
        }
        else
        {
            offset = MAX_CMUX_FRAME_SIZE (cm);
        }
        CMUX_PRINT(CMUX_DEBUG_INFO, "offset = %d\n", offset);
        // Best is been done,start flag locaed, length found and trimmed to max.length, now check for proper frame
        rc = cmux_check_proper_frame (rawptr [1] + i, cm, offset, &fresult);
        if (rc == -1)
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Not a valid frame\n");
            if (NULL != cm->cmux_if.pending_frame)
            {
                memset ((void*)(cm->cmux_if.pending_frame), 0, CMUX_PENDING_FRAME_SIZE);
            }
            cm->cmux_if.pending_frame_idx = 0;
            
            return 0;
        }
        // CORRECT frame check
        if (fresult.start_flag == 1 
            && fresult.length_flag == 1 
            && fresult.fcs == 1 
            && fresult.end_flag == 1 
            && fresult.err == 0)
        {
            if (count == -1)
            {
                count = 0;               // Entry of a good frame
            }
            fr = (struct frame *) cmux_alloc (sizeof (struct frame));

            if (NULL == fr)
            {
                return -1;
            }

            /* HUAWEI DRIVER DEV GROUP <M2M-1552> <Sending NSC response to unsupported commands><begin>*/
            rc_check = cmux_frame_to_struct (&fr, cm, rawptr [1] + i, 
                            (fresult.end_offset - fresult.start_offset + 1));
            CMUX_PRINT(CMUX_DEBUG_INFO, "rc_check = %d\n", rc_check);
            
            //  HUAWEI DRIVER DEV GROUP: to send nsc for invalid frame
            if(rc_check == INVALID_FRAME_ERR)
            {
                fr->f_type = NSC;
            }

            /* HUAWEI DRIVER DEV GROUP <M2M-1552> <Sending NSC response to unsupported commands><end>*/
            count++;

            // Give frame to command layer
            if((rc_check == INVALID_FRAME_ERR) || (rc_check == FRAME_OK))
            {
                int result;
                result = (*cm->frame_map.cmux_cmd_handler)((void *) cm, (void *) fr);
                CMUX_PRINT(CMUX_DEBUG_INFO, "cmux_cmd_handler return %d\n", result);
            }
            cmux_free_frame (fr);
            g_nsc_frm = NULL;
        }

        // Pending frame creation happens here..
        if (fresult.start_flag == 1 && fresult.end_flag == 0 && fresult.err == 0)
        {
            if (count == -1)
            {
                count = 0;               // Entry of a good frame
            }
            // Control is here, means we are at the near end of buffer.. so back up to pending frame
            memcpy (cm->cmux_if.pending_frame, 
                (rawptr [1] + i + fresult.start_offset), offset);
            cm->cmux_if.pending_frame_idx = offset;
            CMUX_PRINT(CMUX_DEBUG_INFO, "frame is not full.count is %d\n", count);
            return count;
        }

        // Move the offset (good frame offset or the bad frame offset(atleast one byte increment and go to check F9 logic )
        i = i + fresult.end_offset + 1;
    }
    CMUX_PRINT(CMUX_DEBUG_INFO, "THE number of frames are %d\n",count);
    return count;
} /* Cmux_Input_frame_to_CMUX */


int cmux_framer_init (struct cmuxlib *cm)
{
    if (((cm->cmux_status & CMUX_TEMODE) != CMUX_TEMODE) 
        && ((cm->cmux_status & CMUX_UEMODE) != CMUX_UEMODE))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_status error!\n");
        return FAIL;
    }

    if (cm->cmux_status & CMUX_TEMODE)
    {
        cm->frame_map.frdetails    = (struct framedetails *) NULL;
        cm->frame_map.pending_mask = 0;
    }
    else
    {
        cm->frame_map.frdetails    = UE_frame_details;
        cm->frame_map.pending_mask = UE_PENDING_MASK;
    }
    cm->cmux_if.pending_frame = cmux_alloc (CMUX_PENDING_FRAME_SIZE);
    return PASS;
} /* cmux_framer_init */
#endif /* MBB_HSUART_CMUX */
