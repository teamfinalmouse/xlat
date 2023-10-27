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

#include "gfx_main.h"

#include "lvgl/lvgl.h"
#include "touchpad/touchpad.h"
#include "tft/tft.h"
#include "xlat.h"
#include "gfx_settings.h"
#include "stdio_glue.h"

#define Y_CHART_SIZE_X 410
#define Y_CHART_SIZE_Y 130

lv_color_t lv_color_lightblue = LV_COLOR_MAKE(0xa6, 0xd1, 0xd1);

static lv_obj_t * chart;
static lv_obj_t * latency_label;
static lv_obj_t * productname_label;
static lv_obj_t * vidpid_label;
static lv_chart_cursor_t * chart_cursor_usb;
//static lv_chart_cursor_t * chart_cursor_avg;
static lv_obj_t * hid_byte_cb;
static lv_obj_t * trigger_label;
static lv_obj_t * trigger_ready_cb;

static size_t chart_point_count = 0;

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

void gfx_set_device_label(const char * name, const char *vidpid)
{
    lv_label_set_text(vidpid_label, vidpid);
    lv_obj_align(vidpid_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_label_set_text(productname_label, name);
    lv_obj_align_to(productname_label, vidpid_label, LV_ALIGN_OUT_LEFT_BOTTOM, -10, 0);
}

static void btn_reset_event_cb(lv_event_t * e)
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

void auto_trigger_callback(lv_timer_t * timer)
{
    /*Use the user_data*/
    uint32_t * user_data = timer->user_data;

    xlat_auto_trigger_action();

    *user_data = (*user_data) - 1;
    char label[20];
    if (*user_data) {
        sprintf(label, "%lu", *user_data);
        lv_label_set_text(trigger_label, label);    /*Set the labels text*/
        lv_obj_center(trigger_label);
    } else {
        lv_label_set_text(trigger_label, "TRIGGER"); /*Set the labels text*/
        lv_obj_center(trigger_label);
    }
}

static void btn_trigger_event_cb(lv_event_t * e)
{
    static int32_t count;

    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        // Trigger a measurement
        printf("AutoTrigger activated\n");
        count = 1000;
        lv_timer_t * timer = lv_timer_create(auto_trigger_callback, 149,  &count);
        lv_timer_set_repeat_count(timer, count);
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

    lv_chart_refresh(chart);
}

static void chart_update(uint32_t value)
{
    lv_chart_set_next_value(chart, lv_chart_get_series_next(chart, NULL), (lv_coord_t)value);

    // get the point id of the last point
    // lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);
    // uint16_t point_count = lv_chart_get_point_count(chart); // always 10
    // lv_point_t point;
    // lv_chart_get_point_pos_by_id(chart, ser, chart_point_count > point_count ? point_count - 1 : chart_point_count, &point);
    // point.x = 11 + 43*9;
    // printf("chart_update: %lu, count: %u, point_count: %u, %d,%d\n", value, chart_point_count, point_count, point.x, point.y);
    // lv_chart_set_cursor_pos(chart, chart_cursor_usb, &point);
    //lv_chart_set_cursor_point(chart, chart_cursor_avg, NULL, xlat_get_average_latency(LATENCY_GPIO_TO_USB));

    chart_point_count++;

    // refresh the chart
    lv_chart_refresh(chart);
}

/**
 * Display 1000 data points with zooming and scrolling.
 * See how the chart changes drawing mode (draw only vertical lines) when
 * the points get too crowded.
 */
void lv_chart_new(lv_coord_t xrange, lv_coord_t yrange)
{
    // Create a chart
    chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart, Y_CHART_SIZE_X, Y_CHART_SIZE_Y);
    lv_obj_align(chart, LV_ALIGN_CENTER, 20, 0);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, yrange);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_X, 0, xrange);

    // Do not display points on the data
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);

    lv_chart_add_series(chart, lv_color_lightblue, LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, 11, 2, true, 20);
    // Y-axis: major tick every 1ms
    lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5 + 1, 2, true, 60);
}

#endif

static void cb_evt_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        xlat_set_hid_byte(lv_obj_get_state(obj) & LV_STATE_CHECKED);
        if (lv_obj_get_state(obj) & LV_STATE_CHECKED) {
            lv_checkbox_set_text(obj, "HID[1]");
        } else {
            lv_checkbox_set_text(obj, "HID[0]");
        }
    }
}

void gfx_set_hid_byte(bool state)
{
    if (state) {
        lv_obj_add_state(hid_byte_cb, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(hid_byte_cb, LV_STATE_CHECKED);
    }
    lv_event_t e;
    e.code = LV_EVENT_VALUE_CHANGED;
    e.target = hid_byte_cb;
    cb_evt_handler(&e);
}

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
//        lv_obj_add_style(obj, &styles->card, 0);
//        lv_obj_add_style(obj, &styles->pad_small, 0);
//        lv_obj_add_style(obj, &styles->chart_bg, 0);
//        lv_obj_add_style(obj, &styles->scrollbar, LV_PART_SCROLLBAR);
//        lv_obj_add_style(obj, &styles->scrollbar_scrolled, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
//        lv_obj_add_style(obj, &styles->chart_series, LV_PART_ITEMS);
//        lv_obj_add_style(obj, &styles->chart_indic, LV_PART_INDICATOR);
//        lv_obj_add_style(obj, &styles->chart_ticks, LV_PART_TICKS);
//        lv_obj_add_style(obj, &styles->chart_series, LV_PART_CURSOR);
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

static void gfx_xlat_gui(void)
{
    // Rotate display
    lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_180);

    // Load theme
    new_theme_init_and_set();

    // Draw logo
    lv_obj_t * logo = lv_img_create(lv_scr_act());
    lv_img_set_src(logo, &xlat_logo);
    lv_obj_align(logo, LV_ALIGN_TOP_LEFT, 10, 10);

    ///////////////////////////
    // DEVICE INFO TOP RIGHT //
    ///////////////////////////

    // Vid/Pid label
    vidpid_label = lv_label_create(lv_scr_act());
    lv_label_set_text(vidpid_label, "0:0");
    lv_obj_align(vidpid_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    // Device label
    productname_label = lv_label_create(lv_scr_act());
    lv_label_set_text(productname_label, "No USB device connected");
    lv_obj_align_to(productname_label, vidpid_label, LV_ALIGN_OUT_LEFT_BOTTOM, -10, 0);

    // HID byte0/1 checkbox
    hid_byte_cb = lv_checkbox_create(lv_scr_act());
    lv_checkbox_set_text(hid_byte_cb, "HID[0]");
    lv_obj_align_to(hid_byte_cb, vidpid_label, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);
    lv_obj_add_event_cb(hid_byte_cb, cb_evt_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // Trigger ready label
    trigger_ready_cb = lv_checkbox_create(lv_scr_act());
    lv_checkbox_set_text(trigger_ready_cb, "READY");
    lv_obj_add_state(trigger_ready_cb, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(trigger_ready_cb, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_align_to(trigger_ready_cb, hid_byte_cb, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);

    ///////////////////////////
    // BUTTONS AT THE BOTTOM //
    ///////////////////////////

    // Clear button
    lv_obj_t * clear_btn = lv_btn_create(lv_scr_act());               /*Add a button the current screen*/
    lv_obj_align(clear_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_size(clear_btn, 80, 30);                              /*Set its size*/
    lv_obj_add_event_cb(clear_btn, btn_reset_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    // Reset button label
    lv_obj_t * reset_label = lv_label_create(clear_btn);            /*Add a label to the button*/
    lv_label_set_text(reset_label, "CLEAR");                        /*Set the labels text*/
    lv_obj_center(reset_label);

    // Reboot button
    lv_obj_t * reboot_btn = lv_btn_create(lv_scr_act());               /*Add a button the current screen*/
    lv_obj_align_to(reboot_btn, clear_btn, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_set_size(reboot_btn, 80, 30);                              /*Set its size*/
    lv_obj_add_event_cb(reboot_btn, btn_reboot_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    // Reboot button label
    lv_obj_t * reboot_label = lv_label_create(reboot_btn);            /*Add a label to the button*/
    lv_label_set_text(reboot_label, "REBOOT");                        /*Set the labels text*/
    lv_obj_center(reboot_label);

    // Settings button
    lv_obj_t * settings_btn = lv_btn_create(lv_scr_act());               /*Add a button the current screen*/
    lv_obj_align_to(settings_btn, reboot_btn, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_set_size(settings_btn, 80, 30);                              /*Set its size*/
    lv_obj_add_event_cb(settings_btn, btn_settings_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    // Settings button label
    lv_obj_t * settings_label = lv_label_create(settings_btn);            /*Add a label to the button*/
    lv_label_set_text(settings_label, "SETTINGS");                        /*Set the labels text*/
    lv_obj_center(settings_label);

    // Trigger button
    lv_obj_t * trigger_btn = lv_btn_create(lv_scr_act());               /*Add a button the current screen*/
    lv_obj_align_to(trigger_btn, settings_btn, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_set_size(trigger_btn, 80, 30);                              /*Set its size*/
    lv_obj_add_event_cb(trigger_btn, btn_trigger_event_cb, LV_EVENT_ALL, NULL); /*Assign a callback to the button*/

    // Trigger button label
    trigger_label = lv_label_create(trigger_btn);            /*Add a label to the button*/
    lv_label_set_text(trigger_label, "TRIGGER");                        /*Set the labels text*/
    lv_obj_center(trigger_label);

    // Latency label
    latency_label = lv_label_create(lv_scr_act());
    lv_label_set_text(latency_label, "Click to start measurement...");
    lv_obj_align_to(latency_label, chart, LV_ALIGN_OUT_TOP_MID, 0, 0);

    ///////////
    // CHART //
    ///////////
    lv_chart_new(1000, 5000); // 5 ms
    chart_cursor_usb = lv_chart_add_cursor(chart, lv_color_white(), LV_DIR_TOP);
    //chart_cursor_avg = lv_chart_add_cursor(chart, lv_color_lightblue, LV_DIR_HOR);
    //chart_cursor_gpio = lv_chart_add_cursor(chart, lv_color_lightblue, LV_DIR_BOTTOM);
}


// PUBLIC FUNCTIONS

void gfx_init(void)
{
    lv_init();

    tft_init();
    touchpad_init();

    gfx_xlat_gui();
}


void gfx_task(void)
{
    xSemaphoreTake(lvgl_mutex, portMAX_DELAY);
    lv_task_handler();
    xSemaphoreGive(lvgl_mutex);

    if (osMessageWaiting(msgQNewData)) {
        // pop the message
        osEvent evt = osMessageGet(msgQNewData, 0);

        // take LVGL mutex
        xSemaphoreTake(lvgl_mutex, portMAX_DELAY);

        // update chart data
        chart_update(evt.value.v);

        // update to latest xlat measurements
        latency_label_update();

        // give LVGL mutex
        xSemaphoreGive(lvgl_mutex);

        xlat_print_measurement();
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
