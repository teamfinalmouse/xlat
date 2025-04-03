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

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <stdbool.h>

#define XLAT_TIMx                           TIM2
#define XLAT_TIMx_CLK_ENABLE()              __HAL_RCC_TIM2_CLK_ENABLE()
#define XLAT_TIMx_handle                   htim2

int hw_init(void);
void hw_debug_init(void);
void hw_exti_interrupts_enable(void);
void hw_exti_interrupts_disable(void);
void hw_config_input_trigger(bool rising);
bool hw_config_input_trigger_is_rising_edge(void);

#endif //HARDWARE_CONFIG_H
