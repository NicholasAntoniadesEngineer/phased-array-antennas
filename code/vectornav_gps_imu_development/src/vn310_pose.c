/**
 * @file vn310_pose.c
 * @brief Implementation of pose-related functions for VectorNav sensor.
 * 
 * This file contains functions for handling pose data, including coordinate transformations,
 * angle conversions, and pose data updates from the VectorNav sensor.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 */

#include "vn310_pose.h"
#include "message_routing.h"
#include "message_pose.h"

float vn310_pose_wrap_0_to_360_degrees(float input)
{
    input = fmod(input, 360.0);
    if (input < 0)
    {
        input = 360 + input;
    }
    return input;
}

float vn310_pose_radians_to_degrees(float input)
{
    return input * (360.0f / (2.0f * M_PI));
}

void vn310_pose_send_updated(struct app_vn310_state_t *state, struct vn310_pose_t *vn310_pose, bool forced)
{
    if (state->driver_state.send_pose || forced)
    {
        struct message_pose_t message;
        message_pose_init(&message);
        struct vn310_pose_t vn310_update_pose = *vn310_pose;
        vn310_update_pose.roll = vn310_pose_wrap_0_to_360_degrees(vn310_pose->roll);
        vn310_update_pose.pitch = vn310_pose_wrap_0_to_360_degrees(vn310_pose->pitch);
        vn310_update_pose.yaw = vn310_pose_wrap_0_to_360_degrees(vn310_pose->yaw);
        // TODO: Add this in properly. This is intended to be distance from sea level, but this
        // may be defined differently by the vertornav?
        vn310_update_pose.altitude = 0;

        message_pose_update_message(&message, vn310_update_pose);

        if (OK != message_routing_send_message_to((uint8_t*)&message,
                BOARD_TYPE_ACON_MAJ_INT, TILE_INDEX_UNSPECIFIED))
        {
            WARN("Routing failed for message_pose from app_vn310");
        }
    }
} 