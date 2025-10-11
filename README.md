# Tablet Mode Driver

## Problem
The **Chuwi Hi10 Max** has out of the box support for tablet mode detection when detatching the keyboard. 
However to get the active stylus working the kernel module for this must be blacklisted.  
`/etc/modprobe.d/blacklist-ilitek.conf`  
```
blacklist ilitek_ts_i2c
```

## Solution
So to still support both this is an additional kernel driver that dects whether the detachable keyboard is connected.  
It listens for attach/detach events from the integrated **keyboard** and reports `SW_TABLET_MODE` via the Linux input subsystem.  

This allows desktop environments and tools like `libinput` or  
[`linux_detect_tablet_mode`](https://github.com/alesguzik/linux_detect_tablet_mode)  
to automatically enable or disable keyboard and touchpad input when switching between laptop and tablet mode.

Unlike the original [`yoga-usage-mode`](https://github.com/lukas-w/yoga-usage-mode), which this is loosely based on,  
this version does **not** rely on ACPI or DMI detection,  
but instead uses a USB notifier for devices with `idVendor=0x258a` and `idProduct=0x0020`.

This kernel module may be interesting for other 2in1 devices, too.
As the device ids are compile time changeable, you may compile it for other devices. You may add a issue to notify me about any other working device.

### Tested hardware

* Chuwi Hi10 Max  
  (HAILUCK CO.,LTD USB Keyboard – VID 258a / PID 0020)

### Usage

* `make` to build  
  * provide custom ids`make KEYBOARD_VENDOR=0x258a KEYBOARD_PRODUCT=0x0020`
* `sudo make load` to load the module for testing  
* `sudo make unload` to unload the module  
* `sudo make install` to install permanently  

Add `tablet-mode` (module name kept for compatibility)  
to e.g. `/etc/modules-load.d/modules.conf` to load automatically at boot.
