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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* For LVGL / GFX */
#include "touchpad/touchpad.h"
#include "gfx_main.h"

#include "main.h"
#include "xlat.h"
#include "stm32f7xx_hal_tim.h"
#include "hardware_config.h"
#include "stdio_glue.h"
#include "class/hid/hid.h"

// LUFA HID Parser
#define __INCLUDE_FROM_USB_DRIVER // NOLINT(*-reserved-identifier)
#define __INCLUDE_FROM_HID_DRIVER // NOLINT(*-reserved-identifier)
#include <usb_task.h>

#include "Drivers/USB/Class/Common/HIDParser.h"

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
static xlat_mode_t  xlat_mode = XLAT_MODE_MOUSE_CLICK;
static bool         auto_trigger_level_high = false;

// The Razer optical switches will constantly trigger the GPIO interrupt, while pressed
// Waveform looks like this in ASCII art:
//
// <   unpressed   ><    pressed         ><    unpressed    >
// _________________    __    __    __    __________________
//                  \__/  \__/  \__/  \__/
//
// Therefore, take a large enough time window to debounce the GPIO interrupt.
#define GPIO_IRQ_HOLDOFF_US (100 * 1000)  // 100ms;
static uint32_t gpio_irq_holdoff_us = GPIO_IRQ_HOLDOFF_US;
static TimerHandle_t xlat_timer_handle;


///////////////////////
// PRIVATE FUNCTIONS //
///////////////////////

// Locations of the clicks and X Y motion bytes in the HID report
#define REPORT_LEN 64
uint8_t prev_report[REPORT_LEN];
uint8_t button_mask[REPORT_LEN];
uint8_t motion_mask[REPORT_LEN];
uint16_t button_bits;
uint16_t motion_bits;
uint8_t report_id;

static inline void hidreport_print_item(HID_ReportItem_t *item)
{
    printf("  BitOffset: %d\n", item->BitOffset);
    switch (item->ItemType) {
        case HID_REPORT_ITEM_In:
            printf("  ItemType: In\n");
            break;
        case HID_REPORT_ITEM_Out:
            printf("  ItemType: Out\n");
            break;
        case HID_REPORT_ITEM_Feature:
            printf("  ItemType: Feature\n");
            break;
        default:
            break;
    }
    printf("  ItemFlags: %d\n", item->ItemFlags);
    printf("  ReportID: %d\n", item->ReportID);
    printf("  Value: 0x%lx\n", item->Value);

    printf("  Attributes:\n");
    printf("    Usage.BitSize: %d\n", item->Attributes.BitSize);
    switch (item->Attributes.Usage.Page) {
        case 0x01:
            printf("    Usage.Page:  Generic Desktop (0x0001)\n");
            switch (item->Attributes.Usage.Usage) {
                case 0x02:
                    printf("    Usage.Usage: Mouse (0x0002)\n");
                    break;

                case 0x30:
                    printf("    Usage.Usage: X (0x0030)\n");
                    break;

                case 0x31:
                    printf("    Usage.Usage: Y (0x0031)\n");
                    break;

                case 0x38:
                    printf("    Usage.Usage: Wheel (0x0038)\n");
                    break;
            }
            break;

        case 0x07:
            printf("    Usage.Page:  Keyboard/Keypad (0x0007)\n");
            break;

        case 0x08:
            printf("    Usage.Page:  LEDs (0x0008)\n");
            break;

        case 0x09:
            printf("    Usage.Page:  Button (0x0009)\n");
            break;

        case 0x0C:
            printf("    Usage.Page:  Consumer (0x000C)\n");
            break;

        default:
            printf("  Usage.Page:  0x%04x (%d)\n", item->Attributes.Usage.Page, item->Attributes.Usage.Page);
            break;
    }
    printf("    Usage.Usage: 0x%04x (%d)\n", item->Attributes.Usage.Usage, item->Attributes.Usage.Usage);
    printf("    Unit.Type:     %lu\n", item->Attributes.Unit.Type);
    printf("    Unit.Exponent: %d\n", item->Attributes.Unit.Exponent);
    // Sign-extend the logical minimum and maximum, using Usage.BitSize
    int32_t val = item->Attributes.Logical.Minimum;
    val = (val << (32 - item->Attributes.BitSize)) >> (32 - item->Attributes.BitSize);
    printf("    Unit.Logical.Minimum:     %ld\n", val);
    val = item->Attributes.Logical.Maximum;
    val = (val << (32 - item->Attributes.BitSize)) >> (32 - item->Attributes.BitSize);
    printf("    Unit.Logical.Maximum:     %ld\n", val);
    printf("    Unit.Physical.Minimum:    %lu\n", item->Attributes.Physical.Minimum);
    printf("    Unit.Physical.Maximum:    %lu\n", item->Attributes.Physical.Maximum);

    HID_CollectionPath_t *collection = item->CollectionPath;
    printf("  Collection\n");
    printf("    Type: %d\n", collection->Type);
    printf("    Usage.Page:  0x%04x (%d)\n", collection->Usage.Page, collection->Usage.Page);
    printf("    Usage.Usage: 0x%04x (%d)\n", collection->Usage.Usage, collection->Usage.Usage);
    printf("\n");
}

static void hidreport_check_item(HID_ReportItem_t *item)
{
    if (item->ItemType != HID_REPORT_ITEM_In) {
        return;
    }

    uint8_t* mask = NULL;
    uint16_t* bits = NULL;

    // Print the item
    // hidreport_print_item(item);

    // Usage Page 0x0009: Buttons
    if (item->Attributes.Usage.Page == 0x0009) {
        mask = button_mask;
        bits = &button_bits;
    }
    // Usage Page 0x0001: Generic Desktop
    // Usage 0x0030: X
    // Usage 0x0031: Y
    if ((item->Attributes.Usage.Page == 0x0001) &&
        ((item->Attributes.Usage.Usage == 0x0030) || (item->Attributes.Usage.Usage == 0x0031))) {
        mask = motion_mask;
        bits = &motion_bits;
    }

    if (mask != NULL) {
        if (report_id == 0) {
            report_id = item->ReportID;
        }
        if (report_id != item->ReportID) {
            return;
        }
        for (uint8_t i = 0; i < item->Attributes.BitSize; i++) {
            int byte_no = (item->BitOffset + i) / 8;
            int bit_no = (item->BitOffset + i) % 8;
            byte_no += (report_id ? 1 : 0);
            if (byte_no < sizeof(button_mask)) {
                mask[byte_no] |= (1 << bit_no);
                (*bits)++;
            }
        }
    }
}

// Mandatory HIDParser callback
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem)
{
    hidreport_check_item(CurrentItem);
    return true;
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

    // drop negative values
    if (us < 0) {
        return -1;
    }

    xlat_add_latency_measurement(us, LATENCY_GPIO_TO_USB);

    // send a message to the gfx thread, to refresh the plot
    struct gfx_event *evt;
    evt = osPoolAlloc(gfxevt_pool); // Allocate memory for the message
    evt->type = GFX_EVENT_MEASUREMENT;
    evt->value = us;
    osMessagePut(msgQGfxTask, (uint32_t)evt, 0U);

    return 0;
}


//////////////////////
// PUBLIC FUNCTIONS //
//////////////////////

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


uint32_t xlat_counter_1mhz_get(void)
{
    return __HAL_TIM_GET_COUNTER(&XLAT_TIMx_handle);
}


void xlat_process_usb_hid_event(void)
{
    osEvent evt;
    evt = osMessageGet(msgQUsbHidEvent, osWaitForever);  // wait for message
    if (evt.status != osEventMessage) {
        return;
    }

    struct hid_event *hevt = evt.value.p;

    switch (hevt->itf_protocol) {
        case HID_ITF_PROTOCOL_MOUSE: {
            uint8_t* hid_raw_data = hevt->report;

            // Check if the report ID is matching what's expected
            if ((report_id != 0) && (hid_raw_data[0] != report_id)) {
                // ignore
                goto out;
            }
    
#if 0
            printf("[%5lu] hid@%lu: ", xTaskGetTickCount(), hevt->timestamp);
            for (int i = 0; i < hevt->report_size; i++) {
                printf("%02x ", hid_raw_data[i]);
            }
            printf("\n");
#endif

            // FOR BUTTONS/CLICKS:
            if (xlat_mode == XLAT_MODE_MOUSE_CLICK) {
                // The correct location of button data is determined by parsing the HID descriptor
                // This information is available in the button_mask
                for (uint8_t i = (report_id ? 1 : 0); i < hevt->report_size; i++) {
                    if (((hid_raw_data[i] ^ prev_report[i]) & hid_raw_data[i] & button_mask[i])) {
                        last_usb_timestamp_us = hevt->timestamp;
                        calculate_gpio_to_usb_time();
                        printf("[%5lu] hid click @ %lu - byte %d\n", xTaskGetTickCount(), hevt->timestamp, i);
                        break;
                    }
                }
            }
            // FOR MOTION:
            else if (xlat_mode == XLAT_MODE_MOUSE_MOTION) {
                // The correct location of button data is determined by parsing the HID descriptor
                // This information is available in the motion_mask
                for (uint8_t i = (report_id ? 1 : 0); i < hevt->report_size; i++) {
                    if (hid_raw_data[i] & motion_mask[i]) {
                        last_usb_timestamp_us = hevt->timestamp;
                        calculate_gpio_to_usb_time();
                        printf("[%5lu] hid motion @ %lu\n", xTaskGetTickCount(), hevt->timestamp);
                        break;
                    }
                }
            }
            // Save the report for the next iteration
            memcpy(prev_report, hid_raw_data, sizeof(prev_report));
        }

        case HID_ITF_PROTOCOL_KEYBOARD:
            if (xlat_mode != XLAT_MODE_KEYBOARD) {
                goto out;
            }
            hid_keyboard_report_t *kbd_report = (hid_keyboard_report_t *)hevt->report;
            // loop over the keycode array to see if any keys are pressed:
            for (uint8_t i = 0; i < 6; i++) {
                if (kbd_report->keycode[i]) {
                    if (kbd_report->keycode[i] > 1) {
                        // Save the captured USB event timestamp
                        last_usb_timestamp_us = hevt->timestamp;
                        printf("USB HID event: key press 0x%02X\n", kbd_report->keycode[i]);
                        // calculate the time between the last key press and this key press:
                        calculate_gpio_to_usb_time();
                    }
                }
            }
            break;

        default:
            break;
    }

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
    // printf("[%5lu] GPIO interrupt for pin: %3d @ %lu\n", xTaskGetTickCountFromISR(), GPIO_Pin, cnt);
    // printf("GPIO irq 0x%x @ %lu\n", GPIO_Pin, cnt);
}


/**
  * @brief  The function is a callback about HID Data events
  *  @param  phost: Selected device
  * @retval None
  */

// In this callback the timestamp is wrapped in an event and sent to the main thread
void xlat_usb_event_callback(uint32_t timestamp, uint8_t const *report, size_t report_size, uint8_t itf_protocol)
{
    struct hid_event *evt;

    evt = osPoolAlloc(hidevt_pool); // Allocate memory for the message
    evt->timestamp = timestamp;
    evt->itf_protocol = itf_protocol;
    if (report_size > sizeof(evt->report)) {
        report_size = sizeof(evt->report);
    }
    evt->report_size = report_size;
    memcpy(evt->report, report, report_size);
    osMessagePut(msgQUsbHidEvent, (uint32_t)evt, 0U);
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
    // random delay, such that we do not always perfectly align with USB timing
    srand(xTaskGetTickCount());
    int val = rand() & 0xFFF;
    for (volatile int i = 0; i < val; i++) {
        __NOP();
    }
    HAL_GPIO_WritePin(ARDUINO_D11_GPIO_Port, ARDUINO_D11_Pin, auto_trigger_level_high ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void xlat_auto_trigger_turn_off_action(void)
{
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
    snprintf(buf, sizeof(buf), "%lu;%lu;%lu;%lu\n",
             xlat_get_latency_count(LATENCY_GPIO_TO_USB),
             xlat_get_latency_us(LATENCY_GPIO_TO_USB),
             xlat_get_average_latency(LATENCY_GPIO_TO_USB),
             xlat_get_latency_standard_deviation(LATENCY_GPIO_TO_USB));
    vcp_writestr(buf);
}


void xlat_parse_hid_descriptor(uint8_t *desc, size_t desc_size, uint8_t itf_protocol)
{
    HID_ReportInfo_t report_info; // Only 333b when using HID_PARSER_STREAM_ONLY

    printf("Parsing HID descriptor with size: %d, itf_protocol: %d\n", desc_size, itf_protocol);

    // Only parse according to the current XLAT mode
    switch (xlat_mode) {
        case XLAT_MODE_MOUSE_CLICK:
        case XLAT_MODE_MOUSE_MOTION:
            if (itf_protocol != HID_ITF_PROTOCOL_MOUSE) {
                return;
            }
            break;
        case XLAT_MODE_KEYBOARD:
            if (itf_protocol != HID_ITF_PROTOCOL_KEYBOARD) {
                return;
            }
            break;
        default:
            return;
    }

    int err = USB_ProcessHIDReport(desc, desc_size, &report_info);
    if (err != HID_PARSE_Successful) {
        printf("[ERROR] USB_ProcessHIDReport: %d\n", err);
        return;
    }

    printf("Button mask: ");
    for (int i = 0; i < REPORT_LEN; i++) {
        printf("%02x", button_mask[i]);
    }
    printf("\n");
    printf("Motion mask: ");
    for (int i = 0; i < REPORT_LEN; i++) {
        printf("%02x", motion_mask[i]);
    }
    printf("\n");

    // Check if using reportIDs:
    printf("Using report ID: %d\n", report_id);
}

void xlat_clear_device_info(void)
{
    // Clear offsets
    xlat_clear_locations();
    // Send a message to the gfx thread, to refresh the device info
    gfx_send_event(GFX_EVENT_DEVICE_DISCONNECTED, 0);
}

uint16_t xlat_get_button_bits(void)
{
    return button_bits;
}

uint16_t xlat_get_motion_bits(void)
{
    return motion_bits;
}

uint16_t xlat_get_report_id(void)
{
    return report_id;
}

void xlat_clear_locations(void)
{
    printf("Clearing locations\n");
    memset(prev_report, 0, sizeof(prev_report));
    memset(button_mask, 0, sizeof(button_mask));
    memset(motion_mask, 0, sizeof(motion_mask));
    button_bits = 0;
    motion_bits = 0;
    report_id = 0;
}

void xlat_init(void)
{
    // create timer
    xlat_timer_handle = xTimerCreate("xlat_timer", pdMS_TO_TICKS(1000), pdFALSE, NULL, xlat_timer_callback);
    xlat_clear_locations();
    hw_exti_interrupts_enable();
    xlat_initialized = true;
    printf("XLAT initialized\n");

    char buf[50];
    snprintf(buf, sizeof(buf), "count;latency_us;avg_us;stdev_us\n");
    vcp_writestr(buf);
}

/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
void xlat_task(void const * argument)
{
    (void)argument;

    // RTT prints
    printf("\n");
    printf("******************************\n");
    printf("Welcome to XLAT v%s\n", APP_VERSION_FULL);
    printf("******************************\n\n");

    // UART prints
    vcp_writestr("\n");
    vcp_writestr("******************************\n");
    vcp_writestr("Welcome to XLAT v");
    vcp_writestr(APP_VERSION_FULL);
    vcp_writestr("\n");
    vcp_writestr("******************************\n\n");

    xlat_init();

    /* Infinite loop */
    for (;;) {
        xlat_process_usb_hid_event(); // blocking
    }
}
