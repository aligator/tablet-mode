TARGET_MODULE:=tablet-mode
obj-m := $(TARGET_MODULE).o

KVER?=$(shell uname -r)
KDIR=/lib/modules/$(KVER)/build
MDIR=/lib/modules/$(KVER)/kernel/platform/x86
LLVM ?= $(shell grep -q '^CONFIG_CC_IS_CLANG=y' $(KDIR)/include/config/auto.conf 2>/dev/null && echo 1 || echo 0)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) LLVM=$(LLVM) modules

install:
	install -d $(MDIR)
	install -m 644 -c $(TARGET_MODULE).ko $(MDIR)
	depmod -a

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) LLVM=$(LLVM) clean

load:
	insmod ./$(TARGET_MODULE).ko
unload:
	rmmod ./$(TARGET_MODULE).ko
