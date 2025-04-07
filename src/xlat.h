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

#ifndef XLAT_H
#define XLAT_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define AUTO_TRIGGER_PERIOD_MS (150)
#define AUTO_TRIGGER_PRESSED_PERIOD_MS (30)

typedef struct hid_event {
    uint32_t timestamp;
    uint8_t report[64];
    size_t report_size;
    uint8_t itf_protocol;
} hid_event_t;

typedef enum latency_type {
    LATENCY_GPIO_TO_USB = 0,
    LATENCY_AUDIO_TO_USB,
    LATENCY_TYPE_MAX,
} latency_type_t;


typedef enum xlat_mode {
    XLAT_MODE_CLICK = 0,
    XLAT_MODE_MOTION,
    XLAT_MODE_KEY,
} xlat_mode_t;

extern volatile bool xlat_initialized;

void xlat_init(void);
void xlat_task(void const * argument);
void xlat_process_usb_hid_event(void);
void xlat_usb_event_callback(uint32_t timestamp, uint8_t const *report, size_t report_size, uint8_t itf_protocol); // called from USB Host library

uint32_t xlat_get_latency_us(enum latency_type type);
uint32_t xlat_get_average_latency(enum latency_type type);
uint32_t xlat_get_latency_count(enum latency_type type);
uint32_t xlat_get_latency_variance(enum latency_type type);
uint32_t xlat_get_latency_standard_deviation(enum latency_type type);

void xlat_reset_latency(void);
void xlat_add_latency_measurement(uint32_t latency_us, enum latency_type type);
void xlat_print_measurement(void);

void xlat_set_gpio_irq_holdoff_us(uint32_t us);
uint32_t xlat_get_gpio_irq_holdoff_us(void);

uint32_t xlat_counter_1mhz_get(void);

uint32_t xlat_get_last_usb_timestamp_us(void);
uint32_t xlat_get_last_button_timestamp_us(void);

void xlat_set_using_reportid(bool use_reportid);
bool xlat_get_using_reportid(void);

void xlat_parse_hid_descriptor(uint8_t *desc, size_t desc_size); // on device connect
void xlat_clear_device_info(void); // on device disconnect

void xlat_set_mode(enum xlat_mode mode);
enum xlat_mode xlat_get_mode(void);

uint16_t xlat_get_button_bits(void);
uint16_t xlat_get_motion_bits(void);
uint16_t xlat_get_report_id(void);
void xlat_clear_locations(void);

void xlat_auto_trigger_action(void);
void xlat_auto_trigger_turn_off_action(void);
void xlat_auto_trigger_level_set(bool high);
bool xlat_auto_trigger_level_is_high(void);

#endif //XLAT_H
