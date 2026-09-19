#pragma once
#include <types.h>

// TODO: make the names better, move entire constraint into just one function (no del's or anything)


typedef float (*constraint_function_2d)(vec2 **vectors, float *inv_weights, int size, void *arguments);
typedef vec2 (*del_constraint_function_2d)(vec2 **vectors, int size, void *arguments);


float dist_constraint_2d(vec2 **vectors, float *inv_weights, int size, void *arguments);
vec2 dist_constraint_del_a_2d(vec2 **vectors, int size, void *arguments);
vec2 dist_constraint_del_b_2d(vec2 **vectors, int size, void *arguments);

typedef struct {
	vec2 **vectors;
	vec2 *output_ptr;
	void *arguments;
	int size;
} constraint_params;

typedef float (*full_constraint_function_2d)(constraint_params*);



float distance_constraint_2d(constraint_params*);




float wall_constraint_2d(vec2 **vectors, float *inv_weights, int size, void *arguments);


vec2 wall_constraint_del_body_2d(vec2 **vectors, int size, void *arguments);
vec2 wall_constraint_del_a_2d(vec2 **vectors, int size, void *arguments);
vec2 wall_constraint_del_b_2d(vec2 **vectors, int size, void *arguments);

// 3d
float dist_constraint_3d(vec3 **vectors, float *inv_weights, int size, void *arguments);
vec3 dist_constraint_del_a_3d(vec3 **vectors, int size, void *arguments);
vec3 dist_constraint_del_b_3d(vec3 **vectors, int size, void *arguments);

float wall_constraint_3d(vec3 **vectors, float *inv_weights, int size, void *arguments);

vec3 wall_constraint_del_body_3d(vec3 **vectors, int size, void *arguments);
vec3 wall_constraint_del_a_3d(vec3 **vectors, int size, void *arguments);
vec3 wall_constraint_del_b_3d(vec3 **vectors, int size, void *arguments);
vec3 wall_constraint_del_c_3d(vec3 **vectors, int size, void *arguments);
