#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


static int __init bad_init(void) {
    int *null_ptr = NULL;
    // Dereference a NULL pointer, which will cause a kernel panic
    *null_ptr = 1;
 
    printk(KERN_INFO "Hello, World! Module loaded.\n");
    return 0;
}   


static void __exit bad_exit(void) {
    printk(KERN_INFO "Goodbye, World! Module unloaded.\n");
}


module_init(bad_init);
module_exit(bad_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EPFL CS477");
MODULE_DESCRIPTION("A simple Hello World LKM");
MODULE_VERSION("1.0");