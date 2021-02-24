

#ifndef __BSP_ACM_H__
#define __BSP_ACM_H__


#include <linux/types.h>

typedef void (*USB_UDI_ENABLE_CB_T)(void);
typedef void (*USB_UDI_DISABLE_CB_T)(void);
void *bsp_acm_open(u32 dev_id);
s32 bsp_acm_close(void *handle);
s32 bsp_acm_write(void *handle, void *buf, u32 size);
s32 bsp_acm_read(void *handle, void *buf, u32 size);
s32 bsp_acm_ioctl(void *handle, u32 cmd, void *para);
unsigned int BSP_USB_RegUdiEnableCB(USB_UDI_ENABLE_CB_T pFunc);
unsigned int BSP_USB_RegUdiDisableCB(USB_UDI_DISABLE_CB_T pFunc);

#endif    /* End of __BSP_ACM_H__ */
