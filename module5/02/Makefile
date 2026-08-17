BUILD_DIR := $(PWD)/build

all:
	@mkdir -p $(BUILD_DIR)
	@ln -sf $(PWD)/src/proc_module.c $(BUILD_DIR)/proc_module.c
	@echo "obj-m += proc_module.o" > $(BUILD_DIR)/Makefile
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) clean
	@rm -rf $(BUILD_DIR)
