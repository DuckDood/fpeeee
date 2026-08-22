#pragma once
typedef struct {
	float x;
	float y;
} vec2;

// vector 2 functions
vec2 v2_add(vec2 a, vec2 b);
vec2 v2_sub(vec2 a, vec2 b);
vec2 v2_mult(vec2 a, vec2 b);
vec2 v2_fmult(vec2 a, float b);
vec2 v2_fdiv(vec2 a, float b);
float v2_dot(vec2 a, vec2 b);
float v2_magnitude(vec2 a);
vec2 v2_normalize(vec2 a);

vec2 v2_lerp(vec2 a, vec2 b, float t);

typedef struct {
	float x;
	float y;
	float z;
} vec3;

// vector 3 functions
vec3 v3_add(vec3 a, vec3 b);
vec3 v3_sub(vec3 a, vec3 b);
vec3 v3_mult(vec3 a, vec3 b);
vec3 v3_fmult(vec3 a, float b);
vec3 v3_fdiv(vec3 a, float b);
float v3_dot(vec3 a, vec3 b);
float v3_magnitude(vec3 a);
vec3 v3_normalize(vec3 a);
vec3 v3_cross(vec3 a, vec3 b);
vec3 v3_lerp(vec3 a, vec3 b, float t);
