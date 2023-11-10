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

#ifndef XLAT_F7_FW_GFX_H
#define XLAT_F7_FW_GFX_H

#include <stdbool.h>
#include <stdint.h>

typedef enum gfx_event_type {
    GFX_EVENT_MEASUREMENT,
    GFX_EVENT_HID_DEVICE_CONNECTED,
    GFX_EVENT_HID_DEVICE_DISCONNECTED,
} gfx_event_t;

struct gfx_event {
    gfx_event_t type;
    int32_t value;
};

void gfx_init(void);
void gfx_task(void);
void gfx_set_device_label(const char * manufacturer, const char * productname, const char *vidpid);
void gfx_set_trigger_ready(bool state);
void gfx_set_offsets_text();

#endif //XLAT_F7_FW_GFX_H
