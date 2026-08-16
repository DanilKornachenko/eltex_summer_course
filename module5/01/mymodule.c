#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init hello_init(void) {
  printk(KERN_INFO "Hello world!\n");
  return 0;
}

static void __exit goodbye(void) {
  printk(KERN_INFO "Cleaning up module.\n");
}

MODULE_LICENSE("DFL");
MODULE_AUTHOR("Danil Kornachenko");
MODULE_DESCRIPTION("My first module. (I'm so excited)");

module_init(hello_init);
module_exit(goodbye);
