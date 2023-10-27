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

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_HID_MOUSE_H
#define __USBH_HID_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_MOUSE
  * @brief This file is the Header file for usbh_hid_mouse.c
  * @{
  */


/** @defgroup USBH_HID_MOUSE_Exported_Types
  * @{
  */

typedef struct _HID_MOUSE_Info
{
  uint8_t              x;
  uint8_t              y;
  uint8_t              buttons[3];
}
HID_MOUSE_Info_TypeDef;

/**
  * @}
  */

/** @defgroup USBH_HID_MOUSE_Exported_Defines
  * @{
  */
#ifndef USBH_HID_MOUSE_REPORT_SIZE
#define USBH_HID_MOUSE_REPORT_SIZE                       (64) //max interrupt transfer size -- was: 0x8U
#endif /* USBH_HID_MOUSE_REPORT_SIZE */
/**
  * @}
  */

/** @defgroup USBH_HID_MOUSE_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_MOUSE_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_MOUSE_Exported_FunctionsPrototype
  * @{
  */
USBH_StatusTypeDef USBH_HID_MouseInit(USBH_HandleTypeDef *phost);
HID_MOUSE_Info_TypeDef *USBH_HID_GetMouseInfo(USBH_HandleTypeDef *phost);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBH_HID_MOUSE_H */

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

