/* PCI driver hello world
 * Copyright (C) 2015 Kevin Grandemange
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 

/*
//#include <linux/init.h>
#include <linux/module.h>
//#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
//#include <linux/printk.h>
//#include <linux/proc_fs.h>
//#include <linux/seq_file.h>
#include <asm/io.h>
*/

#include <linux/module.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/interrupt.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/wait.h>

#include <asm/io.h>

#define PCI_TIC_VENDOR 0x1337
#define PCI_TIC_DEVICE 0x0001
#define PCI_TIC_SUBVENDOR 0x1af4
#define PCI_TIC_SUBDEVICE 0x1100

#define MAX_TIC_MAPS 1
#define MAX_TIC_PORT_REGIONS 1
//#define BUF_SIZE 16

static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;

static struct pci_driver hello_tic;

struct tic_mem {
    const char *name;
    void __iomem *start;
    unsigned long size;
};

struct tic_io {
    const char *name;
    unsigned long start;
    unsigned long size;
};

struct tic_info {
    struct tic_mem mem[MAX_TIC_MAPS];
    struct tic_io port[MAX_TIC_PORT_REGIONS];
    u8 irq;
};

struct tic_info * global_info;

static irqreturn_t hello_tic_handler(int irq, void *dev_info)
{
    struct tic_info *info = (struct tic_info *) dev_info;
    pr_alert("IRQ %d handled ", irq);
    /* is it our device throwing an interrupt ? */
    if (inl(info->port[0].start)) {
        /* deassert it */
	pr_alert("from our PCI\n");
        outl(0, info->port[0].start );
	flag=1;
	wake_up_interruptible(&wq);
        return IRQ_HANDLED;
    } else {
        /* not mine, we have done nothing */
	pr_alert("NOT from our PCI\n");
        return IRQ_NONE;
    }
}

/*
static ssize_t hello_pci_write(struct file *file, const char __user * buf, size_t count, loff_t * ppos){
  char pci_phrase[BUF_SIZE];
  int my_count;

  //printk(KERN_INFO DRIVER_NAME " write\n");
  
  my_count = count;
  if (my_count >= BUF_SIZE) {
    my_count = BUF_SIZE-1;
  }

  if (copy_from_user(pci_phrase, buf, my_count))
    return -EFAULT;
  pci_phrase[my_count] = '\0';

  printk("<1> %s written (%d chars)\n", pci_phrase, my_count);

  return my_count;
}

*/
static int hello_pci_show(struct seq_file *m, void *v) {
  char msg;
  u64 lchar;
  //u8 i;

  struct tic_info *info = (struct tic_info *)v;
  if (info != global_info) {
    pr_alert("inode private data is WRONG\n");
    info = global_info;
  } else {
    pr_alert("inode private data is OK\n");
  }

  wait_event_interruptible(wq, flag != 0);
  flag =0; //probar ...es nuevo
  lchar = inl(info->port[0].start+4);
  msg = (u8)lchar;
  seq_printf(m, "Read: %c\n", msg);
  pr_alert("show: %c\n", msg);
  return 0;
}

static int hello_pci_open(struct inode *inode, struct file *file) {
	return single_open(file, hello_pci_show, global_info /*inode->private*/);
}

static const struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.open = hello_pci_open,
	.read = seq_read,
	//.write = hello_pci_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int hello_tic_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    struct proc_dir_entry *proc_entry;

    struct tic_info *info;
    info = kzalloc(sizeof(struct tic_info), GFP_KERNEL);
    if (!info)
        return -ENOMEM;

    if (pci_enable_device(dev))
        goto out_free;

    pr_alert("enabled device\n");

    if (pci_request_regions(dev, "hello_tic"))
        goto out_disable;

    pr_alert("requested regions\n");

    /* BAR 0 has IO */
    info->port[0].name = "tic-io";
    info->port[0].start = pci_resource_start(dev, 0);
    info->port[0].size = pci_resource_len(dev, 0);

    /* BAR 1 has MMIO */
    info->mem[0].name = "tic-mmio";
    info->mem[0].start = pci_ioremap_bar(dev, 1);
    info->mem[0].size = pci_resource_len(dev, 1);
    if (!info->mem[0].start)
        goto out_unrequest;

    pr_alert("remaped addr for kernel uses\n");
    pr_alert("BAR0: size of %ld\n", info->port[0].size);
    pr_alert("BAR1: size of %ld\n", info->mem[0].size);

//proc_entry = proc_create("hello", 0755, NULL, &proc_fops);

    /* get device irq number */
    if (pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &info->irq))
        goto out_iounmap;

    /* request irq */
    if (devm_request_irq(&dev->dev, info->irq, hello_tic_handler, IRQF_SHARED, hello_tic.name, (void *) info))
        goto out_iounmap;

    /* get a mmio reg value and change it */
    pr_alert("device id=%x\n", ioread32(info->mem[0].start + 4));
    //iowrite32(0x4567, info->mem[0].start + 4);
    //pr_alert("modified device id=%x\n", ioread32(info->mem[0].start + 4));

    /* assert an irq */
  ////  outb(1, info->port[0].start);
    /* try dma without iommu */
    //outl(1, info->port[0].start + 4);

   proc_entry = proc_create_data("hello_proc", 0, NULL, &proc_fops, (void *) info);

   global_info = info;

    pci_set_drvdata(dev, info);
    pr_alert("driver_pci probe ends here\n");
    return 0;

out_iounmap:
    pr_alert("tic:probe_out:iounmap");
    iounmap(info->mem[0].start);
out_unrequest:
    pr_alert("tic:probe_out_unrequest\n");
    pci_release_regions(dev);
out_disable:
    pr_alert("tic:probe_out_disable\n");
    pci_disable_device(dev);
out_free:
    pr_alert("tic:probe_out_free\n");
    kfree(info);
    return -ENODEV;
}

static void hello_tic_remove(struct pci_dev *dev)
{
    /* we get back the driver data we store in
     * the pci_dev struct */
    struct tic_info *info = pci_get_drvdata(dev);

    remove_proc_entry("hello_proc", NULL);

    /* let's clean a little */
    pci_release_regions(dev);
    pci_disable_device(dev);
    iounmap(info->mem[0].start);

    kfree(info);

}


/* vendor and device (+ subdevice and subvendor)
 * identifies a device we support
 */
static struct pci_device_id hello_tic_ids[] = {
    
    { PCI_DEVICE(PCI_TIC_VENDOR, PCI_TIC_DEVICE) },
    { 0, },
};

/* id_table describe the device this driver support
 * probe is called when a device we support exist and
 * when we are chosen to drive it.
 * remove is called when the driver is unloaded or
 * when the device disappears
 */
static struct pci_driver hello_tic = {
    .name = "hello_tic",
    .id_table = hello_tic_ids,
    .probe = hello_tic_probe,
    .remove = hello_tic_remove,
};



/* register driver in kernel pci framework */
module_pci_driver(hello_tic);
MODULE_DEVICE_TABLE(pci, hello_tic_ids);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hello world");
MODULE_AUTHOR("Kevin Grandemange");
/*
module_init(hello_pci_init);
module_exit(hello_pci_exit);
*/
