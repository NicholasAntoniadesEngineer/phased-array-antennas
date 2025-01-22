#!/usr/bin/env python3
# 
# @file patch_translation_rotation_prototype.py
# @brief Prototype for patch translation and rotation in a phased array.
# 
# This script provides functions to calculate the positions of patches in a phased array,
# apply rotations to the array and print the resulting patch positions. It includes
# a Patch class for defining patch poses, functions for translating and rotating patches,
# and test cases to validate the implementation.
# 
# @author Nicholas Antoniades
# @date 2 April 2024
# 

PROJECT_ARRAY_PATCH_SPACING = 1 
PROJECT_ARRAY_NUMBER_OF_PATCHES_X = 1 
PROJECT_ARRAY_NUMBER_OF_PATCHES_Y = 1 
ROT_0, ROT_90, ROT_180, ROT_270 = range(4)

class Patch:
    """
    Initializes a new Patch object with default pose values.
    """
    def __init__(self):
        self.pose = {'t_x': 0, 't_y': 0}

class PhasedArrayPos:
    def __init__(self, column, row):
        """
        Initializes a PhasedArrayPos object with specified column and row.
        
        @param column: Column position of the phased array.
        @param row: Row position of the phased array.
        """
        self.column = column
        self.row = row

def patch_translation(phased_array_pos, nx, ny, spacing, patches):
    """
    Calculates the positions of patches in a phased array and assigns them labels.

    @param phased_array_pos: The PhasedArrayPos object specifying the position of the phased array.
    @param nx: Number of patches in the X direction.
    @param ny: Number of patches in the Y direction.
    @param spacing: Spacing between patches.
    @param patches: List of Patch objects to be updated with positions.
    @return: A status string indicating the result of the operation.
    """
    alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    array_x_offset = phased_array_pos.column * PROJECT_ARRAY_PATCH_SPACING * PROJECT_ARRAY_NUMBER_OF_PATCHES_X
    array_y_offset = phased_array_pos.row * PROJECT_ARRAY_PATCH_SPACING * PROJECT_ARRAY_NUMBER_OF_PATCHES_Y
    patch_x_offset = array_x_offset

    for x in range(nx):
        patch_y_offset = array_y_offset
        for y in range(ny):
            i = y + ny * x
            patches[i].pose['t_x'] = patch_x_offset
            patches[i].pose['t_y'] = patch_y_offset
            patches[i].label = alphabet[i]  
            patch_y_offset += spacing
        patch_x_offset += spacing

    return 'R2_OK'

def matrix_rotation(i, nx, ny, rotation):
    """
    Calculates the new index of a patch after applying a rotation.

    @param i: Current index of the patch.
    @param nx: Number of patches in the X direction.
    @param ny: Number of patches in the Y direction.
    @param rotation: Rotation to be applied (ROT_0, ROT_90, ROT_180, ROT_270).
    @return: New index of the patch after rotation.
    """
    x, y = i % nx, i // nx
    if rotation == ROT_90:
        return y + (nx - 1 - x) * ny 
    elif rotation == ROT_180:
        return (nx - 1 - x) + (ny - 1 - y) * nx 
    elif rotation == ROT_270:
        return x * nx + (ny - 1 - y)  
    else:
        return i
        
def patch_pos_update(array_rotation, nx, ny, patches):
    """
    Updates the positions and labels of patches based on the specified phased rotation.

    @param array_rotation: Rotation to be applied (ROT_0, ROT_90, ROT_180, ROT_270).
    @param nx: Number of patches in the X direction.
    @param ny: Number of patches in the Y direction.
    @param patches: List of Patch objects to be updated.
    @return: List of Patch objects after applying rotation.
    """
    new_patches = [None] * len(patches)
    for i in range(nx * ny):
        new_i = matrix_rotation(i, nx, ny, array_rotation)
        new_patches[new_i] = patches[i]

    return new_patches

def print_patches_matrix(patches, nx, ny, label):
    """
    Prints the patches in a matrix format.

    @param patches: List of Patch objects.
    @param nx: Number of patches in the X direction.
    @param ny: Number of patches in the Y direction.
    @param label: Label to print before the matrix.
    """
    print(f"{label}:")
    for y in range(ny):
        row = []
        for x in range(nx):
            i = y + ny * x
            row.append(patches[i].label)
        print('[' + ', '.join(row) + ']')
    print() 

def test_case(nx, ny):
    """
    Tests the rotation functionality for a given size of patch array.

    @param nx: Number of patches in the X direction.
    @param ny: Number of patches in the Y direction.
    """

    # Calculate initial patch positions and assign labels
    phased_array_pos = PhasedArrayPos(1, 1) 
    patches = [Patch() for _ in range(nx * ny)]
    patch_translation(phased_array_pos, nx, ny, 10, patches)  
    print_patches_matrix(patches, nx, ny, "Original Positions")

    # Apply rotation and get new patch arrangement
    rotated_patches = patch_pos_update(ROT_90, nx, ny, patches)
    print_patches_matrix(rotated_patches, nx, ny, "Positions After 90' Rotation")

        # Calculate initial patch positions and assign labels
    phased_array_pos = PhasedArrayPos(1, 1) 
    patches = [Patch() for _ in range(nx * ny)]
    patch_translation(phased_array_pos, nx, ny, 10, patches)  
    print_patches_matrix(patches, nx, ny, "Original Positions")

    # Apply rotation and get new patch arrangement
    rotated_patches = patch_pos_update(ROT_180, nx, ny, patches)
    print_patches_matrix(rotated_patches, nx, ny, "Positions After 180' Rotation")

    # Calculate initial patch positions and assign labels
    phased_array_pos = PhasedArrayPos(1, 1)  
    patches = [Patch() for _ in range(nx * ny)]
    patch_translation(phased_array_pos, nx, ny, 10, patches)  
    print_patches_matrix(patches, nx, ny, "Original Positions")

    # Apply rotation and get new patch arrangement
    rotated_patches = patch_pos_update(ROT_270, nx, ny, patches)
    print_patches_matrix(rotated_patches, nx, ny, "Positions After 270' Rotation")


def main():
    print("2x2 Array:")
    test_case(2, 2)

    print("4x4 Array:")
    test_case(4, 4)

if __name__ == "__main__":
    main()
