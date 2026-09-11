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

float wall_constraint_2d(vec2 **vectors, [[maybe_unused]]float *inv_weights, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	// vectors should be 0,1,2 -> body, vertex a, vertex b
	vec2 a_b_edge = v2_sub(*vectors[2], *vectors[1]);

	vec2 relative_a_position = v2_sub(*vectors[1], *vectors[0]);
	vec2 relative_b_position = v2_sub(*vectors[2], *vectors[0]);

	float a_position_magnitude = v2_magnitude(relative_a_position);
	float b_position_magnitude = v2_magnitude(relative_b_position);

	float a_b_side_length = v2_magnitude(a_b_edge);
	float a_b_closest_point_ratio = (a_b_side_length + (a_position_magnitude*a_position_magnitude - b_position_magnitude*b_position_magnitude - a_b_side_length*a_b_side_length)/(2 * a_b_side_length)) / a_b_side_length;
	if(a_b_closest_point_ratio < 0) a_b_closest_point_ratio = 0;
	if(a_b_closest_point_ratio > 1) a_b_closest_point_ratio = 1;
	vec2 a_b_closest_point = v2_lerp(*vectors[1], *vectors[2], a_b_closest_point_ratio);

	float dist_to_line = v2_magnitude(v2_sub(a_b_closest_point, *vectors[0])) - *(float*)arguments;
	return dist_to_line;
}

vec2 wall_constraint_del_body_2d(vec2 **vectors, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	vec2 a_b_edge = v2_sub(*vectors[2], *vectors[1]);

	vec2 relative_a_position = v2_sub(*vectors[1], *vectors[0]);
	vec2 relative_b_position = v2_sub(*vectors[2], *vectors[0]);

	float a_position_magnitude = v2_magnitude(relative_a_position);
	float b_position_magnitude = v2_magnitude(relative_b_position);

	float a_b_side_length = v2_magnitude(a_b_edge);
	float a_b_closest_point_ratio = (a_b_side_length + (a_position_magnitude*a_position_magnitude - b_position_magnitude*b_position_magnitude - a_b_side_length*a_b_side_length)/(2 * a_b_side_length)) / a_b_side_length;
	if(a_b_closest_point_ratio < 0) a_b_closest_point_ratio = 0;
	if(a_b_closest_point_ratio > 1) a_b_closest_point_ratio = 1;
	vec2 a_b_closest_point = v2_lerp(*vectors[1], *vectors[2], a_b_closest_point_ratio);

	return v2_normalize(v2_sub(*vectors[0], a_b_closest_point));
}
vec2 wall_constraint_del_a_2d(vec2 **vectors, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	vec2 a_b_edge = v2_sub(*vectors[2], *vectors[1]);

	vec2 relative_a_position = v2_sub(*vectors[1], *vectors[0]);
	vec2 relative_b_position = v2_sub(*vectors[2], *vectors[0]);

	float a_position_magnitude = v2_magnitude(relative_a_position);
	float b_position_magnitude = v2_magnitude(relative_b_position);

	float a_b_side_length = v2_magnitude(a_b_edge);
	float a_b_closest_point_ratio = (a_b_side_length + (a_position_magnitude*a_position_magnitude - b_position_magnitude*b_position_magnitude - a_b_side_length*a_b_side_length)/(2 * a_b_side_length)) / a_b_side_length;
	if(a_b_closest_point_ratio < 0) a_b_closest_point_ratio = 0;
	if(a_b_closest_point_ratio > 1) a_b_closest_point_ratio = 1;
	vec2 a_b_closest_point = v2_lerp(*vectors[1], *vectors[2], a_b_closest_point_ratio);
	float move_ratio = 1-a_b_closest_point_ratio;

	//return v2_fmult(v2_normalize(v2_sub(a_b_closest_point, *vectors[0])), move_ratio);
	return v2_fmult(v2_normalize(v2_sub(a_b_closest_point, *vectors[0])), move_ratio);
}
vec2 wall_constraint_del_b_2d(vec2 **vectors, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	vec2 a_b_edge = v2_sub(*vectors[2], *vectors[1]);

	vec2 relative_a_position = v2_sub(*vectors[1], *vectors[0]);
	vec2 relative_b_position = v2_sub(*vectors[2], *vectors[0]);

	float a_position_magnitude = v2_magnitude(relative_a_position);
	float b_position_magnitude = v2_magnitude(relative_b_position);

	float a_b_side_length = v2_magnitude(a_b_edge);
	float a_b_closest_point_ratio = (a_b_side_length + (a_position_magnitude*a_position_magnitude - b_position_magnitude*b_position_magnitude - a_b_side_length*a_b_side_length)/(2 * a_b_side_length)) / a_b_side_length;
	if(a_b_closest_point_ratio < 0) a_b_closest_point_ratio = 0;
	if(a_b_closest_point_ratio > 1) a_b_closest_point_ratio = 1;
	vec2 a_b_closest_point = v2_lerp(*vectors[1], *vectors[2], a_b_closest_point_ratio);
	float move_ratio = a_b_closest_point_ratio;
	return v2_fmult(v2_normalize(v2_sub(a_b_closest_point, *vectors[0])), move_ratio);
}
