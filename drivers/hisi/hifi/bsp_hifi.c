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

/*lint --e{537} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include <hi_hifi.h>
#include <hi_dsp.h>
#include <hi_onoff.h>
#include <ptable_com.h>

#include <bsp_om.h>
#include <bsp_dsp.h>
#include <bsp_ipc.h>
#include <bsp_icc.h>
#include <bsp_sec.h>
#include <bsp_sram.h>
#include <bsp_nandc.h>
#include <bsp_shared_ddr.h>
#include <drv_nv_id.h>
#include <drv_nv_def.h>
#include <bsp_nvim.h>
#include <bsp_sysctrl.h>
#include <drv_mailbox_cfg.h>
#include <product_config.h>


#define HIFI_IMAGE_NAME "HIFI"
#define HIFI_TEST_IMAGE_NAME "TEST"
static u32 is_hifi_stress = 0;
extern void drv_hifi_fill_mb_info(unsigned int addr);

typedef struct hifi_id_node
{
	unsigned int id;
	const char name[20];
}HIFI_ID_NODE_MAP;


int bsp_dsp_is_hifi_exist(void)
{
    int ret = 0;
    DRV_MODULE_SUPPORT_STRU   stSupportNv = {0};

    ret = (int)bsp_nvm_read(NV_ID_DRV_MODULE_SUPPORT, (u8*)&stSupportNv, sizeof(DRV_MODULE_SUPPORT_STRU));
    if (ret)
        ret = 0;
    else
        ret = (int)stSupportNv.hifi;

    return ret;
}

static void bsp_hifi_init_share_memory(struct drv_hifi_sec_load_info *section_info)
{
	u32 magic_info = HIFI_MEM_BEGIN_CHECK32_DATA; 

	if(is_hifi_stress)
	{
		magic_info = HIFI_STRESS_MAGIC;
	}

    writel(magic_info, (void*)section_info);
    writel(magic_info, (void*)((u32)section_info + SHM_SIZE_HIFI - sizeof(u32)));
}

static int bsp_hifi_check_sections(struct drv_hifi_image_head *img_head,
                                 struct drv_hifi_image_sec *img_sec)
{
    if ((img_sec->sn >= img_head->sections_num)
        || (img_sec->src_offset + img_sec->size > img_head->image_size)
        || (img_sec->type >= (unsigned char)DRV_HIFI_IMAGE_SEC_TYPE_BUTT)
        || (img_sec->load_attib >= (unsigned char)DRV_HIFI_IMAGE_SEC_LOAD_BUTT)) {
        return -1;
    }

    return 0;
}


int bsp_hifi_param_get(SOC_HIFI_ADDR_ITEM_STRU *p_addr)
{
	int ret = 0;
	int prop_len = 0;

	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int data = 0;
	unsigned int count = 0;

	struct device_node *dev_node = NULL;
	struct device_node *ref_node = NULL;

	const char name[] = "hisilicon,hifi_app";

	HIFI_ID_NODE_MAP node_map[] = {
   		//{HI_SYSCTRL_BASE_ADDR_ID, "sysctrl_node"},
   		{HI_SOCP_REGBASE_ADDR_ID, "socp_node"},
   		{SOC_IPC_S_BASE_ADDR_ID, "ipc_node"},
   		{SOC_AP_EDMAC_BASE_ADDR_ID, "edma_node"},
		{SOC_AO_SCTRL_SC_SLICER_COUNT0_ADDR_0_ID, "bbptimer_node"},
		{SOC_HIFI_Timer00_BASE_ADDR_ID, "timer0_node"},
		{SOC_HIFI_Timer08_BASE_ADDR_ID, "timer1_node"},
		{SOC_BBP_TDS_BASE_ADDR_ID, "bbp_node"},
	};

	dev_node = of_find_compatible_node(NULL, NULL, name);
	if(NULL == dev_node)
	{
		printk(KERN_ERR "hifi device node not found\n");
		return -1;
	}

	count = ARRAY_SIZE(node_map);

	for(i = 0; i <= count; i++)
	{
		if(NULL == of_find_property(dev_node, node_map[i].name, &prop_len))
		{
			printk(KERN_DEBUG "hifi prop %s is not find.\n", node_map[i].name);
			continue;
		}

		ref_node = of_parse_phandle(dev_node , node_map[i].name, 0);
		if(NULL == ref_node)
		{
			printk(KERN_ERR "hifi parse %s node error.\n", node_map[i].name);
			return -1;
		}

		ret = of_property_read_u32_index(ref_node, "reg", 0, &data);
		if (ret)
		{
			printk(" hifi read %s reg fail.\n", node_map[i].name);
			return ret;
		}

		p_addr[j].enID = node_map[i].id;
		p_addr[j].uwAddress = data;

		printk(KERN_INFO "hifi %s reg 0x%x.\n", node_map[i].name, p_addr[j].uwAddress);

		++j;

	}
	
	p_addr[j].enID =HI_SYSCTRL_BASE_ADDR_ID;
	p_addr[j].uwAddress = (unsigned int)bsp_sysctrl_addr_phy_byindex(sysctrl_pd);

	p_addr[j+1].enID =DDR_HIFI_ADDR_ID;
	p_addr[j+1].uwAddress = DDR_HIFI_ADDR;

	return 0;

}


void bsp_hifi_phy2phy(unsigned long *sec_addr)
{
    if (*sec_addr >= HIFI_TCM_START_ADDR && *sec_addr < HIFI_TCM_END_ADDR) {
        *sec_addr = *sec_addr - HIFI_TCM_START_ADDR + HIFI_TCM_BASE_ADDR;
    }
}


int bsp_hifi_load_sections(void *hifi_image)
{
    int ret = 0;
    u32 i = 0, dynamic_section_num = 0, dynamic_section_data_offset = 0;
	CARM_HIFI_DYN_ADDR_SHARE_STRU * p = NULL;

    void *section_virtual_addr = NULL;

    struct drv_hifi_sec_load_info *section_info = NULL;

    struct drv_hifi_image_head *hifi_head = (struct drv_hifi_image_head *)hifi_image;

    section_info = (struct drv_hifi_sec_load_info *)(SHM_BASE_ADDR + SHM_OFFSET_HIFI);

    bsp_hifi_init_share_memory(section_info);

    for (i = 0; i < hifi_head->sections_num; i++)
    {
        if (bsp_hifi_check_sections(hifi_head, &(hifi_head->sections[i])))
        {
            printk(KERN_ERR  "error hifi image section, index: %d\r\n", i);
            ret = -1;
            goto err_unmap;
        }

		bsp_hifi_phy2phy(&(hifi_head->sections[i].des_addr));

        if (DRV_HIFI_IMAGE_SEC_LOAD_STATIC == hifi_head->sections[i].load_attib)
        {
        	/* 静态加载 */
            if ((hifi_head->sections[i].des_addr < DDR_HIFI_ADDR) ||
                ((hifi_head->sections[i].des_addr + hifi_head->sections[i].size) > (DDR_HIFI_ADDR + DDR_HIFI_SIZE)))
            {
                printk(KERN_ERR
                    "error hifi address %lu \n", hifi_head->sections[i].des_addr);
                return -1;
            }
			/* 指向镜像存放的ddr   地址 */
            section_virtual_addr = (void*)ioremap_nocache(hifi_head->sections[i].des_addr,
                hifi_head->sections[i].size);
            if (NULL == section_virtual_addr)
            {
                printk(KERN_ERR  "fail to io remap at %d \n", __LINE__);
                return -ENOMEM;
            }
			/* 镜像拷贝(源->   目的) */
            memcpy(section_virtual_addr,
                   (void*)((char*)hifi_head + hifi_head->sections[i].src_offset),
                   hifi_head->sections[i].size);
            iounmap(section_virtual_addr);
        }
        else if (DRV_HIFI_IMAGE_SEC_LOAD_DYNAMIC == hifi_head->sections[i].load_attib)
        {
        	/* 动态加载 */
            if (dynamic_section_data_offset + hifi_head->sections[i].size > HIFI_SEC_DATA_LENGTH)
            {
                printk(KERN_ERR
                    "hifi dynamic section too large\r\n");
                return -1;
            }
            /* copy data to share addr */
			/* 将镜像拷贝到段信息结构体，供其他机制加载 */
            memcpy((void*)&(section_info->sec_data[dynamic_section_data_offset]),
                   (void*)((char*)hifi_head + hifi_head->sections[i].src_offset),
                   hifi_head->sections[i].size);

            /* update section info */
			/* 更新段地址 */
            section_info->sec_addr_info[dynamic_section_num].sec_source_addr
                = (unsigned int)SHD_DDR_V2P((void*)&(section_info->sec_data[dynamic_section_data_offset])); 
			/* 更新段长度 */
            section_info->sec_addr_info[dynamic_section_num].sec_length
                = hifi_head->sections[i].size;
			/* 更新段目的地址 ，DDR   地址*/
            section_info->sec_addr_info[dynamic_section_num].sec_dest_addr
                = hifi_head->sections[i].des_addr;

            dynamic_section_data_offset += hifi_head->sections[i].size;
            dynamic_section_num++;
        }
        else if (DRV_HIFI_IMAGE_SEC_UNLOAD == hifi_head->sections[i].load_attib)
        {
        	/* 无须底软加载 */
			printk(KERN_INFO "hifi param addr %lu.\n", hifi_head->sections[i].des_addr);
            section_virtual_addr = (void*)ioremap_nocache(hifi_head->sections[i].des_addr,
                hifi_head->sections[i].size);
            if (NULL == section_virtual_addr)
            {
                printk(KERN_ERR  "fail to io remap at %d \n", __LINE__);
                return -ENOMEM;
            }

			p = (CARM_HIFI_DYN_ADDR_SHARE_STRU * )section_virtual_addr;

			memset(p, 0, sizeof(CARM_HIFI_DYN_ADDR_SHARE_STRU));

			if(0 != bsp_hifi_param_get(p->stSoCShare.astSocAddr))
			{
				printk(KERN_ERR  "fail to get hifi param at %d \n", __LINE__);
				return -1;
			}
			p->stSoCShare.uwProtectWord = 0x5a5a5a5a;

			/* 将信息放入邮箱中 */
            drv_hifi_fill_mb_info((u32)section_virtual_addr);
            iounmap(section_virtual_addr);
        }
        else
        {
        	/* 加载方式有误 */
            printk(KERN_ERR
                "unknown section attribute %d\r\n", hifi_head->sections[i].load_attib);
            ret = -1;
            goto err_unmap;
        }
    }
    section_info->sec_num = dynamic_section_num;

    ret = bsp_ipc_int_send(IPC_CORE_MCORE, IPC_MCU_INT_SRC_HIFI_PU);
    if (ret)
    {
        printk(KERN_ERR
            "send hifi pu ipc error %d\r\n", ret);
        goto err_unmap;
    }

err_unmap:

    return ret;
}



int bsp_hifi_load_image(char* part_name)
{
    u32 offset = 0;
    u32 skip_len = 0;

	struct drv_hifi_image_head *hifi_head = NULL;
	void *hifi_image = NULL;

    /*coverity[var_decl] */
    struct image_head head;

	/* 读取nand中的hifi镜像头 */
    if (NAND_OK != bsp_nand_read(part_name,  0, (char*)&head, sizeof(struct image_head), &skip_len))
    {
        printk(KERN_ERR "read hifi image head fail.\n");
        return -1;
    }

    /*coverity[uninit_use_in_call] */
	/* 判断是否找到HIFI镜像 */
    if (memcmp(head.image_name, HIFI_IMAGE_NAME, strlen(HIFI_IMAGE_NAME)))
    {
        printk(KERN_ERR "hifi image not found.\n");
        return -1;
    }

	offset += sizeof(struct image_head) + skip_len;

	/* hifi镜像头内存申请 */
	hifi_head = (struct drv_hifi_image_head *)kmalloc(sizeof(struct drv_hifi_image_head), GFP_KERNEL);
	if (NULL == (void *)hifi_head)
	{
		printk(KERN_ERR  "fail to malloc hifi head space\n");
		return -1;
	}
	/* 从nand中读取hifi镜像头 */
	if (NAND_OK != bsp_nand_read(part_name, offset, (char*)hifi_head,
		sizeof(struct drv_hifi_image_head), &skip_len))
	{
		printk(KERN_ERR  "fail to load hifi image\n");
		kfree(hifi_head);
		return -1;
	}

	if (hifi_head->sections_num > HIFI_SEC_MAX_NUM)
	{
		printk(KERN_ERR  "hifi image head error\n");
		kfree(hifi_head);
		return -1;
	}

	if(!memcmp(hifi_head->time_stamp, HIFI_TEST_IMAGE_NAME, strlen(HIFI_TEST_IMAGE_NAME)))
	{
		is_hifi_stress = 1;
		printk(KERN_ERR  "hifi image is just for bus stress.\n");
	}

	hifi_image = vmalloc(hifi_head->image_size);
	if (NULL == (void *)hifi_image)
	{
		printk(KERN_ERR  "fail to malloc hifi image space, size: %lu\n", hifi_head->image_size);
		kfree(hifi_head);
		return -1;
	}
	/* 从nand中读取hifi镜像到hifi_image	 */
	if (NAND_OK != bsp_nand_read(part_name, offset, (char*)hifi_image, hifi_head->image_size, &skip_len))
	{
		printk(KERN_ERR  "fail to load hifi image\n");
		kfree(hifi_head);
		vfree(hifi_image);
		return -1;
	}

	/* 释放 */
	kfree(hifi_head);


	if (bsp_hifi_load_sections(hifi_image))
	{
		vfree(hifi_image);
		return -1;
	}

	vfree(hifi_image);
	return 0;

}


int __init bsp_hifi_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct ST_PART_TBL* dsp_part = NULL;

	if(0 == bsp_dsp_is_hifi_exist())
	{
		printk(KERN_INFO "load hifi is not enable.\n");
		return 0;
	}

	/* 通过模块名来查找HIFI模块的镜像 */
	dsp_part = find_partition_by_name(PTABLE_HIFI_NM);
	if(NULL == dsp_part)
	{
		printk(KERN_ERR "load hifi image succeed\r\n");
		return -1;
	}

	/* 通过镜像名来加载hifi镜像 */
	ret = bsp_hifi_load_image(dsp_part->name);
	if(ret < 0)
	{
		printk(KERN_ERR "load hifi image fail.\n");
		return ret;
	}

	printk(KERN_ERR "load hifi image ok.\n");

    return 0;
}

static struct platform_device bsp_hifi_device = {
    .name = "bsp_hifi",
    .id = 0,
    .dev = {
    .init_name = "bsp_hifi",
    },
};

static struct platform_driver bsp_hifi_drv = {
    .probe      = bsp_hifi_probe,
    .driver     = {
        .name     = "bsp_hifi",
        .owner    = THIS_MODULE,
    },
};

static int __init bsp_hifi_acore_init(void)
{
    int ret = 0;

    ret = platform_device_register(&bsp_hifi_device);
    if(ret)
    {
        printk("register his_modem device failed\r\n");
        return ret;
    }

    ret = platform_driver_register(&bsp_hifi_drv);
    if(ret)
    {
        printk("register his_modem driver failed\r\n");
        platform_device_unregister(&bsp_hifi_device);
    }

    return ret;
}

static void __exit bsp_hifi_acore_exit(void)
{
    platform_driver_unregister(&bsp_hifi_drv);
    platform_device_unregister(&bsp_hifi_device);
}

module_init(bsp_hifi_acore_init);
module_exit(bsp_hifi_acore_exit);

MODULE_AUTHOR("HUAWEI DRIVER GROUP");
MODULE_DESCRIPTION("Driver for huawei product");
MODULE_LICENSE("GPL");


