

#ifndef ZLR96621L_SM2_AUSTRALIA_H
#define ZLR96621L_SM2_AUSTRALIA_H

#ifdef VP_API_TYPES_H
#include "vp_api_types.h"
#else
typedef unsigned char VpProfileDataType;
#endif


/************** Device Profile **************/

/************** DC Profile **************/

/************** AC Profile **************/
extern const VpProfileDataType AC_FXS_RF14_AU[];     /* AC FXS RF14 Australia 220R+820R//120nF Normal Coefficients */
extern const VpProfileDataType AC_FXS_RF14_WB_AU[];  /* AC FXS RF14 Australia 220R+820R//120nF Wideband Coefficients */

/************** Ringing Profile **************/

/************** Tone Profile **************/
extern const VpProfileDataType T_DL_AU[];            /* Australia Dial Tone  */
extern const VpProfileDataType T_RB_AU[];            /* Australia Ringback Tone  */
extern const VpProfileDataType T_BS_AU[];            /* Australia Busy Tone  */
extern const VpProfileDataType T_CW_AU[];            /* Australia Call Waiting Tone */
extern const VpProfileDataType T_CG_AU[];            /* Australia Congestion Tone  */

/************** Cadence Profile **************/
extern const VpProfileDataType CR_AU[];              /* Australia Ringing Signal Cadence  */
extern const VpProfileDataType CT_DL_AU[];           /* Australia Dial Tone Cadence */
extern const VpProfileDataType CT_RB_AU[];           /* Australia Ringback Tone Cadence */
extern const VpProfileDataType CT_BS_AU[];           /* Australia Busy Tone Cadence */
extern const VpProfileDataType CT_CW_AU[];           /* Australia Call Waiting Tone Cadence */
extern const VpProfileDataType CT_CG_AU[];           /* Australia Congestion Tone Cadence */

/************** Caller ID Profile **************/
extern const VpProfileDataType CID_TYPE1_AU[];       /* Australia Caller ID (Type 1 - On-Hook) - Telcordia FSK */
extern const VpProfileDataType CID_TYPE2_AU[];       /* Australia Caller ID (Type 2 - Off-Hook) - Telcordia FSK */

/************** Metering_Profile **************/

#endif /* ZLR96621L_SM2_AUSTRALIA_H */

