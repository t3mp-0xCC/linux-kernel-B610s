/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#ifndef REB_CONFIG_H
#define REB_CONFIG_H

#define YES  1
#define NO   0

#define Reb_Platform_9x25 (NO)
#define Reb_Platform_V7R2 (YES)
#define MBB_FEATURE_REB_DFT (YES)
/*ϵͳ�������̲����ļ�ϵͳ��������*/
#define MBB_REB_WRITE_PROTECT   (YES)
/*�ٹػ�ʧ�ܽ�����ػ�����*/
#define MBB_REB_FAST_POWEROFF   (YES)
/*Flash��Σ��ɨ�蹦��*/
#define MBB_REB_BADZONE_SCAN  (NO)
/*Flash��Σ���д����*/
#define MBB_REB_BADZONE_REWRITE (NO)
/*����дԽ�籣��ʹ��*/
#define MBB_REB_ZONE_WRITE_PROTECT (YES)

#define REB_OS_DEBUG
#if ( YES == Reb_Platform_9x25)
#include <linux/huawei_feature.h>
#ifdef  CONFIG_MBB_FAST_ON_OFF
#define MBB_FAST_ON_OFF FEATURE_ON
#endif
#endif
#if ( YES == Reb_Platform_V7R2)
#include <product_config.h>
#endif
#endif
