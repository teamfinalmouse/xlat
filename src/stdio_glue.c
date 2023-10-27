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
#include <string.h>
#include "main.h"

// RTT will override these
__weak int _write(int file, char *ptr, int len)
{
    (void)file; /* Not used, avoid warning */
    HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, 100);
    return len;
}

// RTT will override these
__weak int _read(int file, char *ptr, int len)
{
    (void)file; /* Not used, avoid warning */
    HAL_UART_Receive(&huart1, (uint8_t *) ptr, len, 100);
    return len;
}

int vcp_write(char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, 100);
    return len;
}

// C-string must be null-terminated
int vcp_writestr(char *ptr)
{
    size_t len = strlen(ptr);
    HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, 100);
    return (int)len;
}

// RTT will override these
int vcp_read(char *ptr, int len)
{
    HAL_UART_Receive(&huart1, (uint8_t *) ptr, len, 100);
    return len;
}

