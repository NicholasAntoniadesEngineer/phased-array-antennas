# Algorithm Development - Phased Array Patch Position Calculator

This repository contains algorithms for calculating and manipulating positions of patches in a phased array system. It includes implementations in multiple languages (Python, C, and C++) for handling array translations and rotations.

## Overview

The codebase provides functionality for:
- Calculating positions of patches in a phased array
- Applying rotations (0°, 90°, 180°, 270°) to patch arrays
- Managing patch positions and labels in array configurations
- Testing and validating array transformations

## Files

- `array_translation_rotation_prototype.py`: Python implementation of the core algorithms with test cases
- `array_patch_position_calculation.c/h`: C implementation of position calculations
- `array_position_calculations_test.cpp`: Comprehensive C++ unit test suite for validating algorithm integrity

## Features

- Support for arbitrary NxM array configurations
- Rotation transformations (0°, 90°, 180°, 270°)
- Position calculation with customizable spacing
- Alphabetic labeling system for patches
- Comprehensive test cases for validation

## Usage

### Running Tests

The Python implementation includes test cases for different array sizes:
```bash
python3 array_translation_rotation_prototype.py
```

## Testing and CI/CD Integration

The repository includes a comprehensive C++ unit test suite (`array_position_calculations_test.cpp`) designed to ensure algorithm integrity through continuous integration and deployment. The test suite:

- Validates patch position calculations across different array configurations
- Tests rotation transformations for correctness
- Verifies edge cases and boundary conditions
- Ensures consistent behavior across different platforms

These tests can be integrated into CI/CD pipelines to ensure algorithm integrity with each commit.
