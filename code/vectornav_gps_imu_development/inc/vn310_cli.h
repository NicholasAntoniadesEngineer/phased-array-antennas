/**
 * @file vn310_cli.h
 * @brief Header file for VectorNav CLI interface.
 * 
 * This file defines the command-line interface functions and structures
 * for interacting with the VectorNav sensor through the command line.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 */

#pragma once

#include "vn310_app.h"
#include "command_line_interface.h"

void vn310_cli_init(struct app_vn310_state_t *state, struct cli_state_t *cli_state); 