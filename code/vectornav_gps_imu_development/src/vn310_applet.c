/**
 * @file vn310_applet.c
 * @brief Implementation of the VectorNav applet functions.
 * 
 * This file contains the main application logic for the VectorNav sensor interface,
 * including initialization, state management, and high-level control functions.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 */
#include "vn310_applet.h"
#include "vn310_driver.h"

/**
 * @brief Initialize the vn310 app.
 *
 * @param state The state of the vn310 app.
 * @param config The configuration for the vn310 app.
 * @return OK if the initialization was successful.
 */
STATUS vn310_applet_init(struct vn310_applet_state_t *state, struct vn310_applet_config_t *config)
{
    state->config = *config;
    return OK;
}

/**
 * @brief Run the vn310 app.
 *
 * This function handles message processing and pose updates.
 *
 * @param state The state of the vn310 app.
 * @return OK if the run was successful.
 */
STATUS vn310_applet_run(struct vn310_applet_state_t *state)
{
    if (state->driver_state.vn310_message_ready)
    {
        if (state->driver_state.response_expected || state->driver_state.uart_stream)
        {
            vn310_driver_print_stream(&state->driver_state, state->config.cli_state);
            state->driver_state.response_expected = false;
        }

        int valid_data = 0;
        if (state->driver_state.assembled_message_type == MSG_ASYNC)
        {
            if (handle_pose_message((const char *)&state->driver_state.assembled_message, &pose) == OK)
            {
                pose.rate[0] = 0.0f;
                pose.rate[1] = 0.0f;
                pose.rate[2] = 0.0f;
                valid_data = 1;
            }
        }
        else if (state->driver_state.assembled_message_type == MSG_BINARY)
        {
            const struct driver_vn310_binout_config0_data_t *data = NULL;
            if (vn310_driver_get_configuration_0_data(&state->driver_state, &data) == OK)
            {
                insStatus = data->ins_status.sol_status;
                pose.latitude = data->position.latitude;
                pose.longitude = data->position.longitude;
                pose.yaw = data->yaw_pitch_roll.yaw;
                pose.pitch = data->yaw_pitch_roll.pitch;
                pose.roll = data->yaw_pitch_roll.roll;
                pose.rate[0] = radians_to_degrees(data->angular_rate.rate[0]);
                pose.rate[1] = radians_to_degrees(data->angular_rate.rate[1]);
                pose.rate[2] = radians_to_degrees(data->angular_rate.rate[2]);
                valid_data = 1;
            }
        }

        if (valid_data)
        {

        }

        state->driver_state.vn310_message_ready = false;
    }

    return OK;
}

/**
 * @brief Start the vn310 app.
 *
 * This function initializes the CLI, GPIO pins, and configures the VN310 driver.
 *
 * @param state The state of the vn310 app.
 * @return OK if the start was successful.
 */
STATUS vn310_applet_start(struct vn310_applet_state_t *state)
{
    vn310_cli_init(state, state->config.cli_state);

    _init_hardware(struct vn310_applet_state_t *state);

    return OK;
} 