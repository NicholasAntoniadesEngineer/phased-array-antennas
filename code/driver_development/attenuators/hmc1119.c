/**
 * @file hmc1119.c
 * @brief Driver for the HMC1119 attenuator.
 * @author Nicholas Antoniades
 * @date Dec 1, 2023
 */

#include "hmc1119.h"
#include <math.h>

/**
 * @brief Initialize the HMC1119 driver.
 *
 * This function initializes the HMC1119 driver based on the provided configuration.
 *
 * @param[in] state Pointer to the HMC1119 driver state.
 * @param[in] config Pointer to the HMC1119 driver configuration.
 * @return OK if initialization is successful, otherwise an error status.
 */
STATUS hmc1119_init(struct hmc1119_state_t *state, struct hmc1119_config_t const *config)
{
    STATUS status;

    state->config = *config;

    state->attenuation = 0;
    state->attenuation_db = 0.0f;
    state->attenuation_db_corrected = 0.0f;

    if (state->config.spi.hspi)
    {
        struct bsp_pin_t spi_cs_pin = BSP_CONSTRUCT_PIN(&state->config.spi.pin_cs);

        status = bsp_gpio_init(&spi_cs_pin, BSP_GPIO_MODE_PUSH_PULL);
        if (status != OK)
        {
            return status;
        }

        status = bsp_gpio_write(&spi_cs_pin, 1);
        if (status != OK)
        {
            return status;
        }
    }
    else if (state->config.LE.port != NULL)
    {
        for (int i = 0; i < 7; i++)
        {
            status = bsp_gpio_init(&config->D[i], BSP_GPIO_MODE_PUSH_PULL);
            if (status != OK)
            {
                return status;
            }
        }

        status = bsp_gpio_init(&config->LE, BSP_GPIO_MODE_PUSH_PULL);
        if (status != OK)
        {
            return status;
        }

        status = bsp_gpio_write(&config->LE, 0);
        if (status != OK)
        {
            return status;
        }
    }

    return OK;
}

/**
 * @brief Convert attenuation in dB to attenuation bits for HMC1119.
 *
 * This function converts the given attenuation in dB to the corresponding
 * number of attenuation bits for the HMC1119 attenuator. If the calculated
 * attenuation bits exceed the valid range, they are clamped to the maximum value
 * and an error status is returned.
 *
 * @param[in] attenuation_db Attenuation in dB to be converted.
 * @param[out] attenuation_code Pointer to store the resulting attenuation bits.
 * @return OK if the attenuation bits are within the valid range, otherwise ERROR.
 */
STATUS hmc1119_convert_attenuation_db_to_code(float attenuation_db, uint8_t *attenuation_code)
{
    *attenuation_code = (int) round(attenuation_db / HMC1119_ATTEN_PER_BIT);

    if ((*attenuation_code > HMC1119_ATTEN_MAX) || (*attenuation_code < HMC1119_ATTEN_MIN))
    {
        *attenuation_code = HMC1119_ATTEN_MAX;
        return ERROR;
    }
    return OK; 
}

/**
 * @brief Set the HMC1119 attenuator.
 *
 * This function sets the attenuation for the HMC1119 attenuator.
 *
 * @param[in] state Pointer to the HMC1119 driver state.
 * @param[in] attenuation Attenuation in bits to be set.
 * @return OK if the attenuation is successfully set, otherwise ERROR.
 */
STATUS hmc1119_set(struct hmc1119_state_t *state, uint8_t attenuation)
{
    if(state->config.spi.hspi)
    {
        struct bsp_pin_t bsp_cs_pin = BSP_CONSTRUCT_PIN(&state->config.spi.pin_cs);

        bsp_gpio_write(&bsp_cs_pin, 0);

        if (OK != bsp_spi_transmit(state->config.spi.hspi, &attenuation, sizeof(uint8_t)))
        {
            return ERROR;
        }

        bsp_gpio_write(&bsp_cs_pin, 1);

        state->attenuation = attenuation;
    }
    else if(state->config.LE.port != NULL)
    {
        for (int i = 0; i < 7; i++)
        {
            bsp_gpio_write(&state->config.D[i], (attenuation & (1 << i)) != 0);
        }

        state->attenuation = attenuation;
    }
    else
    {
        return ERROR;
    }

    return OK;
}

/**
 * @brief Set the HMC1119 attenuator.
 *
 * This function sets the attenuation in dB
 *
 * @param[in] state Pointer to the HMC1119 driver state.
 * @param[in] attenuation Attenuation in dB.
 * @return OK if the attenuation is successfully set, otherwise ERROR.
 */
STATUS hmc1119_set_db(struct hmc1119_state_t *state, float attenuation_db)
{
    float modified_attenuation = attenuation_db - state->config.insertion_loss;

    if (modified_attenuation <= 0)
        return hmc1119_set(state, 0);

    int attenuation_code = (int) round(modified_attenuation * 4);

    STATUS ret = hmc1119_set(state, attenuation_code);
    if (ret !=  OK)
    {
        return ret;
    }

    state->attenuation_db = attenuation_db;
    state->attenuation_db_corrected = modified_attenuation;
    return OK;
}

/**
 * @brief Latch the attenuation value in the HMC1119 attenuator.
 *
 * This function latches the attenuation value in the HMC1119 attenuator by toggling the LE pin.
 *
 * @param[in] state Pointer to the HMC1119 driver state.
 * @return OK if the attenuation value is successfully latched, otherwise ERROR.
 */
STATUS hmc1119_latch(struct hmc1119_state_t *state)
{
    if (state->config.LE.port != NULL)
    {
        STATUS status;

        status = bsp_gpio_write(&state->config.LE, 0);
        if (status != OK)
        {
            return status;
        }

        /* Hold time (Tps) = 10ns */
        bsp_delay_ns(10); /* Placeholder for delay as it is only a few CPU cycles */

        /* Rising edge latches attenuation word */
        status = bsp_gpio_write(&state->config.LE, 1);
        if (status != OK)
        {
            /* No need to restore level of output pin */
            return status;
        }

        /*
         * Minimum LE pulse width (Tlew) = 10ns
         * Hold time (Tph) = 10ns
         * Minimum LE pulse spacing (Tles) = 630ns
         */
        bsp_delay_ns(630); /* Worst case time */

        status = bsp_gpio_write(&state->config.LE, 0);
        if (status != OK)
        {
            return status;
        }

        return OK;
    }
    else
    {
        return ERROR;
    }
}
