/**
 * @file 74hc165pw.c
 * @brief Implementation file for the 74HC165 shift register driver.
 * 
 * This file contains the implementation of functions for initializing, configuring,
 * and controlling the 74HC165 8-bit parallel-in, serial-out shift register. It provides
 * functionality for reading parallel data inputs and shifting them out serially, along with
 * control over the parallel load and output enable operations.
 * 
 * @author Nicholas Antoniades
 * @date July 11, 2023
 */
#include <stdint.h>
#include <string.h>
#include "74hc165pw.h"
#include "74hc595pw.h"

/**
 * @brief Initialize GPIO for the 74HC165 Shift Register
 * @param state Pointer to the state structure containing pin configurations
 * @return STATUS indicating success or failure
 */
STATUS 74hc165pw_init_GPIO(74HC165_state_t *state)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = state->config.CP_pin->number;
    HAL_GPIO_Init(state->config.CP_pin->port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = state->config.PL_pin->number;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(state->config.PL_pin->port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = state->config.OE_pin->number;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(state->config.OE_pin->port, &GPIO_InitStruct);

    // Configure Q7 GPIO pin as an input
    GPIO_InitStruct.Pin = state->config.Q7_pin->number;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(state->config.Q7_pin->port, &GPIO_InitStruct);
    return R2_OK;
}

/**
 * @brief Enable the outputs of the 74HC165 shift register
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc165pw_enable_outputs(74HC165_state_t *state)
{
    HAL_GPIO_WritePin(state->config.OE_pin->port, state->config.OE_pin->number, GPIO_PIN_SET);
    return R2_OK;
}

/**
 * @brief Initialize the 74HC165 shift register
 * @param state Pointer to the state structure to initialize
 * @param config Pointer to the configuration structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc165pw_init(74HC165_state_t *state, 74HC165_config_t const *config)
{
    state->config = *config;
    74hc165pw_init_GPIO(state);
    return R2_OK;
}

/**
 * @brief Shift in a single bit from the 74HC165 shift register
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc165pw_shift_bit(74HC165_state_t *state)
{
    uint8_t read_bit;

    read_bit = HAL_GPIO_ReadPin(state->config.Q7_pin->port, state->config.Q7_pin->number);
    HAL_GPIO_WritePin(state->config.CP_pin->port, state->config.CP_pin->number, GPIO_PIN_RESET);
    HAL_Delay(HC165_DELAY);
    HAL_GPIO_WritePin(state->config.CP_pin->port, state->config.CP_pin->number, GPIO_PIN_SET);
    HAL_Delay(HC165_DELAY);

    state->read_bit = read_bit;
    return R2_OK;
}

/**
 * @brief Read the parallel inputs from the 74HC165 shift register
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc165pw_read_parallel_inputs(74HC165_state_t *state)
{
    uint8_t parallel_data_byte = 0;
    uint8_t serial_data_byte = 0;
    uint8_t bit = 0;

    for (int i = 7; i >= 0; i--)
    {
        74hc165pw_shift_bit(state);
        bit = state->read_bit;
        parallel_data_byte |= (bit << i);
    }

    for (int j = 7; j >= 0; j--)
    {
        74hc165pw_shift_bit(state);
        bit = state->read_bit;
        serial_data_byte |= (bit << j);
    }

    state->read_data.parallel_data = parallel_data_byte;
    state->read_data.serial_data = serial_data_byte;

    return R2_OK;
}

/**
 * @brief Set the parallel load pin to low
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc165pw_latch_low(74HC165_state_t *state)
{
    HAL_GPIO_WritePin(state->config.PL_pin->port, state->config.PL_pin->number, GPIO_PIN_RESET);
    return R2_OK;
}

/**
 * @brief Set the parallel load pin to high
 * @param state Pointer to the state structure
 * @return STATUS indicating success or failure
 */
STATUS 74hc165pw_latch_high(74HC165_state_t *state)
{
    HAL_GPIO_WritePin(state->config.PL_pin->port, state->config.PL_pin->number, GPIO_PIN_SET);
    return R2_OK;
}
