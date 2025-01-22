/**
 * @file 74hc595pw.c
 * @brief Implementation file for the 74HC595 shift register driver.
 * 
 * This file contains the implementation of functions for initializing, configuring,
 * and controlling the 74HC595 8-bit serial-in, parallel-out shift register. It provides
 * functionality for shifting data in serially and outputting it in parallel, along with
 * control over the output enable and storage register operations.
 * 
 * @author Nicholas Antoniades
 * @date July 11, 2023
 */

#include <stdint.h>
#include <string.h>
#include "74hc595pw.h"
#include "74hc165pw.h"

/**
 * @brief Test function for interfacing between 74HC165 and 74HC595 devices
 * @param current_state_74HC165 Pointer to the 74HC165 state structure
 * @param current_state_74HC595 Pointer to the 74HC595 state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_hc165_test(74HC165_state_t current_state_74HC165, 74HC595pw_state_t current_state_74HC595)
{
    74hc165pw_init_GPIO(&current_state_74HC165);
    74hc595pw_init_GPIO(&current_state_74HC595);
    74hc595pw_enable_outputs(&current_state_74HC595);

    for(int i = 0; i < 256; i++) {
        74hc595pw_output_parallel(&current_state_74HC595, i);
        74hc165pw_read_parallel_inputs(&current_state_74HC165);
        74HC165_data_t read_data = current_state_74HC165.read_data;
        (void)read_data;
        HAL_Delay(5);
    }
    return R2_OK;
}

/**
 * @brief Initialize GPIO for the 74HC595 Shift Register
 * @param state Pointer to the state structure containing pin configurations
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_init_GPIO(74HC595pw_state_t *state)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct.Pin = state->config.OE_pin->number | state->config.SHCP_pin->number | 
                         state->config.STCP_pin->number | state->config.DS_pin->number;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(state->config.OE_pin->port, &GPIO_InitStruct);

    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitStruct.Pin = state->config.DS_pin->number;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(state->config.DS_pin->port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(state->config.OE_pin->port, state->config.OE_pin->number, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(state->config.SHCP_pin->port, state->config.SHCP_pin->number, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(state->config.STCP_pin->port, state->config.STCP_pin->number, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(state->config.DS_pin->port, state->config.DS_pin->number, GPIO_PIN_RESET);
    return R2_OK;
}

/**
 * @brief Initialize the 74HC595 shift register
 * @param state Pointer to the state structure to initialize
 * @param config Pointer to the configuration structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_init(74HC595pw_state_t *state, 74HC595pw_config_t const *config)
{
    state->config = *config;
    74hc595pw_init_GPIO(state);
    74hc595pw_enable_outputs(state);
    return R2_OK;
}

/**
 * @brief Shift a single bit into the 74HC595 shift register
 * @param state Pointer to the state structure
 * @param write_bit The bit to shift in
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_shift_bit(74HC595pw_state_t *state, uint8_t write_bit)
{
    HAL_GPIO_WritePin(state->config.SHCP_pin->port, state->config.SHCP_pin->number, GPIO_PIN_RESET);
    HAL_Delay(HC595_DELAY);

    GPIO_PinState pin_state = (write_bit != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(state->config.DS_pin->port, state->config.DS_pin->number, pin_state);

    HAL_GPIO_WritePin(state->config.SHCP_pin->port, state->config.SHCP_pin->number, GPIO_PIN_SET);
    HAL_Delay(HC595_DELAY);

    state->output_parallel_value = ((state->output_parallel_value << 1) | (write_bit != 0)) & 0xFFU;

    return R2_OK;
}

/**
 * @brief Shift a byte into the 74HC595 shift register
 * @param state Pointer to the state structure
 * @param byte The byte to shift in
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_shift_byte(74HC595pw_state_t *state, uint8_t byte)
{
    for (int i = 7; i >= 0; i--) {
        74hc595pw_shift_bit(state, byte & (1 << i));
    }
    HAL_GPIO_WritePin(state->config.DS_pin->port, state->config.DS_pin->number, GPIO_PIN_SET);
    return R2_OK;
}

/**
 * @brief Set the storage register clock pin to low
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_latch_low(74HC595pw_state_t *state)
{
    HAL_GPIO_WritePin(state->config.STCP_pin->port, state->config.STCP_pin->number, GPIO_PIN_RESET);
    return R2_OK;
}

/**
 * @brief Set the storage register clock pin to high
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_latch_high(74HC595pw_state_t *state)
{
    HAL_GPIO_WritePin(state->config.STCP_pin->port, state->config.STCP_pin->number, GPIO_PIN_SET);
    return R2_OK;
}

/**
 * @brief Clear all bits in the shift register
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_clear_shift_register(74HC595pw_state_t *state)
{
    HAL_Delay(HC595_DELAY);
    HAL_GPIO_WritePin(state->config.OE_pin->port, state->config.OE_pin->number, GPIO_PIN_RESET);
    HAL_Delay(HC595_DELAY);
    HAL_GPIO_WritePin(state->config.OE_pin->port, state->config.OE_pin->number, GPIO_PIN_SET);
    return R2_OK;
}

/**
 * @brief Enable the outputs of the 74HC595 shift register
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_enable_outputs(74HC595pw_state_t *state)
{
    HAL_GPIO_WritePin(state->config.OE_pin->port, state->config.OE_pin->number, GPIO_PIN_SET);
    return R2_OK;
}

/**
 * @brief Disable the outputs of the 74HC595 shift register
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_disable_outputs(74HC595pw_state_t *state)
{
    HAL_GPIO_WritePin(state->config.OE_pin->port, state->config.OE_pin->number, GPIO_PIN_RESET);
    return R2_OK;
}

/**
 * @brief Output a byte to the parallel output pins
 * @param state Pointer to the state structure
 * @param value The byte to output
 * @return STATUS indicating success or failure
 */
STATUS 74hc595pw_output_parallel(74HC595pw_state_t *state, uint8_t value)
{
    74hc595pw_latch_low(state);
    74hc595pw_shift_byte(state, value);
    74hc595pw_latch_high(state);
    return R2_OK;
}

/**
 * @brief Get the current value of the parallel output pins
 * @param state Pointer to the state structure
 * @param value Pointer to store the current output value
 */
void 74hc595pw_get_output_parallel(74HC595pw_state_t *state, uint8_t *value)
{
    *value = state->output_parallel_value;
}
