//
// Created by vinz on 1/05/23.
//

#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "main.h"

__weak int _write(int file, char *ptr, int len)
{
    (void)file; /* Not used, avoid warning */
    HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, 100);
    return len;
}

__weak int _read(int file, char *ptr, int len)
{
    (void)file; /* Not used, avoid warning */
    HAL_UART_Receive(&huart1, (uint8_t *) ptr, len, 100);
    return len;
}

