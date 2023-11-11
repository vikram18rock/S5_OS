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

// cdd - Character Device Driver

// Parameters
int kernel_version[2], time;

module_param(time, int, S_IRUSR | S_IWUSR);
module_param_array(kernel_version, int, NULL, S_IRUSR | S_IWUSR);

// variables

// device Number
dev_t dev_no;
// device class
struct class* dev_class;
// cdev struct
static struct cdev cdd_cdev;
// device file struct
static struct device* cdd_device;

// function prototypes for file operations
static int cdd_open(struct inode* inode, struct file* file);
static int cdd_release(struct inode* inode, struct file* file);
static ssize_t cdd_read(struct file* filp, char __user* buf, size_t len, loff_t* off);
static ssize_t cdd_write(struct file* filp, const char* buf, size_t len, loff_t* off);


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
    pr_info("Driver Read Function Called...!!!\n");
    return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t cdd_write(struct file* filp, const char __user* buf, size_t len, loff_t* off)
{
    pr_info("Driver Write Function Called...!!!\n");
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
    dev_class = class_create("cdd_device");

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
    // for returning normally if no errors are encountered
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

    return 0;
}

/*
** Module Exit function
*/
static void __exit cdd_exit(void)
{
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