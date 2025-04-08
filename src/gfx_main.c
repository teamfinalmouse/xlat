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

#include <stdio.h>
#include <stdlib.h>

#include "gfx_main.h"

#include <main.h>
#include <usb_task.h>

#include "cmsis_os.h"
#include "lvgl/lvgl.h"
#include "touchpad/touchpad.h"
#include "tft/tft.h"
#include "xlat.h"
#include "gfx_settings.h"

#define Y_CHART_SIZE_X 410
#define Y_CHART_SIZE_Y 130

#define Y_CHART_RANGE 2000

#define X_CHART_TICKS_MAJOR 11
#define X_CHART_TICKS_MINOR 1
#define Y_CHART_TICKS_MAJOR 6
#define Y_CHART_TICKS_MINOR 2

lv_color_t lv_color_lightblue = LV_COLOR_MAKE(0xa6, 0xd1, 0xd1);

static lv_obj_t * chart;
static lv_obj_t * latency_label;
static lv_obj_t * productname_label;
static lv_obj_t * manufacturer_label;
static lv_obj_t * vidpid_label;
static lv_obj_t * hid_data_locations_label;
static lv_obj_t * mode_label;
static lv_obj_t * trigger_label;
static lv_obj_t * trigger_ready_cb;

static size_t chart_point_count = 0;
static lv_coord_t chart_y_range = 0;

static lv_timer_t * trigger_timer = NULL;
static lv_timer_t * trigger_timer_turn_off = NULL;

static void chart_reset(void);

LV_IMG_DECLARE(xlat_logo);

static void latency_label_update(void)
{
    lv_label_set_text_fmt(latency_label, "#%lu: %ldus, avg %ldus, stdev %ldus",
                          xlat_get_latency_count(LATENCY_GPIO_TO_USB),
                          xlat_get_latency_us(LATENCY_GPIO_TO_USB),
                          xlat_get_average_latency(LATENCY_GPIO_TO_USB),
                          xlat_get_latency_standard_deviation(LATENCY_GPIO_TO_USB)
                          );
    lv_obj_align_to(latency_label, chart, LV_ALIGN_OUT_TOP_MID, 0, 0);
}

void gfx_set_device_label(const char * manufacturer, const char * productname, const char *vidpid)
{
    char tempstr[21];

    // Truncate product name to 20 characters
    strncpy(tempstr, productname, 20);
    tempstr[20] = '\0';
    lv_label_set_text(productname_label, tempstr);
    lv_obj_align(productname_label, LV_ALIGN_TOP_RIGHT, -5, 5);

    // Truncate manufacturer name to 20 characters
    strncpy(tempstr, manufacturer, 20);
    tempstr[20] = '\0';
    lv_label_set_text(manufacturer_label, tempstr);
    lv_obj_align_to(manufacturer_label, productname_label, LV_ALIGN_OUT_LEFT_BOTTOM, -5, 0);

    sprintf(tempstr, "USB ID: %s", vidpid);
    lv_label_set_text(vidpid_label, tempstr);
    lv_obj_set_size(vidpid_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align_to(vidpid_label, productname_label, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 22);
}

static void btn_clear_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        // reset latency numbers
        xlat_reset_latency();
        chart_reset();
        latency_label_update();
    }
}

static void btn_reboot_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        NVIC_SystemReset();
    }
}

static void btn_settings_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        // Create a new screen for settings
        gfx_settings_create_page(lv_scr_act());
    }
}

static void auto_trigger_clear_timer(void)
{
    // Reset the trigger label
    lv_label_set_text(trigger_label, "TRIGGER");
    lv_obj_center(trigger_label);

    // Stop the timer
    lv_timer_del(trigger_timer);
    trigger_timer = NULL;
}

void auto_trigger_turn_off_callback(lv_timer_t * timer)
{
    (void)timer;
    xlat_auto_trigger_turn_off_action();
}

void auto_trigger_callback(lv_timer_t * timer)
{
    char label[20];
    size_t * count = timer->user_data;

    xlat_auto_trigger_action();
    trigger_timer_turn_off = lv_timer_create(auto_trigger_turn_off_callback, AUTO_TRIGGER_PRESSED_PERIOD_MS, NULL);
    lv_timer_set_repeat_count(trigger_timer_turn_off, 1);

    *count = (*count) - 1;
    if (*count) {
        sprintf(label, "%lu", (long)*count);
        lv_label_set_text(trigger_label, label);    /*Set the labels text*/
        lv_obj_center(trigger_label);

        // Restart the timer with a random period added to the base period
        lv_timer_set_period(timer, AUTO_TRIGGER_PERIOD_MS + (rand() % 10));
    } else {
        auto_trigger_clear_timer();
    }
}

static void btn_trigger_event_cb(lv_event_t * e)
{
    static int32_t count;

    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (count) {
            // Already running
            count = 0;
            auto_trigger_clear_timer();
        } else {
            // Trigger a new series of measurements
            printf("AutoTrigger activated\n");
            count = 1000;
            // seed the random number generator
            srand(xlat_counter_1mhz_get());
            // start the timer
            trigger_timer = lv_timer_create(auto_trigger_callback, AUTO_TRIGGER_PERIOD_MS,  &count);
            //lv_timer_set_repeat_count(timer, count);
        }
    }
}

#if LV_USE_CHART && LV_USE_SLIDER

static void chart_reset(void)
{
    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);
    lv_chart_remove_series(chart, ser);
    lv_chart_add_series(chart, lv_color_lightblue, LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_x_start_point(chart, ser, 0);

    chart_point_count = 0;
    chart_y_range = Y_CHART_RANGE;
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, chart_y_range);

    lv_chart_refresh(chart);
}

static void chart_update(uint32_t value)
{
    chart_point_count++;

    // clip to the nearest rounded down 1000
#if LV_USE_LARGE_COORD
    value = value > (INT32_MAX / 1000 * 1000) ? (INT32_MAX / 1000 * 1000) : value;
#else
    value = value > (INT16_MAX / 1000 * 1000) ? (INT16_MAX / 1000 * 1000) : value;
#endif

    lv_chart_set_next_value(chart, lv_chart_get_series_next(chart, NULL), (lv_coord_t)value);

    // can't overflow because we clipped down to the nearest 1000 within signed while value is unsigned
    value = (value + 999) / 1000 * 1000; // round up to nearest 1000

    // update y-axis range if needed
    if (value > chart_y_range) {
        chart_y_range = (lv_coord_t)value;
    }

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, chart_y_range);

    // refresh the chart
    lv_chart_refresh(chart);
}

/**
 * Display 1000 data points with zooming and scrolling.
 * See how the chart changes drawing mode (draw only vertical lines) when
 * the points get too crowded.
 */
void lv_chart_new(lv_coord_t yrange)
{
    // Create a chart
    chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, Y_CHART_SIZE_X, Y_CHART_SIZE_Y);
    lv_obj_align(chart, LV_ALIGN_CENTER, 20, 10);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, yrange);

    // Do not display points on the data
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);

    lv_chart_add_series(chart, lv_color_lightblue, LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, X_CHART_TICKS_MAJOR, X_CHART_TICKS_MINOR, true, 20);
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, Y_CHART_TICKS_MAJOR, Y_CHART_TICKS_MINOR, true, 60);

    // Set the amount of data points according to the X axis tick count
    lv_chart_set_point_count(chart, (X_CHART_TICKS_MAJOR - 1) * X_CHART_TICKS_MINOR + 1);

    chart_y_range = yrange;
}

#endif

static lv_style_t style_btn;
static lv_style_t style_chart;

/*Will be called when the styles of the base theme are already added
  to add new styles*/
static void new_theme_apply_cb(lv_theme_t * th, lv_obj_t * obj)
{
    LV_UNUSED(th);

    if (lv_obj_check_type(obj, &lv_btn_class)) {
        lv_obj_add_style(obj, &style_btn, 0);
    }
#if LV_USE_CHART
    else if(lv_obj_check_type(obj, &lv_chart_class)) {
        lv_obj_add_style(obj, &style_chart, 0);
    }
#endif
}

static void new_theme_init_and_set(void)
{
    // Initialize button style
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_white());
    lv_style_set_text_color(&style_btn, lv_color_black());
    lv_style_set_border_color(&style_btn, lv_color_lightblue);
    lv_style_set_border_width(&style_btn, 1);

    // Initialize chart style
    lv_style_init(&style_chart);
    lv_style_set_bg_color(&style_chart, lv_color_black());
    lv_style_set_border_color(&style_chart, lv_color_lightblue);
    lv_style_set_border_width(&style_chart, 1);
    lv_style_set_text_color(&style_chart, lv_color_white());

    /*Initialize the new theme from the current theme*/
    lv_theme_t * th_act = lv_disp_get_theme(NULL);
    static lv_theme_t th_new;
    th_new = *th_act;

    /*Set the parent theme and the style apply callback for the new theme*/
    lv_theme_set_parent(&th_new, th_act);
    lv_theme_set_apply_cb(&th_new, new_theme_apply_cb);

    /*Assign the new theme to the current display*/
    lv_disp_set_theme(NULL, &th_new);
}

void gfx_set_data_locations_label(void)
{
    char text[100];
    uint16_t button_bits = xlat_get_button_bits();
    uint16_t motion_bits = xlat_get_motion_bits();

    if (button_bits || motion_bits) {
        sprintf(text, "Data (%u): %u button, %u motion bits", xlat_get_report_id(), button_bits, motion_bits);
    } else {
        // offsets not found
        sprintf(text, "Data: locations not found");
    }

    lv_checkbox_set_text(hid_data_locations_label, text);
    lv_obj_align_to(hid_data_locations_label, productname_label, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 3);
}

void gfx_set_mode_label(void)
{
    char text[100];
    const char *mode_str;
    switch (xlat_get_mode()) {
        case XLAT_MODE_MOUSE_CLICK:
            mode_str = "CLICK";
            break;
        case XLAT_MODE_MOUSE_MOTION:
            mode_str = "MOTION";
            break;
        case XLAT_MODE_KEYBOARD:
            mode_str = "KEY";
            break;
        default:
            mode_str = "Unknown";
    }

    sprintf(text, "MODE: %s -", mode_str);
    lv_label_set_text(mode_label, text);
    lv_obj_align_to(mode_label, vidpid_label, LV_ALIGN_OUT_LEFT_BOTTOM, -4, 0);
}

void gfx_update_labels(void)
{
    gfx_set_device_label(usb_host_get_manuf_string(),
                         usb_host_get_product_string(),
                         usb_host_get_vidpid_string());
    gfx_set_data_locations_label();
    gfx_set_mode_label();
}

void gfx_xlat_gui(void)
{
    // Rotate display
    lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_180);

    // Load theme
    new_theme_init_and_set();

    // Draw logo
    lv_obj_t * logo = lv_img_create(lv_scr_act());
    lv_img_set_src(logo, &xlat_logo);
    lv_obj_align(logo, LV_ALIGN_TOP_LEFT, 12, 3);

    ///////////////////////////
    // DEVICE INFO TOP RIGHT //
    ///////////////////////////

    // Vid/Pid label, device name label, manufacturer label
    vidpid_label = lv_label_create(lv_scr_act());
    productname_label = lv_label_create(lv_scr_act());
    manufacturer_label = lv_label_create(lv_scr_act());
    gfx_set_device_label("", "No USB device found", "");

    // HID data locations label
    hid_data_locations_label = lv_label_create(lv_scr_act());
    gfx_set_data_locations_label();

    // Mode label
    mode_label = lv_label_create(lv_scr_act());
    gfx_set_mode_label();

    ///////////////////////////
    // BUTTONS AT THE BOTTOM //
    ///////////////////////////

    // Clear button
    lv_obj_t * clear_btn = lv_btn_create(lv_scr_act());
    lv_obj_align(clear_btn, LV_ALIGN_BOTTOM_LEFT, 10, -5);
    lv_obj_set_size(clear_btn, GFX_BTN_WIDTH, GFX_BTN_HEIGHT);
    lv_obj_add_event_cb(clear_btn, btn_clear_event_cb, LV_EVENT_CLICKED, NULL);

    // Reset button label
    lv_obj_t * reset_label = lv_label_create(clear_btn);
    lv_label_set_text(reset_label, "CLEAR");
    lv_obj_center(reset_label);

    // Reboot button
    lv_obj_t * reboot_btn = lv_btn_create(lv_scr_act());
    lv_obj_align_to(reboot_btn, clear_btn, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_set_size(reboot_btn, GFX_BTN_WIDTH, GFX_BTN_HEIGHT);
    lv_obj_add_event_cb(reboot_btn, btn_reboot_event_cb, LV_EVENT_CLICKED, NULL);

    // Reboot button label
    lv_obj_t * reboot_label = lv_label_create(reboot_btn);
    lv_label_set_text(reboot_label, "REBOOT");
    lv_obj_center(reboot_label);

    // Settings button
    lv_obj_t * settings_btn = lv_btn_create(lv_scr_act());
    lv_obj_align_to(settings_btn, reboot_btn, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_set_size(settings_btn, GFX_BTN_WIDTH, GFX_BTN_HEIGHT);
    lv_obj_add_event_cb(settings_btn, btn_settings_event_cb, LV_EVENT_CLICKED, NULL);

    // Settings button label
    lv_obj_t * settings_label = lv_label_create(settings_btn);
    lv_label_set_text(settings_label, "SETTINGS");
    lv_obj_center(settings_label);

    // Trigger button
    lv_obj_t * trigger_btn = lv_btn_create(lv_scr_act());
    lv_obj_align_to(trigger_btn, settings_btn, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_set_size(trigger_btn, GFX_BTN_WIDTH, GFX_BTN_HEIGHT);
    lv_obj_add_event_cb(trigger_btn, btn_trigger_event_cb, LV_EVENT_CLICKED, NULL);

    // Trigger button label
    trigger_label = lv_label_create(trigger_btn);
    lv_label_set_text(trigger_label, "TRIGGER");
    lv_obj_center(trigger_label);

    // Latency label
    latency_label = lv_label_create(lv_scr_act());
    lv_label_set_text(latency_label, "Click to start measurement...");
    lv_obj_align_to(latency_label, chart, LV_ALIGN_OUT_TOP_MID, 0, 0);

    // Trigger ready label
    trigger_ready_cb = lv_checkbox_create(lv_scr_act());
    lv_checkbox_set_text(trigger_ready_cb, "READY");
    lv_obj_add_state(trigger_ready_cb, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(trigger_ready_cb, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_align(trigger_ready_cb, LV_ALIGN_BOTTOM_RIGHT, -10, -7);


    ///////////
    // CHART //
    ///////////
    lv_chart_new(Y_CHART_RANGE);
    lv_chart_add_cursor(chart, lv_color_white(), LV_DIR_TOP);
}


// PUBLIC FUNCTIONS

void gfx_init(void)
{
    lv_init();

    tft_init();
    touchpad_init();

    gfx_xlat_gui();
}


void gfx_task(void const * argument)
{
    (void)argument;

    while (!xlat_initialized) {
        osDelay(1);
    }

    while (1) {
        xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
        lv_task_handler();
        xSemaphoreGive(lvgl_mutex);

        if (osMessageWaiting(msgQGfxTask))
        {
            // pop the message
            osEvent evt = osMessageGet(msgQGfxTask, 0);
            if (evt.status != osEventMessage)
            {
                return;
            }

            struct gfx_event* g_evt = evt.value.p;
            switch (g_evt->type)
            {
            case GFX_EVENT_MEASUREMENT:
                // New measurement received

                // guard with LVGL mutex
                xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
                {
                    // update chart data
                    chart_update(g_evt->value);

                    // update to latest xlat measurements
                    latency_label_update();
                }
                xSemaphoreGive(lvgl_mutex);

                xlat_print_measurement();
                break;

            case GFX_EVENT_DEVICE_CONNECTED:
                gfx_update_labels();
                break;

            case GFX_EVENT_MODE_CHANGED:
                gfx_set_mode_label();
                break;

            case GFX_EVENT_DEVICE_DISCONNECTED:
                gfx_set_data_locations_label();
                gfx_set_device_label("", "No USB device found", "");
                gfx_set_mode_label();
                break;
            }

            // free event memory
            osPoolFree(gfxevt_pool, g_evt);
        }

        osDelay(1);
    }
}

void HAL_SYSTICK_Callback(void)
{
    lv_tick_inc(1);
}

void gfx_set_trigger_ready(bool state)
{
    if (state) {
        lv_obj_add_state(trigger_ready_cb, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(trigger_ready_cb, LV_STATE_CHECKED);
    }
}

void gfx_send_event(gfx_event_t type, int32_t value)
{
    struct gfx_event *evt;
    evt = osPoolAlloc(gfxevt_pool); // Allocate memory for the message
    evt->type = type;
    evt->value = value;
    osMessagePut(msgQGfxTask, (uint32_t)evt, 0U);
}
