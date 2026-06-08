#pragma once
#include <types.h>

typedef struct {
	// will be column major because for some reason all the graphics api's do that and this is just to transform any points into camera space for viewing
	float matrix[9];
} mat3;

mat3 transpose(mat3 matrix);
mat3 m3_mult(mat3 a, mat3 b);
vec3 m3_v3_mult(mat3 a, vec3 b);

mat3 generate_rotation_matrix(float yaw, float pitch, float roll);
