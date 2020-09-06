
#ifndef __MBB_FLIGHT_MODE_H__
#define __MBB_FLIGHT_MODE_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include <product_config.h>

#if (FEATURE_ON == MBB_FLIGHT_MODE)
#define RF_OFF 0
#define RF_ON  1
#define ENTER_FLIGHT_MODE 1
#define EXIT_FLIGHT_MODE  0

typedef enum
{
    RFSWITCH_SW_TRIG = 0,
    RFSWITCH_HW_TRIG,
    RFSWITCH_INIT_TRIG,
    RFSWITCH_NONE_TRIG
}rfswitch_trig_type;

/*飞行模式切换完成处理函数指针*/
typedef void (*rfswtich_op_func)(unsigned int op_mode);
extern rfswtich_op_func rf_switch_op_end_func[RFSWITCH_NONE_TRIG+1];

extern bool g_therm_rf_off;
void RfSwitch_SW_Set(unsigned int swstate);
unsigned int RfSwitch_SW_Get(void);
unsigned int RfSwitch_State_Get(void);
unsigned int Rfswitch_Change_Mode_To_State(unsigned int current_mode);
unsigned int RfSwitch_HW_Get(void);
void RfSwitch_State_Set(unsigned int rf_state);
void RfSwitch_Trig_Type_Set(rfswitch_trig_type trig_type);
rfswitch_trig_type RfSwitch_Trig_Type_Get(void);
#endif

#ifdef __cplusplus
}
#endif
#endif


