cmake_minimum_required(VERSION 3.10)
project(lvgl_linux)

# Define project root directory
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Set build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Force Debug build type
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)


# Configure LVGL
set(LV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h CACHE STRING "Path to LVGL config file")
set(LV_DRV_CONF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lv_drv_conf.h CACHE STRING "Path to LVGL drivers config file")

set(LV_USE_SDL ON CACHE BOOL "Enable SDL2 support")

if (LV_USE_SDL)
    message("Including SDL2 support")
    find_package(PkgConfig REQUIRED)
    
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

    list(APPEND PKG_CONFIG_LIB ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
    list(APPEND PKG_CONFIG_INC ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

    # Add SDL driver source
    set(SDL_DRIVER_SRC ${PROJECT_ROOT}/libs/lv_drivers/sdl/sdl.c)
endif()

set(LVGL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LVGL_BUILD_DEMOS OFF CACHE BOOL "" FORCE)

# Add LVGL library with configuration
add_subdirectory(${PROJECT_ROOT}/libs/lvgl lvgl)

# Find SDL2
find_package(SDL2 REQUIRED)

# Create executable
add_executable(lvgl_linux
    main_sdl.c
    stubs/stubs.c
    ${SDL_DRIVER_SRC}
    ${PROJECT_ROOT}/src/gfx_main.c
    ${PROJECT_ROOT}/src/theme/xlat_fm_logo_130px.c
)

# Include directories
target_include_directories(lvgl_linux PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/stubs
    ${PROJECT_ROOT}/libs
    ${PROJECT_ROOT}/libs/lvgl
    ${PROJECT_ROOT}/libs/lv_drivers
    ${PROJECT_ROOT}/drivers
    ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(lvgl_linux PRIVATE
    lvgl
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# Add compile definitions for LVGL
target_compile_definitions(lvgl_linux PRIVATE
    LV_CONF_INCLUDE_SIMPLE
    LV_CONF_PATH=${LV_CONF_PATH}
    LV_DRV_CONF_PATH=${LV_DRV_CONF_PATH}
)