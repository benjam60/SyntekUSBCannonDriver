//Commands to get to work:
//make -C /lib/modules/$(uname -r)/build M=`pwd` modules
//sudo rmmod usbhid
//sudo insmod cannon_driver.ko

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/init.h>
#include <linux/slab.h>

#define CANNON_VENDOR_ID 0x2123
#define CANNON_PRODUCT_ID 0x1010

#define DEFAULT_CONTROL_ENDPOINT 0

#define CONTROL_REQUEST 0x09
#define CONTROL_REQUEST_TYPE 0x21

MODULE_LICENSE("Dual BSD/GPL");

static int cannon_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device * dev; 
	unsigned int endpoint;
	void * command;
	int res;
	unsigned char commandStr[8] = {0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
	int commandSize = 8; 
	dev = interface_to_usbdev(interface);
	endpoint = usb_sndctrlpipe(dev, DEFAULT_CONTROL_ENDPOINT);
	command = kmalloc(commandSize, GFP_KERNEL);
	memcpy(command, commandStr, commandSize);
	res = usb_control_msg(dev, endpoint, CONTROL_REQUEST, CONTROL_REQUEST_TYPE, 0x00, 0x00, command, commandSize, 0);
	if (res < 0) {
		printk(KERN_ALERT "Ben here: failed to send control message with error %d", res);
	}
	kfree(command);
	printk(KERN_ALERT "Ben here: My cannon has been plugged in");
	return 0;
}

static void cannon_disconnect(struct usb_interface *interface) {
	printk(KERN_ALERT "Ben here: Removed cannon from the computer");
}

static struct usb_device_id cannon_table[] = {
        { USB_DEVICE(CANNON_VENDOR_ID, CANNON_PRODUCT_ID) },
        {}
};

MODULE_DEVICE_TABLE(usb, cannon_table);

static struct usb_driver cannon_driver =
{
        .name = "Ben's canon driver",
        .id_table = cannon_table,
        .probe = cannon_probe,
        .disconnect = cannon_disconnect
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

