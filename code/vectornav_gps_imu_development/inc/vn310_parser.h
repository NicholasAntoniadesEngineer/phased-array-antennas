/**
 * @file vn310_parser.h
 * @brief Header file for VectorNav message parsing.
 * 
 * This file defines the structures and functions for parsing messages
 * from the VectorNav sensor, including both binary and ASCII formats.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 */

#pragma once

#include "vn310_pose.h"

STATUS vn310_parser_parse_VNINS(const char *recieved_string, struct vn310_pose_t *vn310_pose);
STATUS vn310_parser_handle_pose_message(const char *recieved_string, struct vn310_pose_t *vn310_pose); 