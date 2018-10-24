#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#define DRIVER_NAME "Hello"

static int hello_init(void) {
  printk(KERN_INFO DRIVER_NAME "xx init\n");
  return 0;
}

static void hello_exit(void) {
  printk(KERN_INFO DRIVER_NAME " exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("DIE-UPM");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_NAME ": hello driver");
MODULE_ALIAS(DRIVER_NAME);
