/**
 * @file vn310_cli.c
 * @brief Implementation of the VectorNav CLI interface functions.
 * 
 * This file contains the command-line interface functions for interacting with the VectorNav sensor.
 * It includes commands for configuration, streaming data, reading registers, and managing sensor settings.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 */

#include "vn310_cli.h"
#include "vn310_pose.h"
#include "driver_vn310.h"
#include "bsp_delay.h"

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

static STATUS vn310_cli_stream(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
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

static STATUS vn310_settings(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct driver_vn310_state_t *state = context;

    if (strcmp(argv[2], "write") == 0)
    {
        return driver_vn310_write_settings(state);
    }
    if (strcmp(argv[2], "config") == 0)
    {
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
            }
        }
    }

    return ERROR;
}

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

static STATUS vn310_register(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
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
        return driver_vn310_write_register(state, register_id, data, argc - 4);
    }

    return ERROR;
}

static STATUS vn310_power(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
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
    }
    else
    {
        cli_printf(cli_state, "Usage: vn310 power <on|off>\n");
        return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;
    }
    return CLI_COMMAND_RETURN_CODE_OK;
}

static STATUS vn310_override(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct app_vn310_state_t *state = context;

    if (strcmp(argv[2], "pose") == 0)
    {
        if (argc != 6)
           return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;

        double yaw = strtod(argv[3], NULL);
        double pitch = strtod(argv[4], NULL);
        double roll = strtod(argv[5], NULL);

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

        pose.latitude = lat;
        pose.longitude = lng;

        cli_printf(cli_state, "Lat: %0.3f Lng: %0.3f\n",  pose.latitude, pose.longitude);
        send_updated_pose(state, &pose, true);
    }
    else
    {
        return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;
    }
    return CLI_COMMAND_RETURN_CODE_OK;
}

static STATUS vn310_feed(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct app_vn310_state_t *state = context;

    if (strcmp(argv[2], "on") == 0)
    {
        state->driver_state.send_pose = true;
    }
    else if (strcmp(argv[2], "off") == 0)
    {
        state->driver_state.send_pose = false;
    }
    else
    {
        return CLI_COMMAND_RETURN_CODE_INVALID_PARMS;
    }
    return CLI_COMMAND_RETURN_CODE_OK;
}

static STATUS vn310_set(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
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

static void print_help(struct cli_state_t *cli_state)
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
}

static STATUS cli_vn310(struct cli_state_t *cli_state, void *context, int argc, char const *argv[])
{
    struct app_vn310_state_t *state = (struct app_vn310_state_t *)context;

    if (argc < 2)
    {
        return ERROR;
    }

    if (strcmp(argv[1], "help") == 0)
    {
        print_help(cli_state);
        return OK;
    }
    if (strcmp(argv[1], "cli") == 0)
    {
        return vn310_cli_stream(cli_state, &state->driver_state, argc, argv);
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

void vn310_cli_init(struct app_vn310_state_t *state, struct cli_state_t *cli_state)
{
    cli_add_command(cli_state, "vn310", "vn310 commands", cli_vn310, state);
} 