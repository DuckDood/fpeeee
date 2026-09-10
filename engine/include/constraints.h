#include <types.h>

// TODO: make the names better

float dist_constraint_2d(vec2 **vectors, float *inv_weights,[[maybe_unused]] int size, void *arguments);
vec2 dist_constraint_del_a_2d(vec2 **vectors, int size, void *arguments);
vec2 dist_constraint_del_b_2d(vec2 **vectors, int size, void *arguments);
