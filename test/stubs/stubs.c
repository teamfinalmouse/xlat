#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"

SemaphoreHandle_t lvgl_mutex;
osMessageQId  msgQGfxTask;
osPoolId  gfxevt_pool;
volatile bool xlat_initialized = true;

void NVIC_SystemReset(void) {
    printf("NVIC_SystemReset\n");
}

void osDelay(uint32_t ms) {
    printf("osDelay\n");
}

void *osPoolAlloc (osPoolId pool_id) {
    printf("osPoolAlloc\n");
    return NULL;
}

osStatus osPoolFree (osPoolId pool_id, void *block) {
    printf("osPoolFree\n");
    return 0;
}

osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec) {
    printf("osMessagePut: info=%lu\n", (unsigned long)info);
    return 0;
}

osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec) {
    osEvent event = { 0, };
    printf("osMessageGet\n");
    return event;
}

uint32_t osMessageWaiting(osMessageQId queue_id) {
    printf("osMessageWaiting\n");
    return 0;
}

void xSemaphoreTake(SemaphoreHandle_t xSemaphore, int xTicksToWait) {
    printf("xSemaphoreTake\n");
}

void xSemaphoreGive(SemaphoreHandle_t xSemaphore) {
    printf("xSemaphoreGive\n");
}

uint32_t xlat_get_latency_standard_deviation(enum latency_type type) {
    printf("xlat_get_latency_standard_deviation\n");
    return 0;
}

uint32_t xlat_get_average_latency(enum latency_type type) {
    printf("xlat_get_average_latency\n");
    return 0;
}

uint32_t xlat_get_latency_us(enum latency_type type) {
    printf("xlat_get_latency_us\n");
    return 0;
}

uint32_t xlat_get_latency_count(enum latency_type type) {
    printf("xlat_get_latency_count\n");
    return 0;
}

void xlat_reset_latency(void) {
    printf("xlat_reset_latency\n");
}

void gfx_settings_create_page(lv_obj_t *previous_screen) {
    printf("gfx_settings_create_page\n");
}

void xlat_auto_trigger_turn_off_action(void) {
    printf("xlat_auto_trigger_turn_off_action\n");
}

void xlat_auto_trigger_action(void) {
    printf("xlat_auto_trigger_action\n");
}

uint32_t xlat_counter_1mhz_get(void) {
    static uint32_t counter = 0;
    return counter++;
}

uint16_t xlat_get_button_bits(void) {
    printf("xlat_get_button_bits\n");
    return 0;
}

uint16_t xlat_get_motion_bits(void) {
    printf("xlat_get_motion_bits\n");
    return 0;
}

xlat_mode_t xlat_get_mode(void) {
    printf("xlat_get_mode\n");
    return XLAT_MODE_CLICK;
}

uint16_t xlat_get_report_id(void) {
    printf("xlat_get_report_id\n");
    return 0;
}

void xlat_print_measurement(void) {
    printf("xlat_print_measurement\n");
}


// Other stubs:

void tft_init(void) {
    printf("tft_init\n");
}

void touchpad_init(void) {
    printf("touchpad_init\n");
}