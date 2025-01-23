/**
 * @file vn310_applet.h
 * @brief Header file for VectorNav applet functionality.
 * 
 * This file defines the structures and functions for the main VectorNav
 * application, including state management and initialization.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 */

#pragma once

#include "vn310_cli.h"
#include "vn310_pose.h"
#include "vn310_parser.h"
#include "driver_gpio.h"

struct vn310_applet_state_t {
    struct vn310_applet_config_t config;
    struct driver_vn310_state_t driver_state;
    struct vn310_pose_t pose_data;  // Added pose data to state
};

/**
 * @brief Initialize the vn310 app.
 *
 * @param state The state of the vn310 app.
 * @param config The configuration for the vn310 app.
 * @return OK if the initialization was successful.
 */
STATUS vn310_applet_init(struct vn310_applet_state_t *state, struct vn310_applet_config_t *config);

/**
 * @brief Start the vn310 app.
 *
 * This function initializes the CLI, GPIO pins, and configures the VN310 driver.
 *
 * @param state The state of the vn310 app.
 * @return OK if the start was successful.
 */
STATUS vn310_applet_start(struct vn310_applet_state_t *state);

/**
 * @brief Run the vn310 app.
 *
 * This function handles message processing and pose updates.
 *
 * @param state The state of the vn310 app.
 * @return OK if the run was successful.
 */
STATUS vn310_applet_run(struct vn310_applet_state_t *state); 