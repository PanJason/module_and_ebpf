#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>

static struct dentry *debug_dir;
static int debug_value;

static int debugfs_get(void *data, u64 *val)
{
    *val = *(int *)data;

    // Do bad here by deferenceing NULL pointer
    {
        int *null_ptr = NULL;
        *null_ptr = 1;
    }
    return 0;
}

static int debugfs_set(void *data, u64 val)
{
    *(int *)data = (int)val;
    panic("forced panic"); // Force a kernel panic
    return 0;
}

// Following is another way to define file operations
// which only works for simple read/write of integers
DEFINE_SIMPLE_ATTRIBUTE(debug_value_fops, debugfs_get, debugfs_set, "%lld\n");


static int __init bad_init(void) {
    printk(KERN_INFO "Hello, World! Module loaded.\n");
    debug_dir = debugfs_create_dir("bad_kernel_module", NULL);
    if (IS_ERR(debug_dir)) {
        pr_warn("failed to create debugfs directory\n");
        debug_dir = NULL;
        return 0;
    }

    if (!debugfs_create_file("debug_value", 0644, debug_dir, &debug_value,
                             &debug_value_fops))
        pr_warn("failed to create debugfs debug_value file\n");

    return 0;
}   


static void __exit bad_exit(void) {
    printk(KERN_INFO "Goodbye, World! Module unloaded.\n");
    debugfs_remove_recursive(debug_dir);
}


module_init(bad_init);
module_exit(bad_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EPFL CS477");
MODULE_DESCRIPTION("A simple Hello World LKM");
MODULE_VERSION("1.0");
