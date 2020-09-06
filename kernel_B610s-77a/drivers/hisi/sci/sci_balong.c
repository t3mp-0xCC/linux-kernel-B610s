

#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include "osl_types.h"
#include "osl_bio.h"
#include "bsp_pmu.h"
#include "bsp_icc.h"
#include "sci_balong.h"
#include "mdrv_public.h"
#include <linux/wakelock.h>

/* sci init flag */
bsp_sci_st g_sci_stat = {0};
struct work_struct	sim_detect_work;
struct work_struct	sim_hpd_work;

sim_hotplug_cfg g_sim_hotplug_cfg;

/*sim卡热插拔过程防止睡眠锁*/
static struct wake_lock  sim_hotplug_lock;
/*防止休眠的时间定为1s*/
#define  SIM_HOTPLUG_LOCK_TIME  1000

void sim_pmu_hpd_low_callback(u32 *pSciInOutEvent);
void sim_pmu_hpd_high_callback(u32 *pSciInOutEvent);
void sim_pmu_hpd_raise_callback(u32 *pSciInOutEvent);
void sim_pmu_hpd_fall_callback(u32 *pSciInOutEvent);

static irqreturn_t sim_detect_irq_cb(int irq, void *dev_id) ;
static void sim_detect_work_handler(struct work_struct *data);
static void sim_hpd_work_handler(struct work_struct *data);

/*******************************************************************************
  Function:      sci_debug_status_print
  Description:   
  Input:         irq: irq number
                 devid: device id
  Output:        irq handle result
  Return:        NA
  Others:        NA
*******************************************************************************/
void sci_debug_status_print(void)
{
	/* status */
	SCI_PRINT("sci_init_flag is %d \n",g_sci_stat.sci_init_flag);
	SCI_PRINT("sci0_card_satus is %d \n",g_sci_stat.sci0_card_satus);
    SCI_PRINT("sci0_pmu_hpd_status is %d \n",g_sci_stat.sci0_pmu_hpd_status);
	SCI_PRINT("sci0_detect_level is %d \n",g_sci_stat.sci0_detect_level);
}


void sim_show_current_state(void)
{  
    SCI_PRINT("g_sci_stat.sci0_detect_level = %d\n",g_sci_stat.sci0_detect_level);
    SCI_PRINT("g_sci_stat.sci0_pmu_hpd_status = %d\n",g_sci_stat.sci0_pmu_hpd_status);
}


void sim_pmu_hpd_fall_callback(u32 *pSciInOutEvent)
{
    SCI_PRINT("sim_pmu_hpd_fall_callback\n");
    g_sci_stat.sci0_pmu_hpd_status = SIM_PMU_HPD_FALL;

    switch((u32)(g_sci_stat.sci0_card_satus))
    {
        case SIM_CARD_STATUS_IN:
            g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_IN;
            break;
        case SIM_CARD_STATUS_IN_POSITION:
            g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_LEAVE;            
            schedule_work(&sim_hpd_work);/*发送LEAVE 消息*/
            break;
    }
    sim_show_current_state();
}
void sim_pmu_hpd_raise_callback(u32 *pSciInOutEvent)
{
    g_sci_stat.sci0_pmu_hpd_status = SIM_PMU_HPD_RAISE;
    SCI_PRINT("sim_pmu_hpd_raise_callback\n");

    switch((u32)(g_sci_stat.sci0_card_satus))
    {
        case SIM_CARD_STATUS_IN:
            g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_IN_POSITION;            
            schedule_work(&sim_hpd_work);/*发送IN 消息*/
            break;
        case SIM_CARD_STATUS_LEAVE:
            g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_LEAVE;
            break;
    }
    sim_show_current_state();

}



s32 bsp_sci_init(void)
{ 

	u32 detect_level;
	u32 int_num;
    s32 ret = MDRV_ERROR;
    char* name ="hisilicon,sim0_ap";
    struct device_node* sim_hotplug_node = NULL;
    u32 int_status;

	if(true == g_sci_stat.sci_init_flag)
	{
	    SCI_PRINT("sci has inited\n");
		return MDRV_OK;
	}

    sim_hotplug_node = of_find_compatible_node(NULL, NULL, name);
    if(sim_hotplug_node != NULL)
    {
        (void)of_property_read_u32_index(sim_hotplug_node, "sim0_hpd_low", 0,&g_sim_hotplug_cfg.sim_hpd_low);        
        (void)of_property_read_u32_index(sim_hotplug_node, "sim0_hpd_high", 0,&g_sim_hotplug_cfg.sim_hpd_high);        
        (void)of_property_read_u32_index(sim_hotplug_node, "sim0_hpd_fall", 0,&g_sim_hotplug_cfg.sim_hpd_fall);        
        (void)of_property_read_u32_index(sim_hotplug_node, "sim0_hpd_raise", 0,&g_sim_hotplug_cfg.sim_hpd_raise);        
        (void)of_property_read_u32_index(sim_hotplug_node, "sim0_gpio", 0,&g_sim_hotplug_cfg.sim_gpio);
    }
    else
    {
        SCI_PRINT("find sim hot plug dts fail\n");
        return MDRV_ERROR;
    }

    
    if(MDRV_OK !=gpio_request(g_sim_hotplug_cfg.sim_gpio,"sim_detect"))
	{
		SCI_PRINT("%s gpio_request failed! \n",__FUNCTION__);
		
		return MDRV_ERROR;
	}
	gpio_direction_input(g_sim_hotplug_cfg.sim_gpio); 

	/*  获取detect管脚电平高低*/
	detect_level = gpio_get_value(g_sim_hotplug_cfg.sim_gpio);
	if(SIM_CARD_DETECT_LOW == detect_level)
	{
		g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_OUT;
		g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_LOW;
	}
	else 
	{
		g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_IN_POSITION;
		g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_HIGH;
	}

    wake_lock_init(&sim_hotplug_lock, WAKE_LOCK_SUSPEND, "sim_hotplug_lock");

	int_num = gpio_to_irq(g_sim_hotplug_cfg.sim_gpio);

	if(request_irq(int_num, sim_detect_irq_cb,\
                    IRQF_NO_SUSPEND | IRQF_SHARED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, DETECT_NAME_SIM, "sim_detect"))
	{
		SCI_PRINT("%s request_irq failed! \n",__FUNCTION__);
		
		return MDRV_ERROR;
    }


    int_status = g_sim_hotplug_cfg.sim_hpd_fall;

	ret = bsp_pmu_irq_callback_register(g_sim_hotplug_cfg.sim_hpd_fall, (pmufuncptr)sim_pmu_hpd_fall_callback,&int_status);
    if(ret == MDRV_ERROR)
    {
        SCI_PRINT("register pmu irq fail\n");
        return MDRV_ERROR;
    }
    int_status = g_sim_hotplug_cfg.sim_hpd_raise;

	ret = bsp_pmu_irq_callback_register(g_sim_hotplug_cfg.sim_hpd_raise, (pmufuncptr)sim_pmu_hpd_raise_callback,&int_status);
    if(ret == MDRV_ERROR)
    {
        SCI_PRINT("register pmu irq fail\n");
        return MDRV_ERROR;
    }

    INIT_WORK(&sim_detect_work, sim_detect_work_handler);

    INIT_WORK(&sim_hpd_work, sim_hpd_work_handler);

	g_sci_stat.sci_init_flag = true;

	/* print func error */
	SCI_PRINT("Acore: sci init ok \n");
	sci_debug_status_print();
	return MDRV_OK;
}


/*******************************************************************************
  Function:      sim_detect_irq_cd(int irq, void *dev_id)
  Description:   sd  detect interrpt handle
  Input:         irq: irq number
                 devid: device id
  Output:        irq handle result
  Return:        NA
  Others:        NA
*******************************************************************************/
static irqreturn_t sim_detect_irq_cb(int irq, void *dev_id)    
{  
    SCI_PRINT("enter det irq\n");
    wake_lock_timeout(&sim_hotplug_lock, (long)msecs_to_jiffies(SIM_HOTPLUG_LOCK_TIME));
    schedule_work(&sim_detect_work);

    return IRQ_HANDLED;

}

/*******************************************************************************
  Function:      sim_detect_work(struct work_struct *data)
  Description:   sd detect handle work
  Input:         data:work_struct
                 devid:
  Output:        NA
  Return:        NA
  Others:        NA
*******************************************************************************/
static void sim_detect_work_handler(struct work_struct *data)
{

	u32 detect_level = SIM_CARD_DETECT_LOW;
    s32 u32Lenth;
	u32 pSciInOut;
	u32 channel_id = 0;
        
	detect_level = gpio_get_value(g_sim_hotplug_cfg.sim_gpio);
    switch(detect_level)
    {
    case SIM_CARD_DETECT_HIGH:
  
		g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_HIGH;
        SCI_PRINT("g_sci_stat.sci0_detect_level = %d\n",g_sci_stat.sci0_detect_level);
        msleep(5);

        detect_level = gpio_get_value(g_sim_hotplug_cfg.sim_gpio);
        if(detect_level == SIM_CARD_DETECT_HIGH)
        {
            g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_IN;

        }
        else if(detect_level == SIM_CARD_DETECT_LOW)
        {
            g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_OUT;
        }
        sci_debug_status_print();
        break;
    case SIM_CARD_DETECT_LOW:
        msleep(5);
        
        SCI_PRINT("g_sci_stat.sci0_detect_level = %d\n",g_sci_stat.sci0_detect_level);
        g_sci_stat.sci0_detect_level = SIM_CARD_DETECT_LOW;

        g_sci_stat.sci0_card_satus = SIM_CARD_STATUS_OUT;
        
        sci_debug_status_print();

        pSciInOut = SIM_CARD_STATUS_OUT;
        channel_id = (ICC_CHN_IFC<< 16 | IFC_RECV_FUNC_SIM0);
        u32Lenth = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8*)&pSciInOut, sizeof(u32));
        if(u32Lenth != sizeof(u32))
        {
            SCI_PRINT("send SIM_CARD_OUT icc message fail\n");
        } 
        else
        {
            SCI_PRINT("send SIM_CARD_OUT icc message success\n");
        }
    
        break;
    default:
        SCI_PRINT("wrong detect level\n");
    }

}

/*******************************************************************************
  Function:      sim_hpd_work_handler(struct work_struct *data)
  Description:   sim hpd handle work
  Input:         data:work_struct
                 devid:
  Output:        NA
  Return:        NA
  Others:        NA
*******************************************************************************/
static void sim_hpd_work_handler(struct work_struct *data)
{
    s32 u32Lenth;
	u32 pSciInOut = -1;
	u32 channel_id = 0;

    msleep(200);   
    switch(g_sci_stat.sci0_card_satus)
    {
    case SIM_CARD_STATUS_IN_POSITION:
        pSciInOut = SIM_CARD_STATUS_IN_POSITION;
    	break;
    case SIM_CARD_STATUS_LEAVE:
        pSciInOut = SIM_CARD_STATUS_LEAVE;
        break;
    default:
        break;

    }
    sci_debug_status_print();
    channel_id = (ICC_CHN_IFC<< 16 | IFC_RECV_FUNC_SIM0);
    
    if(pSciInOut != -1)
 	{
 	    u32Lenth = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8*)&pSciInOut, sizeof(u32));
   	    if(u32Lenth != sizeof(u32))
        {
    	    SCI_PRINT("send %d icc message failed! \n",pSciInOut);
        }
        else
        {
    	    SCI_PRINT("send %d icc message success! \n",pSciInOut);
        }
    }
}



/* init func */
module_init(bsp_sci_init); 
/* EXPORT_SYMBOL(bsp_vic_enable); */




