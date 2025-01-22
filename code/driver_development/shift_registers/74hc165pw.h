/**
 * @file 74hc165pw.h
 * @brief Header file for the 74HC165 shift register driver.
 * 
 * This file contains the definitions and function prototypes for the 74HC165 driver.
 * It includes configuration structures, state tracking, and function declarations for
 * initializing, configuring, and controlling the 74HC165 8-bit parallel-in, serial-out
 * shift register.
 * 
 * @author Nicholas Antoniades
 * @date July 11, 2023
 */

#ifndef HC165PW_H
#define HC165PW_H

#include <stdint.h>
#include <string.h>
#include "bsp/bsp_gpio.h"

#define HC165_NUM_BITS 8
#define HC165_DELAY 0

typedef struct
{
    uint8_t parallel_data;  /**< Data from the parallel data lines */
    uint8_t serial_data;    /**< Data that has been fed in serially via the DS pin */
}74HC165_data_t;

typedef struct
{
    struct platform_pin_t const *PL_pin;   /**< Parallel load pin */
    struct platform_pin_t const *CP_pin;   /**< Clock pin */
    struct platform_pin_t const *Q7_pin;   /**< Serial data output pin */
    struct platform_pin_t const *OE_pin;   /**< Output enable pin */
}74HC165_config_t;

typedef struct
{
    74HC165_config_t config;
    uint8_t read_bit;                  /**< Binary value of bit read on last clock cycle */
    74HC165_data_t read_data;         /**< Value pair representing the parallel and serial data bytes */
}74HC165_state_t;

STATUS 74hc165pw_init(74HC165_state_t *state, 74HC165_config_t const *config);
STATUS 74hc165pw_init_GPIO(74HC165_state_t *state);
STATUS 74hc165pw_shift_bit(74HC165_state_t *state);
STATUS 74hc165pw_read_parallel_inputs(74HC165_state_t *state);
STATUS 74hc165pw_latch_low(74HC165_state_t *state);
STATUS 74hc165pw_latch_high(74HC165_state_t *state);
STATUS 74hc165pw_enable_outputs(74HC165_state_t *state);

#endif /* HC165PW_H */

