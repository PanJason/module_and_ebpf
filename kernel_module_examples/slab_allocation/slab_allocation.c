#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MODULE_NAME "slab_allocation"
#define SLAB_ITEM_SIZE 384
#define SLAB_ITEM_PAYLOAD_SIZE \
	(SLAB_ITEM_SIZE - sizeof(struct list_head) - sizeof(unsigned long))

struct slab_item {
	struct list_head node;
	unsigned long id;
	u8 payload[SLAB_ITEM_PAYLOAD_SIZE];
};

static struct kmem_cache *item_cache;

// define a list head for all the allocated objects
static LIST_HEAD(item_list);
// define a mutex to prevent concurrent change
static DEFINE_MUTEX(item_lock);
static unsigned int current_count;

static struct dentry *debugfs_dir;
static struct dentry *debugfs_file;

static void free_single_item(struct slab_item *item)
{
	list_del(&item->node);
	kmem_cache_free(item_cache, item);
}

static int grow_pool(unsigned int target)
{
	unsigned int made = 0;

	while (current_count + made < target) {
		struct slab_item *item;

		item = kmem_cache_alloc(item_cache, GFP_KERNEL);
		if (!item)
			goto err_alloc;

		INIT_LIST_HEAD(&item->node);
		item->id = current_count + made + 1;
		list_add_tail(&item->node, &item_list);
		made++;
	}

	current_count += made;
	return 0;

err_alloc:
	while (made--) {
		struct slab_item *victim;

		victim = list_last_entry(&item_list, struct slab_item, node);
		free_single_item(victim);
	}
	return -ENOMEM;
}

static void shrink_pool(unsigned int target)
{
	while (current_count > target) {
		struct slab_item *victim;

		victim = list_last_entry(&item_list, struct slab_item, node);
		free_single_item(victim);
		current_count--;
	}
}

static int resize_pool(unsigned int new_count)
{
	int ret = 0;

	if (new_count > current_count)
		ret = grow_pool(new_count);
	else if (new_count < current_count)
		shrink_pool(new_count);

	return ret;
}

static ssize_t slab_count_read(struct file *file, char __user *user_buf,
			      size_t count, loff_t *ppos)
{
	char buf[32];
	int len;
	unsigned int snapshot;

	mutex_lock(&item_lock);
	snapshot = current_count;
	mutex_unlock(&item_lock);

	len = scnprintf(buf, sizeof(buf), "%u\n", snapshot);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t slab_count_write(struct file *file, const char __user *user_buf,
			       size_t count, loff_t *ppos)
{
	char buf[32];
	unsigned int new_count;
	ssize_t len;
	int ret;

	if (count == 0)
		return 0;

	len = min_t(size_t, count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';

	ret = kstrtouint(buf, 0, &new_count);
	if (ret)
		return ret;

	mutex_lock(&item_lock);
	ret = resize_pool(new_count);
	mutex_unlock(&item_lock);

    printk(KERN_INFO MODULE_NAME ": resized pool to %u (ret=%d)\n", new_count, ret);

	return ret ? ret : count;
}

static const struct file_operations slab_count_fops = {
	.owner = THIS_MODULE,
	.read = slab_count_read,
	.write = slab_count_write,
};

static int __init slab_allocation_init(void)
{
	BUILD_BUG_ON(SLAB_ITEM_PAYLOAD_SIZE <= 0);
	BUILD_BUG_ON(sizeof(struct slab_item) != SLAB_ITEM_SIZE);

    // Initialize a slab cache. Use SLAB_NO_MERGE to prevent merging
	item_cache = kmem_cache_create(MODULE_NAME "_cache",
			sizeof(struct slab_item), 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!item_cache)
		return -ENOMEM;

	debugfs_dir = debugfs_create_dir(MODULE_NAME, NULL);
	if (IS_ERR(debugfs_dir) || !debugfs_dir) {
		kmem_cache_destroy(item_cache);
		return debugfs_dir ? PTR_ERR(debugfs_dir) : -ENODEV;
	}

	debugfs_file = debugfs_create_file("object_count", 0600, debugfs_dir,
					 NULL, &slab_count_fops);
	if (IS_ERR(debugfs_file) || !debugfs_file) {
		int err = debugfs_file ? PTR_ERR(debugfs_file) : -ENODEV;

		debugfs_remove_recursive(debugfs_dir);
		kmem_cache_destroy(item_cache);
		return err;
	}

	pr_info(MODULE_NAME ": initialized with empty cache\n");
	return 0;
}

static void __exit slab_allocation_exit(void)
{
	mutex_lock(&item_lock);
	shrink_pool(0);
	mutex_unlock(&item_lock);

	if (debugfs_dir)
		debugfs_remove_recursive(debugfs_dir);
	kmem_cache_destroy(item_cache);

	pr_info(MODULE_NAME ": exited and destroyed cache\n");
}

module_init(slab_allocation_init);
module_exit(slab_allocation_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("EPFL CS477");
MODULE_DESCRIPTION("A simple slab allocation example LKM");
MODULE_VERSION("1.0");
