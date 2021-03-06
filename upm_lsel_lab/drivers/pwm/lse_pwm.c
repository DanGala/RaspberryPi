/*
 * PWM driver for Raspberry Pi
 *
 * Provides a sysfs interface to the PWM port present on pin 18
 * It has 2 modes of operation: pwm and servo
 *
 */

#define PWM_CLASS_NAME "lse-pwm"
#define DRIVER_NAME "lsePWM"

#include <linux/module.h>

#include <asm/io.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <linux/sysfs.h>

#include "lse_pwm.h"

//Avoid simultaneous modification of different attributes
static DEFINE_MUTEX(sysfs_lock);
static struct PWMRegisters __iomem *pwm_reg;
static struct GpioRegisters __iomem *gpio_reg;
static struct ClkRegisters __iomem *clk_reg;

struct lse_pwm {
  struct device *dev;
  u32 range;
  u16 duty;
  u16 active;
};


static struct lse_pwm pwm = {
  .duty	      = 50,                 //Duty cycle: 50%
  .range      = (PWM_CLK/1000),     //Freq: 1KHz
  .active     = 0                   //Not active
};

/* Sets the system timer to have the new divisor */
static int pwm_set_clk(struct lse_pwm *dev) {
  /* Stop clock and waiting for busy flag doesn't work, so kill clock */
    
  clk_reg->PWMCTL = 0x5A000000 | (1 << 5);
  udelay(10);

  clk_reg->PWMDIV = 0x5A000000 | (DIVISOR<<12);
	
  /* Enable the PWM clock from 19.2MHz oscillator */
  clk_reg->PWMCTL = 0x5A000011;

  return 0;
}

static int pwm_setup(struct lse_pwm *dev) {
  unsigned long pulse;

  //Add 50 to implement round apoximation instead of floor
  pulse = (50+dev->range*dev->duty)/100;

  if (pulse < 1) {
    dev_err(dev->dev, "pulse is out of range: %ld<1\n", pulse);
    return -ERANGE;
  }

  // Save range and pulse to the correct register. Use structs that have been mapped
  pwm_reg->RNG1 = dev->range;
  pwm_reg->DAT1 = pulse;

  return 0;
}

static int pwm_activate(struct lse_pwm *dev) {
  int ret = 0;

  /* Set PWM alternate function for GPIO18. Look for this info at the bcm2835 datasheet */
  SET_GPIO_ALT(18, 0x2);
    
  /* Disable PWM in PWM_CTL */
  pwm_reg->CTL = 0;
    
  /* Active wait */
  udelay(10);
    
  ret = pwm_set_clk(dev);
  if (ret)
    return ret;

  ret = pwm_setup(dev);

  /* Enable MSEN mode, and start PWM */
  pwm_reg->CTL = 0x81;

  dev->active = 1;
  return ret;
}


static int pwm_deactivate(struct lse_pwm *dev) {
  /* Disable PWM in PWM_CTL */
  pwm_reg->CTL = 0;

  /* Active wait */
  udelay(10);

  /* Set no alternate function for GPIO18, as INPUT. Look for this info at the bcm2835 datasheet */
  SET_GPIO_ALT(18, 0x0);

  /* Active wait */
  udelay(10);

  dev->active = 0;
  return 0;
}


static ssize_t active_show(struct device *d, struct device_attribute *attr, char *buf)
{
  ssize_t ret;
  struct lse_pwm *dev = dev_get_drvdata(d);
  mutex_lock(&sysfs_lock);
  ret = sprintf(buf, "%d\n", !!dev->active);
  mutex_unlock(&sysfs_lock);
  return ret;
}

static ssize_t active_store(struct device *d, struct device_attribute *attr, const char *buf, size_t count)
{
  ssize_t ret = 0;
  long new_active;
  struct lse_pwm *dev = dev_get_drvdata(d);

  mutex_lock(&sysfs_lock);
  ret = kstrtol(buf, 0, &new_active);
  if (ret == 0) {
    if (new_active) {
      ret = pwm_activate(dev);
    } else {
      ret = pwm_deactivate(dev);
    }
  } else {
    ret = -EINVAL;
  }
  mutex_unlock(&sysfs_lock);
  return ret?ret:count;
}
static DEVICE_ATTR(active, 0664, active_show, active_store);

static ssize_t frequency_show(struct device *d,
                         struct device_attribute *attr, char *buf)
{
  ssize_t ret;
  struct lse_pwm *dev = dev_get_drvdata(d);
  mutex_lock(&sysfs_lock);
  ret = sprintf(buf, "%d Hz\n", PWM_CLK/dev->range);
  mutex_unlock(&sysfs_lock);
  return ret;
}

static ssize_t frequency_store(struct device *d, struct device_attribute *attr, const char *buf, size_t count)
{
  ssize_t ret = 0;
  long new_freq;
  struct lse_pwm *dev = dev_get_drvdata(d);
  
  mutex_lock(&sysfs_lock);
  ret = kstrtol(buf, 0, &new_freq);
  if (ret == 0) {
    if (new_freq > 0 && new_freq < PWM_MAX_FREQ) {
      dev->range = PWM_CLK/new_freq;
      if (dev->active) {
        pwm_setup(dev);
      }
    } else {
      ret = -ERANGE;
    }
  }
  mutex_unlock(&sysfs_lock);
  return ret?ret:count;
}
static DEVICE_ATTR(frequency, 0664, frequency_show, frequency_store);


static ssize_t duty_show(struct device *d, struct device_attribute *attr, char *buf)
{
  ssize_t ret;
  struct lse_pwm *dev = dev_get_drvdata(d);
  mutex_lock(&sysfs_lock);
  ret = sprintf(buf, "%d%%\n", dev->duty);
  mutex_unlock(&sysfs_lock);
  return ret;
}

static ssize_t duty_store(struct device *d, struct device_attribute *attr, const char *buf, size_t count)
{
  ssize_t ret = 0;
  long new_duty;
  struct lse_pwm *dev = dev_get_drvdata(d);

  mutex_lock(&sysfs_lock);
  ret = kstrtol(buf, 0, &new_duty);
  if (ret == 0) {
    if (new_duty > 0 && new_duty < 100) {
      dev->duty = new_duty;
      if (dev->active) {
        //Add 50 to implement round apoximation instead of floor
        // Save calculated pulse to the correct register. Use structs that have been mapped
        pwm_reg->DAT1 = (50+dev->range*dev->duty)/100;
      }
    } else {
      ret = -ERANGE;
    }
  }
  mutex_unlock(&sysfs_lock);
  return ret?ret:count;
}
static DEVICE_ATTR(duty, 0664, duty_show, duty_store);

static struct attribute *pwm_sysfs_entries[] = {
  &dev_attr_active.attr,
  &dev_attr_duty.attr,
  &dev_attr_frequency.attr,
  NULL
};

static struct attribute_group pwm_attribute_group = {
  .name = NULL,
  .attrs = pwm_sysfs_entries,
};

static struct class pwm_class = {
  .name =   PWM_CLASS_NAME,
  .owner =  THIS_MODULE,
};


int __init pwm_init(void)
{
  int ret = 0;

  ret = class_register(&pwm_class);
  if (ret < 0) {
    pr_err("%s: Unable to register class\n", pwm_class.name);
    goto out0;
  }

  /* Create device for PWM */
  pwm.dev = device_create(&pwm_class, NULL,
                          MKDEV(0, 0), &pwm, "pwm");
  if (IS_ERR(pwm.dev)) {
    pr_err("%s: device_create failed\n", pwm_class.name);
    ret = PTR_ERR(pwm.dev);
    goto out1;
  }

  /* Create sysfs entry for PWM */
  ret = sysfs_create_group(&pwm.dev->kobj,
                           &pwm_attribute_group);
  if (ret < 0) {
    dev_err(pwm.dev, "failed to create sysfs device attributes\n");
    goto out2;
  }

  //Check lse_pwm.h to fill it up. Allocate the memory you are accesing with the structs
  clk_reg = ioremap(CLOCK_BASE, sizeof(clk_reg));
  pwm_reg = ioremap(PWM_BASE, sizeof(pwm_reg));
  gpio_reg = ioremap(GPIO_BASE, sizeof(gpio_reg));
  return 0;

out2:
  device_unregister(pwm.dev);

out1:
  class_unregister(&pwm_class);

out0:
  return ret;
}

void __exit pwm_exit(void)
{
  pwm_deactivate(&pwm);
  sysfs_remove_group(&pwm.dev->kobj,
                     &pwm_attribute_group);
  if (pwm.dev) {
    device_unregister(pwm.dev);
  }

  iounmap(gpio_reg);
  iounmap(pwm_reg);
  iounmap(clk_reg);

  class_unregister(&pwm_class);
}

module_init(pwm_init);
module_exit(pwm_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("DIE-UPM");
MODULE_DESCRIPTION(DRIVER_NAME ": pwm driver");
MODULE_ALIAS(DRIVER_NAME);
