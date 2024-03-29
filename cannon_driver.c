//Written By: Benjamin Erichsen

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
#define USB_COMMAND_SIZE 8

MODULE_LICENSE("Dual BSD/GPL");

struct cannon_info {
	struct usb_device * dev;
	struct usb_interface * interface;
};

struct entry { 
	char * commandFromUser;
	unsigned char usbCommand;
};

struct entry command_mapping[] = { 
		"down", 0x01,
		"left", 0x04,
		"right", 0x08,
		"stop", 0x20,
		"up", 0x02,
		"fire", 0x010,
		 NULL, 0 
};

static struct usb_driver cannon_driver;
static struct usb_class_driver cannon_class;

static unsigned char * createUSBCommand(struct usb_device * dev, unsigned char command);
static void send_control_command(struct cannon_info*, unsigned char);

static int __init cannon_init(void) {
	int register_result;
	register_result = usb_register(&cannon_driver);
	if (register_result < 0) {
		printk(KERN_ALERT "Failed to register cannon driver. Erro number %d", register_result);
		return register_result;
	}
	printk(KERN_ALERT "Registered cannon driver");
	return 0;
}

static int cannon_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int register_device_result;
	struct cannon_info * cannon_device_info = NULL;
	cannon_device_info = kzalloc(sizeof(struct cannon_info), GFP_KERNEL);
	if (!cannon_device_info) {
		printk(KERN_ALERT "Kernel is out of memory");
		return -1;
	}

	cannon_device_info->dev = usb_get_dev(interface_to_usbdev(interface));
	cannon_device_info->interface = interface;
	usb_set_intfdata(interface, cannon_device_info);
	
	register_device_result = usb_register_dev(interface, &cannon_class);
	if (register_device_result) {
		printk(KERN_ALERT "Failed to register device with error code %d", register_device_result);
		return register_device_result;
	}
	dev_info(&interface->dev, "Cannon device attached to %d", interface->minor);
	printk(KERN_ALERT "Cannon successfully connected to computer");
	return 0;
}

static int cannon_open(struct inode *inode, struct file *file) { 
	struct usb_interface *interface;
	struct cannon_info *cannon_device_info = NULL;
	int minorNum;
	minorNum = iminor(inode);
	interface = usb_find_interface(&cannon_driver, minorNum);
	if (!interface) {
		printk(KERN_ALERT "Could not find interface for minor number");
		return -1;	
	}
	cannon_device_info = usb_get_intfdata(interface);
	if (!cannon_device_info) {
		printk(KERN_ALERT "Could not get cannon device info");
		return -1;
	}
	file->private_data = cannon_device_info;
	printk(KERN_ALERT "Opened cannon device");
	return 0;
}

static ssize_t cannon_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos) {
	struct cannon_info *cannon_device_info;
	const int max_user_command_size = 6;
	char buffer[max_user_command_size];
	int i = 0;
	char *nextEntry = NULL;
	int bytesToRead = count <= max_user_command_size ? count : max_user_command_size;
	cannon_device_info = (struct cannon_info *) file->private_data;
	if (copy_from_user(buffer, user_buffer, bytesToRead)) {
		printk(KERN_ALERT "Failed to copy data from user");
	}
	nextEntry = command_mapping[i].commandFromUser;
	while (nextEntry) {
		if (strcmp(buffer, command_mapping[i].commandFromUser) == 0) {
			send_control_command(cannon_device_info, command_mapping[i].usbCommand);
			return 1;
		}
		else { nextEntry = command_mapping[++i].commandFromUser; }
	}
	printk(KERN_ALERT "%s is an undefined command for cannon", buffer);
	return -1;

}

static void cannon_disconnect(struct usb_interface *interface) {
	struct cannon_info * dev;
	
	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	
	usb_deregister_dev(interface, &cannon_class);
	printk(KERN_ALERT "Removed cannon from the computer");
}


static struct file_operations cannon_fops = { 
	.owner = THIS_MODULE,
	.open  = cannon_open,
	.write = cannon_write,
};

static struct usb_device_id cannon_table[] = {
        { USB_DEVICE(CANNON_VENDOR_ID, CANNON_PRODUCT_ID) },
        {}
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

static void __exit cannon_exit(void) {
	printk(KERN_ALERT "Removed cannon driver");
	usb_deregister(&cannon_driver);
 }


MODULE_DEVICE_TABLE(usb, cannon_table);

module_init(cannon_init);
module_exit(cannon_exit);

static void send_control_command(struct cannon_info *cannon_info, unsigned char command) {
	int res;
	unsigned int endpoint = usb_sndctrlpipe(cannon_info->dev, DEFAULT_CONTROL_ENDPOINT);
	unsigned char *usbCommand = createUSBCommand(cannon_info->dev, command);
	res = usb_control_msg(cannon_info->dev, endpoint, CONTROL_REQUEST, CONTROL_REQUEST_TYPE,
		       	0x00, 0x00, usbCommand, USB_COMMAND_SIZE, 0);
	if (res < 0) {
		printk(KERN_ALERT "Failed to send up command control message with error %d", res);
	}
	kfree(usbCommand);
}

static unsigned char * createUSBCommand(struct usb_device * dev, unsigned char command) {
	unsigned char * usbCommand = NULL;
	unsigned char commandStr[USB_COMMAND_SIZE] = {0x02, command, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	usbCommand = kmalloc(USB_COMMAND_SIZE, GFP_KERNEL);
	memcpy(usbCommand, commandStr, USB_COMMAND_SIZE);
	return usbCommand; 
}

