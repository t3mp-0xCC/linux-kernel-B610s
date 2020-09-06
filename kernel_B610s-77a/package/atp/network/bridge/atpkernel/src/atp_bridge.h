

#ifndef __ATP_BRIDGE_H__
#define __ATP_BRIDGE_H__

#ifdef DEBUG_ATP_BRIDGE
#define ATP_BRIDGE_DEBUG(args...)   color_printk(__func__, __LINE__, _RET_IP_, ## args)
#define ATP_BRIDGE_IOCTL_DEBUG(args...)   color_printk(__func__, __LINE__, _RET_IP_, ## args)
#else
#define ATP_BRIDGE_DEBUG(args...)
#define ATP_BRIDGE_IOCTL_DEBUG(args...)
#endif


#endif
