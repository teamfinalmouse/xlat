#include <unistd.h>
#define SDL_MAIN_HANDLED        /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "lvgl.h"
#include "../src/gfx_main.h"
#include "stubs/main.h"

#include "lv_drivers/sdl/sdl.h"

#if LV_USE_LOG != 0
static void lv_log_print_g_cb(const char * buf)
{
    printf("%s", buf);
}
#endif

void lv_app_init(void)
{
    static lv_color_t buf1[480 * 10];
    static lv_color_t buf2[480 * 10];
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, 480 * 10);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = sdl_display_flush;
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 272;
    lv_disp_drv_register(&disp_drv);

    // Handle mouse input
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_mouse_read;
    printf("sdl_mouse_read: %p\n", sdl_mouse_read);
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv);
    printf("mouse_indev: %p\n", mouse_indev);
}

int main()
{
    /* initialize lvgl */
    lv_init();

    /* Register the log print callback */
    #if LV_USE_LOG != 0
    lv_log_register_print_cb(lv_log_print_g_cb);
    #endif

    sdl_init();

    /* create Widgets on the screen */
    lv_app_init();
    gfx_xlat_gui();
    /* undo display rotation */
    lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_NONE);

    /* pretent a device is connected */
    gfx_set_device_label(usb_host_get_manuf_string(),
                         usb_host_get_product_string(),
                         usb_host_get_vidpid_string());
    gfx_set_data_locations_label();
    gfx_set_mode_label();

    Uint32 lastTick = SDL_GetTicks();
    while(1) {
        SDL_Delay(5);

        Uint32 current = SDL_GetTicks();
        lv_tick_inc(current - lastTick);
        lastTick = current;
        lv_timer_handler();
    }

    return 0;
}

// stubs
const char* usb_host_get_manuf_string(void) {
    return "Manufacturer Name";
}

const char* usb_host_get_product_string(void) {
    return "Product Name";
}

const char* usb_host_get_serial_string(void) {
    return "serial";
}

const char * usb_host_get_vidpid_string(void)
{
    return "1234:5678";
}
