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


#include <linux/bitops.h>
#include "spe.h"
#include "spe_dbg.h"
#include "spe_desc.h"
#include "hi_spe_mpi.h"
#include <linux/io.h>
#include "product_config.h"

#define SPE_GET_CUR_TD(base, pos) \
    ((struct spe_td_desc *)((base) + (pos) * (sizeof(struct spe_td_desc))))

#define SPE_GET_CUR_RD(base, pos) \
    ((struct spe_rd_desc *)((base) + (pos) * (sizeof(struct spe_rd_desc))))

#define PORTNO_INVALID(portno)	(portno>7)
/* we use fls to order the spe time diff, may use 19.2M later */
#define SPE_FW_TIME_RECORD_MAX_NUM      (32)
#define SPE_TIME_FLS_MASK               (SPE_FW_TIME_RECORD_MAX_NUM-1)

/* convert the timer count to us, may use 19.2M later */
#define SPE_DBG_COUNT_TO_US(cnt)         SPE_32KHZ_TO_US(cnt)

/* used for check whether timer is overflow, may use 19.2M later */
#define SPE_MAX_TIME_LIMIT              SPE_US_TO_32KHZ(1000*1000)

typedef struct tag_spe_timestamp_info
{
    unsigned int diff_order_cnt[SPE_FW_TIME_RECORD_MAX_NUM];
    unsigned int cnt_sum;
    unsigned int diff_sum;
    unsigned int diff_max;
    unsigned int overflow;
}spe_timestamp_info_s;

static spe_timestamp_info_s spe_dbg_fw_time;


//u32 spe_dbg_fw_time[SPE_FW_TIME_RECORD_MAX_NUM];
//u32 spe_dbg_fw_max_time = 0;
//static int dbg_record_cur = 0;

void spe_set_rd_timestamp(void * rd_desc)
{
    u32 fw_time;
    int idx;
	struct spe_rd_desc *desc = (struct spe_rd_desc *)rd_desc;
	spe_timestamp_info_s *stamp_info = &spe_dbg_fw_time;

    fw_time = desc->usr_field2 - desc->usr_field1;
	/* avoid to record the overflowed value */
    if (fw_time > SPE_MAX_TIME_LIMIT) {
        stamp_info->overflow++;
    }
    else {
        if (fw_time > stamp_info->diff_max)
            stamp_info->diff_max = fw_time;

        stamp_info->diff_sum += fw_time;
        stamp_info->cnt_sum++;

        /* find the first bit not zero */
        idx = ((fls(fw_time)-1) & SPE_TIME_FLS_MASK);
        stamp_info->diff_order_cnt[idx]++;
    }
	return;
}

static inline
unsigned int spe_calc_percent(unsigned int value, unsigned int sum)
{
    if (0 == sum) {
        return 0;
    }
    return (value * 100 / sum);
}

void spe_fw_time_show(void)
{
    spe_timestamp_info_s* stamp_info = &spe_dbg_fw_time;
    unsigned int tmp = 0;
    int i;

    printk(" max diff:%u(%uus)\n",
           stamp_info->diff_max, SPE_DBG_COUNT_TO_US(stamp_info->diff_max));
    printk(" sum diff:%u(%uus)\n",
           stamp_info->diff_sum, SPE_DBG_COUNT_TO_US(stamp_info->diff_sum));
    printk(" overflow count:%d\n", stamp_info->overflow);

    if (stamp_info->cnt_sum) {
        tmp = stamp_info->diff_sum / stamp_info->cnt_sum;
    }

    printk(" avg diff:%u(%uus)\n", tmp, SPE_DBG_COUNT_TO_US(tmp));

    for (i = 0; i < SPE_FW_TIME_RECORD_MAX_NUM; i++) {
        tmp = spe_calc_percent(stamp_info->diff_order_cnt[i], stamp_info->cnt_sum);
        printk(" diff time (%u~%u) (%uus~%uus) count:%u (%u %%)\n",
            (0x80000000 >> (31-i)),
            (0xFFFFFFFF >> (31-i)),
            SPE_DBG_COUNT_TO_US(0x80000000 >> (31-i)),
            SPE_DBG_COUNT_TO_US(0xFFFFFFFF >> (31-i)),
            stamp_info->diff_order_cnt[i], tmp);
    }
    return;
}

void spe_help(void)
{
	printk("spe_print_td(port_num td_pos) :print info of one td \n ");
	printk("spe_dump_td(port_num) :print basic info of all td \n ");

	printk("spe_print_rd(port_num rd_pos) :print info of one rd\n ");
	printk("spe_dump_rd(port_num) :print basic info of all rd\n ");

	printk("spe_dev_setting \n ");
	printk("spe_dev_info \n ");
	printk("spe_port_setting(port_num) \n ");
	printk("spe_port_rd_info(port_num) \n ");
	printk("spe_port_td_info(port_num) \n ");
	printk("spe_ipfw_setting \n ");
	printk("spe_port_info_all(port_num) \n ");

}

void spe_dump_td(unsigned int port_num)
{
	//unsigned int desc_usr_field = spe_balong.desc_usr_field;
	struct spe_port_ctrl *ctrl = &spe_balong.ports[port_num].ctrl;
	//struct spe_port_stat *stat = &spe_balong.ports[port_num].stat;
	struct spe_td_desc *cur_td;
	int i = 0;
	int td_num;

	if(PORTNO_INVALID(port_num)){
		printk("port num is invalid!\n");
		return;
	}
	td_num = ctrl->td_depth;

	for(i=0; i<td_num; i++){
		cur_td = (struct spe_td_desc *)SPE_GET_CUR_TD(ctrl->td_addr,i);
		printk("buf_addr 0x%x, skb_addr 0x%x, pkt_len %d \n",
			cur_td->trb.buf_addr, cur_td->trb.skb_addr, cur_td->trb.pkt_len);
	}
}

void spe_dump_rd(unsigned int port_num)
{
	//unsigned int desc_usr_field = spe_balong.desc_usr_field;
	struct spe_port_ctrl *ctrl = &spe_balong.ports[port_num].ctrl;
	//struct spe_port_stat *stat = &spe_balong.ports[port_num].stat;
	struct spe_rd_desc *cur_rd;
	int i = 0;
	int rd_num;
	if(PORTNO_INVALID(port_num)){
		printk("port num is invalid!\n");
		return;
	}

	rd_num = ctrl->rd_depth;

	for(i=0; i<rd_num; i++){
		cur_rd = (struct spe_rd_desc *)SPE_GET_CUR_RD(ctrl->rd_addr,i);
		printk("buf_addr 0x%x, skb_addr 0x%x, pkt_len %d \n",
			cur_rd->trb.buf_addr, cur_rd->trb.skb_addr, cur_rd->trb.pkt_len);
	}
}


void spe_print_td(unsigned int port_num, unsigned int td_pos)
{
	struct spe_port_ctrl *ctrl = &spe_balong.ports[port_num].ctrl;
	struct spe_td_desc *cur_td = (struct spe_td_desc *)SPE_GET_CUR_TD(ctrl->td_addr,td_pos);
	struct sk_buff *skb = (struct sk_buff *)cur_td->trb.skb_addr;
	unsigned int len;
	if(PORTNO_INVALID(port_num)){
		printk("port num is invalid!\n");
		return;
	}

	printk("buf_addr 0x%x, skb 0x%x, buf_len %u \n usr1 0x%x usr2 0x%x \n", cur_td->trb.buf_addr,
		cur_td->trb.skb_addr, cur_td->trb.pkt_len, cur_td->usr_field1, cur_td->usr_field2);
	printk("eth_type %u, push_en %u, irq_en %u, timestamp_en %u,\n result 0x%x \n",cur_td->trb.eth_type,
		cur_td->trb.push_en, cur_td->trb.irq_en, cur_td->trb.timestamp_en, cur_td->trb.result);
	if(NULL != skb){
		printk("Skb:\n data 0x%x, len %u,spe_own %u, head 0x%x, tail 0x%x \n",
			(u32)skb->data, skb->data_len, skb->spe_own, (u32)skb->head, (u32)skb->tail);

		if(virt_to_phys(skb->data) != cur_td->trb.buf_addr){
			printk("ERROR! Skb data and td buf_addr are not the same! \n");
		}
	}else{
		printk("Skb invaild !\n");
		return;
	}

	if(skb->len < 64){
		len = skb->len;
	}else{
		len = 64;
	}

	if(NULL != skb->data){
		print_hex_dump(KERN_ERR, "skb data:", DUMP_PREFIX_ADDRESS,
			16, 1, skb->data, len, true);
	}
}

void spe_print_rd(unsigned int port_num, unsigned int rd_pos)
{
	//unsigned int desc_usr_field = spe_balong.desc_usr_field;
	struct spe_port_ctrl *ctrl = &spe_balong.ports[port_num].ctrl;
	//struct spe_port_stat *stat = &spe_balong.ports[port_num].stat;
	struct spe_rd_desc *cur_rd = (struct spe_rd_desc *)SPE_GET_CUR_RD(ctrl->rd_addr,rd_pos);
	struct sk_buff *skb = (struct sk_buff *)cur_rd->trb.skb_addr;
	unsigned int len;

	if(PORTNO_INVALID(port_num)){
		printk("port num is invalid!\n");
		return;
	}
	printk("buf_addr 0x%x, skb 0x%x, buf_len %u \n usr1 0x%x usr2 0x%x \n", cur_rd->trb.buf_addr,
		cur_rd->trb.skb_addr, cur_rd->trb.pkt_len, cur_rd->usr_field1, cur_rd->usr_field2);
	printk("updata_only %u, irq_en %u,\n sport %u, dport %u, \n result %u \n",
		cur_rd->trb.updata_only, cur_rd->trb.irq_en, cur_rd->trb.sport_num,
		cur_rd->trb.dport_num, cur_rd->trb.result);
	if(NULL != skb){
		printk("Skb:\n data 0x%x, len %u,spe_own %u, head 0x%x, tail 0x%x \n",
			(u32)skb->data, skb->data_len, skb->spe_own, (u32)skb->head, (u32)skb->tail);

		if(virt_to_phys(skb->data) != cur_rd->trb.buf_addr){
			printk("ERROR! Skb data and td buf_addr are not the same! \n");
		}
	}else{
		printk("Skb invaild !\n");
		return;
	}

	if(skb->len < 64){
		len = skb->len;
	}else{
		len = 64;
	}

	if(NULL != skb->data){
		print_hex_dump(KERN_ERR, "skb data:", DUMP_PREFIX_ADDRESS,
			16, 1, skb->data, skb->len, true);
	}
}

void spe_dev_setting(void)
{

    struct spe *spe = &spe_balong;

    printk("module name                 :%s\n",spe->name);
    printk("compile info                :%s\n",spe->compile);
    printk("msg_level                   :%u\n",spe->msg_level);
    printk("dbg_level                   :%u\n",spe->dbg_level);
    printk("macfw_timeout               :%u\n",spe->macfw_timeout);
    printk("intr_timeout                :%u\n",spe->intr_timeout);
    printk("ncmwrap_timeout             :%u\n",spe->ncmwrap_timeout);
    printk("phy portmap                 :0x%x\n",(u32)spe->portmap[0]);
    printk("br portmap                  :0x%x\n",(u32)spe->portmap[1]);
    printk("desc_usr_field              :%u\n",spe->desc_usr_field);
    printk("ipfw_usr_field              :%u\n",spe->ipfw_usr_field);
    printk("ipfw_timeout                :%u\n",spe->ipfw_timeout);
    printk("flags                       :%u\n",spe->flags);

}

void spe_dev_info(void)
{
    struct spe *spe = &spe_balong;
    int i;
		
    printk("reg_addr 0x%x \n", (unsigned int)spe->regs);
    if(NULL != spe->ev_buffs[0]){
        printk("evt_buffer_0_addr:0x%x \n", (unsigned int)spe->ev_buffs[0]->buf);
        printk("evt_buffer_0_count %u \n", spe->stat.evt_0_stat);
    }
    if(NULL != spe->ev_buffs[1]){
        printk("evt_buffer_1_addr:0x%x \n", (unsigned int)spe->ev_buffs[1]->buf);
        printk("evt_buffer_1_count %u \n", spe->stat.evt_1_stat);
    }
    printk("evt_td                      :%u\n",spe->stat.evt_td);
	
    for(i=0;i<SPE_PORT_NUM;i++){
    printk("evt_td_complt[i]               :%u\n",spe->stat.evt_td_complt[i]);
    printk("evt_td_empty[i]               :%u\n",spe->stat.evt_td_empty[i]);
    printk("evt_td_full[i]               :%u\n",spe->stat.evt_td_full[i]);
    }
    printk("evt_td_errport              :%u\n",spe->stat.evt_td_errport);
		
    printk("evt_rd                      :%u\n",spe->stat.evt_rd);
     for(i=0;i<SPE_PORT_NUM;i++){
    printk("evt_rd_complt[i]               :%u\n",spe->stat.evt_rd_complt[i]);
    printk("evt_rd_empty[i]               :%u\n",spe->stat.evt_rd_empty[i]);
    printk("evt_rd_full[i]               :%u\n",spe->stat.evt_rd_full[i]);
    }
    printk("evt_rd_errport              :%u\n",spe->stat.evt_rd_errport);
    printk("evt_buf_rd_err              :%u\n",spe->stat.evt_buf_rd_err);
    printk("evt_unknown                 :%u\n",spe->stat.evt_unknown);

    //printk("wait_ready                  :%u\n",spe->stat.wait_ready);
    printk("wait_idle                   :%u\n",spe->stat.wait_idle);
    printk("ipfw_add                    :%u\n",spe->stat.ipfw_add);
    printk("ipfw_del                    :%u\n",spe->stat.ipfw_del);

    for (i = 0; i < SPE_EVENT_STAT_MAX; i++) {
        printk("evt stat cnt[%d]               :%u\n",i, spe->stat.evt_stat[i]);
    }

}

void spe_print_mac_addr(unsigned char * mac)
{
	int i;
	for(i=0; i<6; i++){
		printk("  %2x  ",mac[i]);
	}
	printk(" \n");

}

void spe_port_setting(unsigned int portno)
{

    unsigned int i = portno;
    struct spe_port_ctrl *ctrl = &spe_balong.ports[i].ctrl;
    HI_SPE_PORTX_PROPERTY_T ch_propert;
    ch_propert.u32 = ctrl->property.u32;

	if(PORTNO_INVALID(portno)){
		printk("port num is invalid!\n");
		return;
	}

    printk("port_num        %u \n",    ctrl->portno);
    if(ctrl->net){
        printk("device name     %s \n",    ctrl->net->name);
    }
    printk("is_enable       0x%x \n", ch_propert.bits.spe_port_en);
    printk("port_pad_en   0x%x \n", ch_propert.bits.spe_port_pad_en );
    printk("port_attach     0x%x \n",   ch_propert.bits.spe_port_attach_brg);
    printk("enc_type        0x%x \n",    ch_propert.bits.spe_port_enc_type);

/*
    printk("port mac addr :");
    if (ctrl->net) {
    spe_print_mac_addr(ctrl->net->dev_addr);
    } else {
        printk("no dev\n");
    }
*/
    printk("rd_addr         0x%x \n",	ctrl->rd_addr);
    printk("rd_depth        %u \n",	ctrl->rd_depth);
    printk("rd_evt_gap      0x%x \n",	ctrl->rd_evt_gap);
    printk("rd_dma          0x%x \n",	ctrl->rd_dma);
    printk("rd_skb_num      %u \n",	ctrl->rd_skb_num);
    printk("rd_skb_size     %u \n",	ctrl->rd_skb_size);
    printk("rd_skb_used     0x%x \n",	ctrl->rd_skb_used);
    printk("rd_skb_align    0x%x \n",	ctrl->rd_skb_align);

    printk("td_addr         0x%x \n",	ctrl->td_addr);
    printk("td_depth        %u \n",	ctrl->td_depth);
    printk("td_evt_gap      0x%x \n",	ctrl->td_evt_gap);
    printk("td_dma          0x%x \n",	ctrl->td_dma);


    printk("udp_limit_time  0x%x \n",	ctrl->udp_limit_time);
    printk("udp_limit_cnt   0x%x \n",	ctrl->udp_limit_cnt);
    printk("rate_limit_time 0x%x \n",	ctrl->rate_limit_time);
    printk("rate_limit_byte 0x%x \n",	ctrl->rate_limit_byte);

}

void spe_rd_result_print(struct spe_port_stat *stat)
{
	printk("[rd_result]:rd_result_updata_only %u \n", stat->rd_result[rd_result_updata_only]);
	printk("[rd_result]:rd_result_discard %u \n", stat->rd_result[rd_result_discard]);
	printk("[rd_result]:rd_result_success %u \n", stat->rd_result[rd_result_success]);
	printk("[rd_result]:rd_result_wrap_or_length_wrong %u \n", stat->rd_result[rd_result_wrap_or_length_wrong]);
	printk("\n");
	printk("[rd_reason]:rd_reason_undiscard %u \n", stat->rd_reason[rd_reason_undiscard]);
	printk("[rd_reason]:rd_reason_rd_point_null %u \n", stat->rd_reason[rd_reason_rd_point_null]);
	printk("\n");
	printk("[rd_fw]:rd_fw_mac %u \n", stat->rd_fw[rd_fw_mac]);
	printk("[rd_fw]:rd_fw_eth %u \n", stat->rd_fw[rd_fw_eth]);
	printk("[rd_fw]:rd_fw_first_mac %u \n", stat->rd_fw[rd_fw_first_mac]);
	printk("[rd_fw]:rd_fw_iph %u \n", stat->rd_fw[rd_fw_iph]);
	printk("[rd_fw]:rd_fw_ip_filter %u \n", stat->rd_fw[rd_fw_ip_filter]);
	printk("[rd_fw]:rd_fw_tcp_or_udp %u \n", stat->rd_fw[rd_fw_tcp_or_udp]);
	printk("[rd_fw]:rd_fw_hash_calculate %u \n", stat->rd_fw[rd_fw_hash_calculate]);
	printk("[rd_fw]:rd_fw_hash_read %u \n", stat->rd_fw[rd_fw_hash_read]);
	printk("[rd_fw]:rd_fw_hash_entry_mach %u \n", stat->rd_fw[rd_fw_hash_entry_mach]);
	printk("[rd_fw]:rd_fw_second_mac %u \n", stat->rd_fw[rd_fw_second_mac]);
	printk("[rd_fw]:rd_fw_end %u \n", stat->rd_fw[rd_fw_end]);
	printk("\n");
	printk("[rd_indicate]:rd_udp %u \n", stat->rd_indicate[rd_udp]);
	printk("[rd_indicate]:rd_tcp %u \n", stat->rd_indicate[rd_tcp]);
	printk("[rd_indicate]:rd_ipv6 %u \n", stat->rd_indicate[rd_ipv6]);
	printk("[rd_indicate]:rd_ipv4 %u \n", stat->rd_indicate[rd_ipv4]);
	printk("[rd_indicate]:rd_sport_vlan %u \n", stat->rd_indicate[rd_sport_vlan]);
	printk("\n");
	printk("[rd_wrap]:rd_wrong_format %u \n", stat->rd_wrap[rd_wrong_format]);
	printk("[rd_wrap]:rd_push_en %u \n", stat->rd_wrap[rd_push_en]);
	printk("[rd_wrap]:rd_wrap_num_oversize %u \n", stat->rd_wrap[rd_wrap_num_oversize]);
	printk("[rd_wrap]:rd_wrap_length_oversize %u \n", stat->rd_wrap[rd_wrap_length_oversize]);
	printk("[rd_wrap]:rd_timeout %u \n", stat->rd_wrap[rd_timeout]);
	printk("[rd_wrap]:rd_port_dis %u \n", stat->rd_wrap[rd_port_dis]);

}

void spe_port_rd_info(unsigned int portno)
{

    unsigned int i = portno;
    unsigned int rd_hard_ptr;
    struct spe_port_stat *stat = &spe_balong.ports[i].stat;
    struct spe_port_ctrl *ctrl = &spe_balong.ports[i].ctrl;
    struct spe *spe = &spe_balong;
	if(PORTNO_INVALID(portno)){
		printk("port num is invalid!\n");
		return;
	}

    rd_hard_ptr = spe_readl(spe->regs, SPE_RDQ_PTR_OFFSET(portno));

    printk("rd_busy         0x%x \n",	ctrl->rd_busy);
    printk("rd_free         0x%x \n",	ctrl->rd_free);

    printk("rd_hard_ptr rptr wptrr 0x%x \n",	rd_hard_ptr);


    printk("\n");

    printk("rd send success %u \n", stat->rd_send_success);
    printk("rd net null     %u \n", stat->rd_net_null);
    printk("rd configed     %u \n",	stat->rd_config);
    printk("rd finish       %u \n",	stat->rd_finished);
    printk("rd finish event %u \n",	stat->rd_finsh_intr_complete);
    printk("rd full times   %u \n",	stat->rd_full);
    printk("rd drop       %u \n",	stat->rd_drop);
    printk("rd skb null      %u \n",	stat->rd_skb_null);
    printk("rd irq en       %u \n",	stat->rd_irq_en);
    printk("rd updata only  %u \n",	stat->rd_upd_only);
	printk("netif_rx_drop   %u \n",	stat->netif_rx_drop);
    printk("rd result success %u \n",	stat->rd_result[rd_result_success]);

    printk("\n");

    spe_rd_result_print(stat);

    printk("\n");

    for(i = 0; i<SPE_PORT_NUM; i++){
        printk("rd from port %u count: %u \n",	i, stat->rd_sport[i]);
    }


}

void spe_td_result_print(struct spe_port_stat *stat)
{
	printk("[td_result]:td_result_updata_only %u \n", stat->td_result[td_result_updata_only]);
	printk("[td_result]:td_result_discard %u \n", stat->td_result[td_result_discard]);
	printk("[td_result]:td_result_normal %u \n", stat->td_result[td_result_normal]);
	printk("[td_result]:td_result_wrap_or_lenth_wrong %u \n", stat->td_result[td_result_wrap_or_lenth_wrong]);
	printk("\n");
	printk("[td_reason]:td_reason_undiscard %u \n", stat->td_reason[td_reason_undiscard]);
	printk("[td_reason]:td_reason_wrap_wrong %u \n", stat->td_reason[td_reason_wrap_wrong]);
	printk("[td_reason]:td_reason_length_wrong %u \n", stat->td_reason[td_reason_length_wrong]);
	printk("[td_reason]:td_reason_sport_disable %u \n", stat->td_reason[td_reason_sport_disable]);
	printk("[td_reason]:td_reason_mac_filt %u \n", stat->td_reason[td_reason_mac_filt]);
	printk("[td_reason]:td_reason_ipv4_csum %u \n", stat->td_reason[td_reason_ipv4_csum]);
	printk("[td_reason]:td_reason_ttl %u \n", stat->td_reason[td_reason_ttl]);
	printk("[td_reason]:td_reason_ip_filt %u \n", stat->td_reason[td_reason_ip_filt]);
	printk("[td_reason]:td_reason_udp_rate_limit %u \n", stat->td_reason[td_reason_udp_rate_limit]);
	printk("[td_reason]:td_reason_mac_fw_entry_wrong %u \n", stat->td_reason[td_reason_mac_fw_entry_wrong]);
	printk("[td_reason]:td_reason_dport_disable %u \n", stat->td_reason[td_reason_dport_disable]);
	printk("[td_reason]:td_reason_protocol_differ %u \n", stat->td_reason[td_reason_protocol_differ]);
	printk("[td_reason]:td_reason_rd_port_null %u \n", stat->td_reason[td_reason_rd_port_null]);
	printk("\n");
	printk("[td_fw]:td_fw_mac %u \n", stat->td_fw[td_fw_mac]);
	printk("[td_fw]:td_fw_eth %u \n", stat->td_fw[td_fw_eth]);
	printk("[td_fw]:td_fw_first_mac %u \n", stat->td_fw[td_fw_first_mac]);
	printk("[td_fw]:td_fw_iph %u \n", stat->td_fw[td_fw_iph]);
	printk("[td_fw]:td_fw_ip_filter %u \n", stat->td_fw[td_fw_ip_filter]);
	printk("[td_fw]:td_fw_tcp_or_udp %u \n", stat->td_fw[td_fw_tcp_or_udp]);
	printk("[td_fw]:td_fw_hash_calculate %u \n", stat->td_fw[td_fw_hash_calculate]);
	printk("[td_fw]:td_fw_hash_read %u \n", stat->td_fw[td_fw_hash_read]);
	printk("[td_fw]:td_fw_hash_entry_mach %u \n", stat->td_fw[td_fw_hash_entry_mach]);
	printk("[td_fw]:td_fw_second_mac %u \n", stat->td_fw[td_fw_second_mac]);
	printk("[td_fw]:td_fw_end %u \n", stat->td_fw[td_fw_end]);
	printk("\n");
	printk("[td_indicate]:td_udp %u \n", stat->td_indicate[td_udp]);
	printk("[td_indicate]:td_tcp %u \n", stat->td_indicate[td_tcp]);
	printk("[td_indicate]:td_ipv6 %u \n", stat->td_indicate[td_ipv6]);
	printk("[td_indicate]:td_ipv4 %u \n", stat->td_indicate[td_ipv4]);
	printk("[td_indicate]:td_sport_vlan %u \n", stat->td_indicate[td_sport_vlan]);
	printk("\n");
	printk("[td_ncm]:td_ncm_success %u \n", stat->td_ncm[td_ncm_success]);
	printk("[td_ncm]:td_ncm_part_discard %u \n", stat->td_ncm[td_ncm_part_discard]);
	printk("[td_ncm]:td_ncm_all_discard %u \n", stat->td_ncm[td_ncm_all_discard]);
	printk("\n");
	printk("[td_wrap]:td_wrap_success %u \n", stat->td_wrap[td_wrap_success]);
	printk("[td_wrap]:td_port_disable %u \n", stat->td_wrap[td_port_disable]);
	printk("[td_wrap]:td_abnormity %u \n", stat->td_wrap[td_abnormity]);

	printk("[td_wrap]:td_ncm_nth_flag_wrong %u \n", stat->td_wrap[td_ncm_nth_flag_wrong]);
	printk("[td_wrap]:td_ncm_nth_length_wrong %u \n", stat->td_wrap[td_ncm_nth_length_wrong]);
	printk("[td_wrap]:td_ncm_block_length_wrong %u \n", stat->td_wrap[td_ncm_block_length_wrong]);
	printk("[td_wrap]:td_ncm_ndp_flag_wrong %u \n", stat->td_wrap[td_ncm_ndp_flag_wrong]);
	printk("[td_wrap]:td_ncm_ndp_length_wrong %u \n", stat->td_wrap[td_ncm_ndp_length_wrong]);
	printk("[td_wrap]:td_ncm_datagram_wrong %u \n", stat->td_wrap[td_ncm_datagram_wrong]);
	printk("[td_wrap]:td_ncm_eth_length_wrong %u \n", stat->td_wrap[td_ncm_eth_length_wrong]);

}




void spe_port_td_info(unsigned int portno)
{
    unsigned int i = portno;
    unsigned int td_hard_ptr;
    struct spe_port_stat *stat = &spe_balong.ports[i].stat;
    struct spe_port_ctrl *ctrl = &spe_balong.ports[i].ctrl;
    struct spe *spe = &spe_balong;
	if(PORTNO_INVALID(portno)){
		printk("port num is invalid!\n");
		return;
	}

    printk("td_busy 0x%x \n",	ctrl->td_busy);
    printk("td_free 0x%x \n",	ctrl->td_free);

    td_hard_ptr = spe_readl(spe->regs, SPE_TDQ_PTR_OFFSET(portno));

    printk("td_hard_ptr rptr wptr 0x%x \n",	td_hard_ptr);
    printk("\n");

    printk("td config %u \n",	stat->td_config);
    printk("td config(port disabled) %u \n",	stat->td_port_disabled);
    printk("td full times %u \n",	stat->td_full);
    printk("td finish event %u \n",	stat->td_finsh_intr_complete);
    printk("td push %u \n",	stat->td_push);
    printk("td_irq_en %u \n",	stat->td_irq_en);
    printk("td_ts_en %u \n",	stat->td_ts_en);
    printk("td_dma_null %u \n",	stat->td_dma_null);

    printk("\n");
    spe_td_result_print(stat);

}

void spe_ipfw_setting(void)
{
    struct spe_ip_fw_ctx *ipfw_ctx = &spe_balong.ipfw;

    printk("free_cnt %u \n",	ipfw_ctx->free_cnt);
    printk("free_threhold %u \n",	ipfw_ctx->free_threhold);
    printk("deadtime %u \n",	ipfw_ctx->deadtime);
    printk("hash list base 0x%x \n", (u32)ipfw_ctx->hbucket);
    printk("hash list size %u \n",	ipfw_ctx->hsize);
    printk("hash list width %u \n",	ipfw_ctx->hwidth);
    printk("hash list zone %u \n",	ipfw_ctx->hzone);
}

void spe_port_info_all(unsigned int port_num)
{
	if(PORTNO_INVALID(port_num)){
		printk("port num is invalid!\n");
		return;
	}
	spe_port_setting(port_num);
	spe_port_rd_info(port_num);
	spe_port_td_info(port_num);
}

static void print_hex(unsigned int* base, unsigned int size)
{
	int i;
	unsigned int* x;
	for(x=base,i=0;i<size;i++,x++)
	{
		if(0==i%4) printk("\n0x%4.4x: ",i);
		printk("%8.8x ",*x);
	}
	printk("\n");
}

void spe_dump_desc(void)
{
	struct spe *spe = &spe_balong;
	int i;
	unsigned int x;
	unsigned int size;
	unsigned int *addr;
	
	printk("descrptor:\n");
	for(i=0;i<8;i++)
	{

		x = spe_readl(spe->regs, SPE_TDQ_BADDR_OFFSET(i));
		size = spe_readl(spe->regs,SPE_TDQ_LEN_OFFSET(i));
		
		if(x){
			addr = phys_to_virt(x);
			printk("port%d td_dump:\n",i);
			if(addr)
			print_hex(addr, size);
		}

		x = spe_readl(spe->regs, SPE_RDQ_BADDR_OFFSET(i));
		size = spe_readl(spe->regs,SPE_RDQ_LEN_OFFSET(i));
		
		if(x){
			addr = phys_to_virt(x);
			printk("port%d rd_dump:\n",i);
			if(addr)
			print_hex(addr, size);
		}
	}
}

void spe_common_config_show(void)
{
	struct spe *spe=&spe_balong;
	u32 val;

	val = spe_readl(spe->regs, HI_SPE_MODE_OFFSET);
	printk("spe mode: %s\n", val?"stick":"E5");
}

void spe_dump_all(void)
{
	struct spe *spe = &spe_balong;
	int i;

	printk("reg map:\n");
	print_hex(spe->regs, 0x800);

	for(i=0;i<8;i++)
	{
		spe_port_info_all(i);
	}

	spe_dump_desc();
}

