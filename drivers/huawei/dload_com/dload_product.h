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
#ifndef _DLOAD_PRODUCT_H
#define _DLOAD_PRODUCT_H
/******************************************************************************
  1 ����ͷ�ļ�����:
******************************************************************************/
#include "dload_comm.h"

#ifdef  __cplusplus
    extern "C"{
#endif

/******************************************************************************
  2 ģ����� (�ꡢö�١��ṹ�塢�Զ�����������) ������:
******************************************************************************/


/*******************************************************************
  ��������  : uint32  get_product_mark(void)
  ��������  : ��ȡ��Ʒ������Ϣ
  �������  : NA
  �������  : NA
  ��������ֵ: ��Ʒ��Ϣ�ṹ
********************************************************************/
product_info_t*  get_product_info(void);

/*******************************************************************
  ��������  : get_show_path_info
  ��������  : ��ȡpath ��path_indexת��������
  �������  : NA
  �������  : pppath_info_st : pathinfo_st ����ĵ�ַ
                            info_size: pathinfo_st��size
  ��������ֵ: DLOAD_OK    : �ɹ�
              DLOAD_ERR   : ʧ��
********************************************************************/
uint32  get_show_path_info(path_info_t **pppath_info_st, uint32 *info_size);


/*******************************************************************
  ��������  : get_show_info
  ��������  : ��ȡlcd/led������Ϣ
  �������  : NA
  �������  : ppupstate_info : upstateinfo_st ����ĵ�ַ
                            info_size: upstateinfo_st��size
  ��������ֵ:DLOAD_OK    : �ɹ�
              DLOAD_ERR   : ʧ��
********************************************************************/
uint32  get_show_info(upstate_info_t **ppupstate_info, uint32 *info_size);

/*******************************************************************
  ��������  : get_dload_feature
  ��������  : ��ȡ��������������Ϣ
  �������  : NA
  �������  : NA
  ��������ֵ: ��Ʒ��Ϣ�ṹ
********************************************************************/
dload_feature_t*  get_dload_feature(void);


#ifdef  __cplusplus
    }
#endif
#endif /*_DLOAD_PRODUCT_H */
