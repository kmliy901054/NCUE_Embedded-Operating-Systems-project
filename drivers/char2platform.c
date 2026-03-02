#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/of_irq.h>
#include <linux/cdev.h>

#define CLASS_NAME "myhwip_class"
#define DEVICE_CNT 1
#define MAJOR_num 0
#define MINOR_num 0
#define DRIVER_NAME "myhwip"

struct chardev{
	dev_t devid;
	int major;
	int minor;
	struct cdev cdev;
	struct class *class;
	struct device_node *nd;
	void __iomem *vbase;
	void __iomem *swt;
	unsigned long mem_start;
	unsigned long mem_end;
	unsigned int irq;
};

static struct chardev myhwip;

static int chardev_open(struct inode* node, struct file *filp)
{
    return nonseekable_open(node, filp);
}

static ssize_t chardev_read(struct file *filp, char *buf, size_t size, loff_t *offset)
{ 
    int ret, value;
    value = (readl(myhwip.swt)>>24)& 0xff;
    if((ret = copy_to_user(buf, &value, size))) 
	return ret;
    else
        return 0;    
}

static ssize_t chardev_write(struct file * filp, const char __user *buf, size_t size, loff_t * offset)
{
    int ret, value;
  
    if((ret = copy_from_user(&value, buf, size)))
	printk("err: copy_from_user. ret = %d\n", ret);
    writel(value&0xff, myhwip.vbase);
    return 0;
}

static int chardev_release(struct inode *inode, struct file *flip)
{
    return 0;
}

static struct file_operations chardev_fops = {
    .owner  = THIS_MODULE,
    .open   = chardev_open,
    .write  = chardev_write,
    .read   = chardev_read,
    .release= chardev_release,
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

	int rc = 0;

	dev_info(dev, "Device Tree Probing\n");
	/* Get iospace for the device */
	r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r_mem) {
		dev_err(dev, "invalid address\n");
		return -ENODEV;
	}

	dev_set_drvdata(dev, &myhwip);
	myhwip.mem_start = r_mem->start;
	myhwip.mem_end = r_mem->end;

	if (!request_mem_region(myhwip.mem_start,
				myhwip.mem_end - myhwip.mem_start + 1,
				DRIVER_NAME)) {
		dev_err(dev, "Couldn't lock memory region at %p\n",
			(void *)myhwip.mem_start);
		rc = -EBUSY;
		goto error1;
	}

	myhwip.vbase = ioremap(myhwip.mem_start, myhwip.mem_end - myhwip.mem_start + 1);
	if (!myhwip.vbase) {
		dev_err(dev, "mymodule: Could not allocate iomem\n");
		rc = -EIO;
		goto error2;
	}
	myhwip.swt = myhwip.vbase + 1;
	/* Get IRQ for the device */
	myhwip.irq = platform_get_irq(pdev, 0);	
	rc = request_irq(myhwip.irq, myhwip_irq, IRQF_TRIGGER_RISING, DRIVER_NAME, NULL);
	if (rc<0) {
		dev_err(dev, "testmodule: Could not allocate interrupt %d.\n",
			myhwip.irq);
		goto error3;
	}

	dev_info(dev,"mymodule at 0x%08x mapped to 0x%08x, irq=%d\n",
		(unsigned int __force)myhwip.mem_start,
		(unsigned int __force)myhwip.vbase,
		myhwip.irq);
	
    	myhwip.major = MAJOR_num;
    	myhwip.minor = MINOR_num;
    	if(myhwip.major){
    		myhwip.devid = MKDEV(myhwip.major, 0);
		register_chrdev_region(myhwip.devid, DEVICE_CNT, DRIVER_NAME);
    	}
    	else{
		if((alloc_chrdev_region(&myhwip.devid, myhwip.minor, DEVICE_CNT, DRIVER_NAME))<0)
			printk("allocating chrdev region failed!\n");
		else{
			myhwip.major = MAJOR(myhwip.devid);
			myhwip.minor = MINOR(myhwip.devid);
		}
    	}
    	printk("major:%d, minor:%d\n", myhwip.major, myhwip.minor);

    	cdev_init(&myhwip.cdev, &chardev_fops);
    	cdev_add(&myhwip.cdev, myhwip.devid, DEVICE_CNT);

    	myhwip.class = class_create(THIS_MODULE, CLASS_NAME);
    	device_create(myhwip.class, NULL, myhwip.devid, NULL, DRIVER_NAME);

	
	return 0;
error3:
	free_irq(myhwip.irq, &myhwip);	
error2:
	release_mem_region(myhwip.mem_start, myhwip.mem_end - myhwip.mem_start + 1);
error1:
	kfree(&myhwip);
	dev_set_drvdata(dev, NULL);
	return rc;
}

static int myhwip_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
    	cdev_del(&myhwip.cdev);
    	unregister_chrdev_region(myhwip.devid, DEVICE_CNT);
    	device_destroy(myhwip.class, myhwip.devid);
    	class_destroy(myhwip.class);
    	iounmap(myhwip.vbase);
    	free_irq(myhwip.irq, NULL);
	release_mem_region(myhwip.mem_start, myhwip.mem_end - myhwip.mem_start + 1);
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
