

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/atphooks.h>
#include "atp_bridge.h"

#ifdef CONFIG_IGMP_SNOOPING
extern void br_igmp_snooping_show(void);
extern void br_igmp_snooping_set_enable(int enable);
#endif
extern int atp_register_hook(struct atp_hook_ops* reg);
extern void atp_unregister_hook(struct atp_hook_ops* reg);


static int atp_br_ioctl_proc(unsigned int hooknum,
                             void* pvnet,
                             void* pvargs,
                             void* arg3)
{
    unsigned long args[3];

    if (unlikely(NULL == pvargs))
    {
        return -EOPNOTSUPP;
    }

    memcpy(args, pvargs, sizeof(args));
    ATP_BRIDGE_IOCTL_DEBUG("get in atp br ioctl, args[0]: [%x]\n", args[0]);

    switch (args[0])
    {
#ifdef CONFIG_IGMP_SNOOPING
        case BRCTL_SHOW_IGMP_SNOOPING:
        {
            br_igmp_snooping_show();
            return 0;
        }
        case BRCTL_SET_IGMP_SNOOPING:
        {
            if (!capable(CAP_NET_ADMIN))
            { return -EPERM; }
            ATP_BRIDGE_IOCTL_DEBUG("for igmp snooping, value [%d]\n", (int)args[1]);
            br_igmp_snooping_set_enable((int)args[1]);
            return 0;
        }
#endif
    }
    return -EOPNOTSUPP;
}


static struct atp_hook_ops atp_br_ioctl =
{
    .hook = atp_br_ioctl_proc,
    .hooknum = ATP_BR_IOCTL_HOOK,
    .priority = ATP_PRI_DEFAULT,
};


static int __init atp_br_ioctl_init(void)
{
    int ret = 0;

    ret = atp_register_hook(&atp_br_ioctl);
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}


static void __exit atp_br_ioctl_fini(void)
{
    atp_unregister_hook(&atp_br_ioctl);
    return;
}

module_init(atp_br_ioctl_init);
module_exit(atp_br_ioctl_fini);

