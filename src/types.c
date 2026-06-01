#include <math.h>
#include <stdio.h>
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
	return (vec2){a.x / b, a.y / b};
}

float dot(vec2 a, vec2 b) {
	return a.x * b.x + a.y * b.y;
}

collision_info check_collision(ball a, wall b) {
	vec2 relative_position = v2_sub(b.position, a.position);
	float collision_dot = dot(relative_position, b.normal);
	collision_info info;

	int side = (collision_dot <= 0) * 2 - 1;
	info.normal = v2_fmult(b.normal, side);
	info.depth = side * -collision_dot - a.radius;

	vec2 tangent = {b.normal.y, -b.normal.x};
	float tangent_dot = dot(relative_position, tangent);

	vec2 start_position = v2_add(b.position, v2_fmult(tangent, -b.length/2));
	vec2 end_position = v2_add(b.position, v2_fmult(tangent, b.length/2));

	vec2 relative_start_position = v2_sub(start_position, a.position);
	vec2 relative_end_position = v2_sub(end_position, a.position);

	float dist_to_start = sqrt(relative_start_position.x * relative_start_position.x + relative_start_position.y * relative_start_position.y);
	float dist_to_end = sqrt(relative_end_position.x * relative_end_position.x + relative_end_position.y * relative_end_position.y);
	int on_edge_start = dist_to_start < a.radius;
	int on_edge_end = dist_to_end < a.radius;

	int on_edge = on_edge_start || on_edge_end;

	int on_center = tangent_dot > -b.length/2 && tangent_dot < b.length/2;


	info.normal = v2_fmult(info.normal, on_center);
	info.normal = v2_add(info.normal,
			v2_fmult(
				v2_fdiv(relative_start_position, -dist_to_start),
				on_edge_start && !on_center
				));

	info.normal = v2_add(info.normal,
			v2_fmult(
				v2_fdiv(relative_end_position, -dist_to_end),
				on_edge_end && !on_center
				));
	info.depth *= on_center;
	info.depth += (dist_to_start - a.radius) * (on_edge_start && !on_center);
	info.depth += (dist_to_end - a.radius) * (on_edge_end && !on_center);
	
//	if(on_edge_start && !on_center) {
//		info.normal = v2_fdiv(relative_start_position, -dist_to_start);
//		info.depth = dist_to_start-a.radius;
//	}
//	if(on_edge_end && !on_center) {
//		info.normal = v2_fdiv(relative_end_position, -dist_to_end);
//		info.depth = dist_to_end-a.radius;
//	}
	//info.depth = dist;
	//info.normal = relative_start_position;

	// unreadable and stuff but its branchless by converting a boolean into a -1 or 1 value to decide which side to be on

	//info.hit = side*collision_dot > -a.radius && tangent_dot + a.radius > -b.length/2 && tangent_dot - a.radius < b.length/2;
	//info.hit = side*collision_dot > -a.radius || on_edge;
	info.hit = (side*collision_dot > -a.radius && on_center) || on_edge;

	return info;
}
