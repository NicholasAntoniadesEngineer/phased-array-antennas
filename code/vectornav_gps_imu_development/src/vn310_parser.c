/**
 * @file vn310_parser.c
 * @brief Implementation of the VectorNav message parsing functions.
 * 
 * This file contains functions for parsing and processing messages received from
 * the VectorNav sensor, including binary and ASCII message formats.
 * 
 * @author Nicholas Antoniades
 * @date 15 Jan 2024
 */

#include <stdlib.h>
#include <string.h>
#include "vn310_parser.h"

/**
 * VNINS Message Format:
 * $VNINS,<1>,<2>,...,<15>*<16>
 * 
 * Fields:
 * 1.  Time of Week (seconds)
 * 2.  Week Number
 * 3.  INS Status (hex)
 * 4.  Yaw (degrees)
 * 5.  Pitch (degrees)
 * 6.  Roll (degrees)
 * 7.  Latitude (degrees)
 * 8.  Longitude (degrees)
 * 9.  Altitude (meters)
 * 10. Velocity North (m/s)
 * 11. Velocity East (m/s)
 * 12. Velocity Down (m/s)
 * 13. Attitude Uncertainty (degrees)
 * 14. Position Uncertainty (meters)
 * 15. Velocity Uncertainty (m/s)
 * 16. Checksum
 * 
 * Example:
 * "$VNINS,125176.941097,2332,8206,+082.014,+000.014,+001.063,+51.51992529,-000.11006359,+00089.216,-000.001,-000.008,-000.125,03.9,01.2,0.10*65"
 */

enum {toknum_insstatus = 3, toknum_yaw, toknum_pitch, toknum_roll, toknum_poslat, toknum_poslon, toknum_posalt};

STATUS vn310_parser_parse_VNINS(const char *recieved_string, struct vn310_pose_t *vn310_pose)
{
    char *str_token= strtok((char *)recieved_string, ",");
    int i = 0;
    while (t)
    {
        switch (i)
        {
            case toknum_insstatus:
                vn310_pose->ins_status = (uint16_t)strtoul(t, NULL, 16);
                break;
            case toknum_yaw:
                vn310_pose->yaw = strtod(t, NULL);
                break;
            case toknum_pitch:
                vn310_pose->pitch = strtod(t, NULL);
                break;
            case toknum_roll:
                vn310_pose->roll = strtod(t, NULL);
                break;
            case toknum_poslat:
                {
                    float v = strtod(t, NULL);
                    vn310_pose->latitude = v;
                }
                break;
            case toknum_poslon:
                {
                    float v = strtod(t, NULL);
                    vn310_pose->longitude = v;
                }
                break;
            case toknum_posalt:
                //float alt = strtod(t, NULL);
                break;
            default:
                break;
        }
        str_token= strtok(NULL, ",");
        i++;
    }

    if (i > toknum_roll)
    {
        return OK;
    }
    return ERROR;
}

STATUS vn310_parser_handle_pose_message(const char *recieved_string, struct vn310_pose_t *vn310_pose)
{
    if (0 == strncmp(recieved_string, "$VNINS", 6))
    {
        return vn310_parser_parse_VNINS(recieved_string, vn310_pose);
    }

    /* Future enhancement: Add support for additional message formats (e.g., binary) */

    /* Return ERROR to indicate unrecognized message format.
     * Note: This is not necessarily an error condition - the message
     * may be valid but in an unhandled format. */
    return ERROR;
} 