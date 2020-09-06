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
/* dump memory information */
#include "ap_dump_mem.h"
#include "osl_malloc.h"
#include "bsp_ap_dump.h"


ap_dump_manager_t* g_ap_dump_manager_info = NULL; /*ap mem manager info block*/
ap_dump_mem_ctrl_t g_ap_dump_mem_ctrl;  /*ap dump mem addr info*/




/*****************************************************************************
* 函 数 名  : bsp_ap_dump_mem_map
* 功能描述  : 用于在初始化阶段，映射AP使用的可维可测内存
*
* 输入参数  : 无
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_ap_dump_mem_map(void)
{
    g_ap_dump_mem_ctrl.ap_dump_phy_addr  = DUMP_AREA_KERNEL_ADDR;
	g_ap_dump_mem_ctrl.ap_dump_mem_size  = DUMP_AREA_KERNEL_SIZE;
    g_ap_dump_mem_ctrl.ap_dump_virt_addr = (void *)ioremap_wc(g_ap_dump_mem_ctrl.ap_dump_phy_addr, g_ap_dump_mem_ctrl.ap_dump_mem_size);

    if(g_ap_dump_mem_ctrl.ap_dump_virt_addr == NULL)
    {
        return BSP_ERROR;
    }

    memset((void *)g_ap_dump_mem_ctrl.ap_dump_virt_addr, 0, DUMP_AREA_KERNEL_SIZE);

    ap_dump_fetal("g_ap_dump_mem_ctrl.ap_dump_phy_addr = 0x%x,g_ap_dump_mem_ctrl.ap_dump_mem_size = 0x%x,g_ap_dump_mem_ctrl.ap_dump_virt_addr = 0x%p\n",g_ap_dump_mem_ctrl.ap_dump_phy_addr,g_ap_dump_mem_ctrl.ap_dump_mem_size,g_ap_dump_mem_ctrl.ap_dump_virt_addr);

    return BSP_OK;
}

void ap_dump_show_area_info(ap_dump_manager_t* ap_dump_info)
{
    if(ap_dump_info == NULL)
    {
        return;
    }

    ap_dump_fetal("cur_field_num = %d\n,ur_dynamic_base_addr = 0x%p,cur_dynamic_free_size = 0x%x,p_cur_area = 0x%p", ap_dump_info->cur_field_num,ap_dump_info->cur_dynamic_free_addr,ap_dump_info->cur_dynamic_free_size,ap_dump_info->p_cur_area);

}
/*****************************************************************************
* 函 数 名  : bsp_ap_dump_mem_init
* 功能描述  : 用于在初始化阶段，初始化AP使用的可维可测内存
*
* 输入参数  : 无
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_ap_dump_mem_init(void)
{
    s32 ret = 0;
    ap_dump_manager_t* ap_dump_info = 0;

    if(g_ap_dump_manager_info != NULL)
    {
        return BSP_OK;
    }

    ret = bsp_ap_dump_mem_map();
    if(ret != BSP_OK)
    {
        ap_dump_fetal("dump mem map fail\n");
        return BSP_ERROR;
    }

    ap_dump_info = (ap_dump_manager_t*)osl_malloc(sizeof(ap_dump_manager_t));
    if(ap_dump_info == NULL)
    {
        ap_dump_fetal("malloc ap mem manager info fail\n");
        return BSP_ERROR;
    }

    memset(ap_dump_info,0, sizeof(ap_dump_manager_t));
    ap_dump_info->cur_dynamic_free_addr  = (u8 *)(DUMP_AREA_KERNEL_DYNAMIC_ADDR - DUMP_AREA_KERNEL_ADDR + g_ap_dump_mem_ctrl.ap_dump_virt_addr) ;
    ap_dump_info->cur_dynamic_free_size  = DUMP_AREA_KERNEL_DYNAMIC_SIZE;
    ap_dump_info->p_cur_area             = (ap_dump_area_t*)g_ap_dump_mem_ctrl.ap_dump_virt_addr;
    ap_dump_info->cur_field_num = 0;
    ap_dump_info->cur_hook_list.hook_list.prev = &ap_dump_info->cur_hook_list.hook_list;
    ap_dump_info->cur_hook_list.hook_list.next = &ap_dump_info->cur_hook_list.hook_list;
    ap_dump_info->p_cur_area->area_head.magic_num = DUMP_AREA_MAGICNUM;
    memcpy(ap_dump_info->p_cur_area->area_head.name,"KERNEL",sizeof("KERNEL"));
    spin_lock_init(&ap_dump_info->lock);

    g_ap_dump_manager_info = ap_dump_info;

    return BSP_OK;
}

ap_dump_area_t* bsp_ap_dump_get_area_info(void)
{
    if(g_ap_dump_manager_info == NULL)
    {
        (void)bsp_ap_dump_mem_init();
    }

    return g_ap_dump_manager_info->p_cur_area;
}
/*****************************************************************************
* 函 数 名  : bsp_ap_dump_register_hook
* 功能描述  : 注册异常时的回调函数
*
* 输入参数  : char * name  模块名称
              dump_hook func 模块的回调函数
* 输出参数  :

* 返 回 值  : NULL: 注册失败
              pfieldhook : 注册成功，

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
dump_handle bsp_ap_dump_register_hook(char * name, dump_hook func)
{
    ap_dump_field_hook_t* pfieldhook = NULL;
    unsigned long flags = 0;

    if(g_ap_dump_manager_info == NULL)
    {
        (void)bsp_ap_dump_mem_init();
    }

    pfieldhook = (ap_dump_field_hook_t*)osl_malloc(sizeof(ap_dump_field_hook_t));
    if(pfieldhook == NULL)
    {
        return BSP_ERROR;
    }

    pfieldhook->pfunc = func;
    memset(pfieldhook->name,'\0',sizeof(pfieldhook->name));
    memcpy(pfieldhook->name,name,15);
    spin_lock_irqsave(&g_ap_dump_manager_info->lock, flags);
    list_add(&pfieldhook->hook_list, &g_ap_dump_manager_info->cur_hook_list.hook_list);
    spin_unlock_irqrestore(&g_ap_dump_manager_info->lock, flags);

    return (dump_handle)((unsigned long)pfieldhook);
}

/*****************************************************************************
* 函 数 名  : bsp_ap_dump_unregister_hook
* 功能描述  : 取消异常是的回调函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_ap_dump_unregister_hook(dump_handle handle)
{
    ap_dump_field_hook_t * pfieldhook= NULL;
    ap_dump_field_hook_t * hook_node = NULL;
    unsigned long flags = 0;

    if(handle == 0)
    {
        return BSP_ERROR;
    }

    spin_lock_irqsave(&g_ap_dump_manager_info->lock, flags);

    list_for_each_entry(pfieldhook, &g_ap_dump_manager_info->cur_hook_list.hook_list, hook_list)
    {
        if((dump_handle)((unsigned long)pfieldhook) == handle)
        {
            hook_node = pfieldhook;
        }
    }

    if(hook_node == NULL)
    {
        spin_unlock_irqrestore(&g_ap_dump_manager_info->lock, flags);
        return BSP_ERROR;
    }

    list_del(&hook_node->hook_list);
    osl_free(hook_node);
    spin_unlock_irqrestore(&g_ap_dump_manager_info->lock, flags);

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : ap_dump_verify_field_id
* 功能描述  : 验证当前field id是否属于ap
*
* 输入参数  :
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
static s32 ap_dump_verify_field_id(u32 field_id)
{
    s32 ret = BSP_ERROR ;

    if(((field_id>=DUMP_KERNEL_FIELD_BEGIN) && (field_id<DUMP_KERNEL_FIELD_END)) )
    {
        ret = BSP_OK;
    }
    else
    {
        ap_dump_fetal("%s:invalid field id 0x%x\n", __FUNCTION__, (u32)field_id);
        ret = BSP_ERROR;
    }

    return ret;
}

void ap_dump_mem_show_field_info(ap_dump_field_map_t* field)
{
    if(field == NULL)
    {
        return;
    }

    ap_dump_fetal("field_id = 0x%x\n", field->field_id);
    ap_dump_fetal("offset_addr = 0x%x\n", field->offset_addr);
    ap_dump_fetal("length = 0x%x\n", field->length);
    ap_dump_fetal("status = 0x%x\n", field->status);
    ap_dump_fetal("version = 0x%x\n", field->version);
    ap_dump_fetal("field_name = %s\n", field->field_name);

}

 /*****************************************************************************
* 函 数 名  : bsp_ap_dump_register_field
* 功能描述  : 向AP申请可维可测空间
*             1. 不带地址注册，传入参数时virt_addr,phy_addr必须传0，成功返回dump注册地址
*             2. 自带地址注册，传入参数时phy_addr为自带物理地址，virt_addr为虚拟地址，同时在dump内存中分配相同大小内存，成功返回邋virt_addr
*             PS:
*             1. 两种注册方式，都将在dump划分内存，对于自带地址的注册方式，在系统异常时，由dump模块做数据拷贝
*             2. 每个注册区域需要由使用者传入对应的版本号，高8位为主版本号，低8位为次版本号
*
* 输入参数  :
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_ap_dump_register_field(u32 field_id, char * name, void * virt_addr, void * phy_addr, u32 length, u16 version)
{
    u32 i = 0;
    u8 * p_addr = NULL;
    unsigned long flags = 0;
    ap_dump_area_t* p_area = NULL;
    unsigned long * f_data = NULL;
    ap_dump_field_map_t* pfield = NULL;

    if(g_ap_dump_manager_info == NULL)
    {
        (void)bsp_ap_dump_mem_init();
    }

    if((length == 0) || ((virt_addr != NULL || phy_addr != NULL) && (length < 3*sizeof(unsigned long))))
    {

        ap_dump_fetal("%s: invalid len 0x%x or addr %p %p\n", __FUNCTION__, length, virt_addr, phy_addr);

        return NULL;
    }

    if(BSP_ERROR == ap_dump_verify_field_id(field_id))
    {

        ap_dump_fetal("%s: invalid field id 0x%x, register fail\n", __FUNCTION__, field_id);
        return NULL;
    }

    p_area = (ap_dump_area_t*)(g_ap_dump_manager_info->p_cur_area);

    spin_lock_irqsave(&g_ap_dump_manager_info->lock, flags);

    /* can't regitster if this field has been regitstered, return this field address only */
    for(i = 0; i < g_ap_dump_manager_info->cur_field_num; i++)
    {
        if(p_area->fields[i].field_id == field_id)
        {
            if(p_area->fields[i].length != length)
            {

                 ap_dump_fetal("%s: bsp_dump_register_field, field was registered, len is not equal, old: 0x%x, new: 0x%x\n",
                    __FUNCTION__,  p_area->fields[i].length, length);
            }

            spin_unlock_irqrestore(&g_ap_dump_manager_info->lock, flags);
            return (u8*)((u8 *)p_area + p_area->fields[i].offset_addr);
        }
    }

    /* judge if this field can be registered */
    if(((g_ap_dump_manager_info->cur_field_num == DUMP_FIELD_MAX_NUM)
        || (g_ap_dump_manager_info->cur_dynamic_free_size) <= length ))
    {
        spin_unlock_irqrestore(&g_ap_dump_manager_info->lock, flags);

        ap_dump_fetal("%s:no more space(mod_id = 0x%x,left:0x%x,alloc len:0x%x)\n", __FUNCTION__, field_id,g_ap_dump_manager_info->cur_dynamic_free_size, length);

        return NULL;
    }

    p_addr = g_ap_dump_manager_info->cur_dynamic_free_addr;
    pfield = &(p_area->fields[g_ap_dump_manager_info->cur_field_num]);
    pfield->field_id    = field_id;
    pfield->offset_addr = (unsigned long)(g_ap_dump_manager_info->cur_dynamic_free_addr - (u8 *)p_area);
    pfield->length      = length;
    pfield->status      = DUMP_FIELD_USED;
    pfield->version     = version;

    memset(pfield->field_name,'\0',16);
    memcpy(pfield->field_name,name,15);

    /* save user address in register field */
    f_data    = (unsigned long *)((u8 *)p_area + pfield->offset_addr);
    *f_data++ = (virt_addr != NULL || phy_addr != NULL) ? DUMP_FIELD_MAGIC_NUM : 0;
    *f_data++ = (unsigned long)virt_addr;
    *f_data++ = (unsigned long)phy_addr;

    /* update dynamic area info in this cpu area */
    g_ap_dump_manager_info->cur_field_num ++;
    g_ap_dump_manager_info->cur_dynamic_free_addr = g_ap_dump_manager_info->cur_dynamic_free_addr + length;
    g_ap_dump_manager_info->cur_dynamic_free_size = g_ap_dump_manager_info->cur_dynamic_free_size - length;

    /* update area head information */
    p_area->area_head.field_num = g_ap_dump_manager_info->cur_field_num ;

    spin_unlock_irqrestore(&g_ap_dump_manager_info->lock, flags);

    if(virt_addr != NULL || phy_addr != NULL)
    {
        return (u8 *)virt_addr;
    }

    return (u8 *)p_addr;
}

 /*****************************************************************************
* 函 数 名  : bsp_ap_dump_get_field_addr
* 功能描述  : 通过field id查找ap的filed地址
*
* 输入参数  :
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_ap_dump_get_field_addr(u32 field_id)
{
    u32 i = 0;
    u8 * addr = NULL;
    ap_dump_area_t * parea = (ap_dump_area_t*)g_ap_dump_manager_info->p_cur_area;

    if(NULL == parea)
    {
        ap_dump_fetal("get ap area info fail\n");
        return NULL;
    }

    /* search field addr by field id */
    for(i=0; i<parea->area_head.field_num; i++)
    {
        if(field_id == parea->fields[i].field_id)
        {
            addr = (u8 *)parea + parea->fields[i].offset_addr;
            break;
        }
    }

    return addr;
}

 /*****************************************************************************
* 函 数 名  : bsp_ap_dump_get_field_map
* 功能描述  : 通过field id查找ap的filed结构体信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_ap_dump_get_field_map(u32 field_id)
{
    u32 i = 0;
    ap_dump_area_t *parea = (ap_dump_area_t*)g_ap_dump_manager_info->p_cur_area;

    if(NULL == parea)
    {
        ap_dump_fetal("get ap area info fail\n");
        return NULL;
    }

    /* search field map by field id */
    for(i=0; i<parea->area_head.field_num; i++)
    {
        if(field_id == parea->fields[i].field_id)
        {
            return (u8 * )&parea->fields[i];
        }
    }

    return NULL;
}
 /*****************************************************************************
* 函 数 名  : bsp_ap_dump_get_field_map
* 功能描述  : 通过field id查找ap的filed结构体信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u8 * bsp_ap_dump_get_field_phy_addr(u32 field_id)
{
    u8 * addr = 0;

    addr = bsp_ap_dump_get_field_addr(field_id);
    if(NULL == addr)
    {
        ap_dump_fetal("get filed info fail\n");
        return NULL;
    }

    return (u8 *)((unsigned long)addr - (unsigned long)g_ap_dump_mem_ctrl.ap_dump_virt_addr + g_ap_dump_mem_ctrl.ap_dump_phy_addr);
}

/*****************************************************************************
* 函 数 名  : ap_dump_save_ext
* 功能描述  : 保存扩展区的log
*
* 输入参数  :
*
*
*
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_save_all_fields(void)
{
    struct list_head* cur = NULL;
    ap_dump_field_hook_t* pfieldhook = NULL;
    unsigned long flags = 0;
    ap_dump_area_head_t*  p_head  = (ap_dump_area_head_t*)g_ap_dump_manager_info->p_cur_area;
    ap_dump_field_map_t*  p_field = (ap_dump_field_map_t*)g_ap_dump_manager_info->p_cur_area->fields;
    u32 i = 0;
    unsigned long * f_addr = NULL;

    list_for_each(cur, &g_ap_dump_manager_info->cur_hook_list.hook_list)
    {
        spin_lock_irqsave(&g_ap_dump_manager_info->lock, flags);

        pfieldhook = list_entry(cur, ap_dump_field_hook_t, hook_list);

        spin_unlock_irqrestore(&g_ap_dump_manager_info->lock, flags);

        if(pfieldhook->pfunc)
        {
            pfieldhook->pfunc();
        }
    }

    for(i=0; i<p_head->field_num; i++, p_field++)
    {
        f_addr = (unsigned long *)((u8 *)p_head + p_field->offset_addr);
        if(DUMP_FIELD_MAGIC_NUM == *f_addr)
        {
            memcpy((u8 *)f_addr, (u8 *)(*(f_addr + 1)), p_field->length);
        }
    }

    ap_dump_fetal("dump save ext finish\n");

    return;
}

 /*****************************************************************************
* 函 数 名  : bsp_ap_dump_show_area
* 功能描述  : 显示a核所有filed信息
*
* 输入参数  :
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_ap_dump_show_area(void)
{
    u32 i = 0;
    u8*  pcontent = NULL;
    ap_dump_area_t* parea = NULL;


    if(g_ap_dump_manager_info == 0)
    {
        (void)ap_dump_fetal("dump memory unintialized!\n");
        return (s32)(-1);
    }

    parea = (ap_dump_area_t*)g_ap_dump_manager_info->p_cur_area;

    (void)ap_dump_fetal("=============================AP DUMP AREA INFO================================\n");
    (void)ap_dump_fetal("==Dump area head info:========================================================\n");
    (void)ap_dump_fetal("magic number: 0x%X\n", parea->area_head.magic_num);
    (void)ap_dump_fetal("field number: 0x%X\n", parea->area_head.field_num);
    (void)ap_dump_fetal("name        : %s\n",   parea->area_head.name);
    (void)ap_dump_fetal("version     : %s\n",   parea->area_head.version);
    (void)ap_dump_fetal("==Dump area fields info:======================================================\n");
    for(i=0; i<parea->area_head.field_num; i++)
    {
        pcontent = (u8*)parea->data + parea->fields[i].offset_addr;

        (void)ap_dump_fetal("Index:%3d, field id: 0x%8X, offset addr: 0x%8X, length: 0x%8X, status: 0x%X, ",
            i, parea->fields[i].field_id, parea->fields[i].offset_addr, parea->fields[i].length, parea->fields[i].status);
        (void)ap_dump_fetal("content: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ...\n", pcontent[0], pcontent[1],
            pcontent[2], pcontent[3], pcontent[4], pcontent[5], pcontent[6], pcontent[7],pcontent[8], pcontent[9],
            pcontent[10], pcontent[11], pcontent[12], pcontent[13], pcontent[14], pcontent[15]);
    }

    return 0;
}

 /*****************************************************************************
* 函 数 名  : bsp_ap_dump_show_exchook
* 功能描述  : 显示a核所有模块注册的异常回调函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  : OK : 0
              ERROR : -1

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 bsp_ap_dump_show_exchook(void)
{
    struct list_head* cur = NULL;
    ap_dump_field_hook_t* pfieldhook = NULL;

    (void)ap_dump_fetal("===Area Hook Function List===\n");
    list_for_each(cur, &g_ap_dump_manager_info->cur_hook_list.hook_list)
    {
        pfieldhook = list_entry(cur, ap_dump_field_hook_t, hook_list);

        if(pfieldhook->pfunc)
        {
            (void)ap_dump_fetal("name=%s,Function Addr=%pS\n", pfieldhook->name, pfieldhook->pfunc);
        }
    }

    return 0;
}

/*****************************************************************************
* 函 数 名  : ap_dump_show_filed
* 功能描述  : 调试接口，用于显示当前ap使用的filed情况
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void ap_dump_show_field(void)
{
    ap_dump_area_head_t * head;
    ap_dump_field_map_t * field;
    u32 i;
    u32 used_len = 0;
    u32 field_cnt;

    head = &(g_ap_dump_manager_info->p_cur_area->area_head);
    field_cnt = head->field_num;

    ap_dump_fetal("-----------------------------------------\n");
    ap_dump_fetal("AP AREA INFO(BASE ADDR %p):\n", head);

    for(i=0; i<field_cnt; i++)
    {
        field = &(g_ap_dump_manager_info->p_cur_area->fields[i]);
        ap_dump_fetal("FILED %-16s: id=0x%-8x offset=0x%-8x size=0x%x\n", field->field_name, field->field_id, field->offset_addr, field->length);
        used_len += field->length;
    }

    ap_dump_fetal("USED: 0x%x, LEFT: 0x%x\n", used_len, DUMP_AREA_KERNEL_SIZE - DUMP_AREA_KERNEL_HEAD_SIZE - DUMP_AREA_KERNEL_MAP_SIZE - used_len);
}

arch_initcall_sync(bsp_ap_dump_mem_init);


