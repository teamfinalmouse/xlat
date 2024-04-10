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


/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
- "stm32xxxxx_{eval}{discovery}{adafruit}_lcd.c"
- "stm32xxxxx_{eval}{discovery}_sdram.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_keyboard.h"
#include "usbh_hid_parser.h"


/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_KEYBOARD
  * @brief    This file includes HID Layer Handlers for USB Host HID class.
  * @{
  */

/** @defgroup USBH_HID_KEYBOARD_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_KEYBOARD_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_KEYBOARD_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_KEYBOARD_Private_FunctionPrototypes
  * @{
  */
static USBH_StatusTypeDef USBH_HID_KeyboardDecode(USBH_HandleTypeDef *phost);

/**
  * @}
  */


/** @defgroup USBH_HID_KEYBOARD_Private_Variables
  * @{
  */
HID_KEYBOARD_Info_TypeDef keyboard_info;
uint8_t                   keyboard_report_data[USBH_HID_KEYBOARD_REPORT_SIZE];
uint8_t                   keyboard_rx_report_buf[USBH_HID_KEYBOARD_REPORT_SIZE];

/* Structures defining how to access items in a HID keyboard report */
/* Access key 1 state. */
static const HID_Report_ItemTypedef prop_k1 =
{
  keyboard_report_data, /*data*/
  1,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min value device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

/* Access key 2 state. */
static const HID_Report_ItemTypedef prop_k2 =
{
  keyboard_report_data, /*data*/
  1,     /*size*/
  1,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min value device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

/* Access key 3 state. */
static const HID_Report_ItemTypedef prop_k3 =
{
  keyboard_report_data, /*data*/
  1,     /*size*/
  2,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};


/**
  * @}
  */


/** @defgroup USBH_HID_KEYBOARD_Private_Functions
  * @{
  */

/**
  * @brief  USBH_HID_KeyboardInit
  *         The function init the HID keyboard.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_KeyboardInit(USBH_HandleTypeDef *phost)
{
  uint32_t i;
  HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

  keyboard_info.keys[0] = 0U;
  keyboard_info.keys[1] = 0U;
  keyboard_info.keys[2] = 0U;

  for (i = 0U; i < sizeof(keyboard_report_data); i++)
  {
    keyboard_report_data[i] = 0U;
    keyboard_rx_report_buf[i] = 0U;
  }

  if (HID_Handle->length > sizeof(keyboard_report_data))
  {
    HID_Handle->length = (uint16_t)sizeof(keyboard_report_data);
  }
  HID_Handle->pData = keyboard_rx_report_buf;

  if ((HID_QUEUE_SIZE * sizeof(keyboard_report_data)) > sizeof(phost->device.Data)) {
    return USBH_FAIL;
  } else {
      USBH_HID_FifoInit(&HID_Handle->fifo, phost->device.Data, (uint16_t)(HID_QUEUE_SIZE * sizeof(keyboard_report_data)));
  }

  return USBH_OK;
}

/**
  * @brief  USBH_HID_GetKeyboardInfo
  *         The function return keyboard information.
  * @param  phost: Host handle
  * @retval keyboard information
  */
HID_KEYBOARD_Info_TypeDef *USBH_HID_GetKeyboardInfo(USBH_HandleTypeDef *phost)
{
  if (USBH_HID_KeyboardDecode(phost) == USBH_OK)
  {
    return &keyboard_info;
  }
  else
  {
    return NULL;
  }
}

/**
  * @brief  USBH_HID_KeyboardDecode
  *         The function decode keyboard data.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HID_KeyboardDecode(USBH_HandleTypeDef *phost)
{
  HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

  if ((HID_Handle->length == 0U) || (HID_Handle->fifo.buf == NULL))
  {
    return USBH_FAIL;
  }
  /*Fill report */
  if (USBH_HID_FifoRead(&HID_Handle->fifo, &keyboard_report_data, HID_Handle->length) ==  HID_Handle->length)
  {
    /*Decode report */
    keyboard_info.keys[0] = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_k1, 0U);
    keyboard_info.keys[1] = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_k2, 0U);
    keyboard_info.keys[2] = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_k3, 0U);

    return USBH_OK;
  }
  return   USBH_FAIL;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


/**
  * @}
  */
