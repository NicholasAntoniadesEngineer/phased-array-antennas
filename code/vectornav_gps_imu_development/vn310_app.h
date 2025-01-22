/**
 * @author Nicholas Antoniades
 * @date Jan 26, 2023
 */
#pragma once

#include <string.h>
#include <stdio.h>

#include "driver_vectornav.h"
#include "console_commands.h"
#include "driver_gpio.h"

struct app_vectornav_config_t
{
	struct driver_vectornav_config_t driver_config;
	struct cli_state_t *cli_state;
	struct bsp_pin_t power_enable;
	struct bsp_pin_t pri_r_en_l;
	struct bsp_pin_t pri_d_en;   
	struct bsp_pin_t sec_r_en_l;
	struct bsp_pin_t sec_d_en;
};

struct app_vectornav_state_t
{
	struct driver_vectornav_state_t driver_state;
	struct app_vectornav_config_t config;
};

STATUS app_vectornav_init( struct app_vectornav_state_t *state, struct app_vectornav_config_t *config);
STATUS app_vectornav_start(struct app_vectornav_state_t *state);
STATUS app_vectornav_run(struct app_vectornav_state_t *state);
