/*
 * Copyright (c) 2025 Finalmouse, LLC
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

#include "xlat.h"
#include "xlat_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Configuration state
static enum xlat_mode current_mode = XLAT_MODE_MOUSE_CLICK;
static bool auto_trigger_level_high = true;
static uint32_t auto_trigger_interval_ms = 300;
static uint8_t auto_trigger_output_pin = 6;

uint8_t button_mask[REPORT_LEN];
uint8_t motion_mask[REPORT_LEN];
uint16_t button_bits;
uint16_t motion_bits;
uint8_t report_id;
bool keyboard_usage_page_found = false;

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

// Mode configuration
void xlat_mode_set(enum xlat_mode mode)
{
    current_mode = mode;
}

enum xlat_mode xlat_mode_get(void)
{
    return current_mode;
}

// Auto-trigger level configuration
void xlat_auto_trigger_level_set(bool high)
{
    auto_trigger_level_high = high;
}

bool xlat_auto_trigger_level_is_high(void)
{
    return auto_trigger_level_high;
}

// Auto-trigger interval configuration
void xlat_auto_trigger_interval_ms_set(uint32_t ms)
{
    if (ms < 100) {
        ms = 100;
    } else if (ms > 1000) {
        ms = 1000;
    }
    auto_trigger_interval_ms = ms;
}

uint32_t xlat_auto_trigger_interval_ms_get(void)
{
    return auto_trigger_interval_ms;
}

// Auto-trigger output configuration
void xlat_auto_trigger_output_set(uint8_t pin)
{
    if (pin != 6 && pin != 11) {
        pin = 6; // Default to pin 6 if invalid
    }
    auto_trigger_output_pin = pin;
}

uint8_t xlat_auto_trigger_output_get(void)
{
    return auto_trigger_output_pin;
} 


void xlat_gpio_irq_holdoff_us_set(uint32_t us)
{
    gpio_irq_holdoff_us = us;
}


uint32_t xlat_gpio_irq_holdoff_us_get(void)
{
    return gpio_irq_holdoff_us;
}

uint16_t * xlat_button_bits_get(void)
{
    return &button_bits;
}

uint16_t * xlat_motion_bits_get(void)
{
    return &motion_bits;
}

uint8_t * xlat_button_mask_get(void)
{
    return button_mask;
}

uint8_t * xlat_motion_mask_get(void)
{
    return motion_mask;
}
uint8_t xlat_report_id_get(void)
{
    return report_id;
}

void xlat_report_id_set(uint8_t id)
{
    report_id = id;
}

bool xlat_keyboard_usage_page_found_get(void)
{
    return keyboard_usage_page_found;
}

void xlat_keyboard_usage_page_found_set(bool found)
{
    keyboard_usage_page_found = found;
}