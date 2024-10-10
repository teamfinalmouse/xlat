/**
  ******************************************************************************
  * @file         stm32f7xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
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
#include "main.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* External functions --------------------------------------------------------*/

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
/**
* Initializes the Global MSP.
*/
void HAL_MspInit(void)
{

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* System interrupt init*/
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hadc->Instance==ADC3)
    {
        /* Peripheral clock enable */
        __HAL_RCC_ADC3_CLK_ENABLE();

        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**ADC3 GPIO Configuration
        PF7     ------> ADC3_IN5
        PF6     ------> ADC3_IN4
        PF10     ------> ADC3_IN8
        PF9     ------> ADC3_IN7
        PF8     ------> ADC3_IN6
        PA0/WKUP     ------> ADC3_IN0
        */
        GPIO_InitStruct.Pin = ARDUINO_A4_Pin|ARDUINO_A5_Pin|ARDUINO_A1_Pin|ARDUINO_A2_Pin
                              |ARDUINO_A3_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = ARDUINO_A0_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ARDUINO_A0_GPIO_Port, &GPIO_InitStruct);
    }

}

/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance==ADC3)
    {
        /* Peripheral clock disable */
        __HAL_RCC_ADC3_CLK_DISABLE();

        /**ADC3 GPIO Configuration
        PF7     ------> ADC3_IN5
        PF6     ------> ADC3_IN4
        PF10     ------> ADC3_IN8
        PF9     ------> ADC3_IN7
        PF8     ------> ADC3_IN6
        PA0/WKUP     ------> ADC3_IN0
        */
        HAL_GPIO_DeInit(GPIOF, ARDUINO_A4_Pin|ARDUINO_A5_Pin|ARDUINO_A1_Pin|ARDUINO_A2_Pin
                               |ARDUINO_A3_Pin);

        HAL_GPIO_DeInit(ARDUINO_A0_GPIO_Port, ARDUINO_A0_Pin);
    }

}

/**
* @brief CRC MSP Initialization
* This function configures the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc)
{
    if(hcrc->Instance==CRC)
    {
        /* Peripheral clock enable */
        __HAL_RCC_CRC_CLK_ENABLE();
    }

}

/**
* @brief CRC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hcrc: CRC handle pointer
* @retval None
*/
void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc)
{
    if(hcrc->Instance==CRC)
    {
        /* Peripheral clock disable */
        __HAL_RCC_CRC_CLK_DISABLE();
    }

}

/**
* @brief DMA2D MSP Initialization
* This function configures the hardware resources used in this example
* @param hdma2d: DMA2D handle pointer
* @retval None
*/
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* hdma2d)
{
    if(hdma2d->Instance==DMA2D)
    {
        /* Peripheral clock enable */
        __HAL_RCC_DMA2D_CLK_ENABLE();
        /* DMA2D interrupt Init */
        HAL_NVIC_SetPriority(DMA2D_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(DMA2D_IRQn);
    }

}

/**
* @brief DMA2D MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdma2d: DMA2D handle pointer
* @retval None
*/
void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* hdma2d)
{
    if(hdma2d->Instance==DMA2D)
    {
        /* Peripheral clock disable */
        __HAL_RCC_DMA2D_CLK_DISABLE();

        /* DMA2D interrupt DeInit */
        HAL_NVIC_DisableIRQ(DMA2D_IRQn);
    }

}

/**
* @brief LTDC MSP Initialization
* This function configures the hardware resources used in this example
* @param hltdc: LTDC handle pointer
* @retval None
*/
void HAL_LTDC_MspInit(LTDC_HandleTypeDef* hltdc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hltdc->Instance==LTDC)
    {
        /* Peripheral clock enable */
        __HAL_RCC_LTDC_CLK_ENABLE();

        __HAL_RCC_GPIOE_CLK_ENABLE();
        __HAL_RCC_GPIOJ_CLK_ENABLE();
        __HAL_RCC_GPIOK_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        __HAL_RCC_GPIOI_CLK_ENABLE();
        /**LTDC GPIO Configuration
        PE4     ------> LTDC_B0
        PJ13     ------> LTDC_B1
        PK7     ------> LTDC_DE
        PK6     ------> LTDC_B7
        PK5     ------> LTDC_B6
        PG12     ------> LTDC_B4
        PJ14     ------> LTDC_B2
        PI10     ------> LTDC_HSYNC
        PK4     ------> LTDC_B5
        PJ15     ------> LTDC_B3
        PI9     ------> LTDC_VSYNC
        PK1     ------> LTDC_G6
        PK2     ------> LTDC_G7
        PI15     ------> LTDC_R0
        PJ11     ------> LTDC_G4
        PK0     ------> LTDC_G5
        PI14     ------> LTDC_CLK
        PJ8     ------> LTDC_G1
        PJ10     ------> LTDC_G3
        PJ7     ------> LTDC_G0
        PJ9     ------> LTDC_G2
        PJ6     ------> LTDC_R7
        PJ4     ------> LTDC_R5
        PJ5     ------> LTDC_R6
        PJ3     ------> LTDC_R4
        PJ2     ------> LTDC_R3
        PJ0     ------> LTDC_R1
        PJ1     ------> LTDC_R2
        */
        GPIO_InitStruct.Pin = LCD_B0_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(LCD_B0_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LCD_B1_Pin|LCD_B2_Pin|LCD_B3_Pin|LCD_G4_Pin
                              |LCD_G1_Pin|LCD_G3_Pin|LCD_G0_Pin|LCD_G2_Pin
                              |LCD_R7_Pin|LCD_R5_Pin|LCD_R6_Pin|LCD_R4_Pin
                              |LCD_R3_Pin|LCD_R1_Pin|LCD_R2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LCD_DE_Pin|LCD_B7_Pin|LCD_B6_Pin|LCD_B5_Pin
                              |LCD_G6_Pin|LCD_G7_Pin|LCD_G5_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LCD_B4_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(LCD_B4_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = LCD_HSYNC_Pin|LCD_VSYNC_Pin|LCD_R0_Pin|LCD_CLK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

        /* LTDC interrupt Init */
        HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(LTDC_IRQn);
    }

}

/**
* @brief LTDC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hltdc: LTDC handle pointer
* @retval None
*/
void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* hltdc)
{
    if(hltdc->Instance==LTDC)
    {
        /* Peripheral clock disable */
        __HAL_RCC_LTDC_CLK_DISABLE();

        /**LTDC GPIO Configuration
        PE4     ------> LTDC_B0
        PJ13     ------> LTDC_B1
        PK7     ------> LTDC_DE
        PK6     ------> LTDC_B7
        PK5     ------> LTDC_B6
        PG12     ------> LTDC_B4
        PJ14     ------> LTDC_B2
        PI10     ------> LTDC_HSYNC
        PK4     ------> LTDC_B5
        PJ15     ------> LTDC_B3
        PI9     ------> LTDC_VSYNC
        PK1     ------> LTDC_G6
        PK2     ------> LTDC_G7
        PI15     ------> LTDC_R0
        PJ11     ------> LTDC_G4
        PK0     ------> LTDC_G5
        PI14     ------> LTDC_CLK
        PJ8     ------> LTDC_G1
        PJ10     ------> LTDC_G3
        PJ7     ------> LTDC_G0
        PJ9     ------> LTDC_G2
        PJ6     ------> LTDC_R7
        PJ4     ------> LTDC_R5
        PJ5     ------> LTDC_R6
        PJ3     ------> LTDC_R4
        PJ2     ------> LTDC_R3
        PJ0     ------> LTDC_R1
        PJ1     ------> LTDC_R2
        */
        HAL_GPIO_DeInit(LCD_B0_GPIO_Port, LCD_B0_Pin);

        HAL_GPIO_DeInit(GPIOJ, LCD_B1_Pin|LCD_B2_Pin|LCD_B3_Pin|LCD_G4_Pin
                               |LCD_G1_Pin|LCD_G3_Pin|LCD_G0_Pin|LCD_G2_Pin
                               |LCD_R7_Pin|LCD_R5_Pin|LCD_R6_Pin|LCD_R4_Pin
                               |LCD_R3_Pin|LCD_R1_Pin|LCD_R2_Pin);

        HAL_GPIO_DeInit(GPIOK, LCD_DE_Pin|LCD_B7_Pin|LCD_B6_Pin|LCD_B5_Pin
                               |LCD_G6_Pin|LCD_G7_Pin|LCD_G5_Pin);

        HAL_GPIO_DeInit(LCD_B4_GPIO_Port, LCD_B4_Pin);

        HAL_GPIO_DeInit(GPIOI, LCD_HSYNC_Pin|LCD_VSYNC_Pin|LCD_R0_Pin|LCD_CLK_Pin);

        /* LTDC interrupt DeInit */
        HAL_NVIC_DisableIRQ(LTDC_IRQn);
    }

}

/**
* @brief RTC MSP Initialization
* This function configures the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if(hrtc->Instance==RTC)
    {
        /** Initializes the peripherals clock
        */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_RTC_ENABLE();
    }
}

/**
* @brief RTC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
    if(hrtc->Instance==RTC)
    {
        /* Peripheral clock disable */
        __HAL_RCC_RTC_DISABLE();
    }

}

/**
* @brief SPDIFRX MSP Initialization
* This function configures the hardware resources used in this example
* @param hspdifrx: SPDIFRX handle pointer
* @retval None
*/
void HAL_SPDIFRX_MspInit(SPDIFRX_HandleTypeDef* hspdifrx)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if(hspdifrx->Instance==SPDIFRX)
    {
        /** Initializes the peripherals clock
        */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPDIFRX;
        PeriphClkInitStruct.PLLI2S.PLLI2SN = 100;
        PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLP_DIV2;
        PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
        PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
        PeriphClkInitStruct.PLLI2SDivQ = 1;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_SPDIFRX_CLK_ENABLE();

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**SPDIFRX GPIO Configuration
        PD7     ------> SPDIFRX_IN0
        */
        GPIO_InitStruct.Pin = SPDIF_RX0_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF8_SPDIFRX;
        HAL_GPIO_Init(SPDIF_RX0_GPIO_Port, &GPIO_InitStruct);
    }

}

/**
* @brief SPDIFRX MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hspdifrx: SPDIFRX handle pointer
* @retval None
*/
void HAL_SPDIFRX_MspDeInit(SPDIFRX_HandleTypeDef* hspdifrx)
{
    if(hspdifrx->Instance==SPDIFRX)
    {
        /* Peripheral clock disable */
        __HAL_RCC_SPDIFRX_CLK_DISABLE();

        /**SPDIFRX GPIO Configuration
        PD7     ------> SPDIFRX_IN0
        */
        HAL_GPIO_DeInit(SPDIF_RX0_GPIO_Port, SPDIF_RX0_Pin);
    }
}

/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    if(htim_base->Instance==TIM1)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
    else if(htim_base->Instance==TIM2)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
    else if(htim_base->Instance==TIM3)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
    }
    else if(htim_base->Instance==TIM5)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM5_CLK_ENABLE();
    }
    else if(htim_base->Instance==TIM8)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM8_CLK_ENABLE();
    }
}

/**
* @brief TIM_PWM MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_pwm: TIM_PWM handle pointer
* @retval None
*/
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
    if(htim_pwm->Instance==TIM12)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM12_CLK_ENABLE();
    }

}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(htim->Instance==TIM1)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**TIM1 GPIO Configuration
        PA8     ------> TIM1_CH1
        */
        GPIO_InitStruct.Pin = ARDUINO_PWM_D10_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(ARDUINO_PWM_D10_GPIO_Port, &GPIO_InitStruct);
    }
    else if(htim->Instance==TIM2)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**TIM2 GPIO Configuration
        PA15     ------> TIM2_CH1
        */
        GPIO_InitStruct.Pin = ARDUINO_PWM_D9_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
        HAL_GPIO_Init(ARDUINO_PWM_D9_GPIO_Port, &GPIO_InitStruct);
    }
    else if(htim->Instance==TIM3)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM3 GPIO Configuration
        PB4     ------> TIM3_CH1
        */
        GPIO_InitStruct.Pin = ARDUINO_PWM_D3_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(ARDUINO_PWM_D3_GPIO_Port, &GPIO_InitStruct);
    }
    else if(htim->Instance==TIM5)
    {
        __HAL_RCC_GPIOI_CLK_ENABLE();
        /**TIM5 GPIO Configuration
        PI0     ------> TIM5_CH4
        */
        GPIO_InitStruct.Pin = ARDUINO_PWM_CS_D5_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
        HAL_GPIO_Init(ARDUINO_PWM_CS_D5_GPIO_Port, &GPIO_InitStruct);
    }
    else if(htim->Instance==TIM12)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
        /**TIM12 GPIO Configuration
        PH6     ------> TIM12_CH1
        */
        GPIO_InitStruct.Pin = ARDUINO_PWM_D6_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
        HAL_GPIO_Init(ARDUINO_PWM_D6_GPIO_Port, &GPIO_InitStruct);
    }
}
/**
* @brief TIM_Base MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
    if(htim_base->Instance==TIM1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_TIM1_CLK_DISABLE();
    }
    else if(htim_base->Instance==TIM2)
    {
        /* Peripheral clock disable */
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
    else if(htim_base->Instance==TIM3)
    {
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();
    }
    else if(htim_base->Instance==TIM5)
    {
        /* Peripheral clock disable */
        __HAL_RCC_TIM5_CLK_DISABLE();
    }
    else if(htim_base->Instance==TIM8)
    {
        /* Peripheral clock disable */
        __HAL_RCC_TIM8_CLK_DISABLE();
    }
}

/**
* @brief TIM_PWM MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_pwm: TIM_PWM handle pointer
* @retval None
*/
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{
    if(htim_pwm->Instance==TIM12)
    {
        /* Peripheral clock disable */
        __HAL_RCC_TIM12_CLK_DISABLE();
    }

}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if(huart->Instance==USART1)
    {
        /** Initializes the peripherals clock
        */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
        PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PB7     ------> USART1_RX
        PA9     ------> USART1_TX
        */
        GPIO_InitStruct.Pin = VCP_RX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(VCP_RX_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = VCP_TX_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(VCP_TX_GPIO_Port, &GPIO_InitStruct);
    }
    else if(huart->Instance==USART6)
    {
        /** Initializes the peripherals clock
        */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
        PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_USART6_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**USART6 GPIO Configuration
        PC7     ------> USART6_RX
        PC6     ------> USART6_TX
        */
        GPIO_InitStruct.Pin = ARDUINO_RX_D0_Pin|ARDUINO_TX_D1_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }

}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if(huart->Instance==USART1)
    {
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
        PB7     ------> USART1_RX
        PA9     ------> USART1_TX
        */
        HAL_GPIO_DeInit(VCP_RX_GPIO_Port, VCP_RX_Pin);

        HAL_GPIO_DeInit(VCP_TX_GPIO_Port, VCP_TX_Pin);
    }
    else if(huart->Instance==USART6)
    {
        /* Peripheral clock disable */
        __HAL_RCC_USART6_CLK_DISABLE();

        /**USART6 GPIO Configuration
        PC7     ------> USART6_RX
        PC6     ------> USART6_TX
        */
        HAL_GPIO_DeInit(GPIOC, ARDUINO_RX_D0_Pin|ARDUINO_TX_D1_Pin);
    }
}

static uint32_t SAI2_client =0;

void HAL_SAI_MspInit(SAI_HandleTypeDef* hsai)
{

    GPIO_InitTypeDef GPIO_InitStruct;
/* SAI2 */
    if(hsai->Instance==SAI2_Block_A)
    {
        /* Peripheral clock enable */
        if (SAI2_client == 0)
        {
            __HAL_RCC_SAI2_CLK_ENABLE();
        }
        SAI2_client ++;

        /**SAI2_A_Block_A GPIO Configuration
        PI4     ------> SAI2_MCLK_A
        PI5     ------> SAI2_SCK_A
        PI7     ------> SAI2_FS_A
        PI6     ------> SAI2_SD_A
        */
        GPIO_InitStruct.Pin = SAI2_MCLKA_Pin|SAI2_SCKA_Pin|SAI2_FSA_Pin|SAI2_SDA_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    }
    if(hsai->Instance==SAI2_Block_B)
    {
        /* Peripheral clock enable */
        if (SAI2_client == 0)
        {
            __HAL_RCC_SAI2_CLK_ENABLE();
        }
        SAI2_client ++;

        /**SAI2_B_Block_B GPIO Configuration
        PG10     ------> SAI2_SD_B
        */
        GPIO_InitStruct.Pin = SAI2_SDB_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
        HAL_GPIO_Init(SAI2_SDB_GPIO_Port, &GPIO_InitStruct);

    }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef* hsai)
{
/* SAI2 */
    if(hsai->Instance==SAI2_Block_A)
    {
        SAI2_client --;
        if (SAI2_client == 0)
        {
            /* Peripheral clock disable */
            __HAL_RCC_SAI2_CLK_DISABLE();
        }

        /**SAI2_A_Block_A GPIO Configuration
        PI4     ------> SAI2_MCLK_A
        PI5     ------> SAI2_SCK_A
        PI7     ------> SAI2_FS_A
        PI6     ------> SAI2_SD_A
        */
        HAL_GPIO_DeInit(GPIOI, SAI2_MCLKA_Pin|SAI2_SCKA_Pin|SAI2_FSA_Pin|SAI2_SDA_Pin);

    }
    if(hsai->Instance==SAI2_Block_B)
    {
        SAI2_client --;
        if (SAI2_client == 0)
        {
            /* Peripheral clock disable */
            __HAL_RCC_SAI2_CLK_DISABLE();
        }

        /**SAI2_B_Block_B GPIO Configuration
        PG10     ------> SAI2_SD_B
        */
        HAL_GPIO_DeInit(SAI2_SDB_GPIO_Port, SAI2_SDB_Pin);

    }
}
