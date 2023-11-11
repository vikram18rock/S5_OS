#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>   // passing arguments
#include<linux/version.h>       // for capturing compiled linux version
#include<linux/fs.h> 
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/err.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/wait.h>
#include<linux/kthread.h>
#include<linux/timer.h>
#include<linux/jiffies.h>

// cdd - Character Device Driver

// Parameters
int kernel_version[2], time;

module_param(time, int, S_IRUSR | S_IWUSR);
module_param_array(kernel_version, int, NULL, S_IRUSR | S_IWUSR);

// variables
#define default_size 1024
static int used_len;
#define TIMEOUT 1000        // milli seconds

// device Number
dev_t dev_no;
// device class
static struct class* dev_class;
// cdev struct
static struct cdev cdd_cdev;
// device file struct
static struct device* cdd_device;
// cdd's kernel space buffer pointer
uint8_t* cdd_kernel_buffer;
// wait thread
static struct task_struct* wait_thread;
// wait queue head and usageflag
wait_queue_head_t wait_queue_cdd;
static int wait_queue_flag = 0;
// User name array
char user_name[10];
// timer declaration
static struct timer_list cdd_timer;
// timer expired flag
static int timer_expired = 0;

// read count
static uint32_t read_count = 0;

// function prototypes for file operations
static int cdd_open(struct inode* inode, struct file* file);
static int cdd_release(struct inode* inode, struct file* file);
static ssize_t cdd_read(struct file* filp, char __user* buf, size_t len, loff_t* off);
static ssize_t cdd_write(struct file* filp, const char* buf, size_t len, loff_t* off);

//Timer Callback function. This will be called when timer expires
void timer_callback(struct timer_list* data)
{
    pr_info("Timer of %d secs Expired\n", time);
    timer_expired = 1;
}

// file operations for the cdev struct
struct file_operations f_ops =
{
    .owner = THIS_MODULE,
    .read = cdd_read,
    .write = cdd_write,
    .open = cdd_open,
    .release = cdd_release,
};

/*
** function for the kernle thread
*/
static int wait_function(void* unused)
{
    // purpose is to print no. of reads and capture event of exit 
    while (1) {
        pr_info("Waiting For Event...\n");
        wait_event_interruptible(wait_queue_cdd, wait_queue_flag != 0);
        if (wait_queue_flag == 2 && read_count) {
            pr_info("Event Came From Write after Read Function\n");

            // indicating desired order of events
            wait_queue_flag = 3;

            memcpy(user_name, cdd_kernel_buffer, used_len);
            return 0;
        }
        pr_info("Event Came From Read Function - %d\n", ++read_count);
        wait_queue_flag = 0;
    }
    do_exit(0);
    return 0;
}

/*
** This function will be called when we open the Device file
*/
static int cdd_open(struct inode* inode, struct file* file)
{
    pr_info("Driver Open Function Called...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int cdd_release(struct inode* inode, struct file* file)
{
    pr_info("Driver Release Function Called...!!!\n");
    return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t cdd_read(struct file* filp, char __user* buf, size_t len, loff_t* off)
{
    used_len = strlen(cdd_kernel_buffer);
    pr_info("Device Read Function Called....!!\n");
    /* If off is behind the end of a file we have nothing to read */
    if (*off >= used_len)
        return 0;
    /* If a user tries to read more than we have, read only as many bytes as we have */
    if (*off + len > used_len)
        len = (used_len - *off);
    if (copy_to_user(buf, cdd_kernel_buffer + *off, len) != 0) {
        pr_err("Data Read : ERR!\n");
        return -EFAULT;
    }
    /* Move reading off */
    *off += len;

    // wake up queue if a read signal is encountered
    wait_queue_flag = 1;
    wake_up_interruptible(&wait_queue_cdd);
    return len;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t cdd_write(struct file* filp, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("Driver Write Function Called...!!! off is %lld\n", *off);
    // if the offset is beyond our space
    if (*off >= default_size) {
        return 0;
    }
    // if user tries to access more than size we have
    if (*off + len > default_size) {
        len = default_size - *off;
    }
    if (copy_from_user(cdd_kernel_buffer + *off, buf, len))
    {
        pr_err("Data Wrie : ERR!\n");
        return -EFAULT;
    }
    // Move writing off
    *off += len;

    // wakeup wait queue if order satisfied
    wait_queue_flag = 2;
    wake_up_interruptible(&wait_queue_cdd);

    return len;
}

/*
** Module Init function
*/
static int __init cdd_init(void)
{
    // checking kernel version
    if (KERNEL_VERSION(kernel_version[0], kernel_version[1], 0) != KERNEL_VERSION(LINUX_VERSION_MAJOR, LINUX_VERSION_PATCHLEVEL, 0)) {
        pr_err("Not compatible with this Kernel Version\n");
        return -1;
    }

    pr_info("Kernel Module Inserted Successfully...\n");

    // allocate device number
    if ((alloc_chrdev_region(&dev_no, 0, 1, "cdd_device")) < 0) {
        pr_err("Unable to allocated device number\n");
        return -1;
    }

    // printing major no. and minor no. of device no.
    pr_info("Major No. is %d \nMinor Number is %d \n", MAJOR(dev_no), MINOR(dev_no));

    // create device class
    dev_class = class_create("cdd_class");

    if (IS_ERR(dev_class)) {
        pr_err("Unable to create struct class for the device\n");
        goto r_class;
    }

    // create device
    if (IS_ERR(cdd_device = device_create(dev_class, NULL, dev_no, NULL, "cdd_device"))) {
        pr_err("Unable to create device\n");
        goto r_device;
    }

    // cdev entry
    // initialize cdev entry
    cdev_init(&cdd_cdev, &f_ops);

    // add the cdev entry to system
    if (cdev_add(&cdd_cdev, dev_no, 1) < 0) {
        pr_err("Unable to add Cdev entry\n");
        goto r_cdev;
    }

    // allocating kernel space memory for our driver
    if (IS_ERR(cdd_kernel_buffer = kmalloc(default_size, GFP_KERNEL))) {
        pr_err("Unable to allocate kernel space for the driver\n");
        goto r_cdev;
    }
    strcpy(cdd_kernel_buffer, "Default\n");

    /* setup your timer to call timer_callback */
    timer_setup(&cdd_timer, timer_callback, 0);

    mod_timer(&cdd_timer, jiffies + msecs_to_jiffies(time * TIMEOUT));
    pr_info("Timer Started\n");

    // initializing a waitqueue
    init_waitqueue_head(&wait_queue_cdd);

    // create a wait thread named "waitThread"
    wait_thread = kthread_create(wait_function, NULL, "WaitThread");
    pr_info("Creating a Thred on wait function");
    if (wait_thread) {
        pr_info("Thread Created successfully\n");
        wake_up_process(wait_thread);
    }
    else {
        pr_info("Thread creation failed\n");
    }

    // for returning normally if no errors are enlenered
    return 0;

    // if the entry is not added due to error
r_cdev:
    device_del(cdd_device);

    // if device can't be created destroy the created class and dev_no
r_device:
    class_destroy(dev_class);

    // if class can't be created unregister the dev_no
r_class:
    unregister_chrdev_region(dev_no, 1);

    return -1;
}

/*
** Module Exit function
*/
static void __exit cdd_exit(void)
{
    // remove timer while unloading the module
    del_timer(&cdd_timer);

    if (wait_queue_flag == 3 && !timer_expired) {
        pr_info("Successfully completed the actions within time\nUser Name is %s", user_name);
    }
    else
        pr_info("Failure\n");

    // freeing the space we used
    kfree(cdd_kernel_buffer);

    // delete cdev entry
    cdev_del(&cdd_cdev);

    // delete device
    device_del(cdd_device);

    // destroy the device class
    class_destroy(dev_class);

    // unregistering the device number while exiting
    unregister_chrdev_region(dev_no, 1);

    pr_info("Kernel Module Removed Successfully...\n");
}

// Register init and exit functions
module_init(cdd_init);
module_exit(cdd_exit);

// License
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PONNURU AADARSH");
MODULE_DESCRIPTION("A CHARACTER DEVICE DRIVER");
MODULE_VERSION("2:1.0");