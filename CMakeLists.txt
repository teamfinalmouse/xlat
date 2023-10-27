cmake_minimum_required(VERSION 3.10)
include("arm-none-eabi.cmake")
# project name
project(xlat C ASM)

# Generate compile_commands.json file 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# select linker script
set(LINKER_SCRIPT "STM32F746NGHx_FLASH.ld")

# define CPU OPTIONS
set(CPU_OPTIONS -mthumb -march=armv7e-m -mfloat-abi=hard -mfpu=fpv5-sp-d16 -mcpu=cortex-m7)

# optimizations (-O0 -O1 -O2 -O3 -Os -Ofast -Og -flto)
set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -ggdb -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-Os")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s)

# Define the application version
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 0)
set(APP_VERSION_PATCH 0)
set(APP_VERSION_STRING "${APP_VERSION_MAJOR}.${APP_VERSION_MINOR}.${APP_VERSION_PATCH}")

add_compile_options(
        ${CPU_OPTIONS}
        $<$<COMPILE_LANGUAGE:C>:-std=gnu99>
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>
        $<$<COMPILE_LANGUAGE:CXX>:-fms-extensions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
        -fstrict-volatile-bitfields
        -ffunction-sections
        -fdata-sections
        -Wall
        -Wshadow
)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/drivers
        ${CMAKE_SOURCE_DIR}/drivers/BSP/STM32746G-Discovery
        ${CMAKE_SOURCE_DIR}/drivers/CMSIS/Device/ST/STM32F7xx/Include
        ${CMAKE_SOURCE_DIR}/drivers/CMSIS/Include
        ${CMAKE_SOURCE_DIR}/drivers/STM32F7xx_HAL_Driver/Inc
        ${CMAKE_SOURCE_DIR}/drivers/STM32F7xx_HAL_Driver/Inc/Legacy
        ${CMAKE_SOURCE_DIR}/libs
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/include
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/CMSIS_RTOS
        ${CMAKE_SOURCE_DIR}/libs/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1
        ${CMAKE_SOURCE_DIR}/libs/lvgl
        ${CMAKE_SOURCE_DIR}/libs/RTT/RTT
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/config
        ${CMAKE_SOURCE_DIR}/src/usb
)

link_directories(
        ${CMAKE_SOURCE_DIR}
)

set(HAL_Sources
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_adc.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_adc_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2s.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_crc.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_crc_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma2d.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_ltdc_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dsi.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rtc.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rtc_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sai_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_spdifrx.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_sdram.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart_ex.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_hcd.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_fmc.c
        drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_ll_usb.c
        drivers/BSP/Components/ft5336/ft5336.c
        drivers/BSP/Components/wm8994/wm8994.c
        drivers/BSP/STM32746G-Discovery/stm32746g_discovery.c
        drivers/BSP/STM32746G-Discovery/stm32746g_discovery_sdram.c
        drivers/BSP/STM32746G-Discovery/stm32746g_discovery_audio.c
        drivers/BSP/STM32746G-Discovery/stm32746g_discovery_ts.c
        )

set(FREERTOS_SOURCES
        libs/FreeRTOS/Source/croutine.c
        libs/FreeRTOS/Source/event_groups.c
        libs/FreeRTOS/Source/list.c
        libs/FreeRTOS/Source/queue.c
        libs/FreeRTOS/Source/stream_buffer.c
        libs/FreeRTOS/Source/tasks.c
        libs/FreeRTOS/Source/timers.c
        libs/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c
        libs/FreeRTOS/Source/portable/MemMang/heap_4.c
        libs/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.c
        )

set(RTT_SOURCES
        libs/RTT/RTT/SEGGER_RTT.c
        libs/RTT/RTT/SEGGER_RTT_printf.c
        libs/RTT/Syscalls/SEGGER_RTT_Syscalls_GCC.c
        )

set(USB_SOURCES
        src/usb/usbh_conf.c
        src/usb/usb_host.c
        src/usb/usbh_core.c
        src/usb/usbh_ctlreq.c
        src/usb/usbh_ioreq.c
        src/usb/usbh_pipes.c
        src/usb/usbh_hid.c
        src/usb/usbh_hid_keybd.c
        src/usb/usbh_hid_mouse.c
        )

#FILE(GLOB_RECURSE LVGL_Sources CONFIGURE_DEPENDS hal_stm_lvgl/*.c)


add_definitions(
        -DSTM32
        -DSTM32F7
        -DSTM32F746xx
        -DSTM32F746G_DISCO
        -DDEBUG
        -DUSE_HAL_DRIVER
)

add_subdirectory(libs/lvgl)

target_compile_options(lvgl PRIVATE -DSTM32F7)

# list of modules to build final firmware (without extension .c or .cpp)
add_executable(${PROJECT_NAME}
        src/startup_stm32f746xx.s
        src/main.c
        src/gfx_main.c
        src/gfx_settings.c
        src/hardware_config.c
        src/freertos_hooks.c
        src/stdio_glue.c
        src/stm32f7xx_it.c
        src/stm32f7xx_hal_msp.c
        src/stm32f7xx_hal_timebase_tim.c
        src/syscalls.c
        src/system_stm32f7xx.c
        src/xlat.c
        src/theme/xlat_logo_160px_width_idx8.c
        drivers/tft/tft.c
        drivers/touchpad/touchpad.c
        ${HAL_Sources}
        ${LVGL_Sources}
        ${FREERTOS_SOURCES}
        ${RTT_SOURCES}
        ${USB_SOURCES}
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE -DAPP_VERSION_MAJOR=${APP_VERSION_MAJOR})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DAPP_VERSION_MINOR=${APP_VERSION_MINOR})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DAPP_VERSION_PATCH=${APP_VERSION_PATCH})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DAPP_VERSION="${APP_VERSION_STRING}")
target_compile_definitions(${PROJECT_NAME} PRIVATE -DAPP_VERSION_FULL="${APP_VERSION_STRING}-${GIT_SHA}")

target_link_libraries(${PROJECT_NAME}
        ${CPU_OPTIONS}
        -T${LINKER_SCRIPT}
        -Wl,-Map=${PROJECT_NAME}.map
        -specs=nano.specs
        -Wl,--gc-sections
        c
        m
        nosys
        lvgl
        )

set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX .elf
        LINK_DEPENDS "${LINKER_SCRIPTS}"
        )

add_custom_target(${PROJECT_NAME}.bin ALL DEPENDS ${PROJECT_NAME})
add_custom_command(TARGET ${PROJECT_NAME}.bin
        COMMAND ${CMAKE_C_OBJCOPY} ARGS -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin)

set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_DEPENDS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
