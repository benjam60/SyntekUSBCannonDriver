//Commands to get to work:
//make -C /lib/modules/$(uname -r)/build M=`pwd` modules
//sudo rmmod usbhid
//sudo insmod cannon_driver.ko

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define CANNON_VENDOR_ID 0x2123
#define CANNON_PRODUCT_ID 0x1010

#define DEFAULT_CONTROL_ENDPOINT 0

#define CONTROL_REQUEST 0x09
#define CONTROL_REQUEST_TYPE 0x21

MODULE_LICENSE("Dual BSD/GPL");

struct cannon_info {
	struct usb_device * dev;
	struct usb_interface * interface;
};

static struct usb_driver cannon_driver;
static struct usb_class_driver cannon_class;

static void move_up(struct usb_device *dev, struct usb_interface *interface) {
	unsigned int endpoint;
	void * command;
	int res;
	unsigned char commandStr[8] = {0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
	int commandSize = 8;
	printk(KERN_ALERT "CANNON WRITE STEP 1");
	endpoint = usb_sndctrlpipe(dev, DEFAULT_CONTROL_ENDPOINT);
	printk(KERN_ALERT "CANNON WRITE STEP 2");
	command = kmalloc(commandSize, GFP_KERNEL);
	printk(KERN_ALERT "CANNON WRITE STEP 3");
	memcpy(command, commandStr, commandSize);
	printk(KERN_ALERT "CANNON WRITE STEP 4");
	res = usb_control_msg(dev, endpoint, CONTROL_REQUEST, CONTROL_REQUEST_TYPE, 0x00, 0x00, command, commandSize, 0);
	printk(KERN_ALERT "CANNON WRITE STEP 5");
	if (res < 0) {
		printk(KERN_ALERT "Ben here: failed to send control message with error %d", res);
	}
	kfree(command);


}

static int cannon_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int retval;
	struct cannon_info * cannon_device_info = NULL;
	cannon_device_info = kzalloc(sizeof(struct cannon_info), GFP_KERNEL);
	if (!cannon_device_info) {
		printk(KERN_ALERT "device is out of memory");
		return 1;
	}

	cannon_device_info->dev = usb_get_dev(interface_to_usbdev(interface));
	cannon_device_info->interface = interface;
	usb_set_intfdata(interface, cannon_device_info);
	
	retval = usb_register_dev(interface, &cannon_class);
	if (retval) {
		printk(KERN_ALERT "failed to register device with error code %d", retval);
		return retval;
	}
	dev_info(&interface->dev, "Device attached to %d", interface->minor);
	printk(KERN_ALERT "Ben here: My cannon has been plugged in");
	return 0;
}

static void cannon_disconnect(struct usb_interface *interface) {
	struct cannon_info * dev;
	
	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	
	usb_deregister_dev(interface, &cannon_class);
	printk(KERN_ALERT "Ben here: Removed cannon from the computer");
}

static struct usb_device_id cannon_table[] = {
        { USB_DEVICE(CANNON_VENDOR_ID, CANNON_PRODUCT_ID) },
        {}
};

MODULE_DEVICE_TABLE(usb, cannon_table);

static int cannon_open(struct inode *inode, struct file *file) { 
	struct usb_interface *interface;
	struct cannon_info *cannon_device_info = NULL;
	int minorNum;
	minorNum = iminor(inode);
	interface = usb_find_interface(&cannon_driver, minorNum);
	if (!interface) {
		printk(KERN_ALERT "could not find interface for minor number");
		return 1;	
	}
	cannon_device_info = usb_get_intfdata(interface);
	if (!cannon_device_info) {
		printk(KERN_ALERT "could not get cannon device info");
		return 1;
	}
	file->private_data = cannon_device_info;
	printk(KERN_ALERT "cannon driver is opened");
	return 0;
}

static ssize_t cannon_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos) {
	struct cannon_info *cannon_device_info;
	int max_input_size = 4;
	char * buffer = NULL;
	printk(KERN_ALERT "CANNON WRITE STEP -1");
	cannon_device_info = (struct cannon_info *) file->private_data;
	buffer = kmalloc(max_input_size, GFP_KERNEL);
	printk(KERN_ALERT "CANNON WRITE STEP 0");
	if (copy_from_user(buffer, user_buffer, max_input_size) < 0) {
		printk(KERN_ALERT "failed to copy from user");
	}
	if (strcmp(buffer, "up") == 0) {
		unsigned int endpoint;
		void * command;
		int res;
	unsigned char commandStr[8] = {0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
	int commandSize = 8;
	printk(KERN_ALERT "CANNON WRITE STEP 1");
	endpoint = usb_sndctrlpipe(cannon_device_info->dev, DEFAULT_CONTROL_ENDPOINT);
	printk(KERN_ALERT "CANNON WRITE STEP 2");
	command = kmalloc(commandSize, GFP_KERNEL);
	printk(KERN_ALERT "CANNON WRITE STEP 3");
	memcpy(command, commandStr, commandSize);
	printk(KERN_ALERT "CANNON WRITE STEP 4");
	res = usb_control_msg(cannon_device_info->dev, endpoint, CONTROL_REQUEST, CONTROL_REQUEST_TYPE, 0x00, 0x00, command, commandSize, 0);
	printk(KERN_ALERT "CANNON WRITE STEP 5");
	if (res < 0) {
		printk(KERN_ALERT "Ben here: failed to send control message with error %d", res);
	}
	kfree(command);

		//move_up(cannon_device_info->dev, cannon_device_info->interface);
	}
	kfree(buffer);
	return 1;

}

static struct file_operations cannon_fops = { 
	.owner = THIS_MODULE,
	.open  = cannon_open,
	.write = cannon_write,
};

static struct usb_driver cannon_driver =
{
        .name = "Ben's canon driver",
        .id_table = cannon_table,
        .probe = cannon_probe,
        .disconnect = cannon_disconnect,
};
static struct usb_class_driver cannon_class = {
	.name = "usb/cannon",
	.fops = &cannon_fops,
	.minor_base = 5,
};

static int __init cannon_init(void) {
        int result;
	result = usb_register(&cannon_driver);
	if (result < 0) {
		printk(KERN_ALERT "usb_register failed. Erro number %d", result);
		return -1;
	}
	printk(KERN_ALERT "Ben here: Registered cannon driver");
        return 0;
}

static void __exit cannon_exit(void) {
	printk(KERN_ALERT "Ben here: removed cannon driver");
	usb_deregister(&cannon_driver);
 }

module_init(cannon_init);
module_exit(cannon_exit);

