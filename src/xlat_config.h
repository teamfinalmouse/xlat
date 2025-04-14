/*
 * Copyright (c) 2025 Finalmouse, LLC
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

#ifndef XLAT_CONFIG_H
#define XLAT_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include "xlat.h"

/**
 * @brief Set the XLAT mode
 * @param mode The mode to set
 */
void xlat_set_mode(enum xlat_mode mode);

/**
 * @brief Get the current XLAT mode
 * @return The current mode
 */
enum xlat_mode xlat_get_mode(void);

/**
 * @brief Set the auto-trigger level
 * @param high true for high level, false for low level
 */
void xlat_auto_trigger_level_set(bool high);

/**
 * @brief Get the auto-trigger level
 * @return true if high level, false if low level
 */
bool xlat_auto_trigger_level_is_high(void);

/**
 * @brief Set the auto-trigger interval
 * @param ms The interval in milliseconds (100-1000)
 */
void xlat_auto_trigger_interval_set(uint32_t ms);

/**
 * @brief Get the auto-trigger interval
 * @return The interval in milliseconds
 */
uint32_t xlat_auto_trigger_interval_get(void);

/**
 * @brief Set the auto-trigger output pin
 * @param pin The pin number (6 or 11)
 */
void xlat_auto_trigger_output_set(uint8_t pin);

/**
 * @brief Get the auto-trigger output pin
 * @return The pin number
 */
uint8_t xlat_auto_trigger_output_get(void);

/**
 * @brief Set the GPIO IRQ holdoff time
 * @param us The holdoff time in microseconds
 */
void xlat_set_gpio_irq_holdoff_us(uint32_t us);

/**
 * @brief Get the GPIO IRQ holdoff time
 * @return The holdoff time in microseconds
 */
uint32_t xlat_get_gpio_irq_holdoff_us(void);

/**
 * @brief Get the button bits
 * @return The button bits
 */
uint16_t * xlat_get_button_bits(void);

/**
 * @brief Get the motion bits
 * @return The motion bits
 */
uint16_t * xlat_get_motion_bits(void);

/**
 * @brief Get the report ID
 * @return The report ID
 */
uint8_t xlat_get_report_id(void);

/**
 * @brief Set the report ID
 * @param id The report ID
 */
void xlat_set_report_id(uint8_t id);

/**
 * @brief Get the keyboard usage page found
 * @return true if the keyboard usage page was found, false otherwise
 */
bool xlat_get_keyboard_usage_page_found(void);

/**
 * @brief Get the button mask
 * @return The button mask
 */
uint8_t *xlat_get_button_mask(void);

/**
 * @brief Get the motion mask
 * @return The motion mask
 */
uint8_t *xlat_get_motion_mask(void);

/**
 * @brief Set the keyboard usage page found
 * @param found true if the keyboard usage page was found, false otherwise
 */
void xlat_set_keyboard_usage_page_found(bool found);

#endif /* XLAT_CONFIG_H */ 