#include <math.h>
#include <physics.h>
#include <stdio.h>


void set_velocity(ball *body, vec2 velocity) {
	body->previous_position = v2_sub(body->position, velocity);
}


collision_info check_collision(ball a, wall b) {
	vec2 relative_position = v2_sub(b.position, a.position);
	vec2 previous_relative_position = v2_sub(b.position, a.previous_position);
	float collision_dot = v2_dot(relative_position, b.normal);
	float previous_collision_dot = v2_dot(previous_relative_position, b.normal);
	collision_info info;

	int side = (previous_collision_dot <= 0) * 2 - 1;
	info.normal = v2_fmult(b.normal, side);
	info.depth = side * -collision_dot - a.radius; // subtract a tiny amount like 0.01 if you dont want things of radius 0 to be able to fall through, pretty hacky fix though and doesnt always work

	vec2 tangent = {b.normal.y, -b.normal.x};
	float tangent_dot = v2_dot(previous_relative_position, tangent);

	vec2 start_position = v2_add(b.position, v2_fmult(tangent, -b.length/2));
	vec2 end_position = v2_add(b.position, v2_fmult(tangent, b.length/2));

	vec2 relative_start_position = v2_sub(start_position, a.position);
	vec2 relative_end_position = v2_sub(end_position, a.position);

	float dist_to_start = v2_magnitude(relative_start_position);
	float dist_to_end = v2_magnitude(relative_end_position);

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
	info.hit = (side*collision_dot > -a.radius && on_center) || on_edge;

	return info;
}

void resolve_collision(ball *body, collision_info hit_info, float elasticity, float friction, float deltatime) {
	/* note for future if i add moving walls: if the wall is kinematic, 
	body->position = v2_sub(body->position, v2_fmult(hit_info.normal, hit_info.depth));
	is all you need to do (not really, it makes particles on top a bit jumpy and they clip through pretty easily)
	*/
	if(hit_info.hit) {
		vec2 velocity = v2_sub(body->position, body->previous_position);
		if(v2_dot(velocity, hit_info.normal) > 0) return; // moving in the same direction as wall i dont know why its > 0 instead of < 0 but whatever
		friction *= deltatime; // works for some reason, seemingly it should be squared or not at all but idk
		float speed = v2_magnitude(velocity);
		body->position = v2_sub(body->position, v2_fmult(hit_info.normal, hit_info.depth));

		float bounce_dot = -v2_dot(hit_info.normal, velocity);
		vec2 bounce_velocity = v2_fmult(hit_info.normal, bounce_dot);
		vec2 slide_velocity = v2_add(velocity, bounce_velocity); // this is the velocity without the speed into the wall, so its the velocity which slides along it i think if i did this right

		vec2 out_velocity = v2_add(v2_fmult(slide_velocity, 1-friction), v2_fmult(bounce_velocity, elasticity));
		set_velocity(body, out_velocity);
	}
}

void check_and_resolve(ball *body, wall collider, float elasticity, float friction, float deltatime) {
	resolve_collision(body, check_collision(*body, collider), elasticity, friction, deltatime);
}

void update_ball(ball *body) {
	vec2 new_position = v2_sub(v2_fmult(body->position, 2), body->previous_position);
	body->previous_position = body->position;
	body->position = new_position;
}

void distance_constraint(ball *a, ball *b, float length) {
	vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	vec2 normalized_direction = v2_normalize(direction_between);
	a->position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	b->position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));
}


void spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime) {
	vec2 relative_position = v2_sub(b->position, a->position);
	float relative_magnitude = v2_magnitude(relative_position);
	float force = stiffness * (relative_magnitude - length);
	vec2 normalized_relative_position = v2_fmult(relative_position, 1/relative_magnitude);
	vec2 push = v2_fmult(normalized_relative_position, force * deltatime * deltatime);

	a->position = v2_add(a->position, v2_fmult(push, 0.5));
	b->position = v2_add(b->position, v2_fmult(push, -0.5));
}

void rope_constraint(ball *a, ball *b, float length) {
	vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	float distance = v2_magnitude(direction_between);
	if(distance < length) return;
	vec2 normalized_direction = v2_fmult(direction_between, 1/distance);
	a->position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	b->position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));
}

void rope_spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime) {
	vec2 relative_position = v2_sub(b->position, a->position);
	float relative_magnitude = v2_magnitude(relative_position);
	if(relative_magnitude < length) return;
	float force = stiffness * (relative_magnitude - length);
	vec2 normalized_relative_position = v2_fmult(relative_position, 1/relative_magnitude);
	vec2 push = v2_fmult(normalized_relative_position, force * deltatime * deltatime);

	a->position = v2_add(a->position, v2_fmult(push, 0.5));
	b->position = v2_add(b->position, v2_fmult(push, -0.5));
}

void update_linkage(linkage link, float deltatime) {
	switch(link.type) {
		case DISTANCE:
			distance_constraint(link.a, link.b, link.length);
			break;
		case SPRING:
			spring_constraint(link.a, link.b, link.length, link.stiffness, deltatime);
			break;
		case ROPE:
			rope_constraint(link.a, link.b, link.length);
			break;
		case ROPE_SPRING:
			rope_spring_constraint(link.a, link.b, link.length, link.stiffness, deltatime);
			break;
		default:
			break;
	}
}

// it will give information for ball a, but it should work with ball b when inverting the normal
collision_info check_ball_collision(ball a, ball b) {
	float distance = v2_magnitude(v2_sub(a.position, b.position));
	collision_info hit_info;
	if(a.position.x + a.radius > b.position.x - b.radius && a.position.x - a.radius < b.position.x + b.radius /* x */ && a.position.y + a.radius > b.position.y - b.radius && a.position.y - a.radius < b.position.y + b.radius) {
		hit_info.hit = distance <= a.radius + b.radius;
		hit_info.normal = v2_normalize(v2_sub(a.position, b.position));
		hit_info.depth = distance - a.radius - b.radius;
	} else {
		hit_info.hit = 0;
	}

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
