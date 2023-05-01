/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdbool.h>
#include "main.h"
#include "cmsis_os.h"
#include "usb_host.h"
#include "hardware_config.h"
#include "xlat.h"
#include "gfx_main.h"


osThreadId xlatTaskHandle;
osThreadId lvglTaskHandle;

osPoolDef(hidevt_pool, 16, struct hid_event);               // Define memory pool
osPoolId  hidevt_pool;

osMessageQDef(msgQUsbClick, 16, struct hid_event *);              // Define message queue
osMessageQId  msgQUsbClick;

osMessageQDef(msgQNewData, 4, struct hid_event *);              // Define message queue
osMessageQId  msgQNewData;

// LVGL mutex
SemaphoreHandle_t lvgl_mutex;

/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
void xlat_task(void const * argument)
{
    hidevt_pool = osPoolCreate(osPool(hidevt_pool));                 // create memory pool
    msgQUsbClick = osMessageCreate(osMessageQ(msgQUsbClick), NULL);  // create msg queue
    msgQNewData = osMessageCreate(osMessageQ(msgQNewData), NULL);        // create msg queue

    /* init code for USB_HOST */
    MX_USB_HOST_Init();

    /* UART test */
    printf("\n");
    printf("***************\n");
    printf("Welcome to XLAT\n");
    printf("***************\n\n");

    xlat_init();

    /* Infinite loop */
    for(;;) {
        xlat_usb_hid_event(); // blocking
    }
}

void lvgl_task(void const *argument)
{
    while(!xlat_initialized) {
        osDelay(1);
    }

    while (1) {
        gfx_task();
        osDelay(1);
    }
}

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

    /* Create the thread(s) */
    osThreadDef(xlatTask, xlat_task, osPriorityNormal, 0, 4096 / 4);
    xlatTaskHandle = osThreadCreate(osThread(xlatTask), NULL);
    osThreadDef(lvglTask, lvgl_task, osPriorityLow, 0, 4096 / 4);
    lvglTaskHandle = osThreadCreate(osThread(lvglTask), NULL);

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    /* Infinite loop */
    while (1) {};
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
