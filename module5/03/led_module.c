#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/kd.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/console_struct.h>
#include <linux/vt_kern.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

#define SYSFILE "sysleds"

static struct kobject* my_object;
static int leds_mod = 0xFF;
static int leds_state = 0xFF;
static struct tty_struct* tty = NULL;
static struct timer_list timer;

static void leds_timer(struct timer_list* t) {
  leds_state = (leds_state == 0xFF) ? leds_mod : 0xFF;
  if (tty && tty->driver->ops->ioctl) {
    tty->driver->ops->ioctl(tty, KDSETLED, leds_state);
  }
  mod_timer(&timer, jiffies + HZ/5);
}

static ssize_t readfs(struct kobject* kobj, struct kobj_attribute* attr,
    char* buf) {
  return sprintf(buf, "%d\n", leds_mod);
}

static ssize_t writefs(struct kobject* kobj, struct kobj_attribute* attr,
    const char* buf, size_t count) {
  sscanf(buf, "%d", &leds_mod);
  leds_mod = leds_mod & 0xFF;
  return count;
}

static struct kobj_attribute attr = __ATTR(0, 0660, readfs, writefs);

static int __init led_parse_init(void) {
  printk(KERN_INFO "LED_MODULE insert...\n");

  int error = 0;

  my_object = kobject_create_and_add(SYSFILE, kernel_kobj);

  if (!my_object) {
    printk(KERN_INFO "No object.\n");
    return -ENOMEM;
  }

  error = sysfs_create_file(my_object, &attr.attr);

  if (error) {
    printk(KERN_INFO "error to create file in /sys/kernel/%s\n", SYSFILE);
  } else {
    printk(KERN_INFO "create file in /sys/kernel/%s\n", SYSFILE);
  }

  tty = vc_cons[fg_console].d->port.tty;
  if (!tty) {
    printk(KERN_INFO "No tty for console.\n");
    return -ENODEV;
  }

  timer_setup(&timer, leds_timer, 0);

  mod_timer(&timer, jiffies + HZ/5);

  return error;
}

static void __exit led_parse_cleanup(void) {
  printk(KERN_INFO "Cleaning up LED_MODULE...\n");
  timer_delete(&timer);

  if (tty && tty->driver->ops->ioctl) {
    tty->driver->ops->ioctl(tty, KDSETLED, 0xFF);
  }

  kobject_put(my_object);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danil Kornachenko");
MODULE_DESCRIPTION("Simple LED controll by file");

module_init(led_parse_init);
module_exit(led_parse_cleanup);
