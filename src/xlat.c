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
#include "touchpad/touchpad.h"
#include "gfx_main.h"

#include "main.h"
#include "xlat.h"
#include "usbh_hid.h"
#include "stm32f7xx_hal_tim.h"
#include "hardware_config.h"
#include "stdio_glue.h"

// LUFA HID Parser
#define __INCLUDE_FROM_USB_DRIVER // NOLINT(*-reserved-identifier)
#define __INCLUDE_FROM_HID_DRIVER // NOLINT(*-reserved-identifier)
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


///////////////////////
// PRIVATE FUNCTIONS //
///////////////////////

// Locations of the clicks and X Y motion bytes in the HID report
hid_data_location_t button_location;
hid_data_location_t x_location;
hid_data_location_t y_location;

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
    switch (item->Attributes.Usage.Page) {
        case 0x01:
            switch (item->Attributes.Usage.Usage) {
                case 0x30:
                    printf("    Usage.Usage: X (0x0030)\n");
                    if (!x_location.found) {
                        x_location.found = true;
                        x_location.bit_index = item->BitOffset;
                        x_location.bit_size = item->Attributes.BitSize;
                    }
                    break;

                case 0x31:
                    printf("    Usage.Usage: Y (0x0031)\n");
                    if (!y_location.found) {
                        y_location.found = true;
                        y_location.bit_index = item->BitOffset;
                        y_location.bit_size = item->Attributes.BitSize;
                    }
                    break;
            }
            break;

        case 0x09:
            printf("    Usage.Page:  Button (0x0009)\n");
            if (!button_location.found) {
                button_location.found = true;
                button_location.bit_index = item->BitOffset;
            }
            break;

        default:
            break;
    }
}

// Mandatory HIDParser callback
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem)
{
    hidreport_check_item(CurrentItem);
    return true;
}


static USBH_StatusTypeDef USBH_HID_GetRawData(USBH_HandleTypeDef *phost, uint8_t *hid_raw_data)
{
    HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

    if (HID_Handle->length == 0U) {
        return USBH_FAIL;
    }

    /* Fill report */
    if (USBH_HID_FifoRead(&HID_Handle->fifo,
                          hid_raw_data,
                          HID_Handle->length) == HID_Handle->length) {
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


static void check_offsets(void)
{
    printf("\n");

    if (hid_using_reportid) {
        printf("[*] Using reportId, so actual report data is starting at index [1]\n");
    }

    if (button_location.found) {
        if (button_location.bit_index % 8) {
            printf("[!] Button found at bit index %d, which is not a multiple of 8. Currently not supported by XLAT.\n", button_location.bit_index);
            button_location.found = false;
        } else {
            button_location.byte_offset = button_location.bit_index / 8 + (size_t)hid_using_reportid;
            printf("[*] Button found at bit index %d, which is byte %d\n", button_location.bit_index, button_location.bit_index / 8);
            printf("    Button byte offset: %d\n", button_location.byte_offset);
        }
    } else {
        button_location.found = false;
        printf("[x] Button not found\n");
    }

    // X offset has to start at a byte boundary
    if (x_location.found) {
        if (x_location.bit_index % 8) {
            printf("[!] X found at bit index %d, which is not a multiple of 8. Currently not supported by XLAT.\n", x_location.bit_index);
            x_location.found = false;
        } else {
            x_location.byte_offset = x_location.bit_index / 8 + (size_t)hid_using_reportid;
            printf("[*] X found at bit index %d, which is byte %d\n", x_location.bit_index, x_location.bit_index / 8);
            printf("    X size: %d bits, %d bytes\n", x_location.bit_size, x_location.bit_size / 8);
            printf("    X byte offset: %d\n", x_location.byte_offset);
        }
    } else {
        x_location.found = false;
        printf("[x] X not found\n");
    }

    // Y offset does NOT have to start at a byte boundary,
    // but it has to be contiguous to X
    // and their total size has to be a multiple of 8
    if (y_location.found) {
        if ((y_location.bit_index % 8) &&
                ((y_location.bit_index - x_location.bit_index) != x_location.bit_size)) {
            printf("[!] Y found at bit index %d, which is not a multiple of 8, and not contiguous to X. Currently not supported by XLAT.\n",
                   y_location.bit_index);
            y_location.found = false;
        } else {
            y_location.byte_offset = y_location.bit_index / 8 + (size_t)hid_using_reportid;
            printf("[*] Y found at bit index %d, which is byte %d\n", y_location.bit_index, y_location.bit_index / 8);
            printf("    Y size: %d bits, %d bytes\n", y_location.bit_size, y_location.bit_size / 8);
            printf("    Y byte offset: %d\n", y_location.byte_offset);
        }
    } else {
        y_location.found = false;
        printf("[x] Y not found\n");
    }

    printf("\n");
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
                // FOR BUTTONS/CLICKS:
                // The correct location of button data is determined by parsing the HID descriptor
                // This information is available in the button_location struct

                // First, check if the location was found
                if (!button_location.found) {
                    return;
                }

                static uint8_t prev_button = 0;
                uint8_t button = hid_raw_data[button_location.byte_offset];

                // Check if the button state has changed
                if (button != prev_button) {
                    // Only measure on button PRESS, not on RELEASE
                    if (button > prev_button) {
                        // Save the captured USB event timestamp
                        last_usb_timestamp_us = hevt->timestamp;

                        printf("[%5lu] hid@%lu: ", xTaskGetTickCount(), hevt->timestamp);
                        printf("Button: B=0x%02x @ %lu\n", button, hevt->timestamp);

                        calculate_gpio_to_usb_time();
                    }
                }

                // Save previous state
                prev_button = button;
            }
            else if (xlat_mode == XLAT_MODE_MOTION) {
                // FOR MOTION:
                // The correct location of button data is determined by parsing the HID descriptor
                // This information is available in the [x|y]_location structs

                // First, check if the locations were found
                if ((!x_location.found) || (!y_location.found)) {
                    return;
                }

                // Check X and Y data is contiguous
                if ((x_location.byte_offset + x_location.bit_size / 8) != y_location.byte_offset) {
                    return;
                }

                size_t start_idx = x_location.byte_offset;
                size_t length = (x_location.bit_size + y_location.bit_size) / 8;

                // Loop over the data and check for non-zero values
                // In case there is non-zero data, call calculate_gpio_to_usb_tine();
                for (size_t idx = start_idx; idx < start_idx + length; idx++) {
                    if (hid_raw_data[idx]) {
                        // Save the captured USB event timestamp
                        last_usb_timestamp_us = hevt->timestamp;

                        printf("[%5lu] hid@%lu: ", xTaskGetTickCount(), hevt->timestamp);
                        printf("Motion: X=0x%02x, Y=0x%02x @ %lu\n", hid_raw_data[x_location.byte_offset], hid_raw_data[y_location.byte_offset], hevt->timestamp);

                        calculate_gpio_to_usb_time();
                        break; // stop at the first bit of motion in this report
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
    snprintf(buf, sizeof(buf), "%lu;%lu;%lu;%lu\r\n",
             xlat_get_latency_count(LATENCY_GPIO_TO_USB),
             xlat_get_latency_us(LATENCY_GPIO_TO_USB),
             xlat_get_average_latency(LATENCY_GPIO_TO_USB),
             xlat_get_latency_standard_deviation(LATENCY_GPIO_TO_USB));
    vcp_writestr(buf);
}


void xlat_parse_hid_descriptor(uint8_t *desc, size_t desc_size)
{
    HID_ReportInfo_t report_info; // Only 333b when using HID_PARSER_STREAM_ONLY

    printf("HID descriptor size: %d\n", desc_size);

    int err = USB_ProcessHIDReport(desc, desc_size, &report_info);
    printf("USB_ProcessHIDReport: %d\n", err);
    if (err != HID_PARSE_Successful) {
        return;
    }

    // Check if using reportIDs:
    hid_using_reportid = report_info.UsingReportIDs;
    printf("Using reportIDs: %d\n", hid_using_reportid);

    // Find click and motion data offsets
    check_offsets();

    // Send a message to the gfx thread, to refresh the device info
    struct gfx_event *evt;
    evt = osPoolAlloc(gfxevt_pool); // Allocate memory for the message
    evt->type = GFX_EVENT_HID_DEVICE_CONNECTED;
    evt->value = 0;
    osMessagePut(msgQGfxTask, (uint32_t)evt, 0U);
}

hid_data_location_t * xlat_get_button_location(void)
{
    return &button_location;
}

hid_data_location_t * xlat_get_x_location(void)
{
    return &x_location;
}

hid_data_location_t * xlat_get_y_location(void)
{
    return &y_location;
}

void xlat_clear_locations(void)
{
    printf("Clearing locations\n");
    button_location.found = false;
    x_location.found = false;
    y_location.found = false;
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
