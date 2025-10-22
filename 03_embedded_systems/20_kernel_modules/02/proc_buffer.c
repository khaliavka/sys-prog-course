#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "proc_buffer.h"

#define PROC_NAME "proc_buffer"
#define BUF_SIZE 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AUTHOR");
MODULE_DESCRIPTION("DESCRIPTION");

static struct proc_dir_entry *proc_entry;
static buffer_t buffer;

static ssize_t proc_read(struct file *file, char __user *buf, size_t size, loff_t *off)
{
    int read_len;
    int err_count = 0;

    if (*off > 0)
        return 0;

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

static ssize_t proc_write(struct file *file, const char __user *buf, size_t size, loff_t *off)
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

static struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write
};

static int __init proc_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    if (!proc_entry)
        return -ENOMEM;
    
    buffer.bufp = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!buffer.bufp)
        return -ENOMEM;

    memset(buffer.bufp, 0, BUF_SIZE);
    buffer.capacity = BUF_SIZE;
    buffer.len = 0;
    mutex_init(&buffer.mtx);

    pr_info("%s: /proc/%s created.\n", PROC_NAME, PROC_NAME);
    return 0;
}

static void __exit proc_exit(void)
{
    if (proc_entry)
        remove_proc_entry(PROC_NAME, NULL);

    kfree(buffer.bufp);
    pr_info("%s: /proc/%s deleted.\n", PROC_NAME, PROC_NAME);
}

module_init(proc_init);
module_exit(proc_exit);
