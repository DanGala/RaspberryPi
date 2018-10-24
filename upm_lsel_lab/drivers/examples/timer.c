#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>

#define DRIVER_NAME "Timer"

static struct timer_list s_Timer;
#define PERIOD 1000

static void TimerHandler(unsigned long unused)
{
   mod_timer(&s_Timer, jiffies + msecs_to_jiffies(PERIOD));
   printk(KERN_INFO DRIVER_NAME " Timeout\n");
}

static int timer_init(void) {
  printk(KERN_INFO DRIVER_NAME " init\n");
  setup_timer(&s_Timer, TimerHandler, 0);
  mod_timer(&s_Timer, jiffies + msecs_to_jiffies(PERIOD));

  return 0;
}

static void timer_exit(void) {
  printk(KERN_INFO DRIVER_NAME " exit\n");
  del_timer(&s_Timer);
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_AUTHOR("DIE-UPM");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_NAME ": timer driver");
MODULE_ALIAS(DRIVER_NAME);
