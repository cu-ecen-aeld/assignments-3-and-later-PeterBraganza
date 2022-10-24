/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * 
 * @date 2019-10-22
 * Peter Braganza - Modification done for Char driver implementation
 * @date 08/23/2022
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

#define KMALLOC_ERROR_CHECK(x) \
        if(x == NULL) \
            goto EXIT;


int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Peter Braganza"); 
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    filp->private_data = container_of(inode->i_cdev, struct aesd_dev, cdev);

    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");

    return 0;
}


/**
 * @param filp is the file pointer to the char driver 
 * @param buf pointer to user buffer at which data is to be written from driver
 * @param count number of bytes to be read from driver
 * @param f_pos is the offset at which data is to be read
 * 
 * @brief wrill read count number of bytes from char driver and store it in buf.
 * @return number of bytes read
*/
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    struct aesd_buffer_entry *dequeed_buf;
    size_t entry_offset_byte = 0;
    size_t bytes_written = 0;
    char *write_buf = NULL;
    size_t bytes_not_written = 0;
    struct aesd_dev *ptr = filp->private_data;

    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);

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

    bytes_not_written = copy_to_user(buf, write_buf, bytes_written);
    *f_pos += bytes_written - bytes_not_written;
    retval = bytes_written - bytes_not_written;

    mutex_unlock(&ptr->lock);

    return retval;
}

/**
 * @param buf char buffer to be searched for '\n'
 * @param len length of the buffer
 * @brief used to find position of the return character in buffer
 * @return Returns the (position + 1) of '\n' if found. otherwise returns -1
*/
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

/**
 * @param filp is the file pointer to the char driver 
 * @param buf pointer to user buffer which contains data is to be written to driver
 * @param count number of bytes to be written to driver
 * @param f_pos is the offset at which data is to be written. Not used in out case
 * @return number of bytes written
*/
ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    //ssize_t retval = -ENOMEM;
    ssize_t retval = 0, num_of_bytes_not_copied;
    int pos = 0;
    char *working_buf, *write_buf, *lost_buf;
    struct aesd_buffer_entry write_entry;
    
    struct aesd_dev *ptr = filp->private_data;

    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);

    if(mutex_lock_interruptible(&ptr->lock) != 0)
    {
        return -ENOMEM;
    }
   
    working_buf = (char *)kmalloc( count * sizeof(char), GFP_KERNEL);
    KMALLOC_ERROR_CHECK(working_buf)
        
    num_of_bytes_not_copied = copy_from_user( working_buf, buf, count);
    //Use num_of_bytes_not_copied 

    while( pos != -1)
    {
        pos = find_return_pos(working_buf + retval, count - retval);
        if ( pos > 0 )
        {
            if(ptr->aesd_working_buf.size)
            {
                write_buf = (char *)krealloc(ptr->aesd_working_buf.buffptr , (pos + ptr->aesd_working_buf.size) * sizeof(char), GFP_KERNEL);
                KMALLOC_ERROR_CHECK(working_buf)
            }
            else
            {
                write_buf = (char *)kmalloc( (pos) * sizeof(char), GFP_KERNEL);
                KMALLOC_ERROR_CHECK(working_buf)
            }
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
                KMALLOC_ERROR_CHECK(working_buf)
            }
            else
            {
                write_buf = (char *)kmalloc( (count - retval) * sizeof(char), GFP_KERNEL);
                KMALLOC_ERROR_CHECK(working_buf)
            }

            memcpy(write_buf + ptr->aesd_working_buf.size, working_buf + retval, count - retval);
            ptr->aesd_working_buf.buffptr = write_buf;
            ptr->aesd_working_buf.size += count - retval;
            retval += (count - retval);
        }
    }

    // kfree(write_buf);
    kfree(working_buf);
    
    mutex_unlock(&ptr->lock);
    return retval;

    EXIT:
    mutex_unlock(&ptr->lock);
    return -ENOMEM;
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
    int out_offs, in_offs;
    dev_t devno = MKDEV(aesd_major, aesd_minor);
    //out_offs = aesd_device.aesd_circular_buf.entry[aesd_device.aesd_circular_buf.out_offs]
    out_offs = aesd_device.aesd_circular_buf.out_offs;
    in_offs = aesd_device.aesd_circular_buf.in_offs;
    cdev_del(&aesd_device.cdev);

    //destroying circular buffer
    for ( ; out_offs != in_offs  || aesd_device.aesd_circular_buf.full ; out_offs = (out_offs+1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
    {
        kfree(aesd_device.aesd_circular_buf.entry[out_offs].buffptr);
        aesd_device.aesd_circular_buf.full = false;
    }

    mutex_destroy(&aesd_device.lock);

    unregister_chrdev_region(devno, 1);
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
