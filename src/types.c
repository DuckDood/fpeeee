#include <math.h>
#include <types.h>

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

float dot(vec2 a, vec2 b) {
	return a.x * b.x + a.y * b.y;
}

float magnitude(vec2 a) {
	return sqrt(dot(a, a));
}

vec2 normalize(vec2 a) {
	return v2_fdiv(a, magnitude(a));
}
