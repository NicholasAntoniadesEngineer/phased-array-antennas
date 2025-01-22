/**
 * @file hmc1119.h
 * @brief Driver header for the HMC1119 attenuator.
 * @author Nicholas Antoniades
 * @date Dec 1, 2023
 */

#pragma once

#include <string.h>
#include <stdio.h>
#include "config.h

#include "bsp/bsp_gpio.h"
#include "driver/spi.h"

#define HMC1119_RANGE 31.75
#define HMC1119_RES_BITS 7
#define HMC1119_ATTEN_MAX 127
#define HMC1119_ATTEN_MIN 0
#define HMC1119_ATTEN_PER_BIT (HMC1119_RANGE / ((1 << HMC1119_RES_BITS) - 1))

struct hmc1119_config_t
{
    struct spi_bus_t spi;

    struct bsp_pin_t D[7];
    struct bsp_pin_t LE;
    float insertion_loss;
};

struct hmc1119_state_t
{
    struct hmc1119_config_t config;
    uint8_t attenuation;
    float attenuation_db;
    float attenuation_db_corrected;
};

STATUS hmc1119_init(struct hmc1119_state_t *state, struct hmc1119_config_t const *config);
STATUS hmc1119_convert_attenuation_db_to_code(float attenuation_db, uint8_t *attenuation_code);
STATUS hmc1119_set(struct hmc1119_state_t *state, uint8_t attenuation);
STATUS hmc1119_set_db(struct hmc1119_state_t *state, float attenuation_db);
STATUS hmc1119_latch(struct hmc1119_state_t *state);
