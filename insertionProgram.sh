rmmod usbhid
make -C /lib/modules/$(uname -r)/build M=`pwd` modules
insmod cannon_driver.ko
