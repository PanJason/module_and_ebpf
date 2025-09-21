# This is how to express dependent objects in a module.
# hello_world-objs := hello_world.o

# obj-m is a list of what kernel modules to build.  The .o and other
# objects will be automatically built from the corresponding .c file -
# no need to list the source files explicitly.
obj-m += hello_world.o

# KDIR is the location of the kernel source.  The current standard is
# to link to the associated source tree from the directory containing
# the compiled modules.
KDIR := /lib/modules/`uname -r`/build

# PWD is the current working directory and the location of our module
# source files.
PWD = $(shell pwd)

# make -C $(KDIR) runs make in the kernel build directory (the tree that was
# used to build the running kernel). 
# The M=$(PWD) argument tells that build system to treat your current directory
# as a module directory and look there for its Makefile/sources. 
# The modules target asks the kernel build rules to build any modules listed in
# your obj-m variable, producing the .ko output alongside your sources.
all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean