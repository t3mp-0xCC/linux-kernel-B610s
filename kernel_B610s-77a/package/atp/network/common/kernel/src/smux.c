/***************************************************************************
 * File Name    : smux.c
 * Description  : contains network driver implementation for Service MUX (SMUX) which enables to create multiple services on a physical interface like atm or ptm etc.,
 ***************************************************************************/
#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/capability.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <net/datalink.h>
#include <linux/mm.h>
#include <linux/in.h>
#include <linux/init.h>
#include <net/p8022.h>
#include <net/arp.h>
#include <linux/rtnetlink.h>
#include <linux/notifier.h>
#include <linux/if_smux.h>
#include <linux/etherdevice.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
#include <net/net_namespace.h>
#endif
#ifdef DEBUG
#define DPRINTK(format, args...) printk(KERN_DEBUG "SMUX: " format, ##args)
#else
#define DPRINTK(format, args...)
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)

int smux_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);
int smux_dev_set_mac_address(struct net_device *dev, 
                             void *addr_struct_p);
int smux_dev_change_mtu(struct net_device *vdev, int new_mtu);
int smux_dev_open(struct net_device *vdev);
int smux_dev_stop(struct net_device *dev);
int smux_dev_ioctl(struct net_device *vdev, struct ifreq *ifr, int cmd);

#if (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
extern int mbb_get_spe_wan_portno();
#define LAN_WAN_VLAN_TAG    5
#endif

static const struct net_device_ops smux_netdev_ops2 = {
	.ndo_start_xmit         = smux_dev_hard_start_xmit,
	.ndo_set_mac_address    = smux_dev_set_mac_address,
	.ndo_change_mtu         = smux_dev_change_mtu,
	.ndo_open               = smux_dev_open,
	.ndo_stop               = smux_dev_stop,
	.ndo_do_ioctl           = smux_dev_ioctl,
	.ndo_get_stats          = smux_dev_get_stats,
};
#endif


/***************************************************************************
                         Global variables 
 ***************************************************************************/

static DEFINE_RWLOCK(smux_lock);

static LIST_HEAD(smux_grp_devs);

static int smux_device_event(struct notifier_block *, unsigned long, void *);

static struct notifier_block smux_notifier_block = {
	.notifier_call = smux_device_event,
};
/***************************************************************************
                         Function Definisions
 ***************************************************************************/
static int smux_ioctl_handler(void __user *);

static inline struct smux_group *list_entry_smuxgrp(const struct list_head *le)
{
  return list_entry(le, struct smux_group, smux_grp_devs);
}

/***************************************************************************
 * Function Name: __find_smux_group
 * Description  : returns the smux group of interfaces/devices from list
 * Returns      : struct smux_group.
 ***************************************************************************/
static struct smux_group *__find_smux_group(const char *ifname)
{
  struct list_head *lh;
  struct smux_group *smux_grp;
  struct smux_group *ret_smux = NULL;

  read_lock(&smux_lock);
  list_for_each(lh, &smux_grp_devs) {
    smux_grp = (struct smux_group *)list_entry_smuxgrp(lh);
    if (!strncmp(smux_grp->real_dev->name, ifname, IFNAMSIZ)) {
      ret_smux = smux_grp;
      break;
    }
  }
  read_unlock(&smux_lock);

  return ret_smux;
} /* __find_smux_group */

static inline struct smux_dev_info *list_entry_smuxdev(const struct list_head *le)
{
  return list_entry(le, struct smux_dev_info, list);
}

/***************************************************************************
 * Function Name: __find_smux_in_smux_group
 * Description  : returns the smux device from smux group of devices 
 * Returns      : struct net_device
 ***************************************************************************/
static struct net_device *__find_smux_in_smux_group(
                                     struct smux_group *smux_grp, 
                                     const char *ifname)
{
  struct list_head *lh;
  struct smux_dev_info * sdev = NULL;
  struct net_device    * ret_dev = NULL;

  read_lock(&smux_lock);
  list_for_each(lh, &smux_grp->virtual_devs) {
    sdev = list_entry_smuxdev(lh);
    if(!strncmp(sdev->vdev->name, ifname, IFNAMSIZ)) {
      ret_dev = sdev->vdev;
      break;
    }
  }
  read_unlock(&smux_lock);

  return ret_dev;
} /* __find_smux_in_smux_group */


/***************************************************************************
 * Function Name: smux_pkt_recv
 * Description  : packet recv routine for all smux devices from real dev.
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_pkt_recv(struct sk_buff *skb, 
                  struct net_device *dev,
                  struct net_device *rdev)
{
  struct smux_group *grp;
  unsigned char *dstAddr;
  struct sk_buff *skb2;
  struct smux_dev_info *dev_info;
  struct smux_dev_info *dev_info_first;
  struct list_head *lh;
  struct net_device *vdev;
  int isTxDone = 0;

  if(!dev) {
    dev_kfree_skb(skb);
    return 0;
  }

  grp = __find_smux_group(dev->name);

  if(!grp) {
    dev_kfree_skb(skb);
    return 0;
  }

  dstAddr = eth_hdr(skb)->h_dest;

  read_lock(&smux_lock);
  /* Multicast Traffic will go on all intf.*/
  if (dstAddr[0] & 1)
  {
    dev_info_first = NULL;
    /* multicast or broadcast frames */
    list_for_each(lh, &grp->virtual_devs)
    {
      dev_info = list_entry_smuxdev(lh);

      if(!dev_info_first) {
	dev_info_first = dev_info;
	continue;
      }

      vdev = dev_info->vdev;
      if((dev_info->proto == SMUX_PROTO_PPPOE) &&
         (skb->protocol != htons(ETH_P_PPP_DISC)) && 
         (skb->protocol != htons(ETH_P_PPP_SES))
/*Add for receive dhcp offer packet when use ppp wan detect wantype.*/
#ifdef CONFIG_HISI_HALF_BRIDGE
           && (skb->protocol != htons(ETH_P_IP))
#endif
        ) {
           DPRINTK("non-PPPOE packet dropped on RX dev %s\n", vdev->name);
      }
#ifdef SUPPORT_ATP_WAN_MODE_SWITCH
      else if (htons(ETH_P_ARP) == skb->protocol)
      {
        continue;
      }
#endif
      else {
        dev_info->stats.rx_packets++;
        dev_info->stats.rx_bytes += skb->len; 
        skb2 = skb_copy(skb, GFP_ATOMIC);

        if (skb2)
        {
            skb2->dev = vdev;
#ifdef CONFIG_HISI_HALF_BRIDGE  
            skb2->iif = skb2->dev->ifindex;
#endif
            skb2->pkt_type = PACKET_HOST;
            netif_rx(skb2);
        }
      }
    }
    if(NULL != dev_info_first)
	{
    if((dev_info_first->proto == SMUX_PROTO_PPPOE) &&
       (skb->protocol != htons(ETH_P_PPP_DISC)) && 
       (skb->protocol != htons(ETH_P_PPP_SES))
/*Add for receive dhcp offer packet when use ppp wan detect wantype.*/
#ifdef CONFIG_HISI_HALF_BRIDGE
           && (skb->protocol != htons(ETH_P_IP))
#endif
        ) {
         DPRINTK("non-PPPOE packet dropped on RX dev %s\n", vdev->name);
         dev_kfree_skb(skb);
    }
    else {
      dev_info_first->stats.rx_packets++;
      dev_info_first->stats.rx_bytes += skb->len; 
      skb->dev = dev_info_first->vdev;
#ifdef CONFIG_HISI_HALF_BRIDGE  
        skb->iif = skb->dev->ifindex;
#endif      
      skb->pkt_type = PACKET_HOST;
      netif_rx(skb);
	    }	
	}
	else
	{
         DPRINTK("non-PPPOE packet dropped on RX dev %s\n", vdev->name);
         dev_kfree_skb(skb);
	}
    isTxDone = 1;
  }
  else 
  {
    /* Routing Interface Traffic : check dst mac */
    list_for_each(lh, &grp->virtual_devs) {
      dev_info = list_entry_smuxdev(lh);
      vdev = dev_info->vdev;
      if (!memcmp(dstAddr, vdev->dev_addr, ETH_ALEN)) {

        if((dev_info->proto == SMUX_PROTO_PPPOE) &&
           (skb->protocol != htons(ETH_P_PPP_DISC)) && 
           (skb->protocol != htons(ETH_P_PPP_SES))
/*Add for receive dhcp offer packet when use ppp wan detect wantype.*/
#ifdef CONFIG_HISI_HALF_BRIDGE
           && (skb->protocol != htons(ETH_P_IP))
#endif
            ) {
           DPRINTK("non-PPPOE packet dropped on RX dev %s\n", vdev->name);
           dev_kfree_skb(skb);
        }
        else {
			skb->dev = vdev;
#ifdef CONFIG_HISI_HALF_BRIDGE  
                        skb->iif = skb->dev->ifindex;
#endif            
			dev_info->stats.rx_packets++;
			dev_info->stats.rx_bytes += skb->len; 
			skb->pkt_type = PACKET_HOST;
			netif_rx(skb);
        }
        isTxDone = 1;
        break;
      }
    }
  }

  if(isTxDone != 1) 
  {
    /* Bridging Interface Traffic */
    list_for_each(lh, &grp->virtual_devs) {
      dev_info = list_entry_smuxdev(lh);
      vdev = dev_info->vdev;
      if (vdev->promiscuity) {
        skb->dev = vdev;
#ifdef CONFIG_HISI_HALF_BRIDGE  
        skb->iif = skb->dev->ifindex;
#endif         
        dev_info->stats.rx_packets++;
        dev_info->stats.rx_bytes += skb->len; 
        skb->pkt_type = PACKET_HOST;
        netif_rx(skb);
        isTxDone = 1;
        break;
      }
    }
  }
  read_unlock(&smux_lock);

  if(!isTxDone) {
    DPRINTK("dropping packet that has wrong dest. on RX dev %s\n", dev->name);
    dev_kfree_skb(skb);
  }

  return 0;
} /* smux_pkt_recv */

/***************************************************************************
 * Function Name: smux_dev_hard_start_xmit
 * Description  : xmit routine for all smux devices on real dev.
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
  struct net_device_stats *stats = smux_dev_get_stats(dev);

  stats->tx_packets++; 
  stats->tx_bytes += skb->len;
  
  /*START ADD:Fix the bug that internet led did not flush when
  only upstream packets liujianfeng 37298 2009-12-14*/
  skb->dev->trans_start = jiffies;
  /*END ADD:Fix the bug that internet led did not flush when
  only upstream packets liujianfeng 37298 2009-12-14*/
  skb->dev = SMUX_DEV_INFO(dev)->smux_grp->real_dev;

  if(skb->dev !=NULL){
  	dev_queue_xmit(skb);
  }
  else{
  	printk("smux_dev_hard_start_xmit skb->dev ==NULL\n");
	kfree_skb(skb);
  }

  return 0;
} /* smux_dev_hard_start_xmit */

/***************************************************************************
 * Function Name: smux_dev_open
 * Description  : 
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_open(struct net_device *vdev)
{

  if (!(SMUX_DEV_INFO(vdev)->smux_grp->real_dev->flags & IFF_UP))
    return -ENETDOWN;

  return 0;
} /* smux_dev_open */

/***************************************************************************
 * Function Name: smux_dev_stop
 * Description  : 
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_stop(struct net_device *dev)
{
  return 0;
} /* smux_dev_stop */

/***************************************************************************
 * Function Name: smux_dev_set_mac_address
 * Description  : sets the mac for devs
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_set_mac_address(struct net_device *dev, 
                             void *addr_struct_p)
{
  struct sockaddr *addr = (struct sockaddr *)(addr_struct_p);
  int i, flgs;

  if (netif_running(dev))
    return -EBUSY;

  memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

  printk("%s: Setting MAC address to ", dev->name);
  for (i = 0; i < 6; i++)
    printk(" %2.2x", dev->dev_addr[i]);
  printk(".\n");

  if (memcmp(SMUX_DEV_INFO(dev)->smux_grp->real_dev->dev_addr,
                            dev->dev_addr, dev->addr_len) != 0) {
    if (!(SMUX_DEV_INFO(dev)->smux_grp->real_dev->flags & IFF_PROMISC)) {
      flgs = SMUX_DEV_INFO(dev)->smux_grp->real_dev->flags;

      /* Increment our in-use promiscuity counter */
      dev_set_promiscuity(SMUX_DEV_INFO(dev)->smux_grp->real_dev, 1);

      /* Make PROMISC visible to the user. */
      flgs |= IFF_PROMISC;
      printk("SMUX (%s):  Setting underlying device (%s) to promiscious mode.\n",
            dev->name, SMUX_DEV_INFO(dev)->smux_grp->real_dev->name);
      dev_change_flags(SMUX_DEV_INFO(dev)->smux_grp->real_dev, flgs);
    }
  } else {
    printk("SMUX (%s):  Underlying device (%s) has same MAC, not checking promiscious mode.\n",
            dev->name, SMUX_DEV_INFO(dev)->smux_grp->real_dev->name);
  }

  return 0;
} /* smux_dev_set_mac_address */


/***************************************************************************
 * Function Name: smux_dev_ioctl
 * Description  : handles device related ioctls
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_ioctl(struct net_device *vdev, struct ifreq *ifr, int cmd)
{
  struct net_device *real_dev = SMUX_DEV_INFO(vdev)->smux_grp->real_dev;
  struct ifreq ifrr;
  int err = -EOPNOTSUPP;

  strncpy(ifrr.ifr_name, real_dev->name, IFNAMSIZ);
  ifrr.ifr_name[IFNAMSIZ-1] = 0;
  ifrr.ifr_ifru = ifr->ifr_ifru;

  switch(cmd) {
    case SIOCGMIIPHY:
    case SIOCGMIIREG:
    case SIOCSMIIREG:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
      if (netif_device_present(real_dev))
        err = real_dev->netdev_ops->ndo_do_ioctl(real_dev, &ifrr, cmd);
#else
      if (real_dev->do_ioctl && netif_device_present(real_dev))
        err = real_dev->do_ioctl(real_dev, &ifrr, cmd);
#endif
      break;

    case SIOCETHTOOL:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
     err = dev_ethtool(&init_net, &ifrr);
#else
     err = dev_ethtool(&ifrr);
#endif

     if (!err)
       ifr->ifr_ifru = ifrr.ifr_ifru;
  }

  return err;
} /* smux_dev_ioctl */

/***************************************************************************
 * Function Name: smux_dev_change_mtu
 * Description  : changes mtu for dev
 * Returns      : 0 on Success
 ***************************************************************************/
int smux_dev_change_mtu(struct net_device *vdev, int new_mtu)
{
  if (SMUX_DEV_INFO(vdev)->smux_grp->real_dev->mtu < new_mtu)
    return -ERANGE;

  vdev->mtu = new_mtu;

  return 0;
}

/***************************************************************************
 * Function Name: smux_setup
 * Description  : inits device api
 * Returns      : None
 ***************************************************************************/
static void smux_setup(struct net_device *new_dev)
{
#ifndef CONFIG_MT7620
  //SET_MODULE_OWNER(new_dev);
#endif


  /* Make this thing known as a SMUX device */
  new_dev->priv_flags |= IFF_OSMUX;

  new_dev->tx_queue_len = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
  new_dev->netdev_ops = &smux_netdev_ops2;
#else
  new_dev->get_stats = smux_dev_get_stats;
#endif

} /* smux_setup */

/***************************************************************************
 * Function Name: smux_transfer_operstate
 * Description  : updates the operstate of overlay device 
 * Returns      : None.
 ***************************************************************************/
static void smux_transfer_operstate(const struct net_device *rdev, 
                                    struct net_device *vdev)
{

  if (rdev->operstate == IF_OPER_DORMANT)
    netif_dormant_on(vdev);
  else
    netif_dormant_off(vdev);

  if (netif_carrier_ok(rdev)) {
    if (!netif_carrier_ok(vdev))
      netif_carrier_on(vdev);
  } else {
    if (netif_carrier_ok(vdev))
      netif_carrier_off(vdev);
  }

} /* smux_transfer_operstate */

/***************************************************************************
 * Function Name: smux_register_device
 * Description  : regists new overlay device on real device & registers for 
                  packet handlers depending on the protocol types
 * Returns      : 0 on Success
 ***************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
static const struct net_device_ops smux_netdev_ops = {
	.ndo_start_xmit		    = smux_dev_hard_start_xmit,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr		= eth_validate_addr,
	.ndo_change_mtu         = eth_change_mtu,
	.ndo_get_stats          = smux_dev_get_stats,
};
#endif
static struct net_device *smux_register_device(const char *rifname,
													const char *nifname,
													int smux_proto)
{
  struct net_device *new_dev = NULL;
  struct net_device *real_dev = NULL; 
  struct smux_group *grp = NULL;
  struct smux_dev_info *vdev_info = NULL;
  int    new_dev_created = 0;
  int    mac_reused = 0;
  struct list_head *lh;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
  real_dev = dev_get_by_name(&init_net, rifname);
#else
  real_dev = dev_get_by_name(rifname);
#endif
  if (!real_dev) {
    goto real_dev_invalid;
  }

  if (!(real_dev->flags & IFF_UP)) {
    goto real_dev_invalid;
  }
  /* if not exist, then create */

#ifdef CONFIG_HISI_HALF_BRIDGE  
    new_dev = __dev_get_by_name(&init_net, nifname);
#else
    new_dev = dev_get_by_name(&init_net, nifname);
#endif
  /* Modified end */	
  if (NULL == new_dev)
  {
 	 new_dev = alloc_netdev(sizeof(struct smux_dev_info), 
                         nifname,
                         smux_setup);

 	 if (new_dev == NULL)
 	 {
   		 printk("netdev alloc failure\n");
   		 goto new_dev_invalid;
  	 }
	new_dev_created = 1;
  	ether_setup(new_dev);
  	new_dev->flags &= ~IFF_UP;
  	new_dev->flags &= ~IFF_MULTICAST;
  	real_dev->priv_flags |= IFF_RSMUX;

  	new_dev->state = (real_dev->state & 
                    ((1<<__LINK_STATE_NOCARRIER) |
                     (1<<__LINK_STATE_DORMANT))) |
                     (1<<__LINK_STATE_PRESENT);

  	new_dev->mtu = real_dev->mtu;
 	new_dev->type = real_dev->type;
  new_dev->hard_header_len = real_dev->hard_header_len;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
  new_dev->netdev_ops = &smux_netdev_ops;
#else
  new_dev->hard_start_xmit = smux_dev_hard_start_xmit;
#endif
#if (defined(CONFIG_BALONG_SPE) || defined(CONFIG_BALONG_SPE_MODULE))
/* for multi wan interface eth0_1 eth0_2 ...  
    解决以太网上行加速问题
*/
    if(0 != strcmp("eth0_0", nifname))
    {
#if (FEATURE_ON == MBB_FEATURE_ETH_PHY)
        new_dev->spe_portno = real_dev->spe_portno;
        new_dev->spe_vid = 0;
#elif (FEATURE_ON == MBB_FEATURE_ETH_SWITCH)
        new_dev->spe_portno = mbb_get_spe_wan_portno();
        new_dev->spe_vid = LAN_WAN_VLAN_TAG;
#endif
        new_dev->spe_alloced = real_dev->spe_alloced;
        printk(KERN_ERR"smux_register_device %s spe_portno=%d, spe_alloced=%d\n", 
               nifname, real_dev->spe_portno, real_dev->spe_alloced);
    }
#endif

  }
#ifdef CONFIG_HISI_HALF_BRIDGE
    else
    {
        new_dev->flags &= ~IFF_UP;
        new_dev->flags &= ~IFF_MULTICAST;
        real_dev->priv_flags |= IFF_RSMUX;
        new_dev->state = (real_dev->state & 
                    ((1<<__LINK_STATE_NOCARRIER) |
                     (1<<__LINK_STATE_DORMANT))) |
                     (1<<__LINK_STATE_PRESENT);

        new_dev->mtu = real_dev->mtu;
        new_dev->type = real_dev->type;
    }
#endif    
    /* Modified end */
  
  
  /* find smux group name. if not found create all new smux group */
  grp = __find_smux_group(rifname);

  if (!grp) {
    grp = kzalloc(sizeof(struct smux_group), GFP_KERNEL);

    if(grp) {
      INIT_LIST_HEAD(&grp->virtual_devs);
      INIT_LIST_HEAD(&grp->smux_grp_devs);

        
      grp->real_dev = real_dev;

      write_lock_irq(&smux_lock);
      list_add_tail(&grp->smux_grp_devs, &smux_grp_devs);
      write_unlock_irq(&smux_lock);
    }
    else {
      if (new_dev_created)
      {
	  	free_netdev(new_dev);
      }
    }
  }

  /* Assign mac addr for non-bridge intf & re-use mac for l2/l2-msc */
  if(grp && new_dev) {
    /* Assign default mac to bridge so that we can add it to linux bridge */
    if (smux_proto == SMUX_PROTO_BRIDGE) 
    {
        memcpy( new_dev->dev_addr, "\xFE\xFF\xFF\xFF\xFF\xFF", ETH_ALEN );
    }
    else 
    {
        if (list_empty(&grp->virtual_devs)) 
        {
            memcpy(new_dev->dev_addr, real_dev->dev_addr, ETH_ALEN);
        }
        else 
        {
            list_for_each(lh, &grp->virtual_devs) 
            {
                vdev_info = list_entry_smuxdev(lh);
				if (!memcmp(new_dev, 
                            vdev_info->vdev, sizeof(struct net_device))) 
                {
                    /*found myself, so continue...*/
                    continue;
                }
                if (!memcmp(real_dev->dev_addr, 
                            vdev_info->vdev->dev_addr, ETH_ALEN)) 
                {
                    mac_reused = 1;
                    break;
                }
            }

            if (!mac_reused) 
            {
                memcpy(new_dev->dev_addr, real_dev->dev_addr, ETH_ALEN);
            }
            else 
            {
                /* Read and display the MAC address. */
                new_dev->dev_addr[0] = 0xff;
                //vdev_info = SMUX_DEV_INFO(new_dev);

				memcpy( new_dev->dev_addr, "\xFE\xFF\xFF\xFF\xFF\xFF", ETH_ALEN );				

            }
        }
      }
  }
  
  if(grp && new_dev) {
    /*find new smux in smux group if it does not exit create one*/
    if(!__find_smux_in_smux_group(grp, nifname)) {

      vdev_info = SMUX_DEV_INFO(new_dev);
      memset(vdev_info, 0, sizeof(struct smux_dev_info));
      vdev_info->smux_grp = grp;
      vdev_info->vdev = new_dev;
      vdev_info->proto = smux_proto;
      INIT_LIST_HEAD(&vdev_info->list);
      write_lock_irq(&smux_lock);
      list_add_tail(&vdev_info->list, &grp->virtual_devs);
      write_unlock_irq(&smux_lock);

      if(smux_proto == SMUX_PROTO_BRIDGE) {
        new_dev->promiscuity = 1;
      }
      else if(smux_proto == SMUX_PROTO_IPOE) {
        new_dev->flags |= IFF_MULTICAST;
      }

#ifndef CONFIG_HISI_HALF_BRIDGE
      if (register_netdev(new_dev)) {
        printk("register_netdev failed\n");
        free_netdev(new_dev);
      }
      else {
        smux_transfer_operstate(real_dev, new_dev);
      }
#endif      
    /* Modified end */
    }
  }

  return new_dev;

real_dev_invalid:
new_dev_invalid:

  return NULL;

} /* smux_register_device */

/***************************************************************************
 * Function Name: smux_unregister_device
 * Description  : unregisters the smux devices along with releasing mem.
 * Returns      : 0 on Success
 ***************************************************************************/
static int smux_unregister_device(const char* vifname)
{
  struct net_device *vdev = NULL;
  struct net_device *real_dev = NULL;
  int ret;
  struct smux_dev_info *dev_info;
  ret = -EINVAL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
  vdev = dev_get_by_name(&init_net, vifname);
#else
  vdev = dev_get_by_name(vifname);
#endif

  if(vdev) {

    dev_info = SMUX_DEV_INFO(vdev);
    real_dev = dev_info->smux_grp->real_dev;

    write_lock_irq(&smux_lock);
    list_del(&dev_info->list);
    write_unlock_irq(&smux_lock);

    if (list_empty(&dev_info->smux_grp->virtual_devs)) {

      write_lock_irq(&smux_lock);
      list_del(&dev_info->smux_grp->smux_grp_devs);
      write_unlock_irq(&smux_lock);

      kfree(dev_info->smux_grp);
   }



   dev_put(vdev);
#ifndef CONFIG_HISI_HALF_BRIDGE
   unregister_netdev(vdev);


   synchronize_net();
#endif       
   /* Modified end */
   dev_put(real_dev); 

   ret = 0;
  }

  return ret;
} /* smux_unregister_device */
/***************************************************************************
 * Function Name: smux_device_event
 * Description  : handles real device events to update overlay devs. status
 * Returns      : 0 on Success
 ***************************************************************************/
static int smux_device_event(struct notifier_block *unused, 
                             unsigned long event, 
                             void *ptr)
{
  struct net_device *rdev = ptr;
  struct smux_group *grp = __find_smux_group(rdev->name);
  int flgs;
  struct list_head *lh;
  struct list_head *lhp;
  struct smux_dev_info *dev_info;

  if (!grp)
    goto out;

  switch (event) {

    case NETDEV_CHANGE:

      /* Propagate real device state to overlay devices */
      read_lock(&smux_lock);
      list_for_each(lh, &grp->virtual_devs) {
        dev_info = list_entry_smuxdev(lh);
        if(dev_info) {
          smux_transfer_operstate(rdev, dev_info->vdev);
        }
      }
      read_unlock(&smux_lock);
      break;

    case NETDEV_DOWN:

      /* Put all Overlay devices for this dev in the down state too.*/
      read_lock(&smux_lock);
      list_for_each(lh, &grp->virtual_devs) {
        dev_info = list_entry_smuxdev(lh);
        if(dev_info) {
          flgs = dev_info->vdev->flags;

          if (!(flgs & IFF_UP))
            continue;

          dev_change_flags(dev_info->vdev, flgs & ~IFF_UP);
        }
      }
      read_unlock(&smux_lock);
      break;

    case NETDEV_UP:

      /* Put all Overlay devices for this dev in the up state too.  */
      read_lock(&smux_lock);
      list_for_each(lh, &grp->virtual_devs) {
        dev_info = list_entry_smuxdev(lh);
        if(dev_info) {
          flgs = dev_info->vdev->flags;

          if (flgs & IFF_UP)
            continue;

          dev_change_flags(dev_info->vdev, flgs & IFF_UP);
        }
      }
      read_unlock(&smux_lock);
      break;

    case NETDEV_UNREGISTER:

      /* Delete all Overlay devices for this dev. */
      write_lock_irq(&smux_lock);
      list_for_each_safe(lh, lhp, &grp->virtual_devs) {
        dev_info = list_entry_smuxdev(lh);
        if(dev_info) {
          list_del(&dev_info->list);
          smux_unregister_device(dev_info->vdev->name);
        }
      }
      write_unlock_irq(&smux_lock);
    
      break;
  }

out:
  return NOTIFY_DONE;
} /* smux_device_event */

/***************************************************************************
 * Function Name: smux_ioctl_handler
 * Description  : ioctl handler for user apps
 * Returns      : 0 on Success
 ***************************************************************************/
static int smux_ioctl_handler(void __user *arg)
{
  int err = 0;
  struct smux_ioctl_args args;


  if (copy_from_user(&args, arg, sizeof(struct smux_ioctl_args)))
    return -EFAULT;

  args.ifname[IFNAMSIZ-1] = 0;
  args.u.ifname[IFNAMSIZ-1] = 0;

  switch (args.cmd) {

    case ADD_SMUX_CMD:
      printk("smux add intf.\n");
      if (!capable(CAP_NET_ADMIN))
        return -EPERM;
      if(smux_register_device(args.ifname, args.u.ifname, args.proto)) {
        err = 0;
      } else {
        err = -EINVAL;
      }
      break;

    case REM_SMUX_CMD:
      if (!capable(CAP_NET_ADMIN))
        return -EPERM;
      err = smux_unregister_device(args.u.ifname);
      break;

    default:
      printk("%s: Unknown SMUX CMD: %x \n",
			__FUNCTION__, args.cmd);
      return -EINVAL;
  }

  return err;
} /* smux_ioctl_handler */

/***************************************************************************
 * Function Name: smux_drv_init
 * Description  : Initialization of smux driver
 * Returns      : struct net_device
 ***************************************************************************/
static int __init smux_drv_init(void)
{
  register_netdevice_notifier(&smux_notifier_block);
  smux_ioctl_set(smux_ioctl_handler);

  return 0;
} /* smux_drv_init */

/***************************************************************************
 * Function Name: smux_cleanup_devices
 * Description  : cleans up all the smux devices and releases memory on exit
 * Returns      : None
 ***************************************************************************/
static void __exit smux_cleanup_devices(void)
{
  struct net_device *dev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
#else
  struct net_device *nxt;
#endif
  struct list_head *lh;
  struct list_head *lhp;
  struct smux_dev_info *dev_info;
  struct smux_group *grp;


  /* clean up all the smux devices */
  rtnl_lock();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
  for_each_netdev(&init_net, dev) {
#else
  for (dev = dev_base; dev; dev = nxt) {
    nxt = dev->next;
#endif
    if (dev->priv_flags & IFF_OSMUX) {
      dev_info = SMUX_DEV_INFO(dev);
      write_lock_irq(&smux_lock);
      list_del(&dev_info->list);
      write_unlock_irq(&smux_lock);
      unregister_netdevice(dev);
    }
  }
  rtnl_unlock();

  /* cleanup all smux groups  */
  write_lock_irq(&smux_lock);
  list_for_each_safe(lh, lhp, &smux_grp_devs) {
    grp = list_entry_smuxgrp(lh);
    if(grp) {
      list_del(&grp->virtual_devs);
    }
  }
  write_unlock_irq(&smux_lock);

} /* smux_cleanup_devices */

/***************************************************************************
 * Function Name: smux_drv_exit
 * Description  : smux module clean routine
 * Returns      : None
 ***************************************************************************/
static void __exit smux_drv_exit(void)
{

  smux_ioctl_set(NULL);

  /* Un-register us from receiving netdevice events */
  unregister_netdevice_notifier(&smux_notifier_block);
  smux_cleanup_devices();
  synchronize_net();

} /* smux_drv_exit */

module_init(smux_drv_init);
module_exit(smux_drv_exit);

EXPORT_SYMBOL(smux_pkt_recv);
