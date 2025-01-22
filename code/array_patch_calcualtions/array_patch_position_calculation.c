/**
 * @file patch_position_calculation.c
 * @brief Calculation and rotation of patch positions within a phased array.
 * 
 * This file contains functions to calculate the positions of patches within a phased array,
 * apply rotations to the array, and initialize the patch positions. The functions are 
 * designed to handle various rotations and spacing configurations.
 * 
 * @author Nicholas Antoniades
 * @date 27 April 2024
 *
 */

#include "array_patch_position_calculation.h"

/**
 * @brief Calculates the patch positions within an array.
 *
 * Given the array position, number of patches in X and Y directions, spacing,
 * and an array of patches, this function computes each patch pos.
 *
 * @param array_array_pos Pointer to the array array position (column and row).
 * @param nx Number of patches in the X direction.
 * @param ny Number of patches in the Y direction.
 * @param spacing Spacing between patches.
 * @param patches Array of patches to be updated with positions.
 * @return OK if successful, an error code otherwise.
 */
STATUS phased_array_calc_patch_pose( const uint16_t array_array_col,
					   const uint16_t array_array_row,
					   const int nx,
					   const int ny,
					   const double spacing,
					   struct algorithm_EW_patch_t *patches)
{
    double array_x_offset = array_array_col * spacing * nx;
    double array_y_offset = array_array_row * spacing * ny;

    for (int y = 0; y < ny; y++) 
    {
        double patch_y_offset = array_y_offset + y * spacing;  

        for (int x = 0; x < nx; x++) 
        {
            const int i = y * nx + x;  
            double patch_x_offset = array_x_offset + x * spacing; 

            patches[i].pose.t_x = patch_x_offset;
            patches[i].pose.t_y = patch_y_offset;
        }
    }

	return OK;
}

/**
 * @brief Updates patch positions based on array rotation.
 *
 * Given an array of patches, the number of patches in X and Y directions, spacing,
 * array rotation angle, and array array position, this function adjusts the patch positions.
 *
 * @param array_rotation Rotation angle of the array (ROT_0, ROT_90, ROT_180, ROT_270).
 * @param nx Number of patches in the X direction.
 * @param ny Number of patches in the Y direction.
 * @param patches Array of patches to be updated.
 * @return OK if successful, an error code otherwise.
 */
STATUS phased_array_rot_pos_update(const uint16_t array_rotation,
					 int nx,
					 int ny,
					 struct algorithm_EW_patch_t *patches)
{
    struct algorithm_EW_patch_t temp_patches[nx * ny];

    for (int i = 0; i < nx * ny; ++i)
    {
        int x = i % nx;
        int y = i / nx;
        int new_i;

        // add error case
        switch (array_rotation)
        {
            case 90:
                new_i = x * nx + (ny - 1 - y);
                break;
            case 180:
                new_i = (nx - 1 - x) + (ny - 1 - y) * nx;
                break;
            case 270:
                new_i = y + (nx - 1 - x) * ny;
                break;
            case 0:
                new_i = i;
                break;
            default:
            	return ERROR;
        }

        temp_patches[new_i] = patches[i];
    }

    for (int i = 0; i < nx * ny; ++i)
    {
        patches[i] = temp_patches[i];
    }

    return OK;
}

/**
 * @brief Calculates the positions of elements (patches) within a array array.
 *
 * Given the array array position, number of patches in X and Y directions, spacing,
 * and an array of patches, this function computes the patch positions.
 *
 * @param patches Array of patches to be updated with positions.
 * @param nx Number of patches in the X direction.
 * @param ny Number of patches in the Y direction.
 * @param spacing Spacing between patches.
 * @param array_rotation Rotation angle of the array (ROT_0, ROT_90, ROT_180, ROT_270).
 * @param array_array_pos array array position (column and row).
 * @return OK if successful, an error code otherwise.
 */
STATUS phased_array_init_patches(struct algorithm_EW_patch_t *patches,
				       const uint16_t array_rotation,
				       const uint16_t array_array_col,
				       const uint16_t array_array_row,
				       const uint16_t number_of_patches_x,
				       const uint16_t number_of_patches_y,
				       const double patch_spacing)
{
	// determine array pose within the system
	phased_array_calc_patch_pose(array_array_col, array_array_row, number_of_patches_x, number_of_patches_y, patch_spacing, patches);

	// update array pose based on array rotation
	phased_array_rot_pos_update(array_rotation, number_of_patches_x, number_of_patches_y, patches);

    return OK;
}
