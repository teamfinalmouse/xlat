/*
 * Copyright (C) 2025 Finalmouse, LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <main.h>
#include <stm32f7xx_hal_gpio.h>
#include <xlat.h>

#include "tusb.h"
#include "tusb_config.h"

#define MAX_REPORT  4

// Each HID instance can have multiple reports
static struct {
  uint8_t report_count;
  tuh_hid_report_info_t report_info[MAX_REPORT];
} hid_info[CFG_TUH_HID];

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
  printf("HID device address = %d, instance = %d is mounted\n", dev_addr, instance);

  // Interface protocol (hid_interface_protocol_enum_t)
  const char *protocol_str[] = {"None", "Keyboard", "Mouse"};
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  printf("HID Interface Protocol = %s\n", protocol_str[itf_protocol]);

  // Parse the HID descriptor using xlat
  xlat_parse_hid_descriptor((uint8_t*)desc_report, desc_len);

  // By default host stack will use activate boot protocol on supported interface.
  // Therefore for this simple example, we only need to parse generic report descriptor (with built-in parser)
  if (itf_protocol == HID_ITF_PROTOCOL_NONE) {
    hid_info[instance].report_count = tuh_hid_parse_report_descriptor(hid_info[instance].report_info, MAX_REPORT,
                                                                      desc_report, desc_len);
    printf("HID has %u reports \n", hid_info[instance].report_count);
  }

  // request to receive report
  // tuh_hid_report_received_cb() will be invoked when report is available
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    printf("Error: cannot request to receive report\n");
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  printf("HID device address = %d, instance = %d is unmounted\n", dev_addr, instance);
  xlat_clear_device_info();
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {

  HAL_GPIO_WritePin(ARDUINO_D4_GPIO_Port, ARDUINO_D4_Pin, GPIO_PIN_SET);

  uint32_t timestamp = xlat_counter_1mhz_get(); // not as early as was done in the STM32 USB Host library...
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  xlat_usb_event_callback(timestamp, report, len, itf_protocol); // Call to XLAT module

  HAL_GPIO_WritePin(ARDUINO_D4_GPIO_Port, ARDUINO_D4_Pin, GPIO_PIN_RESET);

  // continue to request to receive report
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    printf("Error: cannot request to receive report\n");
  }
}
