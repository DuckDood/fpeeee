#include <math.h>
#include <physics.h>

collision_info check_collision(ball a, wall b) {
	vec2 relative_position = v2_sub(b.position, a.position);
	vec2 previous_relative_position = v2_sub(b.position, a.previous_position);
	float collision_dot = dot(relative_position, b.normal);
	float previous_collision_dot = dot(previous_relative_position, b.normal);
	collision_info info;

	int changed_sides = (previous_collision_dot * collision_dot) < 0; // just in case, but for some reason even without this it detects the collision if the ball completely changes sides i have no clue how but this is here just in case

	int side = (previous_collision_dot <= 0) * 2 - 1;
	info.normal = v2_fmult(b.normal, side);
	info.depth = side * -collision_dot - a.radius; // subtract a tiny amount like 0.01 if you dont want things of radius 0 to be able to fall through

	vec2 tangent = {b.normal.y, -b.normal.x};
	float tangent_dot = dot(previous_relative_position, tangent);

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
	info.hit = (side*collision_dot > -a.radius && on_center) || on_edge || changed_sides;

	return info;
}

void resolve_collision(ball *body, collision_info hit_info) {
	if(hit_info.hit) {
		vec2 velocity = v2_sub(body->position, body->previous_position);
		float speed = sqrt(dot(velocity, velocity));
		body->position = v2_sub(body->position, v2_fmult(hit_info.normal, hit_info.depth));
		float elasticity = 0.8;
		float friction = 0;

		float bounce_dot = -dot(hit_info.normal, velocity);
		vec2 bounce_velocity = v2_fmult(hit_info.normal, bounce_dot);
		vec2 slide_velocity = v2_add(velocity, bounce_velocity); // this is the velocity without the speed into the wall, so its the velocity which slides along it i think if i did this right

		vec2 out_velocity = v2_add(v2_fmult(slide_velocity, 1-friction), v2_fmult(bounce_velocity, elasticity));
		body->previous_position = v2_sub(body->position, out_velocity);
	}
}

void check_and_resolve(ball *body, wall collider) {
	resolve_collision(body, check_collision(*body, collider));
}

void update_ball(ball *body) {
	vec2 new_position = v2_sub(v2_fmult(body->position, 2), body->previous_position);
	body->previous_position = body->position;
	body->position = new_position;
}

void distance_constraint(ball *a, ball *b, float length) {
	vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	vec2 normalized_direction = v2_fmult(direction_between, 1/(sqrt(dot(direction_between, direction_between))));
	a->position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	b->position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));
}


void spring_constraint(ball *a, ball *b, float length) {
	vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	vec2 normalized_direction = v2_fmult(direction_between, 1/(sqrt(dot(direction_between, direction_between))));
	vec2 a_wanted_position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	vec2 b_wanted_position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));

	a->position = v2_add(a->position, v2_fmult(v2_sub(a_wanted_position, a->position), 0.5));
	b->position = v2_add(b->position, v2_fmult(v2_sub(b_wanted_position, b->position), 0.5));
}
