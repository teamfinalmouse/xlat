//
// Created by vinz on 1/05/23.
//

#ifndef XLAT_H
#define XLAT_H

#include <stdbool.h>
#include <stdint.h>
#include "src/usb/usbh_def.h"

struct hid_event {
    USBH_HandleTypeDef *phost;
    uint32_t timestamp;
};

typedef enum latency_type {
    LATENCY_GPIO_TO_USB = 0,
    LATENCY_AUDIO_TO_USB,
    LATENCY_TYPE_MAX,
} latency_type_t;


typedef enum xlat_mode {
    XLAT_MODE_CLICK,
    XLAT_MODE_MOTION,
} xlat_mode_t;

extern volatile bool xlat_initialized;

void xlat_init(void);
void xlat_usb_hid_event(void);

uint32_t xlat_get_latency_us(enum latency_type type);
uint32_t xlat_get_average_latency(enum latency_type type);
uint32_t xlat_get_latency_count(enum latency_type type);
void xlat_reset_latency(void);
void xlat_add_latency_measurement(uint32_t latency_us, enum latency_type type);

void xlat_set_gpio_irq_holdoff_us(uint32_t us);
uint32_t xlat_get_gpio_irq_holdoff_us(void);

uint32_t xlat_counter_1mhz_get(void);

uint32_t xlat_get_last_usb_timestamp_us(void);
uint32_t xlat_get_last_button_timestamp_us(void);


void xlat_set_hid_byte(bool use_byte_1);
bool xlat_get_hid_byte(void);

void xlat_set_mode(enum xlat_mode mode);
enum xlat_mode xlat_get_mode(void);

void xlat_auto_trigger_action(void);
void xlat_auto_trigger_level_set(bool high);
bool xlat_auto_trigger_level_is_high(void);

#endif //XLAT_H
