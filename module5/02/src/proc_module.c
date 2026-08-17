#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

int len, temp;
char* msg;

static ssize_t read_from_proc(struct file* filep, char* buf, size_t count,
                       loff_t* offp) {
  if (count > temp) {
    count = temp;
  }

  temp = temp - count;
  copy_to_user(buf, msg, count);
  if (count == 0) {
    temp = len;
  }

  return count;
}

static ssize_t write_to_proc(struct file* filep, const char* buf, size_t count,
                      loff_t* offp) {
  copy_from_user(msg, buf, count);
  len = count;
  temp = len;
  return count;
}

static const struct proc_ops proc_fops = {
  proc_read : read_from_proc,
  proc_write : write_to_proc,
};

static int __init start_proc_module(void) {
  proc_create("my_module_proc", 0, NULL, &proc_fops);
  msg = kmalloc(10 * sizeof(char), GFP_KERNEL);

  return 0;
}

static void __exit exit_proc_module(void) {
  remove_proc_entry("my_module_proc", NULL);
  kfree(msg);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danil Kornachenko");
MODULE_DESCRIPTION(
    "My first module with user space interact. (I'm so excited)");

module_init(start_proc_module);
module_exit(exit_proc_module);
