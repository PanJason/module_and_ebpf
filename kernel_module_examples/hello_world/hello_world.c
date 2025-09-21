// Hello World Kernel Module with explanation

// Macros used to mark up functions e.g., __init __exit
#include <linux/init.h>
// Core header for loading LKMs into the kernel
#include <linux/module.h>
// Contains types, macros, functions for the kernel
#include <linux/kernel.h>


// __init macro tells the kernel this init function is only used once at
// initialization so the kernel will free up the memory this function takes
// after execution
static int __init hello_init(void) {
    // KERN_INFO is a loglevel indicating a general information message
    printk(KERN_INFO "Hello, World! Module loaded.\n");
    return 0;
}   


// __exit macro tells that this code is only needed if the module is removable
// so the compiler can discard this code if the module is built-in
static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye, World! Module unloaded.\n");
}


// Register the function that the kernel will call when `insmod`
// #define module_init(x) __initcall(x);
module_init(hello_init);
// Register the function that the kernel will call when `rmmod`
// #define module_exit(x) __exitcall(x);
module_exit(hello_exit);

// Metadata about the module
MODULE_LICENSE("GPL"); // NON-GPL cannot access GPL-only symbols
MODULE_AUTHOR("EPFL CS477");
MODULE_DESCRIPTION("A simple Hello World LKM");
MODULE_VERSION("1.0");