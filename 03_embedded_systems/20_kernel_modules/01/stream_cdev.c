#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "stream_cdev.h"

#define DEVICE_NAME "stream_cdev"
#define MINOR_COUNT 1
#define BUF_SIZE 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AUTHOR");
MODULE_DESCRIPTION("DESCRIPTION");

static dev_t major_minor = 0;
static struct cdev my_cdev;
static buffer_t buffer;

static int dev_open(struct inode *inode, struct file *file)
{
    if (!try_module_get(THIS_MODULE))
        return -ENODEV;
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t size, loff_t *off)
{
    int read_len;
    int err_count = 0;

    if (mutex_lock_interruptible(&buffer.mtx))
        return -ERESTARTSYS;

    read_len = min((int)size, (int)buffer.len);
    if (read_len == 0)
    {
        mutex_unlock(&buffer.mtx);
        return 0;
    }
    err_count = copy_to_user(buf, buffer.bufp, read_len);
    if (err_count != 0)
    {
        mutex_unlock(&buffer.mtx);
        return -EFAULT;
    }
    if (read_len < buffer.len)
    {
        memmove(buffer.bufp, buffer.bufp + read_len, buffer.len - read_len);
    }
    buffer.len -= read_len;
    mutex_unlock(&buffer.mtx);
    return read_len;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
{
    int write_len;
    int err_count = 0;

    if (mutex_lock_interruptible(&buffer.mtx))
        return -ERESTARTSYS;

    write_len = min((int)size, (int)(buffer.capacity - buffer.len));
    if (write_len == 0)
    {
        mutex_unlock(&buffer.mtx);
        return -ENOSPC;
    }
    err_count = copy_from_user(buffer.bufp + buffer.len, buf, write_len);
    if (err_count != 0)
    {
        mutex_unlock(&buffer.mtx);
        return -EFAULT;
    }
    buffer.len += write_len;
    mutex_unlock(&buffer.mtx);
    return write_len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write};

static int __init dev_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&major_minor, 0, MINOR_COUNT, DEVICE_NAME);
    if (ret < 0)
        return ret;

    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    ret = cdev_add(&my_cdev, major_minor, MINOR_COUNT);
    if (ret < 0)
    {
        unregister_chrdev_region(major_minor, MINOR_COUNT);
        return ret;
    }
    buffer.bufp = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!buffer.bufp)
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(major_minor, MINOR_COUNT);
        return -ENOMEM;
    }
    memset(buffer.bufp, 0, BUF_SIZE);
    buffer.capacity = BUF_SIZE;
    buffer.len = 0;
    mutex_init(&buffer.mtx);

    pr_info(DEVICE_NAME " is loaded.\n");
    pr_info("Major number is %d.\n", MAJOR(major_minor));
    pr_info("Use mknod /dev/%s c %d 0\n", DEVICE_NAME, MAJOR(major_minor));
    return 0;
}

static void __exit dev_exit(void)
{
    kfree(buffer.bufp);
    cdev_del(&my_cdev);
    unregister_chrdev_region(major_minor, MINOR_COUNT);
}

module_init(dev_init);
module_exit(dev_exit);
