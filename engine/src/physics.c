#include <physics.h>
#include <stdio.h>
#include <stdlib.h>


void set_velocity_2d(ball_2d *body, vec2 velocity) {
	body->previous_position = v2_sub(body->position, velocity);
}


collision_info_2d check_collision_2d(ball_2d a, wall_2d b) {
	vec2 relative_position = v2_sub(b.position, a.position);
	vec2 previous_relative_position = v2_sub(b.position, a.previous_position);
	float collision_dot = v2_dot(relative_position, b.normal);
	float previous_collision_dot = v2_dot(previous_relative_position, b.normal);
	collision_info_2d info;

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

void resolve_collision_2d(ball_2d *body, collision_info_2d hit_info, float elasticity, float friction, float deltatime) {
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
		set_velocity_2d(body, out_velocity);
	}
}

void check_and_resolve_2d(ball_2d *body, wall_2d collider, float elasticity, float friction, float deltatime) {
	resolve_collision_2d(body, check_collision_2d(*body, collider), elasticity, friction, deltatime);
}

void update_ball_2d(ball_2d *body) {
	vec2 new_position = v2_sub(v2_fmult(body->position, 2), body->previous_position);
	body->previous_position = body->position;
	body->position = new_position;
}

void distance_constraint_2d(ball_2d *a, ball_2d *b, float length) {
	/*
	vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	vec2 normalized_direction = v2_normalize(direction_between);
	a->position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	b->position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));
	*/
	// from the 'Advanced Character Physics' paper by Thomas Jakobson
	
	float inverse_mass_a = 1/a->mass;
	float inverse_mass_b = 1/b->mass;
	vec2 delta = v2_sub(b->position, a->position);
	float deltalength = v2_magnitude(delta);
	float diff = (deltalength - length) / (deltalength*(inverse_mass_a + inverse_mass_b));
	diff *= 1;

	a->position = v2_add(a->position, v2_fmult(delta, inverse_mass_a * diff));
	b->position = v2_sub(b->position, v2_fmult(delta, inverse_mass_b * diff));
}


void spring_constraint_2d(ball_2d *a, ball_2d *b, float length, float stiffness, float deltatime) {
	/*
	vec2 relative_position = v2_sub(b->position, a->position);
	float relative_magnitude = v2_magnitude(relative_position);
	float force = stiffness * (relative_magnitude - length);
	vec2 normalized_relative_position = v2_fmult(relative_position, 1/relative_magnitude);
	vec2 push = v2_fmult(normalized_relative_position, force * deltatime * deltatime);

	a->position = v2_add(a->position, v2_fmult(push, 0.5));
	b->position = v2_add(b->position, v2_fmult(push, -0.5));
	*/
	float inverse_mass_a = 1/a->mass;
	float inverse_mass_b = 1/b->mass;
	vec2 delta = v2_sub(a->position, b->position);
	float deltalength = v2_magnitude(delta);
	// F = -kx (Hooke's law)
	// k = stiffness
	// x = deltalength - length
	float force = -stiffness * (deltalength-length);

	vec2 delta_norm = v2_fdiv(delta, deltalength);

	// F = ma
	// a = F/m
	a->position = v2_add(a->position, v2_fmult(delta_norm, force * inverse_mass_a * deltatime*deltatime));
	b->position = v2_sub(b->position, v2_fmult(delta_norm, force * inverse_mass_b * deltatime*deltatime));
}

void rope_constraint_2d(ball_2d *a, ball_2d *b, float length) {/*
	vec2 direction_between = v2_sub(a->position, b->position);
	vec2 inbetween_point = v2_fmult(v2_add(a->position, b->position), 0.5);
	float distance = v2_magnitude(direction_between);
	if(distance < length) return;
	vec2 normalized_direction = v2_fmult(direction_between, 1/distance);
	a->position = v2_add(inbetween_point, v2_fmult(normalized_direction, length*0.5));
	b->position = v2_add(inbetween_point, v2_fmult(normalized_direction, -length*0.5));
	*/
	vec2 relative_position = v2_sub(b->position, a->position);
	float relative_magnitude = v2_magnitude(relative_position);
	if(relative_magnitude < length) return;
	distance_constraint_2d(a, b, length);
}

void rope_spring_constraint_2d(ball_2d *a, ball_2d *b, float length, float stiffness, float deltatime) {
	/*
	vec2 relative_position = v2_sub(b->position, a->position);
	float relative_magnitude = v2_magnitude(relative_position);
	if(relative_magnitude < length) return;
	float force = stiffness * (relative_magnitude - length);
	vec2 normalized_relative_position = v2_fmult(relative_position, 1/relative_magnitude);
	vec2 push = v2_fmult(normalized_relative_position, force * deltatime * deltatime);

	a->position = v2_add(a->position, v2_fmult(push, 0.5));
	b->position = v2_add(b->position, v2_fmult(push, -0.5));
	*/
	vec2 relative_position = v2_sub(b->position, a->position);
	float relative_magnitude = v2_magnitude(relative_position);
	if(relative_magnitude < length) return;
	spring_constraint_2d(a, b, length, stiffness, deltatime);
}

void update_linkage_2d(linkage_2d link, float deltatime) {
	switch(link.type) {
		case DISTANCE:
			distance_constraint_2d(link.a, link.b, link.length);
			break;
		case SPRING:
			spring_constraint_2d(link.a, link.b, link.length, link.stiffness, deltatime);
			break;
		case ROPE:
			rope_constraint_2d(link.a, link.b, link.length);
			break;
		case ROPE_SPRING:
			rope_spring_constraint_2d(link.a, link.b, link.length, link.stiffness, deltatime);
			break;
		default:
			break;
	}
}

// it will give information for ball a, but it should work with ball b when inverting the normal
collision_info_2d check_ball_collision_2d(ball_2d a, ball_2d b) {
	float distance = v2_magnitude(v2_sub(a.position, b.position));
	collision_info_2d hit_info;
	if(a.position.x + a.radius > b.position.x - b.radius && a.position.x - a.radius < b.position.x + b.radius /* x */ && a.position.y + a.radius > b.position.y - b.radius && a.position.y - a.radius < b.position.y + b.radius) {
		hit_info.hit = distance <= a.radius + b.radius;
		hit_info.normal = v2_normalize(v2_sub(a.position, b.position));
		hit_info.depth = distance - a.radius - b.radius;
	} else {
		hit_info.hit = 0;
	}

	return hit_info;
}

void check_and_resolve_balls_2d(ball_2d *a, ball_2d *b) {
	collision_info_2d hit_info = check_ball_collision_2d(*a, *b);
	resolve_ball_collision_2d(a, b, hit_info);
}

void resolve_ball_collision_2d(ball_2d *a, ball_2d *b, collision_info_2d hit_info) {
	/*
	if(hit_info.hit) {
		vec2 a_velocity = v2_sub(a->position, a->previous_position);
		vec2 b_velocity = v2_sub(b->position, b->previous_position);

		a->position = v2_sub(a->position, v2_fmult(hit_info.normal, hit_info.depth * 0.5));
		b->position = v2_sub(b->position, v2_fmult(hit_info.normal, hit_info.depth * -0.5));
	}*/
	// not 100 percent sure if this is the best way
	if(hit_info.hit) {
		// failed elasticity stuff, cant figure out mass
		// ill add if/when i figure it out
		/*
		vec2 a_velocity = v2_sub(a->position, a->previous_position);
		vec2 b_velocity = v2_sub(b->position, b->previous_position);
		*/
		float inverse_mass_a = 1/a->mass;
		float inverse_mass_b = 1/b->mass;

		float inverse_inverse_mass_total = 1/(inverse_mass_a + inverse_mass_b);
		
		a->position = v2_sub(a->position, v2_fmult(hit_info.normal, hit_info.depth * inverse_mass_a * inverse_inverse_mass_total * 1));
		b->position = v2_sub(b->position, v2_fmult(hit_info.normal, hit_info.depth * -inverse_mass_b * inverse_inverse_mass_total * 1));
		// elastic collisions helped by my sister
		// more not working with mass elastic stuff
		/*

		float a_bounce_dot = -v2_dot(hit_info.normal, a_velocity);
		vec2 a_normal_bounce_velocity = v2_fmult(hit_info.normal, a_bounce_dot * a->mass);
		vec2 a_slide_velocity = v2_add(a_velocity, a_normal_bounce_velocity);

		float b_bounce_dot = -v2_dot(hit_info.normal, b_velocity);
		vec2 b_normal_bounce_velocity = v2_fmult(hit_info.normal, b_bounce_dot * b->mass); // minus because hit_info.normal should be reversed
		vec2 b_slide_velocity = v2_add(b_velocity, b_normal_bounce_velocity);
		
		float elasticity = 1;

		elasticity = elasticity * 0.5 + 0.5; // to make it 0.5 - 1 for 0.5 being the average between the two (no elasticity but not stopping instantly)

		vec2 a_bounce_velocity = v2_add(v2_fmult(b_normal_bounce_velocity, -(elasticity)), v2_fmult(a_normal_bounce_velocity, -(1-elasticity)));
		vec2 b_bounce_velocity = v2_add(v2_fmult(a_normal_bounce_velocity, -(elasticity)), v2_fmult(b_normal_bounce_velocity, -(1-elasticity)));

		//a_bounce_velocity = v2_fmult(a_bounce_velocity, inverse_mass_a);
		//b_bounce_velocity = v2_fmult(b_bounce_velocity, inverse_mass_b);

		vec2 a_out_velocity = v2_add(a_bounce_velocity, a_slide_velocity);
		vec2 b_out_velocity = v2_add(b_bounce_velocity, b_slide_velocity);

		// move it a little bit so it wont get stuck
		//a->position = v2_sub(a->position, v2_fmult(a_out_velocity, 0.01));
		//b->position = v2_sub(b->position, v2_fmult(a_out_velocity, 0.01));

		set_velocity_2d(a, a_out_velocity);
		set_velocity_2d(b, b_out_velocity);*/
		
	}
}

// 3d functions

void set_velocity_3d(ball_3d *body, vec3 velocity) {
	body->previous_position = v3_sub(body->position, velocity);
}

void update_ball_3d(ball_3d *body) {
	vec3 new_position = v3_sub(v3_fmult(body->position, 2), body->previous_position);
	body->previous_position = body->position;
	body->position = new_position;
}


collision_info_3d check_collision_3d(ball_3d a, wall_3d b) {
	collision_info_3d info;
	info.hit = 0;
	vec3 relative_a_position = v3_sub(a.position, b.vertex_a);
	vec3 relative_b_position = v3_sub(a.position, b.vertex_b);
	vec3 relative_c_position = v3_sub(a.position, b.vertex_c);

	float a_position_magnitude = v3_magnitude(relative_a_position);
	float b_position_magnitude = v3_magnitude(relative_b_position);
	float c_position_magnitude = v3_magnitude(relative_c_position);
	int intersecting_a = a_position_magnitude < a.radius;
	int intersecting_b = b_position_magnitude < a.radius;
	int intersecting_c = c_position_magnitude < a.radius;

	// the edge will do the corners as well
	/*
	if(intersecting_a) {
		info.hit = 1;
		info.depth = -a_position_magnitude + a.radius;
		info.normal = v3_normalize(relative_a_position);
	}

	if(intersecting_b) {
		info.hit = 1;
		info.depth = -b_position_magnitude + a.radius;
		info.normal = v3_normalize(relative_b_position);
	}

	if(intersecting_c) {
		info.hit = 1;
		info.depth = -c_position_magnitude + a.radius;
		info.normal = v3_normalize(relative_c_position);
	}
	*/

	// make a normal for each side and then check whether its inside 
	// super hard idk how to make that normal in 3d
	// maybe just like how it is in 2d?
	// i originally wanted to like project things along the triangles normal to and then to do a 2d triangle check
	// actually, the perpendicular trick with the -y, x or whatever it is wouldn't work in 3d
	// dangit
	vec3 a_b_edge = v3_sub(b.vertex_a, b.vertex_b);
	vec3 b_c_edge = v3_sub(b.vertex_b, b.vertex_c);
	vec3 c_a_edge = v3_sub(b.vertex_c, b.vertex_a);

	// how to check if intersecting line? hmmm
	// something with dot product probably
	
	// idea: some random ray intersection test thing and then the cross product with the line and some vector that will give a vector that points away being the normal
	// idk how tho

	// HOW??
	// idea: wall_normal = cross(b.normal, a_b_edge);
	// idk how position works though
	vec3 a_b_edge_normal = v3_normalize(v3_cross(b.normal, a_b_edge));
	vec3 b_c_edge_normal = v3_normalize(v3_cross(b.normal, b_c_edge));
	vec3 c_a_edge_normal = v3_normalize(v3_cross(b.normal, c_a_edge));

	int a_b_edge_side = v3_dot(relative_a_position, a_b_edge_normal) <= 0;
	int b_c_edge_side = v3_dot(relative_b_position, b_c_edge_normal) <= 0;
	int c_a_edge_side = v3_dot(relative_c_position, c_a_edge_normal) <= 0;
	// figured it out

	int in_triangle_plane = a_b_edge_side == b_c_edge_side && a_b_edge_side == c_a_edge_side;


	// my sister came up with this edge collision stuff
	// a_b_closest_point = a_b_side_length - (a_position_magnitude^2  - b_position_magnitude^2 - a_b_side_length^2)/-2 * a_b_side_length
	float a_b_side_length = v3_magnitude(a_b_edge);
	float a_b_closest_point_ratio = (a_b_side_length + (a_position_magnitude*a_position_magnitude - b_position_magnitude*b_position_magnitude - a_b_side_length*a_b_side_length)/(2 * a_b_side_length)) / a_b_side_length;
	if(a_b_closest_point_ratio < 0) a_b_closest_point_ratio = 0;
	if(a_b_closest_point_ratio > 1) a_b_closest_point_ratio = 1;
	vec3 a_b_closest_point = v3_lerp(b.vertex_a, b.vertex_b, a_b_closest_point_ratio);

	float b_c_side_length = v3_magnitude(b_c_edge);
	float b_c_closest_point_ratio = (b_c_side_length + (b_position_magnitude*b_position_magnitude - c_position_magnitude*c_position_magnitude - b_c_side_length*b_c_side_length)/(2 * b_c_side_length)) / b_c_side_length;
	if(b_c_closest_point_ratio < 0) b_c_closest_point_ratio = 0;
	if(b_c_closest_point_ratio > 1) b_c_closest_point_ratio = 1;
	vec3 b_c_closest_point = v3_lerp(b.vertex_b, b.vertex_c, b_c_closest_point_ratio);

	float c_a_side_length = v3_magnitude(c_a_edge);
	float c_a_closest_point_ratio = (c_a_side_length + (c_position_magnitude*c_position_magnitude - a_position_magnitude*a_position_magnitude - c_a_side_length*c_a_side_length)/(2 * c_a_side_length)) / c_a_side_length;
	if(c_a_closest_point_ratio < 0) c_a_closest_point_ratio = 0;
	if(c_a_closest_point_ratio > 1) c_a_closest_point_ratio = 1;
	vec3 c_a_closest_point = v3_lerp(b.vertex_c, b.vertex_a, c_a_closest_point_ratio);

	vec3 a_b_closest_relative = v3_sub(a.position, a_b_closest_point);
	vec3 b_c_closest_relative = v3_sub(a.position, b_c_closest_point);
	vec3 c_a_closest_relative = v3_sub(a.position, c_a_closest_point);

	float a_b_closest_magnitude = v3_magnitude(a_b_closest_relative);
	float b_c_closest_magnitude = v3_magnitude(b_c_closest_relative);
	float c_a_closest_magnitude = v3_magnitude(c_a_closest_relative);
	
	float closest_magnitude;
	vec3 closest_point;
	if(a_b_closest_magnitude < b_c_closest_magnitude) {
		closest_magnitude = a_b_closest_magnitude;
		closest_point = a_b_closest_relative;
	} else {
		closest_magnitude = b_c_closest_magnitude;
		closest_point = b_c_closest_relative;
	}

	if(c_a_closest_magnitude < closest_magnitude) {
		closest_magnitude = c_a_closest_magnitude;
		closest_point = c_a_closest_relative;
	}

	if(closest_magnitude < a.radius) {
		info.hit = 1;
		info.depth = -closest_magnitude + a.radius;
		info.normal = v3_normalize(closest_point);
		info.normal = v3_fmult(info.normal, 1);
	}

	float side_dot = v3_dot(relative_a_position, b.normal);
	int side = (side_dot <= 0) * 2 - 1;

	if(in_triangle_plane) {
		info.hit = in_triangle_plane && side*side_dot > -a.radius;
		info.depth = -side_dot - side * a.radius;
		info.normal = b.normal;
	}

	return info;
}

void resolve_collision_3d(ball_3d *body, collision_info_3d hit_info, float elasticity, float friction, float deltatime) {
	if(hit_info.hit) {
		vec3 velocity = v3_sub(body->position, body->previous_position);

		friction *= deltatime; // works for some reason, seemingly it should be squared or not at all but idk
		float speed = v3_magnitude(velocity);
		body->position = v3_sub(body->position, v3_fmult(hit_info.normal, -hit_info.depth));

		float bounce_dot = -v3_dot(hit_info.normal, velocity);
		vec3 bounce_velocity = v3_fmult(hit_info.normal, bounce_dot);
		vec3 slide_velocity = v3_add(velocity, bounce_velocity); // this is the velocity without the speed into the wall, so its the velocity which slides along it i think if i did this right

		vec3 out_velocity = v3_add(v3_fmult(slide_velocity, 1-friction), v3_fmult(bounce_velocity, elasticity));
		set_velocity_3d(body, out_velocity);
	}
}
void check_and_resolve_3d(ball_3d *body, wall_3d collider, float elasticity, float friction, float deltatime) {
	resolve_collision_3d(body, check_collision_3d(*body, collider), elasticity, friction, deltatime);
}

void distance_constraint_3d(ball_3d *a, ball_3d *b, float length) {
	float inverse_mass_a = 1/a->mass;
	float inverse_mass_b = 1/b->mass;
	vec3 delta = v3_sub(b->position, a->position);
	float deltalength = v3_magnitude(delta);
	float diff = (deltalength - length) / (deltalength*(inverse_mass_a + inverse_mass_b));
	a->position = v3_add(a->position, v3_fmult(delta, inverse_mass_a * diff));
	b->position = v3_sub(b->position, v3_fmult(delta, inverse_mass_b * diff));
}
void spring_constraint_3d(ball_3d *a, ball_3d *b, float length, float stiffness, float deltatime) {
	float inverse_mass_a = 1/a->mass;
	float inverse_mass_b = 1/b->mass;
	vec3 delta = v3_sub(a->position, b->position);
	float deltalength = v3_magnitude(delta);
	// F = -kx
	// k = stiffness
	// x = deltalength - length
	float force = -stiffness * (deltalength-length);

	vec3 delta_norm = v3_fdiv(delta, deltalength);

	// F = ma
	// a = F/m
	a->position = v3_add(a->position, v3_fmult(delta_norm, force * inverse_mass_a * deltatime*deltatime));
	b->position = v3_sub(b->position, v3_fmult(delta_norm, force * inverse_mass_b * deltatime*deltatime));
}

void rope_constraint_3d(ball_3d *a, ball_3d *b, float length) {
	vec3 relative_position = v3_sub(b->position, a->position);
	float relative_magnitude = v3_magnitude(relative_position);
	if(relative_magnitude < length) return;

	distance_constraint_3d(a, b, length);
}
void rope_spring_constraint_3d(ball_3d *a, ball_3d *b, float length, float stiffness, float deltatime) {
	vec3 relative_position = v3_sub(b->position, a->position);
	float relative_magnitude = v3_magnitude(relative_position);
	if(relative_magnitude < length) return;

	spring_constraint_3d(a, b, length, stiffness, deltatime);
}

void update_linkage_3d(linkage_3d link, float deltatime) {
	switch(link.type) {
		case DISTANCE:
			distance_constraint_3d(link.a, link.b, link.length);
			break;
		case SPRING:
			spring_constraint_3d(link.a, link.b, link.length, link.stiffness, deltatime);
			break;
		case ROPE:
			rope_constraint_3d(link.a, link.b, link.length);
			break;
		case ROPE_SPRING:
			rope_spring_constraint_3d(link.a, link.b, link.length, link.stiffness, deltatime);
			break;
		default:
			break;
	}
}

collision_info_3d check_ball_collision_3d(ball_3d a, ball_3d b) {
	float distance = v3_magnitude(v3_sub(a.position, b.position));
	collision_info_3d hit_info;
	//if(a.position.x + a.radius > b.position.x - b.radius && a.position.x - a.radius < b.position.x + b.radius /* x */ && a.position.y + a.radius > b.position.y - b.radius && a.position.y - a.radius < b.position.y + b.radius) {
		hit_info.hit = distance <= a.radius + b.radius;
		hit_info.normal = v3_normalize(v3_sub(a.position, b.position));
		hit_info.depth = distance - a.radius - b.radius;
	/*} else {
		hit_info.hit = 0;
	}*/

	return hit_info;

}
void resolve_ball_collision_3d(ball_3d *a, ball_3d *b, collision_info_3d hit_info) {
	// not 100 percent sure if this is the best way
	if(hit_info.hit) {
		vec3 a_velocity = v3_sub(a->position, a->previous_position);
		vec3 b_velocity = v3_sub(b->position, b->previous_position);

		float inverse_mass_a = 1/a->mass;
		float inverse_mass_b = 1/b->mass;

		float inverse_inverse_mass_total = 1/(inverse_mass_a + inverse_mass_b);

		a->position = v3_sub(a->position, v3_fmult(hit_info.normal, hit_info.depth * inverse_mass_a * inverse_inverse_mass_total));
		b->position = v3_sub(b->position, v3_fmult(hit_info.normal, hit_info.depth * -inverse_mass_b * inverse_inverse_mass_total));
	}
}
void check_and_resolve_balls_3d(ball_3d *a, ball_3d *b) {
	collision_info_3d hit_info = check_ball_collision_3d(*a, *b);
	resolve_ball_collision_3d(a, b, hit_info);
}
