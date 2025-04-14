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
#include <stdio.h>
#include "main.h"
#include "cmsis_os.h"
#include "hardware_config.h"
#include "xlat.h"
#include "gfx_main.h"
#include "usb_task.h"

osThreadId xlatTaskHandle;
osThreadId lvglTaskHandle;
osThreadId usbHostTaskHandle;

osPoolDef(hidevt_pool, 16, hid_event_t);               // Define memory pool
osPoolId  hidevt_pool;

osPoolDef(gfxevt_pool, 16, gfx_event_t);               // Define memory pool
osPoolId  gfxevt_pool;

osMessageQDef(msgQUsbClick, 16, hid_event_t *);              // Define message queue
osMessageQId  msgQUsbHidEvent;

osMessageQDef(msgQGfxTask, 4, gfx_event_t *);              // Define message queue
osMessageQId  msgQGfxTask;

// LVGL mutex
SemaphoreHandle_t lvgl_mutex;

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    hw_init();
    hw_debug_init();
    gfx_init();

    lvgl_mutex = xSemaphoreCreateMutex();

    hidevt_pool = osPoolCreate(osPool(hidevt_pool)); // create memory pool
    gfxevt_pool = osPoolCreate(osPool(gfxevt_pool)); // create memory pool
    msgQUsbHidEvent = osMessageCreate(osMessageQ(msgQUsbClick), NULL);  // create msg queue
    msgQGfxTask = osMessageCreate(osMessageQ(msgQGfxTask), NULL);    // create msg queue

    /* Create the thread(s) */
    osThreadDef(xlatTask, xlat_task, osPriorityNormal, 0, 2048 / 4);
    xlatTaskHandle = osThreadCreate(osThread(xlatTask), NULL);
    osThreadDef(lvglTask, gfx_task, osPriorityLow, 0, 4096 * 2 / 4);
    lvglTaskHandle = osThreadCreate(osThread(lvglTask), NULL);
    osThreadDef(usbHostTask, usb_host_task, osPriorityHigh, 0, 2048 / 4);
    usbHostTaskHandle = osThreadCreate(osThread(usbHostTask), NULL);

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    /* Infinite loop */
    while (1) {};
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
