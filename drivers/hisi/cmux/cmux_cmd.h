

#ifndef CMUX_CMD_H
#define CMUX_CMD_H

#include "cmux_framer.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
/*<juvin><M2M-1534><to inclrease the buffer size of serialisation 
of At commands on all dlcs>*/
#define DSTASK_MAX_BUFF_CMD_SIZE 128 
enum NEGOTIATION_CMD {
    MATCH    = 1,
    NO_MATCH = 2,
    MODIFY   = 3,
    COMPARE  = 4
};
typedef enum {
    SNC_TYPE_AT = 0,
    SNC_TYPE_DATA,
    SNC_TYPE_GPS,
    SNC_TYPE_AT_DATA,
    SNC_TYPE_INVALID
}SNC_Type;

// Expects cmux_framer.h to be included first

struct cmux_timer {
    unsigned int ack_timer_t1;     // acknowledgement Timer (T1)
    unsigned int retransmit_n2;    // Max no of retransmission (NA), (0-255), default 3
    unsigned int window_k;         // window size for error recovery mode (K)
    unsigned int res_timer_t2;     // Response Timer (T2)
    unsigned int wakeup_timer_t3;  // Wakeup Response Timer (T3)
    /*coverity´íÎóÐÞ¸Ä£¬É¾³ýres_timer*/
};

struct cmux_cmd {
    struct frame *fr;
    struct cmux_timer ct;
    struct cmux_cmd *next; // next cmd_node
};

struct DLC_statistics {
    unsigned int pending_tx_data;
    unsigned int pending_tx_total;
    unsigned int pending_tx_ctrl;
    unsigned int dlc_buffer_size;
};
struct snc_entry {
    unsigned char dlc;
    SNC_Type task;
}; // new one added to include snc mapping for dlc in target.
struct DLC_Entry {
    unsigned char dlc;
    unsigned int dlc_state;
    struct cmux_cmd *cmd_list;
    struct DLC_statistics stats;
    struct dlc_param_neg dlcparam;
    struct modem_status_cmd msc;
    struct snc_cmd snc;
    void *dlc_user_tcb_ptr;
    void *dlc_user_wm_ptr;
    // AT-serialiser DLC Buffer
    unsigned char at_buff [DSTASK_MAX_BUFF_CMD_SIZE];
    unsigned short at_buff_used_len;
};

struct cmux_tgt_dlc_entry {
    unsigned char dlc;
    void *dlc_user_tcb_ptr;
    void *dlc_user_wm_ptr;
    unsigned char dlc_status;
    unsigned char free_after_enqueue_to_task;
    unsigned long signal_task;
};

/* Note: Struct DLC-entry represents only one-DLC entry,
   *refer to CMUXLib structure which contains array of pointers of type Struct DLC-entry,
 *or in other words each Non-NULL index points to an allocated dlc-entry */

// LIST OF DLC-STATES
#define DLC_ENTRY_DONE                0x1
#define DLC_COM_READY                 0x02
#define DLC_COMMAND_FAILED            0x80000000
#define DLC_ESTABLISHED               0x40000000
#define DLC_CLOSED                    0x20000000
#define DLC_NON_CRIT_COMMAND_FAILED   0x10000000

// UE only
#define UE_RECVD_DLC_PN_RESP_SENT   0x4
#define UE_RECVD_SABM_CMD_UA_SENT   0x8
#define UE_RECVD_SABM_CMD_DM_SENT   0x10
#define UE_RECVD_MSC_RESP_SENT      0x20
#define UE_RECVD_SNC_RESP_SENT      0x40
#define UE_RECVD_TEST_CMD_RESP_SENT 0x80
#define UE_RECVD_PSC_RESP_SENT      0x100
#define UE_SENT_NSC_RESP            0x200
#define UE_RECVD_UIH_DATA           0x400
#define UE_RECVD_DISC_CMD_UA_SENT   0x800
#define UE_RECVD_DISC_CMD_DM_SENT   0x1000
#define UE_SENT_DISC_CMD_UA_PENDING 0x2000
#define UE_SENT_DISC_CMD_UA_RECVD   (~0x2000)
#define UE_RECVD_CLD_RESP_SENT      0x4000

// #define UE_RECVD_FCON_RESP_SENT     0x8000
// #define UE_RECVD_FCOFF_RESP_SENT	0x10000

#define UE_DLC_ENABLED               0x8000
#define UE_DLC_DISABLED              0x10000

#define UE_RECVD_FCON_RESP_SENT      0x20000
#define UE_RECVD_FCOFF_RESP_SENT     0x40000
#define UE_SENT_MUX_CLD_RESP_PENDING 0x80000
#define UE_SENT_MUX_CLD_RESP_RECVD   (~0x80000)
/*HUAWEI DRIVER DEV GROUP added for FCON/FCOFF command sending*/
#define UE_SENT_FCON_PENDING            0x100000
#define UE_SENT_FCOFF_PENDING           0x200000
#define UE_SENT_FCON_RESP_RECVD      (~0x100000)
#define UE_SENT_FCOFF_RESP_RECVD     (~0x200000)

// (except DM_RESPONSE)
#define UE_PENDING_MASK UE_SENT_DISC_CMD_UA_PENDING

#define CHECK_DLC_STATE(DLC_STATE, EXPECTED_STATE) \
((DLC_STATE & EXPECTED_STATE) == EXPECTED_STATE)
#define SET_DLC_STATE(DLC_STATE, NEW_STATE)        ((DLC_STATE) |= (NEW_STATE))
#define CLEAR_DLC_STATE(DLC_STATE, STATE)          ((DLC_STATE) &= (STATE))

#endif /* MBB_HSUART_CMUX */

#endif /* ifndef CMUX_CMD_H */
