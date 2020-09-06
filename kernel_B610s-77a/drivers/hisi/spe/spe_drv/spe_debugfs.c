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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/netfilter_ipv6/ip6_tables.h>

#include "spe.h"
#include "spe_dbg.h"

#define MAX_CMD_LEN     (128)
#define MAX_CMD_ARGS    (4)

static void spe_testmode_statsclr(struct spe *spe);
static void spe_testmode_help(void);


typedef  void (* spe_dgb_func_t)(int arg, ...) ;
struct spe_testmode_cmd
{
    //char name[MAX_CMD_LEN];
    char* name;
    spe_dgb_func_t func;
};

static void msg_level(int level)
{
    spe_balong.msg_level = (uint32_t)level;
    return;
}

static void dbg_level(int level)
{
    spe_balong.dbg_level = (uint32_t)level;
    return;
}

static void entry_alive(int timeout)
{
    spe_balong.ipfw_timeout = (uint32_t)timeout;
    return;
}

static void set_flags(int flg)
{
    spe_balong.flags = (uint32_t)flg;
    return;
}

static void stats_clr(void)
{
    spe_testmode_statsclr(&spe_balong);
    return;
}

static void help(void)
{
    spe_testmode_help();
    return;
}

static int spe_entrydump_show(struct seq_file *s, void *unused)
{
	//struct spe *spe = s->private;

	printk("seq proto    sip:sport    dip:dport S/DNAT    natip:natport\n");

	return 1;
}

static int spe_entrydump_open(struct inode *inode, struct file *file)
{
	return single_open(file, spe_entrydump_show, inode->i_private);
}


static int spe_testmode_show(struct seq_file *s, void *unused)
{
	struct spe *spe = s->private;
	int i;

    printk("module name             :%s\n",spe->name);
    printk("compile info            :%s\n",spe->compile);
    printk("msg_level               :%u\n",spe->msg_level);
    printk("dbg_level               :%u\n",spe->dbg_level);
    printk("flags                   :%u\n",spe->flags);

    for(i=0;i<SPE_PORT_NUM;i++){
    printk("evt_td_complt[%d]           :%u\n",i,spe->stat.evt_td_complt[i]);
    printk("evt_rd_complt[%d]           :%u\n",i,spe->stat.evt_rd_complt[i]);
    }
	
    printk("evt_td_errport          :%u\n",spe->stat.evt_td_errport);
    printk("evt_rd_errport          :%u\n",spe->stat.evt_rd_errport);
    printk("evt_buf_rd_err          :%u\n",spe->stat.evt_buf_rd_err);
    //printk("wait_ready              :%u\n",spe->stat.wait_ready);
    printk("rd config               :%u\n",spe->stat.evt_rd);
    printk("td config               :%u\n",spe->stat.evt_td);
    printk("ipfw_add                :%u\n",spe->stat.ipfw_add);
    printk("ipfw_add_enter          :%u\n",spe->stat.ipfw_add_enter);
    printk("ipfw_add_leave          :%u\n",spe->stat.ipfw_add_leave);
    printk("ipfw_del                :%u\n",spe->stat.ipfw_del);
    printk("ipfw_del_enter          :%u\n",spe->stat.ipfw_del_enter);
    printk("ipfw_del_leave          :%u\n",spe->stat.ipfw_del_leave);
    printk("ipfw_del_leave          :%u\n",spe->stat.ipfw_del_nothing_leave);
    printk("disable_timeout         :%u\n",spe->stat.disable_timeout);

	return 0;
}

static int spe_testmode_open(struct inode *inode, struct file *file)
{
	return single_open(file, spe_testmode_show, inode->i_private);
}

static int spe_om_show(struct seq_file *s, void *unused)
{
	struct spe *spe = s->private;

    printk("wrr_debug:%d\n",spe_readl(spe->regs, HI_SPE_WRR_DEBUG_OFFSET));
    printk("unpack_debug:%d\n",spe_readl(spe->regs, HI_SPE_UNPACK_DEBUG_OFFSET));
    printk("eng_debug:%d\n",spe_readl(spe->regs, HI_SPE_ENG_DEBUG_OFFSET));
    printk("pack_debug:%d\n",spe_readl(spe->regs, HI_SPE_PACK_DEBUG_OFFSET));
    printk("done_debug:%d\n",spe_readl(spe->regs, HI_SPE_DONE_DEBUG_OFFSET));
    printk("if_debug:%d\n",spe_readl(spe->regs, HI_SPE_IF_DEBUG_OFFSET));
    printk("lp_rd_block_cnt:%d\n",spe_readl(spe->regs, HI_SPE_LP_RD_BLOCK_CNT_OFFSET));
    printk("hp_rd_block_cnt:%d\n",spe_readl(spe->regs, HI_SPE_HP_RD_BLOCK_CNT_OFFSET));
    printk("pack_overtime_cnt:%d\n",spe_readl(spe->regs, HI_SPE_PACK_OVERTIME_CNT_OFFSET));
    printk("pack_push_cnt:%d\n",spe_readl(spe->regs, HI_SPE_PACK_PUSH_CNT_OFFSET));
	printk("enent_buff0_ptr:0x%x\n",spe_readl(spe->regs, SPE_EVENT_BUF_PTR_OFFSET(BUFFER_0_EVENT)));
	printk("enent_buff0_ptr:0x%x\n",spe_readl(spe->regs, SPE_EVENT_BUF_PTR_OFFSET(BUFFER_1_EVENT)));
	return 1;
}

static int spe_om_open(struct inode *inode, struct file *file)
{
	return single_open(file, spe_om_show, inode->i_private);
}

#define BUILD_DEBUG_CMD(func) {#func, (spe_dgb_func_t)func}
static struct spe_testmode_cmd spe_testmode_cmdtbl[]=
{
    /* cmd in spe_dbgfs */
    BUILD_DEBUG_CMD(msg_level),
    BUILD_DEBUG_CMD(dbg_level),
    BUILD_DEBUG_CMD(entry_alive),
    BUILD_DEBUG_CMD(set_flags),
    BUILD_DEBUG_CMD(stats_clr),

    /* cmd in spe_dbg */
    BUILD_DEBUG_CMD(spe_fw_time_show),
    BUILD_DEBUG_CMD(spe_dump_td),
    BUILD_DEBUG_CMD(spe_dump_rd),
    BUILD_DEBUG_CMD(spe_print_td),
    BUILD_DEBUG_CMD(spe_print_rd),
    BUILD_DEBUG_CMD(spe_dev_setting),
    BUILD_DEBUG_CMD(spe_dev_info),
    BUILD_DEBUG_CMD(spe_port_setting),
    BUILD_DEBUG_CMD(spe_port_rd_info),
    BUILD_DEBUG_CMD(spe_port_td_info),
    BUILD_DEBUG_CMD(spe_ipfw_setting),
    BUILD_DEBUG_CMD(spe_port_info_all),

    BUILD_DEBUG_CMD(help)
};

static void spe_testmode_help(void)
{
    u32 loop;

    printk("spe debugfs cmdlist:\n");
    for(loop=0; loop<sizeof(spe_testmode_cmdtbl)/sizeof(struct spe_testmode_cmd); loop++)
    {
        printk("%s\n",spe_testmode_cmdtbl[loop].name);
    }
}

static void spe_testmode_statsclr(struct spe *spe)
{
    memset((void *)&spe->stat, 0 , sizeof(spe->stat));
}

static ssize_t spe_testmode_write(struct file *file,
		const char __user *ubuf, size_t count, loff_t *ppos)
{
	//struct seq_file *s = file->private_data;
	//struct spe  *spe = s->private;
	//unsigned long   flags;
	char separator[] = " ,\n";
	static char buf[MAX_CMD_LEN] = {0};
	char* cmd_str, *cmd_str_find;
    unsigned long   loop, i = 0;
    int    cmd_val[MAX_CMD_ARGS] = {0};
    int    cmd_len;

	if(sizeof(buf)<=count)
		return -EFAULT;

	if (copy_from_user(buf, ubuf, count))
		return -EFAULT;

    cmd_str = buf;
	cmd_len = strnlen(cmd_str, MAX_CMD_LEN);
    cmd_str_find = strsep(&cmd_str, separator);
    if (NULL == cmd_str) {
        return -EINVAL;
    }

    /* find function */
    for(loop=0; loop<sizeof(spe_testmode_cmdtbl)/sizeof(struct spe_testmode_cmd); loop++){
        if(!memcmp(cmd_str_find, spe_testmode_cmdtbl[loop].name,strlen(spe_testmode_cmdtbl[loop].name))){
            break;
        }
    }

    if(sizeof(spe_testmode_cmdtbl)/sizeof(struct spe_testmode_cmd) == loop) {
        printk("unsupport cmd(%s)!\n",cmd_str);
        return -EIO;
    }

    /* find args */
    while((int)(cmd_str - buf) < cmd_len) {
        /*
         * get one arg, set base param to 0,
         * kernel will fixup the right hex or dec
         */
        cmd_str_find = strsep(&cmd_str, separator);

		if(!cmd_str){
			SPE_ERR("strsep return null\n");
			return -EFAULT;
		}
		
        if (kstrtou32(cmd_str_find, 0, &cmd_val[i++]) < 0) {
            break;
        }

        if (i >= MAX_CMD_ARGS) {
            break;
        }
    }

    /* do func */
    if (spe_testmode_cmdtbl[loop].func) {
        spe_testmode_cmdtbl[loop].func(cmd_val[0],
            cmd_val[1], cmd_val[2], cmd_val[3]);
    }

	return count;
}

static const struct file_operations spe_entrydump_fops = {
	.open			= spe_entrydump_open,
	.read			= seq_read,
	.release		= single_release,
};

static const struct file_operations spe_testmode_fops = {
	.open			= spe_testmode_open,
	.write			= spe_testmode_write,
	.read			= seq_read,
	.llseek			= seq_lseek,
	.release		= single_release,
};
static const struct file_operations spe_om_fops = {
	.open			= spe_om_open,
	.read			= seq_read,
	.release		= single_release,
};


int spe_debugfs_init(struct spe *spe)
{
	struct dentry		*root;
	struct dentry		*file;
	int			ret;

	root = debugfs_create_dir(spe->name, NULL);
	if (!root) {
		ret = -ENOMEM;
		goto err0;
	}

	spe->root = root;

	file = debugfs_create_file("testmode", S_IRUGO | S_IWUSR, root,
			spe, &spe_testmode_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	file = debugfs_create_file("entrydump", S_IRUGO, root,
			spe, &spe_entrydump_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

    file = debugfs_create_file("om", S_IRUGO, root,
			spe, &spe_om_fops);
	if (!file) {
		ret = -ENOMEM;
		goto err1;
	}

	return 0;

err1:
	debugfs_remove_recursive(root);

err0:
	return ret;
}

void spe_debugfs_exit(struct spe *spe)
{
	debugfs_remove_recursive(spe->root);
	spe->root = NULL;
}
MODULE_LICENSE("GPL");

