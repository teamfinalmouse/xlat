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

// Pointers to the widgets
lv_obj_t *settings_screen;
lv_dropdown_t *edge_dropdown;
lv_slider_t *debounce_dropdown;
lv_dropdown_t *trigger_dropdown;
lv_dropdown_t *detection_dropdown;
lv_obj_t *prev_screen = NULL; // Pointer to store previous screen

LV_IMG_DECLARE(xlat_logo);

// Event handler for the back button
static void back_btn_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (prev_screen) {
            lv_scr_load(prev_screen); // Switch back to the previous screen
            lv_obj_del(settings_screen);  // Delete the settings screen and free its memory
        }
    }
}

static void event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (obj == (lv_obj_t *)edge_dropdown) {
            // Detection edge changed
            uint16_t sel = lv_dropdown_get_selected(obj);
            hw_config_input_trigger(sel);
        } else if (obj == (lv_obj_t *)debounce_dropdown) {
            // Hold-off time changed
            uint16_t sel = lv_dropdown_get_selected(obj);
            uint32_t val = 100;
            switch (sel) {
                case 0:
                    // Set hold-off time to 20ms
                    val = 20;
                    break;
                case 1:
                    // Set hold-off time to 100ms
                    val = 100;
                    break;
                case 2:
                    // Set hold-off time to 200ms
                    val = 200;
                    break;
                case 3:
                    // Set hold-off time to 500ms
                    val = 500;
                    break;
                case 4:
                    // Set hold-off time to 1000ms
                    val = 1000;
                    break;
                default:
                    break;
            }
            // Set hold-off time to "value"
            xlat_set_gpio_irq_holdoff_us(val * 1000);
        }
        else if (obj == (lv_obj_t *)trigger_dropdown) {
            // Auto-trigger level changed
            uint16_t sel = lv_dropdown_get_selected(obj);
            xlat_auto_trigger_level_set(sel);
        }
        else if (obj == (lv_obj_t *)detection_dropdown) {
            // Detection mode changed
            uint16_t sel = lv_dropdown_get_selected(obj);
            if (sel == 0) {
                // Click
                xlat_set_mode(XLAT_MODE_MOUSE_CLICK);
                gfx_send_event(GFX_EVENT_MODE_CHANGED, 0); // to refresh the UI
            } else if (sel == 1) {
                // Motion
                xlat_set_mode(XLAT_MODE_MOUSE_MOTION);
                gfx_send_event(GFX_EVENT_MODE_CHANGED, 0); // to refresh the UI
            } else if (sel == 2) {
                // Key
                xlat_set_mode(XLAT_MODE_KEYBOARD);
                gfx_send_event(GFX_EVENT_MODE_CHANGED, 0); // to refresh the UI
            }
        }
        else {
            printf("Unknown event\n");
        }
    }
}

void gfx_settings_create_page(lv_obj_t *previous_screen)
{
    // Create a new screen for settings
    prev_screen = previous_screen;
    settings_screen = lv_obj_create(NULL);
    lv_scr_load(settings_screen);

    // Detection Edge Label & Dropdown
    lv_obj_t *edge_label = lv_label_create(settings_screen);
    lv_label_set_text(edge_label, "Detection Edge:");
    lv_obj_align(edge_label, LV_ALIGN_TOP_LEFT, 10, 10);

    edge_dropdown = (lv_dropdown_t *) lv_dropdown_create(settings_screen);
    lv_dropdown_set_options((lv_obj_t *) edge_dropdown, "Falling\nRising");
    // Will align this after determining max label width
    lv_obj_add_event_cb((struct _lv_obj_t *) edge_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);


    // Debounce Time Label & Slider
    lv_obj_t *debounce_label = lv_label_create(settings_screen);
    lv_label_set_text(debounce_label, "Debounce Time:");
    lv_obj_align_to(debounce_label, edge_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    debounce_dropdown = (lv_slider_t *) lv_dropdown_create(settings_screen);
    lv_dropdown_set_options((lv_obj_t *) debounce_dropdown, "20ms\n100ms\n200ms\n500ms\n1000ms");
    // Will align this after determining max label width
    lv_obj_add_event_cb((struct _lv_obj_t *) debounce_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);


    // Auto-Trigger Level Label & Dropdown
    lv_obj_t *trigger_label = lv_label_create(settings_screen);
    lv_label_set_text(trigger_label, "Auto-trigger Level:");
    lv_obj_align_to(trigger_label, debounce_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    trigger_dropdown = (lv_dropdown_t *) lv_dropdown_create(settings_screen);
    lv_dropdown_set_options((lv_obj_t *) trigger_dropdown, "Pull Low\nDrive High");
    // Will align this after determining max label width
    lv_obj_add_event_cb((struct _lv_obj_t *) trigger_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);


    // Click vs. motion detection label
    lv_obj_t *detection_mode = lv_label_create(settings_screen);
    lv_label_set_text(detection_mode, "Detection Mode:");
    lv_obj_align_to(detection_mode, trigger_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    // Click vs. motion detection dropdown
    detection_dropdown = (lv_dropdown_t *) lv_dropdown_create(settings_screen);
    lv_dropdown_set_options((lv_obj_t *) detection_dropdown, "Click\nMotion\nKey");
    lv_obj_add_event_cb((struct _lv_obj_t *) detection_dropdown, event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // If we don't add this label, the y-value of the last item will be 0
    lv_obj_t *debounce_label2 = lv_label_create(settings_screen);
    lv_label_set_text(debounce_label2, "");
    lv_obj_align_to(debounce_label2, trigger_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 30);

    // Determine max label width and align widgets accordingly
    int max_width = lv_obj_get_width(edge_label);
    max_width = LV_MAX(max_width, lv_obj_get_width(debounce_label));
    max_width = LV_MAX(max_width, lv_obj_get_width(trigger_label));

    int widget_gap = 20; // Space between the widest label and the widget

    // Now, align the widgets based on the maximum label width
    lv_obj_align((struct _lv_obj_t *) edge_dropdown, LV_ALIGN_DEFAULT, max_width + widget_gap, lv_obj_get_y(edge_label) - 10);
    lv_obj_align((struct _lv_obj_t *) debounce_dropdown, LV_ALIGN_DEFAULT, max_width + widget_gap, lv_obj_get_y(debounce_label) - 10);
    lv_obj_align((struct _lv_obj_t *) trigger_dropdown, LV_ALIGN_DEFAULT, max_width + widget_gap, lv_obj_get_y(trigger_label) - 10);
    lv_obj_align((struct _lv_obj_t *) detection_dropdown, LV_ALIGN_DEFAULT, max_width + widget_gap, lv_obj_get_y(detection_mode) - 10);

    // Print all y-values for debugging
    //printf("edge_label y: %d\n", lv_obj_get_y(edge_label));
    //printf("debounce_label y: %d\n", lv_obj_get_y(debounce_label));
    //printf("trigger_label y: %d\n", lv_obj_get_y(trigger_label));

    // Back button
    lv_obj_t *btn_back = lv_btn_create(settings_screen);
    lv_obj_set_size(btn_back, 80, 30);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_event_cb(btn_back, back_btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_t *back_label = lv_label_create(btn_back);
    lv_label_set_text(back_label, "BACK");
    lv_obj_center(back_label);

    // Version number label in the top right
    lv_obj_t *version_label = lv_label_create(settings_screen);
    // Get the version number from APP_VERSION_* defines
    char version_str[30];
    sprintf(version_str, "XLAT v%s", APP_VERSION_FULL);
    lv_label_set_text(version_label, version_str);
    lv_obj_align(version_label, LV_ALIGN_TOP_RIGHT, -10, 10);


    // Display current settings
    uint32_t debounce_time = xlat_get_gpio_irq_holdoff_us() / 1000;
    uint16_t debounce_index = 0;
    switch (debounce_time) {
        case 20:
            debounce_index = 0;
            break;
        case 100:
            debounce_index = 1;
            break;
        case 200:
            debounce_index = 2;
            break;
        case 500:
            debounce_index = 3;
            break;
        case 1000:
            debounce_index = 4;
            break;
        default:
            break;
    }
    lv_dropdown_set_selected((lv_obj_t *) debounce_dropdown, debounce_index);

    // Display current detection mode
    uint8_t current_mode = xlat_get_mode();
    lv_dropdown_set_selected((lv_obj_t *) detection_dropdown, (uint16_t)current_mode);

    // Display current detection edge
    lv_dropdown_set_selected((lv_obj_t *) edge_dropdown, hw_config_input_trigger_is_rising_edge());

    // Display current auto-trigger level
    lv_dropdown_set_selected((lv_obj_t *) trigger_dropdown, xlat_auto_trigger_level_is_high());

}

