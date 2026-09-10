#include <types.h>
#include <constraints.h>

// TODO: make the names better

float dist_constraint_2d(vec2 **vectors,[[maybe_unused]] float *inv_weights,[[maybe_unused]] int size, void *arguments) {
	return v2_magnitude(v2_sub(*vectors[0], *vectors[1])) - *(float*)arguments;
}
vec2 dist_constraint_del_a_2d(vec2 **vectors,[[maybe_unused]] int size, [[maybe_unused]]void *arguments) {
	return v2_normalize(v2_sub(*vectors[0], *vectors[1]));
}
vec2 dist_constraint_del_b_2d(vec2 **vectors,[[maybe_unused]] int size, [[maybe_unused]]void *arguments) {
	return v2_normalize(v2_sub(*vectors[1], *vectors[0]));
}

