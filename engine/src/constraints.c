#include <math.h>
#include <stdio.h>
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

float distance_constraint_2d(vec2 **vectors, vec2 *gradients, [[maybe_unused]]int size, void *arguments) {
	vec2 relative_position = v2_sub(*vectors[0], *vectors[1]);
	float distance_between = v2_magnitude(relative_position);

	float inv_dist_between = 1/distance_between;

	gradients[0] = v2_fmult(relative_position, inv_dist_between);
	gradients[1] = v2_fmult(relative_position, -inv_dist_between);

	return distance_between - *(float*)arguments;
}

float penetration_constraint_2d(vec2 **vectors, vec2 *gradients, [[maybe_unused]]int size, void *arguments) {
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

	vec2 relative_to_closest = v2_sub(*vectors[0], a_b_closest_point);

	float dist_to_closest = v2_magnitude(relative_to_closest);// - *(float*)arguments;
	float inv_dist_to_closest = 1/dist_to_closest;

	gradients[0] = v2_fmult(relative_to_closest, inv_dist_to_closest);
	float a_move_ratio = 1-a_b_closest_point_ratio;
	float b_move_ratio = a_b_closest_point_ratio;

	gradients[1] =  v2_fmult(relative_to_closest, -a_move_ratio * inv_dist_to_closest);
	gradients[2] =  v2_fmult(relative_to_closest, -b_move_ratio * inv_dist_to_closest);


	return dist_to_closest - *(float*)arguments;

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

// 3d
float dist_constraint_3d(vec3 **vectors,[[maybe_unused]] float *inv_weights,[[maybe_unused]] int size, void *arguments) {
	return v3_magnitude(v3_sub(*vectors[0], *vectors[1])) - *(float*)arguments;
}
vec3 dist_constraint_del_a_3d(vec3 **vectors,[[maybe_unused]] int size, [[maybe_unused]]void *arguments) {
	return v3_normalize(v3_sub(*vectors[0], *vectors[1]));
}
vec3 dist_constraint_del_b_3d(vec3 **vectors,[[maybe_unused]] int size, [[maybe_unused]]void *arguments) {
	return v3_normalize(v3_sub(*vectors[1], *vectors[0]));
}

float closest_point_along_line(vec3 a, vec3 b, vec3 point) {
	vec3 relative_a_position = v3_sub(point, a);
	vec3 relative_b_position = v3_sub(point, b);

	float a_position_magnitude = v3_magnitude(relative_a_position);
	float b_position_magnitude = v3_magnitude(relative_b_position);

	vec3 a_b_edge = v3_sub(a, b);
	float a_b_side_length = v3_magnitude(a_b_edge);
	float a_b_closest_point_ratio = (a_b_side_length + (a_position_magnitude*a_position_magnitude - b_position_magnitude*b_position_magnitude - a_b_side_length*a_b_side_length)/(2 * a_b_side_length)) / a_b_side_length;
	if(a_b_closest_point_ratio < 0) a_b_closest_point_ratio = 0;
	if(a_b_closest_point_ratio > 1) a_b_closest_point_ratio = 1;
	return a_b_closest_point_ratio; // value used to lerp between two line points
}

void get_point_triangle_info(vec3 a, vec3 b, vec3 c, vec3 collider, float *barycentric_a, float *barycentric_b, float *barycentric_c, bool *inside_triangle_plane, float *distance, int *side, vec3 *closest_normal) {
	vec3 a_b_edge = v3_sub(a, b); // gets relative edges
	vec3 b_c_edge = v3_sub(b, c);
	vec3 c_a_edge = v3_sub(c, a);

	vec3 relative_a_position = v3_sub(collider, a); // gets relative positions from vertices
	vec3 relative_b_position = v3_sub(collider, b);
	vec3 relative_c_position = v3_sub(collider, c);

	vec3 normal = v3_cross(a_b_edge, b_c_edge); // get normal vector

	float normal_magnitude = v3_magnitude(normal);
	float inverse_triangle_area = 1/(normal_magnitude * 0.5); // the magnitude of the cross product also gives double the area (for some reason)
	normal = v3_fdiv(normal, normal_magnitude); // normalize normal vector

	vec3 a_b_edge_normal = v3_normalize(v3_cross(normal, a_b_edge)); // gives a vector pointing off of each edge into the triangle
	vec3 b_c_edge_normal = v3_normalize(v3_cross(normal, b_c_edge));
	vec3 c_a_edge_normal = v3_normalize(v3_cross(normal, c_a_edge));


	float a_b_edge_height = v3_dot(relative_a_position, a_b_edge_normal); // gives the distance between the edge and the point
	float b_c_edge_height = v3_dot(relative_b_position, b_c_edge_normal);
	float c_a_edge_height = v3_dot(relative_c_position, c_a_edge_normal);

	float a_b_edge_area = a_b_edge_height * v3_magnitude(a_b_edge) * 0.5; // gives the area of the triangle made from each sides vertices and the point we're checking against (edge height toward point * edge length * 0.5)
	float b_c_edge_area = b_c_edge_height * v3_magnitude(b_c_edge) * 0.5;
	float c_a_edge_area = c_a_edge_height * v3_magnitude(c_a_edge) * 0.5;
	
	int a_b_edge_side = a_b_edge_height <= 0;
	int b_c_edge_side = b_c_edge_height <= 0;
	int c_a_edge_side = c_a_edge_height <= 0;

	*barycentric_a = -b_c_edge_area * inverse_triangle_area; // convert to barycentrics
	*barycentric_b = -c_a_edge_area * inverse_triangle_area;
	*barycentric_c = -a_b_edge_area * inverse_triangle_area;

	*inside_triangle_plane = a_b_edge_side == b_c_edge_side && a_b_edge_side == c_a_edge_side; // inside triangle (on the same side of each line)
	float face_dot = v3_dot(relative_a_position, normal);
	*side = (face_dot <= 0) * 2 - 1;
	*distance = fabs(face_dot);

	*closest_normal = normal;

	// detected face stuff but still need to do edges
	if(!*inside_triangle_plane) {
		// do line checks
		
		float a_b_lerp_t = closest_point_along_line(a, b, collider);
		float b_c_lerp_t = closest_point_along_line(b, c, collider);
		float c_a_lerp_t = closest_point_along_line(c, a, collider);

		vec3 a_b_closest_point = v3_lerp(a, b, a_b_lerp_t);
		vec3 b_c_closest_point = v3_lerp(b, c, b_c_lerp_t);
		vec3 c_a_closest_point = v3_lerp(c, a, c_a_lerp_t);

		float a_b_distance = v3_magnitude(v3_sub(a_b_closest_point, collider));
		float b_c_distance = v3_magnitude(v3_sub(b_c_closest_point, collider));
		float c_a_distance = v3_magnitude(v3_sub(c_a_closest_point, collider));
		
		float closest_distance = a_b_distance;
		vec3 closest = a_b_closest_point;

		*barycentric_a = 1-a_b_lerp_t;
		*barycentric_b = a_b_lerp_t;
		*barycentric_c = 0;

		if(b_c_distance < closest_distance) {
			closest_distance = b_c_distance;
			closest = b_c_closest_point;

			*barycentric_a = 0;
			*barycentric_b = 1-b_c_lerp_t;
			*barycentric_c = b_c_lerp_t;
		}
		if(c_a_distance < closest_distance) {
			closest_distance = c_a_distance;
			closest = c_a_closest_point;

			*barycentric_a = c_a_lerp_t;
			*barycentric_b = 0;
			*barycentric_c = 1-c_a_lerp_t;
		}
		*distance = closest_distance;
		*closest_normal = v3_fmult(v3_normalize(v3_sub(closest, collider)), *side); // we don't want the side affecting the edge normals, but we still want to keep the information for any callers
	}
}
/*void collide_wall_3d(ball_3d *a, ball_3d *b, ball_3d *c, ball_3d *collider) {
	float min_x, min_y, min_z;
	float max_x, max_y, max_z;
	min_x = fmin(fmin(a->position.x, b->position.x), c->position.x) - collider->radius;
	min_y = fmin(fmin(a->position.y, b->position.y), c->position.y) - collider->radius;
	min_z = fmin(fmin(a->position.z, b->position.z), c->position.z) - collider->radius;

	max_x = fmax(fmax(a->position.x, b->position.x), c->position.x) + collider->radius;
	max_y = fmax(fmax(a->position.y, b->position.y), c->position.y) + collider->radius;
	max_z = fmax(fmax(a->position.z, b->position.z), c->position.z) + collider->radius;

	
	if(collider->position.x < min_x || collider->position.x > max_x ||
	collider->position.y < min_y || collider->position.y > max_y ||
	collider->position.z < min_z || collider->position.z > max_z) return;

	float a_barycentric, b_barycentric, c_barycentric, distance;
	bool in_triangle_plane;
	int side;
	vec3 normal;

	get_point_triangle_info(a->position, b->position, c->position, collider->position, &a_barycentric, &b_barycentric, &c_barycentric, &in_triangle_plane, &distance, &side, &normal);


	if(distance < collider->radius) {
		float inverse_mass_collider = 1/collider->mass;

		float inverse_mass_a = 1/a->mass;
		float inverse_mass_b = 1/b->mass;
		float inverse_mass_c = 1/c->mass;

		float a_move_ratio = a_barycentric;
		float b_move_ratio = b_barycentric;
		float c_move_ratio = c_barycentric;

		float inverse_inverse_mass_total = 1/(inverse_mass_collider + inverse_mass_a * a_move_ratio + inverse_mass_b * b_move_ratio + inverse_mass_c * c_move_ratio);

		collider->position = v3_add(collider->position, v3_fmult(normal, side * inverse_mass_collider * inverse_inverse_mass_total * (distance - collider->radius)));
		a->position = v3_sub(a->position, v3_fmult(normal, side * a_move_ratio * inverse_mass_a * inverse_inverse_mass_total * (distance - collider->radius)));
		b->position = v3_sub(b->position, v3_fmult(normal, side * b_move_ratio * inverse_mass_b * inverse_inverse_mass_total * (distance - collider->radius)));
		c->position = v3_sub(c->position, v3_fmult(normal, side * c_move_ratio * inverse_mass_c * inverse_inverse_mass_total * (distance - collider->radius)));
	}
}*/

float wall_constraint_3d(vec3 **vectors, [[maybe_unused]]float *inv_weights, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	// 0,1,2,3 -> body, a, b, c
	float a_barycentric, b_barycentric, c_barycentric, distance;
	bool in_triangle_plane;
	int side;
	vec3 normal;

	get_point_triangle_info(*vectors[1], *vectors[2], *vectors[3], *vectors[0], &a_barycentric, &b_barycentric, &c_barycentric, &in_triangle_plane, &distance, &side, &normal);

	return distance - *(float*)arguments;
}

vec3 wall_constraint_del_body_3d(vec3 **vectors, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	float a_barycentric, b_barycentric, c_barycentric, distance;
	bool in_triangle_plane;
	int side;
	vec3 normal;

	get_point_triangle_info(*vectors[1], *vectors[2], *vectors[3], *vectors[0], &a_barycentric, &b_barycentric, &c_barycentric, &in_triangle_plane, &distance, &side, &normal);

	return v3_fmult(normal, -side);
}
vec3 wall_constraint_del_a_3d(vec3 **vectors, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	float a_barycentric, b_barycentric, c_barycentric, distance;
	bool in_triangle_plane;
	int side;
	vec3 normal;

	get_point_triangle_info(*vectors[1], *vectors[2], *vectors[3], *vectors[0], &a_barycentric, &b_barycentric, &c_barycentric, &in_triangle_plane, &distance, &side, &normal);
	float move_ratio = a_barycentric;
	return v3_fmult(normal, side * move_ratio);
}
vec3 wall_constraint_del_b_3d(vec3 **vectors, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	float a_barycentric, b_barycentric, c_barycentric, distance;
	bool in_triangle_plane;
	int side;
	vec3 normal;

	get_point_triangle_info(*vectors[1], *vectors[2], *vectors[3], *vectors[0], &a_barycentric, &b_barycentric, &c_barycentric, &in_triangle_plane, &distance, &side, &normal);
	float move_ratio = b_barycentric;

	return v3_fmult(normal, side * move_ratio);
}
vec3 wall_constraint_del_c_3d(vec3 **vectors, [[maybe_unused]]int size, [[maybe_unused]]void *arguments) {
	float a_barycentric, b_barycentric, c_barycentric, distance;
	bool in_triangle_plane;
	int side;
	vec3 normal;

	get_point_triangle_info(*vectors[1], *vectors[2], *vectors[3], *vectors[0], &a_barycentric, &b_barycentric, &c_barycentric, &in_triangle_plane, &distance, &side, &normal);
	float move_ratio = c_barycentric;

	return v3_fmult(normal, side * move_ratio);
}


float distance_constraint_3d(vec3 **vectors, vec3 *gradient_outputs, [[maybe_unused]]int size, void *arguments) {
	vec3 relative_position = v3_sub(*vectors[0], *vectors[1]);
	float distance_between = v3_magnitude(relative_position);

	float inv_dist_between = 1/distance_between;

	gradient_outputs[0] = v3_fmult(relative_position, inv_dist_between);
	gradient_outputs[1] = v3_fmult(relative_position, -inv_dist_between);

	return distance_between - *(float*)arguments;
}

float penetration_constraint_3d(vec3 **vectors, vec3 *gradient_outputs, [[maybe_unused]]int size, void *arguments) {
	float a_barycentric, b_barycentric, c_barycentric, distance;
	bool in_triangle_plane;
	int side;
	vec3 normal;

	get_point_triangle_info(*vectors[1], *vectors[2], *vectors[3], *vectors[0], &a_barycentric, &b_barycentric, &c_barycentric, &in_triangle_plane, &distance, &side, &normal);
	gradient_outputs[0] = v3_fmult(normal, -side);
	gradient_outputs[1] = v3_fmult(normal, side * a_barycentric);
	gradient_outputs[2] = v3_fmult(normal, side * b_barycentric);
	gradient_outputs[3] = v3_fmult(normal, side * c_barycentric);

	return distance - *(float*)arguments;
}
