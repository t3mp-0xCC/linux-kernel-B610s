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
 
#ifndef __BSP_MEMORY_H__
#define __BSP_MEMORY_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "bsp_memmap.h"
#include "bsp_s_memory.h"
#include "soc_onchiprom.h"
#include "product_config.h"
#include "bsp_shared_ddr.h"

#ifndef __ASSEMBLY__

struct uart_infor
{
	unsigned int flag;          /*flag which denotes wether this uart will be used*/
	unsigned int base_addr;     /*the register base address*/
	unsigned int interrupt_num; /*interrupt number*/
};
#if (FEATURE_ON == MBB_HWTEST) || (FEATURE_ON == MBB_FEATURE_MODULE_AT) 
/*hardware test*/
typedef  struct
{
    unsigned int smem_hw_mode;             /* �Լ�ģʽ*/
    unsigned int smem_hw_option;           /* ������ѡ��*/
    unsigned int smem_reserved;            /* ����*/
}hwtest_smem_info;
#define SRAM_HWTEST_SIZE    12
#define HW_TEST_MAGIC_NUM    (0x33557799)
#endif

#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)  
/*dev test*/
typedef  struct
{
    unsigned int   error_type;  /**< Error type */
    unsigned int   error_addr;  /**< Address where error occurs */
    unsigned int   error_data;  /**< Data pattern where error occurs */
    unsigned int   ddr_status;  /*DDR ����״̬*/
    unsigned int   bitflip_status;/*bitflip ����״̬*/
}hwtest_boot_test_info_stype;
#define SRAM_DEVTEST_SIZE    20
#endif

#if (MBB_CHARGE == FEATURE_ON)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
#define MAX_TEMPS 10
#define RESERVED_LEN 4
struct ss_coul_nv_info{
    int charge_cycles;
    int r_pcb; // uohm
    int v_offset_a;
    int v_offset_b;
    int c_offset_a;
    int c_offset_b;
    short temp[MAX_TEMPS];
    short real_fcc[MAX_TEMPS];
    short calc_ocv_reg_v;
    short calc_ocv_reg_c;
    short hkadc_batt_temp;
    short hkadc_batt_id_voltage;
    int start_cc;
    int ocv_temp;
    int limit_fcc;
    int reserved[RESERVED_LEN];
};

#define SRAM_COUL_SIZE    sizeof(struct ss_coul_nv_info)
#endif
#endif
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
#define CONFIDENTIAL_NV_OPR_SIZE    (12)
typedef  struct
{
    unsigned int smem_confidential_nv_opr_flag;   /* datalock,simlock NV�������� ��־*/
    unsigned int smem_datalock_state;                  /* datalock ״̬*/
    unsigned int smem_reserve;
}confidential_nv_opr_info;
#endif
/*ռ�ÿռ��С�Ĳ���*/
#define SRAM_TEMP_PROTECT_SIZE 32
#define SRAM_DLOAD_SIZE 64
#define SRAM_DSP_MNTN_SIZE 32
#define SRAM_CDSP_MNTN_SIZE 32
struct dfs_ddr_cfg
{
    unsigned int ddr_freq;
    unsigned int ddr_reg_83c;
    unsigned int ddr_reg_abc;
    unsigned int ddr_reg_b3c;
    unsigned int ddr_reg_bbc;
    unsigned int ddr_reg_c3c;
};
typedef struct tag_SRAM_SMALL_SECTIONS
{
	unsigned int        SRAM_USB_ASHELL;
	struct uart_infor   UART_INFORMATION[3];                            /*three uarts:0/1/2*/
	unsigned int        SRAM_ONOFF[8];
	unsigned int        SRAM_DICC[8];				                    /*GUʹ�õ�DICC*/
	unsigned int        SRAM_DSP_DRV;
	unsigned int        SRAM_PCIE_INFO[64];			                    /*DSP�������ʱʹ��*/
	unsigned int        SRAM_RESERVED[65];
	unsigned int        SRAM_WDT_AM_FLAG;	                            /*��������ͣ����־*/
	unsigned int        SRAM_WDT_CM_FLAG;
	unsigned int        SRAM_BUCK3_ACORE_ONOFF_FLAG;                    /*BUCK3���µ��־*/
	unsigned int        SRAM_BUCK3_CCORE_ONOFF_FLAG;
	unsigned int        SRAM_CUR_CPUFREQ_PROFILE;                       /* current profile */
	unsigned int        SRAM_MAX_CPUFREQ_PROFILE;                       /* max profile */
	unsigned int        SRAM_MIN_CPUFREQ_PROFILE;                       /* min profile */
	unsigned int        SRAM_CPUFREQ_DOWN_FLAG[2];
	unsigned int        SRAM_CPUFREQ_DOWN_PROFILE[2];
#if (MBB_COMMON == FEATURE_ON)
    unsigned int        SRAM_MBB_COMMON;          /* for MBB COMMON driver */
#endif
#if (MBB_CHARGE == FEATURE_ON)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
    unsigned int        SRAM_COUL[SRAM_COUL_SIZE];/*���ؼ�NV��Ϣ�洢*/
#endif
#endif
	unsigned int        SRAM_REBOOT_INFO[8];		                    /* E5 ������Ϣ�� �����Ա��޸�*/
	unsigned int        SRAM_TEMP_PROTECT[SRAM_TEMP_PROTECT_SIZE];		/*�±�ʹ�õĸõ�ַ���ܱ��޸�*/
	unsigned char       SRAM_DLOAD[SRAM_DLOAD_SIZE];			        /*����ģ��ʹ�ã������Ա��޸�*/
	struct tagOcrShareData  SRAM_SEC_SHARE;			                    /*onchiprom����ʱ�����Ϣ�ı�־λ������SRAM�ĸߵ�ַ��,�����޸�*/

	unsigned char       SRAM_DSP_MNTN_INFO[SRAM_DSP_MNTN_SIZE];		    /* DSP�����쳣ʱ�Ŀ�ά�ɲ���Ϣ */
	struct dfs_ddr_cfg  SRAM_DFS_DDRC_CFG[2];
	unsigned int 	    SRAM_DUMP_POWER_OFF_FLAG;
	unsigned int 	    SRAM_PM_CHECK_ADDR;
    unsigned int        SRAM_CDSP_DRV;
	unsigned char       SRAM_CDSP_MNTN_INFO[SRAM_CDSP_MNTN_SIZE];		    /* CDSP�����쳣ʱ�Ŀ�ά�ɲ���Ϣ */
 	unsigned int        SRAM_SEC_ROOTCA[ROOT_CA_LEN/4];
#ifdef FEATURE_E5_ONOFF
    unsigned int        SRAM_MBB_PD_CHARGE;          /* for MBB PDCHARGE */
#endif
#if (FEATURE_ON == MBB_HWTEST) || (FEATURE_ON == MBB_FEATURE_MODULE_AT)  
    unsigned char   SRAM_HWTEST[SRAM_HWTEST_SIZE];   /*Ӳ���Լ�*/
#endif

#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)  
    unsigned char   SRAM_DEVTEST[SRAM_DEVTEST_SIZE];   /*DEV TEST*/
#endif
#if (FEATURE_ON == MBB_MLOG)  
    unsigned int   SRAM_ABNORAL_REBOOT;   /*MLOG�쳣����*/
#endif
#ifdef BSP_CONFIG_BOARD_SOLUTION
    unsigned int SRAM_SOLUTION_TYPE;  /*ģ��ʹ��,��������ģ���µ��Ӳ�Ʒ��̬(M2M/CE/����)*/
#endif
#if (FEATURE_ON == MBB_MODULE_PM)
    unsigned int  SRAM_MODULE_PM[2];
#endif
#if (FEATURE_ON == MBB_MLOG)
    unsigned int   SRAM_MLOG_INIT_FLAG;   /*MLOGģ���ʼ���ɹ���־*/
#endif
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
unsigned char SRAM_CONFIDENTIAL_NV_OPR[CONFIDENTIAL_NV_OPR_SIZE];
#endif
}SRAM_SMALL_SECTIONS;

#define SRAM_DLOAD_ADDR   (void *)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_DLOAD)

#if (MBB_COMMON == FEATURE_ON)
#define SRAM_MBB_COMMON_NUM              (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MBB_COMMON)
#endif
#if (MBB_CHARGE == FEATURE_ON)
#if (MBB_CHG_COULOMETER == FEATURE_ON)
#define SRAM_COUL_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_COUL)/*���ؼƹ����ڴ��ַ*/
#endif
#endif
#ifdef FEATURE_E5_ONOFF
#define SRAM_MBB_PD_CHARGE_ON     0x77778888
#define SRAM_MBB_PD_CHARGE_OFF    0x00000000
#define SRAM_MBB_PD_CHARGE_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MBB_PD_CHARGE)
#endif
#if (FEATURE_ON == MBB_HWTEST)  
#define SRAM_HWTEST_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_HWTEST)/*Ӳ���Լ칲���ڴ��ַ*/
#endif
#if (FEATURE_ON == MBB_MLOG)
#define SRAM_ABNORAL_REBOOT_NUM     0x6F545550
#define SRAM_ABNORAL_REBOOT_CLEAR   0x00000000
#define SRAM_ABNORMAL_REBOOT_ADDR (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_ABNORAL_REBOOT)/*�쳣���������ڴ��ַ*/
#endif
#ifdef BSP_CONFIG_BOARD_SOLUTION
/*ģ���Ӳ�Ʒ��̬(M2M/CE/����)�����ڴ��ַ*/
#define SRAM_SOLUTION_TYPE_ADDR  (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_SOLUTION_TYPE)
#endif
#if (FEATURE_ON == MBB_MODULE_PM)
#define USB_SUSPEND_STATUS          (0x12345678)
#define USB_RESUME_STATUS           (0x87654321)
#define PIN_SUSPEND_STATUS          (0x12345678)
#define PIN_RESUME_STATUS           (0x87654321)
#define SRAM_MODULE_PM_ADDR  (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MODULE_PM)/*ģ���Դ�������ڴ��ַ*/
#endif
#if (FEATURE_ON == MBB_FEATURE_MODULE_AT)  
#define SRAM_DEVTEST_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_DEVTEST)/*ddr �Լ칲���ڴ��ַ*/
#endif

#if (FEATURE_ON == MBB_MLOG)
#define SRAM_MLOG_INIT_VALUE      0x5A5AAA55
#define SRAM_MLOG_INIT_ADDR    (unsigned int)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MLOG_INIT_FLAG)
#endif
#if (FEATURE_ON == MBB_SIMLOCK_FOUR)
#define SMEM_DATALOCK_STATE_NUM   0X8D79F60A  /*DATALOCK����״̬ħ���� */
#define SMEM_CONFIDENTIAL_NV_OPR_FLAG_NUM   0X9C7C021D  /*�����ȡDATALOCK\SIMLOCK NVħ���� */
#define SRAM_CONFIDENTIAL_NV_OPR_FLAG_CLEAR   0x00000000
#define SRAM_CONFIDENTIAL_NV_OPR_ADDR (unsigned int)(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_CONFIDENTIAL_NV_OPR)
#endif

/*RTT �͹��Ĳ���ʹ�õ�AXI,��ű�־����Ϣ,��32�ֽ�*/
typedef struct tag_SRAM_RTT_SLEEP_FLAG
{
    unsigned int        SRAM_DSP_FLAG;          /*DSPʹ�ã����DSP�ǽ�����˯������ǳ˯�ı�־*/
}SRAM_RTT_SLEEP_FLAG;

#if(FEATURE_ON == MBB_DLOAD)
#define SMEM_DLOAD_FLAG_NUM          0X444C464E  /*DLFN*/
#define SMEM_SDUP_FLAG_NUM           0X53445550  /*SDUP*/
#define SMEM_DLOAD_FLAG_NUM          0X444C464E  /*DLFN*/
#define SMEM_SUOK_FLAG_NUM           0X53554F4B  /*SUOK  Scard Upgrade OK*/
#define SMEM_SUFL_FLAG_NUM           0X5355464C  /*SUFL   Scard Upgrade Fail*/
#define SMEM_SDNR_FLAG_NUM           0X53444E52  /*SDNR SD������ NV�Զ��ָ��׶�  ħ����*/
#define SMEM_ONNR_FLAG_NUM           0X4F4E4E52  /*ONNR  ���� ���� NV�Զ��ָ��׶�  ħ����*/
#define SMEM_RECA_FLAG_NUM           0X5245434A  /*RECA*/
#define SMEM_RECB_FLAG_NUM           0X5245434B  /*RECB*/
#define SMEM_SWITCH_PUCI_FLAG_NUM    0X444E5350  /*NV�Զ��ָ��׶�PCUI�˿���̬*/
#define SMEM_FORCELOAD_FLAG_NUM      0X46434C44  /*FCLD Modem������־ ħ����*/
#define SMEM_FORCELOAD_SUCCESS_NUM  0X4643534E /*FCSN  �����ɹ�ħ���֣�    
                                                       ���������Զ��ָ�Ҳ�滻Ϊ��ħ����,   
                                                       �������������Զ��ָ���ص�ħ���ֲ���ʹ�� */
#define SMEM_ONLINE_AUTO_UPDATE_ENTER_NUM   0x4F415545 /*OAUE ���������Զ��ָ�����ħ����*/
#define SMEM_ONLINE_AUTO_UPDATE_SUCCESS_NUM 0x4F415553 /*OAUS ���������Զ��ָ��ɹ�ħ����*/
#define SMEM_BURN_UPDATE_FLAG_NUM           0x4255464E /*BUFN ��Ƭ�汾����ħ����*/
#define SMEM_MULTIUPG_FLAG_NUM                  0x4D545550 /* MTUP �鲥����ħ����*/
#define SMEM_ONUP_FLAG_NUM           0x53555246    /* ��������ħ���� */
#define SMEM_DATALOCK_STATUS_FLAG_NUM   0X444C554C  /* DATALOCK ����״̬ħ���� */
#define SMEM_DT_UPDATE_FLAG_NUM       0x44545546   /*DT���ر�־DTUF*/

typedef  struct
{
    unsigned int smem_dload_flag;                  /* ����Ƿ��������ģʽ */
    unsigned int smem_new_recovery_flag;           /* ��ʶ����������Ҫ���ص�recovery flag*/
    unsigned int smem_sd_upgrade;                  /* ��¼SD������������ʶλ������ʱ����SD����ȡ�����ļ� */
    unsigned int smem_switch_pcui_flag;            /* ��ʶNV�Զ��ָ��׶Σ�����ʱUSB�˿���̬����*/
    unsigned int smem_online_upgrade_flag;         /* ��¼��������/SD������NV �Զ��ָ��׶Σ�APP������ */
    unsigned int smem_forceload_flag;              /* ����Ƿ��������ģʽ */
    unsigned int smem_online_auto_updata_flag;     /* ��ʶ���������Զ��ָ���ʼ���ɹ�*/ 
    unsigned int smem_burn_update_flag;            /* ��Ƭ�汾������ʶ*/        
    unsigned int smem_multiupg_flag;               /* �鲥������ʶ*/ 
    unsigned int smem_update_times;                /* mlog��������ͳ�Ʊ�ʶ*/
    unsigned int smem_datalock_status;             /* datalock����״̬ */
    unsigned int smem_dt_update_flag;             /*dt���������ڴ��־*/
    unsigned int smem_reserve7;                   /*7 -- 11 ��ʱ����*/
    unsigned int smem_reserve8;
    unsigned int smem_reserve9;
    unsigned int smem_reserve10;
}huawei_smem_info;
#endif  /*MBB_DLOAD*/

#endif/*__ASSEMBLY__*/


#if !defined(__KERNEL__) && !defined(__OS_VXWORKS__) && !defined(__OS_RTOSCK__)

#ifdef CONFIG_SRAM_SECURE
#define SRAM_BASE_ADDR                  (SHM_BASE_ADDR+SHM_OFFSET_SRAM_TO_DDR)
#define SRAM_MEM_TOP_ADDR               (SRAM_BASE_ADDR + HI_SRAM_MEM_SIZE )
#else
#define SRAM_MEM_TOP_ADDR				(HI_SRAM_MEM_BASE_ADDR + HI_SRAM_MEM_SIZE )
#define SRAM_BASE_ADDR                  (HI_SRAM_MEM_BASE_ADDR)
#endif

#else

#ifdef CONFIG_SRAM_SECURE
#define SRAM_BASE_ADDR                  (void *)((unsigned long)SHM_BASE_ADDR+SHM_OFFSET_SRAM_TO_DDR)
#define SRAM_MEM_TOP_ADDR               (void *)((unsigned long)SRAM_BASE_ADDR + HI_SRAM_MEM_SIZE )
#define SRAM_V2P(addr)                  SHD_DDR_V2P(addr)
#define SRAM_P2V(addr)                  SHD_DDR_P2V(addr)
#else
#define SRAM_MEM_TOP_ADDR				(void *)((unsigned long)g_mem_ctrl.sram_virt_addr + g_mem_ctrl.sram_mem_size)
#define SRAM_BASE_ADDR                  (g_mem_ctrl.sram_virt_addr)
#define SRAM_V2P(addr)                  (void *)((unsigned long)(addr) - (unsigned long)g_mem_ctrl.sram_virt_addr + (unsigned long)g_mem_ctrl.sram_phy_addr)
#define SRAM_P2V(addr)                  (void *)((unsigned long)(addr) - (unsigned long)g_mem_ctrl.sram_phy_addr + (unsigned long)g_mem_ctrl.sram_virt_addr)
#endif

#endif

#define SRAM_OFFSET_SMALL_SECTIONS      (0)
#define SRAM_SIZE_SMALL_SECTIONS		(2*1024) /*Ԥ��2k��С����Ϊ����*/

/* MCU PM 1K */
#define SRAM_OFFSET_MCU_RESERVE         (SRAM_OFFSET_SMALL_SECTIONS + SRAM_SIZE_SMALL_SECTIONS)
#define SRAM_SIZE_MCU_RESERVE           (1*1024)

/* ICCͨ��(mcore��acore; mcore��ccore), ��2KB, ��4KB */
#define SRAM_OFFSET_ICC                 (SRAM_OFFSET_MCU_RESERVE + SRAM_SIZE_MCU_RESERVE)
#define SRAM_SIZE_ICC                   (4*1024)


/*LDSP ʹ�� ����rtt ǳ˯��־/ ��̬��Ƶ��ѹ
��һ��4�ֽ�Ϊǳ˯��־  ��Ϊ1��ʾ�˴�˯����dspǳ˯*/
#define SRAM_OFFSET_TLDSP_SHARED        (SRAM_OFFSET_ICC + SRAM_SIZE_ICC)
#define SRAM_SIZE_TLDSP_SHARED          (96)

#define SRAM_OFFSET_GU_MAC_HEADER       (SRAM_OFFSET_TLDSP_SHARED + SRAM_SIZE_TLDSP_SHARED)
#ifndef SRAM_SIZE_GU_MAC_HEADER
#define SRAM_SIZE_GU_MAC_HEADER         (56*1024)
#endif

/*SRAM��̬��*/
#define SRAM_OFFSET_DYNAMIC_SEC         (SRAM_OFFSET_GU_MAC_HEADER + SRAM_SIZE_GU_MAC_HEADER)
#define SRAM_SIZE_DYNAMIC_SEC           ((unsigned long)SRAM_MEM_TOP_ADDR - ((unsigned long)SRAM_BASE_ADDR + SRAM_OFFSET_DYNAMIC_SEC))

#ifdef __cplusplus
}
#endif

#endif

