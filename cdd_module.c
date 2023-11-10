#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>   // passing arguments
#include<linux/version.h>       // for capturing compiled linux version
#include<linux/fs.h> 
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/err.h>

// cdd - Character Device Driver

// Parameters
int kernel_version[2], time;

// variables

// device Number
dev_t dev_no;
// device class
struct class * dev_class;


module_param(time, int, S_IRUSR | S_IWUSR);
module_param_array(kernel_version, int, NULL, S_IRUSR | S_IWUSR);

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

    if(IS_ERR(dev_class)) {
        pr_err("Unable to create struct class for the device\n");
        goto r_class;
    }

    // create device
    if (IS_ERR(device_create(dev_class, NULL, dev_no, NULL, "cdd_device"))) {
        pr_err("Unable to create device\n");
        goto r_device;
    }

    // if device can't be created destroy the created class and dev_no
    r_device:
        class_destroy(dev_class);

    // if class can't be created unregister the dev_no
    r_class:
        unregister_chrdev_region(dev_no);

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