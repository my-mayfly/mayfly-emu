#include "device.h"

void update_device() {
	aclint_update();
	serail_update();
	plic_update();
}

void init_device(){
    serial_init();
}

void device_exit(){
	serial_exit();
}