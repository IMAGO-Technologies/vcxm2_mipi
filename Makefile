# Comment/uncomment the following line to disable/enable debugging
# or call make DEBUG=y default is DEBUG=n
# DEBUG = y


ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DDEBUG # "-O" is needed to expand inlines
  $(info we use debug flags [${DEBFLAGS}])
else
  DEBFLAGS = -O2  
endif

ccflags-y := $(DEBFLAGS) -Werror -Wall -Wno-unused-parameter -Wno-date-time -I/mnt/Linux_for_Tegra_36.4/source/nvidia-oot/include
vcxm2_mipi-objs := vcxm2_mipi_sensor.o
obj-m	:= vcxm2_mipi.o


# If KERNELDIR is defined, we've been invoked from the DKMS
# otherwise we were called directly from the command line
# '?=' has only an effect if the variable not defined
KERNELDIR ?= /lib/modules/$(shell uname -r)/build


default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules 

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order Module.symvers

install:
	make -C $(KERNELDIR) M=$(PWD) modules_install
	depmod -a 
