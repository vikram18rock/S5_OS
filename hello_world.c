#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>   // passing arguments
#include<linux/version.h> // for capturing compiled linux version
 
// cdd - Character Device Driver

// Parameters
int kernel_version[2], time;

module_param(time, int, S_IRUSR | S_IWUSR);
module_param_array(kernel_version, int, NULL, S_IRUSR | S_IWUSR);

/*
** Module Init function
*/
static int __init cdd_init(void)
{
    if (KERNEL_VERSION(kernel_version[0], kernel_version[1], 0) != KERNEL_VERSION(LINUX_VERSION_MAJOR, LINUX_VERSION_PATCHLEVEL, 0)) {
        printk(KERN_INFO "Not compatible with this Kernel Version\n");
        return -1;
    }
    printk(KERN_INFO "Kernel Module Inserted Successfully...\n");
    return 0;
}

/*
** Module Exit function
*/
static void __exit cdd_exit(void)
{
    printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}

// Register init and exit functions
module_init(cdd_init);
module_exit(cdd_exit);

// License
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PONNURU AADARSH");
MODULE_DESCRIPTION("A CHARACTER DEVICE DRIVER");
MODULE_VERSION("2:1.0");