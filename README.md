# Tablet Mode Driver

## Problem
The **Chuwi Hi10 Max** has out-of-the-box tablet mode detection when detaching the keyboard.
However, to get the active stylus working, the original kernel module must be blacklisted:  
`/etc/modprobe.d/blacklist-ilitek.conf`  
```
blacklist ilitek_ts_i2c
```

## Solution
This project provides an additional kernel driver that detects whether the detachable keyboard is connected.
It listens for attach/detach events from the integrated **keyboard** and reports `SW_TABLET_MODE` via the Linux input subsystem.
On module load, it also checks the current USB state and sets the initial `SW_TABLET_MODE` value immediately.

This allows desktop environments and tools like `libinput` or  
[`linux_detect_tablet_mode`](https://github.com/alesguzik/linux_detect_tablet_mode)  
to automatically enable or disable keyboard and touchpad input when switching between laptop and tablet mode.

Unlike the original [`yoga-usage-mode`](https://github.com/lukas-w/yoga-usage-mode), which this is loosely based on,  
this version does **not** rely on ACPI or DMI detection,  
but instead uses a USB notifier for devices with `idVendor=0x258a` and `idProduct=0x0020`.

This kernel module may be interesting for other 2in1 devices, too.
As device IDs are configurable at compile time, you can build it for other devices too.
Feel free to open an issue to report additional working hardware.

### Tested hardware

* Chuwi Hi10 Max  
  (HAILUCK CO.,LTD USB Keyboard – VID 258a / PID 0020)

### Usage

* `make` to build
* provide custom IDs: `make KEYBOARD_VENDOR=0x258a KEYBOARD_PRODUCT=0x0020`
* `sudo make load` to load the module for testing  
* `sudo make unload` to unload the module  
* `sudo make install` to install permanently  

Add `tablet-mode` (module name kept for compatibility)  
to e.g. `/etc/modules-load.d/modules.conf` to load automatically at boot.

### Build notes

The `Makefile` auto-detects whether the target kernel was built with Clang/LLVM and sets `LLVM=1` only when needed. This lets the same source work for stock Arch kernels built with GCC and CachyOS kernels built with Clang.

You can still override this manually if needed:
* `make LLVM=0`
* `make LLVM=1`
