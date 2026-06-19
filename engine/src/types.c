#include <math.h>
#include <types.h>

// vector 2 functions
vec2 v2_add(vec2 a, vec2 b) {
	return (vec2){a.x + b.x, a.y + b.y};
}

vec2 v2_sub(vec2 a, vec2 b) {
	return (vec2){a.x - b.x, a.y - b.y};
}

vec2 v2_mult(vec2 a, vec2 b) {
	return (vec2){a.x * b.x, a.y * b.y};
}

vec2 v2_fmult(vec2 a, float b) {
	return (vec2){a.x * b, a.y * b};
}

vec2 v2_fdiv(vec2 a, float b) {
	float inverse = 1 / b; // faster i think
	return (vec2){a.x * inverse, a.y * inverse};
}

float v2_dot(vec2 a, vec2 b) {
	return a.x * b.x + a.y * b.y;
}

float v2_magnitude(vec2 a) {
	return sqrt(v2_dot(a, a));
}

vec2 v2_normalize(vec2 a) {
	return v2_fdiv(a, v2_magnitude(a));
}

// vector 3 functions

vec3 v3_add(vec3 a, vec3 b) {
	return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

vec3 v3_sub(vec3 a, vec3 b) {
	return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

vec3 v3_mult(vec3 a, vec3 b) {
	return (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

vec3 v3_fmult(vec3 a, float b) {
	return (vec3){a.x * b, a.y * b, a.z * b};
}

vec3 v3_fdiv(vec3 a, float b) {
	float inverse = 1 / b; // faster i think
	return (vec3){a.x * inverse, a.y * inverse, a.z * inverse};
}

float v3_dot(vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float v3_magnitude(vec3 a) {
	return sqrt(v3_dot(a, a));
}

vec3 v3_normalize(vec3 a) {
	return v3_fdiv(a, v3_magnitude(a));
}

vec3 v3_cross(vec3 a, vec3 b) {
	vec3 crossed;

	crossed.x = a.y * b.z - a.z * b.y;
	crossed.y = a.z * b.x - a.x * b.z;
	crossed.z = a.x * b.y - a.y * b.x;

	return crossed;
}

vec3 v3_lerp(vec3 a, vec3 b, float t) {
	vec3 c;
	c.x = (a.x * (1-t)) + b.x * t;
	c.y = (a.y * (1-t)) + b.y * t;
	c.z = (a.z * (1-t)) + b.z * t;

	return c;
}
