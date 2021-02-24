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
#ifndef _BREATH_LED_H
#define _BREATH_LED_H

#ifdef __cplusplus
extern "C" {
#endif

#define PUBLIC 
#define PRIVATE static
#define BR_OK    1
#define BR_ERROR 0

/* ������״̬���� */
typedef enum
{
    BREATH_LED_STATE_LIGHT   = 0, /* �����Ƴ��� */
    BREATH_LED_STATE_SLEEP   = 1, /* �����Ƴ��� */
    BREATH_LED_STATE_SPARK   = 2, /* �����ƺ��� */
    BREATH_LED_STATE_ENABLE  = 3, /* ������ʹ�� */
    BREATH_LED_STATE_DISABLE = 4, /* �����ƹر� */
    BREATH_LED_STATE_MAX
}breathled_state;


PUBLIC int breathled_controller(breathled_state led_state);

#ifdef __cplusplus
}
#endif

#endif /* _BREATH_LED_H */