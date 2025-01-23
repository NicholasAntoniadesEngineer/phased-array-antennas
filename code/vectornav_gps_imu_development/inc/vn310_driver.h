/**
 * @file vn310_driver.h
 * @brief Header file for the VectorNav driver.
 * 
 * This file contains the definitions and function prototypes for the VectorNav driver.
 * It includes configuration structures, enumerations for message types and register IDs,
 * and function declarations for initializing, configuring, and communicating with the 
 * VectorNav sensor.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 *
 */

#pragma once

#include <stdbool.h>
#include "config.h"
#include "driver_uart.h"
#include "console_commands.h"

#define UART_DMA_READ_BUF_SIZE       256
#define VN310_COMMAND_BUFFER_SIZE    100

#define VECTORNAV_HEADER             "$VN"
#define VECTORNAV_ERR                "ERR"
#define VECTORNAV_RESET_CMD          "RST"
#define VECTORNAV_RESET_FS_CMD       "RFS"
#define VECTORNAV_RRG_CMD            "RRG"
#define VECTORNAV_WRG_CMD            "WRG"
#define VECTORNAV_WRITE_SETTINGS_CMD "WNV"
#define VECTORNAV_ASYNC_CMD          "ASY"
#define VECTORNAV_BOM_CMD            "BOM"
#define VECTORNAV_NO_CRC             "XX"
#define VECTORNAV_CRLF               "\r\n"
#define VECTORNAV_SYNC_BYTE          "\xFA"

enum vectornav_msg_type
{
    MSG_ASYNC  = 0,
    MSG_BINARY = 1,  
    MSG_ERROR  = 2, 
};

enum vectornav_async_mode
{
    ASYNC_MODE_NONE       = 0,  // User message is not automatically sent out either serial port
    ASYNC_MODE_PORT_1     = 1,  // Message is sent out serial port 1 at a fixed rate
    ASYNC_MODE_PORT_2     = 2,  // Message is sent out serial port 2 at a fixed rate
    ASYNC_MODE_BOTH_PORTS = 3   // Message is sent out both serial ports at a fixed rate
};

enum vectornav_rate_divisor 
{
    RATE_DIVISOR_1 = 1,
    RATE_DIVISOR_2 = 2,
    RATE_DIVISOR_4 = 4,
    RATE_DIVISOR_8 = 8,
    RATE_DIVISOR_16 = 16,
    RATE_DIVISOR_32 = 32,
    RATE_DIVISOR_64 = 64,
    RATE_DIVISOR_128 = 128
};

enum vectornav_register_id
{
    USER_TAG_REGISTER                                   = 0,
    MODEL_NUMBER_REGISTER                               = 1,
    HARDWARE_REVISION_REGISTER                          = 2,
    SERIAL_NUMBER_REGISTER                              = 3,
    FIRMWARE_VERSION_REGISTER                           = 4,
    SERIAL_BAUD_RATE_REGISTER                           = 5,
    ASYNC_DATA_OUTPUT_TYPE_REGISTER                     = 6,
    ASYNC_DATA_OUTPUT_FREQUENCY_REGISTER                = 7,
    COMMUNICATION_PROTOCOL_CONTROL_REGISTER             = 30,
    SYNCHRONIZATION_CONTROL_REGISTER                    = 32,
    SYNCHRONIZATION_STATUS_REGISTER                     = 33,
    IMU_MEASUREMENTS                                    = 54,
    BINARY_OUTPUT_REGISTER_1                            = 75,
    BINARY_OUTPUT_REGISTER_2                            = 76,
    BINARY_OUTPUT_REGISTER_3                            = 77,
    NMEA_OUTPUT_REGISTER_1                              = 101,
    NMEA_OUTPUT_REGISTER_2                              = 102,
    DELTA_THETA_DELTA_VELOCITY                          = 80,
    MAGNETOMETER_COMPENSATION                           = 23,
    ACCELEROMETER_COMPENSATION                          = 25,
    GYRO_COMPENSATION                                   = 84,
    REFERENCE_FRAME_ROTATION                            = 26,
    IMU_FILTERING_CONFIGURATION                         = 85,
    DELTA_THETA_DELTA_VELOCITY_CONFIGURATION            = 82,
    GNSS_SOLUTION_LLA_REGISTER                          = 58,
    GNSS_SOLUTION_ECEF_REGISTER                         = 59,
    GNSS2_SOLUTION_LLA_REGISTER                         = 103,
    GNSS2_SOLUTION_ECEF_REGISTER                        = 104,
    GNSS_CONFIGURATION_REGISTER                         = 55,
    GNSS_ANTENNA_A_OFFSET_REGISTER                      = 57,
    GNSS_COMPASS_BASELINE_REGISTER                      = 93,
    GNSS_COMPASS_ESTIMATED_BASELINE_REGISTER            = 97,
    GNSS_COMPASS_STARTUP_STATUS_REGISTER                = 98,
    GNSS_COMPASS_SIGNAL_HEALTH_STATUS_REGISTER          = 86,
    NMEA_RMC_REGISTER                                   = 200,
    NMEA_GGA_REGISTER                                   = 201,
    NMEA_GLL_REGISTER                                   = 202,
    NMEA_GSA_REGISTER                                   = 203,
    NMEA_GSV_REGISTER                                   = 204,
    NMEA_HDT_REGISTER                                   = 205,
    NMEA_THS_REGISTER                                   = 206,
    NMEA_VTG_REGISTER                                   = 207,
    NMEA_ZDA_REGISTER                                   = 208,
    NMEA_PASHR_REGISTER                                 = 209,
    SET_GYRO_BIAS_COMMAND                               = 210,
    SET_INITIAL_HEADING_COMMAND                         = 211,
    YAW_PITCH_ROLL                                      = 212,
    ATTITUDE_QUATERNION                                 = 213,
    YAW_PITCH_ROLL_MAGNETIC_ACCELERATION_ANGULAR_RATES  = 214,
    QUATERNION_MAGNETIC_ACCELERATION_ANGULAR_RATES      = 215,
    MAGNETIC_MEASUREMENTS                               = 216,
    ACCELERATION_MEASUREMENTS                           = 217,
    ANGULAR_RATE_MEASUREMENTS                           = 218,
    MAGNETIC_ACCELERATION_ANGULAR_RATES                 = 219,
    YAW_PITCH_ROLL_TRUE_BODY_ACCELERATION_ANGULAR_RATES = 220,
    MAGNETIC_ACCELERATION_ANGULAR_RATES_INERTIAL_FRAME  = 221,
    VPE_BASIC_CONTROL                                   = 222,
    FACTORY_DEFAULTS                                    = 223,
    MAGNETOMETER_CALIBRATION_CONTROL                    = 44,
    CALCULATED_MAGNETOMETER_CALIBRATION                 = 47,
    INS_SOLUTION_LLA                                    = 63,
    INS_SOLUTION_ECEF                                   = 64,
    INS_BASIC_CONFIGURATION                             = 67,
    INS_STATE_LLA                                       = 72,
    INS_STATE_ECEF                                      = 73,
    STARTUP_FILTER_BIAS_ESTIMATE                        = 74

};

struct vn310_driver_config_t
{
    struct driver_uart_config_t vectornav_uart_config;

};

struct vn310_driver_state_t
{
    struct vn310_driver_config_t config;
    char assembled_message[UART_DMA_READ_BUF_SIZE];
    struct driver_uart_state_t uart_state;
    bool uart_stream;
    bool vectornav_message_ready;
    uint8_t message_counter;

};

STATUS vn310_driver_configure(struct vn310_driver_state_t *state);
STATUS vn310_driver_eventcallback(struct vn310_driver_state_t *vectornav_driver_state, uint16_t message_size);
STATUS vn310_driver_init(struct vn310_driver_state_t *state, const struct vn310_driver_config_t *config);
enum vectornav_msg_type vn310_driver_message_check(char *received_data, char *assembled_data, uint16_t recieved_message_size, uint16_t uart_dma_buffer_size);
STATUS vn310_driver_print_stream(struct vn310_driver_state_t *state, struct cli_state_t *cli_state);
STATUS vn310_driver_read_byte(struct vn310_driver_state_t *state, uint8_t *pData);
STATUS vn310_driver_send_byte(struct vn310_driver_state_t *state, uint8_t *data, size_t data_size);
STATUS vn310_driver_write_register(struct vn310_driver_state_t *state, enum vectornav_register_id register_id, const uint8_t *data, size_t data_size);
STATUS vn310_driver_read_register(struct vn310_driver_state_t *state, enum vectornav_register_id register_id);
STATUS vn310_driver_write_settings(struct vn310_driver_state_t *state);
STATUS vn310_driver_factory_settings(struct vn310_driver_state_t *state);
STATUS vn310_driver_reset_device(struct vn310_driver_state_t *state);
STATUS vn310_driver_disable_output(struct vn310_driver_state_t *state);
STATUS vn310_driver_output_pause(struct vn310_driver_state_t *state);
STATUS vn310_driver_output_enable_port_1(struct vn310_driver_state_t *state);
STATUS vn310_driver_set_antenna_a(struct vn310_driver_state_t *state, double x_cordinate, double y_cordinate, double z_cordinate);
STATUS vn310_driver_set_antenna_b(struct vn310_driver_state_t *state, double x_cordinate, double y_cordinate, double z_cordinate);
STATUS vn310_driver_set_configuration_0(struct vn310_driver_state_t *state);
STATUS vn310_driver_set_asynchronous_output(struct vn310_driver_state_t *state, char const *setting);
STATUS vn310_driver_set_output_data_freq(struct vn310_driver_state_t *state, uint8_t data_freq);
STATUS vn310_driver_set_vectoranv_baud_rate(struct vn310_driver_state_t *state, unsigned int baud_rate);
STATUS vn310_driver_set_uart_baud_rate(struct vn310_driver_state_t *state, unsigned int baud_rate);
STATUS vn310_driver_binary_output_poll(struct vn310_driver_state_t *state, uint8_t register_num);
STATUS vn310_driver_read_model_number(struct vn310_driver_state_t *state);
STATUS vn310_driver_read_hardware_revision(struct vn310_driver_state_t *state);
STATUS vn310_driver_read_serial_number(struct vn310_driver_state_t *state);
STATUS vn310_driver_read_firmware_version(struct vn310_driver_state_t *state); 
