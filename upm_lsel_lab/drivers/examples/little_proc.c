#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#define DRIVER_NAME "Little"

static int proc_little_open(struct inode *inode, struct file *file){
  printk(KERN_INFO DRIVER_NAME " open\n");
  return 0;
}

static ssize_t proc_little_write(struct file *file, const char __user * buf, size_t count, loff_t * ppos){

  printk(KERN_INFO DRIVER_NAME " write\n");
  return count;
}

static ssize_t proc_little_read(struct file *file, char __user * buf, size_t count, loff_t * ppos){
  printk(KERN_INFO DRIVER_NAME " read\n");
  return 0;
}

static int proc_little_release(struct inode *inode, struct file *file){
  printk(KERN_INFO DRIVER_NAME " close\n");
  return 0;
}

/* File Operations for /proc/little */
static const struct file_operations proc_little_operations = {
  .open = proc_little_open,
  .read = proc_little_read,
  .write = proc_little_write,
  .release = proc_little_release
};

static int __init little_init(void) {
  struct proc_dir_entry *little_proc_entry;
  printk(KERN_INFO DRIVER_NAME " init\n");

  little_proc_entry = proc_create(DRIVER_NAME, 0666, NULL, &proc_little_operations);
  if (little_proc_entry == NULL) {
    printk (KERN_ERR DRIVER_NAME " Couldn't create proc entry\n");
    return -ENOMEM;
  }
  return 0;
}

static void __exit little_exit(void) {
  printk(KERN_INFO DRIVER_NAME " exit\n");
  remove_proc_entry(DRIVER_NAME, NULL);
}

module_init(little_init);
module_exit(little_exit);

MODULE_AUTHOR("DIE-UPM");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_NAME ": little driver");
MODULE_ALIAS(DRIVER_NAME);
