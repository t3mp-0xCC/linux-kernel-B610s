/*lint -save -e7 -e21 -e34 -e63 -e124 -e578 -e537 -e573 -e52  -e831 -e413 -e132 -e101 -e2 -e565 
-e84 -e438 -e516 -e527 -e530 -e533  -e550 -e43  -e62 -e64 -e648 -e616 -e30 -e529 -e665 -e123 -e19 
-e732 -e539 -e322 -e401 -e713 -e737 -e718 -e746 -e752 -e830 -e958*/
/*lint --e{438}*/
/*lint --e{529}*/
/*lint --e{550}*/
/*lint --e{533}*/
/*lint --e{752}*/
/*lint --e{830}*/
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/usb/otg.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/freezer.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/usb/phy.h>
#include <linux/of.h>

static int dwc3_otg_set_power(struct usb_phy *x, unsigned mA)
{
	return 0;
}

static int dwc3_otg_set_suspend(struct usb_phy *x, int suspend)
{
	return 0;
}

static int dwc3_phy_probe(struct platform_device *pdev)
{
    struct device_node *node = pdev->dev.of_node;
    struct usb_phy *phy;
    enum usb_phy_type phy_type;
    int ret;

    phy = devm_kzalloc(&pdev->dev, sizeof *phy, GFP_KERNEL);
   	if (!phy) {
    	return -ENOMEM;
	}

	phy->dev	= &pdev->dev;
    phy->type = USB_PHY_TYPE_UNDEFINED;
    phy->state = OTG_STATE_UNDEFINED;
	phy->set_power      = dwc3_otg_set_power;
	phy->set_suspend    = dwc3_otg_set_suspend;

    if(of_device_is_compatible(node, "synopsys,usb2phy")){
    	phy->label	= "dwc3_otg2";
        phy_type = USB_PHY_TYPE_USB2;
    }

    if(of_device_is_compatible(node, "synopsys,usb3phy")){
        phy->label	= "dwc3_otg3";
        phy_type = USB_PHY_TYPE_USB3;
    }

	ret = usb_add_phy(phy, phy_type);
	if (ret) {
		dev_err(&pdev->dev, "can't register usb2 transceiver, err: %d\n",
			(USB_PHY_TYPE_USB2==phy_type)?"usb2":"usb3",ret);
		return ret;
	}

    return 0;
}

static int dwc3_phy_remove(struct platform_device *pdev)
{
    struct usb_phy *phy;

    phy = usb_get_phy(USB_PHY_TYPE_USB2);
    if(phy){
        usb_remove_phy(phy);
        usb_put_phy(phy);
    }

    phy = usb_get_phy(USB_PHY_TYPE_USB3);
    if(phy){
        usb_remove_phy(phy);
        usb_put_phy(phy);
    }    

	return 0;
}

static const struct of_device_id dwc3_phy_match[] = {
			{.compatible = "synopsys,usb2phy"},
            {.compatible = "synopsys,usb3phy"},
			{},
};

static struct platform_driver dwc3_phy_driver = {
	.probe		= dwc3_phy_probe,
	.remove		= dwc3_phy_remove,
	.driver	= {
		.name	= "dwc3_phy",
        .of_match_table	= of_match_ptr(dwc3_phy_match),
		.owner	= THIS_MODULE,
	},
};

int dwc3_phy_init(void)
{
    int ret;
    
    ret = platform_driver_register(&dwc3_phy_driver);
    if(ret){
        printk("failed to register dwc3_phy_driver!\n");
    }

    return ret;
}
EXPORT_SYMBOL(dwc3_phy_init);

void dwc3_phy_exit(void)
{
    platform_driver_unregister(&dwc3_phy_driver);

    return ;
}
EXPORT_SYMBOL(dwc3_phy_exit);

MODULE_AUTHOR("Synopsys, Inc");
MODULE_DESCRIPTION("Synopsys DWC3 USB PHY");
MODULE_LICENSE("Dual BSD/GPL");
/*lint -restore*/
