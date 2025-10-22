#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "sys_buffer.h"


#define KOBJ_NAME "sysfs_buffer"
#define ATTR_NAME buffer_data
#define BUF_SIZE 512 // < 4096 !!

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AUTHOR");
MODULE_DESCRIPTION("DESCRIPTION");

static buffer_t buffer;
static struct kobject *my_kobject;

static ssize_t sys_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ssize_t read_len;

    if (mutex_lock_interruptible(&buffer.mtx))
        return -ERESTARTSYS;

    read_len = scnprintf(buf, buffer.len, "%.*s", (int)buffer.len, buffer.bufp);

    mutex_unlock(&buffer.mtx);
    return read_len;
}

static ssize_t sys_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int write_len = count;

    if (write_len >= buffer.capacity)
        return -ENOSPC;

    if (mutex_lock_interruptible(&buffer.mtx))
        return -ERESTARTSYS;

    strncpy(buffer.bufp, buf, write_len);
    buffer.len = write_len + 1;
    buffer.bufp[write_len] = '\0';

    mutex_unlock(&buffer.mtx);
    return count;
}

static struct kobj_attribute buffer_attr = 
    __ATTR(ATTR_NAME, 0660, sys_show, sys_store);

static int __init sys_init(void)
{
    buffer.bufp = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!buffer.bufp)
        return -ENOMEM;

    memset(buffer.bufp, 0, BUF_SIZE);
    buffer.capacity = BUF_SIZE;
    buffer.len = 0;
    mutex_init(&buffer.mtx);

    my_kobject = kobject_create_and_add(KOBJ_NAME, kernel_kobj);
    if (!my_kobject)
    {
        kfree(buffer.bufp);
        return -ENOMEM;
    }
    
    int ret = 0;
    ret = sysfs_create_file(my_kobject, &buffer_attr.attr);
    if (ret)
    {
        kobject_put(my_kobject);
        kfree(buffer.bufp);
        return ret;
    }

    pr_info("%s: Sysfs entry created at /sys/kernel/%s/%s\n", KOBJ_NAME, KOBJ_NAME, __stringify(ATTR_NAME));
    return 0;
}

static void __exit sys_exit(void)
{
    if (my_kobject)
        kobject_put(my_kobject);

    kfree(buffer.bufp);
}

module_init(sys_init);
module_exit(sys_exit);
