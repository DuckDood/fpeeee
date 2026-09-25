#pragma once
#include <types.h>

typedef float (*full_constraint_function_2d)(vec2 **vectors, vec2 *gradients, int size, void *arguments);



float distance_constraint_2d(vec2 **vectors, vec2 *gradients, int size, void *arguments);
float penetration_constraint_2d(vec2 **vectors, vec2 *gradients, int size, void *arguments);


// 3d
typedef float (*constraint_function_3d)(vec3 **vectors, vec3 *gradients, int size, void *arguments);

float distance_constraint_3d(vec3 **vectors, vec3 *gradients, int size, void *arguments);
float penetration_constraint_3d(vec3 **vectors, vec3 *gradients, int size, void *arguments);
