#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Finaldie");
MODULE_DESCRIPTION("hello world module");
MODULE_SUPPORTED_DEVICE("testdevice");
MODULE_VERSION("0.0.1");

int __init hello_init(void)
{
	printk(KERN_INFO "hello world, kernel\n");
	return 0;
}

void __exit hello_exit(void)
{
	printk(KERN_INFO "Goodbye world, kernel\n");
}

module_init(hello_init);
module_exit(hello_exit);
