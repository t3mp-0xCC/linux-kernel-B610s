#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>

#include "vp_api.h"
#include "ZLR96621L_SM2_CHINA.h"

#include "bsp_softtimer.h"
#include "soc_interrupts.h"

extern VpDevCtxType DevCtx;
extern struct softtimer_list   slic_softtimer;
extern void fault_event_handle(VpLineCtxType *pLineCtx,VpEventType *pEvent);
extern void response_event_handle(VpLineCtxType *pLineCtx,VpEventType *pEvent);
extern void signal_event_handle(VpLineCtxType *pLineCtx,VpEventType *pEvent);
extern void process_event_handle(VpLineCtxType *pLineCtx,VpEventType *pEvent);

irqreturn_t zsi_irq_thread_handler(int irq, void *devid)
{
    VpLineCtxType *pLineCtx = NULL;
    VpEventCategoryType eventCategory;
    VpEventType   Event;

	if( VpGetEvent(&DevCtx, &Event) )
	{
		pLineCtx = Event.pLineCtx;
		eventCategory = Event.eventCategory;
		switch(eventCategory)
		{
			case VP_EVCAT_FAULT:
				 fault_event_handle(pLineCtx,&Event);
				 break;

			case VP_EVCAT_RESPONSE:
				 response_event_handle(pLineCtx,&Event);
				 break;

			case VP_EVCAT_SIGNALING:
				 signal_event_handle(pLineCtx,&Event);
				 break;

			case VP_EVCAT_PROCESS:
				 process_event_handle(pLineCtx,&Event);
				 break;

			default:
				 break;
		}
	}
	return IRQ_HANDLED;

}


void zsi_irq_init(void)
{
	int ret = 0;
	unsigned int irq_num = INT_LVL_ZSI;
	ret = request_threaded_irq(irq_num, NULL, zsi_irq_thread_handler, IRQF_TRIGGER_HIGH | IRQF_ONESHOT, "zsi_irq", NULL);
	if (ret) {
		printk(KERN_ERR "failed to request irq %d.\n", irq_num);
		return -1;
	}
	printk(KERN_ERR "zsi irq init ok.\n");

}

void zsi_stop_timer(void)
{
	bsp_softtimer_delete(&slic_softtimer);
}

void zsi_start_timer(void)
{
	bsp_softtimer_add(&slic_softtimer);
}


