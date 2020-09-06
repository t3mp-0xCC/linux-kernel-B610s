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
#ifndef __MBB_TOUCH_H__
#define __MBB_TOUCH_H__

#include <product_config.h>

#if(FEATURE_ON == MBB_TOUCH_BOOST)
#define TOUCH_BOOST_DURATION  3000000
#endif

#if (FEATURE_ON == MBB_TOUCHSCREEN_MSTAR)
extern int touch_mstar_init(void);
extern void touch_mstar_exit(void);
#endif
#if (FEATURE_ON == MBB_TOUCHSCREEN_FOCAL)
extern  int    touch_fts_init(void);
extern  void   touch_fts_exit(void);
#endif
/* 支持melfas touch ic 时打开此宏 */
#if (FEATURE_ON == MBB_TOUCHSCREEN_MELFAS)
extern int  touch_melfas_init(void);
extern void touch_melfas_exit(void);
#endif
#endif