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
	endpoint = usb_sndctrlpipe(dev, DEFAULT_CONTROL_ENDPOINT);
	command = kmalloc(commandSize, GFP_KERNEL);
	memcpy(command, commandStr, commandSize);
	res = usb_control_msg(dev, endpoint, CONTROL_REQUEST, CONTROL_REQUEST_TYPE, 0x00, 0x00, command, commandSize, 0);
	if (res < 0) {
		printk(KERN_ALERT "Ben here: failed to send control message with error %d", res);
	}
	kfree(command);


}

static int cannon_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct cannon_info * cannon_device_info = NULL;
	cannon_device_info = kmalloc(sizeof(struct cannon_info), GFP_KERNEL);
	memset(cannon_device_info, 0x00, sizeof(struct cannon_info));
	cannon_device_info->dev = usb_get_dev(interface_to_usbdev(interface));
	cannon_device_info->interface = interface;
	usb_set_intfdata(interface, cannon_device_info->dev);
	usb_register_dev(interface, &cannon_class);
	dev_info(&interface->dev, "Device attached to %d", interface->minor);
	printk(KERN_ALERT "Ben here: My cannon has been plugged in");
	return 0;
}

static void cannon_disconnect(struct usb_interface *interface) {
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
	struct cannon_info *cannon_device_info;
	int minorNum;
	minorNum = iminor(inode);
	interface = usb_find_interface(&cannon_driver, minorNum);
	cannon_device_info= usb_get_intfdata(interface);
	file->private_data = cannon_device_info;
	return 0;
}

static ssize_t cannon_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos) {
	struct cannon_info *cannon_device_info;
	int max_input_size = 4;
	char * buffer = NULL;

	cannon_device_info = (struct cannon_info *) file->private_data;
	buffer = kmalloc(GFP_KERNEL, max_input_size);
	copy_from_user(buffer, user_buffer, max_input_size);
	if (strcmp(buffer, "up")) {
		move_up(cannon_device_info->dev, cannon_device_info->interface);
	}
	else {
	//do nothing for now
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
	.minor_base = 192, //NOTE MIGHT BE WRONG AND NEED TO GET MINOR RANGE! 
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

