/******************************************************************************
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
******************************************************************************/



#ifndef __MBB_ANTEN_H__
#define __MBB_ANTEN_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* ���ߵĲ���״̬( -1��ȡʧ�ܣ�0����δ���룬1�����Ѳ��� ) */
#define ANTEN_DETECT_STATUS_NOT_INSERTED  (0)
#define ANTEN_DETECT_STATUS_INSERTED      (1)
#define ANTEN_DETECT_STATUS_ERR           (-1)

/*****************************************************************************
 �� �� ��  : anten_switch_get
 ��������  : �ṩ��ATģ���ȡ�����л�״̬�Ľӿ�
 �������  : ant_type : �������ͣ�0--�����ߣ�1--������
 �������  : ��
 �� �� ֵ  : ��ǰ���ߵ�״̬
 ˵    ��  : �˽ӿ�ֻ��atʹ�ã�Ӧ�û�ȡ״̬ͨ���ڵ����
*****************************************************************************/
int anten_switch_get(unsigned int ant_type);


/*****************************************************************************
 �� �� ��  : anten_switch_set
 ��������  : �ṩ��ATģ����������л��Ľӿ�
 �������  : ant_type : �������ͣ�0--�����ߣ�1--�����ߣ�2--��������
             in_or_out: ʹ�����û����ⲿ����
 �������  : ��
 �� �� ֵ  : 0--�ɹ�������--ʧ��
 ˵    ��  : �˽ӿ�ֻ��atʹ�ã�Ӧ���л�����ͨ���ڵ����
*****************************************************************************/
int anten_switch_set(unsigned int ant_type,unsigned int in_or_out);

/*****************************************************************************
 �� �� ��  : anten_number_get
 ��������  : �ṩ��ATģ���ȡ������Ŀ�Ľӿ�
 �������  : ��
 �������  : ��
 �� �� ֵ  : ������Ŀ
 ˵    ��  : �˽ӿ�ֻ��atʹ��
*****************************************************************************/
int anten_number_get(void);

/*****************************************************************************
 �� �� ��  : anten_detect_get
 ��������  : �ṩ��ATģ���ȡ���߲���״̬�Ľӿ�
 �������  : ant_type : �������ͣ�0--�����ߣ�1--������
 �������  : ��
 �� �� ֵ  : ��ǰ���ߵ�״̬( -1��ȡʧ�ܣ�0����δ���룬1�����Ѳ��� )
 ˵    ��  : �˽ӿ�ֻ��atʹ��
*****************************************************************************/
int anten_detect_get(int ant_type);

#ifdef __cplusplus
}
#endif

#endif /*__MBB_ANTEN_H__*/

