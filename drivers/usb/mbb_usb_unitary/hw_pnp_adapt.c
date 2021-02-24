/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "f_mass_storage_api.h"
#include "usb_hotplug.h"
#include "usb_config.h"
#include <linux/usb/usb_interface_external.h>
#include "hw_pnp.h"
#include "hw_pnp_adapt.h"
#include "hw_pnp_api.h"
#include "usb_smem.h"
#include "usb_nv_get.h"
/*****************************************************************************
ȫ�ֱ�������begin
*****************************************************************************/

USB_INT g_set_adress_flag = SET_ADD_NONE;

static usb_enum_state  g_usb_enum_state    = USB_ENUM_NONE;
/* ���ö�ʱ�������set_config ״̬ */
static struct timer_list g_soft_reconnect_timer ;
/*��λ��л���10sû���յ�set_config���ʱ��*/
static USB_INT g_soft_reconnect_loop = 10000;

/*����3.0 Э��Ҫ������30ms��Ϊ���ź��ȶ���Ĭ����ʱ100ms*/
USB_INT g_soft_delyay = 100;

USB_INT is_in_dload = 0;

USB_INT usb_net_drv_state = NET_STATE_NOT_READY;

/*support fd����*/
static struct android_usb_function* supported_functions[PNP_SUPPORT_FUNC_MAX] =
{
    NULL
};

/*KO api ����*/
usb_pnp_api_hanlder g_pnp_api_hanlder =
{
    NULL,
};

USB_CHAR* g_pnp_port_usable[DYNAMIC_PID_MAX_PORT_NUM] =
{
#ifdef USB_CPE
    "mass",
    "mass_two",
#else
    "rndis",
    "ecm",
    "mass",
    "pcui",
    "modem",
    "mass_two",
    "ncm",
#endif
    NULL
};

/*�ӿ�Э���ֶ�*/
huawei_interface_info g_huawei_interface_vector[MAX_INTERFACE_VECTOR_NUM] =
{
    /*M2M protocol*/
    //#ifdef USB_PORT_NAME_UNITARY    
    {USB_IF_PROTOCOL_COMM_C,        "3g_diag"}, 
    {USB_IF_PROTOCOL_SHEEL_A,       "a_shell"}, 
    {USB_IF_PROTOCOL_SHEEL_B,       "c_shell"},
    {USB_IF_PROTOCOL_COMM_B,            "gps"}, 
    {USB_IF_PROTOCOL_GPS,           "m2m_gps"}, 
    //#endif
    /*ͨ�ð�Protocol*/
    
    {USB_IF_PROTOCOL_VOID,        "mass"},    //����
#ifdef MBB_USB_UNITARY_Q
    {USB_IF_PROTOCOL_3G_MODEM,    "modem"},//modem
    {USB_IF_PROTOCOL_3G_PCUI,     "pcui"}, //PCUI
    {USB_IF_PROTOCOL_3G_DIAG,     "diag"}, //diag
    {USB_IF_PROTOCOL_3G_NDIS,     "rmnet"},    //������
#else
    {USB_IF_PROTOCOL_3G_MODEM,    "3g_modem"},//modem
    {USB_IF_PROTOCOL_3G_PCUI,        "pcui"}, //PCUI
    {USB_IF_PROTOCOL_3G_DIAG,     "3g_diag"}, //diag
    {USB_IF_PROTOCOL_3G_GPS,      "a_shell"}, //shell��
    {USB_IF_PROTOCOL_CTRL,        "gps_ctl"}, //
    {USB_IF_PROTOCOL_3G_NDIS,     "ndis"},    //������
    {USB_IF_PROTOCOL_NDISDATA,    "ndisData"},//������
    {USB_IF_PROTOCOL_NDISCTRL,    "ndisCtrl"},//������
    {USB_IF_PROTOCOL_BLUETOOTH,   "c_shell"}, //chsell
    {USB_IF_PROTOCOL_FINGERPRINT, "finger"},  //ָ��
    {USB_IF_PROTOCOL_ACMCTRL,     "acmCtrl"}, //�ɴ���
    {USB_IF_PROTOCOL_MMS,         "mms"},     //���ſ�
    {USB_IF_PROTOCOL_3G_PCVOICE,  "3g_voice"},//pc����
    {USB_IF_PROTOCOL_DVB,         "dvb"},     //���ӿ�
#endif
    {USB_IF_PROTOCOL_PCSC,        "pcsc"},    //���ܿ�
    {USB_IF_PROTOCOL_MODEM,    "modem"},   //modem��
    {USB_IF_PROTOCOL_PCUI,        "pcui"},    //AT��
#ifdef MBB_USB_UNITARY_Q
    {USB_IF_PROTOCOL_NDIS,        "rmnet"},    //����
    {USB_IF_PROTOCOL_DIAG,        "diag"}, //
    {USB_IF_PROTOCOL_MBIM,        "usb_mbim"},    //mbim
#else
    {USB_IF_PROTOCOL_NDIS,        "ndis"},    //������
    {USB_IF_PROTOCOL_DIAG,        "4g_diag"}, //��Ͽ�
    {USB_IF_PROTOCOL_MBIM,        "ncm"},    //mbim
#endif
    {USB_IF_PROTOCOL_GPS,         "gps"},     //
    {USB_IF_PROTOCOL_PCVOICE,     "voice"},   //pc����
    {USB_IF_PROTOCOL_NCM,         "ncm"},     //��������
    {USB_IF_PROTOCOL_CDROM,       "mass"},    //����
    {USB_IF_PROTOCOL_SDRAM,       "mass_two"}, //SD
    {USB_IF_PROTOCOL_RNDIS,       "rndis"},   //RNDIS����
    {USB_IF_PROTOCOL_ADB,         "adb"},   //ADB
    /*VDF���ư�Protocol*/
#ifdef MBB_USB_UNITARY_Q
    {VDF_USB_IF_PROTOCOL_MODEM,    "modem"},//modem
    {VDF_USB_IF_PROTOCOL_DIAG,     "diag"}, //diag
    {VDF_USB_IF_PROTOCOL_NDIS,     "rmnet"},    //������
#else
    {VDF_USB_IF_PROTOCOL_MODEM,         "3g_modem"},//modem
    {VDF_USB_IF_PROTOCOL_DIAG,          "3g_diag"}, //diag
    {VDF_USB_IF_PROTOCOL_NDIS,          "ndis"},    //������
#endif
    {VDF_USB_IF_PROTOCOL_GPS,           "a_shell"}, //shell��
    {VDF_USB_IF_PROTOCOL_PCUI,          "pcui"}, //PCUI
    {VDF_USB_IF_PROTOCOL_PCSC,          "pcsc"},    //���ܿ�
    {VDF_USB_IF_PROTOCOL_CTRL,          "gps_ctl"}, //
    {VDF_USB_IF_PROTOCOL_NDISDATA,      "ndisData"},//������
    {VDF_USB_IF_PROTOCOL_NDISCTRL,      "ndisCtrl"},//������
    {VDF_USB_IF_PROTOCOL_BLUETOOTH,     "c_shell"}, //chsell
    {VDF_USB_IF_PROTOCOL_FINGERPRINT,   "finger"},  //ָ��
    {VDF_USB_IF_PROTOCOL_ACMCTRL,       "acmCtrl"}, //�ɴ���
    {VDF_USB_IF_PROTOCOL_MMS,           "mms"},     //���ſ�
    {VDF_USB_IF_PROTOCOL_PCVOICE,       "voice"},//pc����
    {VDF_USB_IF_PROTOCOL_DVB,           "dvb"},     //���ӿ�
    {VDF_USB_IF_PROTOCOL_NCM,           "ncm"},     //��������
    {VDF_USB_IF_PROTOCOL_SHEEL_A,       "a_shell"},     //a shell ��
    {VDF_USB_IF_PROTOCOL_SHEEL_B,       "c_shell"},     //a shell ��
    {VDF_USB_IF_PROTOCOL_COMM_A,        "comm_a"},     //ͨ�ô���A
    {VDF_USB_IF_PROTOCOL_COMM_B,        "comm_b"},     //ͨ�ô���B
    {VDF_USB_IF_PROTOCOL_COMM_C,        "comm_c"},     //ͨ�ô���C
    {VDF_USB_IF_PROTOCOL_COMM_GNSS,     "gnss"},     //GNSS�˿ڣ���ԭ��GPS�˿ڣ�

    /*��Huawei ����*/
#ifdef MBB_USB_UNITARY_Q
    {NO_HW_USB_IF_PROTOCOL_DIAG,           "3g_diag"}, //diag
    {NO_HW_USB_IF_PROTOCOL_NDIS,           "rmnet"},    //������
#else
    {NO_HW_USB_IF_PROTOCOL_DIAG,           "3g_diag"}, //diag
    {NO_HW_USB_IF_PROTOCOL_NDIS,           "ndis"},    //������
#endif
    {NO_HW_USB_IF_PROTOCOL_MODEM,          "modem"},   //modem��
    {NO_HW_USB_IF_PROTOCOL_PCUI,           "pcui"}, //PCUI
    {NO_HW_USB_IF_PROTOCOL_PCSC,           "pcsc"},    //���ܿ�
    {NO_HW_USB_IF_PROTOCOL_GPS,            "a_shell"}, //shell��
    {NO_HW_USB_IF_PROTOCOL_CTRL,           "gps_ctl"}, //
    {NO_HW_USB_IF_PROTOCOL_NDISDATA,       "ndisData"},//������
    {NO_HW_USB_IF_PROTOCOL_NDISCTRL,       "ndisCtrl"},//������
    {NO_HW_USB_IF_PROTOCOL_BLUETOOTH,      "c_shell"}, //chsell
    {NO_HW_USB_IF_PROTOCOL_FINGERPRINT,    "finger"},  //ָ��
    {NO_HW_USB_IF_PROTOCOL_ACMCTRL,        "acmCtrl"}, //�ɴ���
    {NO_HW_USB_IF_PROTOCOL_MMS,            "mms"},     //���ſ�
    {NO_HW_USB_IF_PROTOCOL_PCVOICE,        "voice"},//pc����
    {NO_HW_USB_IF_PROTOCOL_DVB,            "dvb"},     //���ӿ�
    /*�����˿����ƴ�3G  ���һ�Ϊ����*/
    {NO_HW_USB_IF_PROTOCOL_3G_PCUI ,       "pcui"},   //modem��
    {NO_HW_USB_IF_PROTOCOL_3G_DIAG,        "3g_diag"},    //������
    {NO_HW_USB_IF_PROTOCOL_3G_GPS,         "a_shell"},    //AT��
    {NO_HW_USB_IF_PROTOCOL_3G_PCVOICE,     "voice"}, //��Ͽ�
    {NO_HW_USB_IF_PROTOCOL_NCM,            "ncm"},     // ����
    {NO_HW_USB_IF_PROTOCOL_SHEEL_A,        "a_shell"},   //a shell ��
    {NO_HW_USB_IF_PROTOCOL_SHEEL_B,        "c_cshell"},     //c shell ��
    {NO_HW_USB_IF_PROTOCOL_COMM_A,         "comm_a"},   //ͨ�ô���A
    {NO_HW_USB_IF_PROTOCOL_COMM_B,         "comm_b"},   //ͨ�ô���A
    {NO_HW_USB_IF_PROTOCOL_COMM_C,         "comm_c"},   //ͨ�ô���C
    {NO_HW_USB_IF_PROTOCOL_COMM_GNSS,      "gnss"},   //GNSS�˿ڣ���ԭ��GPS�˿ڣ�
};
EXPORT_SYMBOL(g_huawei_interface_vector);

/*��PNP �̶��˿���̬����������չ��Ҫ��ports_enum_mode����һ��*/
/*��д�˿����ʱע���ַ���,�Լ��˿�˳��*/
hw_usb_mode g_usb_port_mode_info[MAX_PORT_MODE_NUM] =
{
    {
        CDROM_MODE,
        {"mass"},
        CDROM_SCENE_PID,    USB_CLASS_PER_INTERFACE,
        EXISTS,     NON_EXISTS
    },
    
    {
        POWER_OFF_MODE,
        {"mass"},
        POWER_OFF_CDROM_PID, USB_CLASS_PER_INTERFACE,
        EXISTS,     NON_EXISTS
    },

    {/*RNDIS DOWNLOAD*/
        PCUI_DIAG_MODE,
        {"pcui", "mass"},
        DOWNLOAD_SCENE_PID, USB_CLASS_PER_INTERFACE,    EXISTS, NON_EXISTS
    },
#ifdef MBB_USB_UNITARY_Q
    {
        RNDIS_DEBUG_MODE,
        {"rndis", "diag", "pcui", "adb"},
        RNDIS_DEBUG,        USB_CLASS_PER_INTERFACE,
        NON_EXISTS, NON_EXISTS
    },

    {
        RNDIS_DEBUG_MODE_SD,
        {"rndis", "diag", "pcui", "mass_two"},
        RNDIS_DEBUG,        USB_CLASS_PER_INTERFACE,
        NON_EXISTS, EXISTS
    },

    {
        ECM_DEBUG_MODE,
        {"ecm", "diag", "pcui"},
        RNDIS_DEBUG_PID,    USB_CLASS_COMM,
        NON_EXISTS, NON_EXISTS
    },

    {
        ECM_DEBUG_MODE_SD,
        {"ecm", "diag", "pcui", "mass_two"},
        RNDIS_DEBUG_PID,    USB_CLASS_COMM,
        NON_EXISTS, EXISTS
    },

    {/*e5/stick DOWNLOAD*/
        DIAG_PCUI_MODE_EX,/*mass ��Ҫ����Ϊmodem*/
        {"mass", "diag", "pcui"},
        DOWNLOAD_SCENE_PID_E5, USB_CLASS_PER_INTERFACE,
        EXISTS,     NON_EXISTS
    },

    {/*MBIM DOWNLOAD*/
        MBIM_DOWNLOAD_MODE,
        {"pcui", "mass"},
        MBIM_DOWNLOAD_PID, USB_CLASS_PER_INTERFACE,
        NON_EXISTS, NON_EXISTS
    },

    {
        PCUI_DIAG_MODE_FACTORY,
        {"mass", "diag", "pcui","adb"},/*mass ��Ҫ����Ϊmodem*/
        DOWNLOAD_SCENE_PID_E5, USB_CLASS_PER_INTERFACE,
        EXISTS,     NON_EXISTS
    },

    {
        MBIM_SWITCH_DEBUG_MODE,
        {"usb_mbim", "pcui", "diag", "gps", "adb"},
        MBIM_DEBUG_MODE_PID, USB_CLASS_PER_INTERFACE,
        NON_EXISTS, NON_EXISTS
    },

    {
        MODEM_DIAG_PCUI_GATEWAY,
        {"mass", "diag", "pcui"},/*mass ��Ҫ����Ϊmodem*/
        LOW_LINUX_PID,      USB_CLASS_PER_INTERFACE,
        NON_EXISTS, NON_EXISTS
    },

    {
        MODEM_DIAG_PCUI_NCM_GATEWAY,
        {"modem", "diag", "pcui", "mass", "rnmet"},/*mass ��Ҫ����Ϊmodem*/
        GATEWAY_NDIS_PID,   USB_CLASS_COMM,
        NON_EXISTS, EXISTS
    },

#else
    {
        RNDIS_DEBUG_MODE,
        {"rndis", "pcui", "c_shell", "a_shell", "3g_diag", "gps", "4g_diag"},
        RNDIS_DEBUG_PID,    USB_CLASS_PER_INTERFACE,    NON_EXISTS, NON_EXISTS
    },
    {
        RNDIS_DEBUG_MODE_SD,
        {"rndis", "pcui", "c_shell", "a_shell", "3g_diag", "gps", "4g_diag", "mass_two"},
        RNDIS_DEBUG_PID,    USB_CLASS_PER_INTERFACE,    NON_EXISTS, EXISTS
    },
    {
        ECM_DEBUG_MODE,
        {"ecm", "pcui", "c_shell", "a_shell", "3g_diag", "gps", "4g_diag"},
        RNDIS_DEBUG_PID,    USB_CLASS_COMM,             NON_EXISTS, NON_EXISTS
    },

    {
        ECM_DEBUG_MODE_SD,
        {"ecm", "pcui", "c_shell", "a_shell", "3g_diag", "gps", "4g_diag", "mass_two"},
        RNDIS_DEBUG_PID,    USB_CLASS_COMM,             NON_EXISTS, EXISTS
    },
#ifdef USB_E5/*Ŀǰֻ�޸�E5*/
    {/*e5/stick DOWNLOAD*/
        DIAG_PCUI_MODE_EX,
        {"modem", "mass", "pcui"},
        DOWNLOAD_SCENE_PID_E5, USB_CLASS_PER_INTERFACE,  EXISTS, NON_EXISTS
    },
#else
    {/*e5/stick DOWNLOAD*/
        DIAG_PCUI_MODE_EX,
        {"a_shell", "mass", "pcui"},
        DOWNLOAD_SCENE_PID_E5, USB_CLASS_PER_INTERFACE,  EXISTS, NON_EXISTS
    },
#endif
    {/*MBIM DOWNLOAD*/
        MBIM_DOWNLOAD_MODE,
    #if (FEATURE_ON == MBB_DLOAD)
        {"pcui", "3g_diag"},
    #else
        {"pcui", "mass"},
    #endif
        MBIM_DOWNLOAD_PID, USB_CLASS_PER_INTERFACE,
        NON_EXISTS, NON_EXISTS
    },

    {
        PCUI_DIAG_MODE_FACTORY,
        {"gps", "4g_diag", "pcui"},
        //{"a_shell", "3g_diag", "pcui"},
        DOWNLOAD_SCENE_PID_E5, USB_CLASS_PER_INTERFACE,
        EXISTS,     NON_EXISTS
    },
    {
        MBIM_SWITCH_DEBUG_MODE,
        {"ncm", "pcui", "c_shell", "a_shell", "3g_diag", "gps", "4g_diag"},
        MBIM_DEBUG_MODE_PID, USB_CLASS_PER_INTERFACE,
        NON_EXISTS, NON_EXISTS
    },
    {
        NCM_PCUI_GPS_4GDIAG_3GDIAG_CPE,
        {"ncm", "pcui", "gps", "3g_diag", "4g_diag"},
        PID_USER,           USB_CLASS_PER_INTERFACE,    NON_EXISTS, NON_EXISTS
    },
    {
        MODEM_DIAG_PCUI_GATEWAY,
        {"3g_modem", "3g_diag", "pcui"},
        LOW_LINUX_PID,      USB_CLASS_PER_INTERFACE,
        NON_EXISTS, NON_EXISTS
    },
    {
        MODEM_DIAG_PCUI_NCM_GATEWAY,
        {"3g_modem", "3g_diag", "pcui", "ncm"},
        GATEWAY_NDIS_PID,   USB_CLASS_COMM,
        NON_EXISTS, EXISTS
    },
#endif
};
EXPORT_SYMBOL(g_usb_port_mode_info);


huawei_interface_info *pnp_get_huawei_interface_vector(USB_VOID)
{
   return g_huawei_interface_vector;
}

hw_usb_mode *pnp_get_huawei_usb_mode(USB_VOID)
{
    return g_usb_port_mode_info;
}

USB_INT get_dload_flag(USB_VOID)
{
    return is_in_dload;
}
USB_VOID set_dload_flag(USB_INT flag)
{
    is_in_dload = flag;
}

/*****************************************************************************
ȫ�ֱ�������end
*****************************************************************************/
/*****************************************************************
Parameters    :  usb_func
Return        :   MBB_USB_NOδͨ����Ȩ���޷�ע���pnp
                  MBB_USB_YES ͨ����Ȩ������ע���pnp
Description   :  ����pnp֧�ֵĶ˿ڼ�Ȩ��ֻ��֧�ֵĶ˿ڲ���ע�ᵽpnp����ͨ��NV��
                 ��ʽ�������á�
*****************************************************************/
USB_INT pnp_usb_port_authentication (struct android_usb_function* usb_func)
{
    USB_INT ret = MBB_USB_NO;

#if (defined(USB_E5)) || (defined(USB_PORT_AUTH))/*�޸�E5��KD11*/
#ifdef USB_PORT_AUTHENTICATION
    USB_UINT index = 0;
    USB_UINT usable_name_len = 0;
    if (NULL == usb_func->name)
    {
        DBG_E(MBB_PNP, "empty func name\n");
        return ret;
    }
    for (index = 0; index < DYNAMIC_PID_MAX_PORT_NUM; index++)
    {
        if (NULL == g_pnp_port_usable[index])
        {
            break;
        }
        
        usable_name_len = strlen(g_pnp_port_usable[index]);
        
        if (!strncmp(usb_func->name,g_pnp_port_usable[index], usable_name_len))
        {
            ret = MBB_USB_YES;
            break;
        }
    }
#endif/*USB_PORT_AUTHENTICATION*/
#else
    ret = MBB_USB_YES;
#endif/*USB_E5��USB_KD11*/
    return ret;
}

/*****************************************************************
Parameters    :  usb_func
Return        :
Description   :  ��FD��pnp֧���б�ע��Ľӿ�
*****************************************************************/
USB_VOID pnp_register_usb_support_function(struct android_usb_function* usb_func)
{
    USB_UINT index = 0;
    USB_INT port_usable = MBB_USB_NO;
    
    if (NULL == usb_func)
    {
        DBG_E(MBB_PNP, "empty func\n");
        return;
    }
#if (defined(USB_E5)) || (defined(USB_PORT_AUTH))/*�޸�E5��KD11*/
#if (FEATURE_OFF == MBB_FACTORY)
#ifdef USB_PORT_AUTHENTICATION
    DBG_I(MBB_PNP, "pnp_usb_port_authentication\n");
    port_usable = pnp_usb_port_authentication(usb_func);
    if (MBB_USB_NO == port_usable)
    {
        DBG_E(MBB_PNP, "port'%s' authenticate failed can not be used\n",usb_func->name);
        return;
    }
#endif
#endif
#endif
    for (index = 0; index < PNP_SUPPORT_FUNC_MAX; index++)
    {
        if (usb_func == supported_functions[index])
        {
            DBG_I(MBB_PNP, "already reg '%s' at supported_functions[%u]\n", usb_func->name, index);
            break;
        }
        
        if (NULL == supported_functions[index])
        {
            supported_functions[index] = usb_func;
            DBG_I(MBB_PNP, "reg '%s' at supported_functions[%u]\n", usb_func->name, index);
            break;
        }
    }

    if ( PNP_SUPPORT_FUNC_MAX == index )
    {
        DBG_W(MBB_PNP, "list is full\n");
    }
}
EXPORT_SYMBOL_GPL(pnp_register_usb_support_function);

/*****************************************************************
Parameters    :  functions
Return        :
Description   :  �����FD��Ϣ
*****************************************************************/
void android_cleanup_functions(struct android_usb_function** functions)
{
    struct android_usb_function* f;
    struct device_attribute** attrs;
    struct device_attribute* attr;
    struct class* dev_class = android_get_android_class();

    if( NULL == dev_class)
    {
        DBG_E(MBB_PNP, "dev_class NULL!\n");
        return ;
    }
    
    while (*functions)
    {
        f = *functions++;

        if (f->dev)
        {
            attrs = f->attributes;

            if (attrs)
            {
                while ((attr = *attrs++))
                { device_remove_file(f->dev, attr); }
            }
            device_destroy(dev_class, f->dev->devt);
            kfree(f->dev_name);
        }
        else
        { continue; }

        if (f->cleanup)
        { f->cleanup(f); }
    }
#ifdef MBB_USB_UNITARY_Q
#else
    gether_cleanup();
    gacm_cleanup();
#endif
}
EXPORT_SYMBOL_GPL(android_cleanup_functions);

/*****************************************************************
Parameters    :  data
Return        :
Description   :  androidĬ�ϵı�uevent��Ϣ�ĵط�
*****************************************************************/
static void android_work(struct work_struct* data)
{
    struct android_dev* dev = container_of(data, struct android_dev, work);
    struct usb_composite_dev* cdev = dev->cdev;
    char* disconnected[2] = { "USB_STATE=DISCONNECTED", NULL };
    char* connected[2]    = { "USB_STATE=CONNECTED", NULL };
    char* configured[2]   = { "USB_STATE=CONFIGURED", NULL };
    char* suspended[2]   = { "USB_STATE=SUSPENDED", NULL };
    char* resumed[2]   = { "USB_STATE=RESUMED", NULL };
    char** uevent_envp = NULL;
    static enum android_device_state last_uevent, next_state;
    unsigned long flags;
    int pm_qos_vote = -1;
    spin_lock_irqsave(&cdev->lock, flags);

    if (dev->suspended != dev->sw_suspended && cdev->config)
    {
        if (strncmp(dev->pm_qos, "low", 3))
        { pm_qos_vote = dev->suspended ? 0 : 1; }

        next_state = dev->suspended ? USB_SUSPENDED : USB_RESUMED;
        uevent_envp = dev->suspended ? suspended : resumed;
    }
    else if (cdev->config)
    {
        uevent_envp = configured;
        next_state = USB_CONFIGURED;
    }
    else if (dev->connected != dev->sw_connected)
    {
        uevent_envp = dev->connected ? connected : disconnected;
        next_state = dev->connected ? USB_CONNECTED : USB_DISCONNECTED;

        if (dev->connected && strncmp(dev->pm_qos, "low", 3))
        { pm_qos_vote = 1; }
        else if (!dev->connected || !strncmp(dev->pm_qos, "low", 3))
        { pm_qos_vote = 0; }
    }

    dev->sw_connected = dev->connected;
    dev->sw_suspended = dev->suspended;
    spin_unlock_irqrestore(&cdev->lock, flags);
    DBG_I(MBB_PNP, "pm_qos_vote: %d\n", pm_qos_vote);

    if (uevent_envp)
    {
        /*
         * Some userspace modules, e.g. MTP, work correctly only if
         * CONFIGURED uevent is preceded by DISCONNECT uevent.
         * Check if we missed sending out a DISCONNECT uevent. This can
         * happen if host PC resets and configures device really quick.
         */
        if (((uevent_envp == connected) &&
             (last_uevent != USB_DISCONNECTED)) ||
            ((uevent_envp == configured) &&
             (last_uevent == USB_CONFIGURED)))
        {
            pr_info("%s: sent missed DISCONNECT event\n", __func__);
            kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE,
                               disconnected);
            msleep(20);
        }

        /*
         * Before sending out CONFIGURED uevent give function drivers
         * a chance to wakeup userspace threads and notify disconnect
         */
        if (uevent_envp == configured)
        { msleep(50); }

        /* Do not notify on suspend / resume */
        if (next_state != USB_SUSPENDED && next_state != USB_RESUMED)
        {

            kobject_uevent_env(&dev->dev->kobj, KOBJ_CHANGE,
                               uevent_envp);
            last_uevent = next_state;
        }

        pr_info("%s: sent uevent %s\n", __func__, uevent_envp[0]);
    }
    else
    {
        pr_info("%s: did not send uevent (%d %d %p)\n", __func__,
                dev->connected, dev->sw_connected, cdev->config);
    }

}

ssize_t enable_store(struct device* pdev, struct device_attribute* attr,
                            const char* buff, size_t size)
{
    struct android_dev* dev = dev_get_drvdata(pdev);
    struct usb_composite_dev* cdev = dev->cdev;
    struct android_usb_function* f;
    struct android_configuration* conf;
    USB_INT vbus_status = 0;
    usb_hotplug_api_t *usb_hotplug_api = usb_get_usb_hotplug_api();
    usb_pnp_api_hanlder *pnp_api_handler = pnp_adp_get_api_handler();
    
    int enabled = 0;
    bool audio_enabled = false;
#ifdef MBB_USB_UNITARY_Q
    static DEFINE_RATELIMIT_STATE(rl, 10 * HZ, 1);
#endif

    if (!cdev)
    { return -ENODEV; }

    mutex_lock(&dev->mutex);
    sscanf(buff, "%d", &enabled);

    if (enabled && !dev->enabled)
    {
        list_for_each_entry(conf, &dev->android_configs, list_item)
        list_for_each_entry(f, &conf->enabled_functions,
                            enabled_list)
        {
            if (f->enable)
            { f->enable(f); }

            if (!strncmp(f->name,
                         "audio_source", 12))
            { audio_enabled = true; }
        }

        if (audio_enabled)
        { msleep(100); }

        mutex_unlock(&dev->mutex);

        dev->irq_ctl_port = MBB_USB_ENABLE;
        
        if(NULL != usb_hotplug_api->get_hotplug_status_cb)
        {
            vbus_status = usb_hotplug_api->get_hotplug_status_cb();
            if(vbus_status)
            {
                if(pnp_api_handler->pnp_probe_cb)
                {
                    pnp_api_handler->pnp_probe_cb();
                }
                else
                {
                    DBG_I(MBB_PNP, " pnp_api_handler->pnp_probe_cb not exsit!\n");
                }
            }
        }
        else
        {
            DBG_I(MBB_PNP,"usb_hotplug_api->get_hotplug_status_cb NOT EXIT !\n");
        }
        mutex_lock(&dev->mutex);
    }
    else if (!enabled && dev->enabled)
    {
        if(pnp_api_handler->android_disable_cb)
        {
            pnp_api_handler->android_disable_cb(dev);
        }
        else
        {
            DBG_I(MBB_PNP,"android_disable_cb NOT EXIT !\n");
        }

        list_for_each_entry(conf, &dev->android_configs, list_item)
        list_for_each_entry(f, &conf->enabled_functions,
                            enabled_list)
        {
            if (f->disable)
            { f->disable(f); }
        }
        dev->enabled = false;
    }

#ifdef MBB_USB_UNITARY_Q
    else if (__ratelimit(&rl))
#else
    else
#endif
    {
        pr_err("android_usb: already %s\n",
               dev->enabled ? "enabled" : "disabled");
    }

    mutex_unlock(&dev->mutex);
    return size;
}
EXPORT_SYMBOL_GPL(enable_store);

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ���sn������Ϣ
*****************************************************************/
USB_VOID pnp_check_daynamic_sn_flag(USB_VOID)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    usb_pnp_api_hanlder *pnp_api_handler = pnp_adp_get_api_handler();
    huawei_dynamic_info_st* pnp_info = NULL;

    if(pnp_api_handler->pnp_get_pnp_info_cb)
    {
        pnp_info = pnp_api_handler->pnp_get_pnp_info_cb();
    }
    else
    {
        DBG_I(MBB_PNP, " pnp is not insmod!\n");
        return;
    }
#ifdef MBB_USB_UNITARY_Q
    /* �л�ǰSN �̶�. NV�����л���SN*/
    if (( 0 == usb_nv_info->stUSBNvSnSupp.sn_enable))
    {
        //null sn win8 unknown device
        DBG_I(MBB_PNP, " auto sn\n");
        pnp_info->is_daynamic_sn = 0x01;
    }
#else
    /* �л�ǰSN �̶�. NV�����л���SN*/
    if (( 0 == usb_nv_info->stUSBNvSnSupp.usbSnNvStatus))
    {
        //null sn win8 unknown device
        DBG_T(MBB_PNP, " auto sn\n");
        pnp_info->is_daynamic_sn = 0x01;
    }
#endif
}
EXPORT_SYMBOL_GPL(pnp_check_daynamic_sn_flag);

USB_VOID pnp_accessory_disconnect(USB_VOID)
{
#ifdef MBB_USB_UNITARY_Q
    /* accessory HID support can be active while the
       accessory function is not actually enabled,
       so we need to inform it when we are disconnected.
     */
    acc_disconnect();
#endif
}
EXPORT_SYMBOL_GPL(pnp_accessory_disconnect);


/*****************************************************************
Parameters    :  USB_VOID
Return        :    
Description   :  ����Ƿ����ģʽ
*****************************************************************/
USB_INT pnp_check_factory(USB_VOID)
{
    USB_INT ret = MBB_USB_NO;
    /*��һ���޸ģ�����ģʽ����Ҳʹ��50091��50071���䣬���ж˿����ú�PID����
     *�˴����ܲ�����Ч
     *NV���ù���:
     *50091�л�ǰ�˿�����ΪFF���л�������Ϊ����Ĭ�ϵ���Ƭ�˿���̬��
     *50071����Ϊ 1C05
     */
#ifdef MBB_USB_UNITARY_Q
    /*��ͨƽ̨��Ƭ�汾*/
#ifdef MBB_FACTORY_FEATURE
    ret = MBB_USB_YES;
#endif/*MBB_FACTORY_FEATURE*/

#else/*not MBB_USB_UNITARY_Q*/
    /*balongƽ̨��Ƭ�汾*/
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();

    if (0 == usb_nv_info->factory_mode)
    {
        ret = MBB_USB_YES;
    }
#endif/*MBB_USB_UNITARY_Q*/
    return ret;
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ����Ƿ����ģʽ
*****************************************************************/
USB_INT pnp_status_check_factory(USB_VOID)
{
    return PNP_CHECK_DEVICE_STATUS_CONTINUE;
	/*��һ���޸ģ�����ģʽ����Ҳʹ��50091��50071���䣬���ж˿����ú�PID����
     *�˴����ܲ�����Ч
     *NV���ù���:
     *50091�л�ǰ�˿�����ΪFF���л�������Ϊ����Ĭ�ϵ���Ƭ�˿���̬��
     *50071����Ϊ 1C05
     */
}
EXPORT_SYMBOL_GPL(pnp_status_check_factory);

/*****************************************************************
Parameters    :  cdev
Return        :
Description   :  ��ʼ���豸������
*****************************************************************/
USB_VOID pnp_init_device_descriptor(struct usb_composite_dev* cdev)
{
    mbb_usb_nv_info_st* usb_nv_info = usb_nv_get_ctx();
    /*config cdev desc*/
    cdev->desc.bLength          = sizeof(struct usb_device_descriptor);
    cdev->desc.bDescriptorType  = USB_DT_DEVICE;
    cdev->desc.bcdUSB           = __constant_cpu_to_le16(0x0200);
    cdev->desc.bcdDevice        = USB_VENDOR_BCDDEVICE;
    cdev->desc.bDeviceSubClass  = 0;
    /*idVendor NV�ɶ��ƣ�Ĭ��Ϊ12D1(��Ϊ����)*/
    cdev->desc.idVendor = usb_nv_info->dynamic_vid_info.dynamic_vid;
    /*��������Ĭ��ֵPNP��������*/
    cdev->desc.idProduct        = __constant_cpu_to_le16(CDROM_SCENE_PID);
    cdev->desc.bDeviceClass     = USB_CLASS_PER_INTERFACE;
    cdev->desc.bDeviceProtocol  = 0;
}
EXPORT_SYMBOL_GPL(pnp_init_device_descriptor);

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ��ʼ��ö��״̬,balongƽ̨��Ҫʵ�ִ˽ӿ�
*****************************************************************/
USB_VOID pnp_usb_init_enum_stat(USB_VOID)
{

#ifdef MBB_USB_UNITARY_Q
#else
    bsp_usb_init_enum_stat();
#endif
}
EXPORT_SYMBOL_GPL(pnp_usb_init_enum_stat);


/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ����reconnect_timer�ĵ���ʱ��
*****************************************************************/
USB_VOID pnp_mod_soft_reconnect_timer(USB_VOID)
{
    mod_timer(&g_soft_reconnect_timer, jiffies + msecs_to_jiffies(g_soft_reconnect_loop));
}
EXPORT_SYMBOL_GPL(pnp_mod_soft_reconnect_timer);

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ɾ��֮ǰ���õ�reconnect_timer
*****************************************************************/
USB_VOID pnp_del_soft_reconnect_timer(USB_VOID)
{
    del_timer_sync(&g_soft_reconnect_timer);
}

#ifdef USB_DLOAD
/******************************************************************************
*Function:           dload_report_port_status
*Description:        ��usbģ���ϱ��˿�״ֵ̬
*Calls:              
*Input:              
*Output:             NA
*Return:             0: normal, 1: ����pcui�� 2: ����ģʽ����, 3: nv�ָ�ģʽ����
*Others:             NA
******************************************************************************/

extern int dload_report_port_status(void);
#else
static int dload_report_port_status(void)
{
    DBG_T(MBB_PNP, "no USB_DLOAD normal mode\n");
    return 0;
}
#endif


USB_INT pnp_get_dload_flag(USB_VOID)
{
    int dload_flag = PORT_NORMAL;
#ifdef MBB_USB_UNITARY_Q
    struct android_dev* dev = android_get_android_dev();
#endif
    dload_flag = dload_report_port_status();
    /*һ������NV���ݻָ��׶���Ҫ����PCUI�ڣ���Q�˶Խ�*/
    if ( PORT_NV_RES == dload_flag)
    {
        DBG_T(MBB_PNP, "PORT_NV_RES MODE\n");
#ifdef MBB_USB_UNITARY_Q
        memset(serial_transports, 0, 32);
        strncpy(serial_transports, "smd", 32 - 1);
#endif
    }
    /*һ��������Ҫ����PCUI�ڣ���A7Ӧ�öԽ�*/
    if ( PORT_DLOAD == dload_flag)
    {
        set_dload_flag(1);
        DBG_T(MBB_PNP, "PORT_DLOAD MODE\n");
#ifdef MBB_USB_UNITARY_Q
        memset(serial_transports, 0, 32);
        strncpy(serial_transports, "tty", 32 - 1);
        dwc3_gadget_set_timeout(USB_NUM_1);
        dev->irq_ctl_port = MBB_USB_ENABLE;
#endif
    }

    return dload_flag;

}

/*****************************************************************
Parameters    :  ��
Return        :    1: �ػ�  0:����ػ�
Description   :  ��ȡ��ػ�״̬
*****************************************************************/
USB_INT usb_power_off_chg_stat(USB_VOID)
{
    int pwr_off_chg = MBB_USB_NO;
#ifdef MBB_USB_UNITARY_Q
    huawei_smem_info* smem_data = NULL;

    smem_data = usb_get_smem_info();
    
    if (NULL == smem_data)
    {
        DBG_E(MBB_PNP, "get smem_data error\n");
        return pwr_off_chg;
    }
    
    if ( POWER_OFF_CHARGER == smem_data->smem_huawei_poweroff_chg )
#else
    if( DRV_START_MODE_CHARGING == bsp_start_mode_get())
#endif
    {
        DBG_T(MBB_PNP, "The power off status\n");
        pwr_off_chg = MBB_USB_YES;
    }
    return pwr_off_chg;
}
EXPORT_SYMBOL(usb_power_off_chg_stat);


/*****************************************************************
 ����Ϊusb��������
*****************************************************************/

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ��ȡ�Ƿ�����ģʽ
*****************************************************************/
USB_INT pnp_status_check_charge_only(USB_VOID)
{
    USB_INT fd = 0;
    usb_pnp_api_hanlder *pnp_api_handler = pnp_adp_get_api_handler();
    huawei_dynamic_info_st* pnp_info = NULL;
    mm_segment_t fs;
    char charge_mode = USER_MODE;

    if(pnp_api_handler->pnp_get_pnp_info_cb)
    {
        pnp_info = pnp_api_handler->pnp_get_pnp_info_cb();
    }
    else
    {
        DBG_I(MBB_PNP, " pnp is not insmod!\n");
        return PNP_CHECK_DEVICE_STATUS_CONTINUE;
    }
    
    fs = get_fs();
    set_fs(KERNEL_DS);
    fd = sys_open(ONLY_CHARGE_PATH, O_RDWR, 0);
    if (fd >= 0)
    {
        sys_read(fd, &charge_mode, 1);
        DBG_I(MBB_PNP, "only_charge_mode %c.\n", charge_mode);
    }
    sys_close(fd);
    set_fs(fs);
    if (ONLY_CHARGE_MODE == charge_mode)
    {
        DBG_T(MBB_PNP, " only_charge_mode = %c.\n", charge_mode);
        pnp_info->current_port_style   = charge_port_style;
        /*����籨�ػ����ڣ����̲��ɶ�*/
        pnp_info->portModeIndex   = POWER_OFF_MODE;
        DBG_T(MBB_PNP, " COMPLETE\n");
        return PNP_CHECK_DEVICE_STATUS_COMPLETE;
    }
    DBG_I(MBB_PNP, "return CONTINUE\n");
    return PNP_CHECK_DEVICE_STATUS_CONTINUE;
}
EXPORT_SYMBOL(pnp_status_check_charge_only);

/*****************************************************************
Parameters    :  state
Return        :
Description   :  ����set_adress_flag
*****************************************************************/
USB_VOID huawei_set_adress_flag(USB_INT state)
{
    usb_pnp_api_hanlder *pnp_api_handler = pnp_adp_get_api_handler();
    if(pnp_api_handler->pnp_is_rewind_before_mode_cb )
    {
        if(pnp_api_handler->pnp_is_rewind_before_mode_cb())
        {
            g_set_adress_flag = state;
        }
    }
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ��ȡset_adress_flag
*****************************************************************/
USB_INT huawei_get_adress_flag(USB_VOID)
{
    return   g_set_adress_flag ;
}

/*****************************************************************
Parameters    :  void
Return        :
Description   :  ����USBö��״̬usb_enum_state(���Թ��ĳЩUSB����������)
*****************************************************************/
usb_enum_state huawei_get_usb_enum_state( void )
{
    return   g_usb_enum_state ;
}
EXPORT_SYMBOL_GPL(huawei_get_usb_enum_state);

/*****************************************************************
Parameters    :  state
Return        :
Description   :  ��ȡUSBö��״̬usb_enum_state(���Թ��ĳЩUSB����������)
*****************************************************************/
USB_VOID huawei_set_usb_enum_state(usb_enum_state state)
{
    g_usb_enum_state = state;
}
EXPORT_SYMBOL_GPL(huawei_set_usb_enum_state);

/*****************************************************************
Parameters    :  ��Ҫʹ�ܶ˿ڵ�����
Return        : 0:δʹ��1:ʹ��
Description   :  ��ѯ�˿��Ƿ�ʹ�ܣ��ⲿֱ�ӵ���
*****************************************************************/
#ifdef USB_SECURITY
int usb_port_enable(char *name)
{
#ifdef MBB_USB_UNITARY_Q
    int ret = -1;
    huawei_smem_info* smem_data = NULL;
    
    smem_data = usb_get_smem_info();
    
    if (NULL == smem_data)
    {
        DBG_T(MBB_PNP, " get smem fail\n");
        return ret;
    }

    if (USB_REPORT_MAGIC != smem_data->smem_huawei_usb_port_security)
    {
        DBG_I(MBB_PNP, "The Card is locked, please unlock it .\n");
        ret = -1;
    }
    else
    {
        DBG_I(MBB_PNP, "The Card is unlocked.\n");
        adb_notify_to_work();
        ret = 0;
    }
    return ret;
#else
    mbb_usb_nv_info_st *usb_nv_ctx = usb_nv_get_ctx();

    if (NULL == name)
    {
        DBG_E(MBB_PNP, " name is null\n");
        return 0;
    }
    else if (0 == strcmp(name, "diag"))
    {
        return usb_nv_ctx->usb_security_flag.diag_enable;
    }
    else if (0 == strcmp(name, "shell"))
    {
        return usb_nv_ctx->usb_security_flag.shell_enable;
    }
    else if (0 == strcmp(name, "adb"))
    {
        return usb_nv_ctx->usb_security_flag.adb_enable;
    }
    else if (0 == strcmp(name, "cbt"))
    {
        return usb_nv_ctx->usb_security_flag.cbt_enable;
    }
    else
    {
        return 0;
    }
#endif    
}
#else
int usb_port_enable(char *name)
{
    DBG_I(MBB_PNP, "no lock\n");
    return 0;
}
#endif /* MBB_USB_UNITARY_Q */

/*****************************************************************
Parameters    :  mode_str
Return        :
Description   :  ��ȡ��ǰ������������
*****************************************************************/
void get_support_mode_list_str(char* mode_str)
{
#ifdef MBB_USB_UNITARY_Q
    USB_INT32 support_mode = 0;
    huawei_smem_info* smem_data = NULL;
    smem_data = usb_get_smem_info();
    if (NULL == smem_data)
    {
        DBG_T(MBB_PNP, " get smem fail\n");
        return ;
    }
    else
    {
        support_mode = smem_data->smem_hw_support_mode;
    }
    if(SUPPORT_LTE_ACT & smem_data->smem_hw_support_mode)        /* LTE */
    {
        memcpy((void*)mode_str, "LTE,", LTE_MODE_STRING_LENGTH);
        mode_str += LTE_MODE_STRING_LENGTH;
        support_mode = 1;
    }

    if(SUPPORT_CDMA_ACT & smem_data->smem_hw_support_mode)        /* CDMA */
    {
        memcpy((void*)mode_str, "CDMA,", CDMA_MODE_STRING_LENGTH);
        mode_str += CDMA_MODE_STRING_LENGTH;
        support_mode = 1;
    }

    if((SUPPORT_WCDMA_ACT & smem_data->smem_hw_support_mode)      /* WCDMA or TDSCDMA */ 
        || (SUPPORT_TDSCDMA_ACT & smem_data->smem_hw_support_mode))   
    {
        memcpy((void*)mode_str, "WCDMA,", WCDMA_MODE_STRING_LENGTH);
        mode_str += WCDMA_MODE_STRING_LENGTH;
        support_mode = 1;
    }

    if(SUPPORT_GSM_ACT & smem_data->smem_hw_support_mode)        /* GSM */
    {
        memcpy((void*)mode_str, "GSM,", GSM_MODE_STRING_LENGTH);
        mode_str += GSM_MODE_STRING_LENGTH;
        support_mode = 1;
    }

    if(support_mode)
    {
        *(--mode_str) = '\0';
    }
#endif /* MBB_USB_UNITARY_Q */
}
EXPORT_SYMBOL_GPL(get_support_mode_list_str);

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ��ȡ��ǰUSB����usb_speed_work_mode����
*****************************************************************/
USB_UINT pnp_get_usb_speed(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();
    USB_UINT speed = 0;

    if (NULL == dev || NULL == dev->cdev
        || NULL == dev->cdev->gadget)
    {
        return 0;
    }

    speed = (USB_UINT)dev->cdev->gadget->speed;
    return speed;
}


/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ���ģ�����������Ƴ�����
*****************************************************************/
USB_INT usb_speed_work_mode(USB_VOID)
{
    USB_UINT usb_speed = pnp_get_usb_speed();
    DBG_I(MBB_CHARGER, "usb_speed = 0x%X \n", usb_speed);
    /*if usb3.0 set 900mA,else if invalid usb set 0mA ,else set 500mA*/
    if (USB_SPEED_SUPER == usb_speed)
    {
        DBG_T(MBB_CHARGER, "CHG_CURRENT_SS\n");
        return CHG_CURRENT_SS;
    }
    else if (USB_SPEED_UNKNOWN == usb_speed )
    {
        DBG_T(MBB_CHARGER, "CHG_CURRENT_NO\n");
        return CHG_CURRENT_NO;
    }
    else
    {
        DBG_T(MBB_CHARGER, "CHG_CURRENT_HS\n");
        return CHG_CURRENT_HS;
    }
}

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  �Ƿ�֧��USB 3.0
*****************************************************************/
USB_INT BSP_USB_CapabilityThree( USB_VOID )
{
#ifdef USB_SUPER_SPEED
    return MBB_USB_OK;
#else
    return MBB_USB_ERROR;
#endif
}


/*****************************************************************
Parameters    :  buf
Return        :
Description   :  ��ѯ��ǰUSB�����ʣ����߲鿴USB���ʣ���������ĳЩ����
*****************************************************************/
USB_INT BSP_USB_GetSpeed(USB_UCHAR* buf)
{
    USB_UCHAR    speed;
    USB_INT      ret = MBB_USB_ERROR;

    speed = pnp_get_usb_speed();
    DBG_I(MBB_PNP, "USBMODE:%u\n", speed);
    
    switch (speed)
    {
        case USB_SPEED_UNKNOWN:
            break;

        case USB_SPEED_LOW:
        case USB_SPEED_FULL:
            snprintf(buf, MAX_SPEED_NAME_LEN, "USB1.1");
            ret = MBB_USB_OK;
            break;

        case USB_SPEED_HIGH:
            snprintf(buf, MAX_SPEED_NAME_LEN, "USB2.0");
            ret = MBB_USB_OK;
            break;

        case USB_SPEED_WIRELESS:
            snprintf(buf, MAX_SPEED_NAME_LEN, "USB2.5");
            ret = MBB_USB_OK;
            break;

        case USB_SPEED_SUPER:
            snprintf(buf, MAX_SPEED_NAME_LEN, "USB3.0");
            ret = MBB_USB_OK;
            break;
    }

    return ret;
}

/*****************************************************************
Parameters    :  pucDialmode
                 pucCdcSpec
Return        :   �ɹ�����0��ʧ�ܷ���1
               ucDialmode:  0: Modem����; 1: NDIS����; 2: Modem��NDIS����
               ucCdcSpec:   0: Modem/NDIS��������CDC�淶;
                            1: Modem����CDC�淶;
                            2: NDIS����CDC�淶;
                            3: Modem/NDIS������CDC��
Description   : AT^dialmode����ֵ
*****************************************************************/
USB_INT Query_USB_PortType(USB_UINT* pucDialmode, USB_UINT* pucCdcSpec)
{
    struct android_dev* dev = android_get_android_dev();
    struct usb_composite_dev* cdev = NULL;
    struct usb_configuration*   c = NULL;
    struct android_configuration* conf = NULL;
    struct android_usb_function* f = NULL;

    USB_INT   dail_modem = 0;
    USB_INT   dail_ndis = 0;

    if (NULL == pucDialmode || NULL == pucCdcSpec || NULL == dev)
    {
        DBG_E(MBB_PNP, "NULL point,dev/%p\n",dev);
        return 1;
    }
    
    cdev = dev->cdev;
    c = cdev->config;
    conf = container_of(c, struct android_configuration, usb_config);
    if(NULL == conf)
    {
        DBG_E(MBB_PNP, "conf NULL point\n");
        return 1;
    }
    list_for_each_entry(f, &conf->enabled_functions, enabled_list)   //������ѯ��ǰ�ϱ��Ķ˿���Ϣ
    {
        if ((!strcmp("modem", f->name))
            || (!strcmp("3g_modem", f->name)) )            //�ж��Ƿ��ϱ���MODEM�˿�
        {
            dail_modem = 1;
        }

        if ((!strcmp("rmnet", f->name))     
            || (!strcmp("ncm", f->name)))                //�ж��Ƿ��ϱ���Ndis�˿�
        {
            dail_ndis = 1;
        }
    }

    if ((1 == dail_modem) && (1 == dail_ndis))
    {
        *pucDialmode = DIAL_MODE_MODEM_N_NDIS;   //MODEM ��NDIS���ŷ�ʽ����
        *pucCdcSpec =  CDC_SPEC_MODEM_N_NDIS;    //Modem/NDIS������CDC��
    }
    else if (1 == dail_ndis)
    {
        *pucDialmode = DIAL_MODE_NDIS;  //����NDIS���ŷ�ʽ
        *pucCdcSpec =  CDC_SPEC_NDIS;   //NDIS����CDC�淶
    }
    else if (1 == dail_modem)
    {
        *pucDialmode = DIAL_MODE_MODEM;  //����MODEM���ŷ�ʽ
        *pucCdcSpec =  CDC_SPEC_MODEM;   //Modem����CDC�淶
    }
    else
    {
        *pucCdcSpec = CDC_SPEC_NONE;    // Modem/NDIS��������CDC�淶;
    }
    return 0;
}
EXPORT_SYMBOL_GPL(Query_USB_PortType);

/*****************************************************************
Parameters    :  str
Return        :
Description   :  ��Сдװ��Ϊ��д
*****************************************************************/
void  str_upper(char* str)
{
    unsigned int i = 0;

    if( NULL == str)
    {
        return ;
    }
    
    for(i=0; i<strlen(str); i++) 
    { 
        if((str[i] >= 'a')&&(str[i] <= 'z')) 
        {
            str[i]-=32; /*32Ϊ��д��Сд֮��Ĳ�ֵ*/
        }
    }
    return ;
} 
typedef struct getportname
{
	USB_CHAR* func_name;
	USB_CHAR* get_port_name;
}GET_PORT_TYPE;

GET_PORT_TYPE  get_port_list[]=
{
	{"MASS" ,"CDROM"},
	{"MASS_TWO" ,"SD"},
	{"MODEM","MDM"},
};

/*****************************************************************
Parameters    :  PsBuffer�����ѯ���Ķ˿����Ƽ��˿��ϱ���˳��
                 Length   ��¼*PsBuffer���ַ����ĳ���
Return        :
Description   :  ��ѯ��ǰ�����ϱ��Ķ˿�
*****************************************************************/
USB_INT Check_EnablePortName(USB_CHAR* PsBuffer, USB_ULONG* Length)
{
    struct android_dev* dev = android_get_android_dev();
    struct usb_composite_dev* cdev = NULL;
    struct usb_configuration* c = NULL;
    struct android_configuration* conf = NULL;
    struct android_usb_function* f = NULL;
    USB_UCHAR   PortCount = 0;
    USB_CHAR  func_name[32]={0};
    USB_INT i = 0;
    static USB_CHAR* PortSerialNumber[] =
    {
        "0", "1", "2", "3", "4",
        "5", "6", "7", "8", "9",
        "10", "11", "12", "13",
        "14", "15", "16", "17"
    };
    
    if ( NULL == PsBuffer || NULL == Length || NULL == dev)
    {
        DBG_E(MBB_PNP, "NULL point\n");
        return 1;
    }

    cdev = dev->cdev;
    if (NULL == cdev)
    {
        DBG_E(MBB_PNP,"NULL cdev\n");
        return 1;
    }
    c = cdev->config;
    if (NULL == c)
    {
        DBG_E(MBB_PNP,"NULL config\n");
        return 1;
    }
    conf = container_of(c, struct android_configuration, usb_config);
    
    list_for_each_entry(f, &conf->enabled_functions, enabled_list)
    {
       memset(func_name,0,sizeof(func_name));
       memcpy(func_name , f->name ,sizeof(func_name));
       str_upper(func_name);
       for(i = 0 ; i < sizeof(get_port_list) /sizeof(GET_PORT_TYPE) ;i++)
  	   {
	  		if(!strcmp(get_port_list[i].func_name , func_name))
			{
				memset(func_name, 0 , sizeof(func_name));
				memcpy(func_name , get_port_list[i].get_port_name ,sizeof(func_name));
			}
  	    }
        strncat(PsBuffer, ",", strlen(","));
        strncat(PsBuffer, func_name, strlen(func_name) );  //�����ѯ���Ķ˿�����
        strncat(PsBuffer, ":", strlen(":"));
        strncat(PsBuffer, PortSerialNumber[PortCount], strlen(PortSerialNumber[PortCount])); //����˿�˳���
        PortCount += 1;   //��¼�ϱ��Ķ˿���
    }

    if ( 0 == PortCount || 0 == strlen(PsBuffer) )
    {
        return 1;
    }

    *Length = strlen(PsBuffer) + 1;  //����PsBuffer �ַ�������
    return 0;

}
EXPORT_SYMBOL_GPL(Check_EnablePortName);

/*****************************************************************
Parameters    :  m
Return        :
Description   :  �������л���ʱʱ�䣨��Ҫ�������ԵĽӿڣ�
*****************************************************************/
USB_VOID pnp_set_soft_delay(USB_INT m)
{
    g_soft_delyay = m;
    DBG_I(MBB_PNP, "g_soft_delyay is %d ms\n", g_soft_delyay);
}
/*****************************************************************
Parameters    :  m
Return        :
Description   :  ��ȡ���л���ʱʱ�䣨��Ҫ�������ԵĽӿڣ�
*****************************************************************/
USB_INT pnp_get_soft_delay(USB_VOID)
{
    return g_soft_delyay;
}
EXPORT_SYMBOL_GPL(pnp_get_soft_delay);

#ifdef MBB_USB_UNITARY_Q
#else
/*****************************************************************
Parameters    :  w
Return        :
Description   :    �����setconfig2��ֱ�ӵ��������صĺ������ȶ����������ᵼ�º˼�ͨ��ʧ��
   ���ܳɹ��Ͽ����磬��work�ص����������ػص���
*****************************************************************/
USB_VOID gatway_data_disconnect (struct work_struct* w)
{
    usb_adp_ctx_t* UsbCtx = NULL;

    UsbCtx = usb_get_adp_ctx();
    if(UsbCtx->usb_switch_gatway_mode_cb)
    {
        UsbCtx->usb_switch_gatway_mode_cb(UsbCtx->gatway_work_mode);
    }
    DBG_T(MBB_PNP, "gatway_data_disconnect %d \n", UsbCtx->gatway_work_mode);
}
#endif

/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   : ��ȡUSB��λ״̬
*****************************************************************/
USB_INT pnp_usb_state_get(USB_VOID)
{
    USB_INT usb_state = MBB_USB_FALSE;

#ifdef MBB_USB_UNITARY_Q
    usb_state = MBB_USB_TRUE;
#else
    //if (bsp_usb_is_support_pmu_detect())
    //{
    //    usb_state = bsp_pmu_usb_state_get();
    //}
    //else
    {
        usb_state = MBB_USB_TRUE;
    }
#endif

    DBG_I(MBB_PNP, "state %d\n", usb_state);

    return usb_state;
}

/*RNDIS,ECM,NCM �ṩ��·�ɵ�һ������ʱ��ѯ�����豸�Ƿ�׼�����������Ƿ����*/
/*****************************************************************
Parameters    :  USB_VOID
Return        :  NET_STATE_ALREADY ��������׼��OK��
                 NET_STATE_NOT_READY�����豸δOK
Description   :  ��ȡ��ǰ��������״̬
*****************************************************************/
USB_INT pnp_get_net_drv_state(USB_VOID)
{
    return usb_net_drv_state;
}
EXPORT_SYMBOL_GPL(pnp_get_net_drv_state);
/*RNDIS,ECM,NCM �ṩ��·�ɵ�һ������ʱ��ѯ�����豸�Ƿ�׼�����������Ƿ����*/
/*****************************************************************
Parameters    :  mode
Return        :  USB_VOID
Description   :  ������������״̬������������(RNDIS,ECM,NCM)׼����ʱ�����ø�״̬��
*****************************************************************/
USB_VOID pnp_set_net_drv_state(USB_INT mode)
{

    if (NET_STATE_ALREADY == mode)
    {
        DBG_T(MBB_PNP, "set_net_drv_state ALREADY\n");
        usb_net_drv_state = NET_STATE_ALREADY;
    }
    else
    {
        DBG_T(MBB_PNP, "set_net_drv_state NOT_READY\n");
        usb_net_drv_state = NET_STATE_NOT_READY;
    }
}
EXPORT_SYMBOL_GPL(pnp_set_net_drv_state);

/*****************************************************************
Parameters    :  data
Return        :
Description   : ���ģ���εĹ���
*****************************************************************/
USB_VOID soft_reconnect_handler( USB_ULONG data )
{
    struct android_dev* dev = android_get_android_dev();
    struct usb_composite_dev* cdev = NULL;
    usb_enum_state state = USB_ENUM_NONE;
    state = huawei_get_usb_enum_state();

    if ( USB_ENUM_DONE == state )
    {
        DBG_I(MBB_PNP, " soft_reconnect_handler  state:%d \n", state);
        return;
    }

    if (NULL != dev)
    {
        cdev = dev->cdev;
    }
    else
    {
        DBG_E(MBB_PNP, "android_dev NULL!\n");
        return;
    }

    if ((!cdev) || (!cdev->gadget))
    {
        DBG_E(MBB_PNP, "cdev or gadget NULL\n");
        return;
    }

    DBG_I(MBB_PNP, " soft_reconnect_handler process! \n");


    usb_gadget_disconnect(cdev->gadget);
    mdelay(g_soft_delyay);
    usb_gadget_connect(cdev->gadget);
}

/*****************************************************************************
KO API ����begin
*****************************************************************************/
/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  ��ȡpnp�Ľӿ�
*****************************************************************/
usb_pnp_api_hanlder* pnp_adp_get_api_handler(USB_VOID)
{
    return &g_pnp_api_hanlder;
}
EXPORT_SYMBOL_GPL(pnp_adp_get_api_handler);

/*****************************************************************************
KO API ����end
*****************************************************************************/


/*****************************************************************************
��ʼ�����˳�begin
*****************************************************************************/
/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  pnp adp��ʼ��
*****************************************************************/
USB_VOID usb_pnp_adp_init(USB_VOID)
{
#ifdef MBB_USB_UNITARY_Q
#else
    usb_adp_ctx_t* UsbCtx = usb_get_adp_ctx();
    INIT_DELAYED_WORK(&(UsbCtx->gatway_disconnect_work), gatway_data_disconnect);
#endif
    setup_timer(&g_soft_reconnect_timer , soft_reconnect_handler, (USB_ULONG)0);
}
EXPORT_SYMBOL_GPL(usb_pnp_adp_init);

/*****************************************************************************
��ʼ�����˳�end
*****************************************************************************/
/*PNP��ά�ɲ⺯��*/
/*****************************************************************
Parameters    :  USB_VOID
Return        :
Description   :  pnp_adp��صĵ�ǰ��Ϣ����ʹ��ecall����usb_dfxֱ�ӵ���
*****************************************************************/
USB_VOID pnp_adp_dump(USB_VOID)
{
    struct android_dev* dev = android_get_android_dev();
    struct usb_gadget*   gadget = NULL;
    USB_INT speed = 0;
    
    if(NULL == dev ||NULL == dev->cdev)
    {
        DBG_T(MBB_DEBUG, "NULL  dev->cdev\n");
        return;
    }
    gadget = dev->cdev->gadget;
    speed = (USB_INT)gadget->speed;
    
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");
    DBG_T(MBB_DEBUG, "|gadget of '%s' info\n", gadget->name);

    switch (speed)
    {
        case USB_SPEED_SUPER:
            DBG_T(MBB_DEBUG, "|gadget speed------- = superspeed\n");
            break;

        case USB_SPEED_HIGH:
            DBG_T(MBB_DEBUG, "|gadget speed------- = highspeed\n");
            break;

        case USB_SPEED_FULL:
        case USB_SPEED_LOW:
            DBG_T(MBB_DEBUG, "|gadget speed------- = fullspeed\n");
            break;

    }

    DBG_T(MBB_DEBUG, "|sg_supported------- = %d\n", gadget->sg_supported);
    DBG_T(MBB_DEBUG, "|is_otg------------- = %d\n", gadget->is_otg);
    DBG_T(MBB_DEBUG, "|is_a_peripheral---- = %d\n", gadget->is_a_peripheral);
    DBG_T(MBB_DEBUG, "|a_hnp_support------ = %d\n", gadget->a_hnp_support);
    DBG_T(MBB_DEBUG, "|b_hnp_enable------- = %d\n", gadget->b_hnp_enable);
    DBG_T(MBB_DEBUG, "|a_alt_hnp_support-- = %d\n", gadget->a_alt_hnp_support);
#ifdef MBB_USB_UNITARY_Q
    DBG_T(MBB_DEBUG, "|host_request------- = %d\n", gadget->host_request);
    DBG_T(MBB_DEBUG, "|remote_wakeup------ = %d\n", gadget->remote_wakeup);
#else
    DBG_T(MBB_DEBUG, "|is_suspend--------- = %d\n", gadget->is_suspend);
    DBG_T(MBB_DEBUG, "|rwakeup------------ = %d\n", gadget->rwakeup);
    DBG_T(MBB_DEBUG, "|is_in_dload-------- = %d\n", is_in_dload);
#endif
    DBG_T(MBB_DEBUG, "|-------------------------------------------------------\n");

    /*�豸��Ϣ������*/
    if ((NULL != dev) && (NULL != dev->cdev))
    {
        DBG_T(MBB_DEBUG, "|bLength------------ = 0x%X\n", dev->cdev->desc.bLength);
        DBG_T(MBB_DEBUG, "|bDescriptorType---- = 0x%X\n", dev->cdev->desc.bDescriptorType);
        DBG_T(MBB_DEBUG, "|bcdUSB------------- = 0x%4X\n", dev->cdev->desc.bcdUSB);
        DBG_T(MBB_DEBUG, "|bDeviceClass------- = 0x%X\n", dev->cdev->desc.bDeviceClass);
        DBG_T(MBB_DEBUG, "|bDeviceSubClass---- = 0x%X\n", dev->cdev->desc.bDeviceSubClass);
        DBG_T(MBB_DEBUG, "|bDeviceProtocol---- = 0x%X\n", dev->cdev->desc.bDeviceProtocol);
        DBG_T(MBB_DEBUG, "|bMaxPacketSize0---- = 0x%X\n", dev->cdev->desc.bMaxPacketSize0);
        DBG_T(MBB_DEBUG, "|idVendor----------- = 0x%X\n", dev->cdev->desc.idVendor);
        DBG_T(MBB_DEBUG, "|idProduct---------- = 0x%X\n", dev->cdev->desc.idProduct);
        DBG_T(MBB_DEBUG, "|bcdDevice---------- = 0x%X\n", dev->cdev->desc.bcdDevice);
        DBG_T(MBB_DEBUG, "|iManufacturer------ = 0x%X\n", dev->cdev->desc.iManufacturer);
        DBG_T(MBB_DEBUG, "|iProduct----------- = 0x%X\n", dev->cdev->desc.iProduct);
        DBG_T(MBB_DEBUG, "|iSerialNumber------ = 0x%X\n", dev->cdev->desc.iSerialNumber);
        DBG_T(MBB_DEBUG, "|bNumConfigurations- = 0x%X\n", dev->cdev->desc.bNumConfigurations);
    }
    
}
EXPORT_SYMBOL_GPL(pnp_adp_dump);

