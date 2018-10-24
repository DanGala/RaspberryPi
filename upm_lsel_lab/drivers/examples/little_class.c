#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/fs.h>             // Header for the Linux file system support

//#include <linux/seq_file.h>
//#include <linux/platform_device.h>

#define DRIVER_NAME "LittlePWM"
#define CLASS_NAME "LittleClass"
#define MODE (S_IRUSR|S_IWUSR)

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  littleClass  = NULL;  ///< The device-driver class struct pointer
static struct device* littleDevice = NULL;  ///< The device-driver device struct pointer

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

/* File Operations for /dev/Little2 */
static const struct file_operations fops = {
  .open = proc_little_open,
  .read = proc_little_read,
  .write = proc_little_write,
  .release = proc_little_release
};

static int __init little_init(void){
  printk(KERN_INFO DRIVER_NAME " init\n");
 
  // Try to dynamically allocate a major number for the device
  majorNumber = register_chrdev(0, DRIVER_NAME, &fops);
  if (majorNumber<0){
    printk(KERN_ALERT DRIVER_NAME " failed to register a major number\n");
    return majorNumber;
  }
  printk(KERN_INFO DRIVER_NAME " registered correctly with major number %d\n", majorNumber);
 
  // Register the device class
  littleClass = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(littleClass)){                // Check for error and clean up if there is
    unregister_chrdev(majorNumber, DRIVER_NAME);
    printk(KERN_ALERT DRIVER_NAME " Failed to register device class\n");
    return PTR_ERR(littleClass);          // Correct way to return an error on a pointer
  }
  printk(KERN_INFO DRIVER_NAME " device class registered correctly\n");
 
  // Register the device driver
  littleDevice = device_create(littleClass, NULL, MKDEV(majorNumber, 0), NULL, DRIVER_NAME);
  if (IS_ERR(littleDevice)){               // Clean up if there is an error
    class_destroy(littleClass);           // Repeated code but the alternative is goto statements
    unregister_chrdev(majorNumber, DRIVER_NAME);
    printk(KERN_ALERT DRIVER_NAME " Failed to create the device\n");
    return PTR_ERR(littleDevice);
  }

  printk(KERN_INFO DRIVER_NAME " device class created correctly\n");
  return 0;
}
 
static void __exit little_exit(void) {
  device_destroy(littleClass, MKDEV(majorNumber, 0));     // remove the device
  class_unregister(littleClass);                          // unregister the device class
  class_destroy(littleClass);                             // remove the device class
  unregister_chrdev(majorNumber, DRIVER_NAME);            // unregister the major number
  printk(KERN_INFO DRIVER_NAME " exit\n");
}

module_init(little_init);
module_exit(little_exit);

MODULE_AUTHOR("DIE-UPM");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_NAME ": little class driver");
MODULE_ALIAS(DRIVER_NAME);
