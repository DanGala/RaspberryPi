#
# Makefile for the empty
#

#obj-m	+= empty.o
#obj-m	+= hello.o
#obj-	+= timer.o
obj-m	+= driver_gpio.o

cross:
	ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make -C $(KERNEL_SRC) M=$(PWD) modules

native:
	make -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	 make -C $(KERNEL_SRC) M=$(PWD) clean

