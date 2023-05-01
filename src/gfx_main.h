//
// Created by vinz on 2/05/23.
//

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
