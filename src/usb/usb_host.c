/*
 * Copyright (c) 2015 STMicroelectronics.
 * Copyright (c) 2023 Finalmouse, LLC
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

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "src/usb/usbh_core.h"
#include "usbh_hid.h"
#include "gfx_main.h"

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostHS;
static char label[128];
static char product_string[110] = {0};

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostHS, USBH_UserProcess, HOST_HS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostHS, USBH_HID_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostHS) != USBH_OK)
  {
    Error_Handler();
  }
}

/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            gfx_set_device_label("No USB device connected", "");
            break;

        case HOST_USER_CLASS_ACTIVE: {
            uint16_t vid = phost->device.DevDesc.idVendor;
            uint16_t pid = phost->device.DevDesc.idProduct;
            printf("USB HID device connected: 0x%04X:%04X\n", vid, pid);
            if (vid == 0x361d && pid == 0x0100) {
                printf("Finalmouse UltralightX detected\n");
                gfx_set_hid_byte(true);
            } else {
                gfx_set_hid_byte(false);
            }
            memset(label, 0, sizeof(label));
            snprintf(label, sizeof(label), "0x%04X:%04X", vid, pid);
            label[sizeof(label) - 1] = '\0';
            gfx_set_device_label(product_string, label);
        }
            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }
}

void usb_host_set_product_string(const char * product)
{
    snprintf(product_string, sizeof(product_string), "%s", product);
    product_string[sizeof(product_string) - 1] = '\0';
}
