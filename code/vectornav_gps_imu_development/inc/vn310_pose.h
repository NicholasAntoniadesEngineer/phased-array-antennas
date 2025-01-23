/**
 * @file vn310_pose.h
 * @brief Header file for VectorNav pose data structures and functions.
 * 
 * This file defines the pose data structure and related functions for handling
 * orientation, position, and rate data from the VectorNav sensor.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 */

#pragma once

#include <math.h>
#include "vn310_app.h"

struct vn310_pose_t {
    float roll;
    float pitch;
    float yaw;
    float latitude;
    float longitude;
    float altitude;
    float rate[3];
    uint16_t ins_status;
};

float vn310_pose_wrap_0_to_360_degrees(float input);
float vn310_pose_radians_to_degrees(float input);
void vn310_pose_send_updated(struct app_vn310_state_t *state, struct vn310_pose_t *p, bool forced); 