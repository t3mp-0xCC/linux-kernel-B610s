#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/atomic.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/wait.h>
//#include <asm-generic/uaccess.h>

#include "agent.h"
#include "teek_ns_client.h"
#include "tui.h"
#include "smc.h"
#include "tee_client_constants.h"
#include "tc_ns_client.h"
#include "hisi_fb.h"

/* add for CMA malloc framebuffer */
#include <linux/hisi/hisi_ion.h>
#include <linux/ion.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>


#define HEAP_ID_MASK (1<<ION_TUI_HEAP_ID)

static void tui_delay_work_func(struct work_struct *work);
static void tui_timeout_work_func(struct work_struct *work);
static void tui_poweroff_work_func(struct work_struct *work);
static DECLARE_DELAYED_WORK(tui_poweroff_work, tui_poweroff_work_func);
static DECLARE_DELAYED_WORK(tui_delay_work, tui_delay_work_func);
static DECLARE_DELAYED_WORK(tui_timeout_work, tui_timeout_work_func);

/* command from secure os */
#define TUI_CMD_ENABLE       1
#define TUI_CMD_DISABLE      2
#define TUI_CMD_POLL		3
#define TUI_CMD_SET_STATE   4
#define TUI_CMD_PAUSE		5
#define TUI_CMD_DO_SYNC		6
#define TUI_CMD_START_DELAY_WORK    7
#define TUI_CMD_CANCEL_DELAY_WORK   8
#define TUI_CMD_SEND_INPUT_WORK 9

/* tui states */
#define TUI_STATE_UNUSED	0
#define TUI_STATE_CONFIG	1
#define TUI_STATE_RUNNING	2
#define TUI_STATE_ERROR	3

#define TUI_TICK_RATE	2000	/* ms */
#define TUI_DEFAULT_TIMEOUT	30000	/* ms  modify to  ten minutes for test */
#define TUI_DELAY_WORK_TIME	1000	/* ms */

/* tui-max should be bigger than TUI_POLL_MAX in tui.h*/
static const char * const poll_event_type_name[] = {
	"config-ok",
	"config-fail",
	"tp",
	"tui-tick",
	"tui-delaywork",
	"tui-pause",
	"tui-resume",
	"tui-terminate",
	"tui-handle",
	"tui-hs",
	"tui-sh",
	"tui-01",
	"tui-02",
	"tui-10",
	"tui-20",
	"tui-0",
	"tui-90",
	"tui-180",
	"tui-270",
	"tui-k0",
	"tui-k3",
	"tui-max"};

static const char * const state_name[] = {
	"unused",
	"config",
	"running",
	"error"
};

#define tui_dbg(fmt, args...)	pr_info(KERN_INFO "[tui]: %s :" fmt, \
		__func__, ## args)
#define tui_err(fmt, args...)	pr_err(KERN_ERR "[tui]: %s :" fmt, \
		__func__, ## args)

struct tui_ctl_shm {
	struct {
		int command;
		int value;
		int ret;
	} s2n;
	struct {
		int event_type;
		int value;
	} n2s;
};

struct tui_msg_node {
	int type;
	int val;
	void *data;
	struct list_head list;
};

struct tui_drv_node {
	tui_drv_init init_func;
	void *pdata;
	char name[TUI_DRV_NAME_MAX];
	int state;
	struct list_head list;
};

static struct task_struct *tui_task;
static struct tui_ctl_shm *tui_ctl;

static spinlock_t tui_msg_lock;
static struct list_head tui_msg_head;

static atomic_t tui_state = ATOMIC_INIT(TUI_STATE_UNUSED);

DEFINE_MUTEX(tui_drv_lock);
static struct list_head tui_drv_head = LIST_HEAD_INIT(tui_drv_head);

static unsigned int tui_attached_device;

struct timer_list tui_tick_timer;
struct timer_list tui_timeout_timer;
struct timer_list tui_delay_work_timer;

static unsigned int tui_timeout;

static wait_queue_head_t tui_state_wq;
static int tui_state_flag;

static const char *ion_name="TUI_ION";
static struct ion_client *tui_client = NULL;
static struct ion_handle *tui_handle = NULL;

static wait_queue_head_t tui_msg_wq;
static wait_queue_head_t wait_event_get_keyCode;
static int get_keyCode_flag;
static int tui_msg_flag;
struct hisi_fb_data_type *dss_fd = NULL;

//size now 1080*1920 should be 16M
int get_frame_addr(int size)
{
       ion_phys_addr_t addr=0;
	size_t len = 0;
       tui_handle = ion_alloc(tui_client, size, 0, HEAP_ID_MASK, ION_FLAG_SECURE_BUFFER);
       if(IS_ERR(tui_handle)){
	      tui_err("get tui_handld error tui_handle=%p \n",tui_handle);
             return -1;
       }
       ion_phys(tui_client, tui_handle, &addr, &len);
	//0x2f000000 or 0x30000000
	tui_dbg("addr=0x%x \n",(unsigned int)addr);
       return (unsigned int)addr;
}

void free_frame_addr()
{
       if(!IS_ERR(tui_handle) && tui_handle != NULL){
              ion_free(tui_client, tui_handle);
              tui_handle = NULL;
       }
}


int register_tui_driver(tui_drv_init fun, const char *name, void *pdata)
{
	struct tui_drv_node *tui_drv, *pos;

	mutex_lock(&tui_drv_lock);
	list_for_each_entry(pos, &tui_drv_head, list) {
		if (!strncmp(pos->name, name, TUI_DRV_NAME_MAX)) {
			tui_err("this drv(%s) have registered\n", name);
			mutex_unlock(&tui_drv_lock);
			return -EINVAL;
		}
	}
	mutex_unlock(&tui_drv_lock);

	tui_drv = kzalloc(sizeof(*tui_drv), GFP_KERNEL);
	if (!tui_drv) {
		tui_err("alloc for tui_drv failed\n");
		return -ENOMEM;
	}

	tui_drv->init_func = fun;
	tui_drv->pdata = pdata;
	if(!strcmp(name, "DSS"))
		dss_fd = (struct hisi_fb_data_type *)pdata;
	strncpy(tui_drv->name, name, TUI_DRV_NAME_MAX);
	INIT_LIST_HEAD(&tui_drv->list);

	mutex_lock(&tui_drv_lock);
	list_add_tail(&tui_drv->list, &tui_drv_head);
	mutex_unlock(&tui_drv_lock);

	return 0;
}

void unregister_tui_driver(const char *name)
{
	struct tui_drv_node *pos;

	mutex_lock(&tui_drv_lock);
	list_for_each_entry(pos, &tui_drv_head, list) {
		if (!strncmp(pos->name, name, TUI_DRV_NAME_MAX)) {
			list_del(&pos->list);
			kfree(pos->pdata);
			kfree(pos);
			break;
		}
	}
	mutex_unlock(&tui_drv_lock);
}

static int add_tui_msg(int type, int val, void *data)
{
	struct tui_msg_node *tui_msg;
	unsigned long flags;

	tui_msg = kmalloc(sizeof(*tui_msg), GFP_KERNEL);
	if (!tui_msg) {
		tui_err("alloc for tui_msg failed\n");
		return -ENOMEM;
	}

	tui_msg->type = type;
	tui_msg->val = val;
	tui_msg->data = data;
	INIT_LIST_HEAD(&tui_msg->list);

	spin_lock_irqsave(&tui_msg_lock, flags);
	list_add_tail(&tui_msg->list, &tui_msg_head);
	tui_msg_flag = 1;
	spin_unlock_irqrestore(&tui_msg_lock, flags);

	return 0;
}

char *dss_driver = "DSS";
char *gpio_driver = "fff0d000.gpio";
char *tp_driver = "tp";

char *init_driver[3] = {"DSS","tp","fff0d000.gpio"};
char *deinit_driver[3] = {"DSS", "fff0d000.gpio","tp"};
#define DRIVER_NUM 3
#define TIME_OUT_FOWER_ON 100

/* secure : 0-unsecure, 1-secure */
static int init_tui_driver(int secure)
{
	struct tui_drv_node *pos;
	char *drv_name = NULL;
	char **drv_array = deinit_driver;
	int count = 0;
	if(secure)
		drv_array = init_driver;
        int i=0;
	while(i<3){
		drv_name = drv_array[i];
		i++;
		mutex_lock(&tui_drv_lock);
		list_for_each_entry(pos, &tui_drv_head, list) {
			if(!strcmp(drv_name,pos->name))
			{
				tui_dbg("init tui drv(%s) state=%d\n", pos->name, secure);
				if (pos->init_func) {
					if(!strcmp("DSS",pos->name) && secure){
						tui_err("init_tui_driver wait power on status---\n");
						while(!dss_fd->panel_power_on && count < TIME_OUT_FOWER_ON){
							count++;
							msleep(1);
						}
						if(count == TIME_OUT_FOWER_ON){
							mutex_unlock(&tui_drv_lock);
							tui_err("init_tui_driver wait power on status time out\n");
							return -1;
						}
					}
					if (pos->init_func(pos->pdata, secure)) {
						pos->state = -1;
						mutex_unlock(&tui_drv_lock);
						return -1;
					} else {
						/* set secure state will be proceed in tui msg */
						if (!secure)
							pos->state = 0;
					}
				}
			}
		}
		mutex_unlock(&tui_drv_lock);
	}

	return 0;
}

/* Only after all drivers cfg ok or some one failed, it need
 * to add_tui_msg.
 * ret val:	 1 - all cfg ok
 *           0 - cfg is not complete, or have done
 *          -1 - cfg failed
 *          -2 - invalid name
 */
static int tui_cfg_filter(const char *name, bool ok)
{
	struct tui_drv_node *pos;
	char find = 0;
	int drv_count = 0, drv_ok_count = 0;
	int lock_flag = 0;

	/* some drivers may call send_tui_msg_config at the end
	 * of drv_init_func which had got the lock.
	 */
	if (mutex_is_locked(&tui_drv_lock))
		lock_flag = 1;
	if (!lock_flag)
		mutex_lock(&tui_drv_lock);
	list_for_each_entry(pos, &tui_drv_head, list) {
		drv_count++;
		if (!strncmp(pos->name, name, TUI_DRV_NAME_MAX)) {
			find = 1;
			if (ok)
				pos->state = 1;
			else {
				mutex_unlock(&tui_drv_lock);
				return -1;
			}
		}
		if (pos->state == 1)
			drv_ok_count++;
	}
	if (!lock_flag)
		mutex_unlock(&tui_drv_lock);

	if (find == 0)
		return -2;

	if (drv_count > drv_ok_count)
		return 0;

	return 1;
}

enum poll_class {
	CLASS_POLL_CONFIG,
	CLASS_POLL_RUNNING,
	CLASS_POLL_COMMON
};

static inline enum poll_class tui_poll_class(int event_type)
{
	enum poll_class class = CLASS_POLL_COMMON;

	switch (event_type) {
	case TUI_POLL_CFG_OK:
	case TUI_POLL_CFG_FAIL:
	case TUI_POLL_RESUME_TUI:
		class = CLASS_POLL_CONFIG;
		break;
	case TUI_POLL_TP:
	case TUI_POLL_TICK:
	case TUI_POLL_DELAYED_WORK:
	case TUI_POLL_PAUSE_TUI:
		class = CLASS_POLL_RUNNING;
		break;
	case TUI_POLL_CANCEL:
		class = CLASS_POLL_COMMON;
		break;
	default:
		break;
	}
	return class;
}

int send_tui_msg_config(int type, int val, void *data)
{
	int ret;

	if (type >= TUI_POLL_MAX) {
		tui_err("invalid tui event type\n");
		return -EINVAL;
	}

	if (atomic_read(&tui_state) != TUI_STATE_CONFIG) {
		tui_err("failed to send tui msg(%s) when tui is't in config state\n",
				poll_event_type_name[type]);
		return -EINVAL;
	}

	if (tui_poll_class(type) == CLASS_POLL_RUNNING) {
		tui_err("invalid tui event type(%s) in config state\n",
			 poll_event_type_name[type]);
		return -EINVAL;
	}

	tui_dbg("send config event type %s(%s)\n",
		 poll_event_type_name[type], (char*)data);

	if (type == TUI_POLL_CFG_OK || type == TUI_POLL_CFG_FAIL) {
		int cfg_ret;

		cfg_ret = tui_cfg_filter(
			(const char *)data,	type == TUI_POLL_CFG_OK);
		tui_dbg("tui driver(%s) cfg ret = %d\n", (char*)data, cfg_ret);
		if (!cfg_ret)
			return 0;
		else if (cfg_ret == -2)
			return -EINVAL;
	}

	ret = add_tui_msg(type, val, data);
	if (ret) {
		tui_err("add tui msg ret=%d\n", ret);
		return ret;
	}

	tui_dbg("add config msg type %s\n", poll_event_type_name[type]);

	/* wake up tui kthread */
	wake_up(&tui_msg_wq);

	return 0;
}


int tui_send_event(int event)
{
    TC_NS_SMC_CMD smc_cmd = {0};
    unsigned int smc_cmd_phys;
    int ret = 0;
    unsigned char uuid[17] = {0};
    TC_NS_Operation operation = {0};


    tui_dbg("need send tui event = %d \n",event);
    uuid[0] = 1;
    smc_cmd.uuid_phys = virt_to_phys(uuid);
    smc_cmd.cmd_id = 17;
    smc_cmd.agent_id = event;
    smc_cmd.operation_phys = virt_to_phys(&operation);

    ret = TC_NS_SMC(&smc_cmd, 0);

    return ret;
}

int tui_get_keyCode(){
	int r;
	tui_dbg("wait_event_interruptible----------\n");
	r = wait_event_interruptible(wait_event_get_keyCode, get_keyCode_flag);
	tui_dbg("get tui keyCode is interrupted and keyCode=%d---\n", tui_ctl->s2n.value);
	get_keyCode_flag = 0;
	return tui_ctl->s2n.value;
}

int send_tui_msg_running(int type, int val, void *data)
{
	int ret;

	if (type >= TUI_POLL_MAX) {
		tui_err("invalid tui event type\n");
		return -EINVAL;
	}

	if (atomic_read(&tui_state) != TUI_STATE_RUNNING) {
		tui_err("failed to send tui msg(%s) when tui isn't in running state\n",
				 poll_event_type_name[type]);
		return -EINVAL;
	}

	if (tui_poll_class(type) == CLASS_POLL_CONFIG) {
		tui_err("invalid tui event type(%s) in running state\n",
			poll_event_type_name[type]);
		return -EINVAL;
	}

	ret = add_tui_msg(type, val, data);
	if (ret)
		return ret;

	tui_dbg("add running msg type %s\n", poll_event_type_name[type]);

	/* wake up tui kthread */
	wake_up(&tui_msg_wq);

	return 0;
}

int send_tui_msg_adapt(int type, int val, void *data)
{
	if (atomic_read(&tui_state) == TUI_STATE_UNUSED){
		tui_dbg("do nothing because TUI in unused\n");
		return 0;
	}
	else if (atomic_read(&tui_state) == TUI_STATE_RUNNING)
		//return send_tui_msg_running(type, 0, NULL);
		return tui_send_event(type);
	else
		return tui_send_event(type);//send_tui_msg_config(type, 0, NULL);
}
#if 0
static void tui_tick_work(unsigned long data)
{
	unsigned long expires;

	/* TODO: wait for cursor imp */
	//send_tui_msg_running(TUI_POLL_TICK, 0, NULL);

	expires = jiffies + msecs_to_jiffies(TUI_TICK_RATE);
	mod_timer(&tui_tick_timer, expires);
}
#endif
static void tui_tick_start(void)
{
	unsigned long expires;

	if (atomic_read(&tui_state) == TUI_STATE_RUNNING
		&& !timer_pending(&tui_tick_timer)) {
		expires = jiffies + msecs_to_jiffies(TUI_TICK_RATE);
		tui_tick_timer.expires = expires;
		add_timer(&tui_tick_timer);
		tui_dbg("start tick timer(%dms)\n", TUI_TICK_RATE);
	}
}

static void tui_timeout_start(void)
{
	unsigned long expires;

	tui_dbg("tui_timeout_start----------\n");
	expires = msecs_to_jiffies(TUI_DEFAULT_TIMEOUT);
	if (atomic_read(&tui_state) == TUI_STATE_RUNNING) {
		tui_dbg("come int delay work expire = %llu\n",expires);
		if(!delayed_work_pending(&tui_timeout_work)){
			queue_delayed_work(system_wq, &tui_timeout_work, expires);
			tui_dbg("start timeout work timer(%dms)\n", TUI_DEFAULT_TIMEOUT);
		}else{
			mod_delayed_work(system_wq, &tui_timeout_work, expires);
			tui_dbg("reset timeout work timer(%dms)\n", TUI_DEFAULT_TIMEOUT);
		}
	}
}

static void tui_timeout_end(void)
{
	tui_dbg("tui_timeout_end----------\n");
	if(delayed_work_pending(&tui_timeout_work)){
		cancel_delayed_work_sync(&tui_timeout_work);
		flush_scheduled_work();
	}
}

static void tui_delay_work_func(struct work_struct *work)
{
    tui_send_event(TUI_POLL_DELAYED_WORK);
}
static void tui_timeout_work_func(struct work_struct *work)
{
    tui_send_event(TUI_POLL_CANCEL);
}

static void tui_delay_work_start(void)
{
	unsigned long expires;

	tui_dbg("tui_delay_work_start----------\n");
	expires = msecs_to_jiffies(TUI_DELAY_WORK_TIME);
	if (atomic_read(&tui_state) == TUI_STATE_RUNNING) {
		tui_dbg("come int delay work expire = %llu\n",expires);
		if(!delayed_work_pending(&tui_delay_work)){
			queue_delayed_work(system_wq, &tui_delay_work, expires);
			tui_dbg("start delay work timer(%dms)\n", TUI_DELAY_WORK_TIME);
		}else{
			mod_delayed_work(system_wq, &tui_delay_work, expires);
			tui_dbg("reset delay work timer(%dms)\n", TUI_DELAY_WORK_TIME);
		}
	}
}

static void tui_poweroff_work_func(struct work_struct *work)
{
    tui_send_event(TUI_POLL_CANCEL);
}

void tui_poweroff_work_start(void)
{
       tui_dbg("tui_poweroff_work_start----------\n");

	   if (atomic_read(&tui_state)  !=  TUI_STATE_UNUSED && dss_fd->panel_power_on) {
		   tui_dbg("come in tui_poweroff_work_start state=%d--\n",atomic_read(&tui_state));
		   queue_work(system_wq, &tui_poweroff_work);
	   }
}

static void tui_delay_work_cancel(void)
{
       tui_dbg("tui_delay_work_cancel----------\n");
       tui_timeout_start();
}


static void wait_tui_msg(void)
{
	int r;

	tui_tick_start();
	tui_timeout_start();
	r = wait_event_interruptible(tui_msg_wq, tui_msg_flag);
	if (r)
		tui_err("get tui state is interrupted\n");
}

static int valid_msg(int msg_type)
{
	switch (msg_type) {
	case TUI_POLL_RESUME_TUI:
		if (atomic_read(&tui_state) == TUI_STATE_RUNNING)
			return 0;
		break;
	case TUI_POLL_CANCEL:
		if (atomic_read(&tui_state) == TUI_STATE_UNUSED)
			return 0;
		break;
	default:
		break;
	}

	return 1;
}

static void process_tui_msg(void)
{
	struct tui_msg_node *tui_msg;

fetch_msg:
	spin_lock(&tui_msg_lock);
	while (list_empty(&tui_msg_head)) {
		tui_msg_flag = 0;
		spin_unlock(&tui_msg_lock);
		tui_dbg("wait step first-----\n");
		wait_tui_msg();
		tui_dbg("wait step second-----\n");
		spin_lock(&tui_msg_lock);
	}

	tui_msg = list_first_entry(&tui_msg_head, struct tui_msg_node, list);

	list_del(tui_msg_head.next);
	spin_unlock(&tui_msg_lock);

	tui_dbg("process tui type(%s) msg\n",
			poll_event_type_name[tui_msg->type]);

	/* pre-process tui poll event if needed */
	switch (tui_msg->type) {
	case TUI_POLL_RESUME_TUI:
		if (atomic_read(&tui_state) == TUI_STATE_CONFIG)
			if (init_tui_driver(1))
				tui_ctl->s2n.ret = -1;
	case TUI_POLL_CFG_OK:
		tui_msg->val = get_frame_addr(1080*1920*4*2);
		break;
	default:
		break;
	}

	tui_ctl->n2s.event_type = tui_msg->type;
	tui_ctl->n2s.value = tui_msg->val;

	kfree(tui_msg);

	if (!valid_msg(tui_ctl->n2s.event_type)) {
		tui_dbg("refetch tui msg\n");
		goto fetch_msg;
	}
}

#define CMA_MALLOC
static int init_tui_agent(void)
{
	TC_NS_Shared_MEM tui_tc_shm = {0};
	int ret;

	tui_ctl = kzalloc(sizeof(*tui_ctl), GFP_KERNEL);
	if (!tui_ctl) {
		tui_err("tui control alloc failed\n");
		return -ENOMEM;
	}

	tui_tc_shm.kernel_addr = tui_ctl;
	tui_tc_shm.len = sizeof(*tui_ctl);

	ret = TC_NS_register_agent(NULL, TEE_TUI_AGENT_ID, &tui_tc_shm);
	if (ret)
		tui_err("register tui agent failed\n");

#ifdef CMA_MALLOC
       tui_client = hisi_ion_client_create(ion_name);
	if(!tui_client)
		tui_err("create ion client failed ret=%d \n",tui_client);
#endif

	return ret;
}

static void exit_tui_agent(void)
{
	if (TC_NS_unregister_agent(TEE_TUI_AGENT_ID))
		tui_err("unregister tui agent failed\n");
	kfree(tui_ctl);
#ifdef CMA_MALLOC
	if(tui_client)
	       ion_client_destroy(tui_client);
#endif
}

static void init_tui_timeout(unsigned int timeout)
{
	tui_timeout = timeout ? timeout : TUI_DEFAULT_TIMEOUT;
}

static void set_tui_state(int state)
{
	if (atomic_read(&tui_state) != state) {
		atomic_set(&tui_state, state);
		tui_state_flag = 1;
		if (atomic_read(&tui_state) == TUI_STATE_RUNNING)
                  tui_timeout_start();
		wake_up(&tui_state_wq);
	}
}

static int do_tui_work(void)
{
	int ret = 0;

	/* clear s2n cmd ret */
	tui_ctl->s2n.ret = 0;
	switch (tui_ctl->s2n.command) {
	case TUI_CMD_ENABLE:
		if (atomic_read(&tui_state) != TUI_STATE_CONFIG) {
			tui_dbg("tui enable\n");
			set_tui_state(TUI_STATE_CONFIG);
			init_tui_timeout(tui_ctl->s2n.value);
			if (init_tui_driver(1)){
				tui_ctl->s2n.ret = -1;
				set_tui_state(TUI_STATE_ERROR);
			}
		}
		break;
	case TUI_CMD_DISABLE:
		if (atomic_read(&tui_state) != TUI_STATE_UNUSED) {
			tui_dbg("tui disable\n");
			del_timer(&tui_tick_timer);
			del_timer(&tui_timeout_timer);
			del_timer(&tui_delay_work_timer);
			init_tui_timeout(0);
			init_tui_driver(0);
			free_frame_addr();
			tui_timeout_end();
			set_tui_state(TUI_STATE_UNUSED);
		}
		break;
	case TUI_CMD_PAUSE:
		if (atomic_read(&tui_state) != TUI_STATE_UNUSED) {
			tui_dbg("tui pause\n");
			del_timer(&tui_tick_timer);
			del_timer(&tui_timeout_timer);
			del_timer(&tui_delay_work_timer);
			init_tui_driver(0);
			set_tui_state(TUI_STATE_CONFIG);
		}
		break;
	case TUI_CMD_POLL:
		process_tui_msg();
		break;
	case TUI_CMD_DO_SYNC:
		//if (tp_sync_func) {
		tui_dbg("enable tp irq cmd\n");
		//	tp_sync_func();
		//	tui_timeout_reset();
		//}
		break;
	case TUI_CMD_SET_STATE:
		tui_dbg("tui set state %d\n", tui_ctl->s2n.value);
		set_tui_state(tui_ctl->s2n.value);
		break;
	case TUI_CMD_START_DELAY_WORK:
		tui_dbg("start delay work\n");
		tui_delay_work_start();
		break;
	case TUI_CMD_CANCEL_DELAY_WORK:
		tui_dbg("cancel delay work\n");
		tui_delay_work_cancel();
		break;
	case TUI_CMD_SEND_INPUT_WORK:
		tui_dbg("send input work\n");
		get_keyCode_flag = 1;
		wake_up(&wait_event_get_keyCode);
		break;
	default:
		ret = -EINVAL;
		tui_err("get error tui command\n");
		break;
	}
	return ret;
}

void set_tui_attach_device(unsigned int id)
{
	tui_attached_device = id;
}

unsigned int tui_attach_device(void)
{
	return tui_attached_device;
}

static int tui_kthread_work_fn(void *data)
{
	int	ret;

	ret = init_tui_agent();
	if (ret)
		return ret;

	while (1) {
		//tui_dbg("tui before sleep  1\n");
		TC_NS_wait_event(TEE_TUI_AGENT_ID);
		//tui_dbg("tui after sleep   2\n");

		if (kthread_should_stop())
			break;

		do_tui_work();

		TC_NS_send_event_reponse(TEE_TUI_AGENT_ID);
	}

	exit_tui_agent();

	return 0;
}

static ssize_t tui_dbg_state_read(struct file *filp, char __user *ubuf,
					size_t cnt, loff_t *ppos)
{
	char buf[128];
	int r;
	struct tui_drv_node *pos;

	r = snprintf(buf, 128, "tui state:%s\n", state_name[atomic_read(&tui_state)]);

	r += snprintf(buf + r, 128 - r, "drv config state:");

	mutex_lock(&tui_drv_lock);
	list_for_each_entry(pos, &tui_drv_head, list)
		r += snprintf(buf + r, 128 - r, "%s-%s,", pos->name,
					pos->state == 1 ? "ok" : "no ok");
	buf[r-1] = '\n';
	mutex_unlock(&tui_drv_lock);

	return simple_read_from_buffer(ubuf, cnt, ppos, buf, r);
}

static const struct file_operations tui_dbg_state_fops = {
	.owner = THIS_MODULE,
	.read  = tui_dbg_state_read,
};

static ssize_t tui_dbg_changable_state_read(struct file *filp, char __user *ubuf,
					size_t cnt, loff_t *ppos)
{
	char buf[32];
	int r;

	tui_state_flag = 0;
	r = wait_event_interruptible(tui_state_wq, tui_state_flag);
	if (r) {
		tui_err("get tui state is interrupted\n");
		return 0;
	}

	r = snprintf(buf, 32, "%s", state_name[atomic_read(&tui_state)]);

	return simple_read_from_buffer(ubuf, cnt, ppos, buf, r);
}

static const struct file_operations tui_dbg_changable_state_fops = {
	.owner = THIS_MODULE,
	.read  = tui_dbg_changable_state_read,
};

static ssize_t tui_dbg_msg_read(struct file *filp, char __user *ubuf,
					size_t cnt, loff_t *ppos)
{
	char buf[512];
	int r;
	int i;
	struct tui_drv_node *pos;

	r = snprintf(buf, 512, "%s", "event format: event_type:val\n"
						   "event type:\n");

	/* event type list */
	for (i = 0; i < TUI_POLL_MAX - 1; i++)
		r += snprintf(buf + r, 512 - r, "%s, ", poll_event_type_name[i]);
	r += snprintf(buf + r, 512 - r, "%s\n", poll_event_type_name[i]);

	/* cfg drv type list */
	r += snprintf(buf + r, 512 - r, "val type for %s or %s:\n",
				poll_event_type_name[TUI_POLL_CFG_OK],
				poll_event_type_name[TUI_POLL_CFG_FAIL]);
	list_for_each_entry(pos, &tui_drv_head, list)
		r += snprintf(buf + r, 512 - r, "%s,", pos->name);
	buf[r-1] = '\n';
	return simple_read_from_buffer(ubuf, cnt, ppos, buf, r);
}

static ssize_t tui_dbg_msg_write(struct file *filp,
		const char __user *ubuf, size_t cnt, loff_t *ppos)
{
	char buf[64];
	int i;
	int event_type = -1;
	char *tokens, *begins;

	if (cnt >= sizeof(buf))
		return -EINVAL;

	if (copy_from_user(&buf, ubuf, cnt))
		return -EFAULT;

	buf[cnt] = 0;

	begins = buf;

	/* event type */
	tokens = strsep(&begins, ":");
	tui_dbg("1: tokens:%s\n", tokens);
	for (i = 0; i < TUI_POLL_MAX; i++) {
		if (!strncmp(tokens, poll_event_type_name[i],
				strlen(poll_event_type_name[i]))) {
			event_type = i;
			break;
		}
	}
	/* drv type */
	tokens = strsep(&begins, ":");
	tui_dbg("2: tokens:%s\n", tokens);

	if (event_type == -1)
		return -EFAULT;
	else {
		enum poll_class class = tui_poll_class(event_type);

		if (class == CLASS_POLL_RUNNING)
			//send_tui_msg_running(event_type, 0, NULL);
			tui_send_event(event_type);
		else if (class == CLASS_POLL_CONFIG)
			send_tui_msg_config(event_type, 0, tokens);
		else
			//send_tui_msg_adapt(event_type, 0, NULL);
			tui_send_event(event_type);
	}

	*ppos += cnt;

	return cnt;
}

static const struct file_operations tui_dbg_msg_fops = {
	.owner = THIS_MODULE,
	.read  = tui_dbg_msg_read,
	.write = tui_dbg_msg_write,
};

static struct dentry *dbg_dentry;

int __init init_tui(void)
{
	struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };

	tui_task =
		kthread_create(tui_kthread_work_fn, NULL, "tuid");
	if (IS_ERR(tui_task))
		return PTR_ERR(tui_task);

	sched_setscheduler_nocheck(tui_task, SCHED_FIFO, &param);
	get_task_struct(tui_task);

	wake_up_process(tui_task);

	INIT_LIST_HEAD(&tui_msg_head);
	spin_lock_init(&tui_msg_lock);

	init_waitqueue_head(&tui_state_wq);
	init_waitqueue_head(&tui_msg_wq);
    init_waitqueue_head(&wait_event_get_keyCode);
	dbg_dentry = debugfs_create_dir("tui", NULL);
	debugfs_create_file("message", 0440, dbg_dentry,
		NULL, &tui_dbg_msg_fops);
	debugfs_create_file("d_state", 0440, dbg_dentry,
		NULL, &tui_dbg_state_fops);
	debugfs_create_file("c_state", 0440, dbg_dentry,
		NULL, &tui_dbg_changable_state_fops);

	return 0;
}

void tui_exit(void)
{
	kthread_stop(tui_task);
	put_task_struct(tui_task);
	debugfs_remove(dbg_dentry);
}
