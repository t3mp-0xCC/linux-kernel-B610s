

#ifndef CMUX_FRAMER_H
#define CMUX_FRAMER_H

#include "mbb_config.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
// Macros for Actual frame content (during frame-formation)
#define CMUX_BASIC_MODE                  0
#define CMUX_DEFAULT_FRAME_SIZE          256
#define CMUX_PENDING_FRAME_SIZE          2048
#define CMUX_MAXIMUM_FRAME_SIZE          1540
#define CMUX_MINIMUM_FRAME_SIZE          31
#define CMUX_MAX_CHANNELS                63
#define CMUX_FRAME_FLAG                  0xF9
#define MAX_FRAME_LENGTH                 128
#define CMUX_FRAME_HDR(cm)      \
(cm->mp.frame_size_n1 > (CMUX_LENGTH_MASK) \
 ? CMUX_TWO_BYTE_HEADER_LENGTH : CMUX_ONE_BYTE_HEADER_LENGTH) 
#define MAX_CMUX_FRAME_SIZE(cm) \
(cm->mp.frame_size_n1) + CMUX_FRAME_HDR (cm) 
#define MAX_DLC                          63
#define MAX_PRIORITY                     63
#if (FEATURE_ON == MBB_WPG_ECALL)
#define MAX_DLC_TGT                      5 // the number of dlcs in the target.
#else
#define MAX_DLC_TGT                      4 // the number of dlcs in the target.
#endif/*MBB_WPG_ECALL*/
#define MAX_CMUX_FRAMES                  27
#define MAX_ACK_TIMER_T1                 250
#define MAX_RESP_TIMER_T2                250
#define MAX_RETRANSMIT_N2                10
#define MAX_WINDOW_K                     7
#define MIN_DEFAULT_ACK_TIMER_T1         100
#define MAX_DEFAULT_ACK_TIMER_T1         2500
#define MIN_DEFAULT_RETRANS_N2           0
#define MAX_DEFAULT_RETRANS_N2           10
#define DEFAULT_RETRANSMIT_N2            3
#define DEFAULT_WINDOW_K                 2
#define MIN_DEFAULT_RES_TIMER_T2         100
#define MAX_DEFAULT_RES_TIMER_T2         2500

#define DEFAULT_WAKEUP_TIMER_T3          1
#define MAX_WAKEUP_TIMER_T3              255 // sec

#define TEST_LEN_LIMIT                   129 
// Command Frame Macros
#define CMUX_CTRL_SABM_FRAME             0x3F
#define CMUX_CTRL_SABM_FRAME_LENGTH      0
#define CMUX_CTRL_UA                     0x73
#define CMUX_CTRL_UA_FRAME_LENGTH        0
#define CMUX_CTRL_DM                     0x1F
#define CMUX_CTRL_DM_FRAME_LENGTH        0
#define CMUX_CTRL_DISC                   0x53
#define CMUX_CTRL_DISC_FRAME_LENGTH      0
/*juvin: M2M-1669 to make the control frames as EF 
on sending and as well as responding.. 
this was made as per the chat done on 2/4/2014 with xiajiaxin */
#define CMUX_CTRL_UIH                    0xEF 
#define CMUX_DATA_UIH                    0xEF // In pure UIH-data, P/F bit is NOT set
#define CMUX_DATA_UI                     0x03 // In pure UI-data, P/F bit is NOT set

#define CMUX_PPP_DATA                    1  

#define CMUX_VOICE_DLC                   2  
// MUX Frames Macros
#define CMUX_MUX_DLC_NEGO_PARAM_CMD      0x83
#define CMUX_MUX_DLC_NEGO_PARAM_RESP     0x81

#define CMUX_MUX_SERVICE_NEGO_CMD        0xD3
#define CMUX_MUX_SERVICE_NEGO_RESP       0xD1
#define CMUX_MUX_POWER_SAVING_CMD        0x43
#define CMUX_MUX_CLOSE_CMD               0xC3
#define CMUX_MUX_MODEM_STATUS_CMD        0xE3
#define CMUX_MUX_MODEM_STATUS_RESP       0xE1

#define CMUX_MUX_TEST_CMD                0x23
/*juvin<m2m-1594,1593,1580><proper NSC response format><24-march>*/
#define CMUX_MUX_NON_SUPORTED_RES        0X11 
#define  CMUX_MUX_POWER_SAVING_RESP      0x41
#define CMUX_MUX_CLOSE_RESP              0xC1
#define CMUX_MUX_TEST_CMD_RESP           0x21

#define CMUX_FCON_CMD                    0xA3
#define CMUX_FCON_RESP                   0xA1
#define CMUX_FCOFF_CMD                   0x63
#define CMUX_FCOFF_RESP                  0x61
#define CMUX_RLS_CMD                     0x53
#define CMUX_RLS_RESP                    0x51
#define CMUX_RPN_CMD                     0x93
#define CMUX_RPN_RESP                    0x91

#define CONVERGENCE_LAYER                0
#define DEFAULT_PRIORITY                 1
// Length macros for each mux control frames
#define CMUX_MUX_DLC_NEGO_PARAM_LENGTH   8
#define CMUX_MUX_SERVICE_NEGO_LENGTH     3
#define CMUX_MUX_POWER_SAVING_LENGTH     0
#define CMUX_MUX_CLOSE_CMD_LENGTH        0
#define CMUX_MUX_MODEM_STATUS_CMD_LENGTH 2
#define CMUX_MUX_TEST_CMD_LENGTH         0
#define CMUX_MUX_NON_SUPORTED_RES_LENGTH 1

#define CMUX_FCON_LENGTH                 0
#define CMUX_FCOFF_LENGTH                0

#define CMUX_ONE_BYTE_HEADER_LENGTH      6
#define CMUX_TWO_BYTE_HEADER_LENGTH      7

#define CMUX_PF_BIT_MASK                 0xEF
#define CMUX_LENGTH_MASK                 0x7F
#define CMUX_EA_BIT_MASK                 0x01
#define CMUX_DLC_BIT_MASK                0x3F
#define CMUX_PRIORITY_BIT_MASK           0x3F
#define CMUX_WINDOW_BIT_MASK             0x07
#define PASS                             1
#define FAIL                             0

int cmux_UE_rxed_UA (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_CLD_RESP (void *, void *, unsigned char);
int cmux_TE_UE_rxed_DM (void *cmxlib, void *frm, unsigned char dlc);
int cmux_UE_rxed_UIHDATA (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_DLCPN_cmd (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_Test_cmd (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_PSC_cmd (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_MSC_cmd (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_SNC_cmd (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_CLD_cmd (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_NSC_handler (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_FCON_cmd (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_FCOFF_cmd (void *cm, void *rxfr, unsigned char dlc);
int cmux_UE_rxed_FCON_FCOFF_RESP (void *cmxlib, void *frm, unsigned char dlc);

struct framedetails {
    unsigned char frame_code;
    unsigned char frame_length; // will be hardcoded
    unsigned int accept_flag;
    unsigned int reject_flag;
    int (*frame_rx_handler)(void *cm, void *rxfr, unsigned char dlc);
    unsigned int dlc_next_state;
    /*For the given frame type, that is transmitted, what will be the DLC-state, 
    that will be auto-set after successful transmission of frame is set above
    kick_start cmd, TE/UE response() is the central place where 
    the frame tranmssion actually happens, so that update there..*/
};
// MUX frame structures
// Service Negotiation Parameter
struct service_param_neg {
    unsigned char dlci;
    unsigned char service; // voice or data
    unsigned char voice_codec;
    /*Type field (EA, CR, code), length, 
    will be hardcoded for this frame type, refer macro*/
};
// DLC-Mux-params can be used for a specific DLC or for entire channel
// hence making a structure for re-usability
struct mux_params {
    unsigned int mode;
    unsigned int baud_rate;
    unsigned int frame_size_n1;    // Max frame size (N1), (1-128), default 31
    unsigned int ack_timer_t1;     // acknowledgement Timer (T1),default 0,1s
    unsigned int retransmit_n2;    // Max no of retransmission (NA), (0-255), default 3
    unsigned int window_k;         // window size for error recovery mode (K)
    unsigned int res_timer_t2;     // Response Timer (T2),default 0,3s
    unsigned int wakeup_timer_t3;  // Wakeup Response Timer (T3),1-255s default 10s
};

struct test_cmd
{
    unsigned char *dataptr;
    unsigned short test_data_len;
};

// DLC parameter Negotiation structure
struct dlc_param_neg {
    /*Type field (EA, CR, code), length, 
    will be hardcoded for this frame type, refer macro*/
    unsigned char dlci;             // For which DLCI
    unsigned char priority;         // Priority for particular DLC
    unsigned char conv_layer;       // To be hardcoded refer macros
    struct mux_params muxparams;
};

// Modem status command structure
struct modem_status_cmd {
    // cmd, length to be hardcoded value
    unsigned char dlci;             // For which DLCI
    unsigned char msc_command;      // Modem status command or response
    unsigned char fc : 1;           // Flow control
    unsigned char rtc : 1;          // Ready to communicate
    unsigned char rtr : 1;          // Ready to Receive
    unsigned char resrvd1:1;        // reserved bit1
    unsigned char resrvd2:1;        // reserved bit2
    unsigned char ic : 1;           // Incoming call indicator
    unsigned char dv : 1;           // valid data sent
    unsigned char EA : 1;           // hard coded as 1
};

struct non_supported_cmd_resp {
    unsigned char nsc_cmd_type;
};

// Service Negotiation command structure
struct snc_cmd {
    unsigned char dlci;             // For which DLCI
    unsigned char s1 : 1;           // Service Value 1
    unsigned char s2 : 1;           // Service Value 2
    unsigned char s3 : 1;           // Service Value 3
    unsigned char s4 : 1;           // Service Value 4
    unsigned char s5 : 1;           // Service Value 5
    unsigned char s6 : 1;           // Service Value 6
    unsigned char s7 : 1;           // Service Value 7
    unsigned char EA : 1;           // hard coded as 1
};
enum  fr_type {
    /* support for basic mode only */
    /*DLC Control Commands*/
    CTRL_SABM = 0,  // Set Asynchronous Balanced Mode
    CTRL_UA   = 1,  // Unnumbered Acknowledgement Response
    CTRL_DM   = 2,  // Disconnected Mode
    CTRL_DISC = 3,  // Disconnect mode
    CTRL_UIH  = 4,  // Unnumbered information with header check and response

    /* UIH MUX frames*/
    DLC_PN_CMD  = 5,  // DLC Parameter Negotiation command
    DLC_PN_RESP = 6,  // DLC Parameter Negotiation response
    PSC_CMD     = 7,  // Power saving Control command
    PSC_RESP    = 8,  // Power saving Control response
    CLD_CMD     = 9,  // Multiplexer Close Down command
    CLD_RESP    = 10, // Multiplexer Close Down response
    TEST_CMD    = 11, // Test Command command
    TEST_RESP   = 12, // Test Command response
    MSC_CMD     = 13, // Modem Status Command command
    MSC_RESP    = 14, // Modem Status response
    NSC         = 15, // Non Supported Command Response
    SNC_CMD     = 16, // Service Negotiation Command
    SNC_RESP    = 17, // Service Negotiation response

    // Data frames
    UIH_DATA   = 18,
    FCON_CMD   = 19,
    FCON_RESP  = 20,
    FCOFF_CMD  = 21,  // Flow control off command
    FCOFF_RESP = 22,  // Flow control off response
    RLS_CMD    = 23,
    RLS_RESP   = 24,
    RPN_CMD    = 25,
    RPN_RESP   = 26
};

// Frame error codes
enum fr_err_code        // (write drop scenarios)
{
    FRAME_OK  = 0,  // store nfptr
    NO_HEADER = 1,
    NO_TAIL   = 2,  // too short frame, store nfptr and NOT rawptr

    // For all below error codes, use nfptr and call back
    BASIC_FRAME_ERR  = 4,
    INVALID_FRAME_ERR = 5, /*juvin: to send the nsc only to the invlid frames: 28march */
    FCS_ERR          = 8,   // HARI
    BASIC_FRLEN_ERR  = 16,  // error in length
    BASIC_FR_DLC_ERR = 32,  // dlc error
    BASIC_FR_EA_ERR  = 64,  // EA bit error
    BASIC_FR_CR_ERR  = 128, // CR bit error
    BASIC_FRTYPE_ERR = 256, // frame type error

    MUX_FRTYPE_ERR = 512,   // Mux frame errors
    DATA_ERR       = 1024,
   /*in this case the first frame is taken return 
   unconsumed-bytes (filled in length), 
   so as to callback with more data later*/
    EXTRA_INCOMPLETE_FRAME           = 4,
    EXTRA_ERROR_FRAME                = 8,   // new frame does not have header
     /*Header 0xf9, moved-IN but still tail not available, return value indicates*/
    HEADER_MOVED_IN_INCOMPLETE_FRAME = 16,
    HEADER_MOVED_IN_COMPLETE_FRAME   = 32
};

// Basic frame structure (includes control, mux and data)
struct frame {
    /*used for control_field (or) UIH-Data (or) UIH-MUX*/
    enum    fr_type f_type;
    unsigned short int length;
    unsigned char dlc;                  /*goes into addr-field*/
    unsigned char fcs;
    unsigned char addr_EA_bit : 1;/*goes into addr-field*/
    unsigned char add_CR_bit : 1; /*goes into addr-field*/
    unsigned char UI_bit : 1;         /* UI or UIH data*/
    unsigned char pf_bit;              /*poll or final bit*/
    /*points to DLC-0, UIH Mux or UIH data (for DLC-1-63 UIH pure data), 
    will be void* to a variable length structure or data, 
    typecasted based on fr_type*/
    void *uih_data;
    /*contains the framed data to be sent over actual serial port, 
    or received over serial port, useful for raw-dump-logs in QT*/
    unsigned char *raw_data;  
    /*contains the length of the frame data to be sent over actual serial port, 
    or received over serial port, useful for raw-dump-logs in QT*/
    unsigned short int raw_len;    
    /*contains PROCEED_TO_NEXT, or not*/
    unsigned char flag;             
};

/*This structure is used for TE/UE based registration, which happens one time*/
typedef struct framer {
    const struct framedetails *frdetails;
    int (*cmux_cmd_handler)(void *, void *);
    unsigned int pending_mask;
} FRAMER;

unsigned int cmux_struct_to_frame (struct frame **fr, void *cm);

#endif /* MBB_HSUART_CMUX */
#endif /* ifndef CMUX_FRAMER_H */
