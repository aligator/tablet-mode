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
#include <linux/workqueue.h>

#ifndef KEYBOARD_VENDOR
#define KEYBOARD_VENDOR  0x258a
#endif

#ifndef KEYBOARD_PRODUCT
#define KEYBOARD_PRODUCT 0x0020
#endif

static struct input_dev *device_input;
static struct notifier_block usb_nb;
static struct delayed_work initial_state_work;
static bool tablet_mode_enabled;

/*
 * Some userspace components only react once they observe a fresh
 * tablet-mode switch event. Reasserting the initial state shortly
 * after module load helps when the system already booted without
 * the keyboard attached.
 */
static void report_tablet_mode(bool enabled, bool force_event)
{
	if (force_event && test_bit(SW_TABLET_MODE, device_input->sw) == enabled)
		__change_bit(SW_TABLET_MODE, device_input->sw);

	tablet_mode_enabled = enabled;
	input_report_switch(device_input, SW_TABLET_MODE, enabled);
	input_sync(device_input);
}

static void initial_state_reassert(struct work_struct *work)
{
	report_tablet_mode(tablet_mode_enabled, true);
	pr_info("tablet-mode: reasserted initial tablet mode %s\n",
		tablet_mode_enabled ? "ON" : "OFF");
}

/* Returns true when a matching keyboard is already connected. */
static int match_keyboard(struct usb_device *udev, void *data)
{
	bool *present = data;

	if (udev->descriptor.idVendor == KEYBOARD_VENDOR &&
	    udev->descriptor.idProduct == KEYBOARD_PRODUCT) {
		*present = true;
		return 1;
	}

	return 0;
}

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
			report_tablet_mode(true, false);
			pr_info("tablet-mode: keyboard detached → tablet mode ON\n");
			break;

		case USB_DEVICE_ADD:
			report_tablet_mode(false, false);
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
	bool keyboard_present = false;

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

	usb_for_each_dev(&keyboard_present, match_keyboard);
	report_tablet_mode(!keyboard_present, false);
	pr_info("tablet-mode: initial state -> tablet mode %s\n",
		keyboard_present ? "OFF" : "ON");

	INIT_DELAYED_WORK(&initial_state_work, initial_state_reassert);
	if (!keyboard_present)
		schedule_delayed_work(&initial_state_work, msecs_to_jiffies(3000));

	pr_info("tablet-mode: module loaded (listening for HAILUCK keyboard)\n");
	return 0;
}

/*
 * Cleanup on module removal
 */
static void __exit exit(void)
{
	cancel_delayed_work_sync(&initial_state_work);
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
MODULE_VERSION("1.1");
