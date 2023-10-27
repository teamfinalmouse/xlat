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

void gfx_init(void);
void gfx_task(void);
void gfx_set_hid_byte(bool state);
void gfx_set_device_label(const char * name, const char *vidpid);
void gfx_set_trigger_ready(bool state);

#endif //XLAT_F7_FW_GFX_H
