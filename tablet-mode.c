/*
 * Tablet Mode Driver
 * Adapted from yoga-usage-mode (initially) for Chuwi Hi10 Max
 *
 * Detects attach/detach of the keyboard
 * and reports SW_TABLET_MODE accordingly via the Linux input subsystem.
 *
 * Original yoga-usage-mode author: lukas-w
 * Adaptation: Johannes Hörmann
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/usb.h>

#ifndef KEYBOARD_VENDOR
#define KEYBOARD_VENDOR  0x258a
#endif

#ifndef KEYBOARD_PRODUCT
#define KEYBOARD_PRODUCT 0x0020
#endif

static struct input_dev *device_input;
static struct notifier_block usb_nb;

/*
 * USB event handler:
 * toggles SW_TABLET_MODE when the keyboard is attached or removed.
 */
static int usb_notify(struct notifier_block *nb, unsigned long action, void *data)
{
	struct usb_device *udev = data;

	if (!udev)
		return NOTIFY_DONE;

	if (udev->descriptor.idVendor == KEYBOARD_VENDOR &&
	    udev->descriptor.idProduct == KEYBOARD_PRODUCT) {

		switch (action) {
		case USB_DEVICE_REMOVE:
			input_report_switch(device_input, SW_TABLET_MODE, 1);
			input_sync(device_input);
			pr_info("tablet-mode: keyboard detached → tablet mode ON\n");
			break;

		case USB_DEVICE_ADD:
			input_report_switch(device_input, SW_TABLET_MODE, 0);
			input_sync(device_input);
			pr_info("tablet-mode: keyboard attached → tablet mode OFF\n");
			break;

		default:
			break;
		}
	}

	return NOTIFY_OK;
}

/*
 * Module initialization
 */
static int __init init(void)
{
	int err;

	device_input = input_allocate_device();
	if (!device_input)
		return -ENOMEM;

	device_input->name = "Tablet Mode Switch";
	device_input->phys = "tablet-mode/input0";
	device_input->id.bustype = BUS_USB;
	device_input->id.vendor  = KEYBOARD_VENDOR;
	device_input->id.product = KEYBOARD_PRODUCT;

	__set_bit(EV_SW, device_input->evbit);
	__set_bit(SW_TABLET_MODE, device_input->swbit);

	err = input_register_device(device_input);
	if (err) {
		input_free_device(device_input);
		return err;
	}

	usb_nb.notifier_call = usb_notify;
	usb_register_notify(&usb_nb);

	pr_info("tablet-mode: module loaded (listening for HAILUCK keyboard)\n");
	return 0;
}

/*
 * Cleanup on module removal
 */
static void __exit exit(void)
{
	usb_unregister_notify(&usb_nb);

	if (device_input) {
		input_unregister_device(device_input);
		device_input = NULL;
	}

	pr_info("tablet-mode: module unloaded\n");
}

module_init(init);
module_exit(exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes Hörmann (based on lukas-w)");
MODULE_DESCRIPTION("Tablet mode switch via USB keyboard attach/detach (initially developed for Chuwi Hi10 Max)");
MODULE_VERSION("1.0");
