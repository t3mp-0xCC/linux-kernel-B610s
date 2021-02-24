/*
 * register rdr buffer for TEEOS. (RDR: kernel run data recorder.)
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/stat.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/hisi/rdr_pub.h>
#include "tee_rdr_register.h"
#include "tc_ns_client.h"
#include "tee_client_constants.h"
#include "teek_ns_client.h"

#define TEEOS_MODID HISI_BB_MOD_TEE_START
#define TEEOS_MODID_END  HISI_BB_MOD_TEE_END
//static struct semaphore rdr_soc_sem;
struct rdr_register_module_result current_rdr_info;
//static pfn_cb_dump_done pfn_cb_dumpdone;
static u64 current_core_id = RDR_TEEOS; //const
static u32 g_modid;

#if 1
void tee_fn_dump( u32 modid, u32 etype, u64 coreid,
                char* pathname, pfn_cb_dump_done pfn_cb)
{
    BB_PRINT_DBG(" ====================================\n");
    BB_PRINT_DBG(" modid:          [0x%x]\n", modid);
    BB_PRINT_DBG(" coreid:         [0x%llx]\n", coreid);
    BB_PRINT_DBG(" exce tpye:      [0x%x]\n", etype);
    BB_PRINT_DBG(" path name:      [%s]\n", pathname);
    BB_PRINT_DBG(" dump start:     [0x%llx]\n", current_rdr_info.log_addr);
    BB_PRINT_DBG(" dump len:       [%d]\n", current_rdr_info.log_len);
    BB_PRINT_DBG(" nve:            [0x%llx]\n", current_rdr_info.nve);
    BB_PRINT_DBG(" callback:       [0x%p]\n", pfn_cb);
    BB_PRINT_DBG(" ====================================\n");
    g_modid = modid;
    pfn_cb(g_modid, current_core_id);
}
#endif

int tee_rdr_register_core(void){
    struct rdr_module_ops_pub s_module_ops;
    int ret = -1;

    BB_PRINT_START();

    s_module_ops.ops_dump = tee_fn_dump;
    s_module_ops.ops_reset = NULL;

    ret = rdr_register_module_ops(current_core_id, &s_module_ops, &current_rdr_info);

    BB_PRINT_DBG("Address of rdr buffer is %d, buffer len is %d\n"
        ,current_rdr_info.log_addr,current_rdr_info.log_len);
    BB_PRINT_END();
    return ret;
}
const char tee_module_name[] = "RDR_TEEOS";
const char tee_module_desc[] = "RDR_TEEOS crash";
int teeos_register_exception(void)
{
	struct rdr_exception_info_s einfo;
	int ret = -1;

	BB_PRINT_DBG("%s start!\n", __func__);

	memset(&einfo, 0, sizeof(struct rdr_exception_info_s));
	einfo.e_modid = TEEOS_MODID;
	einfo.e_modid_end = TEEOS_MODID_END;
	einfo.e_process_priority = RDR_ERR;
	einfo.e_reboot_priority = RDR_REBOOT_WAIT;
	einfo.e_notify_core_mask = RDR_TEEOS | RDR_AP;
	einfo.e_reset_core_mask = RDR_TEEOS | RDR_AP;
	einfo.e_reentrant = RDR_REENTRANT_ALLOW;
	einfo.e_exce_type = TEE_S_EXCEPTION;
	einfo.e_from_core = RDR_TEEOS;
	einfo.e_upload_flag = RDR_UPLOAD_YES;
	memcpy(einfo.e_from_module, tee_module_name, sizeof(tee_module_name));
	memcpy(einfo.e_desc, tee_module_desc, sizeof(tee_module_desc));
	ret = rdr_register_exception(&einfo);

	BB_PRINT_DBG("%s end!\n", __func__);

	return ret;
}

//Register rdr memory
int TC_NS_register_rdr_mem(void)
{
    TC_NS_SMC_CMD smc_cmd = {0};
    unsigned int smc_cmd_phys;
    int ret = 0;
    unsigned char uuid[17] = {0};
    TC_NS_Operation operation = {0};
    unsigned int rdr_mem_addr;
    unsigned int rdr_mem_len;

    ret = tee_rdr_register_core();

    if (!ret){
        rdr_mem_addr = current_rdr_info.log_addr;
        rdr_mem_len = current_rdr_info.log_len;

        uuid[0] = 1;
        smc_cmd.uuid_phys = virt_to_phys(uuid);
        smc_cmd.cmd_id = GLOBAL_CMD_ID_REGISTER_RDR_MEM;

        operation.paramTypes = TEE_PARAM_TYPE_VALUE_OUTPUT;
        operation.paramTypes = operation.paramTypes << 12;
        operation.params[0].value.a= rdr_mem_addr;
        operation.params[0].value.b = rdr_mem_len;

        smc_cmd.operation_phys = virt_to_phys(&operation);

        ret = TC_NS_SMC(&smc_cmd, 0);
    }

    return ret;
}

unsigned int TC_NS_get_rdr_mem_addr()
{

    return current_rdr_info.log_addr;
}

unsigned int TC_NS_get_rdr_mem_len()
{
    return current_rdr_info.log_len;
}
