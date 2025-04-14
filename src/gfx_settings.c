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
#include "gfx_settings.h"
#include "gfx_main.h"
#include "lvgl/lvgl.h"
#include "xlat.h"
#include "hardware_config.h"

// UI layout constants
#define LABEL_WIDTH 180
#define DROPDOWN_WIDTH 180

// Pointers to the widgets
lv_obj_t *settings_screen;
lv_obj_t *prev_screen = NULL;
lv_obj_t *edge_dropdown;
lv_obj_t *bias_dropdown;
lv_obj_t *debounce_dropdown;
lv_obj_t *trigger_dropdown;
lv_obj_t *mode_dropdown;
lv_obj_t *trigger_output_dropdown;

// Event handler for the back button
static void back_btn_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (prev_screen) {
            lv_scr_load(prev_screen);
            lv_obj_del(settings_screen);
        }
    }
}

static void event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (obj == edge_dropdown) {
            uint16_t sel = lv_dropdown_get_selected(obj);
            hw_config_input_trigger_set_edge(sel);
        } else if (obj == bias_dropdown) {
            uint16_t sel = lv_dropdown_get_selected(obj);
            uint32_t bias;
            switch (sel) {
                case 0: bias = INPUT_BIAS_NOPULL; break;
                case 1: bias = INPUT_BIAS_PULLUP; break;
                case 2: bias = INPUT_BIAS_PULLDOWN; break;
                default: bias = INPUT_BIAS_NOPULL; break;
            }
            hw_config_input_bias(bias);
        } else if (obj == debounce_dropdown) {
            uint16_t sel = lv_dropdown_get_selected(obj);
            uint32_t val = 100;
            switch (sel) {
                case 0: val = 20; break;
                case 1: val = 100; break;
                case 2: val = 200; break;
                case 3: val = 500; break;
                case 4: val = 1000; break;
                default: break;
            }
            xlat_set_gpio_irq_holdoff_us(val * 1000);
        } else if (obj == trigger_dropdown) {
            uint16_t sel = lv_dropdown_get_selected(obj);
            xlat_auto_trigger_level_set(sel);
        } else if (obj == mode_dropdown) {
            uint16_t sel = lv_dropdown_get_selected(obj);
            if (sel == 0) {
                xlat_set_mode(XLAT_MODE_MOUSE_CLICK);
                gfx_send_event(GFX_EVENT_MODE_CHANGED, 0);
            } else if (sel == 1) {
                xlat_set_mode(XLAT_MODE_MOUSE_MOTION);
                gfx_send_event(GFX_EVENT_MODE_CHANGED, 0);
            } else if (sel == 2) {
                xlat_set_mode(XLAT_MODE_KEYBOARD);
                gfx_send_event(GFX_EVENT_MODE_CHANGED, 0);
            }
        }
    }
}

void gfx_settings_create_page(lv_obj_t *previous_screen)
{
    prev_screen = previous_screen;
    settings_screen = lv_obj_create(NULL);
    lv_scr_load(settings_screen);

    // Create a tabview
    lv_obj_t *tabview = lv_tabview_create(settings_screen, LV_DIR_TOP, 30);
    lv_obj_set_size(tabview, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL) - 30);
    lv_obj_align(tabview, LV_ALIGN_TOP_MID, 0, 0);

    // Create 3 tabs
    lv_obj_t *tab_mode = lv_tabview_add_tab(tabview, "Mode");
    lv_obj_t *tab_detection = lv_tabview_add_tab(tabview, "Detection");
    lv_obj_t *tab_trigger = lv_tabview_add_tab(tabview, "Trigger");

    // Mode Tab Content
    // Add explanatory text for Mode tab first
    lv_obj_t *mode_info = lv_label_create(tab_mode);
    lv_label_set_text(mode_info, "Select the detection mode.\n"
                                 "After changing the mode, you might need to re-plug the USB device.");
    lv_obj_set_style_text_align(mode_info, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(mode_info, &lv_font_montserrat_12, 0);
    lv_obj_align(mode_info, LV_ALIGN_TOP_LEFT, 10, 10);

    // Then add the mode dropdown
    lv_obj_t *mode_label = lv_label_create(tab_mode);
    lv_label_set_text(mode_label, "Detection Mode:");
    lv_obj_set_width(mode_label, LABEL_WIDTH);
    lv_obj_align_to(mode_label, mode_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    lv_obj_t *mode_dropdown = lv_dropdown_create(tab_mode);
    lv_dropdown_set_options(mode_dropdown, "Mouse: Click\nMouse: Motion\nKeyboard: Keypress");
    lv_obj_set_width(mode_dropdown, DROPDOWN_WIDTH);
    lv_obj_align_to(mode_dropdown, mode_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_event_cb(mode_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // Detection Tab Content
    // Add explanatory text for Detection tab first
    lv_obj_t *detection_info = lv_label_create(tab_detection);
    lv_label_set_text(detection_info, "Configure the input signal detection.");
    lv_obj_set_style_text_align(detection_info, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(detection_info, &lv_font_montserrat_12, 0);
    lv_obj_align(detection_info, LV_ALIGN_TOP_LEFT, 10, 10);

    // Then add the detection settings
    lv_obj_t *edge_label = lv_label_create(tab_detection);
    lv_label_set_text(edge_label, "Detection Edge:");
    lv_obj_set_width(edge_label, LABEL_WIDTH);
    lv_obj_align_to(edge_label, detection_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    lv_obj_t *edge_dropdown = lv_dropdown_create(tab_detection);
    lv_dropdown_set_options(edge_dropdown, "Falling\nRising");
    lv_obj_set_width(edge_dropdown, DROPDOWN_WIDTH);
    lv_obj_align_to(edge_dropdown, edge_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_event_cb(edge_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *debounce_label = lv_label_create(tab_detection);
    lv_label_set_text(debounce_label, "Debounce Time:");
    lv_obj_set_width(debounce_label, LABEL_WIDTH);
    lv_obj_align_to(debounce_label, edge_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    lv_obj_t *debounce_dropdown = lv_dropdown_create(tab_detection);
    lv_dropdown_set_options(debounce_dropdown, "20ms\n100ms\n200ms\n500ms\n1000ms");
    lv_obj_set_width(debounce_dropdown, DROPDOWN_WIDTH);
    lv_obj_align_to(debounce_dropdown, debounce_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_event_cb(debounce_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *bias_label = lv_label_create(tab_detection);
    lv_label_set_text(bias_label, "Input Bias:");
    lv_obj_set_width(bias_label, LABEL_WIDTH);
    lv_obj_align_to(bias_label, debounce_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    lv_obj_t *bias_dropdown = lv_dropdown_create(tab_detection);
    lv_dropdown_set_options(bias_dropdown, "No Pull\nPull Up\nPull Down");
    lv_obj_set_width(bias_dropdown, DROPDOWN_WIDTH);
    lv_obj_align_to(bias_dropdown, bias_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_event_cb(bias_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // Trigger Tab Content
    // Add explanatory text for Trigger tab first
    lv_obj_t *trigger_info = lv_label_create(tab_trigger);
    lv_label_set_text(trigger_info, "Configure automatic trigger behavior.");
    lv_obj_set_style_text_align(trigger_info, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(trigger_info, &lv_font_montserrat_12, 0);
    lv_obj_align(trigger_info, LV_ALIGN_TOP_LEFT, 10, 10);

    // Then add the trigger settings
    lv_obj_t *trigger_level_label = lv_label_create(tab_trigger);
    lv_label_set_text(trigger_level_label, "Auto-trigger Level:");
    lv_obj_set_width(trigger_level_label, LABEL_WIDTH);
    lv_obj_align_to(trigger_level_label, trigger_info, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    lv_obj_t *trigger_dropdown = lv_dropdown_create(tab_trigger);
    lv_dropdown_set_options(trigger_dropdown, "Pull Low\nDrive High");
    lv_obj_set_width(trigger_dropdown, DROPDOWN_WIDTH);
    lv_obj_align_to(trigger_dropdown, trigger_level_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_event_cb(trigger_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *trigger_output_label = lv_label_create(tab_trigger);
    lv_label_set_text(trigger_output_label, "Auto-trigger Output:");
    lv_obj_set_width(trigger_output_label, LABEL_WIDTH);
    lv_obj_align_to(trigger_output_label, trigger_level_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    lv_obj_t *trigger_output_dropdown = lv_dropdown_create(tab_trigger);
    lv_dropdown_set_options(trigger_output_dropdown, "D6\nD11");
    lv_obj_set_width(trigger_output_dropdown, DROPDOWN_WIDTH);
    lv_obj_align_to(trigger_output_dropdown, trigger_output_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    lv_obj_add_event_cb(trigger_output_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // Back button
    lv_obj_t *btn_back = lv_btn_create(settings_screen);
    lv_obj_set_size(btn_back, GFX_BTN_WIDTH, GFX_BTN_HEIGHT);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 10, -5);
    lv_obj_add_event_cb(btn_back, back_btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *back_label = lv_label_create(btn_back);
    lv_label_set_text(back_label, "BACK");
    lv_obj_center(back_label);

    // Version number label
    lv_obj_t *version_label = lv_label_create(settings_screen);
    char version_str[30];
    sprintf(version_str, "XLAT v%s", APP_VERSION_FULL);
    lv_label_set_text(version_label, version_str);
    lv_obj_align(version_label, LV_ALIGN_BOTTOM_RIGHT, -10, -10);

    // Set initial values
    lv_dropdown_set_selected(mode_dropdown, xlat_get_mode());
    lv_dropdown_set_selected(edge_dropdown, hw_config_input_trigger_is_rising_edge());
    lv_dropdown_set_selected(trigger_dropdown, xlat_auto_trigger_level_is_high());

    // Set debounce time
    uint32_t debounce_time = xlat_get_gpio_irq_holdoff_us() / 1000;
    uint16_t debounce_index = 0;
    switch (debounce_time) {
        case 20: debounce_index = 0; break;
        case 100: debounce_index = 1; break;
        case 200: debounce_index = 2; break;
        case 500: debounce_index = 3; break;
        case 1000: debounce_index = 4; break;
        default: break;
    }
    lv_dropdown_set_selected(debounce_dropdown, debounce_index);

    // Set input bias
    uint32_t current_bias = hw_config_input_bias_get();
    uint16_t bias_index = 0;
    switch (current_bias) {
        case INPUT_BIAS_NOPULL: bias_index = 0; break;
        case INPUT_BIAS_PULLUP: bias_index = 1; break;
        case INPUT_BIAS_PULLDOWN: bias_index = 2; break;
        default: bias_index = 0; break;
    }
    lv_dropdown_set_selected(bias_dropdown, bias_index);
}

