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

//example of s format is
//"$VNINS,125176.941097,2332,8206,+082.014,+000.014,+001.063,+51.51992529,-000.11006359,+00089.216,-000.001,-000.008,-000.125,03.9,01.2,0.10*65"
enum {toknum_insstatus = 3, toknum_yaw, toknum_pitch, toknum_roll, toknum_poslat, toknum_poslon, toknum_posalt};

STATUS vn310_parser_parse_VNINS(const char *s, struct vn310_pose_t *p)
{
    char *t = strtok((char *)s, ",");
    int i = 0;
    while (t)
    {
        switch (i)
        {
            case toknum_insstatus:
                p->ins_status = (uint16_t)strtoul(t, NULL, 16);
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

STATUS vn310_parser_handle_pose_message(const char *s, struct vn310_pose_t *p)
{
    if (0 == strncmp(s, "$VNINS", 6))
    {
        return vn310_parser_parse_VNINS(s, p);
    }
    //todo call parsers for other formats here eg binary

    return ERROR;    //not a handled pose message, but might be another response, doesn't mean corrupted
} 