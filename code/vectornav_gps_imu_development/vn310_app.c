/**
 * @author Nicholas Antoniades
 * @date Jan 26, 2023
 */
#include <string.h>
#include <stdio.h>
#include "app_main.h"

#include "command_line_interface.h"
#include "vn310_app.h"
#include "driver_gpio.h"

// #include "system_metadata.h"


#define LOCATION_DEFAULT_LAT 51.52010
#define LOCATION_DEFAULT_LNG -0.11002
#define USE_DEFAULT_LOC 0   //overwrites lat and lng with these fixed values, for test purposes when VN has no antennas


static struct pose_t pose;   //VN ranges are Yaw:+/-180, Pitch: +/-90, Roll: +/-180. Yaw is around Z-axis, Pitch is around Y-axis and Roll is around X-axis as marked on the top of the VN
static uint16_t insStatus;

/**
 * @brief Initialize the vn310 app.
 *
 * @param vn310_app_config The configuration for the vn310 app.
 * @param vn310_app_state The state of the vn310 app.
 * @return OK if the initialization was successful.
 */
STATUS app_vn310_init( struct app_vn310_state_t *state, struct app_vn310_config_t *config)
{
	state->config = *config;

#if USE_DEFAULT_LOC
pose.latitude = LOCATION_DEFAULT_LAT;
pose.longitude = LOCATION_DEFAULT_LNG;
#warning "Latitude/Longitude hardwired to values in LOCATION_DEFAULT_LAT / LOCATION_DEFAULT_LNG. Should be disabled other than for testing"
#endif

    return OK;
}

static float wrap_0_to_360_degrees(float input)
{
	input = fmod(input, 360.0);
	if (input < 0)
	{
		input = 360 + input;
	}
	return input;
}

static inline float radians_to_degrees(float input)
{
    return input * (360.0f / (2.0f * M_PI));
}

static void send_updated_pose(struct app_vn310_state_t *state, struct pose_t *p, bool forced)
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

//example of s format is
//"$VNINS,125176.941097,2332,8206,+082.014,+000.014,+001.063,+51.51992529,-000.11006359,+00089.216,-000.001,-000.008,-000.125,03.9,01.2,0.10*65"
enum {toknum_insstatus = 3, toknum_yaw, toknum_pitch, toknum_roll, toknum_poslat, toknum_poslon, toknum_posalt};
static STATUS parse_VNINS(const char *s, struct pose_t *p)
{
    char *t = strtok((char *)s, ",");
    int i = 0;
    while (t)
    {
        switch (i)
        {
            case toknum_insstatus:
                insStatus = (uint16_t)strtoul(t, NULL, 16);
                break;
            case toknum_yaw:
                p->yaw = strtod(t, NULL);
                break;
            case toknum_pitch:
                p->pitch = strtod(t, NULL);
                break;
            case toknum_roll:
                p->roll = strtod(t, NULL);
                break;
            case toknum_poslat:
                {
                    float v = strtod(t, NULL);
                    p->latitude = v;
                }
                break;
            case toknum_poslon:
                {
                    float v = strtod(t, NULL);
                    p->longitude = v;
                }
                break;
            case toknum_posalt:
                //float alt = strtod(t, NULL);
                break;
            default:
                break;
        }
        t = strtok(NULL, ",");
        i++;
    }

    if (i > toknum_roll)
    {
        return OK;
    }
    return ERROR;
}

static STATUS handle_pose_message(const char *s, struct pose_t *p)
{
    if (0 == strncmp(s, "$VNINS", 6))
    {
        return parse_VNINS(s, p);
    }
    //todo call parsers for other formats here eg binary

    return ERROR;    //not a handled pose message, but might be another response, doesn't mean corrupted
}

#define INS_STATUS_MASK_MODE 0x0003 //Mode of INS filter
#define INS_STATUS_MASK_GNSS_FIX 0x0004 //Indicates whether the GNSS has a valid fix.
#define INS_STATUS_MASK_GNSS_ERR 0x0040 //High if GNSS communication error is detected or if no valid PPS signal is received.
#define INS_STATUS_MASK_GNSS_COMPASS 0x0200 //Indicates if the GNSS compass is operational and reporting a heading solution.

static char *strInsMode(uint16_t mode)
{
    switch (mode & INS_STATUS_MASK_MODE)
    {
        case 0:
            return "Magn";   //heading is entirely magnetometer-derived
        case 1:
            return "M/GS"; //heading is either entirely magnetometer-derived or is in the process of switching from that to GNSS
        case 2:
            return "GNSS";  //heading is entirely GNSS-derived, the magnetometer is ignored
    }
    return "Unknown";
}


/**
 * @brief Run the vn310 app.
 *
 * @param vn310_state The state of the vn310 driver.
 * @param cli_state The current state of the command line interface.
 * @return OK if the run was successful.
 */
STATUS app_vn310_run(struct app_vn310_state_t *state)
{
	if (state->driver_state.vn310_message_ready)
    {
        if (state->driver_state.response_expected || state->driver_state.uart_stream)
	    {
    	    driver_vn310_print_stream(&state->driver_state, state->config.cli_state);

    	    state->driver_state.response_expected = false;
        }

        // Manage pointing compensation variable updates here
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
            if (driver_vn310_get_configuration_0_data(&state->driver_state, &data) == OK)
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

static STATUS vn310_set_output(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct app_vn310_state_t *app_state = context;
	struct driver_vn310_state_t *state = &app_state->driver_state;

	if (strcmp(argv[2], "freq") == 0)
	{
		int freq = atoi(argv[3]);
		if (freq == 1   ||
			freq == 2  ||
			freq == 4  ||
			freq == 5  ||
		    freq == 10 ||
			freq == 20 ||
			freq == 25 ||
			freq == 40 ||
		    freq == 50||
			freq == 100 ||
		    freq == 200)
		{
			driver_vn310_set_output_data_freq(state, (uint8_t)freq);

			return OK;
		}
        else
		{
			cli_printf_line(cli_state, "Usage: vn310 output freq <1/ 2/ 4/ 5/ 10/ 20/ 25/ 40/ 50/ 100/ 200>");
		}
	}
	if(strcmp(argv[2], "pause") == 0)
	{
		driver_vn310_output_pause(state);
		return OK;
	}
	if (strcmp(argv[2], "enable") == 0)
	{
        if (app_state->config.pri_r_en_l.port && app_state->config.sec_r_en_l.port)
        {
            bsp_gpio_write(&app_state->config.pri_r_en_l, 0);
            bsp_gpio_write(&app_state->config.pri_d_en, 1);
            bsp_gpio_write(&app_state->config.sec_r_en_l, 0);
            bsp_gpio_write(&app_state->config.sec_d_en, 1);
        }
		driver_vn310_output_enable_port_1(state);
		return OK;
	}
	else if (strcmp(argv[2], "disable") == 0)
    {
        //turn off RS422 drivers for low power shutdown state
        if (app_state->config.pri_r_en_l.port && app_state->config.sec_r_en_l.port)
        {
            bsp_gpio_write(&app_state->config.pri_r_en_l, 1);
            bsp_gpio_write(&app_state->config.pri_d_en, 0);
            bsp_gpio_write(&app_state->config.sec_r_en_l, 1);
            bsp_gpio_write(&app_state->config.sec_d_en, 0);
        }
    }
	if(strcmp(argv[2], "async") == 0)
	{
		driver_vn310_set_asynchronous_output(state, argv[3]);

		return OK;
	}

	return ERROR;


}

/**
 * @brief CLI command to start or stop streaming data from the vn310 device.
 *
 * @param cli_state The state of the CLI.
 * @param context The context for the command.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return int The status of the command execution.
 */
static STATUS vn310_cli(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
	struct driver_vn310_state_t *state = context;
	if(strcmp(argv[2], "stream") == 0)
	{
		if(strcmp(argv[3], "start") == 0)
		{
			state->uart_stream = true;
            state->response_expected = false;
			return OK;
		}
		if(strcmp(argv[3], "stop") == 0)
		{
			state->uart_stream = false;
            state->response_expected = false;
			return OK;
		}
		if(strcmp(argv[3], "single") == 0)
		{
            state->uart_stream = false;
            state->response_expected = true;
			return OK;
		}
	}

    if (strcmp(argv[2], "pose_stream") == 0)
    {
        if (strcmp(argv[3], "start") == 0)
        {
            state->pose_stream = true;
            state->response_expected = false;
			return OK;
        }
        if (strcmp(argv[3], "stop") == 0)
        {
            state->pose_stream = false;
            state->response_expected = false;
			return OK;
        }
    }

	return ERROR;
}
/**
 * @brief Configures the vn310 device based on the specified configuration number.
 *
 * This function configures the vn310 device based on the specified configuration number.
 *
 * @param cli_state Pointer to the CLI state structure.
 * @param context Pointer to the vn310 driver state structure.
 * @param argc Number of arguments.
 * @param argv Array of argument strings.
 * @return Status of the operation (success or failure).
 */
static STATUS vn310_config(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct driver_vn310_state_t *state = context;

    if (strcmp(argv[3], "0") == 0)
    {
        bsp_delay_ms(2);
        if (driver_vn310_set_asynchronous_output(state, "0") != OK)
		{
			return ERROR;
		}

        bsp_delay_ms(4);
        if (driver_vn310_set_vn310_baud_rate(state, 115200) != OK)
		{
			return ERROR;
		}

        bsp_delay_ms(4);
        if (driver_vn310_set_uart_baud_rate(state, 115200) != OK)
		{
			return ERROR;
		}

        bsp_delay_ms(4);
        if (driver_vn310_set_configuration_0(state) != OK)
		{
			return ERROR;
		}

        return OK;
    }

    return ERROR;
}

/**
 * @brief CLI command to reset the vn310 device to factory settings.
 *
 * @param cli_state The state of the CLI.
 * @param context The context for the command.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return int The status of the command execution.
 */
static STATUS vn310_settings(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
	struct driver_vn310_state_t *state = context;

	if (strcmp(argv[2], "write") == 0)
	{
		return driver_vn310_write_settings(state);
	}
	if (strcmp(argv[2], "config") == 0)
	{
		return vn310_config(cli_state, context, argc, argv);
	}
	if (strcmp(argv[2], "device") == 0)
	{
		if (strcmp(argv[3], "baud") == 0)
		{
			int baud_rate = atoi(argv[4]);

			if (baud_rate == 9600   ||
				baud_rate == 19200  ||
				baud_rate == 38400  ||
				baud_rate == 57600  ||
				baud_rate == 115200 ||
				baud_rate == 128000 ||
				baud_rate == 230400 ||
				baud_rate == 460800)
			{
				return driver_vn310_set_vn310_baud_rate(state, baud_rate);
			}
		}
		if (strcmp(argv[3], "reset") == 0)
		{
			return driver_vn310_reset_device(state);
		}
	}
	if (strcmp(argv[2], "uart") == 0)
	{
		if (strcmp(argv[3], "baud") == 0)
		{
			int baud_rate = atoi(argv[4]);

			if (baud_rate == 9600   ||
				baud_rate == 19200  ||
				baud_rate == 38400  ||
				baud_rate == 57600  ||
				baud_rate == 115200 ||
				baud_rate == 128000 ||
				baud_rate == 230400 ||
				baud_rate == 460800)
			{
				return driver_vn310_set_uart_baud_rate(state, baud_rate);
			}
		}
	}
	if (strcmp(argv[2], "factory") == 0)
	{
		if (strcmp(argv[3], "reset") == 0)
		{
			return driver_vn310_factory_settings(state);
		}
	}
	if (strcmp(argv[2], "set") == 0)
	{
		if (strcmp(argv[3], "ant") == 0)
		{
			struct driver_vn310_state_t *state = context;
			double x_pos = strtod(argv[5], NULL);
			double y_pos = strtod(argv[6], NULL);
			double z_pos = strtod(argv[7], NULL);

			if(strcmp(argv[4], "a") == 0)
			{
				return driver_vn310_set_antenna_a(state, x_pos, y_pos, z_pos);
			}
			else if(strcmp(argv[4], "b") == 0)
			{
                double x_uncert = strtod(argv[8], NULL);
                double y_uncert = strtod(argv[9], NULL);
                double z_uncert = strtod(argv[10], NULL);
				return driver_vn310_set_antenna_baseline(state, x_pos, y_pos, z_pos, x_uncert, y_uncert, z_uncert);
			}else
			{
				return ERROR;
			}
		}
	}

	return ERROR;
}

/**
 * @brief CLI command to read the vn310 device.
 *
 * @param cli_state The state of the CLI.
 * @param context The context for the command.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return Status of the operation (success or failure).
 */
static STATUS vn310_read(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
	struct driver_vn310_state_t *state = context;

	if(strcmp(argv[2], "model_number") == 0)
	{
        state->response_expected = true;
		return driver_vn310_read_model_number(state);
	}
	if(strcmp(argv[2], "hardware_revision") == 0)
	{
        state->response_expected = true;
		return driver_vn310_read_hardware_revision(state);
	}
	if(strcmp(argv[2], "serial_number") == 0)
	{
        state->response_expected = true;
		return driver_vn310_read_serial_number(state);
	}
	if(strcmp(argv[2], "firmware_version") == 0)
	{
        state->response_expected = true;
		return driver_vn310_read_firmware_version(state);
	}

	return ERROR;
}

/**
 * @brief CLI command to read or write to a specified the register.
 *
 * @param cli_state The state of the CLI.
 * @param context The context for the command.
 * @param argc The number of arguments.
 * @param argv The arguments.
 * @return Status of the operation (success or failure).
 */
static STATUS vn310_register(struct cli_state_t *cli_state, void *context, int argc, char const *argv[]) //todo fix this cli return codes are not STATUS.
{
    struct driver_vn310_state_t *state = context;

    if (strcmp(argv[2], "read") == 0)
    {
        if (argc < 4)
        {
            cli_printf_line(cli_state, "Usage: vn310 read <register_id>");
            return ERROR;
        }
        state->response_expected = true;
        int register_id = atoi(argv[3]);
        return driver_vn310_read_register(state, register_id);
    }
    if (strcmp(argv[2], "write") == 0)
    {
        if (argc < 5)
        {
            cli_printf_line(cli_state, "Usage: vn310 write <register_id> <data...>");
            return ERROR;
        }
        int register_id = atoi(argv[3]);
        uint8_t data[argc - 4];
        for (int i = 0; i < argc - 4; ++i)
        {
            data[i] = atoi(argv[i + 4]);
        }
        return driver_vn310_write_register(state, register_id, data, argc - 4); // Corrected indexing
    }

    return ERROR;
}

static int vn310_power(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct app_vn310_state_t *state = context;

    if (argc != 3)
        return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;

    if (strcmp(argv[2], "on") == 0)
    {
        cli_printf(cli_state, "Powering vn310 on\n");
        bsp_gpio_write(&state->config.power_enable, 1);
    }
    else if (strcmp(argv[2], "off") == 0)
    {
        cli_printf(cli_state, "Powering vn310 off\n");
        bsp_gpio_write(&state->config.power_enable, 0);

    } else {
        cli_printf(cli_state, "Usage: vn310 power <on|off>\n");
        return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;
    }
    return CLI_COMMAND_RETURN_CODE_OK;
}

static int vn310_override(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct app_vn310_state_t *state = context;

    if (strcmp(argv[2], "pose") == 0)
    {
        if (argc != 6)
           return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;

        double yaw = strtod(argv[3], NULL);
        double pitch = strtod(argv[4], NULL);
        double roll = strtod(argv[5], NULL);

        //todo check limits

        pose.yaw = yaw;
        pose.pitch = pitch;
        pose.roll = roll;

        cli_printf(cli_state, "Yaw: %0.3f Pitch: %0.3f Roll: %0.3f\n",  pose.yaw, pose.pitch, pose.roll);
        send_updated_pose(state, &pose, true);
    }
    else if (strcmp(argv[2], "loc") == 0)
    {
        if (argc != 5)
           return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;

        double lat = strtod(argv[3], NULL);
        double lng = strtod(argv[4], NULL);

        //todo check limits

        pose.latitude = lat;
        pose.longitude = lng;

        cli_printf(cli_state, "Lat: %0.3f Lng: %0.3f\n",  pose.latitude, pose.longitude);
        send_updated_pose(state, &pose, true);

    } else {
        return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;
    }
    return CLI_COMMAND_RETURN_CODE_OK;
}

static int vn310_feed(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct app_vn310_state_t *state = context;

    if (strcmp(argv[2], "on") == 0)
    {
        // if (argc != 6)
        //    return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;

        state->driver_state.send_pose = true;
    }
    else if (strcmp(argv[2], "off") == 0)
    {
        // if (argc != 5)
        //    return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;

        state->driver_state.send_pose = false;

    } else {
        return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;
    }
    return CLI_COMMAND_RETURN_CODE_OK;
}

static int vn310_set(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct driver_vn310_state_t *state = context;

    if (strcmp(argv[2], "heading") == 0)
    {
         if (argc != 4)
            return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;

        double heading = strtod(argv[3], NULL);
        return driver_vn310_set_initial_heading(state, heading);
    }
    else
    {
        return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;
    }
}


/**
 * @brief Handles vn310-related CLI commands.
 *
 * This function handles various vn310-related CLI commands.
 *
 * @param cli_state Pointer to the CLI state structure.
 * @param context Pointer to the vn310 driver state structure.
 * @param argc Number of arguments.
 * @param argv Array of argument strings.
 * @return Status of the operation (success or failure).
 */
static int cli_vn310(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct app_vn310_state_t *state = (struct app_vn310_state_t *)context;

    if (argc < 2)
    {
        return ERROR;
    }

	if (strcmp(argv[1], "help") == 0)
	{
		cli_printf_line(cli_state, "");
		cli_printf_line(cli_state, "--------------------");
		cli_printf_line(cli_state, "vn310 300/310");
		cli_printf_line(cli_state, "--------------------");
		cli_printf_line(cli_state, "Steps to using the vn310:");
		cli_printf_line(cli_state, "1. Enable power on VNAB (vn310 power on) ");
		cli_printf_line(cli_state, "2. Enable output on vn310 (vn310 output enable) ");
		cli_printf_line(cli_state, "");

		cli_printf_line(cli_state, "All parameters:");
		cli_printf_line(cli_state, "");
	        cli_printf_line(cli_state, "vn310 cli stream start                    : print data to the cli");
	        cli_printf_line(cli_state, "vn310 cli stream stop                     : stop printing data to the cli");
	        cli_printf_line(cli_state, "vn310 cli stream single                   : print a single piece of data to the cli");
	        cli_printf_line(cli_state, "vn310 cli pose_stream start               : print pose information to the cli");
	        cli_printf_line(cli_state, "vn310 cli pose_stream stop                : stop printing pose to the cli");
	        cli_printf_line(cli_state, "vn310 output freq <freq>                  : set output freq of async data");
	        cli_printf_line(cli_state, "vn310 output async <output setting>       : set output to a specified asynchronous setting, 0 for async off.");
	        cli_printf_line(cli_state, "vn310 output <enable|disable|pause>       : enable or disable or pause device output");
	        cli_printf_line(cli_state, "vn310 read firmware_version               : read the firmware version");
	        cli_printf_line(cli_state, "vn310 read hardware_revision              : read the hardware revision");
	        cli_printf_line(cli_state, "vn310 read model_number                   : read the model number");
	        cli_printf_line(cli_state, "vn310 read serial_number                  : read the serial number");
	        cli_printf_line(cli_state, "vn310 register read <register_id>         : read the value of a register");
	        cli_printf_line(cli_state, "vn310 register write <register_id> <data> : write data to a register");
	        cli_printf_line(cli_state, "vn310 settings config <config number>     : Enable specified device configuration");
	        cli_printf_line(cli_state, "vn310 settings device reset               : reset the device");
	        cli_printf_line(cli_state, "vn310 settings <device|uart> baud         : set device or uart baud rate");
	        cli_printf_line(cli_state, "vn310 settings factory reset              : reset the settings to factory");
	        cli_printf_line(cli_state, "vn310 power <on|off>                      : power on or off the VN board");
	        cli_printf_line(cli_state, "vn310 settings set ant a <X> <Y> <Z>      : set antenna A position relative to vn310");
	        cli_printf_line(cli_state, "vn310 settings set ant b <X> <Y> <Z> <X_uncert> <Y_uncert> <Z_uncert>: set Baseline offset relative to vn310");
	        cli_printf_line(cli_state, "vn310 settings write                      : write the current register settings to NVM");
	        cli_printf_line(cli_state, "vn310 set heading <yaw>                   : sets the initial heading [-180..180] (volatile)");
	        cli_printf_line(cli_state, "vn310 override pose <yaw> <pitch> <roll>  : temporary set pose until next VN update");
	        cli_printf_line(cli_state, "vn310 override loc <lat> <lng>            : temporary set location until next VN update");
	        cli_printf_line(cli_state, "vn310 feed <on|off>                       : enables live vn310 feed through to ACON INT");

		return OK;
	}
	if (strcmp(argv[1], "cli") == 0)
	{
		return vn310_cli(cli_state, &state->driver_state, argc, argv); //todo fix this should pass in app state not driver...
	}
	if (strcmp(argv[1], "output") == 0)
	{
		return vn310_set_output(cli_state, context, argc, argv);
	}
	if (strcmp(argv[1], "settings") == 0)
    {
		return vn310_settings(cli_state, &state->driver_state, argc, argv);
    }
	if (strcmp(argv[1], "register") == 0)
    {
		return vn310_register(cli_state, &state->driver_state, argc, argv);
    }
    if (strcmp(argv[1], "read") == 0)
    {
        return vn310_read(cli_state, &state->driver_state, argc, argv);
    }
    if (strcmp(argv[1], "power") == 0)
    {
        return vn310_power(cli_state, context, argc, argv);
    }
    if (strcmp(argv[1], "override") == 0)
    {
        return vn310_override(cli_state, context, argc, argv);
    }
    if (strcmp(argv[1], "feed") == 0)
    {
        return vn310_feed(cli_state, context, argc, argv);
    }
    if (strcmp(argv[1], "set") == 0)
    {
        return vn310_set(cli_state, &state->driver_state, argc, argv);
    }

    return ERROR;
}

/**
 * @brief Initialize the vn310 app.
 *
 * @param vn310_config The configuration for the vn310 driver.
 * @param vn310_state The state of the vn310 driver.
 * @param cli_state The current state of the command line interface.
 * @return OK if the initialization was successful.
 */
STATUS app_vn310_start(struct app_vn310_state_t *state)
{
    cli_add_command(state->config.cli_state, "vn310","vn310 commands",cli_vn310, &state->driver_state);

    RETURN_ON_ERROR(bsp_gpio_init(&state->config.power_enable, BSP_GPIO_MODE_PUSH_PULL));
    if (state->config.pri_r_en_l.port && state->config.sec_r_en_l.port)
    {
        RETURN_ON_ERROR(bsp_gpio_init(&state->config.pri_r_en_l, BSP_GPIO_MODE_PUSH_PULL));
        RETURN_ON_ERROR(bsp_gpio_init(&state->config.pri_d_en, BSP_GPIO_MODE_PUSH_PULL));
        RETURN_ON_ERROR(bsp_gpio_init(&state->config.sec_r_en_l, BSP_GPIO_MODE_PUSH_PULL));
        RETURN_ON_ERROR(bsp_gpio_init(&state->config.sec_d_en, BSP_GPIO_MODE_PUSH_PULL));
    }

    RETURN_ON_ERROR(driver_vn310_init(&state->driver_state, &state->config.driver_config));
    RETURN_ON_ERROR(driver_vn310_configure(&state->driver_state));

    //start with RS422 drivers disabled for low power shutdown mode

    if (state->config.pri_r_en_l.port && state->config.sec_r_en_l.port)
    {
        RETURN_ON_ERROR(bsp_gpio_write(&state->config.pri_r_en_l, 1));
        RETURN_ON_ERROR(bsp_gpio_write(&state->config.pri_d_en, 0));
        RETURN_ON_ERROR(bsp_gpio_write(&state->config.sec_r_en_l, 1));
        RETURN_ON_ERROR(bsp_gpio_write(&state->config.sec_d_en, 0));
    }

    return OK;
}
