//Commands to get to work:
//make -C /lib/modules/$(uname -r)/build M=`pwd` modules
//sudo rmmod usbhid
//sudo insmod cannon_driver.ko

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/init.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");

static int cannon_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int i = 0;
	struct usb_host_interface *iface_desc;
	//try command
	struct usb_device * dev; 
	unsigned int endpoint;
	int commandSize = 8; 
	printk(KERN_ALERT "Ben here: My cannon has been plugged in");
	dev = interface_to_usbdev(interface);
	iface_desc = interface->cur_altsetting;
	
	//register dev
	//int res1 = usb_register_dev(interface, dev);
	//if (res1) { printk(KERN_ALERT "Failed to register dev");  }

	endpoint = usb_sndctrlpipe(dev, 0); //what is the endpoint number?
	usb_clear_halt(dev, endpoint);
	void * command = kmalloc(commandSize, GFP_KERNEL);
	unsigned char commandStr[8] = {0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
	memcpy(command, commandStr, commandSize);
	printk(KERN_ALERT "Ben here: about to send control message");
	int res = usb_control_msg(dev, endpoint,
					 0x09, //request
				         0x21, //request type
					 0x00, 0x00, command, commandSize, 0);
	if (res < 0) {
		printk(KERN_ALERT "Ben here: failed to send control message with error %d", res);
	}
	return 0;
}

static void cannon_disconnect(struct usb_interface *interface) {
	printk(KERN_ALERT "Ben here: Removed cannon from the computer");
}

static struct usb_device_id cannon_table[] = {
        { USB_DEVICE(0x2123, 0x1010) },
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

