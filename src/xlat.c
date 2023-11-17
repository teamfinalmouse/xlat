/*
 * Copyright (C) 2023 Finalmouse, LLC
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

/* For LVGL / GFX */
#include <math.h>
#include "lvgl/lvgl.h"
#include "tft/tft.h"
#include "touchpad/touchpad.h"
#include "gfx_main.h"

#include "main.h"
#include "xlat.h"
#include "usbh_hid.h"
#include "stm32f7xx_hal_tim.h"
#include "hardware_config.h"
#include "stdio_glue.h"

static uint32_t last_btn_gpio_timestamp = 0;
static uint32_t last_usb_timestamp_us = 0;
static uint32_t last_latency_us[LATENCY_TYPE_MAX];
static uint64_t average_latency_us_sum[LATENCY_TYPE_MAX]; // sum of all measurements
static uint64_t average_latency_us_sum_sq[LATENCY_TYPE_MAX]; // sum of squares, for variance
static uint32_t average_latency_us_count[LATENCY_TYPE_MAX];

static volatile uint_fast8_t gpio_irq_producer = 0;
static volatile uint_fast8_t gpio_irq_consumer = 0;

// SETTINGS
volatile bool       xlat_initialized = false;
static xlat_mode_t  xlat_mode = XLAT_MODE_CLICK;
static bool         hid_using_reportid = false;
static bool         auto_trigger_level_high = false;

// The Razer optical switches will constantly trigger the GPIO interrupt, while pressed
// Waveform looks like this in ASCII art:
//
// <   unpressed   ><    pressed         ><    unpressed    >
// _________________    __    __    __    __________________
//                  \__/  \__/  \__/  \__/
//
// Therefore, take a large enough time window to debounce the GPIO interrupt.
#define GPIO_IRQ_HOLDOFF_US (50 * 1000)  // 20ms;
static uint32_t gpio_irq_holdoff_us = GPIO_IRQ_HOLDOFF_US;
static TimerHandle_t xlat_timer_handle;
static uint16_t trigger_count = 1000;

// gpio_irq_holdoff_us setter
void xlat_set_gpio_irq_holdoff_us(uint32_t us)
{
    printf("Setting GPIO IRQ holdoff to %lu us\n", us);
    gpio_irq_holdoff_us = us;
}

uint32_t xlat_get_gpio_irq_holdoff_us(void)
{
    return gpio_irq_holdoff_us;
}
uint16_t xlat_set_auto_trigger_count(uint16_t count)
{
    trigger_count = count;
}
uint16_t xlat_get_auto_trigger_count(void)
{
    return trigger_count;
}

uint32_t xlat_counter_1mhz_get(void)
{
    return __HAL_TIM_GET_COUNTER(&XLAT_TIMx_handle);
}


static USBH_StatusTypeDef USBH_HID_GetRawData(USBH_HandleTypeDef *phost, uint8_t *hid_raw_data)
{
    HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

    if (HID_Handle->length == 0U)
    {
        return USBH_FAIL;
    }
    /*Fill report */
    if (USBH_HID_FifoRead(&HID_Handle->fifo, hid_raw_data, HID_Handle->length) == HID_Handle->length)
    {
        return USBH_OK;
    }
    return   USBH_FAIL;
}

static int calculate_gpio_to_usb_time(void)
{
    // only accept if there was a gpio irq first
    if (gpio_irq_producer == gpio_irq_consumer) {
        return -1;
    }
    gpio_irq_consumer = gpio_irq_producer;

    xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
    gfx_set_trigger_ready(false);
    xSemaphoreGive(lvgl_mutex);

    // gpio -> usb stats
    int32_t us = last_usb_timestamp_us - last_btn_gpio_timestamp;
    printf("[gpio -> usb] diff: us: %5ld\n", us);
    xlat_add_latency_measurement(us > 0 ? us : 0, LATENCY_GPIO_TO_USB);

    // send a message to the gfx thread, to refresh the plot
    osMessagePut(msgQNewData, us, 0);

    return 0;
}

void xlat_usb_hid_event(void)
{
    osEvent evt;
    evt = osMessageGet(msgQUsbClick, osWaitForever);  // wait for message
    if (evt.status != osEventMessage) {
        return;
    }

    struct hid_event *hevt = evt.value.p;
    USBH_HandleTypeDef *phost = hevt->phost;

    if (USBH_HID_GetDeviceType(phost) == HID_MOUSE)
    {  // if the HID is Mouse
        uint8_t hid_raw_data[64];
        static uint8_t prev_byte_0 = 0;
        static uint8_t prev_byte_1 = 0;
        static bool first_data = 1;

        if (USBH_HID_GetRawData(phost, hid_raw_data) == USBH_OK) {
            // check reportId for ULX
            if (hid_using_reportid && (hid_raw_data[0] != 0x01)) {
                // ignore
                goto out;
            }
#if 0
            printf("[%5lu] hid@%lu: ", xTaskGetTickCount(), hevt->timestamp);
            for (int i = 0; i < 8 /*sizeof(hid_raw_data) */; i++) {
                printf("%02x ", hid_raw_data[i]);
            }
            printf("\n");
#endif
            if (xlat_mode == XLAT_MODE_CLICK) {
                // FOR BUTTONS:

                // Either the first byte contains the button data, or the second byte
                // For ULX, the first byte must be 0x01 (reportId) for mouse data, so we use the second byte
                // For other mice, the first byte usually contains the button data
                uint8_t byte0 = hid_raw_data[0];
                uint8_t byte1 = hid_raw_data[1];

                // Check if the button state has changed
                if (byte0 != prev_byte_0 || byte1 != prev_byte_1) {
                    // Use USB trigger
                    // Only on button PRESS
                    if (first_data) {
                        first_data = false;
                    } else if (((byte0 > prev_byte_0) && !hid_using_reportid) ||
                               ((byte1 > prev_byte_1) && hid_using_reportid)) {
                        // Save the captured USB event timestamp
                        last_usb_timestamp_us = hevt->timestamp;

                        printf("[%5lu] hid@%lu: ", xTaskGetTickCount(), hevt->timestamp);
                        printf("Button: B=0x%02x%02x @ %lu\n",
                               hid_raw_data[0], hid_raw_data[1], hevt->timestamp);

                        calculate_gpio_to_usb_time();
                    }
                }

                // save previous state
                prev_byte_0 = byte0;
                prev_byte_1 = byte1;
            }
            else if (xlat_mode == XLAT_MODE_MOTION) {
                // FOR MOTION:

                // Where to find the X Y motion bytes?
                // E.g. Zowie EC1-CW: 2 bytes X, 2 bytes Y, at position 1-2 and 2-3.
                // E.g. ULX: 2 bytes X, 2 bytes Y, at position 2-3 and 4-5.
                size_t base = hid_using_reportid ? 2 : 1;

                // Check if there's any motion data (ULX):
                if (hid_raw_data[base] | hid_raw_data[base+1] | hid_raw_data[base+2] | hid_raw_data[base+3]) {
                    // Save the captured USB event timestamp
                    last_usb_timestamp_us = hevt->timestamp;

                    if (!calculate_gpio_to_usb_time()) {
                        printf("[%5lu] MOTION@%lu: ", xTaskGetTickCount(), hevt->timestamp);
                    }
                }
            }
        }
    }

#if 0
    if (USBH_HID_GetDeviceType(phost) == HID_KEYBOARD) {  // if the HID is Keyboard
        uint8_t key;
        HID_KEYBD_Info_TypeDef *Keyboard_Info;
        Keyboard_Info = USBH_HID_GetKeybdInfo(phost);  // get the info
        key = USBH_HID_GetASCIICode(Keyboard_Info);  // get the key pressed
        printf("Key Pressed = %c\n", key);
    }
#endif

out:
    // free event memory
    osPoolFree(hidevt_pool, hevt);
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t cnt = xlat_counter_1mhz_get();
    // debounce X ms
    if (cnt - last_btn_gpio_timestamp < gpio_irq_holdoff_us) {
        return;
    }
    last_btn_gpio_timestamp = cnt;
    gpio_irq_producer++;

    // disable the interrupt and re-enable later in a timer
    hw_exti_interrupts_disable();
    xTimerChangePeriodFromISR(xlat_timer_handle, pdMS_TO_TICKS(gpio_irq_holdoff_us / 1000), NULL);
    xTimerStartFromISR(xlat_timer_handle, NULL);

    // print the event
    printf("[%5lu] GPIO interrupt for pin: %3d @ %lu\n", xTaskGetTickCountFromISR(), GPIO_Pin, cnt);
    printf("GPIO irq 0x%x @ %lu\n", GPIO_Pin, cnt);
}


/**
  * @brief  The function is a callback about HID Data events
  *  @param  phost: Selected device
  * @retval None
  */

// In this callback the timestamp is wrapped in an event and sent to the main thread
void USBH_HID_EventCallback(USBH_HandleTypeDef *phost, uint32_t timestamp)
{
    struct hid_event *evt;

    HAL_GPIO_WritePin(ARDUINO_D5_GPIO_Port, ARDUINO_D5_Pin, 1);

    evt = osPoolAlloc(hidevt_pool);                     // Allocate memory for the message
    evt->timestamp = timestamp;
    evt->phost = phost;
    osMessagePut(msgQUsbClick, (uint32_t)evt, 0U);

    HAL_GPIO_WritePin(ARDUINO_D5_GPIO_Port, ARDUINO_D5_Pin, 0);
}


uint32_t xlat_get_latency_us(enum latency_type type)
{
    if (type >= LATENCY_TYPE_MAX) {
        return 0;
    }
    return last_latency_us[type];
}

uint32_t xlat_get_last_button_timestamp_us(void)
{
    return last_btn_gpio_timestamp;
}

uint32_t xlat_get_average_latency(enum latency_type type)
{
    if (type >= LATENCY_TYPE_MAX) {
        return 0;
    }
    return (uint32_t)(average_latency_us_sum[type] / average_latency_us_count[type]);
}

uint32_t xlat_get_latency_variance(enum latency_type type)
{
    if (type >= LATENCY_TYPE_MAX) {
        return 0;
    }
    uint64_t avg = average_latency_us_sum[type] / average_latency_us_count[type];
    uint64_t avg_sq = average_latency_us_sum_sq[type] / average_latency_us_count[type];
    return (uint32_t)(avg_sq - avg * avg);
}

uint32_t xlat_get_latency_standard_deviation(enum latency_type type)
{
    return (uint32_t)sqrt(xlat_get_latency_variance(type));
}

uint32_t xlat_get_last_usb_timestamp_us(void)
{
    return last_usb_timestamp_us;
}

uint32_t xlat_get_latency_count(enum latency_type type)
{
    if (type >= LATENCY_TYPE_MAX) {
        return 0;
    }
    return average_latency_us_count[type];
}

void xlat_add_latency_measurement(uint32_t latency_us, enum latency_type type)
{
    if (type >= LATENCY_TYPE_MAX) {
        return;
    }
    last_latency_us[type] = latency_us;
    average_latency_us_sum[type] += latency_us;
    average_latency_us_sum_sq[type] += latency_us * latency_us;
    average_latency_us_count[type]++;

//    printf(">>> GPIO->USB latency: %5lu us, ", last_gpio_to_usb_latency_us);
//    printf("average latency: %5lu us\n", (uint32_t)(average_latency_us_sum / average_latency_us_count));
}

void xlat_reset_latency(void)
{
    for (int i = 0; i < LATENCY_TYPE_MAX; i++) {
        last_latency_us[i] = 0;
        average_latency_us_sum[i] = 0;
        average_latency_us_sum_sq[i] = 0;
        average_latency_us_count[i] = 0;
    }
}

void xlat_set_using_reportid(bool use_reportid)
{
    hid_using_reportid = use_reportid;
}

bool xlat_get_using_reportid(void)
{
    return hid_using_reportid;
}

static void xlat_timer_callback(TimerHandle_t xTimer)
{
    // re-enable GPIO interrupts
    hw_exti_interrupts_enable();

    // Update the trigger ready flag in the UI
    xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
    gfx_set_trigger_ready(true);
    xSemaphoreGive(lvgl_mutex);
}

void xlat_set_mode(enum xlat_mode mode)
{
    xlat_mode = mode;
}

enum xlat_mode xlat_get_mode(void)
{
    return xlat_mode;
}

void xlat_auto_trigger_action(void)
{
    HAL_GPIO_WritePin(ARDUINO_D11_GPIO_Port, ARDUINO_D11_Pin, auto_trigger_level_high ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(ARDUINO_D11_GPIO_Port, ARDUINO_D11_Pin, auto_trigger_level_high ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void xlat_auto_trigger_level_set(bool high)
{
    auto_trigger_level_high = high;
}

bool xlat_auto_trigger_level_is_high(void)
{
    return auto_trigger_level_high;
}

void xlat_print_measurement(void)
{
    // print the new measurement to the console in csv format
    char buf[50];
    snprintf(buf, sizeof(buf), "%lu;%lu;%lu;%lu\r\n",
             xlat_get_latency_count(LATENCY_GPIO_TO_USB),
             xlat_get_latency_us(LATENCY_GPIO_TO_USB),
             xlat_get_average_latency(LATENCY_GPIO_TO_USB),
             xlat_get_latency_standard_deviation(LATENCY_GPIO_TO_USB));
    vcp_writestr(buf);
}

void xlat_init(void)
{
    // create timer
    xlat_timer_handle = xTimerCreate("xlat_timer", pdMS_TO_TICKS(1000), pdFALSE, NULL, xlat_timer_callback);
    hw_exti_interrupts_enable();
    xlat_initialized = true;
    printf("XLAT initialized\n");

    char buf[50];
    snprintf(buf, sizeof(buf), "count;latency_us;avg_us;stdev_us\r\n");
    vcp_writestr(buf);
}
