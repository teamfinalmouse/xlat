cmake_minimum_required(VERSION 3.12.4)

if(NOT ESP_PLATFORM)
  project(lvgl HOMEPAGE_URL https://github.com/lvgl/lvgl)
endif()

set(LVGL_ROOT_DIR .)

if(ESP_PLATFORM)
  include(env_support/cmake/esp.cmake)
elseif(ZEPHYR_BASE)
  include(env_support/cmake/zephyr.cmake)
elseif(MICROPY_DIR)
  include(env_support/cmake/micropython.cmake)
else()
  include(env_support/cmake/custom.cmake)
endif()
