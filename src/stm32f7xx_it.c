/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f7xx_it.h"

#include <tusb.h>


/* Private function prototypes -----------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern HCD_HandleTypeDef hhcd_USB_OTG_HS;
extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;
extern TIM_HandleTypeDef htim6;

extern DMA_HandleTypeDef   hdma;
/* SAI handler declared in "stm32746g_discovery_audio.c" file */
extern SAI_HandleTypeDef haudio_out_sai;
/* SAI handler declared in "stm32746g_discovery_audio.c" file */
extern SAI_HandleTypeDef haudio_in_sai;
/* SDRAM handler declared in "stm32746g_discovery_sdram.c" file */
extern SDRAM_HandleTypeDef sdramHandle;

/******************************************************************************/
/*           Cortex-M7 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
    while (1)
    {
    }
}


/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}



/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(ARDUINO_SCK_D13_Pin);
}

/**
  * @brief  This function handles External line 2 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void)
{
   HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(ARDUINO_D2_Pin);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_WritePin(ARDUINO_D5_GPIO_Port, ARDUINO_D5_Pin, 1);
    HAL_GPIO_EXTI_IRQHandler(ARDUINO_D12_Pin);
    HAL_GPIO_WritePin(ARDUINO_D5_GPIO_Port, ARDUINO_D5_Pin, 0);
}

/**
  * @brief This function handles DMA2 Stream 7 interrupt request.
  * @param None
  * @retval None
  */
void AUDIO_IN_SAIx_DMAx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(haudio_in_sai.hdmarx);
}

/**
  * @brief  This function handles DMA2 Stream 6 interrupt request.
  * @param  None
  * @retval None
  */
void AUDIO_OUT_SAIx_DMAx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
}

/**
  * @brief This function handles DMA2D global interrupt.
  */
void DMA2D_IRQHandler(void)
{
    HAL_DMA2D_IRQHandler(&hdma2d);
}


/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}

/**
  * @brief This function handles LTDC global interrupt.
  */
void LTDC_IRQHandler(void)
{
    HAL_LTDC_IRQHandler(&hltdc);
}

/**
  * @brief Forward USB interrupt events to TinyUSB IRQ Handler
  */
void OTG_FS_IRQHandler(void) {
  //tusb_int_handler(0, true);
}

/**
 * @brief Forward USB interrupt events to TinyUSB IRQ Handler
 * Despite being called USB2_OTG
 * OTG_HS is marked as RHPort1 by TinyUSB to be consistent across stm32 port
 */
void OTG_HS_IRQHandler(void) {
    extern void hcd_int_handler(uint8_t rhport, bool in_isr);
    HAL_GPIO_WritePin(ARDUINO_D3_GPIO_Port, ARDUINO_D3_Pin, GPIO_PIN_SET);
    // tusb_int_handler(1, true);
    hcd_int_handler(1, true);
    HAL_GPIO_WritePin(ARDUINO_D3_GPIO_Port, ARDUINO_D3_Pin, GPIO_PIN_RESET);
}
