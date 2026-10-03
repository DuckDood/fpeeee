#pragma once
#include <types.h>

typedef float (*constraint_function_2d)(vec2 **vectors, vec2 *gradients, int size, void *arguments);



float distance_constraint_2d(vec2 **vectors, vec2 *gradients, int size, void *arguments);
float penetration_constraint_2d(vec2 **vectors, vec2 *gradients, int size, void *arguments);

typedef struct {
	vec2 position;
	vec2 normal;
	float distance;
} boundary_args_2d;
float boundary_constraint_2d(vec2 **vectors, vec2 *gradients, int size, void *arguments);


// 3d
typedef float (*constraint_function_3d)(vec3 **vectors, vec3 *gradients, int size, void *arguments);

float distance_constraint_3d(vec3 **vectors, vec3 *gradients, int size, void *arguments);
float penetration_constraint_3d(vec3 **vectors, vec3 *gradients, int size, void *arguments);
