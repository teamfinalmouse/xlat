/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stm32f7xx_hal.h"
#include "usb_task.h"

#include <cmsis_os.h>
#include <xlat.h>

#include "tusb.h"
#include "tusb_config.h"

#include "gfx_main.h"

extern void hid_app_init(void);

static void board_init(void) {
  // Explicitly disable systick to prevent its ISR runs before scheduler start
  // SysTick->CTRL &= ~1U;

  // If freeRTOS is used, IRQ priority is limited by max syscall ( smaller value is higher priority )
  // NVIC_SetPriority(OTG_FS_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );
  NVIC_SetPriority(OTG_HS_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY );

  GPIO_InitTypeDef GPIO_InitStruct;

  //------------- rhport1: OTG_HS -------------//
  // MCU with external ULPI PHY
  /* ULPI CLK */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* ULPI D0 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* ULPI D1 D2 D3 D4 D5 D6 D7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* ULPI STP */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* NXT */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* ULPI DIR */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  // Enable USB HS & ULPI Clocks
  __HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();
  __HAL_RCC_USB_OTG_HS_CLK_ENABLE();

#if OTG_HS_VBUS_SENSE
  #error OTG HS VBUS Sense enabled is not implemented
#else
  // No VBUS sense
  USB_OTG_HS->GCCFG &= ~USB_OTG_GCCFG_VBDEN;

  // B-peripheral session valid override enable
  USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN;
  USB_OTG_HS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;
#endif
}

// USB Host task
// This top level thread process all usb events and invoke callbacks
void usb_host_task(void const *param) {
  (void) param;

  board_init();

  // init host stack on configured roothub port
  tusb_rhport_init_t host_init = {
    .role = TUSB_ROLE_HOST,
    .speed = TUSB_SPEED_AUTO
  };

  while(!xlat_initialized) {
    osDelay(1);
  }

  if (!tusb_init(BOARD_TUH_RHPORT, &host_init)) {
    printf("Failed to init USB Host Stack\n");
    vTaskSuspend(NULL);
  }

#if CFG_TUH_ENABLED && CFG_TUH_MAX3421
  // FeatherWing MAX3421E use MAX3421E's GPIO0 for VBUS enable
  enum { IOPINS1_ADDR  = 20u << 3, /* 0xA0 */ };
  tuh_max3421_reg_write(BOARD_TUH_RHPORT, IOPINS1_ADDR, 0x01, false);
#endif

  // RTOS forever loop
  while (1) {
    // put this thread to waiting state until there is new events
    tuh_task();

    // following code only run if tuh_task() process at least 1 event
  }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Buffer for string descriptors
static char manufacturer_string[64];
static char product_string[64];
static char serial_string[64];
static char vidpid_string[32];  // Format: "VID:XXXX PID:XXXX"

// Helper function to convert UTF-16LE to UTF-8
static void utf16le_to_utf8(const uint16_t* utf16, size_t utf16_len, char* utf8, size_t utf8_len) {
    size_t utf8_pos = 0;
    for (size_t i = 0; i < utf16_len && utf8_pos < utf8_len - 1; i++) {
        uint16_t chr = utf16[i];
        if (chr == 0) break;
        
        if (chr < 0x80) {
            utf8[utf8_pos++] = (char)chr;
        } else if (chr < 0x800) {
            if (utf8_pos + 1 >= utf8_len - 1) break;
            utf8[utf8_pos++] = (char)(0xC0 | (chr >> 6));
            utf8[utf8_pos++] = (char)(0x80 | (chr & 0x3F));
        } else {
            if (utf8_pos + 2 >= utf8_len - 1) break;
            utf8[utf8_pos++] = (char)(0xE0 | (chr >> 12));
            utf8[utf8_pos++] = (char)(0x80 | ((chr >> 6) & 0x3F));
            utf8[utf8_pos++] = (char)(0x80 | (chr & 0x3F));
        }
    }
    utf8[utf8_pos] = '\0';
}

// Get and buffer string descriptors
static void update_string_descriptors(uint8_t daddr, tusb_desc_device_t *desc_device) {
    uint16_t temp_buf[64];
    
    // Get manufacturer string
    if (tuh_descriptor_get_manufacturer_string_sync(daddr, 0x0409, temp_buf, sizeof(temp_buf)) == XFER_RESULT_SUCCESS) {
        // First byte is length, second byte is descriptor type (3)
        uint8_t str_len = ((uint8_t*)temp_buf)[0] - 2; // Subtract header size (2 bytes)
        utf16le_to_utf8((uint16_t*)&temp_buf[1], str_len/2, manufacturer_string, sizeof(manufacturer_string));
    } else {
        manufacturer_string[0] = '\0';
    }
    
    // Get product string
    if (tuh_descriptor_get_product_string_sync(daddr, 0x0409, temp_buf, sizeof(temp_buf)) == XFER_RESULT_SUCCESS) {
        uint8_t str_len = ((uint8_t*)temp_buf)[0] - 2;
        utf16le_to_utf8((uint16_t*)&temp_buf[1], str_len/2, product_string, sizeof(product_string));
    } else {
        product_string[0] = '\0';
    }
    
    // Get serial string
    if (tuh_descriptor_get_serial_string_sync(daddr, 0x0409, temp_buf, sizeof(temp_buf)) == XFER_RESULT_SUCCESS) {
        uint8_t str_len = ((uint8_t*)temp_buf)[0] - 2;
        utf16le_to_utf8((uint16_t*)&temp_buf[1], str_len/2, serial_string, sizeof(serial_string));
    } else {
        serial_string[0] = '\0';
    }

    // Update VID/PID string
    snprintf(vidpid_string, sizeof(vidpid_string), "%04x:%04x",
            desc_device->idVendor, desc_device->idProduct);

}

// Update the mount callback to get string descriptors
void tuh_mount_cb(uint8_t daddr) {
    // Get device descriptor first
    tusb_desc_device_t desc_device;

    printf("Device with address %d mounted\n", daddr);

    if (tuh_descriptor_get_device_sync(daddr, &desc_device, sizeof(desc_device)) == XFER_RESULT_SUCCESS) {
        // Update string descriptors
        update_string_descriptors(daddr, &desc_device);
        
        printf("\t-> %04x:%04x %s: %s\n",
               desc_device.idVendor, desc_device.idProduct,
               manufacturer_string, product_string);


        // Send a message to the gfx thread, to refresh the device info
        gfx_event_send(GFX_EVENT_DEVICE_CONNECTED, 0);
    }
}

// XXX FIXME: doesn't seem to be called
void tuh_umount_cb(uint8_t dev_addr) {
  // application tear-down
  printf("A device with address %d is unmounted\n", dev_addr);
  xlat_clear_device_info();
}


// Public functions to get buffered strings
const char* usb_host_get_manuf_string(void) {
    return manufacturer_string;
}

const char* usb_host_get_product_string(void) {
    return product_string;
}

const char* usb_host_get_serial_string(void) {
    return serial_string;
}

const char * usb_host_get_vidpid_string(void)
{
    return vidpid_string;
}
