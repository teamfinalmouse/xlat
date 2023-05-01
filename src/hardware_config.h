//
// Created by vinz on 1/05/23.
//

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

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
