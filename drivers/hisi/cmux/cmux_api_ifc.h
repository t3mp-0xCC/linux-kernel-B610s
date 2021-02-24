


/* Key fields that affect the way library works and these values are set
 * during cmuxlib_init or mux_start and mux_stop cmuxlib_del and never changed anywhere
 */

#ifndef CMUX_API_IFC_H
#define CMUX_API_IFC_H

#include "cmux_framer.h"
#include "cmux_cmd.h"

#if ( FEATURE_ON == MBB_HSUART_CMUX )
#define COMPORT_WIDTH 10

enum CMUX_MODE {
    CMUX_TEMODE = 1,
    CMUX_UEMODE = 2,
    ATCMUX_ON   = 4,
    CTRL_DLC    = 8,
    ATCMUX_OFF  = 16
};

/* The structure ds_mux_cmux_info_type holds the parameters passed +CMUX
    and is as per 3GPP 27.010 spec */    
typedef struct 
{
    unsigned int   operating_mode;               
    /**< Mode 0 - basic, 1- advanced.        */
    unsigned int   subset;                         
    /**< 0-UIH, 1-UI, 2-I frames.            */
    unsigned int   max_cmd_num_tx_times_N2;      
    /**< Max re-tries N2 (0-255).            */
    unsigned int   window_size_k;                
    /**< Window size default 2, range (1-7). */
    unsigned int   port_speed;                    
    /**< port speed valid values: 1-6.       */
    unsigned int   max_frame_size_N1;            
    /**< Max frame size (1-32768).           */
    unsigned int   response_timer_T1;            
    /**<  Time UE waits for an acknowledgement before
    resorting to other action (e.g. transmitting a frame)
    default 100ms min value 10 ms.
    */
    unsigned int   response_timer_T2;            
    /**< Time mux control channel waits before 
    re-transmitting a command default 300ms and
    min value 20 ms.
    */
    unsigned int   response_timer_T3;            
    /**<  Time UE waits for response for a 
    power wake-up command default 10ms and min
    value 1 ms.
    */
} cmux_info_type;

#define MAX_SERIAL_PKT_LEN 672
#define MAX_TX_DATA_PKT_LEN(cm)   cm->mp.frame_size_n1 
#define MAX_TEST_DATA      1540
#define MAX_TEST_DATA_PKT_LEN(cm) MAX_TEST_DATA - CMUX_FRAME_HDR (cm) - 2 

typedef struct frame_result {
    // Flags below indicate the pending frame details
    unsigned char start_flag : 1;
    unsigned char length_flag : 1;
    unsigned char fcs : 1;
    unsigned char end_flag : 1;
    unsigned char err : 1;
    unsigned short start_offset;    // 0..n
    unsigned short end_offset;      // 1 .. n
} FRAME_RESULT;

typedef struct cmux_input_frame {
    unsigned char *pending_frame;/* CMUX appends data into this*/
    unsigned short pending_frame_idx;/* points to index of where to put data*/
    struct frame_result result;
} CMUX_INPUT_FRAME;

enum callback_type {
    TE_SENT                       = 1,
    UE_SENT                       = 2,
    DLC_STATE_CHANGE              = 4,
    AT_STATE_CHANGE               = 8,
    /*called by Input_frame_to_CMUX*/
    ERROR_FRAME                   = 16,
    /*called by command-node->rx_handler, on all-fails*/
    TIMEOUT_ON_ALL_RETRANSMISSION = 32,
    /*called by command-node->rx_handler, 
    more like informing upper layer for retransmission*/
    TIMEOUT_RETRANSMISSION        = 64,    
    /*called by kick_start() on one-frame transmission*/
    TX_COMPLETE                   = 128,
    RX_COMPLETE                   = 256,
    /*called by kick_start() on error, and it is a critical issue*/
    TX_FAILED                     = 512,
    GENERAL_NOTIFICATION          = 1024,
    DLC_ESTABLISH_COMPLETE        = 2048
};

enum CMUX_TX_BUFFER_CTRL {
    CMUX_FRAGMENT_DATA = 1,
    CMUX_FLUSH_DATA    = 2
};

/* Below structure is a parameter to cmux_register_callback() 
which is invoked by QT or C-unit for call backs.
Note QT fills this structure and calls cmux_register_callback, 
while QT is called back with this same structure */
struct cmux_callback {
    enum callback_type callback;    // consumed by QT/C-unit, filled by CMUXLib
    struct frame *fr;               // consumed by QT/C-unit, filled by CMUXLib
    void (*callback_func)(unsigned int cbtype, void *ptr, struct cmux_timer *ct);
#ifdef WIN32
    int (*send_to_com)( char* com_name, unsigned char* rawptr, unsigned short len);
#else
    unsigned int (*send_dl_data)( unsigned char* rawptr, unsigned short len);
#endif
    unsigned int (*set_power_cb)(void);
    /*timer structure has meaning only in RX-path, 
    where the callback_type can have TIMEOUT_RETRANSMISSION value, 
    so the QT application can open this structure to figure out retransmission count*/
    struct cmux_timer *ct; 
};

/* cmuxlib is the master structure which the QT library will 
use when calling any entry functions into CMUX library,
as it is only through this structure does the CMUX library 
remember its state of DLCs, commands, frames sent/received etc,.
This structure is initialized using cmux_init, destroyed using cmux_del routines
*/
typedef struct  cmuxlib
{
    /*Mux-on/off , Control-DLC established-status*/
    enum CMUX_MODE cmux_status; 
    char com_port [COMPORT_WIDTH];
    /*Mux-Parameters set through AT+CMUX*/
    /*mux_params refere cmux_framer*/
    struct mux_params mp;   
    /*For QT ?reference & usage only*/
    struct qt_com_status *comdb;
    /*array of pointers to DLC-Entry, only for allocated DLCs*/
    struct DLC_Entry *dlc_db [MAX_DLC + 1]; 
    /*should be changes as  per the number of dlcs required in target*/
    struct snc_entry snc_db [MAX_DLC_TGT + 1]; 
    /*Frame details*/
    struct cmux_input_frame cmux_if;
    struct cmux_callback cb;
    unsigned short dlc_count;
    unsigned int ue_dlc_ctrl_0;
    unsigned int ue_dlc_ctrl_1;
    /*Framer related*/
    struct framer frame_map;
    void *qtcom;
    void *qttimer;
    struct file *fp; // 存放设备指针，linux用
    /*FCON/FCOFF的标志位，1表示FCON，即可以接受上位机数据*/
    char flow_ctrl;
} CMUXLIB;

struct cmuxlib *Cmux_cmuxlib_init (enum CMUX_MODE mode, cmux_info_type *cmux_info);
int Cmux_register_callback (struct cmuxlib *cm, struct cmux_callback *callbk);
int Cmux_Input_frame_to_CMUX (struct cmuxlib *cm, unsigned char *buffer, 
                                                    unsigned short buffer_len);
int Cmux_send_dlc_data (void *cmxlib, unsigned char *buff, unsigned short len, 
    unsigned char dlc, unsigned int control, unsigned short flush_len, 
    unsigned short no_of_frames);
int Cmux_send_msc (struct cmuxlib *cm, unsigned char dlc, struct modem_status_cmd *msc);
int Cmux_send_fcon_fcoff (struct cmuxlib *cm, unsigned char dlc, enum fr_type ftype);
#endif /* MBB_HSUART_CMUX */

#endif /* CMUX_API_IFC_H */
