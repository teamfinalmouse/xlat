/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v1.0_Cube
  * @brief           : This file implements the USB Host
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
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
