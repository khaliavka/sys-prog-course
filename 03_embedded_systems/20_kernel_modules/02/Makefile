obj-m += proc_buffer.o
ccflags-y += -g -DDEBUG
PWD := $(CURDIR)  
   
all:  
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 
  
clean: 
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
