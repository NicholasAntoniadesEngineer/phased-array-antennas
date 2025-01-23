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

struct pose_t pose;
uint16_t insStatus;

float wrap_0_to_360_degrees(float input)
{
    input = fmod(input, 360.0);
    if (input < 0)
    {
        input = 360 + input;
    }
    return input;
}

float radians_to_degrees(float input)
{
    return input * (360.0f / (2.0f * M_PI));
}

void send_updated_pose(struct app_vn310_state_t *state, struct pose_t *p, bool forced)
{
    if (state->driver_state.send_pose || forced)
    {
        struct message_pose_t message;
        message_pose_init(&message);
        struct pose_t wp = *p;
        wp.roll = wrap_0_to_360_degrees(p->roll);
        wp.pitch = wrap_0_to_360_degrees(p->pitch);
        wp.yaw = wrap_0_to_360_degrees(p->yaw);
        // TODO: Add this in properly. This is intended to be distance from sea level, but this
        // may be defined differently by the vertornav?
        wp.altitude = 0;

        message_pose_update_message(&message, wp);

        if (OK != message_routing_send_message_to((uint8_t*)&message,
                BOARD_TYPE_ACON_MAJ_INT, TILE_INDEX_UNSPECIFIED))
        {
            WARN("Routing failed for message_pose from app_vn310");
        }
    }
} 