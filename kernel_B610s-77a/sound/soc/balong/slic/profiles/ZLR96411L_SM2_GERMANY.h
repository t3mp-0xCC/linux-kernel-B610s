

#ifndef ZLR96621L_SM2_GERMANY_H
#define ZLR96621L_SM2_GERMANY_H

#ifdef VP_API_TYPES_H
#include "vp_api_types.h"
#else
typedef unsigned char VpProfileDataType;
#endif


/************** Device Profile **************/

/************** DC Profile **************/
extern const VpProfileDataType DC_FXS_VE960_ABS100V_ETSI[];

/************** AC Profile **************/
extern const VpProfileDataType AC_FXS_RF14_DE[];     /* AC FXS RF14 Germany 220R+820R//115nF Normal Coefficients */
extern const VpProfileDataType AC_FXS_RF14_WB_DE[];

/************** Ringing Profile **************/

/************** Tone Profile **************/
extern const VpProfileDataType T_DL_DE[];
extern const VpProfileDataType T_RB_DE[];
extern const VpProfileDataType T_BS_DE[];
extern const VpProfileDataType T_CW_DE[];
extern const VpProfileDataType T_CG_DE[];
extern const VpProfileDataType T_ALARM_DE[];
extern const VpProfileDataType T_SVCEN_DE[];
extern const VpProfileDataType T_SVCDIS_DE[];

/************** Cadence Profile **************/
extern const VpProfileDataType CR_DE[];
extern const VpProfileDataType CT_DL_DE[];
extern const VpProfileDataType CT_RB_DE[];
extern const VpProfileDataType CT_BS_DE[];
extern const VpProfileDataType CT_CW_DE[];
extern const VpProfileDataType CT_CG_DE[];
extern const VpProfileDataType CT_ALARM_DE[];
extern const VpProfileDataType CT_SVCEN_DE[];
extern const VpProfileDataType CT_SVCDIS_DE[];

/************** Caller ID Profile **************/
extern const VpProfileDataType CID_TYPE1_DE[];
extern const VpProfileDataType CID_TYPE2_DE[];

/************** Metering_Profile **************/

#endif /* ZLR96621L_SM2_GERMANY_H */

