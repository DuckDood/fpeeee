#pragma once
typedef struct {
	float x;
	float y;
} vec2;


vec2 v2_add(vec2 a, vec2 b);
vec2 v2_sub(vec2 a, vec2 b);
vec2 v2_mult(vec2 a, vec2 b);
vec2 v2_fmult(vec2 a, float b);
vec2 v2_fdiv(vec2 a, float b);
float dot(vec2 a, vec2 b);
float magnitude(vec2 a);
vec2 normalize(vec2 a);
