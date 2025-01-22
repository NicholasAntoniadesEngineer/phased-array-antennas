#ifndef ARRAY_PATCH_POSITION_CALCULATION_H
#define ARRAY_PATCH_POSITION_CALCULATION_H

#include <stdint.h>

// Status codes
typedef enum {
    OK = 0,
    ERROR = -1
} STATUS;

// Patch pose structure
struct patch_pose_t {
    double t_x;
    double t_y;
};

// Patch structure
struct algorithm_EW_patch_t {
    struct patch_pose_t pose;
    // Add other patch-related fields if needed
};

// Function declarations
STATUS phased_array_calc_patch_pose(
    const uint16_t array_array_col,
    const uint16_t array_array_row,
    const int nx,
    const int ny,
    const double spacing,
    struct algorithm_EW_patch_t *patches);

STATUS phased_array_rot_pos_update(
    const uint16_t array_rotation,
    int nx,
    int ny,
    struct algorithm_EW_patch_t *patches);

STATUS phased_array_init_patches(
    struct algorithm_EW_patch_t *patches,
    const uint16_t array_rotation,
    const uint16_t array_array_col,
    const uint16_t array_array_row,
    const uint16_t number_of_patches_x,
    const uint16_t number_of_patches_y,
    const double patch_spacing);

#endif /* ARRAY_PATCH_POSITION_CALCULATION_H */ 