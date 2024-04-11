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
#include "src/usb/usbh_def.h"

#define AUTO_TRIGGER_PERIOD_MS (150)

typedef struct hid_event {
    USBH_HandleTypeDef *phost;
    uint32_t timestamp;
} hid_event_t;

typedef struct hid_data_location {
    bool found;
    size_t bit_index;
    size_t bit_size;
    size_t byte_offset;
} hid_data_location_t;

typedef enum latency_type {
    LATENCY_GPIO_TO_USB = 0,
    LATENCY_AUDIO_TO_USB,
    LATENCY_TYPE_MAX,
} latency_type_t;


typedef enum xlat_mode {
    XLAT_MODE_CLICK,
    XLAT_MODE_MOTION,
    XLAT_MODE_KEY,
} xlat_mode_t;

typedef enum xlat_interface {
    XLAT_INTERFACE_AUTO = -1,
    XLAT_INTERFACE_0    = 0,
    XLAT_INTERFACE_1,
    XLAT_INTERFACE_2,
    XLAT_INTERFACE_3,
    XLAT_INTERFACE_4,
    XLAT_INTERFACE_5,
    XLAT_INTERFACE_6,
    XLAT_INTERFACE_7,
    XLAT_INTERFACE_8,
} xlat_interface_t;

extern volatile bool xlat_initialized;

void xlat_init(void);
void xlat_usb_hid_event(void);

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

void xlat_parse_hid_descriptor(uint8_t *desc, size_t desc_size);

void xlat_set_mode(enum xlat_mode mode);
enum xlat_mode xlat_get_mode(void);

hid_data_location_t * xlat_get_button_location(void);
hid_data_location_t * xlat_get_x_location(void);
hid_data_location_t * xlat_get_y_location(void);
hid_data_location_t * xlat_get_key_location(void);
void xlat_clear_locations(void);

void xlat_auto_trigger_action(void);
void xlat_auto_trigger_level_set(bool high);
bool xlat_auto_trigger_level_is_high(void);

void xlat_set_interface_selection(xlat_interface_t number);
xlat_interface_t xlat_get_interface_selection();

void xlat_set_found_interface(uint8_t number);
uint8_t xlat_get_found_interface();

#endif //XLAT_H
