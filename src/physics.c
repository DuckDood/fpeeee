#include <math.h>
#include <physics.h>
#include <stdio.h>


void set_velocity(ball *body, vec2 velocity) {
	body->previous_position = v2_sub(body->position, velocity);
}


collision_info check_collision(ball a, wall b) {
	vec2 relative_position = v2_sub(b.position, a.position);
	vec2 previous_relative_position = v2_sub(b.position, a.previous_position);
	float collision_dot = dot(relative_position, b.normal);
	float previous_collision_dot = dot(previous_relative_position, b.normal);
	collision_info info;

	//int changed_sides = (previous_collision_dot * collision_dot) < 0; // just in case, but for some reason even without this it detects the collision if the ball completely changes sides i have no clue how but this is here just in case
	int changed_sides = 0; // doesnt actually do much i dont think and stops just changing the velocity against walls

	int side = (previous_collision_dot <= 0) * 2 - 1;
	info.normal = v2_fmult(b.normal, side);
	info.depth = side * -collision_dot - a.radius; // subtract a tiny amount like 0.01 if you dont want things of radius 0 to be able to fall through

	vec2 tangent = {b.normal.y, -b.normal.x};
	float tangent_dot = dot(previous_relative_position, tangent);

	vec2 start_position = v2_add(b.position, v2_fmult(tangent, -b.length/2));
	vec2 end_position = v2_add(b.position, v2_fmult(tangent, b.length/2));

	vec2 relative_start_position = v2_sub(start_position, a.position);
	vec2 relative_end_position = v2_sub(end_position, a.position);

	//float dist_to_start = sqrt(relative_start_position.x * relative_start_position.x + relative_start_position.y * relative_start_position.y);
	//float dist_to_end = sqrt(relative_end_position.x * relative_end_position.x + relative_end_position.y * relative_end_position.y);
	float dist_to_start = magnitude(relative_start_position);
	float dist_to_end = magnitude(relative_end_position);

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

void resolve_collision(ball *body, collision_info hit_info, float deltatime) {
	if(hit_info.hit) {
		vec2 velocity = v2_sub(body->position, body->previous_position);
		if(dot(velocity, hit_info.normal) > 0) return; // moving in the same direction as wall i dont know why its > 0 instead of < 0 but whatever
		//float speed = sqrt(dot(velocity, velocity));
		float speed = magnitude(velocity);
		body->position = v2_sub(body->position, v2_fmult(hit_info.normal, hit_info.depth));
		//float elasticity = 0.5;
		//float friction = 0.01;
		//float elasticity = 0.5;
		float elasticity = 0.8;
		//float friction = 10 * deltatime;
		float friction = 0;

		float bounce_dot = -dot(hit_info.normal, velocity);
		vec2 bounce_velocity = v2_fmult(hit_info.normal, bounce_dot);
		vec2 slide_velocity = v2_add(velocity, bounce_velocity); // this is the velocity without the speed into the wall, so its the velocity which slides along it i think if i did this right

		vec2 out_velocity = v2_add(v2_fmult(slide_velocity, 1-friction), v2_fmult(bounce_velocity, elasticity));
		//body->previous_position = v2_sub(body->position, out_velocity);
		set_velocity(body, out_velocity);
	}
}

void check_and_resolve(ball *body, wall collider, float deltatime) {
	resolve_collision(body, check_collision(*body, collider), deltatime);
}

void update_ball(ball *body) {
	vec2 new_position = v2_sub(v2_fmult(body->position, 2), body->previous_position);
	body->previous_position = body->position;
	body->position = new_position;
}

void distance_constraint(ball *a, ball *b, float length) {
	vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	//vec2 normalized_direction = v2_fmult(direction_between, 1/(sqrt(dot(direction_between, direction_between))));
	vec2 normalized_direction = normalize(direction_between);
	a->position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	b->position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));
}


void spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime) {
	/*vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	vec2 normalized_direction = v2_fmult(direction_between, 1/(sqrt(dot(direction_between, direction_between))));
	vec2 a_wanted_position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	vec2 b_wanted_position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));

	stiffness *= stiffness <= 1;

	a->position = v2_add(a->position, v2_fmult(v2_sub(a_wanted_position, a->position), stiffness));
	b->position = v2_add(b->position, v2_fmult(v2_sub(b_wanted_position, b->position), stiffness));*/
	// k(l−l0)n
	vec2 relative_position = v2_sub(b->position, a->position);
	//float relative_magnitude = sqrt(dot(relative_position, relative_position));
	float relative_magnitude = magnitude(relative_position);
	float force = stiffness * (relative_magnitude - length);
	vec2 normalized_relative_position = v2_fmult(relative_position, 1/relative_magnitude);
	vec2 push = v2_fmult(normalized_relative_position, force * deltatime * deltatime);

	a->position = v2_add(a->position, v2_fmult(push, 0.5));
	b->position = v2_add(b->position, v2_fmult(push, -0.5));
}

void rope_constraint(ball *a, ball *b, float length) {
	vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	//float distance = sqrt(dot(direction_between, direction_between));
	float distance = magnitude(direction_between);
	if(distance < length) return;
	vec2 normalized_direction = v2_fmult(direction_between, 1/distance);
	a->position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	b->position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));
}

void rope_spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime) {
	/*vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	vec2 normalized_direction = v2_fmult(direction_between, 1/(sqrt(dot(direction_between, direction_between))));
	vec2 a_wanted_position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	vec2 b_wanted_position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));

	stiffness *= stiffness <= 1;

	a->position = v2_add(a->position, v2_fmult(v2_sub(a_wanted_position, a->position), stiffness));
	b->position = v2_add(b->position, v2_fmult(v2_sub(b_wanted_position, b->position), stiffness));*/
	// k(l−l0)n
	vec2 relative_position = v2_sub(b->position, a->position);
	//float relative_magnitude = sqrt(dot(relative_position, relative_position));
	float relative_magnitude = magnitude(relative_position);
	if(relative_magnitude < length) return;
	float force = stiffness * (relative_magnitude - length);
	vec2 normalized_relative_position = v2_fmult(relative_position, 1/relative_magnitude);
	vec2 push = v2_fmult(normalized_relative_position, force * deltatime * deltatime);

	a->position = v2_add(a->position, v2_fmult(push, 0.5));
	b->position = v2_add(b->position, v2_fmult(push, -0.5));
}

// it will give information for ball a, but it should work with ball b when inverting the normal
collision_info check_ball_collision(ball a, ball b) {
	float distance = magnitude(v2_sub(a.position, b.position));
	collision_info hit_info;
	hit_info.hit = distance <= a.radius + b.radius;
	hit_info.normal = normalize(v2_sub(b.position, a.position));
	hit_info.normal = v2_fmult(hit_info.normal, -1);
	hit_info.depth = distance - a.radius - b.radius;

	return hit_info;
}

void check_and_resolve_balls(ball *a, ball *b) {
	collision_info hit_info = check_ball_collision(*a, *b);
	resolve_ball_collision(a, b, hit_info);
}

void resolve_ball_collision(ball *a, ball *b, collision_info hit_info) {
	if(hit_info.hit) {
		vec2 a_velocity = v2_sub(a->position, a->previous_position);
		vec2 b_velocity = v2_sub(b->position, b->previous_position);

		a->position = v2_sub(a->position, v2_fmult(hit_info.normal, hit_info.depth * 0.5));
		b->position = v2_sub(b->position, v2_fmult(hit_info.normal, hit_info.depth * -0.5));
	}
}
