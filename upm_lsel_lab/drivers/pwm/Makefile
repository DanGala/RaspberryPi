#
# Makefile for the empty
#

obj-m	+= lse_pwm.o

cross:
	ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make -C $(KERNEL_SRC) M=$(PWD) modules

native:
	make -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	 make -C $(KERNEL_SRC) M=$(PWD) clean

