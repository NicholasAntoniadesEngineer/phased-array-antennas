/**
 * @file driver_vectornav.c
 * @brief Implementation of the VectorNav driver functions.
 * 
 * This file contains the functions necessary to initialize, configure, and communicate with the VectorNav sensor.
 * It includes initialization, configuration, and callback functions, as well as functions for sending and receiving data
 * via UART, setting sensor configurations, and reading/writing sensor registers.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 *
 */

#include <string.h>
#include <stdio.h>

#include "console_commands.h"
#include "vn310.h"
#include "driver_uart.h"
    
/**
 * @brief Initialize the VectorNav driver.
 * 
 * @param state The state of the VectorNav driver.
 * @param config The configuration for the VectorNav driver.
 * @return STATUS The status of the initialization.
 */
STATUS driver_vectornav_init(struct driver_vectornav_state_t *state, const struct driver_vectornav_config_t *config)
{
    state->config = *config;

    RETURN_ON_ERROR(driver_uart_init(&state->uart_state, &state->config.vectornav_uart_config));

    return OK;
}

/**
 * @brief Configure the VectorNav driver.
 * 
 * @param state The state of the VectorNav driver.
 * @return STATUS The status of the configuration.
 */
STATUS driver_vectornav_configure(struct driver_vectornav_state_t *state)
{
    state->vectornav_message_ready = false;
    state->message_counter = 0;

    return OK;
}

/**
 * @brief Callback function for secondary VectorNav driver events.
 *
 * This function is called when an event occurs in the secondary VectorNav driver.
 * It assembles the received message and updates the state of the VectorNav driver accordingly.
 *
 * @param vectornav_state The state of the VectorNav driver.
 * @param uart_assembled_message The assembled message from the UART.
 * @param message_size The size of the received message.
 */
STATUS driver_vectornav_eventcallback(struct driver_vectornav_state_t *vectornav_driver_state, uint16_t message_size)
{
	char *uart_received_data = (char*) vectornav_driver_state->uart_state.config.rx_buf;

	enum vectornav_msg_type recieved_msg_type = driver_vectornav_message_check(uart_received_data, vectornav_driver_state->assembled_message, message_size,UART_DMA_READ_BUF_SIZE);

	//check message type
	if (MSG_ASYNC == recieved_msg_type || MSG_BINARY == recieved_msg_type)
	{
		vectornav_driver_state->vectornav_message_ready = true;
		return OK;
	}

    // Clear rx_buf
    memset(vectornav_driver_state->uart_state.config.rx_buf, 0, UART_DMA_READ_BUF_SIZE);

	return ERROR;
}

/**
 * @brief Print the stream from the VectorNav driver.
 *
 * @param state The state of the VectorNav driver.
 * @param cli_state The state of the CLI.
 * @return STATUS The status of the print operation.
 */
STATUS driver_vectornav_print_stream(struct driver_vectornav_state_t *state, struct cli_state_t *cli_state)
{
	if(true == state->uart_stream )
	{
		cli_printf(cli_state, state->assembled_message);
		memset(state->assembled_message, 0, UART_DMA_READ_BUF_SIZE);
		return OK;
	}

    return ERROR;
}

/**
 * @brief Assemble a circular message from the VectorNav driver.
 * 
 * @param state The state of the UART driver.
 * @param received_data The data received from the UART.
 * @param assembled_data The assembled data.
 * @param recieved_message_size The size of the received message.
 * @param uart_dma_buffer_size The size of the UART DMA buffer.
 * @return STATUS The status of the message assembly.
 */
enum vectornav_msg_type driver_vectornav_message_check(char *received_data, char *assembled_data, uint16_t recieved_message_size, uint16_t uart_dma_buffer_size)
{
	// todo: there should be a crc check 

	// Async message header
	if (received_data[0] == '$' && received_data[1] == 'V' && received_data[2] == 'N')
	{
	    // INS header
	    if (received_data[3] == 'I' && received_data[4] == 'N' && received_data[5] == 'S')
	    {
	        memset(assembled_data, 0, uart_dma_buffer_size);
	        memcpy(assembled_data, received_data, recieved_message_size);

	        return MSG_ASYNC;
	    }

	    // Error header
	    if (received_data[3] == 'E' && received_data[4] == 'R' && received_data[5] == 'R')
	    {
	        memset(assembled_data, 0, uart_dma_buffer_size);
	        memcpy(assembled_data, received_data, recieved_message_size);

	        return MSG_ERROR;
	    }
	}

	// config 0 binary message header
	if (received_data[1] == 0xFA && received_data[2] == 0x16 && received_data[3] == 0x3)
	{
		memset(assembled_data, 0, uart_dma_buffer_size);
		memcpy(assembled_data, received_data, recieved_message_size);

		return MSG_BINARY;
	}

	return MSG_ERROR;
}

/**
 * @brief Send a command to the VectorNav driver.
 * 
 * @param state The state of the UART driver.
 * @param data The data to be sent.
 * @param data_size The size of the data.
 * @return STATUS The status of the send operation.
 */
STATUS driver_vectornav_send_byte(struct driver_vectornav_state_t *state, uint8_t *data, size_t data_size)
{
    return driver_uart_transmit(&state->uart_state, data, data_size);
}

/**
 * @brief Read a byte from the VectorNav driver.
 * 
 * @param state The state of the UART driver.
 * @param data The data to be read.
 * @return STATUS The status of the read operation.
 */
STATUS driver_vectornav_read_byte(struct driver_vectornav_state_t *state, uint8_t *data)
{
	driver_uart_read_byte(&state->uart_state, data);

	return OK;
}

/*
* Measurement #1:
*
* GNSS Antenna A Offset register in the GNSS subsystem section
*
* $VNWRG,57,x_cordinate,y_cordinate,z_cordinate*XX
*
* Measurement #2:
*
* $VNWRG,93,1.5,0,0,0.038,0.038,0.038*XX
* The above command writes the values {+1.5, 0, 0} to the GNSS Compass Baseline Register
* in the GNSS subsystem section along with the uncertainties values of {0.038, 0.038, 0.038}.
*
* In this example we have scaled the default uncertainty values of {0.0254, 0.0254, 0.0254}
* by a factor of 1.5 to adjust for the longer baseline length.
* It is highly recommended that this type of scaling be performed for any baseline longer than the default of 1.0m.
*
*/
STATUS driver_vectornav_set_antenna_a(struct driver_vectornav_state_t *state, double x_cordinate, double y_cordinate, double z_cordinate)
{
	//	$VNWRG,57,x_cordinate,y_cordinate,z_cordinate*XX????
	return OK;
}
STATUS driver_vectornav_set_antenna_b(struct driver_vectornav_state_t *state, double x_cordinate, double y_cordinate, double z_cordinate)
{
	//	$VNWRG,57,x_cordinate,y_cordinate,z_cordinate*XX????

	return OK;
}

/**
 * @brief Send a factory settings command to the VectorNav driver.
 * 
 * @param state The state of the UART driver.
 * @return STATUS The status of the command operation.
 */
STATUS driver_vectornav_factory_settings(struct driver_vectornav_state_t *state)
{
    char command[32];
    snprintf(command, sizeof(command), "%s%s*%s%s",VECTORNAV_HEADER ,VECTORNAV_RESET_FS_CMD, VECTORNAV_NO_CRC, VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    return driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
}

/**
 * @brief Send a reset device command to the VectorNav driver.
 * 
 * @param state The state of the UART driver.
 * @return STATUS The status of the command operation.
 */
STATUS driver_vectornav_reset_device(struct driver_vectornav_state_t *state)
{
    char command[32];
    snprintf(command, sizeof(command), "%s%s*%s%s",VECTORNAV_HEADER ,VECTORNAV_RESET_CMD, VECTORNAV_NO_CRC, VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    return driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
}

/**
 * @brief Sets the output data freq for the VectorNav device.
 *
 * This function sends a command to the VectorNav device to set the output data freq.
 * Setting this to 0, disables the output
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @param freq Desired output data freq.
 * @return Status of the operation (success or failure).
 * @retval STATUS_SUCCESS If the command is sent successfully and acknowledged.
 * @retval STATUS_ERROR If an error occurs during command transmission or if the command is not acknowledged.
 */
STATUS driver_vectornav_set_output_data_freq(struct driver_vectornav_state_t *state, uint8_t data_freq)
{
    char command[32];
    snprintf(command, sizeof(command), "%s%s,%d,%d*%s%s",
    		VECTORNAV_HEADER,
			VECTORNAV_WRG_CMD,
			ASYNC_DATA_OUTPUT_FREQUENCY_REGISTER,
			data_freq,
			VECTORNAV_NO_CRC,
			VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    return driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
}

/**
 * @brief Sets the vectornav baud rate for the VectorNav device.
 *
 * This function sends a command to the VectorNav device to set the output baud rate.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @param baud_rate Desired output baud rate.
 * @return Status of the operation (success or failure).
 * @retval STATUS_SUCCESS If the command is sent successfully and acknowledged.
 * @retval STATUS_ERROR If an error occurs during command transmission or if the command is not acknowledged.
 */
STATUS driver_vectornav_set_vectoranv_baud_rate(struct driver_vectornav_state_t *state, unsigned int baud_rate)
{
    char command[32];
    snprintf(command, sizeof(command), "%s%s,%d,%u*%s%s",
    		VECTORNAV_HEADER,
			VECTORNAV_WRG_CMD,
			SERIAL_BAUD_RATE_REGISTER,
			baud_rate,
			VECTORNAV_NO_CRC,
			VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    return driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
}

/**
 * @brief Sets the uart baud rate for the uart connected to the vectornav device.
 *
 * This function sends a command to the VectorNav device to set the output baud rate.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @param baud_rate Desired output baud rate.
 * @return Status of the operation (success or failure).
 * @retval STATUS_SUCCESS If the command is sent successfully and acknowledged.
 * @retval STATUS_ERROR If an error occurs during command transmission or if the command is not acknowledged.
 */
STATUS driver_vectornav_set_uart_baud_rate(struct driver_vectornav_state_t *state, unsigned int baud_rate)
{
	return driver_uart_set_baud_rate(&state->uart_state, baud_rate, (uint8_t *)state->assembled_message, UART_DMA_READ_BUF_SIZE);
}


/**
 * @brief Sets configuration for the VectorNav sensor.
 *
 * Configures the VectorNav sensor driver using predefined settings. The function
 * defines output groups and their respective fields, and sends a formatted command
 * to the sensor. It configures time, IMU, and attitude data groups with specific 
 * field selections.
 * 
 * Output groups and fields:
 * - Time Group		: TimeGpsPps (0x0001)
 * - IMU Group		: Accel, AngularRate (0x0600)
 * - Attitude Group	: YawPitchRoll, Quaternion (0x0006)
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @return STATUS indicating the success or failure of the configuration operation.
 */
STATUS driver_vectornav_set_configuration_0(struct driver_vectornav_state_t *state) 
{
    int output_group 	 	 = 0x0012;	// Group selection: 0b00000010010 0x0016: Time Group, IMU Group, Attitude Group
    int group_field_1 		 = 0x0003;	// Time Group:	    0b00000000001 0x0001: TimeGpsPps
    int group_field_4 		 = 0x0006;  // Attitude Group:  0b00000000110 0x0006: YawPitchRoll, Quaternion

    char command[64]; 
    snprintf(command, sizeof(command), "%s%s,%d,%d,%d,%x,%x,%x*%s%s",
		VECTORNAV_HEADER,
		VECTORNAV_WRG_CMD,
		BINARY_OUTPUT_REGISTER_1,
		ASYNC_MODE_PORT_1,
		RATE_DIVISOR_4,
		output_group,
		group_field_1,
		group_field_4,
		VECTORNAV_NO_CRC,
		VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    return driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
}


/**
 * @brief Configure the asynchronous to a specific output setting
 * 
 * This function configures the VectorNav driver with the provided settings.
 * 
 * @param state Pointer to the VectorNav driver state structure.
 * @param setting Pointer to the setting value. 
 * @return STATUS indicating the success or failure of the operation.
 */
STATUS driver_vectornav_set_asynchronous_output(struct driver_vectornav_state_t *state, char const *setting)
{
    char command[64]; 
    snprintf(command, sizeof(command), "%s%s,%d,%s*%s%s",
		VECTORNAV_HEADER,
		VECTORNAV_WRG_CMD,
		ASYNC_DATA_OUTPUT_TYPE_REGISTER,
		setting,
		VECTORNAV_NO_CRC,
		VECTORNAV_CRLF);
 
    size_t command_size = strlen(command);

    return driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
}

/**
 * @brief Calculates the 8-bit checksum for the given byte sequence.
 *
 * This function from the Vectornav user manual calculates the checksum
 * for the provided byte sequence, including comma delimiters in the calculation.
 * The resultant checksum is represented as two hexadecimal characters in the command.
 *
 * @param data The byte sequence for which the checksum is to be calculated.
 * @param length The length of the byte sequence.
 * @return The calculated checksum as an unsigned char (8-bit number).
 */
unsigned char calculate_8_bit_crc(unsigned char data[], unsigned int length)
{
	unsigned int i;
	unsigned char crc = 0;

	for(i=0; i<length; i++){
		crc ^= data[i];
	}

	return crc;
}

/**
 * @brief Calculates the 16-bit CRC for the given ASCII or binary message.
 *
 * This function from the Vectornav user manual calculates the CRC16-CCITT checksum for the provided byte sequence,
 * which provides enhanced error detection compared to an 8-bit checksum. The CRC is
 * represented as four hexadecimal characters in the command.
 *
 * @param data The byte sequence for which the CRC is to be calculated.
 * @param length The length of the byte sequence.
 * @return The calculated CRC as an unsigned short (16-bit number).
 */
unsigned short calculate_16_bit_crc(unsigned char data[], unsigned int length)
{
	unsigned int i;
	unsigned short crc = 0;

	for(i=0; i<length; i++){
		crc = (unsigned char)(crc >> 8) | (crc << 8);
		crc ^= data[i];
		crc ^= (unsigned char)(crc & 0xff) >> 4;
		crc ^= crc << 12;
		crc ^= (crc & 0x00ff) << 5;
	}

	return crc;
}

/**
 * @brief Read the value of a register
 *
 * This function sends a command to read the value of a specified register
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @param register_id The ID of the register to be read.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_read_register(struct driver_vectornav_state_t *state, enum vectornav_register_id register_id) 
{
    char command[32];
    snprintf(command, sizeof(command), "%s%s,%d*%s%s",VECTORNAV_HEADER,VECTORNAV_RRG_CMD, register_id, VECTORNAV_NO_CRC, VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    STATUS status = driver_vectornav_send_byte(state, (uint8_t*)command, command_size);

    return status;
}

/**
 * @brief Write data values to a specified register on the VN-310 device.
 *
 * This function sends a command to write data values to a specified register.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @param register_id The ID of the register to be written to.
 * @param data Pointer to the data values to be written.
 * @param data_size Size of the data to be written.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_write_register(struct driver_vectornav_state_t *state, enum vectornav_register_id register_id, const uint8_t *data, size_t data_size) 
{
    char command[32 + 2 * data_size];

    snprintf(command, sizeof(command), "%s%s,%d,%d*%s%s",VECTORNAV_HEADER, VECTORNAV_WRG_CMD, register_id, data[0], VECTORNAV_NO_CRC, VECTORNAV_CRLF);
    size_t command_size = strlen(command);

    STATUS status = driver_vectornav_send_byte(state, (uint8_t*)command, command_size);

    return status;
}

/**
 * @brief Writes the current register settings into non-volatile memory.
 *
 * This function sends a command to the VectorNav device to write the current register settings into non-volatile memory.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_write_settings(struct driver_vectornav_state_t *state) {

    char command[32];
    snprintf(command, sizeof(command), "%s%s*%s%s",VECTORNAV_HEADER,VECTORNAV_WRITE_SETTINGS_CMD, VECTORNAV_NO_CRC, VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    STATUS status = driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
    
    return status;
}

/**
 * @brief Pauses asynchronous output messages on the VectorNav device.
 *
 * This function sends a command to the VectorNav device to temporarily pause the asynchronous output messages.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_output_pause(struct driver_vectornav_state_t *state) 
{
    char command[32];
    snprintf(command, sizeof(command), "%s%s,%d*%s%s",
		VECTORNAV_HEADER,
		VECTORNAV_ASYNC_CMD, 
		ASYNC_MODE_NONE, 
		VECTORNAV_NO_CRC, 
		VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    STATUS status = driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
    return status;
}

/**
 * @brief Resumes asynchronous output messages on the VectorNav device.
 *
 * This function sends a command to the VectorNav device to resume the asynchronous output messages.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_output_enable_port_1(struct driver_vectornav_state_t *state)
{
    char command[32];
    snprintf(command, sizeof(command), "%s%s,%d*%s%s",VECTORNAV_HEADER, VECTORNAV_ASYNC_CMD, ASYNC_MODE_PORT_1, VECTORNAV_NO_CRC, VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    STATUS status = driver_vectornav_send_byte(state, (uint8_t*)command, command_size);
    return status;
}

/**
 * @brief Polls the sensor measurements available in the binary output protocol.
 *
 * This function sends a command to the VectorNav device to poll the sensor measurements available in the binary output protocol.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @param register_num The number of the binary output register (1-3) to select the appropriate binary output register.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_binary_output_poll(struct driver_vectornav_state_t *state, uint8_t register_num) 
{
    char command[32];
    snprintf(command, sizeof(command), "%s%s,%d*%s%s",VECTORNAV_HEADER ,VECTORNAV_BOM_CMD , register_num, VECTORNAV_NO_CRC, VECTORNAV_CRLF);

    size_t command_size = strlen(command);

    STATUS status = driver_vectornav_send_byte(state, (uint8_t*)command, command_size);

    return status;
}

/**
 * @brief Reads the model number from the Model Number Register.
 *
 * This function sends a command to the VectorNav device to read the model number from the Model Number Register.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_read_model_number(struct driver_vectornav_state_t *state) 
{
    return driver_vectornav_read_register(state, MODEL_NUMBER_REGISTER);
}

/**
 * @brief Reads the hardware revision from the Hardware Revision Register.
 *
 * This function sends a command to the VectorNav device to read the hardware revision from the Hardware Revision Register.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_read_hardware_revision(struct driver_vectornav_state_t *state) 
{
    return driver_vectornav_read_register(state, HARDWARE_REVISION_REGISTER);
}

/**
 * @brief Reads the serial number from the Serial Number Register.
 *
 * This function sends a command to the VectorNav device to read the serial number from the Serial Number Register.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_read_serial_number(struct driver_vectornav_state_t *state) 
{
    return driver_vectornav_read_register(state, SERIAL_NUMBER_REGISTER);
}

/**
 * @brief Reads the firmware version from the Firmware Version Register.
 *
 * This function sends a command to the VectorNav device to read the firmware version from the Firmware Version Register.
 *
 * @param state Pointer to the VectorNav driver state structure.
 * @return Status of the operation (success or failure).
 */
STATUS driver_vectornav_read_firmware_version(struct driver_vectornav_state_t *state) 
{
    return driver_vectornav_read_register(state, FIRMWARE_VERSION_REGISTER);
}
