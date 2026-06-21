#pragma once
#include <types.h>
#include <physics.h>

shape_2d generate_rectangle(float width, float height, vec2 starting_position);
shape_2d generate_cloth(float width, float height, int width_resolution, int height_resolution, float stiffness, vec2 starting_position);
shape_2d generate_rope(float length, int resolution, float stiffness, vec2 starting_position);

shape_2d generate_wheel(float radius, int resolution, vec2 starting_position);
shape_2d generate_circle(float radius, int resolution, vec2 starting_position);

shape_3d generate_cube_3d(float width, float height, float depth, vec3 starting_position);
shape_3d generate_cloth_3d(float width, float height, int width_resolution, int height_resolution, float stiffness, vec3 starting_position);

void free_shape_2d(shape_2d shape);
void free_shape_3d(shape_3d shape);
