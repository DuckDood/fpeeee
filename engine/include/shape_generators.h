#pragma once
#include <types.h>
#include <physics.h>

shape generate_rectangle(float width, float height, vec2 starting_position);
shape generate_cloth(float width, float height, int width_resolution, int height_resolution, float stiffness, vec2 starting_position);
shape generate_rope(float length, int resolution, float stiffness, vec2 starting_position);

shape_3d generate_cube_3d(float width, float height, float depth, vec3 starting_position);
shape_3d generate_cloth_3d(float width, float height, int width_resolution, int height_resolution, float stiffness, vec3 starting_position);
