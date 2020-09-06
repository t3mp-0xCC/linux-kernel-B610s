/*
 * sttype.h
 *
 *  Created on: 5 d茅c. 2014
 *      Author: naqunoeil
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MMY-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
/**********************问题单修改记录******************************************
日    期              修改人         问题单号           修改内容
******************************************************************************/
#ifndef STTYPE_H_
#define STTYPE_H_

#include <mbb_config.h>

#if (FEATURE_OFF == MBB_NFC)
/* exact-width signed integer types */
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;


/* exact-width unsigned integer types */
typedef unsigned           char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;

typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u8;
#endif
typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

#endif /* STTYPE_H_ */
