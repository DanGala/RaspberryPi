#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_INFO */
#include <linux/init.h>     /* Needed for the macros */
#include <linux/device.h>
#include <linux/err.h>
MODULE_LICENSE("GPL");

#define DRIVER_NAME "LittlePWM"
#define CLASS_NAME "LittlePWMClass"

static ssize_t show_period(struct device *dev, struct device_attribute *attr, char *buf);
static ssize_t set_period(struct device* dev,
                          struct device_attribute* attr,
                          const char* buf,
                          size_t count);

#define MODE (S_IRUSR|S_IWUSR)

static DEVICE_ATTR(period, MODE, show_period, set_period);
static struct device *s_pDeviceObject;
static struct class *s_pDeviceClass;

static struct pwm_device_state
{
    int m_Period;
} s_DeviceState;

static int __init pwmdriver_init(void)
{
  int result;
  printk(KERN_INFO DRIVER_NAME " init\n");

  s_pDeviceClass = class_create(THIS_MODULE, CLASS_NAME);
  BUG_ON(IS_ERR(s_pDeviceClass));

  s_pDeviceObject = device_create(s_pDeviceClass, NULL, 0, &s_DeviceState, DRIVER_NAME);
  result = device_create_file(s_pDeviceObject, &dev_attr_period);
  BUG_ON(result < 0);

  return result;
}

static ssize_t show_period(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_device_state *pwm_device_data;

    pwm_device_data = dev_get_drvdata(dev);
    return scnprintf(buf, PAGE_SIZE, "%d\n", pwm_device_data->m_Period);
}

static ssize_t set_period(struct device* dev,
                          struct device_attribute* attr,
                          const char* buf,
                          size_t count)
{
    long period_value = 0;
    struct pwm_device_state *pwm_device_data;

    if (kstrtol(buf, 10, &period_value) < 0)
        return -EINVAL;
    if (period_value < 10)  //Safety check
        return -EINVAL;

    pwm_device_data = dev_get_drvdata(dev);
    pwm_device_data->m_Period = period_value;

    return count;
}

static void __exit pwmdriver_exit(void)
{
    device_remove_file(s_pDeviceObject, &dev_attr_period);
    device_destroy(s_pDeviceClass, 0);
    class_destroy(s_pDeviceClass);
}

module_init(pwmdriver_init);
module_exit(pwmdriver_exit);
