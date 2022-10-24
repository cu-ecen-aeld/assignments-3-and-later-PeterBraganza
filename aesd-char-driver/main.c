/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include <linux/fs.h> // file_operations
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Your Name Here"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    /**
     * TODO: handle open
     */

    filp->private_data = container_of(inode->i_cdev, struct aesd_dev, cdev);
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    struct aesd_dev *ptr = filp->private_data;
    struct aesd_buffer_entry *dequeed_buf;
    size_t entry_offset_byte = 0;
    size_t bytes_written = 0;
    char *write_buf = NULL;

    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */


    if(mutex_lock_interruptible(&ptr->lock) != 0)
    {
        return -ENOMEM;
    }

    dequeed_buf = aesd_circular_buffer_find_entry_offset_for_fpos(&ptr->aesd_circular_buf, *f_pos, &entry_offset_byte);
    if(dequeed_buf == NULL)
    {
        *f_pos = 0;
        mutex_unlock(&ptr->lock);
        return retval;
    }

    write_buf = (char *)(dequeed_buf->buffptr + entry_offset_byte);
    if(count < dequeed_buf->size - entry_offset_byte )
    {
        bytes_written = count;
    }
    else
    {
        bytes_written = dequeed_buf->size - entry_offset_byte;
    }

    copy_to_user(buf, write_buf, bytes_written);
    *f_pos += bytes_written;

    mutex_unlock(&ptr->lock);

    retval = bytes_written;

    return retval;
}


/* Returns the (position + 1) of '\n' if found. otherwise returns -1*/
int find_return_pos(char *buf, size_t len)
{
    int i; 

    for( i = 0; i < len ; i++)
    {
        if( *(buf+i) == '\n')
            return i+1;
    }

    return -1;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    //ssize_t retval = -ENOMEM;
    ssize_t retval = 0;
    int pos = 0;
    char *working_buf, *write_buf, *lost_buf;
    struct aesd_buffer_entry write_entry;
    struct aesd_dev *ptr = filp->private_data;

    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle write
     */

    
    if(mutex_lock_interruptible(&ptr->lock) != 0)
    {
        return -ENOMEM;
    }
   
    working_buf = (char *)kmalloc( count * sizeof(char), GFP_KERNEL);
    if(working_buf == NULL)
    {
        retval = -ENOMEM;
        mutex_unlock(&ptr->lock);
        return -ENOMEM;
        //return after freeing things 
    }
        

    copy_from_user( working_buf, buf, count);


    while( pos != -1)
    {
        pos = find_return_pos(working_buf + retval, count - retval);
        if ( pos > 0 )
        {
            if(ptr->aesd_working_buf.size)
            {
                write_buf = (char *)krealloc(ptr->aesd_working_buf.buffptr , (pos + ptr->aesd_working_buf.size) * sizeof(char), GFP_KERNEL);
            }
            else
            {
                write_buf = (char *)kmalloc( (pos) * sizeof(char), GFP_KERNEL);
            }
            //TODO: handle for data in aesd_working_buf
            memcpy(write_buf + ptr->aesd_working_buf.size, working_buf + retval, pos);
            write_entry.buffptr = write_buf;
            write_entry.size = pos + ptr->aesd_working_buf.size;
            lost_buf = aesd_circular_buffer_add_entry(&ptr->aesd_circular_buf, &write_entry);
            if(lost_buf != NULL)
            {
                kfree(lost_buf);
            }
            ptr->aesd_working_buf.size = 0;
            retval += pos;
        }
        if ( pos == -1)
        {
            if(ptr->aesd_working_buf.size)
            {
                write_buf = (char *)krealloc(ptr->aesd_working_buf.buffptr , ((count - retval) + ptr->aesd_working_buf.size) * sizeof(char) , GFP_KERNEL);
            }
            else
            {
                write_buf = (char *)kmalloc( (count - retval) * sizeof(char), GFP_KERNEL);
            }

            memcpy(write_buf + ptr->aesd_working_buf.size, working_buf + retval, count - retval);
            //Need to think about use of write_buf as it gets freeeeeeed after exit from from funtion 
            ptr->aesd_working_buf.buffptr = write_buf;
            ptr->aesd_working_buf.size += count - retval;
            retval += (count - retval);
        }
    }

    // kfree(write_buf);
    kfree(working_buf);
    

    mutex_unlock(&ptr->lock);

    return retval;
}


struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */

    //Initialize mutex
    mutex_init(&aesd_device.lock);
   

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    mutex_destroy(&aesd_device.lock);

    unregister_chrdev_region(devno, 1);
}






module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
