/**
 * @file 74hc595pw.h
 * @brief Header file for the 74HC595 shift register driver.
 * 
 * This file contains the definitions and function prototypes for the 74HC595 driver.
 * It includes configuration structures, state tracking, and function declarations for
 * initializing, configuring, and controlling the 74HC595 8-bit serial-in, parallel-out
 * shift register.
 * 
 * @author Nicholas Antoniades
 * @date July 11, 2023
 */

#ifndef HC595PW_H
#define HC595PW_H

#include <stdint.h>
#include <string.h>
#include "74hc165pw.h"

#define HC595_DELAY 0

typedef struct
{
    struct platform_pin_t const *DS_pin;   /**< Serial data input pin */
    struct platform_pin_t const *OE_pin;   /**< Output enable pin */
    struct platform_pin_t const *SHCP_pin; /**< Shift register clock pin */
    struct platform_pin_t const *STCP_pin; /**< Storage register clock pin */
}74HC595pw_config_t;

typedef struct
{
    74HC595pw_config_t config;
    uint8_t output_parallel_value;         /**< Current value of parallel output pins */
}74HC595pw_state_t;

STATUS 74hc595pw_init_GPIO(74HC595pw_state_t *state);
STATUS 74hc595pw_init(74HC595pw_state_t *state, 74HC595pw_config_t const *config);
STATUS 74hc595pw_shift_bit(74HC595pw_state_t *state, uint8_t bit);
STATUS 74hc595pw_shift_byte(74HC595pw_state_t *state, uint8_t byte);
STATUS 74hc595pw_clear_shift_register(74HC595pw_state_t *state);
STATUS 74hc595pw_output_parallel(74HC595pw_state_t *state, uint8_t value);
STATUS 74hc595pw_enable_outputs(74HC595pw_state_t *state);
STATUS 74hc595pw_disable_outputs(74HC595pw_state_t *state);
STATUS 74hc595pw_latch_low(74HC595pw_state_t *state);
STATUS 74hc595pw_latch_high(74HC595pw_state_t *state);
STATUS 74hc595pw_hc165_test(74HC165_state_t current_state_74HC165, 74HC595pw_state_t current_state_74HC595);
void 74hc595pw_get_output_parallel(74HC595pw_state_t *state, uint8_t *value);

#endif /* HC595PW_H */

