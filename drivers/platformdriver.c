#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#define DRIVER_NAME "myhwip"

struct platform{
	int irq;
	unsigned long mem_start;
	unsigned long mem_end;
	void __iomem *base_addr;
};

static irqreturn_t myhwip_irq(int irq, void *lp)
{
	printk("mymodule interrupt\n");
	return IRQ_HANDLED;
}

static int myhwip_probe(struct platform_device *pdev)
{
	struct resource *r_mem; /* IO mem resources */
	struct device *dev = &pdev->dev;
	struct platform *lp = NULL;

	int rc = 0;

	dev_info(dev, "Device Tree Probing\n");
	/* Get iospace for the device */
	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		dev_err(dev, "invalid address\n");
		return -ENODEV;
	}
	lp = (struct platform *) kmalloc(sizeof(struct platform), GFP_KERNEL);
	if (!lp) {
		dev_err(dev, "Cound not allocate mymodule device\n");
		return -ENOMEM;
	}
	dev_set_drvdata(dev, lp);
	lp->mem_start = r_mem->start;
	lp->mem_end = r_mem->end;

	if (!request_mem_region(lp->mem_start,
				lp->mem_end - lp->mem_start + 1,
				DRIVER_NAME)) {
		dev_err(dev, "Couldn't lock memory region at %p\n",
			(void *)lp->mem_start);
		rc = -EBUSY;
		goto error1;
	}

	lp->base_addr = ioremap(lp->mem_start, lp->mem_end - lp->mem_start + 1);
	if (!lp->base_addr) {
		dev_err(dev, "mymodule: Could not allocate iomem\n");
		rc = -EIO;
		goto error2;
	}

	/* Get IRQ for the device */
	lp->irq = platform_get_irq(pdev, 0);	
	rc = request_irq(lp->irq, myhwip_irq, IRQF_TRIGGER_RISING, DRIVER_NAME, NULL);
	if (rc<0) {
		dev_err(dev, "testmodule: Could not allocate interrupt %d.\n",
			lp->irq);
		goto error3;
	}

	dev_info(dev,"mymodule at 0x%08x mapped to 0x%08x, irq=%d\n",
		(unsigned int __force)lp->mem_start,
		(unsigned int __force)lp->base_addr,
		lp->irq);
	return 0;
error3:
	free_irq(lp->irq, lp);	
error2:
	release_mem_region(lp->mem_start, lp->mem_end - lp->mem_start + 1);
error1:
	kfree(lp);
	dev_set_drvdata(dev, NULL);
	return rc;
}

static int myhwip_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct platform *lp = dev_get_drvdata(dev);
	free_irq(lp->irq, lp);
	iounmap(lp->base_addr);
	release_mem_region(lp->mem_start, lp->mem_end - lp->mem_start + 1);
	kfree(lp);
	dev_set_drvdata(dev, NULL);
	return 0;

}

static struct of_device_id myhwip_of_match[] = {
	{ .compatible = "xlnx,myhwip-1.00", },
	{ /* end of list */ },
};

MODULE_DEVICE_TABLE(of, myhwip_of_match);

static struct platform_driver myhwip_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table	= myhwip_of_match,
	},
	.probe		= myhwip_probe,
	.remove		= myhwip_remove,
};

static int __init myhwip_init(void)
{
	printk("Module initialization!\n");
	return platform_driver_register(&myhwip_driver);
}

static void __exit myhwip_exit(void)
{
	platform_driver_unregister(&myhwip_driver);
	printk(KERN_ALERT "Close module!\n");
}

module_init(myhwip_init);
module_exit(myhwip_exit);
MODULE_LICENSE("GPL");
