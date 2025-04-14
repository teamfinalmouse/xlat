#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"

SemaphoreHandle_t lvgl_mutex;
osMessageQId  msgQGfxTask;
osPoolId  gfxevt_pool;
volatile bool xlat_initialized = true;
enum xlat_mode xlat_mode;

void NVIC_SystemReset(void) {
    printf("[stub] NVIC_SystemReset\n");
}

void osDelay(uint32_t ms) {
    printf("[stub] osDelay\n");
}

void *osPoolAlloc (osPoolId pool_id) {
    printf("[stub] osPoolAlloc (100 bytes)\n");
    return malloc(100);
}

osStatus osPoolFree (osPoolId pool_id, void *block) {
    printf("[stub] osPoolFree\n");
    return 0;
}

osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec) {
    printf("[stub] osMessagePut: info=%lu\n", (unsigned long)info);
    return 0;
}

osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec) {
    osEvent event = { 0, };
    printf("[stub] osMessageGet\n");
    return event;
}

uint32_t osMessageWaiting(osMessageQId queue_id) {
    printf("[stub] osMessageWaiting\n");
    return 0;
}

void xSemaphoreTake(SemaphoreHandle_t xSemaphore, int xTicksToWait) {
    printf("[stub] xSemaphoreTake\n");
}

void xSemaphoreGive(SemaphoreHandle_t xSemaphore) {
    printf("[stub] xSemaphoreGive\n");
}

uint32_t xlat_get_latency_standard_deviation(enum latency_type type) {
    printf("[stub] xlat_get_latency_standard_deviation\n");
    return 0;
}

uint32_t xlat_get_average_latency(enum latency_type type) {
    printf("[stub] xlat_get_average_latency\n");
    return 0;
}

uint32_t xlat_get_latency_us(enum latency_type type) {
    printf("[stub] xlat_get_latency_us\n");
    return 0;
}

uint32_t xlat_get_latency_count(enum latency_type type) {
    printf("[stub] xlat_get_latency_count\n");
    return 0;
}

void xlat_reset_latency(void) {
    printf("[stub] xlat_reset_latency\n");
}

void xlat_auto_trigger_turn_off_action(void) {
    printf("[stub] xlat_auto_trigger_turn_off_action\n");
}

void xlat_auto_trigger_action(void) {
    printf("[stub] xlat_auto_trigger_action\n");
}

uint32_t xlat_counter_1mhz_get(void) {
    static uint32_t counter = 0;
    return counter++;
}

uint16_t xlat_get_button_bits(void) {
    printf("[stub] xlat_get_button_bits\n");
    return 0;
}

uint16_t xlat_get_motion_bits(void) {
    printf("[stub] xlat_get_motion_bits\n");
    return 0;
}

void xlat_set_mode(enum xlat_mode mode)
{
    xlat_mode = mode;
}

enum xlat_mode xlat_get_mode(void)
{
    return xlat_mode;
}

uint16_t xlat_get_report_id(void) {
    printf("[stub] xlat_get_report_id\n");
    return 0;
}

void xlat_print_measurement(void) {
    printf("[stub] xlat_print_measurement\n");
}


// Other stubs:

void tft_init(void) {
    printf("[stub] tft_init\n");
}

void touchpad_init(void) {
    printf("[stub] touchpad_init\n");
}

// Stubs for gfx_settings.c:
void hw_config_input_trigger_set_edge(bool rising) {
    printf("[stub] hw_config_input_trigger_set_edge: rising=%d\n", rising);
}

void hw_config_input_bias(bool enable) {
    printf("[stub] hw_config_input_bias: enable=%d\n", enable);
}

void xlat_set_gpio_irq_holdoff_us(uint32_t us) {
    printf("[stub] xlat_set_gpio_irq_holdoff_us: us=%lu\n", (unsigned long)us);
}

void xlat_auto_trigger_level_set(bool high) {
    printf("[stub] xlat_auto_trigger_level_set: high=%d\n", high);
}

uint32_t xlat_get_gpio_irq_holdoff_us(void) {
    printf("[stub] xlat_get_gpio_irq_holdoff_us\n");
    return 0;
}

bool hw_config_input_trigger_is_rising_edge(void) {
    printf("[stub] hw_config_input_trigger_is_rising_edge\n");
    return false;
}

bool xlat_auto_trigger_level_is_high(void) {
    printf("[stub] xlat_auto_trigger_level_is_high\n");
    return false;
}

bool hw_config_input_bias_get(void) {
    printf("[stub] hw_config_input_bias_get\n");
    return false;
}
