/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and 
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may 
* *    be used to endorse or promote products derived from this software 
* *    without specific prior written permission.
* 
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef __GEN_EVT_H__
#define __GEN_EVT_H__





/******************************************************************************

 MSP ���¼�����,�¼�ID�Ķ����MSG �Բ�ͬ�Ķ�������

******************************************************************************/
#define ID_EV_MSP_BASE_ID                      0x10000
#define ID_EV_PS_BASE_ID                       0x10500
#define ID_EV_DSP_BASE_ID                      0x11100

/******************************************************************************
 �¼�ID����.
 �¼�ID������һ�����ΪID_LOG_EV_XXX,ID_LOG_EV_Ϊ�����Զ����ɵ�.
 �¼�ID��СΪ32bit, �����Ҫ, ����ģ���������¼�ID�����ظ�.
******************************************************************************/



#define ID_LOG_EV_LOW_MEM                            0x10001 /* LOG_TYPE_WARNING
 */
#define ID_LOG_EV_LOW_POWER                          0x10002 /* LOG_TYPE_WARNING
 */
#define ID_LOG_EV_QUEUE_FULL                         0x10003 /* LOG_TYPE_WARNING
 */
#define ID_LOG_EV_OS_RESOURCE_UNAVAILABLE            0x10004 /* LOG_TYPE_WARNING
 */
#define ID_LOG_EV_MALLOC_FAIL                        0x10005 /* LOG_TYPE_WARNING
 */
#define ID_LOG_EV_POST_MESSAGE_FAIL                  0x10006 /* LOG_TYPE_WARNING
 */
#define ID_LOG_EV_API_CALL_FAIL                      0x10007 /* LOG_TYPE_WARNING
 */

#define ID_LOG_EV_START_CPU_FAIL                     0x10010 /* LOG_TYPE_ERROR
 */
#define ID_LOG_EV_LOAD_CODE_FAIL                     0x10011 /* LOG_TYPE_ERROR
 */
#define ID_LOG_EV_INIT_DEVICE_FAIL                   0x10012 /* LOG_TYPE_ERROR
 */
#define ID_LOG_EV_INIT_FAIL                          0x10013 /* LOG_TYPE_ERROR
 */
#define ID_LOG_EV_ACCESS_DEVICE_FAIL                 0x10014 /* LOG_TYPE_ERROR
 */
#define ID_LOG_EV_START_TASK_FAIL                    0x10015 /* LOG_TYPE_ERROR
 */

#define ID_LOG_EV_POWER_ON                           0x10020 /* LO G_TYPE_AUDIT_SUCCESS or LOG_TYPE_AUDIT_FAILURE
 */
#define ID_LOG_EV_POWER_OFF                          0x10021 /* LOG_TYPE_AUDIT_SUCCESS
 */
#define ID_LOG_EV_INIT                               0x10022 /* LOG_TYPE_ERROR
 */
#define ID_LOG_EV_INIT_DEVIC                         0x10023 /* LOG_TYPE_ERROR
 */
#define ID_LOG_EV_CREATE_TASK                        0x10024 /* LOG_TYPE_ERROR
 */
#define ID_LOG_EV_DELETE_TASK                        0x10025 /* LOG_TYPE_ERROR
 */


#define ID_LOG_EV_NV_INIT                            0x10030
#define ID_LOG_EV_NV_WRITE                           0x10031
#define ID_LOG_EV_NV_READ                            0x10032


#define ID_LOG_EV_UNKNOWN                            0x10050
#endif /* __GEN_MOD_ID_H__
 */

