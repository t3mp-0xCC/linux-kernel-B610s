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

#include <product_config.h>
#include <linux/hisi/rdr_pub.h>
#include "rdr_exc.h"


/*异常类型的添加需要同步修改python脚本中的config.py以及HIDS配置文件的xml配置*/
static struct rdr_exc_type_s  g_stExcType[]={
    RDR_EXC_TYPE(FAST_POWERUP),
    RDR_EXC_TYPE(POWERKEY_POWERUP),
    RDR_EXC_TYPE(VBUS_POWERUP),
    RDR_EXC_TYPE(ALARM_ON_POWERUP),
    RDR_EXC_TYPE(POWERHOLD_POWERUP),
    RDR_EXC_TYPE(POWERKEY_SHUTDOWN),
    RDR_EXC_TYPE(POWERHOLD_SHUTDOWN),
    RDR_EXC_TYPE(POWERKEY_10S_REBOOT),
    RDR_EXC_TYPE(RESETKEY_REBOOT),
    RDR_EXC_TYPE(SOFT_REBOOT),
    RDR_EXC_TYPE(PMU_EXCEPTION),
    RDR_EXC_TYPE(AP_S_WDT_TIMEOUT),
    RDR_EXC_TYPE(AP_S_EXC_PANIC),
    RDR_EXC_TYPE(AP_S_EXC_SFTRESET),
    RDR_EXC_TYPE(AP_S_EXC_PANIC_INT),
    RDR_EXC_TYPE(LPM3_S_EXCEPTION),
    RDR_EXC_TYPE(LPM3_S_WDT_TIMEOUT),
    RDR_EXC_TYPE(TEEOS_S_WDT_TIMEOUT),
    RDR_EXC_TYPE(CP_S_MODEMAP),
    RDR_EXC_TYPE(CP_S_EXCEPTION),
    RDR_EXC_TYPE(CP_S_RESETFAIL),
    RDR_EXC_TYPE(CP_S_NORMALRESET),
    RDR_EXC_TYPE(CP_S_MODEMNOC),
    RDR_EXC_TYPE(CP_S_MODEMDMSS),
    RDR_EXC_TYPE(CP_S_RILD_EXCEPTION),
    RDR_EXC_TYPE(CP_S_3RD_EXCEPTION),



    RDR_EXC_TYPE(RDR_TEST_WDT_TIMEOUT),
    RDR_EXC_TYPE(RDR_TEST_PANIC),
    RDR_EXC_TYPE(RDR_TEST_DIE),
    RDR_EXC_TYPE(RDR_TEST_STACKOVERFLOW),
    RDR_EXC_TYPE(UNDEFINE),
};

/*
 * func name: rdr_get_exception_type
 * get exception core str for core id.
 * func args:
 *    u64 coreid
 * return value
 *	NULL  error
 *	!NULL core str.
 */
char* rdr_get_exception_type(u32 exc_type)
{
    int i;
    char* type = "UNDEF";
    int count;

    count = (int)(sizeof(g_stExcType)/sizeof(g_stExcType[0]));

	for (i=0; i<count; i++) {
		if (g_stExcType[i].exc_type == exc_type) {
			return g_stExcType[i].name;
		}
	}
    return type;
}

static struct rdr_core_info_s g_stCoreInfo[] =
{
    {RDR_AP,"AP"},
    {RDR_CP,"CP"},
    {RDR_LPM3,"LPM3"},
    {RDR_MDMAP,"MDMAP"},
    {RDR_TEEOS,"TEEOS"},
};


char* rdr_get_core_name(u64 coreid)
{
    int i = 0;
    int count;

    count = (int)(sizeof(g_stCoreInfo)/sizeof(g_stCoreInfo[0]));

    for(i=0;i<count;i++)
    {
        if(g_stCoreInfo[i].coreid == coreid)
        {
            return g_stCoreInfo[i].name;
        }
    }
    return "unknown";
}

void rdr_show_exc_type(void)
{
    int i;
    int count;

    count = (int)(sizeof(g_stExcType)/sizeof(g_stExcType[0]));

    /*打印结果可用于python脚本解析使用*/
	for (i=0; i<count; i++) {
        printk("'%s':0x%x,\n",g_stExcType[i].name,g_stExcType[i].exc_type);
	}
    /*打印结果用于HIDS xml解析使用*/
    printk("======================================\n");
	for (i=0; i<count; i++) {
        printk("{0x%x,%s},",g_stExcType[i].exc_type,g_stExcType[i].name);
	}
}

EXPORT_SYMBOL(rdr_show_exc_type);
EXPORT_SYMBOL(rdr_get_exception_type);
EXPORT_SYMBOL(rdr_get_core_name);



