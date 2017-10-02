/*
*  AMD FCH GPIO Driver
*
*  Copyright (c) 2017 Florian Eckert <fe@dev.tdt.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/pci.h>
#include <linux/spinlock.h>

struct fch_gpio {
	struct gpio_chip      chip;
	u32                   pmbase;
	void __iomem          *pm;
	struct pci_dev        *pdev;
	spinlock_t            lock;
	u8                    orig[4];
};

#define to_fgp(chip)	container_of(chip, struct fch_gpio, chip)

#define FCH_ACPI_MMIO_BASE     0xFED80000
#define PMBASE_OFFSET          0x1500
#define PMBASE_SIZE            0x300

//#define PMGPIO_BASE           (FCH_ACPI_MMIO_BASE + PMBASE_OFFSET)

#define GPIO_BIT_DIR            23
#define GPIO_BIT_WRITE          22
#define GPIO_BIT_READ           16

static int fch_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	struct fch_gpio *fg = to_fgp(chip);
	u8 pin;

	switch(offset) {
		case 0:
			pin = 89;
			break;
		case 1:
			pin = 68;
			break;
		case 2:
			pin = 69;
			break;
		case 3:
			pin = 70;
			break;
		default:
			return -ENXIO;
	}

	fg->orig[offset] = ioread8(fg->pm + pin);
	dev_dbg(&fg->pdev->dev, "Requested gpio %d, data %x\n", offset, fg->orig[offset]);

	return 0;
}

static void fch_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	struct fch_gpio *fg = to_fgp(chip);
	u8 pin;

	switch(offset) {
		case 0:
			pin = 89;
			break;
		case 1:
			pin = 68;
			break;
		case 2:
			pin = 69;
			break;
		case 3:
			pin = 70;
			break;
	}

	dev_dbg(&fg->pdev->dev, "Freed gpio %d, data %x\n", offset, fg->orig[offset]);
	iowrite8(fg->orig[offset], fg->pm + pin);
}

//static int fch_gpio_getdir(struct gpio_chip *chip, unsigned offset)
//{
//	u32 val;
//
//	val = ~ioread32 (gpio_addr[offset]);
//
//	return (val >> GPIO_BIT_DIR) & 1;
//}

static void fch_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct fch_gpio *fg = to_fgp(chip);
	u32 temp;
	unsigned long flags;
	u8 pin;

	switch(offset) {
		case 0:
			pin = 89;
			break;
		case 1:
			pin = 68;
			break;
		case 2:
			pin = 69;
			break;
		case 3:
			pin = 70;
			break;
	}

	spin_lock_irqsave(&fg->lock, flags);
	temp = ioread32(fg->pm + pin);
	if (temp)
		temp |= BIT(GPIO_BIT_WRITE);
	else
		temp &= ~BIT(GPIO_BIT_WRITE);
	iowrite32(temp, fg->pm + pin);
	spin_unlock_irqrestore(&fg->lock, flags);

	dev_dbg(&fg->pdev->dev, "Setting gpio %d, value %d, reg=%02x\n", offset, value, temp);
}

static int fch_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct fch_gpio *fg = to_fgp(chip);
	u32 temp;
	u8 pin;

	switch(offset) {
		case 0:
			pin = 89;
			break;
		case 1:
			pin = 68;
			break;
		case 2:
			pin = 69;
			break;
		case 3:
			pin = 70;
			break;
		default:
			return -ENXIO;
	}

	temp = ioread32(fg->pm + pin);

	dev_dbg(&fg->pdev->dev, "Getting gpio %d, reg=%02x\n", offset, temp);

	return (temp >> GPIO_BIT_READ) & 1;
}

static int fch_gpio_dirout(struct gpio_chip *chip, unsigned offset, int value)
{
	struct fch_gpio *fg = to_fgp(chip);
	u32 temp;
	unsigned long flags;
	u8 pin;

	switch(offset) {
		case 0:
			pin = 89;
			break;
		case 1:
			pin = 68;
			break;
		case 2:
			pin = 69;
			break;
		case 3:
			pin = 70;
			break;
		default:
			return -ENXIO;
	}

	spin_lock_irqsave(&fg->lock, flags);
	temp = ioread32(fg->pm + pin);
	temp |= BIT(GPIO_BIT_DIR);
	iowrite32(temp, fg->pm + pin);
	spin_unlock_irqrestore(&fg->lock, flags);

	return 0;
}

static int fch_gpio_dirin(struct gpio_chip *chip, unsigned offset)
{
	struct fch_gpio *fg = to_fgp(chip);
	u32 temp;
	unsigned long flags;
	u8 pin;

	switch(offset) {
		case 0:
			pin = 89;
			break;
		case 1:
			pin = 68;
			break;
		case 2:
			pin = 69;
			break;
		case 3:
			pin = 70;
			break;
		default:
			return -ENXIO;
	}

	spin_lock_irqsave(&fg->lock, flags);
	temp = ioread32(fg->pm + pin);
	temp &= ~BIT(GPIO_BIT_DIR);
	iowrite32(temp, fg->pm + pin);
	spin_unlock_irqrestore(&fg->lock, flags);

	dev_dbg(&fg->pdev->dev, "Dirin gpio %d, reg=%02x\n", offset, temp);

	return 0;
}

static const struct pci_device_id fch_gpio_pci_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_HUDSON2_SMBUS), 0 },
	//{PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_HUDSON2_SMBUS, PCI_ANY_ID, PCI_ANY_ID},
	{ 0, } /* End of list */
};
MODULE_DEVICE_TABLE (pci, fch_gpio_pci_tbl);

static struct fch_gpio fg = {
	.chip = {
		.label            = "FCH GPIO",
		.owner            = THIS_MODULE,
		.base             = -1,
		.ngpio            = 4,
		.request          = fch_gpio_request,
		.free             = fch_gpio_free,
//		.get_direction    = fch_gpio_getdir,
		.set              = fch_gpio_set,
		.get              = fch_gpio_get,
		.direction_output = fch_gpio_dirout,
		.direction_input  = fch_gpio_dirin,
	},
};

static int __init fch_gpio_init(void)
{
	int err = -ENODEV;
	struct pci_dev *pdev = NULL;
	const struct pci_device_id *ent;

	/* Match the PCI device */
	for_each_pci_dev (pdev) {
		ent = pci_match_id(fch_gpio_pci_tbl, pdev);
		if (ent)
			goto found;
	}
	/* Device not found. */
	goto out;

found:
	//err = pci_read_config_dword(pdev, 0x42, &fg.pmbase);
	//dev_err(&pdev->dev, "0x%x\n",fg.pmbase);
	//if (err)
	//	goto out;
	//err = -EIO;
	//fg.pmbase &= 0x0000FF00;

	fg.pmbase = FCH_ACPI_MMIO_BASE;

	if (!request_region(fg.pmbase + PMBASE_OFFSET, PMBASE_SIZE, "AMD FCH GPIO")) {
		dev_err(&pdev->dev, "AMD FCH GPIO region 0x%x already in use!\n",
			fg.pmbase + PMBASE_OFFSET);
		err = -EBUSY;
		goto out;
	}
	fg.pm = ioport_map(fg.pmbase + PMBASE_OFFSET, PMBASE_SIZE);
	if (!fg.pm) {
		dev_err(&pdev->dev, "Couldn't map io port into io memory\n");
		release_region(fg.pmbase + PMBASE_OFFSET, PMBASE_SIZE);
		err = -ENOMEM;
		goto out;
	}
	fg.pdev = pdev;
	fg.chip.dev = &pdev->dev;

	spin_lock_init(&fg.lock);

	printk(KERN_INFO "AMD FCH GPIO detected\n");
	err = gpiochip_add(&fg.chip);
	if (err) {
		printk(KERN_ERR "GPIO registering failed (%d)\n", err);
		ioport_unmap(fg.pm);
		release_region(fg.pmbase + PMBASE_OFFSET, PMBASE_SIZE);
		goto out;
	}
out:
	return err;
}

static void __exit fch_gpio_exit(void)
{
	gpiochip_remove(&fg.chip);
	ioport_unmap(fg.pm);
	release_region(fg.pmbase + PMBASE_OFFSET, PMBASE_SIZE);
}

module_init(fch_gpio_init);
module_exit(fch_gpio_exit);

MODULE_AUTHOR ("Florian Eckert <fe@dev.tdt.de>");
MODULE_DESCRIPTION("AMD FCH GPIO driver");
MODULE_LICENSE("GPL");
