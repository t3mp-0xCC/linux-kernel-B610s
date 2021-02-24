


// Cmux common includes
#include "cmux_framer.h"
#include "cmux_cmd.h"
#include "cmux_api_ifc.h"
#include "cmux_drv_ifc.h"

// OSAL, platform specific includes
#include "cmux_tgt_osal.h"

#ifndef WIN32
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
#include <linux/string.h>
#endif /* WIN32 */

#if ( FEATURE_ON == MBB_HSUART_CMUX )
#ifndef TRUE
#define TRUE 1
#endif

extern unsigned char gcmux_active_ppp_pstask_dlc;
extern struct delayed_work at_cmux_expiry_timer;
extern unsigned int g_nsc_received;

int Cmux_get_default_cmux_frame (struct cmuxlib *, unsigned char, 
                enum fr_type, struct frame **, unsigned char *, unsigned int);
int cmux_free_frame (struct frame *);
int cmux_framer_init (struct cmuxlib *);
int cmux_validate_TE_UE_DLC_Params (struct dlc_param_neg *, 
                                struct dlc_param_neg *, enum NEGOTIATION_CMD);
int cmux_UE_cmd_handler (void *, void *);
int cmux_rxed_timer_expiry (struct cmuxlib *cm, unsigned char dlc);


int cmux_cmd_init (struct cmuxlib *cm)
{
    if (((cm->cmux_status & CMUX_TEMODE) != CMUX_TEMODE) 
        && ((cm->cmux_status & CMUX_UEMODE) != CMUX_UEMODE))
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_state error!\n");
        return FAIL;
    }

    cm->frame_map.cmux_cmd_handler = cmux_UE_cmd_handler;

    return (cmux_framer_init (cm));
} /* cmux_cmd_init */

void cmux_flush_cmd (struct cmuxlib *cm, unsigned char dlc_to_flush)
{
    struct cmux_cmd *node_to_free;
    struct cmux_cmd *head;

    head = cm->dlc_db [dlc_to_flush]->cmd_list;

    while (head)
    {
        node_to_free = head;
        head         = node_to_free->next;
        if (node_to_free->fr != NULL)
        {
            cmux_free_frame (node_to_free->fr);
        }
        cmux_free ((unsigned char *) node_to_free);
        node_to_free = NULL;
    }
    cm->dlc_db [dlc_to_flush]->cmd_list = (struct cmux_cmd *) NULL;
} /* cmux_flush_cmd */

/* Delete the cmd node belonging to a specific DLC
 * Every command deleted is always in the head
 */
void delete_head_cmd (struct cmuxlib *cm, unsigned char dlc)
{
    struct cmux_cmd *cmd_node;

    if (cm != NULL)
    {
        cmd_node = cm->dlc_db [dlc]->cmd_list;

        if (cmd_node != NULL)
        {
            if (CHECK_DLC_STATE (cm->dlc_db [dlc]->dlc_state, 
                                    cm->frame_map.pending_mask))
            {
                cmux_stop_timer (cm, dlc);
            }

            cm->dlc_db [dlc]->cmd_list = cmd_node->next;

            if (cmd_node->fr != NULL)
            {
                // update statistics..
                if ((UIH_DATA == cmd_node->fr->f_type) 
                    && (cm->cmux_status & CMUX_UEMODE))
                {
                    if (cm->dlc_db [dlc]->stats.pending_tx_data)
                    {
                        cm->dlc_db [dlc]->stats.pending_tx_data--;
                    }
                    if (cm->dlc_db [dlc]->stats.dlc_buffer_size 
                        && cm->dlc_db [dlc]->msc.fc == 1)
                    {
                        cm->dlc_db [dlc]->stats.dlc_buffer_size -= cmd_node->fr->length;
                    }
                }
                else
                {
                    cm->dlc_db [dlc]->stats.pending_tx_ctrl--;
                }

                cm->dlc_db [dlc]->stats.pending_tx_total--;
                cmux_free_frame (cmd_node->fr);
            }
            cmux_free ((unsigned char *) cmd_node);
            /*juvin<m2m-1638,1639,1641><Crash on wrong frame format,avoid dangling pointer>*/
            cmd_node = NULL;
        }
    }
} /* delete_head_cmd */

void Cmux_set_default_sncdb (struct cmuxlib *cm)
{
    int i;

    if (cm == NULL)
    {
        return;
    }

    for (i = 1; i <= MAX_DLC_TGT; i++)
    {
        cm->snc_db [i].dlc  = (unsigned char) i;
        cm->snc_db [i].task = SNC_TYPE_AT_DATA;
    }
} /* Cmux_set_default_sncdb */

int cmux_create_dlc_entry (struct cmuxlib *cm, unsigned char dlc)
{
    if (NULL == cm)
    {
        return FAIL;
    }

    if (cm->cmux_status & CMUX_UEMODE)
    {
        if (!(dlc <= MAX_DLC))
        {
            return FAIL;
        }
    }
    if (dlc == 0)
    {
        cm->dlc_count = 1;              // reset
    }
    if (dlc != 0)
    {
        cm->dlc_count++;
    }

    // check if already created..
    if (cm->dlc_db [dlc] == NULL)
    {
        cm->dlc_db [dlc] = (struct DLC_Entry *) cmux_alloc (sizeof (struct DLC_Entry));
        if(cm->dlc_db [dlc] == NULL) //juvin: A null check added
        {
            return FAIL;
        }
        /* Hari:<> Begin*/
        memset ((void *)cm->dlc_db [dlc], 0, sizeof (struct DLC_Entry));/*lint !e160 !e506 !e522*/

        cm->dlc_db [dlc]->dlc = dlc; // added newly
        /* HARI:<> End*/
        
        cm->dlc_db [dlc]->stats.pending_tx_ctrl  = 0;
        cm->dlc_db [dlc]->stats.pending_tx_data  = 0;
        cm->dlc_db [dlc]->stats.pending_tx_total = 0;
        cm->dlc_db [dlc]->stats.dlc_buffer_size  = 0;
        cm->dlc_db [dlc]->dlc_state |= (DLC_ENTRY_DONE | DLC_COM_READY);
        cm->dlc_db [dlc]->cmd_list  = (struct cmux_cmd *) NULL;

        // set DLC params from default set mux params
        cm->dlc_db [dlc]->dlcparam.muxparams  = cm->mp;
        cm->dlc_db [dlc]->dlcparam.conv_layer = CONVERGENCE_LAYER;
        cm->dlc_db [dlc]->dlcparam.priority   = DEFAULT_PRIORITY;

        if (dlc != 0 && (dlc > 0 && dlc < (MAX_DLC_TGT + 1)))
        {
            cm->dlc_db [dlc]->snc.dlci = dlc;
            if (cm->snc_db [dlc].task == SNC_TYPE_AT_DATA)
            {
                cm->dlc_db [dlc]->snc.s1 = 1;
            }
            else if (cm->snc_db [dlc].task == SNC_TYPE_AT)
            {
                cm->dlc_db [dlc]->snc.s1 = 0;
            }
#if 0
            else if (cm->snc_db [dlc].task == SNC_TYPE_DATA)
            {
                cm->dlc_db [dlc]->snc.s1 = 1;
            }
            else if (cm->snc_db [dlc].task == SNC_TYPE_GPS)
            {
                cm->dlc_db [dlc]->snc.s5 = 1;
            }
#endif
        }
        return PASS;
    }
    return FAIL;
} /* cmux_create_dlc_entry */

/******************************************************************************
Function:       cmux_delete_dlc_entry
Description:    关闭dlc指定的通道
Input:          None
Output:         None
Return:         None
Others:         None
******************************************************************************/
int cmux_delete_dlc_entry (struct cmuxlib *cm, unsigned char dlc)
{
    int i;
    if (cm == NULL || (dlc > MAX_DLC))
    {
        return FAIL;
    }

    if (dlc != 0)
    {
        cm->dlc_count--;
    }
    if ((0 == dlc) && (1 != cm->dlc_count))
    {
        for (i = 1; i <= MAX_DLC_TGT; i++)
        {
            if (cm->dlc_db[i])
            {
                cm->dlc_count--;
                cmux_flush_cmd (cm, i);
                cm->dlc_db [i]->cmd_list = (struct cmux_cmd*) NULL;
                cmux_free ((unsigned char*) cm->dlc_db [i]);
                cm->dlc_db [i] = (struct DLC_Entry*) NULL;
            }
        }
    }
    if (dlc == 0 && cm->dlc_count != 1)// !(only one left that is control-DLC)
    {
        return FAIL;// we dont allow to delete control DLC until all are deleted
    }

    // check if already created..
    if (cm->dlc_db [dlc] != NULL)
    {
        cmux_flush_cmd (cm, dlc);
        cm->dlc_db [dlc]->cmd_list = (struct cmux_cmd *) NULL;
        cmux_free ((unsigned char *) cm->dlc_db [dlc]);
        cm->dlc_db [dlc] = (struct DLC_Entry *) NULL;

        if (dlc == 0)
        {
            cm->cmux_status &= (~(ATCMUX_ON | CTRL_DLC));
            cm->cmux_status |= (ATCMUX_OFF);
        }
        return PASS;
    }
    return FAIL;
} /* cmux_delete_dlc_entry */

struct cmux_cmd *cmux_make_cmd (struct frame *fr, struct cmux_cmd **cmd)
{       // Put the frame pointer in the command frame
    // Based on the frame-type fill the Rx_Handler();
    // Based on the DLC of frame-type fill the CMUX_Timer value (timeout, retransmission values
    // Return the cmd;
    if (NULL == cmd)
    {
        return (struct cmux_cmd *) NULL;
    }

    *cmd = (struct cmux_cmd *) cmux_alloc (sizeof (struct cmux_cmd));
    if (NULL == *cmd)
    {
        return *cmd;
    }

    (*cmd)->fr = fr;

    // timer memset to zero
    (*cmd)->next = (struct cmux_cmd *) NULL;

    return *cmd;
} /* cmux_make_cmd */

int cmux_queue_cmd (struct cmuxlib *cm, struct cmux_cmd **cmd)
{
    // Get the DLC for which this is destined (cmd->frame->DLC)
    // Follow the list in DLC_Entry[DLC].cmd_list and Queue the command node
    unsigned char dlc;
    struct cmux_cmd * head;
    struct cmux_timer cmux_time;

    dlc = (*cmd)->fr->dlc;
    // update statistics..
    if (((*cmd)->fr->f_type == UIH_DATA) && (cm->cmux_status & CMUX_UEMODE))
    {
        cm->dlc_db [dlc]->stats.pending_tx_data++;

        if (cm->dlc_db [dlc]->msc.fc == 1)
        {
            cm->dlc_db [dlc]->stats.dlc_buffer_size += (*cmd)->fr->length;
        }
    }
    else
    {
        cm->dlc_db [dlc]->stats.pending_tx_ctrl++;
    }
    // DLC close or Mux close (for DLC-0)
    if (((*cmd)->fr->f_type == CTRL_DISC) || ((*cmd)->fr->f_type == CLD_CMD))
    {
        // No point in having anything queued..
        cmux_flush_cmd (cm, dlc);
    }
    if (cm->dlc_db [dlc]->cmd_list == NULL)
    {
        cm->dlc_db [dlc]->cmd_list = *cmd;
    }
    else
    {
        head = cm->dlc_db [dlc]->cmd_list;

        while (head->next != NULL)
        {
            head = head->next;
        }

        head->next   = *cmd;
        (*cmd)->next = (struct cmux_cmd *) NULL;
    }

    cm->dlc_db [dlc]->stats.pending_tx_total++;

    // For the added entry, if its response pending type.. copy the timeout requirements
    cmux_time.ack_timer_t1    = cm->dlc_db [dlc]->dlcparam.muxparams.ack_timer_t1;
    cmux_time.res_timer_t2    = cm->dlc_db [dlc]->dlcparam.muxparams.res_timer_t2;
    cmux_time.retransmit_n2   = cm->dlc_db [dlc]->dlcparam.muxparams.retransmit_n2;
    cmux_time.wakeup_timer_t3 = cm->dlc_db [dlc]->dlcparam.muxparams.wakeup_timer_t3;
    cmux_time.window_k        = cm->dlc_db [dlc]->dlcparam.muxparams.window_k;
    (*cmd)->ct                = cmux_time;
    return PASS;
} /* cmux_queue_cmd */

void cmux_invoke_uihdata_callback (void *cmxlib, struct frame *rxfr)
{
    int len_bkp  = 0;
    int len_vrfy = 0;
    int i;
    unsigned char * data_ptr_bkp = (unsigned char *) NULL;
    struct cmuxlib *cm           = (struct cmuxlib *) cmxlib;

    if (cm == NULL || rxfr == NULL || rxfr->uih_data == 0 || rxfr->length == 0)
    {
        return;
    }

    data_ptr_bkp = (unsigned char *) rxfr->uih_data;
    len_vrfy     = rxfr->length;
    len_bkp      = len_vrfy;

    for (i = 0; i < len_bkp; )
    {
        if (len_vrfy > MAX_SERIAL_PKT_LEN)
        {
            rxfr->uih_data = ((unsigned char *) rxfr->uih_data) + i;
            rxfr->length   = MAX_SERIAL_PKT_LEN;
            len_vrfy       = len_vrfy - MAX_SERIAL_PKT_LEN;
            (*(cm->cb.callback_func))(TE_SENT | RX_COMPLETE, (void *) rxfr, 
                                    (struct cmux_timer *) NULL);
        }
        else
        {
            rxfr->uih_data = ((unsigned char *) rxfr->uih_data) + i;
            rxfr->length   = (unsigned short int) len_vrfy;

            (*(cm->cb.callback_func))(TE_SENT | RX_COMPLETE, (void *) rxfr, 
                                    (struct cmux_timer *) NULL);
            break;
        }
        i += MAX_SERIAL_PKT_LEN;
    }
    rxfr->uih_data = (void *) data_ptr_bkp;
    rxfr->length   = (unsigned short int) len_bkp;

    return;
} /* cmux_invoke_uihdata_callback */

int cmux_send_to_com_fragmented (void *cmx, char *comname, 
                                unsigned char *rawptr, unsigned short len)
{
    struct cmuxlib *cm = (struct cmuxlib *) cmx;
    int rc = PASS;             // return code
    unsigned short i;

    for (i = 0; i < len; )
    {
        if ((len - i) > MAX_SERIAL_PKT_LEN)
        {
#ifdef WIN32
            rc = (*(cm->cb.send_to_com))(comname, rawptr + i, MAX_SERIAL_PKT_LEN);
#else
            rc = (*(cm->cb.send_dl_data))(rawptr + i, MAX_SERIAL_PKT_LEN);
#endif
            if (rc == FAIL)
            {
                break;
            }
        }
        else
        {
#ifdef WIN32
            rc = (*(cm->cb.send_to_com))(comname, rawptr + i, (len - i));
#else
            rc = (*(cm->cb.send_dl_data))(rawptr + i, (len - i));
#endif
            break;
        }
        i += MAX_SERIAL_PKT_LEN;
    }
    return rc;
} /* cmux_send_to_com_fragmented */

/* cmux_kick_start_cmd:
 *
    Dequeue from specific DLC commands one by one until pending flag is set
    Then transmit over Physical COM (or COM associated with DLC-0)
    After transmit kick start the timer for that command (if pending)
    If no ack-required free up that command move to next one

    maxlen defines the maximum frame size sent in one shot
    (maxlen DOES NOT guarantee that caller will get that much size, it only means atleast we will get that much size and wont CROSS that size.
    Which also means that we need to read-head the next frames (buffering) and give.. instead of giving frames one by one...

    no_of_frames defines how many frames to send

    if maxlen = 31, no_of_frames = 1, only one is sent, until 31 bytes are hit (non-stop, means in between pf bit should not be set)
    if maxlen = 500, no_of_frames = 1, loop accumulates up to 500, provided EOF-list or  pf bit should not be set (**Used in Target for first frame kick start)
    if maxlen = 0, no_of_frames = 5, dequeues and sends 5 - frames in NON-buffered manner
    if maxlen = 0, no_of_frames = 0, dequeues the full LIST,(**Typical use in Host for all control frames and Data)
    if maxlen = 31, no_of_frames = 0, deques full LIST but buffers in chunks of 31 bytes (provided no pf bit frames in between are encountered)

 */

int cmux_kick_start_cmd (struct cmuxlib *cm, unsigned char dlc, 
                        unsigned short maxlen, unsigned short no_of_frames)
{
    struct cmux_cmd *cmd_node;
    struct frame *   fr;
    int rc                      = PASS;              // return code
    unsigned short cmux_buffered_frame_len = 0;
    unsigned char *  cmux_buffered_frame;
    unsigned char jumbo_data_frame_mode   = 0;
    unsigned char filled_jumbo_data_frame = 0;

    // First check DLC exists ?
    if (cm->dlc_db [dlc] == NULL)
    {
        return FAIL;
    }

    cmd_node = cm->dlc_db [dlc]->cmd_list;

    // maxlen to be 0 or above MAX_CMUX_FRAME_SIZE
    if (maxlen > MAX_SERIAL_PKT_LEN)
    {
        return FAIL; // wrong parameter range
    }
    if (no_of_frames == 0)
    {
        no_of_frames = 0xffff;  // 0 means flush out the list
    }
    if (MAX_CMUX_FRAME_SIZE (cm) > MAX_SERIAL_PKT_LEN)
    {
        jumbo_data_frame_mode = 1;
        maxlen                = 0;
    }

    if (maxlen == 0)
    {
        cmux_buffered_frame = cmux_alloc (MAX_CMUX_FRAME_SIZE (cm));
    }
    else
    {
        cmux_buffered_frame = cmux_alloc (maxlen);
    }

    if (cmux_buffered_frame == NULL)
    {
        return FAIL;
    }

    // if DLC has data and we dont overflow maxlen (space for ONE Single 31 byte frame is there proceed.. else dont..
    // cmux_buffered_frame_len is zeroed out, once a tranmission is done, and it starts buffering until the desired no_of_frames is hit!
    while (cmd_node && no_of_frames)
    {
        // Means buffering enabled.. and value is above MAX_CMUX_FRAME_SIZE
        if (maxlen != 0 && jumbo_data_frame_mode == 0)
        {
            // We check if we can stand one more iteration where one-full-CMUX frame can be filled in ?
            if ((maxlen - cmux_buffered_frame_len) < MAX_CMUX_FRAME_SIZE (cm))/*lint !e574*/
            {
                rc = cmux_send_to_com_fragmented (cm, cm->com_port, 
                                cmux_buffered_frame, cmux_buffered_frame_len);
                cmux_buffered_frame_len = 0; // RESET after send to com
                no_of_frames--;
                if (no_of_frames == 0)
                {
                    break;
                }
            }
        }

        // Get the frame to be transmitted
        fr = cmd_node->fr;

        // If frame type is MUX CLD ensure to proceed only if all DLCs are closed, else let it stay in Q
        // It will transmitted only if all DLCs are down, this is a special case handling
        if (fr->f_type == CLD_CMD && cm->dlc_count != 1)
        {
            break;
        }

        if (cmux_struct_to_frame (&fr, cm) == FAIL)
        {
            // struct to frame conversion not proper; delete the node
            delete_head_cmd (cm, dlc);
            cmd_node = cm->dlc_db [dlc]->cmd_list;
            rc       = FAIL;
            break;
        }

        if (fr->raw_data == NULL)
        {
            // struct to frame conversion not proper; delete the node
            delete_head_cmd (cm, dlc); 
            cmd_node = cm->dlc_db [dlc]->cmd_list;
            rc       = FAIL;
            break;
        }

        // Jumbo frame support.. if we have filled to max,..send it
        // if (jumbo_data_frame_mode && fr->length == MAX_TX_DATA_PKT_LEN(cm))// j v n
        filled_jumbo_data_frame = 1;
        // else
        // filled_jumbo_data_frame = 0;

        // Conversion Done. Send it to COM (BUFFERED and non-buffered approach)
        // Suggest to aggregate data or not, for efficiency purposes
        // All  are termination conditions.. which means data MUST be sent to COM..
        // pf bit set, next is null or non-buffered mode (with cmux-frame size less than 672) or jumbo-data frame
        if ((1 == fr->pf_bit) || (NULL == cmd_node->next) 
            || (jumbo_data_frame_mode && filled_jumbo_data_frame) 
            || (jumbo_data_frame_mode == 0 && maxlen == 0))
        {
            // First send whatever that was buffered.. and send the current node as well
            // Best way to do it is append to buffer and send it as "one" send to come
            memcpy ((cmux_buffered_frame + cmux_buffered_frame_len), 
                    fr->raw_data, fr->raw_len);
            cmux_buffered_frame_len += fr->raw_len;

            // READY to send to com
            if (cmux_buffered_frame_len)
            {
                rc = cmux_send_to_com_fragmented(cm, cm->com_port, 
                                cmux_buffered_frame, cmux_buffered_frame_len);
                cmux_buffered_frame_len = 0; // RESET after send to com
                no_of_frames--;
            }
        }
        // lets buffer it ONLY if relevant like MAX_CMUX_FRAME_SIZE(cm) <= MAX_SERIAL_PKT_LEN ie,. NOT jumb frame mode
        else if (jumbo_data_frame_mode == 0)
        {
            // Means pf bit zero and next is there..
            // Buffer now.. dont send it
            memcpy ((cmux_buffered_frame + cmux_buffered_frame_len), fr->raw_data, fr->raw_len);
            cmux_buffered_frame_len += fr->raw_len;
        }

        // Inform result of processing the frame to upper layers
        if (rc == PASS)
        {
            (*(cm->cb.callback_func))(TE_SENT | TX_COMPLETE, (void *) fr,
                                      &(cmd_node->ct));
            cm->dlc_db [dlc]->dlc_state |=
                (cm->frame_map.frdetails [fr->f_type].dlc_next_state);
        }
        else
        {
            (*(cm->cb.callback_func))(TE_SENT | TX_FAILED, (void *) fr,
                                      &(cmd_node->ct));
            cm->dlc_db [dlc]->dlc_state |= 
                (cm->frame_map.frdetails [fr->f_type].dlc_next_state 
                | DLC_COMMAND_FAILED);

            // free up node and frame along with it as well
            delete_head_cmd (cm, dlc);
            cmd_node = cm->dlc_db [dlc]->cmd_list;
            // rc already fail
            break;
        }

        // Poll bit set to 1 means, sending station has to poll/wait for response..
        if (fr->pf_bit == 1)
        {
            // Needs response, so hold the node in list and kick-start the timer
            // Kick start the CMUX-timer (DLC, cmux_cmd*, timeout function)
            cmux_start_timer (cm, dlc, &(cmd_node->ct), cmux_rxed_timer_expiry);
            break; // cannot continue for any further transmission
        }
        else
        {
            delete_head_cmd (cm, dlc);
            cmd_node = cm->dlc_db [dlc]->cmd_list;
            continue;
        }
    } // while

    if (cmux_buffered_frame_len)
    {
        rc = cmux_send_to_com_fragmented (cm, cm->com_port, 
                                cmux_buffered_frame, cmux_buffered_frame_len);
        cmux_buffered_frame_len = 0; // RESET after send to com
    }

    if (cmux_buffered_frame != NULL)
    {
        cmux_free (cmux_buffered_frame);
        cmux_buffered_frame = NULL;
    }

    return rc;
} /* cmux_kick_start_cmd */

int cmux_rxed_timer_expiry (struct cmuxlib *cm, unsigned char dlc)
{
    struct cmux_cmd *cmd_node;
    unsigned int *           dlc_state, *dlcx_state;
    unsigned char dlcx;
    struct dlc_param_neg *   dlcpn_txed;
    struct modem_status_cmd *msc_txed;
    int rc = FAIL;

    if ((cm->cmux_status & ATCMUX_ON) != ATCMUX_ON)
    {
        cm->cb.callback_func (TIMEOUT_ON_ALL_RETRANSMISSION, 
                            NULL, (struct cmux_timer *) NULL);
        cmux_stop_timer (cm, dlc);
        return PASS;
    }

    if (cm->dlc_db [dlc] == NULL || cm->dlc_db [dlc]->cmd_list == NULL)
    {
        return FAIL;
    }
    if (!(cm->dlc_db [dlc]))
    {
        return FAIL;
    }
    cmd_node  = cm->dlc_db [dlc]->cmd_list;
    dlc_state = &(cm->dlc_db [dlc]->dlc_state);

    if (cmd_node == NULL)
    {
        return FAIL;
    }
    
    CMUX_PRINT(CMUX_DEBUG_INFO, "cmd_node->ct.retransmit_n2 = %d!\n", 
        cmd_node->ct.retransmit_n2);
    // Timed out case
    // check for retransmission, Did not exceed maximum Re-tries ?
    if (cmd_node->ct.retransmit_n2 > 1 /* 0 means Tx + 3 times, so 4 times */)
    {
        // Re-transmit the packet in cmd_node over DLC-0
        cmd_node->ct.retransmit_n2--;

        // Send for ALL the frames except MUX-CLD, as it is handled below in generic fashion
        if (!(cm->dlc_count == 1 && cm->dlc_db [0]->cmd_list->fr->f_type == CLD_CMD))
        {
            // kick start will inform upper layer about retransmission
            cmux_kick_start_cmd (cm, dlc, 0, 0);
        }
        rc = PASS;
    }
    else
    {
        if (cmd_node->fr->f_type == DLC_PN_CMD)
        {
            dlcpn_txed = (struct dlc_param_neg *) cmd_node->fr->uih_data;
            dlcx       = dlcpn_txed->dlci;
            if (!(cm->dlc_db [dlcx]))
            {
                return FAIL;
            }
            dlcx_state = &(cm->dlc_db [dlcx]->dlc_state);
            SET_DLC_STATE (*dlcx_state, (DLC_COMMAND_FAILED | DLC_CLOSED));
            CLEAR_DLC_STATE (*dlcx_state, (~DLC_ESTABLISHED));
            delete_head_cmd (cm, 0);
            cmux_delete_dlc_entry (cm, dlcx);
            rc = FAIL;
        }
        else
        {
            if (cmd_node->fr->f_type == FCON_CMD)
            {
                dlc_state = &(cm->dlc_db [dlc]->dlc_state);
                SET_DLC_STATE (*dlc_state, (DLC_NON_CRIT_COMMAND_FAILED));
                if ( CHECK_DLC_STATE(*dlc_state, UE_SENT_FCON_PENDING) ) 
                {
                    CMUX_PRINT(CMUX_DEBUG_INFO, "FCON is pending, clear...\n");
                    CLEAR_DLC_STATE (*dlc_state, (UE_SENT_FCON_RESP_RECVD));
                }
                delete_head_cmd (cm, dlc);
                rc = FAIL;
            }
            else if (cmd_node->fr->f_type == FCOFF_CMD)
            {
                dlc_state = &(cm->dlc_db [dlc]->dlc_state);
                SET_DLC_STATE (*dlc_state, (DLC_NON_CRIT_COMMAND_FAILED));
                if ( CHECK_DLC_STATE(*dlc_state, UE_SENT_FCOFF_PENDING) )
                {
                    CMUX_PRINT(CMUX_DEBUG_INFO, "FCOFF is pending, clear...\n");
                    CLEAR_DLC_STATE (*dlc_state, (UE_SENT_FCOFF_RESP_RECVD));
                }
                delete_head_cmd (cm, dlc);
                rc = FAIL;
            }
            else if (cmd_node->fr->f_type == TEST_CMD)
            {
                dlc_state = &(cm->dlc_db [dlc]->dlc_state);
                SET_DLC_STATE (*dlc_state, (DLC_NON_CRIT_COMMAND_FAILED));
                delete_head_cmd (cm, dlc);
                rc = FAIL;
            }
            else if (cmd_node->fr->f_type == PSC_CMD)
            {
                dlc_state = &(cm->dlc_db [dlc]->dlc_state);
                SET_DLC_STATE (*dlc_state, (DLC_NON_CRIT_COMMAND_FAILED));
                delete_head_cmd (cm, dlc);
                rc = FAIL;
            }
            else if (cmd_node->fr->f_type == MSC_CMD)
            {
                dlc_state  = &(cm->dlc_db [dlc]->dlc_state);
                msc_txed   = (struct modem_status_cmd *) cmd_node->fr->uih_data;
                dlcx       = msc_txed->dlci;
                if (dlcx > MAX_DLC)
                {
                    return FAIL;
                }
                dlcx_state = &(cm->dlc_db [dlcx]->dlc_state);
                SET_DLC_STATE (*dlcx_state, (DLC_NON_CRIT_COMMAND_FAILED));
                SET_DLC_STATE (*dlc_state, (DLC_NON_CRIT_COMMAND_FAILED));
                delete_head_cmd (cm, dlc);
                rc = FAIL;
            }
            else if (cmd_node->fr->f_type == SNC_CMD)
            {
                dlc_state = &(cm->dlc_db [dlc]->dlc_state);
                // snc_txed  = (struct snc_cmd *) cmd_node->fr->uih_data;/*juvin:lint fix*/
                // dlcx       = msc_txed->dlci;
                // dlcx_state = &(cm->dlc_db [dlcx]->dlc_state);

                SET_DLC_STATE (*dlc_state, (DLC_NON_CRIT_COMMAND_FAILED));
                delete_head_cmd (cm, dlc);
                // delete_head_cmd (cm, dlcx);
                rc = FAIL;
            }
            else
            {
                // Will come out as COMMAND RESPONSE pending + DLC Failed flag set
                SET_DLC_STATE (*dlc_state, (DLC_COMMAND_FAILED | DLC_CLOSED));
                CLEAR_DLC_STATE (*dlc_state, (~DLC_ESTABLISHED));

                (*(cm->cb.callback_func))(DLC_STATE_CHANGE, 
                                        (void *) dlc, (struct cmux_timer *) NULL);

                cmux_delete_dlc_entry (cm, dlc);
                rc = FAIL;
            }
        }
    }

    // LAST DLC check: Put in common cleanup
    if (cm->dlc_count == 1)
    {
        if (cm->dlc_db [0] != NULL && cm->dlc_db [0]->cmd_list != NULL
            && cm->dlc_db [0]->cmd_list->fr->f_type == CLD_CMD)
        {
            cmux_kick_start_cmd (cm, 0, 0, 0);
        }
    }
    return rc;
} /* cmux_rxed_timer_expiry */

void cmux_get_default_dlc_param_nego (struct dlc_param_neg *dlcpn_default,
                                    unsigned char dlc)
{
    dlcpn_default->dlci       = dlc;
    dlcpn_default->priority   = DEFAULT_PRIORITY;
    dlcpn_default->conv_layer = CONVERGENCE_LAYER;

    dlcpn_default->muxparams.ack_timer_t1  = 10;
    dlcpn_default->muxparams.frame_size_n1 = 31;
    dlcpn_default->muxparams.retransmit_n2 = 3;
    dlcpn_default->muxparams.window_k      = 2;
    dlcpn_default->muxparams.res_timer_t2  = 0;
}

int cmux_validate_TE_UE_DLC_Params (struct dlc_param_neg *dlcpn_tx,
                                    struct dlc_param_neg *dlcpn_rx,
                                    enum NEGOTIATION_CMD neg_cmd)
{
    if ((neg_cmd == MODIFY)
        && (dlcpn_tx->muxparams.frame_size_n1 !=
            dlcpn_rx->muxparams.frame_size_n1))
    {
        dlcpn_rx->muxparams.frame_size_n1 = dlcpn_tx->muxparams.frame_size_n1;
        dlcpn_rx->muxparams.ack_timer_t1  = dlcpn_tx->muxparams.ack_timer_t1;
        dlcpn_rx->muxparams.window_k      = dlcpn_tx->muxparams.window_k;
        dlcpn_rx->muxparams.retransmit_n2 = dlcpn_tx->muxparams.retransmit_n2;
        dlcpn_rx->dlci                    = dlcpn_tx->dlci;
        dlcpn_rx->priority                = dlcpn_tx->priority;
        dlcpn_rx->conv_layer              = dlcpn_tx->conv_layer;
        return PASS;
    }
    else if ((neg_cmd == COMPARE) && (dlcpn_tx->conv_layer == dlcpn_rx->conv_layer)
             && (dlcpn_tx->dlci == dlcpn_rx->dlci)
             && (dlcpn_tx->priority == dlcpn_rx->priority)
             && (dlcpn_tx->muxparams.ack_timer_t1 ==
                 dlcpn_rx->muxparams.ack_timer_t1)
             && (dlcpn_tx->muxparams.frame_size_n1 ==
                 dlcpn_rx->muxparams.frame_size_n1)
             && (dlcpn_tx->muxparams.retransmit_n2 ==
                 dlcpn_rx->muxparams.retransmit_n2)
             && (dlcpn_tx->muxparams.window_k == dlcpn_rx->muxparams.window_k))
    {
        return MATCH;
    }
    else
    {
        return NO_MATCH;
    }
} /* cmux_validate_TE_UE_DLC_Params */

/* cmux_TE_UE_rxed_DM: CTRL_DM Received
 * Merger of two functions: cmux_UE_rxed_DM AND cmux_TE_rxed_DM

   In HOST SIDE behaviour is as follows:

 * Already verified for:
 * Accept Flags: DLC_CLOSED or TE_PENDING_MASK
 * Reject Flags: NO_REJECT_FLAGS
 *
 * DM response received means its a NACK for SABM (or any command that is pending for a response) or TE sent a command to DLC
 * which is "already down"
 * However it does not necessarily mean that TE is pending for a response, DM can be sent for any command by UE
 * It only means UE not willing to negotiate now, so if any pending commands are there clear them off and close-DLC
 * This cannot be called in timeout case

   In TARGET SIDE behaviour is as follows:

 * cmux_UE_rxed_DM: CTRL_DM Received
 * Already verified for:
 * Accept Flags: UE_SENT_DISC_CMD_UA_PENDING
 * Reject Flags: NO_REJECT_FLAGS
 *
 * Target receives DM response only if DISC cmd is sent from Target
 * to a wrong DLC number or if DLC is already down in TE side
 *
 * This ideally should not be done by target, however for frame-reception point of
 * view handler MUST be present
 *
 * Also if Target has sent DISC cmd sent to a right DLC, it is closed by UA response from host
 * If DLC-down then target has sent for wrong DLC
 * So only possible action is "if UE_DLC_ESTABLISHED or any DISC cmd send and response pending close timer and the DLC"
 */
int cmux_TE_UE_rxed_DM (void *cmxlib, void *frm, unsigned char dlc)
{
    struct cmux_cmd *cmd_node;
    struct frame *   txfr;
    unsigned int *   dlc_state;

    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    // CHECK not required, as its already done ahead.. for pclint it was done
    if (cm->dlc_db [dlc] == NULL)
    {
        if ( (cm->cmux_status & CMUX_TEMODE) == CMUX_TEMODE)
        {
            (*(cm->cb.callback_func))(UE_SENT | RX_COMPLETE, 
                                    (void *) rxfr, (struct cmux_timer *) NULL);
        }
        return PASS;
    }
    cmd_node = cm->dlc_db [dlc]->cmd_list;

    if (cmd_node != NULL)
    {
        txfr = cmd_node->fr;
    }
    else
    {
        txfr = (struct frame *) NULL;
    }
    dlc_state = &(cm->dlc_db [dlc]->dlc_state);

    // case 1: SABM sent DM received, UE not willing to start DLC
    if ((*dlc_state) & cm->frame_map.pending_mask)
    {
        cmux_stop_timer (cm, dlc);
        if (txfr != NULL)
        {
            // Removes DLC PN, and next command is moved ahead
            delete_head_cmd (cm, txfr->dlc);
        }
    }
    // case 2: DM response received AND NO-response pending. ie,. Un-solicited DM response
    // DM response received for no-frame transmission on a Healthy DLC.
    SET_DLC_STATE (*dlc_state, (DLC_CLOSED));
    CLEAR_DLC_STATE (*dlc_state, (~DLC_ESTABLISHED));

    // Call upper layer and inform
    if ( (cm->cmux_status & CMUX_TEMODE) == CMUX_TEMODE)
    {
        (*(cm->cb.callback_func))(UE_SENT | RX_COMPLETE, 
                                (void *) rxfr, (struct cmux_timer *) NULL);
    }
    (*(cm->cb.callback_func))(DLC_STATE_CHANGE, (void *)dlc, (struct cmux_timer *)NULL);
    return PASS;
} /* cmux_TE_UE_rxed_DM */

int cmux_UE_send_response (struct cmuxlib *cm, unsigned char dlc, 
                        enum fr_type frame_type, void *muxframe)
{
    struct frame * fr = (struct frame *) NULL, *frm;
    int rc;
    unsigned char *buf;
    unsigned short length;
    
    if (((frame_type == TEST_RESP) || (frame_type == MSC_RESP) 
        || (frame_type == DLC_PN_RESP) || (frame_type == SNC_RESP)) 
        && (muxframe == NULL))
    {
        return FAIL;
    }
    if (((frame_type == CTRL_SABM) || (frame_type == CTRL_UA) 
        || (frame_type == CTRL_DM) || (frame_type == CTRL_DISC) 
        || (frame_type == CTRL_DM) || (frame_type == FCON_RESP)) 
        && (muxframe != NULL))
    {
        return FAIL;
    }

    if (frame_type == TEST_RESP)
    {
        frm = (struct frame *) muxframe;
        buf = (unsigned char*) frm->uih_data;
        if (TEST_LEN_LIMIT >= frm->length)
        {
            length = frm->length - 2;
        }
        else
        {
            length = frm->length - 3;
        }
        rc = Cmux_get_default_cmux_frame (cm, dlc, frame_type, 
                                        &fr, (unsigned char *) buf, length);
        if (FAIL == rc)
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Cmux_get_default_cmux_frame failed1!\n");
            return FAIL;
        }
    }
    else
    {
        rc = Cmux_get_default_cmux_frame (cm, dlc, frame_type, 
                                        &fr, (unsigned char *) muxframe, 0);
        if (FAIL == rc)
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "Cmux_get_default_cmux_frame failed2!\n");
            return FAIL;
        }
    }

    // Fill in the values.
    if (cmux_struct_to_frame (&fr, cm) == FAIL)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_struct_to_frame failed!\n");
        return FAIL;
    }

    rc = cmux_send_to_com_fragmented (cm, cm->com_port, fr->raw_data, fr->raw_len);
    if (FAIL == rc)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_send_to_com_fragmented failed!\n");
        (*(cm->cb.callback_func))(TE_SENT | TX_FAILED, 
                                (void *) fr, (struct cmux_timer *) NULL);
    }
    cmux_free_frame (fr); // Free up the just-sent frame
    return PASS;
} /* cmux_UE_send_response */

void mux_close_data_service(unsigned char Index)
{
    cmux_modem_msc_stru msc_stru;

    memset((void *)&msc_stru, 0, sizeof(cmux_modem_msc_stru));/*lint !e160 !e506 !e522*/

    msc_stru.OP_Dtr = 1;
    msc_stru.ucDtr = 0;

    CMUX_PRINT(CMUX_DEBUG_INFO, "Index is %d, MSC dtr is %d!\r\n", 
                Index, msc_stru.ucDtr);
    cmux_send_msc_status(Index, &msc_stru);

    return;
}

/* cmux_cmd_handler:
 * called by Cmux_Input_frame_to_CMUX, which will try and settle the received frame
 *
 * In UE side (Target), it handles SABM, DISC here itself as part of generic code itself unlike TE
 * as its simpler if done here
 *
 * For other frames handled through pointers

        For DLC param neg. it happens on DLC0, and DLC X not created
        DLC X create only on reception of SBAM and if DLC is enabled
 */
int cmux_UE_cmd_handler (void *cmxlib, void *frm)
{
    const struct framedetails *frame_details;
    int rc = FAIL;
    unsigned int *             dlc_state;

    struct cmuxlib *cm = (struct cmuxlib *) cmxlib;
    struct frame *  fr = (struct frame *) frm;

    if (NULL == cm)
    {
        return rc;
    }
    
    if ((fr->f_type == CTRL_SABM) && (cm->dlc_db [0] == NULL) && (fr->dlc != 0))
    {
        // to send dm response in case if the dlc0 is not created
        cmux_UE_send_response (cm, fr->dlc, CTRL_DM, NULL);
        rc = PASS;
        return rc;
    }
    if ((fr->f_type != CTRL_SABM) && (NULL == cm->dlc_db[0]))
    {
        // to send dm response in case if the dlc0 is not created
        cmux_UE_send_response (cm, fr->dlc, CTRL_DM, NULL);
        rc = PASS;
        return rc;
    }
    if (MAX_DLC_TGT < fr->dlc)
    {
        cmux_UE_send_response (cm, fr->dlc, CTRL_DM, NULL);
        rc = PASS;
        return rc;
    }

    do
    {
        if (fr->f_type == CTRL_SABM || fr->f_type == CTRL_DISC)
        {
            // Handle for SABM (causes DLC entry creation) and DISC cmd on reception
            if (cm->dlc_db [fr->dlc] == NULL)
            {
                // DLC disabled (any frame type) or got DISC for non-existing DLC-entry -> DM
                if ((CTRL_DISC == fr->f_type) && (NULL == cm->dlc_db[fr->dlc]))
                {
                    cmux_UE_send_response (cm, fr->dlc, CTRL_DM, NULL);
                    rc = PASS;
                    break;
                }

                // Got a frame for non-exisiting/created DLC, Check if its SABM,then UA by default, if DISC, then send DM (as DLC is already down)
                if (fr->f_type == CTRL_SABM)
                {
                    cmux_UE_send_response(cm, fr->dlc, CTRL_UA, NULL);
                    cmux_create_dlc_entry(cm, fr->dlc);
                    SET_DLC_STATE(cm->dlc_db [fr->dlc]->dlc_state, 
                                UE_RECVD_SABM_CMD_UA_SENT | DLC_ESTABLISHED);
                    CLEAR_DLC_STATE(cm->dlc_db[fr->dlc]->dlc_state, (~DLC_CLOSED));

                    // UE side enable DLC-0
                    if (fr->dlc == 0)
                    {
                        cm->cmux_status |= (ATCMUX_ON | CTRL_DLC);
                        
                        /*如果SABM 0帧成功建立了通道，关闭timer定时器*/
                        CMUX_timer_end(&at_cmux_expiry_timer);
                    }
                    
                    if ((0 != fr->dlc) && ((fr->dlc > 0) 
                        && (fr->dlc <= MAX_DLC_TGT)))
                    {
                        /* 创建非0的DLC通道上报管脚状态 */
                        (*(cm->cb.callback_func))(DLC_ESTABLISH_COMPLETE, 
                                    (void *)fr->dlc, (struct cmux_timer *) NULL);
                    }
                    
                    (*(cm->cb.callback_func))(DLC_STATE_CHANGE, 
                                (void *) fr->dlc, (struct cmux_timer *) NULL);
                }
            }
            else
            {
                // DLC entry is made, which means its Enabled/created, if DISC/SABM send UA
                dlc_state = &(cm->dlc_db [fr->dlc]->dlc_state);

                if (fr->f_type == CTRL_SABM)
                {
                    SET_DLC_STATE (*dlc_state, 
                        UE_RECVD_SABM_CMD_UA_SENT | DLC_ESTABLISHED);
                    CLEAR_DLC_STATE (*dlc_state, (~DLC_CLOSED));
                }

                if (fr->f_type == CTRL_DISC)
                {
                    SET_DLC_STATE (*dlc_state, UE_RECVD_DISC_CMD_UA_SENT | DLC_CLOSED);
                    CLEAR_DLC_STATE (*dlc_state, (~DLC_ESTABLISHED));
                    mux_close_data_service(fr->dlc);
                }

                cmux_UE_send_response (cm, fr->dlc, CTRL_UA, NULL);
                (*(cm->cb.callback_func))(DLC_STATE_CHANGE, 
                                (void *) fr->dlc, (struct cmux_timer *) NULL);
            }
            rc = PASS;
            break;
        } // Master check for SABM/DISC frame type

        // Now pending frames are Ctrl UA, Ctrl DM, MUX-commands, UIH-data, for which DLC-Entry MUST be created..
        if (cm->dlc_db [fr->dlc] == NULL)
        {
            // to send dm response in case if the dlc0 is not created
            cmux_UE_send_response (cm, fr->dlc, CTRL_DM, NULL);
            rc = PASS;
            break;
        }

        // Check accept & reject flags
        // Current DLC state && DLC states Allowed for that particular frame format
        // In main data path, no need to check for UE/TE its already loaded at init-time,one-time
        frame_details = &(cm->frame_map.frdetails [fr->f_type]);

        if (((frame_details->accept_flag & cm->dlc_db[fr->dlc]->dlc_state) == 0) 
            || ((frame_details->reject_flag & cm->dlc_db[fr->dlc]->dlc_state) != 0))
        {
            rc = FAIL; // Not allowed any further
            break;
        }

        // Now go for specific frame type handling...approach
        rc = (*(frame_details->frame_rx_handler))(cm, fr, fr->dlc);
    } while (0);

    // General DLC-clean up job..
    if ((cm->dlc_db [fr->dlc] != NULL) 
        && (CHECK_DLC_STATE (cm->dlc_db [fr->dlc]->dlc_state, DLC_CLOSED)))
    {
        cmux_delete_dlc_entry (cm, fr->dlc);
    }

    if ((cm->cmux_status & ATCMUX_OFF) == ATCMUX_OFF)
    {
        SetCmuxClosePort(TRUE);
    }

    // LAST DLC check: Put in common cleanup
    if (fr->f_type == CTRL_UA && cm->dlc_count == 1)
    {
        if ((cm->dlc_db [0] != NULL) 
            && (cm->dlc_db [0]->cmd_list != NULL) 
            && (cm->dlc_db [0]->cmd_list->fr->f_type == CLD_CMD))
        {
            cmux_kick_start_cmd (cm, 0, 0, 0);
        }
    }
    return rc;
} /* cmux_UE_cmd_handler */

/* cmux_UE_rxed_UA: CTRL_UA Received
 *
 * Already verified for:
 * Accept Flags: UE_SENT_DISC_CMD_UA_PENDING or UE_DLC_ESTABLISHED
 * Reject Flags: UE_DLC_CLOSED
 *
 * Target receives UA response only if DISC cmd is sent from Target
 * and it is the only command that is waiting for response etc,.
 *
 *
 */
int cmux_UE_rxed_UA (void *cmxlib, void *frm, unsigned char dlc)
{
    struct cmux_cmd *cmd_node;
    struct frame *   txfr;
    unsigned int *   dlc_state;

    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    if (cm == NULL)
    {
        return FAIL;
    }
    cmd_node = cm->dlc_db [dlc]->cmd_list;
    if (NULL == cmd_node)
    {
        return FAIL;
    }
    else
    {
        txfr = cmd_node->fr;
    }

    dlc_state = &(cm->dlc_db [dlc]->dlc_state);

    // Check sanity of values
    if (txfr == NULL)       // Ideally should not get txfr NULL
    {
        return FAIL;
    }
    if (cmd_node->fr->pf_bit != 1)
    {
        return FAIL;
    }

    // Normal reception
    // Check sanity of values
    if (rxfr->dlc != txfr->dlc)   // this MUST never happen
    {
        return FAIL;
    }

    // Stop the timer (as it is not required now)
    cmux_stop_timer (cm, dlc);

    if (CHECK_DLC_STATE (*dlc_state, UE_SENT_DISC_CMD_UA_PENDING))
    {
        CLEAR_DLC_STATE (*dlc_state, UE_SENT_DISC_CMD_UA_RECVD);
        CLEAR_DLC_STATE (*dlc_state, (~DLC_ESTABLISHED));
        SET_DLC_STATE (*dlc_state, (DLC_CLOSED));
        (*(cm->cb.callback_func))(DLC_STATE_CHANGE, 
                                (void *) dlc, (struct cmux_timer *) NULL);
    }

    // Free-up the txfr, and rxfr
    delete_head_cmd (cm, dlc);

    // Delete DLC entry is done in generic fashion
    return PASS;
} /* cmux_UE_rxed_UA */

int cmux_UE_rxed_CLD_RESP (void *cmxlib, void *frm, unsigned char dlc)
{
    unsigned int *dlc_state;
    int i, rc = FAIL;
    
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    if ((cm == NULL) 
        || ((cm->cmux_status & ATCMUX_ON) != ATCMUX_ON) 
        || (rxfr == NULL))
    {
        return rc;
    }

    for (i = 1; i <= MAX_DLC_TGT; i++)
    {
        if (cm->dlc_db [i] != NULL)
        {
            // check for flags,. estbalisehd etc,. and delete the dlc
        }
    }

    if (cm->dlc_db [dlc] != NULL)
    {
        dlc_state = &(cm->dlc_db [dlc]->dlc_state);

        if (CHECK_DLC_STATE (*dlc_state, UE_SENT_MUX_CLD_RESP_PENDING))
        {
            cmux_stop_timer (cm, dlc);
            SET_DLC_STATE (*dlc_state, (UE_SENT_MUX_CLD_RESP_RECVD | DLC_CLOSED));
            CLEAR_DLC_STATE (*dlc_state, (~DLC_ESTABLISHED));
            cm->cb.callback_func (DLC_STATE_CHANGE, 
                                (void *) 0, (struct cmux_timer *) NULL);
        }

        // Free-up
        cm->cmux_status &= (~(ATCMUX_ON | CTRL_DLC));
    }

    return PASS;
} /* cmux_UE_rxed_CLD_RESP */

/* cmux_UE_rxed_UIHDATA: UIH_DATA Received
 *
 * Already verified for:
 * Accept Flags: DLC_ESTABLISHED
 * Reject Flags: DLC_CLOSED or UE_PENDING_MASK
 *
 * Means DLC !=0 and it is UIH-Data framed in to CMUX-frame
 * send data to consumer (watermark IDs of mapped DLC)
 */
int cmux_UE_rxed_UIHDATA (void *cmxlib, void *frm, unsigned char dlc)
{
    struct frame *  rxfr = (struct frame *) frm;
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    int rc   = FAIL;

    cmux_invoke_uihdata_callback (cm, rxfr);
    rc = PASS;
    return rc;
} /* cmux_UE_rxed_UIHDATA */

int cmux_UE_rxed_FCON_cmd (void *cmxlib, void *frm, unsigned char dlc)
{
    unsigned int *dlc0_state;
    int rc   = FAIL;
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;
    int i;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }
    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);
    SET_DLC_STATE (*dlc0_state, (UE_RECVD_FCON_RESP_SENT));
    for (i = 1; i <= MAX_DLC_TGT; i++)
    {
        if (cm->dlc_db [i] == NULL)
        {
            continue;
        }
        cm->dlc_db [i]->msc.fc = 0;
    }
    cmux_UE_send_response (cm, rxfr->dlc, FCON_RESP, NULL);
    for (i = 0; i <= MAX_DLC_TGT; i++)
    {
        if (cm->dlc_db [i] == NULL)
        {
            continue;
        }

        if (cm->dlc_db [i]->cmd_list)
        {
            cmux_kick_start_cmd (cm, (unsigned char) i, 0, 0);
            cm->dlc_db [i]->stats.dlc_buffer_size = 0;
        }
    }
    return PASS;
} /* cmux_UE_rxed_FCON_cmd */

int cmux_UE_rxed_FCOFF_cmd (void *cmxlib, void *frm, unsigned char dlc)
{
    unsigned int *dlc0_state;
    int rc   = FAIL;
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;
    int i;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }
    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);
    SET_DLC_STATE (*dlc0_state, (UE_RECVD_FCOFF_RESP_SENT));
    
    for (i = 1; i <= MAX_DLC_TGT; i++)
    {
        if ((cm->dlc_db [i] == NULL) 
            || (!(CHECK_DLC_STATE (cm->dlc_db [i]->dlc_state, DLC_ESTABLISHED))))
        {
            continue;
        }
        cm->dlc_db [i]->msc.fc = 1;
    }
    
    cmux_UE_send_response (cm, rxfr->dlc, FCOFF_RESP, NULL);
    return PASS;
} /* cmux_UE_rxed_FCOFF_cmd */

int cmux_UE_rxed_Test_cmd (void *cmxlib, void *frm, unsigned char dlc)
{
    int rc = FAIL;
    unsigned int *  dlc0_state;
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }

    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);

    SET_DLC_STATE (*dlc0_state, (UE_RECVD_TEST_CMD_RESP_SENT));
    cmux_UE_send_response (cm, rxfr->dlc, TEST_RESP, (void *) rxfr);

    return PASS;
} /* cmux_UE_rxed_Test_cmd */

void Cmux_msc_handler(struct modem_status_cmd *mscrxed)
{
    cmux_modem_msc_stru msc_stru;
    unsigned char Index;

    if (NULL == mscrxed)
    {
        CMUX_PRINT(CMUX_DEBUG_ERR, "mscrxed is NULL!\r\n");
        return;
    }
    
    memset((void *)&msc_stru, 0, sizeof(cmux_modem_msc_stru));/*lint !e160 !e506 !e522*/
    Index = mscrxed->dlci;

    msc_stru.OP_Dtr = 1;
    msc_stru.ucDtr = mscrxed->rtc;

    cmux_send_msc_status(Index, &msc_stru);
    
    return;
}

int cmux_UE_rxed_MSC_cmd (void *cmxlib, void *frm, unsigned char dlc)
{
    unsigned int *           dlc0_state;
    unsigned char dlcx;
    int rc = FAIL;
    struct modem_status_cmd *mscrxed;
    struct cmuxlib *         cm   = (struct cmuxlib *) cmxlib;
    struct frame *           rxfr = (struct frame *) frm;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }

    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);
    mscrxed    = (struct modem_status_cmd *) rxfr->uih_data;
    dlcx       = mscrxed->dlci;
    SET_DLC_STATE (*dlc0_state, (UE_RECVD_MSC_RESP_SENT));
    if(dlcx > MAX_DLC)
    {
        return FAIL;
    }
    if (cm->dlc_db [dlcx] == NULL)
    {
        return FAIL;
    }
    if (!((CHECK_DLC_STATE (cm->dlc_db [dlcx]->dlc_state, DLC_ESTABLISHED))))
    {
        return FAIL;
    }
    cm->dlc_db [dlcx]->msc.fc = mscrxed->fc;

    /* 根据下发的DTR位的消息，做对应的处理 */
    Cmux_msc_handler(mscrxed);
    
    // Carefully modifiy other flags/fields in rx-frame
    cmux_UE_send_response (cm, rxfr->dlc, MSC_RESP, (void *) mscrxed);
    if (cm->dlc_db [dlcx]->msc.fc == 0)
    {
        if (cm->dlc_db [dlcx]->cmd_list)
        {
            cmux_kick_start_cmd (cm, dlcx, 0, 0);
            cm->dlc_db [dlcx]->stats.dlc_buffer_size = 0;
        }
    }
    return PASS;
} /* cmux_UE_rxed_MSC_cmd */

int cmux_UE_rxed_PSC_cmd (void *cmxlib, void *frm, unsigned char dlc)
{
    int rc = FAIL;
    unsigned int *  dlc0_state;
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }

    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);
    SET_DLC_STATE (*dlc0_state, (UE_RECVD_PSC_RESP_SENT));
    cmux_UE_send_response (cm, rxfr->dlc, PSC_RESP, NULL);
    
    if (cm->cb.set_power_cb)
    {
        rc = cm->cb.set_power_cb();
        if ( FAIL == rc )
        {
            CMUX_PRINT(CMUX_DEBUG_ERR, "cmux_set_power_status failed!\n");
            return rc;
        }
    }
   
    return PASS;
} /* cmux_UE_rxed_PSC_cmd */

void Cmux_snc_set_dlc_process (unsigned int dlcx, 
                            struct snc_cmd *sncrxed, struct cmuxlib *cm)
{
    unsigned int i;
    struct snc_cmd snc_tmp = {0};
    /*flag that if the further process should be done*/
    unsigned char ucCallFlag = 0;
    if (cm == NULL || sncrxed == NULL)
    {
        return;
    }
    if (!(dlcx > 0 && dlcx < (MAX_DLC_TGT + 1)))
    {
        return;
    }
    CMUX_PRINT(CMUX_DEBUG_INFO, "start!\n");
    CMUX_PRINT(CMUX_DEBUG_INFO, "dlcx = %d !sncrxed->s1 = %d !\n", dlcx, sncrxed->s1);
    if (sncrxed->s1 == 1)
    {
        cm->snc_db [dlcx].task = SNC_TYPE_DATA;
        ucCallFlag = 1;
    }
    else if (sncrxed->s1 == 0)
    {
        if ( SNC_TYPE_DATA == cm->snc_db [dlcx].task )
        {
            for (i = 1; i <= MAX_DLC_TGT; i++)
            {
                if ( i != dlcx )
                {
                    if ( SNC_TYPE_AT == cm->snc_db [i].task )
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            CMUX_PRINT(CMUX_DEBUG_ERR, "sncrxed->s1 = %d !i = %d!\n", sncrxed->s1, i);
            if ( i > MAX_DLC_TGT )
            {
                ucCallFlag = 1;
                cm->snc_db [dlcx].task = SNC_TYPE_AT_DATA;
            }
        }
    }
    CMUX_PRINT(CMUX_DEBUG_INFO, "ucCallFlag = %d!\n", ucCallFlag);
    if ( ucCallFlag )
    {
        for (i = 1; i <= MAX_DLC_TGT; i++)
        {
            if (i != dlcx)
            {
                if (cm->snc_db [dlcx].task == SNC_TYPE_DATA)
                {
                    cm->snc_db [i].task = SNC_TYPE_AT;
                }
                else if (cm->snc_db [dlcx].task == SNC_TYPE_AT_DATA)
                {
                    cm->snc_db [i].task = SNC_TYPE_AT_DATA;
                }
            }
        }
        if ( cm->cb.callback_func )
        {
            snc_tmp.dlci = dlcx;
            snc_tmp.s1 = sncrxed->s1;
            cm->cb.callback_func(AT_STATE_CHANGE, (void*)&snc_tmp, NULL);
        }
    }
    return;
} /* Cmux_snc_set_dlc_process */


int cmux_UE_rxed_SNC_cmd (void *cmxlib, void *frm, unsigned char dlc)
{
    unsigned int *dlc0_state;
    unsigned char dlcx;
    int rc = FAIL;
    struct snc_cmd *sncrxed;
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }
    
    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);
    sncrxed    = (struct snc_cmd *) rxfr->uih_data;
    dlcx       = sncrxed->dlci;
    SET_DLC_STATE (*dlc0_state, (UE_RECVD_SNC_RESP_SENT));

    if (gcmux_active_ppp_pstask_dlc != 0)
    {
        cmux_UE_send_response (cm, dlcx, NSC, NULL);
        return PASS;
    }
    
    if (dlcx > MAX_DLC_TGT)
    {
        cmux_UE_send_response (cm, dlcx, CTRL_DM, NULL);
    }
    else
    {
        Cmux_snc_set_dlc_process (dlcx, sncrxed, cm);
        cmux_UE_send_response (cm, rxfr->dlc, SNC_RESP, (void *) sncrxed);
    }
    
    return PASS;
} /* cmux_UE_rxed_SNC_cmd */


int cmux_UE_rxed_DLCPN_cmd (void *cmxlib, void *frm, unsigned char dlc)
{
    unsigned int *dlc0_state;
    unsigned char dlcx;
    int rc = FAIL;
    struct dlc_param_neg dlcpn_default;
    struct dlc_param_neg *dlcpn_rxed;
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }

    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);
    dlcpn_rxed = (struct dlc_param_neg *) rxfr->uih_data;
    dlcx       = dlcpn_rxed->dlci;

    cmux_get_default_dlc_param_nego (&dlcpn_default, dlcx);

    cmux_validate_TE_UE_DLC_Params (&dlcpn_default, dlcpn_rxed, MODIFY);
    SET_DLC_STATE (*dlc0_state, (UE_RECVD_DLC_PN_RESP_SENT));

    // Carefully modifiy other flags/fields in rx-frame
    cmux_UE_send_response (cm, rxfr->dlc, DLC_PN_RESP, (void *) dlcpn_rxed);
    
    return PASS;
} /* cmux_UE_rxed_DLCPN_cmd */

int cmux_UE_NSC_handler (void *cmxlib, void *frm, unsigned char dlc)
{
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    unsigned int *dlc0_state;

    int rc = FAIL;
    unsigned char frame_code;
    struct frame *                 fr = (struct frame *) NULL;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }
    
    if (dlc > MAX_DLC)
    {
        return FAIL;
    }

    /* 如果是收到的NSC响应，停止流控计时器 */
    if (1 == g_nsc_received)
    {
        rc = cmux_UE_rxed_FCON_FCOFF_RESP(cm, rxfr, dlc);
        g_nsc_received = 0;
        return rc;
    }

    dlc0_state = &(cm->dlc_db [dlc]->dlc_state);
    if (Cmux_get_default_cmux_frame (cm, dlc, NSC, &fr, 0, 0) == FAIL)
    {
        return FAIL;
    }

    frame_code = cm->frame_map.frdetails [rxfr->f_type].frame_code;
    rc         = (int) cmux_struct_to_frame (&fr, cm);
    if (rc == FAIL)
    {
        return FAIL;
    }

    if (cm->dlc_db [fr->dlc] == NULL)
    {
        cmux_UE_send_response (cm, rxfr->dlc, CTRL_DM, NULL);
    }
    else
    {
        if (frame_code == CMUX_MUX_POWER_SAVING_CMD)
        {
            SET_DLC_STATE (*dlc0_state, (UE_RECVD_PSC_RESP_SENT));
        }
        if (frame_code == CMUX_MUX_TEST_CMD)
        {
            SET_DLC_STATE (*dlc0_state, (UE_RECVD_TEST_CMD_RESP_SENT));
        }
        if (frame_code == CMUX_MUX_MODEM_STATUS_CMD)
        {
            SET_DLC_STATE (*dlc0_state, (UE_RECVD_MSC_RESP_SENT));
        }
        if (frame_code == CMUX_MUX_SERVICE_NEGO_CMD)
        {
            SET_DLC_STATE (*dlc0_state, (UE_RECVD_SNC_RESP_SENT));
        }
        /* amrit:NSC support Begin*/
        if (frame_code == CMUX_RLS_CMD)
        {
            SET_DLC_STATE (*dlc0_state, (UE_RECVD_SNC_RESP_SENT));
        }
        if (frame_code == CMUX_RPN_CMD)
        {
            SET_DLC_STATE (*dlc0_state, (UE_RECVD_SNC_RESP_SENT));
        }
        /* amirt:NSC support END*/
        rc = cmux_send_to_com_fragmented (cm, cm->com_port, fr->raw_data, fr->raw_len);

        if (PASS == rc)
        {
            (*(cm->cb.callback_func))(TE_SENT | TX_COMPLETE, 
                                    (void *) fr, (struct cmux_timer *) NULL);
        }
        else
        {
            (*(cm->cb.callback_func))(TE_SENT | TX_FAILED, 
                                    (void *) fr, (struct cmux_timer *) NULL);
        }
    }
    cmux_free_frame (fr);
    return PASS;
} /* cmux_UE_NSC_handler */

int cmux_UE_rxed_CLD_cmd (void *cmxlib, void *frm, unsigned char dlc)
{
    int rc = FAIL;
    int i;
    unsigned int *dlc_state;

    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;

    // check cm done, ATCMUX on
    if (!((cm != NULL) 
        && ((cm->cmux_status & ATCMUX_ON) == ATCMUX_ON) 
        && (rxfr != NULL)))
    {
        return rc;
    }

    dlc_state = &(cm->dlc_db [dlc]->dlc_state);

    for (i = 1; i <= MAX_DLC_TGT; i++)
    {
        if (cm->dlc_db [i] != NULL)
        {
            SET_DLC_STATE (cm->dlc_db [i]->dlc_state, DLC_CLOSED);
            CLEAR_DLC_STATE (cm->dlc_db [i]->dlc_state, (~DLC_ESTABLISHED));
            mux_close_data_service(i);/*lint !e10*/
            cmux_delete_dlc_entry (cm, i);
            (*(cm->cb.callback_func))(DLC_STATE_CHANGE, 
                                    (void *) i, (struct cmux_timer *) NULL);
        }
    }

    if (cm->dlc_count == 1)
    {
        SET_DLC_STATE (*dlc_state, (DLC_CLOSED));
        CLEAR_DLC_STATE (*dlc_state, (~DLC_ESTABLISHED));
        cmux_UE_send_response (cm, rxfr->dlc, CLD_RESP, NULL);
        cm->cmux_status &= (~(ATCMUX_ON | CTRL_DLC));
        cm->cmux_status |= (ATCMUX_OFF);
        (*(cm->cb.callback_func))(DLC_STATE_CHANGE, 
                                (void *) dlc, (struct cmux_timer *) NULL);
    }
    
    return PASS;
} /* cmux_UE_rxed_CLD_cmd */

int cmux_UE_rxed_FCON_FCOFF_RESP (void *cmxlib, void *frm, unsigned char dlc)
{
    int rc = FAIL;
    struct cmuxlib *cm   = (struct cmuxlib *) cmxlib;
    struct frame *  rxfr = (struct frame *) frm;
    unsigned int *dlc0_state;
    int stop_proc = 0;
    CMUX_PRINT(CMUX_DEBUG_INFO, "enter...\n");
    if ((cm == NULL) 
        || ((cm->cmux_status & ATCMUX_ON) != ATCMUX_ON) 
        || (rxfr == NULL))
    {
        return rc;
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

    return PASS;
} /* cmux_TE_rxed_FCON_RESP */
#endif /* MBB_HSUART_CMUX */
