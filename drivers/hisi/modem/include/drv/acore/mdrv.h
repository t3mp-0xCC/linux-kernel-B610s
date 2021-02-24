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

#ifndef __MDRV_ACORE_H__
#define __MDRV_ACORE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*公共头文件*/
#include "mdrv_public.h"
#include "mdrv_version.h"

/*soc*/
#include "mdrv_int.h"
#include "mdrv_wdt.h"
#include "mdrv_timer.h"
#include "mdrv_edma.h"
#include "mdrv_ipc.h"
#include "mdrv_icc.h"
#include "mdrv_memory.h"
#include "mdrv_rtc.h"
#include "mdrv_sysctrl.h"
#include "mdrv_sync.h"
#include "mdrv_pm.h"

/*devcie ip*/
#include "mdrv_hkadc.h"
#include "mdrv_efuse.h"
#include "mdrv_misc.h"
#include "mdrv_ipf.h"

/*封装层*/
#include "mdrv_udi.h"
#include "mdrv_om.h"
#include "mdrv_rfile.h"
#include "mdrv_nvim.h"

/*solution*/
#include "mdrv_sysboot.h"
#include "mdrv_dload.h"
#include "mdrv_chg.h"
#include "mdrv_wifi.h"
#include "mdrv_sd.h"
#include "mdrv_mbb_channel.h"
#include "mdrv_socp.h"
#include "mdrv_usb.h"

/*stub*/
#include "mdrv_stub.h"

#include "drv_mailbox_cfg.h"    /* 多核ICC项目后此头文件删除 */
#include "mdrv_hsuart.h"
#include "mdrv_crypto.h"
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
#include "mdrv_simlock.h"
#endif/*end for MBB_SIMLOCK_FOUR*/
#ifdef __cplusplus
}
#endif
#endif

