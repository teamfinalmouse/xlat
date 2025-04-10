/*
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

#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "pinconfig.h"

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;

extern const osPoolDef_t os_pool_def_hidevt_pool;
extern const osMessageQDef_t os_messageQ_def_MsgBox;

extern osPoolId  hidevt_pool;
extern osPoolId  gfxevt_pool;
extern osMessageQId  msgQUsbHidEvent;
extern osMessageQId  msgQGfxTask;
extern SemaphoreHandle_t lvgl_mutex;

static inline void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {};
}


#endif /* __MAIN_H */
