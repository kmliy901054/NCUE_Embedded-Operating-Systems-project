#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>

static void __iomem *intr_base;
static int irq;
static u32 current_mode = 0;

static irqreturn_t intr_irq_handler(int irq, void *dev_id)
{
    u32 val = ioread32(intr_base + 4);  
    pr_info("INTR IRQ Triggered, value = %d\n", val);
    switch(val) {
	    case 1: pr_info("[INTR] Next will exec GCD\n"); break;
	    case 2: pr_info("[INTR] Next will exec DES\n"); break;
	    case 3: pr_info("[INTR] Next will exec AES\n"); break;
	    default: pr_info("[INTR] None\n"); break;
	}
    return IRQ_HANDLED;
}

static int intr_probe(struct platform_device *pdev)
{
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    intr_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(intr_base))
        return PTR_ERR(intr_base);

    irq = platform_get_irq(pdev, 0);
    if (irq < 0)
        return irq;

    return devm_request_irq(&pdev->dev, irq, intr_irq_handler, 0,
                            "intr_driver", NULL);
}

static int intr_remove(struct platform_device *pdev)
{
    return 0;
}

static const struct of_device_id intr_of_match[] = {
    { .compatible = "xlnx,intr", },
    { },
};
MODULE_DEVICE_TABLE(of, intr_of_match);

static struct platform_driver intr_platform_driver = {
    .probe  = intr_probe,
    .remove = intr_remove,
    .driver = {
        .name           = "intr_driver",
        .of_match_table = intr_of_match,
    },
};
module_platform_driver(intr_platform_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wayne");
MODULE_DESCRIPTION("INTR Custom Driver");

