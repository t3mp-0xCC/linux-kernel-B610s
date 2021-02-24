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
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/pm.h>
#include "sdio_slave_cfg.h"
#include "sdio_slave.h"
#include "sdio_slave_osi.h"
#include "sdio_slave_hal.h"
#include "sdio_slave_errcode.h"

#include <linux/err.h>
#include <linux/delay.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>
#include <sdio_ops.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mutex.h>
#include <linux/dma-mapping.h>

#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/wait.h>
#include <linux/random.h>
//////////////////////////////////////////////

#define SDIO_VENDOR_ID_ARASAN			0x0296
#define SDIO_DEVICE_ID_ARASAN_COMBO		0x5347
#define RESULT_OK		0
#define RESULT_FAIL		1

#define BUFFER_ORDER		2
#define BUFFER_SIZE		(PAGE_SIZE << BUFFER_ORDER)

#define DBG(fmt, ...)                   \
do { \
	dev_dbg(&arasan_sdio_func->dev, "[%s] "fmt, __func__, ##__VA_ARGS__); \
} while(0)

#define ERR(fmt, ...)                   \
do { \
	dev_err(&arasan_sdio_func->dev, "[%s] "fmt, __func__, ##__VA_ARGS__); \
} while(0)
	
#define INFO(fmt, ...)                   \
do { \
	 dev_info(&arasan_sdio_func->dev, "[%s] "fmt, __func__, ##__VA_ARGS__); \
} while(0)

unsigned loop = 0;
static unsigned send_triger = 1;
wait_queue_head_t write_over_wait;
static unsigned long next = 1;
unsigned int dbg_count;

extern struct arasan_sido_log arasan_sdio_logs;

/* RAND_MAX assumed to be 32767 */
int myrand(void) {
   next = next * 1103515245 + 12345;
   return((unsigned)(next/65536) % 32768);
}

void mysrand(unsigned seed) {
   next = seed;
}

//////////////////////////////////////////////

struct arasan_sdio_test_func {
	struct sdio_func 	*func;

	u8		scratch[BUFFER_SIZE];
	u8		*buffer;
	struct arasan_sdio_test_general_result	*gr;
};

struct arasan_sdio_test_case {
	const char *name;

	int (*prepare)(struct arasan_sdio_test_func *);
	int (*run)(struct arasan_sdio_test_func *);
	int (*cleanup)(struct arasan_sdio_test_func *);
};

struct arasan_sdio_test_general_result {
	struct list_head link;
	struct arasan_sdio_test_func *func;
	int testcase;
	int result;
	struct list_head tr_lst;
};

/**
 * struct arasan_sdio_test_dbgfs_file - debugfs related file.
 * @link: double-linked list
 * @card: card under test
 * @file: file created under debugfs
 */
struct arasan_sdio_test_dbgfs_file {
	struct list_head link;
	struct arasan_sdio_test_func *func;
	struct dentry *file;
};

#define TEST_BUFFER_SIZE  	0x4000   // 32k
#define SDIO_MAX_PACKET_SIZE 0x40000
#define MAX_BLK_SIZE 	  	0x200
int g_delay = 50;

DEFINE_MUTEX(arasan_sdio_transfe_lock);

DEFINE_MUTEX(arasan_sdio_test_lock);


static LIST_HEAD(arasan_sdio_test_result);
/*SLAVE�ӿ�*/
extern BSP_S32 BSP_SLAVE_Init(BSP_VOID);
extern BSP_S32 BSP_SLAVE_Open(BSP_VOID);
extern BSP_S32 BSP_SLAVE_Close(BSP_S32 u32SlaveDevId);
extern BSP_S32 BSP_SLAVE_Ioctl(BSP_S32 handle, BSP_U32 cmd, BSP_U32 arg);
extern BSP_S32 BSP_SLAVE_Write(BSP_S32 handle, BSP_U8 *buf, BSP_U8 *bufPhy, BSP_U32 len);
extern BSP_S32 BSP_SLAVE_Read(BSP_S32 handle, BSP_U8 *buf, BSP_U32 len);
extern BSP_S32 BSP_SLAVE_Write(BSP_S32 handle, BSP_U8 *buf, BSP_U8 *bufPhy, BSP_U32 len);
extern BSP_U32 slave_recv_loop_count(void);

int blocksize = 0;

/*host���豸*/
struct sdio_func *arasan_sdio_func;
struct arasan_sdio_card *g_sdio_modem;

/*slave���豸*/
int g_slave_hd;
/* �ж����ݴ�����ź���*/
struct semaphore g_sem_irq;
struct semaphore g_sem_slave;

/* host�ϲ���õ����ݻ��� */
//unsigned char host_recv_buff[TEST_BUFFER_SIZE];
unsigned char *host_send_buff  = BSP_NULL;
unsigned char *host_recv_buff = BSP_NULL;

/* slave�ϲ���õ����ݻ��� */
unsigned char *slave_recv_buff = BSP_NULL;
unsigned char *slave_send_buff = BSP_NULL;
dma_addr_t slave_send_phy;

void host_send_data_prepare(unsigned char *data, unsigned int len);
int host_recv_data_check(unsigned char *sdata, unsigned char *rdata, unsigned int len);
void slave_send_data_prepare(unsigned char *data, unsigned int len);
void slave_recv_data_check(unsigned char *sdata, unsigned char *rdata, unsigned int len);




void arasan_sdio_to_host_irq_endisable(struct sdio_func *func, int enable)
{
	u8 reg;
	int ret;

	reg = enable ? 0xf : 0;
	
	sdio_claim_host(func);

	sdio_writeb(func, reg, 0x9, &ret);
	if (ret) {
		DBG("%s irq err\n", enable ? "enable" : "disable");
	}

	sdio_release_host(func);
	
}

unsigned int arasan_sdio_read_count(struct sdio_func *func, int *ret)
{
	u32  count;
	sdio_claim_host(func);

	count = sdio_readl(func, 0xc, &ret);

	sdio_release_host(func);

	return count;
	
}

 int host_get_blksize(struct sdio_func *func)
{
	int blksize,ret;
	sdio_claim_host(func);

	blksize = sdio_readl(func, 0x110, &ret);
	if(ret) {
		return ret;	
	}
	sdio_release_host(func);

	DBG("func%d blksize: %d\n", func->num, blksize);
	return 0;
}

void host_set_blksize(struct sdio_func *func, int blksize)
{
	sdio_claim_host(func);
	
	sdio_set_block_size(func, blksize);
	
	sdio_release_host(func);
	
	DBG("set func%d blksize: %d\n", func->num, blksize);
}

void host_send_data_prepare(unsigned char *data, unsigned int len)
{
    int i;
    for (i=0; i<len; i++)
    {
        data[i] = i;    
    }
}

int host_recv_data_check(unsigned char *sdata, unsigned char *rdata, unsigned int len)
{
	int i;
	int ret = 0;
	for (i=0; i<len; i++)
	{
		pr_info("send data:%d read data: %d\n", sdata[i], rdata[i]);
		if (sdata[i] != rdata[i] && !ret)
		{
			ret = -1;
			break;
		}
	}

	DBG("%d bytes transfered, check %s\n", len, ret ? "ERR": "OK");
	return 0;
}

void slave_send_data_prepare(unsigned char *data, unsigned int len)
{
    int i;
    for (i=0; i<len; i++)
    {
        data[i] = i;    
    }
}

void slave_recv_data_check(unsigned char *sdata,unsigned char *rdata, unsigned int len)
{
    int i;
    for (i=0; i<len; i++)
    {
        if (rdata[i] != sdata[i])
        {
            printk(KERN_ERR "slave recv data check error at pos[0x%02x] = 0x%x, 0x%x\n",i, rdata[i], sdata[i]);
            return;
        }
    }
    DBG("slave recv %d bytes check OK\n", len);
}

int arasan_sdio_data_check(unsigned char *sdata,unsigned char *rdata, unsigned int len)
{
	int i;
	int ret = 0;
	DBG("send addr:0x%0x  recv addr:0x%0x\n", sdata, rdata);
	for (i=0; i<len; i++)
	{
		//pr_info("send data:%d read data: %d\n", sdata[i], rdata[i]);
		if (sdata[i] != rdata[i] && !ret)
		{
			ret = -1;
			break;
		}
	}
	if (ret == -1) {
		int curr;
		curr = arasan_sdio_logs.curr;
		ERR("send data[%d]:%d read data[%d]: %d\n", i, sdata[i], i, rdata[i]);
		for(i = 0; i < curr; i++) {
			ERR("%s buffer:0x%0x len:0x%0x", 
				arasan_sdio_logs.desc[i].dir ? "HOST" : "DEV", 
				arasan_sdio_logs.desc[i].addr, arasan_sdio_logs.desc[i].len);
		}

	}
	arasan_sdio_logs.curr = 0;
	DBG("%d bytes transfered, check %s\n", len, ret ? "ERR": "OK");
	return ret;
}
void arasan_sdio_host_handler(struct sdio_func *func)
{
	unsigned char reg;
	int ret;
	unsigned int count;
	
	struct arasan_sdio_test_func * test = 
		container_of(func, struct arasan_sdio_test_func, func);
	
	u8 *buffer = test->buffer;
	
	arasan_sdio_to_host_irq_endisable(func, 0);
	
	sdio_claim_host(func);
	/*read f1 indent int*/
	reg = sdio_readb(func, 0x8, &ret);
	if (ret) {
		return ret;
	}
	/*clear f1 indent int*/
	sdio_writeb(func, reg, 0x8, &ret);
	if (ret) {
		return ret;
	}

	sdio_release_host(func);

	DBG("f1 indent reg: 0x%x\n", reg);

	/*data ready int*/
	if (reg & 0x01)
	{
		count = arasan_sdio_read_count(func, &ret);
		if (ret) {
			ERR("read count error\n");
		}
		INFO("sdcc req data (%d)bytes\n", count);
		
		/*why?*/
		count = count & 0x001fffff;

		
		INFO("sdcc req data (%d)bytes\n", count);

		sdio_claim_host(func);

		ret = sdio_readsb(func, buffer, 0x0, count);
		if (ret) {
			ERR("read data error\n");
			return ret;
		}
		sdio_release_host(func);

	}

	arasan_sdio_to_host_irq_endisable(func, 1);

	return;

}

void host_irq_handler()
{
	unsigned char reg;
	int ret;
	unsigned int count;
	// �ر�slave�豸����host���ж�,  ʹ��CMD52дFUN1��0x09�Ĵ���, ����Ϊ0x00
	arasan_sdio_to_host_irq_endisable(arasan_sdio_func, 0);

	sdio_claim_host(arasan_sdio_func);
	// ��ȡslave�豸�ϱ����ж�״̬, ʹ��CMD52��FUN1��0x08�Ĵ���, �õ�IntStatus
	reg = sdio_readb(arasan_sdio_func, 0x8, &ret);
	if (ret) {
		return ret;
	}
	// ����ж�״̬, ʹ��CMD52дFUN1��0x08�Ĵ�����������2�е�IntStatusԭ��д��
	sdio_writeb(arasan_sdio_func, reg, 0x8, &ret);
	if (ret) {
		return ret;
	}
	
	sdio_release_host(arasan_sdio_func);

	DBG("f1 indent reg: 0x%x\n", reg);
	
	//�жϲ���2�еõ���IntStatus, ������λΪ1, (IntStatus & 0x01),������slave��������:
	if (reg & 0x01)
	{
		count = arasan_sdio_read_count(arasan_sdio_func, &ret);
		if (ret) {
			ERR("read count error\n");
		}
		DBG("sdcc req data (%d)bytes\n", count);

		count = count & 0x001fffff;

		DBG("FUN1, host irq, data count = 0x%02x,(%02d)bytes\n ", count,(int)count);

		
		// ����CMD53��ȡFUN1����, regaddr=0x00, ����ֽ�ģʽ��, ����Ϊbyte count����,
		// ����ǿ�ģʽ���������byte count����block count������Ϊblock count

		sdio_claim_host(arasan_sdio_func);

		ret = sdio_readsb(arasan_sdio_func, host_recv_buff, 0x0, count);
		if (ret) {
			ERR("read data error\n");
			return ret;
		}
		sdio_release_host(arasan_sdio_func);
		
		//msleep(1000);
		
		//__dma_single_dev_to_cpu(host_recv_buff, count, 2);

	}

	/* ��slave�豸����host���ж�, ʹ��CMD52дFUN1��0x09��ַ�Ĵ���������Ϊ0x0f */

	arasan_sdio_to_host_irq_endisable(arasan_sdio_func, 1);

	return;
}

int initFlag = BSP_FALSE;

/*******************************************************************/
/*  Test preparation and cleanup                                   */
/*******************************************************************/

/*
 * Fill the first couple of sectors of the card with known data
 * so that bad reads/writes can be detected
 */
static int __arasan_sdio_test_prepare(struct arasan_sdio_test_func *test, int write)
{
	int ret, i;
	
	if (write) {
		memset(test->buffer, 0xDF, BUFFER_SIZE);
		ret = sdio_writesb(test->func, 0x0, test->buffer, BUFFER_SIZE);
		if (ret) {
			return ret;
		}
	}
	else {
		for (i = 0;i < BUFFER_SIZE;i++)
			test->buffer[i] = i;
		BSP_SLAVE_Write(g_slave_hd, test->buffer, slave_send_phy, BUFFER_SIZE);

		/*TODO: send int to host*/
	}
	return 0;
}

static int arasan_sdio_test_prepare_write(struct arasan_sdio_test_func *test)
{
	return __arasan_sdio_test_prepare(test, 1);
}

static int arasan_sdio_test_prepare_read(struct arasan_sdio_test_func *test)
{
	return __arasan_sdio_test_prepare(test, 0);
}

static int arasan_sdio_test_cleanup(struct arasan_sdio_test_func *test)
{

	memset(test->buffer, 0, BUFFER_SIZE);
	memset(test->scratch, 0, BUFFER_SIZE);
	return 0;
}


/*
 * Does a complete transfer test where data is also validated
 *
 * Note: arasan_sdio_test_prepare() must have been done before this call
 */
static int arasan_sdio_test_transfer(struct arasan_sdio_test_func *test,
				unsigned dev_addr,unsigned count, int write)
{
	int ret, i;
	unsigned long flags;

	if (write) {
		/*TODO: waitfor wirte over int*/
		ret = sdio_readsb(test->func, test->scratch, dev_addr, count);
		if (ret) {
			return ret;
		}
	} else {
		/*
		*TODO:	waitfor read over int
		*hand by sdio_irq
		**/

	}

	for (i = 0;i < count;i++) {
		if (test->scratch[i] != (u8)0xDF) {
			return RESULT_FAIL;
		} 
	}
	return 0;
}

static int arasan_sdio_test_verify_write(struct arasan_sdio_test_func *test)
{
	int ret;

	ret = arasan_sdio_test_transfer(test, 0x0, BUFFER_SIZE, 1);
	if (ret)
		return ret;

	return 0;
}

static int arasan_sdio_test_verify_read(struct arasan_sdio_test_func *test)
{
	int ret;

	ret = arasan_sdio_test_transfer(test, 0x0, BUFFER_SIZE, 0);
	if (ret)
		return ret;

	return 0;
}


static struct arasan_sdio_test_case arasan_sdio_test_cases[] = {
	{
		.name = "Basic write (with data verification)",
		.prepare = arasan_sdio_test_prepare_write,
		.run = arasan_sdio_test_verify_write,
		.cleanup = arasan_sdio_test_cleanup,
	},

	{
		.name = "Basic read (with data verification)",
		.prepare = arasan_sdio_test_prepare_read,
		.run = arasan_sdio_test_verify_read,
		.cleanup = arasan_sdio_test_cleanup,
	},
};

static int arasan_sdio_testlist_show(struct seq_file *sf, void *data)
{
	int i;

	mutex_lock(&arasan_sdio_test_lock);

	for (i = 0; i < ARRAY_SIZE(arasan_sdio_test_cases); i++)
		seq_printf(sf, "%d:\t%s\n", i+1, arasan_sdio_test_cases[i].name);

	mutex_unlock(&arasan_sdio_test_lock);

	return 0;
}

static void arasan_sdio_test_run(struct arasan_sdio_test_func *test, int testcase)
{
	int i, ret;
	struct mmc_card * card = test->func->card;
	
	pr_info("%s: Starting tests of card %s...\n",
		mmc_hostname(card->host), sdio_func_id(test->func));

	sdio_claim_host(test->func);

	for (i = 0;i < ARRAY_SIZE(arasan_sdio_test_cases);i++) {
		struct arasan_sdio_test_general_result *gr;

		if (testcase && ((i + 1) != testcase))
			continue;

		pr_info("%s: Test case %d. %s...\n",
			mmc_hostname(card->host), i + 1,
			arasan_sdio_test_cases[i].name);

		if (arasan_sdio_test_cases[i].prepare) {
			ret = arasan_sdio_test_cases[i].prepare(test);
			if (ret) {
				pr_info("%s: Result: Prepare "
					"stage failed! (%d)\n",
					mmc_hostname(card->host),
					ret);
				continue;
			}
		}

		gr = kzalloc(sizeof(struct arasan_sdio_test_general_result),
			GFP_KERNEL);
		if (gr) {
			INIT_LIST_HEAD(&gr->tr_lst);

			/* Assign data what we know already */
			gr->func= test;
			gr->testcase = i;

			/* Append container to global one */
			list_add_tail(&gr->link, &arasan_sdio_test_result);

			/*
			 * Save the pointer to created container in our private
			 * structure.
			 */
			test->gr = gr;
		}


		ret = arasan_sdio_test_cases[i].run(test);
		switch (ret) {
		case RESULT_OK:
			pr_info("%s: Result: OK\n",
				mmc_hostname(card->host));
			break;
		case RESULT_FAIL:
			pr_info("%s: Result: FAILED\n",
				mmc_hostname(card->host));
			break;
		default:
			pr_info("%s: Result: ERROR (%d)\n",
				mmc_hostname(card->host), ret);
		}

		/* Save the result */
		if (gr)
			gr->result = ret;

		if (arasan_sdio_test_cases[i].cleanup) {
			ret = arasan_sdio_test_cases[i].cleanup(test);
			if (ret) {
				pr_info("%s: Warning: Cleanup "
					"stage failed! (%d)\n",
					mmc_hostname(card->host),
					ret);
			}
		}
	}

	sdio_release_host(test->func);

	pr_info("%s: Tests completed.\n",
		mmc_hostname(card->host));
}

static int arasan_sdio_testlist_open(struct inode *inode, struct file *file)
{
	return single_open(file, arasan_sdio_testlist_show, inode->i_private);
}

static int arasan_sdio_test_show(struct seq_file *sf, void *data)
{
	struct arasan_sdio_func *func = (struct arasan_sdio_card *)sf->private;
	struct arasan_sdio_test_general_result *gr;

	mutex_lock(&arasan_sdio_test_lock);

	list_for_each_entry(gr, &arasan_sdio_test_result, link) {

		if (gr->func != func)
			continue;

		seq_printf(sf, "Test %d: %d\n", gr->testcase + 1, gr->result);

	}

	mutex_unlock(&arasan_sdio_test_lock);

	return 0;
}

static int arasan_sdio_test_open(struct inode *inode, struct file *file)
{
	return single_open(file, arasan_sdio_test_show, inode->i_private);
}

static ssize_t arasan_sdio_test_write(struct file *file, const char __user *buf,
	size_t count, loff_t *pos)
{
	struct seq_file *sf = (struct seq_file *)file->private_data;
	struct sdio_func *func = (struct sdio_func *)sf->private;
	struct arasan_sdio_test_func *test;
	char lbuf[12];
	long testcase;

	if (count >= sizeof(lbuf))
		return -EINVAL;

	if (copy_from_user(lbuf, buf, count))
		return -EFAULT;
	lbuf[count] = '\0';

	if (strict_strtol(lbuf, 10, &testcase))
		return -EINVAL;

	test = kzalloc(sizeof(struct arasan_sdio_test_func), GFP_KERNEL);
	if (!test)
		return -ENOMEM;


	test->func = func;

	test->buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);

	if (test->buffer) {
		mutex_lock(&arasan_sdio_test_lock);
		arasan_sdio_test_run(test, testcase);
		mutex_unlock(&arasan_sdio_test_lock);
	}
	
	kfree(test->buffer);
	kfree(test);

	return count;
}

static const struct file_operations arasan_sdio_fops_test = {
	.open		= arasan_sdio_test_open,
	.read		= seq_read,
	.write		= arasan_sdio_test_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};
static struct file_operations arasan_sdio_fops_testlist = {
	.open		= arasan_sdio_testlist_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
//int __devinit arasan_sdio_debugfs_init(struct sdio_func *func)
int  arasan_sdio_debugfs_init(struct sdio_func *func)

{
	struct dentry		*root;
	struct dentry		*file;
	int			ret;
	
	if (func->card->debugfs_root) {
		root = debugfs_create_dir(dev_name(&func->dev), 
			func->card->debugfs_root);
	}
	if (!root) {
		ret = -ENOMEM;
		goto err0;
	}
	file = debugfs_create_file("test", S_IRUGO | S_IWUSR, 
		root, func, &arasan_sdio_fops_test);
	if (IS_ERR_OR_NULL(file)) {
		dev_err(&func->dev,
			"Can't create 'test'. Perhaps debugfs is disabled.\n");
		ret =  -ENODEV;
		goto err1;
	}
	file = debugfs_create_file("testlist", S_IRUGO | S_IWUSR, 
		root, func, &arasan_sdio_fops_testlist);
	if (IS_ERR_OR_NULL(file)) {
		dev_err(&func->dev,
			"Can't create 'testlist'. Perhaps debugfs is disabled.\n");
		ret =  -ENODEV;
		goto err1;
	}

	return 0;

err1:
	debugfs_remove_recursive(root);

err0:
	return ret;

}
//void __devexit arasan_sdio_debugfs_exit(struct sdio_func *func)
void arasan_sdio_debugfs_exit(struct sdio_func *func)
{
#if 0
	debugfs_remove_recursive(dwc->root);
	dwc->root = NULL;
#endif
}

// �����ܳ�ʼ��
void arasan_sdio_test_init()
{
    //////////////////////////
    // 1. Host��
    //////////////////////////
	if(BSP_TRUE == initFlag)
		return ;

	initFlag = BSP_TRUE;

    // ���host����ջص�����
	
	g_sdio_modem = arasan_sdio_func->card;


    // ��ʼ���жϴ����ν��ź���
    sema_init(&(g_sem_irq),0);
    sema_init(&(g_sem_slave),0);
    // ���������ж����ݴ����߳�
    //kthread_run(host_irq_thread, NULL, "host_irq_thread");
	
    // ��ջ�����
    host_send_buff = kzalloc(TEST_BUFFER_SIZE,GFP_KERNEL);
	host_recv_buff = kzalloc(TEST_BUFFER_SIZE,GFP_KERNEL);
	if(!host_send_buff||!host_recv_buff)
		return;
    memset(host_send_buff, 0, TEST_BUFFER_SIZE);
    memset(host_recv_buff, 0, TEST_BUFFER_SIZE);
    printk("host_send_buff = 0x%08x\n", host_send_buff);
    printk("host_recv_buff = 0x%08x\n", host_recv_buff);
    
    //////////////////////////
    // 2. Slave��
    //////////////////////////

    // ���slave�豸
    g_slave_hd = BSP_SLAVE_Open();
    slave_recv_buff = kzalloc(TEST_BUFFER_SIZE,GFP_KERNEL);
	if(!slave_recv_buff)
		return;
	
    // ����slave���ݷ���buffer
    slave_send_buff = dma_alloc_coherent(NULL, TEST_BUFFER_SIZE, &slave_send_phy, GFP_DMA|__GFP_WAIT);
    if(!slave_send_buff)
    {
        printk("sdio_slave_test_02 alloc failed!\n");
        return;
    }
    
    // ��ջص�����
    BSP_SLAVE_Ioctl(g_slave_hd, SLAVE_IOCTL_SET_READ_CB, NULL);
    BSP_SLAVE_Ioctl(g_slave_hd, SLAVE_IOCTL_SET_WRITE_CB, NULL);
    
    // ��ջ�����
    memset(slave_send_buff, 0, TEST_BUFFER_SIZE);
    memset(slave_recv_buff, 0, TEST_BUFFER_SIZE);
    printk("slave_recv_buff = 0x%08x\n", slave_recv_buff);
    DBG("slave_send_buff  virtual(0x%08x), physic(0x%08x)\n", slave_send_buff, slave_send_phy);
}

int arasan_sdio_test_card_to_host(int count)
{
	int ret;
	DBG("enter\n");
	/* ��ջ����� */
	memset(host_recv_buff, 0, TEST_BUFFER_SIZE);
	memset(slave_recv_buff, 0, TEST_BUFFER_SIZE);


	// ׼������
	slave_send_data_prepare(slave_send_buff, count);
	
	/* slave���� */   
	BSP_SLAVE_Write(g_slave_hd, slave_send_buff, slave_send_phy, count);
	ret = arasan_sdio_data_check(slave_send_buff, host_recv_buff, count);
	if (ret) {
		ERR("check err\n");
	}

	DBG("out\n");
	return ret;
	
}

int arasan_sdio_test_host_to_card(int count)
{
	DBG("enter\n");

	/* ��ջ����� */
	memset(host_send_buff, 0, TEST_BUFFER_SIZE);
	memset(slave_recv_buff, 0, TEST_BUFFER_SIZE);

	/* ׼������ */
	host_send_data_prepare(host_send_buff, count);
	dbg_count = count;
	
	/* host���� */
	sdio_claim_host(arasan_sdio_func);

	sdio_writesb(arasan_sdio_func, 0x0, host_send_buff, count);
	
	sdio_release_host(arasan_sdio_func);

	/* slave���� */    
	BSP_SLAVE_Read(g_slave_hd, slave_recv_buff, count);
	
	/* ���������� */
	arasan_sdio_data_check(host_send_buff,slave_recv_buff, count);

	DBG("out\n");

	return 0;
}


int arasan_sdio_recv_only_thread()
{
	int ret = 0;
	unsigned int count, block = 0, byte = 0;
	unsigned long time = 0;
	DBG("host-to-dev write thread starting\n");	
	slave_send_data_prepare(host_send_buff, TEST_BUFFER_SIZE);
	do {
		count = prandom_u32() % ADMA_BUF_MAXSIZE ;
		printk(KERN_INFO "host send %d to slave\n", count);

		block = count / 512 * 512;
		byte = count % 512;
		
		
		if (!count) {
			continue;
		}

		if(block)
		{
			dbg_count = block;
			sdio_claim_host(arasan_sdio_func);
			ret = sdio_writesb(arasan_sdio_func, 0x0, host_send_buff, block);
			sdio_release_host(arasan_sdio_func);
			
			ret = BSP_SLAVE_Read(g_slave_hd, slave_recv_buff,block);
			if (ret != block) {
				ERR("slave read num %d from host not equal %d\n", ret, block);
				break;
			}
			ret = arasan_sdio_data_check(host_send_buff, slave_recv_buff, block);
			if (ret) {
				ERR("count(%d) check err\n", block);
				break;
			}
		}

		if(byte)
		{
			dbg_count = byte;
			sdio_claim_host(arasan_sdio_func);
			ret = sdio_writesb(arasan_sdio_func, 0x0, host_send_buff, byte);
			sdio_release_host(arasan_sdio_func);
			
			ret = BSP_SLAVE_Read(g_slave_hd, slave_recv_buff,byte);
			if (ret != byte) {
				ERR("slave read byte %d from host not equal %d\n", ret, byte);
				break;
			}
			ret = arasan_sdio_data_check(host_send_buff, slave_recv_buff, byte);
			if (ret) {
				ERR("byte count(%d) check err\n", byte);
				break;
			}
		}


	} while(!ret);
	
	DBG("host-to-dev write thread exiting\n");	
	return ret;
}


int arasan_sdio_send_only_thread()
{
	int ret = 0;
	unsigned int count;
	DBG("host-to-dev read only thread starting\n");
	
	slave_send_data_prepare(slave_send_buff, TEST_BUFFER_SIZE);

	do {
		count = prandom_u32() % ADMA_BUF_MAXSIZE ;
		if (!count) {
			continue;
		}
		DBG("slave send %d to host\n", count);
		
		/*device to host*/	
		ret = BSP_SLAVE_Write(g_slave_hd, slave_send_buff, slave_send_phy, count);
		if (ret != count) {
			ERR("slave send %d to host not equal %d\n", ret, count);
			break;
		}
		ret = arasan_sdio_data_check(slave_send_buff, host_recv_buff, count);
		if (ret) {
			ERR("count(%d) check err\n", count);
			break;
		}

	} while(1);
	DBG("host-to-dev read only thread exiting\n");	
	return ret;
}

int arasan_sdio_send_thread(void* data)
{
	unsigned int count , block = 0, byte = 0;
	int ret;
	unsigned int times;
	DBG("device-host-device send thread starting\n");	
	// ׼������
	slave_send_data_prepare(slave_send_buff, TEST_BUFFER_SIZE);
	times = 0;
	do {
		count = prandom_u32() % ADMA_BUF_MAXSIZE ;
		dbg_count = count;
		if (!count) {
			continue;
		}
		
		block = count / 512 * 512;
		byte = count % 512;
		
		DBG("slave send %d to host\n", count);
		
		/*device to host*/	
		ret = BSP_SLAVE_Write(g_slave_hd, slave_send_buff, slave_send_phy, count);
		if (ret != count) {
			ERR("slave send %d to host not equal %d\n", ret, count);
			break;
		}
		//msleep(10);
		/*host to device*/
		if(block)
		{
			dbg_count = block;
			sdio_claim_host(arasan_sdio_func);
			ret = sdio_writesb(arasan_sdio_func, 0x0, host_recv_buff, block);
			sdio_release_host(arasan_sdio_func);
			
			ret = BSP_SLAVE_Read(g_slave_hd, slave_recv_buff,block);
			if (ret != block) {
				ERR("slave read block num %d from host not equal %d\n", ret, block);
				break;
			}

		}

		if(byte)
		{
			dbg_count = byte;
			sdio_claim_host(arasan_sdio_func);
			ret = sdio_writesb(arasan_sdio_func, 0x0, host_recv_buff + block, byte);
			sdio_release_host(arasan_sdio_func);
			
			ret = BSP_SLAVE_Read(g_slave_hd, slave_recv_buff + block, byte);
			if (ret != byte) {
				ERR("slave read byte %d from host not equal %d\n", ret, byte);
				break;
			}

		}
		//msleep(10);
		ret = arasan_sdio_data_check(slave_send_buff, slave_recv_buff, count);
		if (ret) {
			ERR("count(%d) check err\n", count);
			break;
		}	
		dbg_count = count;
		times++;
	} while(1);
	
	ERR("device-host-device send thread exiting times(%d)\n", times);	
	
	return ret;
}


void arasan_sdio_test_stress()
{

	struct task_struct	*stress_send = NULL;
	struct task_struct	*stress_recv = NULL;

	DBG("enter\n");

	memset(host_recv_buff, 0, TEST_BUFFER_SIZE);
	memset(slave_recv_buff, 0, TEST_BUFFER_SIZE);
	
	stress_send = kthread_run(arasan_sdio_send_thread, arasan_sdio_func, 
		"arsan %s",sdio_func_id(arasan_sdio_func));

	if (IS_ERR(stress_send)) {
		int err = PTR_ERR(stress_send);
		return err;
	}
	DBG("out\n");
	return 0;
}


static const struct sdio_device_id arasan_sdio_ids[] = {
	{ SDIO_DEVICE(SDIO_VENDOR_ID_ARASAN,
			SDIO_DEVICE_ID_ARASAN_COMBO) },
	{ /* end: all zeroes */				},
};

MODULE_DEVICE_TABLE(sdio, arasan_sdio_ids);
static void arasan_sdio_remove(struct sdio_func *func)
{
	sdio_claim_host(func);
	sdio_release_irq(func);
	sdio_release_host(func);
	arasan_sdio_debugfs_exit(func);
}

static int arasan_sdio_probe(struct sdio_func *func,
		const struct sdio_device_id *id)
{
	int ret = 0;

	unsigned blksz = 512;
	
	arasan_sdio_func = func;
	
	DBG("class = 0x%X, vendor = 0x%X, "
			"device = 0x%X",
			func->class, func->vendor, 
			func->device);

	sdio_claim_host(func);
	ret = sdio_enable_func(func);
	if (ret) {
		return ret;
	}
	/*set f1 block size 512*/
	ret = sdio_set_block_size(func,512);
	if (ret) {
		DBG("set f1 block size(%d)failed\n", blksz);
		return ret;
	}
	/*enable sdcc to host int*/
	sdio_writeb(func, 0xf, ARASAN_SDIO_F1_INT_ENABLE, &ret);
	if (ret) {
		DBG("enable  f1 ident inter reg failed\n");
		return ret;
	}
	//ret = sdio_claim_irq(func, arasan_sdio_host_handler);
	ret = sdio_claim_irq(func, host_irq_handler);

	if (ret) {
		DBG("enable  f0 cccr INTN failed\n");
		return ret;
	}

	sdio_release_host(func);

	arasan_sdio_debugfs_init(func);
	
	return ret;
	
}

static int arasan_sdio_suspend(struct device *dev)
{
	printk(KERN_INFO "%s here.\n", __func__);
	return 0;
}

static int arasan_sdio_resume(struct device *dev)
{
	printk(KERN_INFO "%s here.\n", __func__);

	return 0;
}

SIMPLE_DEV_PM_OPS(arasan_sdio_pmops, arasan_sdio_suspend, arasan_sdio_resume);
static struct sdio_driver arasan_sdio_driver = {
	.name		= "arasan-sdio",
	.id_table	= arasan_sdio_ids,
	.probe		= arasan_sdio_probe,
	.remove		= arasan_sdio_remove,
	.drv		= {
		.pm		= &arasan_sdio_pmops,
	},
};


int  arasan_sdio_init(void)
{
	int ret = 0;
	ret = sdio_register_driver(&arasan_sdio_driver);
	if (ret) {
		printk("register err\n");
	}

	return ret;
}

static void __exit arasan_sdio_exit(void)
{
	sdio_unregister_driver(&arasan_sdio_driver);
}
module_init(arasan_sdio_init);
module_exit(arasan_sdio_exit);
