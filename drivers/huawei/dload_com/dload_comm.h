
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


#ifndef _DLOAD_COMM_H_
#define _DLOAD_COMM_H_
#ifdef  __cplusplus
    extern "C"{
#endif


/****************************************************************************
  2 ���������ض���
*****************************************************************************/

typedef int                int32;
typedef short              int16;
typedef char               int8;
typedef unsigned long long uint64;
typedef long long          int64;
typedef unsigned long      uint32;
typedef unsigned short     uint16;
typedef unsigned char      uint8;
typedef unsigned char      boolean;

/****************************************************************************
 3 ö�����ͣ��ṹ�壬�궨��
 �ṹ�����ͼ� :_s
 ö�����ͼ�   :_e
 ���������ͼ� :_u
 
****************************************************************************/
#define EXTERN   extern
#define STATIC   static
#define CONST    const

#ifndef NULL
#define NULL 0
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define DLOAD_ERR        -1
#define DLOAD_OK          0

#define DLOAD_NAME_MAX_LEN 256

/****************************************************************************
 3 ö�����ͣ��ṹ�壬�궨��
 �ṹ�����ͼ� :_s
 ö�����ͼ�   :_e
 ���������ͼ� :_u
****************************************************************************/
typedef enum
{
    DLOAD_BALONG,  /* ����ƽ̨ */
    DLOAD_QCOMM,   /* ��ͨƽ̨ */
}platform_type_t;

typedef enum
{
    DLOAD_STICK,    /* ���ݿ���̬ */
    DLOAD_HILINK,   /* ���翨��̬ */
    DLOAD_WINGLE,   /* wingle��̬ */
    DLOAD_E5,       /* ͨ��E5��̬ */
    DLOAD_CPE,      /* CPE��̬ */
    DLOAD_MOD_M2M,      /* ģ��m2m��̬ */
    DLOAD_MOD_CE,      /* ģ��ce��̬ */
    DLOAD_MOD_CAR,      /* ģ�鳵����̬ */
    DLOAD_UDP,      /* UDP��̬ */
}product_type_t;

/* ����״̬���� */
typedef enum
{
    DLOAD_STATE_NORMAL_IDLE,       /*����״̬*/
    DLOAD_STATE_NORMAL_RUNNING,    /*����������*/
    DLOAD_STATE_AUTOUG_RUNNING,    /*����������*/
    DLOAD_STATE_NORMAL_LOW_POWER,  /*SD��������ص͵�*/
    DLOAD_STATE_NORMAL_SUCCESS,    /*SD�������ɹ�*/
    DLOAD_STATE_NORMAL_FAIL,       /*SD����������������ʧ��ʱ��LCD��ʾ*/
    DLOAD_STATE_FAIL,              /*������������������ʾʧ�� �� usb�����������������ʧ�ܣ�E5 �ڴ�����״̬��lcd ��ʾidle*/
    DLOAD_STATE_INVALID,
    /* ���֧��10��״̬ */
}dload_state_t;

typedef struct _PRODUCT_INFO
{
    platform_type_t  platfm_id_e; /* ƽ̨��ʶ */
    product_type_t   prodct_id_e; /* ��Ʒ��̬ */
    uint32           hw_major_id; /* ��ƷӲ����ID */
    uint32           hw_minor_id; /* ��ƷӲ����ID */
    uint8            platfm_name[DLOAD_NAME_MAX_LEN]; /* ƽ̨���� */
    uint8            prodct_name[DLOAD_NAME_MAX_LEN]; /* �����Ʒ���� */
    uint8            upgrade_id[DLOAD_NAME_MAX_LEN];  /* ��Ʒ����ID */
    uint8            feature_name[DLOAD_NAME_MAX_LEN];/* ��Ʒ��̬��Ӧ������ */
}product_info_t;

/* ƽ̨��Ϣ�ṹ�� */
typedef struct _PLATFORM_INFO
{
    product_info_t  product_info_st; /* ƽ̨��Ʒ��Ϣ */
    struct _PLATFORM_FEATURE
    {
        boolean  led_feature;           /* LED���� TRUE:֧�֣�FALSE: ��֧�� */
        boolean  lcd_feature;           /* LCD���� TRUE:֧�֣�FALSE: ��֧�� */
        boolean  reserved4;             /* Ԥ�� */
        boolean  reserved3;             /* Ԥ�� */
        boolean  reserved2;             /* Ԥ�� */
        boolean  reserved1;             /* Ԥ�� */
    }platform_feature;
}platform_info_t;

/*------Fota����������Զ���begin--------*/
/*Fota����fs�ļ����ö�ٽṹ,��֧��16���ļ���ʽ*/
typedef enum
{
    FSFILE_SYSTEM_CPIO = 0,  /*system.cpio�ļ����*/
    FSFILE_APP_CPIO,  /*app.cpio�ļ����*/
    FSFILE_WEBUI_CPIO,  /*webui.cpio�ļ����*/
    FSFILE_MODEM_CPIO,  /*modem.cpio�ļ����*/
    /*�����ļ��ڴ���չ*/

    FSFILE_MAX = 16
}dload_fota_fsfile_t;
/*Fota����fs�ļ�ϵͳ����ö�ٽṹ,���֧��4���ļ���ʽ*/
typedef enum
{
    FSTYPE_YAFFS2 = 0,  /*yaffs2�ļ�ϵͳ����*/
    FSTYPE_JFFS2 = 1,  /*jffs2�ļ�ϵͳ����*/
    /*�����ļ�ϵͳ�����ڴ���չ*/

    FSTYPE_MAX = 4
}dload_fota_fstype_def_t;
/*Fota���Զ���Ԥ����չ�ռ�*/
#define FOTA_FEATURE_RESERV_NUM  6
/*ȡֵ����궨��,���ೡ������������ĺ궨����ʹ��ʱ���*/
#define FOTA_FSLIST(fsfile)  ((1 << fsfile) & 0xFFFF)
#define FOTA_FSTYPE(fsfile, fstype)  ((fstype << (2 * fsfile)) & 0xFFFFFFFF)
#define FOTA_FSTYPE_MASK(fsfile)  ((0x3 << fsfile) & 0xFFFFFFFF)
#define FOTA_FSLIST_BALONG_COMMON  \
    (FOTA_FSLIST(FSFILE_SYSTEM_CPIO) | FOTA_FSLIST(FSFILE_APP_CPIO) | FOTA_FSLIST(FSFILE_WEBUI_CPIO))
#define FOTA_FSTYPE_BALONG_COMMON  \
    (FOTA_FSTYPE(FSFILE_SYSTEM_CPIO, FSTYPE_YAFFS2) | FOTA_FSTYPE(FSFILE_APP_CPIO, FSTYPE_YAFFS2) \
    | FOTA_FSTYPE(FSFILE_WEBUI_CPIO, FSTYPE_YAFFS2))
#define FOTA_FSLIST_QC_COMMON  \
    (FOTA_FSLIST(FSFILE_SYSTEM_CPIO) | FOTA_FSLIST(FSFILE_APP_CPIO) | FOTA_FSLIST(FSFILE_WEBUI_CPIO) | FOTA_FSLIST(FSFILE_MODEM_CPIO))
#define FOTA_FSTYPE_QC_COMMON  \
    (FOTA_FSTYPE(FSFILE_SYSTEM_CPIO, FSTYPE_YAFFS2) | FOTA_FSTYPE(FSFILE_APP_CPIO, FSTYPE_YAFFS2) \
    | FOTA_FSTYPE(FSFILE_WEBUI_CPIO, FSTYPE_YAFFS2) | FOTA_FSTYPE(FSFILE_MODEM_CPIO, FSTYPE_YAFFS2))
/*Fota����fs�ļ�list*/
typedef union _DLOAD_FOTA_FSLIST
{
    uint16 fslist_value;
    struct
    {
        uint16 fslist_system_cpio  :1;  /*bit[0] -- system.cpio*/
        uint16 fslist_app_cpio  :1;  /*bit[1] -- app.cpio*/
        uint16 fslist_webui_cpio  :1;  /*bit[2] -- webui.cpio*/
        uint16 fslist_modem_cpio  :1;  /*bit[3] -- modem.cpio*/
        uint16 fslist_reserved  :12;  /*bit[4]-bit[15] -- reserved*/
    }fsfile_list;
}dload_fota_fslist_t;
/*Fota����fs�ļ�����,4byte����,ÿ2bit��Ӧ����һ���ļ�*/
/*ÿ2bit���ݴ������¶�Ӧ��ϵ:
   00 -- yaffs2�ļ�ϵͳ
   01 -- jiffs�ļ�ϵͳ
   10/11 -- Ԥ��*/
typedef union _DLOAD_FOTA_FSTYPE
{
    uint32 fstype_value;
    struct
    {
        uint32 fstype_system_cpio  :2;  /*bit[0]-bit[1] -- system.cpio*/
        uint32 fstype_app_cpio  :2;  /*bit[2]-bit[3] -- app.cpio*/
        uint32 fstype_webui_cpio  :2;  /*bit[4]-bit[5] -- webui.cpio*/
        uint32 fstype_modem_cpio  :2;  /*bit[6]-bit[7] -- modem.cpio*/
        uint32 fstype_reserved :24;  /*bit[8]-bit[31] -- reserved*/
    }fsfile_type;
}dload_fota_fstype_t;
/*Fota����������Ϣ�ṹ��*/
typedef struct _DLOAD_FOTA_FEATURE
{
    dload_fota_fslist_t fota_fslist;  /*fota�����е�fs�ļ��б�*/
    dload_fota_fstype_t fota_fstype;  /*fota�����е��ļ�����*/
    uint32 fota_memsize;  /*fota������Ҫ���ڴ��С*/
    boolean reserved[FOTA_FEATURE_RESERV_NUM];  /*fota����Ԥ��6���ֽ��Ժ���չ*/
}dload_fota_feature_t;
/*------Fota����������Զ���end--------*/

/* �����������ýṹ�� */
typedef struct _SAFETY1_3_DLOAD_FEATURE
{
    boolean safe_support;    /* ��ȫ����v1.3����֧��, TRUE: ֧�֣� FALSE: ��֧�� */
    boolean datalock_q;      /* datalock֧�֣�TRUE: ֧�ֿ�C�汾�������� FALSE: ��֧��  */
    boolean server_sign;     /* ������ǩ����FALSE: ����ǩ��  TRUE: ������ǩ�� */
    boolean en_dver_check;   /* d �汾У�鿪�� */
}dload_safety1_3_t;
#define  FEATURE_RESERV_NUM   50 - sizeof(dload_fota_feature_t) - sizeof(dload_safety1_3_t)     /* ����Ԥ�����Ը��� */
typedef struct _DLOAD_FEATURE
{
    uint32           hw_major_id;        /* ��ƷӲ����ID */
    uint32           hw_minor_id;        /* ��ƷӲ����ID */
    boolean          usb_dload;          /* ����: USB���� */
    boolean          scard_dload;        /* ����: SD���� */
    boolean          online_dload;       /* ����: �������� */
    boolean          safety_dload;       /* ����: ��ȫ���� */
    boolean          hota_dload;         /* ����: HOTA���� */
    boolean          fota_dload;         /* ����: FOTA���� */
    dload_fota_feature_t  fota_feature;  /*Fota����������Զ���*/
	dload_safety1_3_t  safety1_3_dload;    /* ����: ��ȫ����1.3���� */
    boolean          reserved[FEATURE_RESERV_NUM]; /* Ԥ�� */
} dload_feature_t;


/* �����ڴ����ö�ٶ��� */
typedef enum
{
    SET_SMEM_DLOAD_MODE_MAGIC,             /* ���ý�������ģʽ��־ */
    SET_SMEM_DLOAD_MODE_ZERO,              /* ���ý�������ģʽ��־���� */
    GET_SMEM_IS_DLOAD_MODE,                /* ��ȡ�Ƿ��������ģʽ */
    SET_SMEM_SCARD_DLOAD_MAGIC,            /* ���ð�������SD������־ */
    SET_SMEM_SCARD_DLOAD_ZERO,             /* ���ð�������SD������־���� */  
    GET_SMEM_IS_SCARD_DLOAD,               /* ��ȡ�Ƿ���ҪSD���� */  
    SET_SMEM_SWITCH_PORT_MAGIC,            /* ����NV�Զ��ָ��׶�USB�˿���̬��־ */
    SET_SMEM_SWITCH_PORT_ZERO,             /* ����NV�Զ��ָ��׶�USB�˿���̬��־���� */ 
    GET_SMEM_IS_SWITCH_PORT,               /* ��ȡ�Ƿ�NV�Զ��ָ��׶�USB�˿���̬���л� */  
    SET_SMEM_ONL_RESTORE_REBOOT_MAGIC,    /* ������������NV�Զ��ָ���ɺ󵥰���Ҫ���� */
    SET_SMEM_ONL_RESTORE_REBOOT_ZERO,     /* ������������NV�Զ��ָ���ɺ󵥰���Ҫ�������� */
    GET_SMEM_ONL_IS_RESTORE_REBOOT,       /* ��ȡ��������NV�Զ��ָ���ɺ󵥰��Ƿ���Ҫ���� */
    SET_SMEM_SD_RESTORE_REBOOT_MAGIC,     /* ����SD����NV�Զ��ָ���ɺ󵥰�������־ */  
    SET_SMEM_SD_RESTORE_REBOOT_ZERO,      /* ����SD����NV�Զ��ָ���ɺ󵥰�������־���� */  
    GET_SMEM_SD_IS_RESTORE_REBOOT,        /* ��ȡSD����NV�Զ��ָ���ɺ󵥰��Ƿ���Ҫ���� */  
    SET_SMEM_FORCE_MODE_MAGIC,             /* ���ý�������ģʽ��־ */
    SET_SMEM_FORCE_MODE_ZERO,              /* ���ý�������ģʽ��־���� */  
    SET_SMEM_FORCE_SUCCESS_MAGIC,          /* ��������ģʽ�����ɹ���־ */  
    GET_SMEM_IS_FORCE_MODE,                 /* ��ȡ�Ƿ��������ģʽ */  
    SET_SMEM_EXCEP_REBOOT_INTO_ONL_MAGIC, /* �����쳣������ν�������������־ */
    SET_SMEM_EXCEP_REBOOT_INTO_ONL_ZERO,  /* �����쳣������ν�������������־���� */  
    GET_SMEM_IS_EXCEP_REBOOT_INTO_ONL,    /* ��ȡ�쳣��������Ƿ������������ */  
    SET_SMEM_UT_DLOAD_MAGIC,                /* ���ö�̨����������־ */  
    SET_SMEM_UT_DLOAD_ZERO,                 /* ���ö�̨����������־���� */  
    GET_SMEM_IS_UT_DLOAD,                   /* ��ȡ�Ƿ��Ƕ�̨�������� */ 
    SET_SMEM_MULTIUPG_DLOAD_MAGIC,         /* �����鲥������־ */
    GET_SMEM_DATALOCK_STATUS_MAGIC,         /* ��ȡdatalock�Ƿ������� */
    SET_SMEM_DATALOCK_MODE_ZERO,            /* ����datalock״̬��� */
}smem_visit_t;

/*************************************************��ʾģ�鹫����Ϣ************************************************/
#define STR_MAX_LEN            (64)
#define PATH_MAX_LEN           (256)

/* ״̬����ָʾ */
typedef enum
{
    LED_READ,
    LED_WRITE,
    LCD_READ,
    LCD_WRITE,
}oper_mode_t;

/* ·����ʶ��ӳ��ṹ */
typedef struct
{
    uint32  path_index;
    char    path_content[PATH_MAX_LEN];
}path_info_t;

/* ���� lcd/led ����״̬���ݽṹ */
typedef struct
{
    uint32        up_state;
    oper_mode_t   oper_mode;
    union
    {
        uint32    path_index;
        int32     num_value;
        char      str_value[STR_MAX_LEN];
    };
    uint32        sleep_len;
}upstate_info_t;

#ifdef  __cplusplus
    }
#endif

#endif /* _DLOAD_COMM_H_ */
