#pragma once

// This defines a lot of stubs to make gfx_main.c compile
// It is used to test the GUI without the rest of the application

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lvgl/lvgl.h"
#include "../src/xlat.h"  // Include the original header for enums

// Defines
#define osEventMessage 0x10
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL

// Types
typedef void *SemaphoreHandle_t;
typedef int osMessageQId;
typedef int osStatus;
typedef void *osPoolId;
typedef uint32_t TickType_t;

typedef struct  {
  int                 status;     ///< status code: event or error information
  union  {
    uint32_t                    v;     ///< message as 32-bit value
    void                       *p;     ///< message or mail as void pointer
    int32_t               signals;     ///< signal flags
  } value;                             ///< event value
  union  {
    int             mail_id;     ///< mail id obtained by \ref osMailCreate
    int       message_id;     ///< message id obtained by \ref osMessageCreate
  } def;                               ///< event definition
} osEvent;


// Global variables
extern SemaphoreHandle_t lvgl_mutex;
extern osMessageQId  msgQGfxTask;
extern osPoolId  gfxevt_pool;
extern volatile bool xlat_initialized;
extern const lv_img_dsc_t xlat_logo;

// OS stubs
void NVIC_SystemReset(void);
void osDelay(uint32_t ms);
void *osPoolAlloc (osPoolId pool_id);
osStatus osPoolFree (osPoolId pool_id, void *block);
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec);
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec);
uint32_t osMessageWaiting(osMessageQId queue_id);
void xSemaphoreTake(SemaphoreHandle_t xSemaphore, int xTicksToWait);
void xSemaphoreGive(SemaphoreHandle_t xSemaphore);

// XLAT function stubs
uint32_t xlat_latency_standard_deviation_get(enum latency_type type);
uint32_t xlat_latency_average_get(enum latency_type type);
uint32_t xlat_last_latency_us_get(enum latency_type type);
uint32_t xlat_latency_count_get(enum latency_type type);
void xlat_latency_reset(void);
void gfx_settings_create_page(lv_obj_t *previous_screen);
void xlat_auto_trigger_turn_off_action(void);
void xlat_auto_trigger_action(void);
uint32_t xlat_counter_1mhz_get(void);
void xlat_print_measurement(void);

// USB stubs
const char* usb_host_get_manuf_string(void);
const char* usb_host_get_product_string(void);
const char* usb_host_get_serial_string(void);
const char * usb_host_get_vidpid_string(void);


// Other stugbs
void tft_init(void);
void touchpad_init(void);
