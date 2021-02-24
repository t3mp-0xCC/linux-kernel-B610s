
#ifdef WIN32
#include <string.h>
#else
#include <linux/string.h>
#endif /* WIN32 */

// Cmux common includes
#include "cmux_framer.h"
#include "cmux_cmd.h"
#include "cmux_drv_ifc.h"
#include "cmux_tgt_osal.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
/* Functions common to both TE (Host) and UE (Target) mode are placed here */
int Cmux_get_default_cmux_frame (struct cmuxlib *, unsigned char, enum fr_type, 
                            struct frame **, unsigned char *, unsigned int);
struct cmux_cmd *cmux_make_cmd (struct frame *, struct cmux_cmd **);
int cmux_queue_cmd (struct cmuxlib *, struct cmux_cmd **);
int cmux_kick_start_cmd (struct cmuxlib *, unsigned char, 
                        unsigned short, unsigned short);
int cmux_create_dlc_entry (struct cmuxlib *, unsigned char);
int cmux_cmd_init (struct cmuxlib *);
int cmux_delete_dlc_entry (struct cmuxlib *, unsigned char);
void delete_head_cmd (struct cmuxlib *cm, unsigned char dlc);


void Cmux_get_default_mux_params (struct mux_params *muxparam, 
                                                cmux_info_type *cmux_info)
{
    muxparam->ack_timer_t1    = cmux_info->response_timer_T1;
    muxparam->baud_rate       = cmux_info->port_speed;
    muxparam->frame_size_n1   = cmux_info->max_frame_size_N1;
    muxparam->mode            = cmux_info->operating_mode;
    muxparam->res_timer_t2    = cmux_info->response_timer_T2;
    muxparam->retransmit_n2   = cmux_info->max_cmd_num_tx_times_N2;
    muxparam->wakeup_timer_t3 = cmux_info->response_timer_T3;
    muxparam->window_k        = cmux_info->window_size_k;
}


int cmux_mux_param_validate (struct mux_params mux)
{
    if ((mux.ack_timer_t1 < MIN_DEFAULT_ACK_TIMER_T1) 
        || (mux.ack_timer_t1 > MAX_DEFAULT_ACK_TIMER_T1))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "mux T1 error!\n");
        return FAIL;
    }

    if ((mux.frame_size_n1 > CMUX_MAXIMUM_FRAME_SIZE) 
        || (mux.frame_size_n1 < CMUX_MINIMUM_FRAME_SIZE))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "mux N1 error!\n");
        return FAIL;
    }

    if ((mux.res_timer_t2 < MIN_DEFAULT_RES_TIMER_T2) 
        || (mux.res_timer_t2 > MAX_DEFAULT_RES_TIMER_T2))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "mux T2 error!\n");
        return FAIL;
    }

    if (mux.retransmit_n2 > MAX_DEFAULT_RETRANS_N2)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "mux N2 error!\n");
        return FAIL;
    }

    if (mux.wakeup_timer_t3 < DEFAULT_WAKEUP_TIMER_T3)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "mux T3 error!\n");
        return FAIL;
    }

    if (mux.window_k > MAX_WINDOW_K || mux.window_k == 0)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "mux WINDOW K error!\n");
        return FAIL;
    }

    return PASS;
} /* cmux_mux_param_validate */


void Cmux_set_default_sncdb (struct cmuxlib *cm);

struct cmuxlib *Cmux_cmuxlib_init (enum CMUX_MODE mode, cmux_info_type *cmux_info)
{
    CMUXLIB *         cm;
    int status;
    struct mux_params mux;
    
    if (((mode & CMUX_TEMODE) != CMUX_TEMODE) 
        && ((mode & CMUX_UEMODE) != CMUX_UEMODE))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "mode error!\n");
        return (struct cmuxlib *) NULL;
    }

    cm = (CMUXLIB *) cmux_alloc (sizeof (struct cmuxlib));
    if (cm == NULL)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cm cmux_alloc error!\n");
        return (struct cmuxlib *) NULL;
    }
    cm->cmux_status = mode;
    /*default we can recv data from host*/
    cm->flow_ctrl = 1;/*lint !e63*/
    Cmux_get_default_mux_params (&mux, cmux_info);
    Cmux_set_default_sncdb (cm);
    
    // VALIDATE input mux structure is with proper values
    status = cmux_mux_param_validate (mux);
    if (0 == status)
    {
        if (cm != NULL)
        {
            cmux_free ((unsigned char *) cm);
        }
        
        CMUX_PRINT(CMUX_DEBUG_ERR, "mux validate error!\n");
        return (struct cmuxlib *) NULL;
    }
    cm->mp = mux;

    // Perform cmux_cmd, cmux_framer module initialisation, if any
    cmux_cmd_init (cm);

    return cm;
} /* Cmux_cmuxlib_init */


int Cmux_register_callback (struct cmuxlib *cm, struct cmux_callback *cmux_cb)
{
    if (cm == NULL)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cm NULL error!\n");
        return FAIL;
    }

    if (((cm->cmux_status & CMUX_TEMODE) != CMUX_TEMODE) 
        && ((cm->cmux_status & CMUX_UEMODE) != CMUX_UEMODE))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_status error!\n");
        return FAIL;
    }

    if ((cmux_cb->callback_func == NULL) || (cmux_cb->send_dl_data == NULL) 
        || (cmux_cb->set_power_cb == NULL))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "callbk NULL error!\n");
        return FAIL;
    }

    cm->cb.callback_func = cmux_cb->callback_func;
    cm->cb.send_dl_data   = cmux_cb->send_dl_data;
    cm->cb.set_power_cb = cmux_cb->set_power_cb;
    
    return PASS;
} /* Cmux_register_callback */


int Cmux_send_dlc_data (void *cmxlib, unsigned char *buff, unsigned short len, 
                        unsigned char dlc, unsigned int control, 
                        unsigned short flush_len, unsigned short no_of_frames)
{
    struct cmuxlib * cm = (struct cmuxlib *) cmxlib;
    struct frame *   fr;
    struct cmux_cmd *cmd_node;
    unsigned short i, fragment_len = 0;
    unsigned char *ptr = (unsigned char *) NULL;
    int rc  = FAIL;

    if ((NULL == cm) 
        || ((cm->cmux_status & (ATCMUX_ON | CTRL_DLC)) != (ATCMUX_ON | CTRL_DLC)) 
        || (dlc > MAX_DLC) 
        || (!(control & (CMUX_FRAGMENT_DATA | CMUX_FLUSH_DATA))))
    {
        return FAIL;
    }

    // Check for FC bit for that particular dlc
    if ((NULL == cm->dlc_db [dlc]) 
        || (!(CHECK_DLC_STATE (cm->dlc_db [dlc]->dlc_state, DLC_ESTABLISHED))) )
    {
        return FAIL;     // Not allowed any further
    }

    // Additional validations for CMUX_FRAGMENT_DATA
    if (control & CMUX_FRAGMENT_DATA)
    {
        if (NULL == buff || len > MAX_SERIAL_PKT_LEN || len == 0)
        {
            return FAIL;
        }
    }

    // Additional validations for CMUX_FLUSH_DATA
    if (control & CMUX_FLUSH_DATA)
    {
        if (flush_len > MAX_SERIAL_PKT_LEN)
        {
            return FAIL;
        }
    }

    // STEP 1: First fragment to smaller pieces if a huge-data than max-cmux-fr size, OR fill smaller pieces of data in to one-cmux frame if max-cmux-frame size is BIG
    // kick_start_rdy is set only if atleast one frame is full up to max-cmux-fr size
    if (control & CMUX_FRAGMENT_DATA)
    {
        // check dlc establised.
        if ((cm->dlc_db [dlc] != NULL) 
            && (CHECK_DLC_STATE (cm->dlc_db [dlc]->dlc_state, DLC_ESTABLISHED)))
        {
            ptr = buff;
            for (i = 0; i < len; )
            {
                if (len - i > MAX_TX_DATA_PKT_LEN (cm))/*lint !e574*/
                {
                    fragment_len = MAX_TX_DATA_PKT_LEN (cm);
                }
                else
                {
                    fragment_len = len - i;
                }

                // 2. Now making a new frame
                rc = Cmux_get_default_cmux_frame (cm, dlc, UIH_DATA, 
                                                &fr, &ptr [i], fragment_len);

                if (rc == PASS)
                {
                    cmd_node = cmux_make_cmd (fr, &cmd_node);
                    if (NULL == cmd_node)
                    {
                        return FAIL;
                    }
                    cmux_queue_cmd (cm, &cmd_node);
                }
                else
                {
                    break;
                }
                i += fragment_len;
            }
        }
    }

    if (control & CMUX_FLUSH_DATA)
    {
        if (!((cm->cmux_status & CMUX_UEMODE) 
            && (cm->dlc_db [dlc]) 
            && (cm->dlc_db [dlc]->msc.fc == 1)))
        {
            rc = cmux_kick_start_cmd (cm, dlc, flush_len, no_of_frames);
        }
    }
    return rc;
} /* Cmux_send_dlc_data */

int cmux_send_msc_cmd (struct cmuxlib *cm, unsigned char dlc, 
                    int kickstart, struct modem_status_cmd *msc)
{
    struct frame *   fr;
    struct cmux_cmd *cmd_node;
    int rc = 0;

    if ((NULL == cm) || (dlc > MAX_DLC) 
        || ((cm->cmux_status & (ATCMUX_ON | CTRL_DLC)) != (ATCMUX_ON | CTRL_DLC)))
    {
        return FAIL;
    }

    if (cm->dlc_db [dlc] == NULL)
    {
        return FAIL;
    }

    cm->dlc_db [dlc]->msc.dlci = msc->dlci;
    cm->dlc_db [dlc]->msc.fc   = msc->fc;
    cm->dlc_db [dlc]->msc.rtc  = msc->rtc;
    cm->dlc_db [dlc]->msc.rtr  = msc->rtr;
    cm->dlc_db [dlc]->msc.dv   = msc->dv;
    cm->dlc_db [dlc]->msc.ic   = msc->ic;

    /* 当前此函数仅做MSC主动上报用，用MSC_RESP使TypeCR位为0 */
    rc = Cmux_get_default_cmux_frame(cm, dlc, MSC_RESP, &fr, (unsigned char*)msc, 0);
    if (rc == PASS)
    {
        cmd_node = cmux_make_cmd (fr, &cmd_node);
        if (NULL == cmd_node)
        {
            return FAIL;
        }
        cmux_queue_cmd (cm, &cmd_node);
    }
    if (kickstart == 1)
    {
        rc = cmux_kick_start_cmd (cm, 0, 0, 0);
    }

    return rc;
} /* cmux_send_msc_cmd */

int Cmux_send_msc (struct cmuxlib *cm, unsigned char dlc, 
                struct modem_status_cmd *msc)
{
    int status = 0;

    // No DLC-param for DLC-0, also nothing to goahead with DLC-0 being up
    if ((NULL == cm) || (dlc > MAX_DLC) 
        || (0 == dlc) || (NULL == cm->dlc_db [0]) 
        || ((cm->cmux_status & (ATCMUX_ON | CTRL_DLC)) != (ATCMUX_ON | CTRL_DLC)))
    {
        return FAIL;
    }
    if ((cm->dlc_db [dlc] == NULL) 
        || (!(CHECK_DLC_STATE (cm->dlc_db [dlc]->dlc_state, DLC_ESTABLISHED))) )
    {
        CMUX_PRINT(CMUX_DEBUG_ERR,"dlc is not done!\n");
        return FAIL;     // Not allowed any further
    }
    status = cmux_send_msc_cmd (cm, dlc, 1, msc);

    return status;
}

int cmux_send_fcon_fcoff_cmd (struct cmuxlib *cm, unsigned char dlc, 
                            int kickstart, enum fr_type ftype)
{
    struct frame *   fr;
    struct cmux_cmd *cmd_node;
    int rc = 0;
    unsigned int *dlc0_state;
    int stop_proc = 0;

    if ((NULL == cm) || (dlc > MAX_DLC) 
        || ((cm->cmux_status & (ATCMUX_ON | CTRL_DLC)) != (ATCMUX_ON | CTRL_DLC)))
    {
        return FAIL;
    }

    if (cm->dlc_db [dlc] == NULL)
    {
        return FAIL;
    }
    
    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);
    if ( CHECK_DLC_STATE(*dlc0_state, UE_SENT_FCON_PENDING) ) 
    {
        CMUX_PRINT(CMUX_DEBUG_INFO, "FCON is pending, clear...\n");
        CLEAR_DLC_STATE (*dlc0_state, (UE_SENT_FCON_RESP_RECVD));
        stop_proc = 1;
    }
    if ( CHECK_DLC_STATE(*dlc0_state, UE_SENT_FCOFF_PENDING) )
    {
        CMUX_PRINT(CMUX_DEBUG_INFO, "FCOFF is pending, clear...\n");
        CLEAR_DLC_STATE (*dlc0_state, (UE_SENT_FCOFF_RESP_RECVD));
        stop_proc = 1;
    }
    if ( stop_proc )
    {
        cmux_stop_timer (cm, dlc);
        delete_head_cmd (cm, dlc);
    }
    if ( ftype == FCOFF_CMD )
    {
        CMUX_PRINT(CMUX_DEBUG_INFO, "FCOFF pending is set...\n");
        SET_DLC_STATE (*dlc0_state, (UE_SENT_FCOFF_PENDING));
    }
    else if ( ftype == FCON_CMD )
    {
        CMUX_PRINT(CMUX_DEBUG_INFO, "FCON pending is set...\n");
        SET_DLC_STATE (*dlc0_state, (UE_SENT_FCON_PENDING));
    }

    rc = Cmux_get_default_cmux_frame (cm, 0, ftype, &fr, 0, 0); // SABM for DLC
    if (rc == PASS)
    {
        cmd_node = cmux_make_cmd (fr, &cmd_node);
        if (NULL == cmd_node)
        {
            return FAIL;
        }
        if ( 0x01 == fr->pf_bit )
        {
            cmd_node->ct.res_timer_t2 = cm->mp.res_timer_t2;
            cmd_node->ct.retransmit_n2 = cm->mp.retransmit_n2;
        }
        cmux_queue_cmd (cm, &cmd_node);
    }
    if (kickstart == 1)
    {
        rc = cmux_kick_start_cmd (cm, 0, 0, 0);
    }

    return rc;
} /* cmux_send_fcon_fcoff_cmd */

int Cmux_send_fcon_fcoff (struct cmuxlib *cm, unsigned char dlc, enum fr_type ftype)
{
    int status = 0;

    if ((ftype != FCOFF_CMD) && (ftype != FCON_CMD))
    {
        return FAIL;
    }
    // No DLC-param for DLC-0, also nothing to goahead with DLC-0 being up
    if ((NULL == cm) || (dlc != 0) || (NULL == cm->dlc_db [0]) 
        || ((cm->cmux_status & (ATCMUX_ON | CTRL_DLC)) != (ATCMUX_ON | CTRL_DLC)))
    {
        return FAIL;
    }

    status = cmux_send_fcon_fcoff_cmd (cm, dlc, 1, ftype);

    return status;
} /* Cmux_send_fcon_fcoff */
#endif /* MBB_HSUART_CMUX */
