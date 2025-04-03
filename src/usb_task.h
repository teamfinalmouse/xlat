#pragma once

void usb_host_task(void const *param);
const char* usb_host_get_manuf_string(void);
const char* usb_host_get_product_string(void);
const char* usb_host_get_serial_string(void);
const char* usb_host_get_vidpid_string(void);
