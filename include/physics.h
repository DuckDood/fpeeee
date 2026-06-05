#pragma once
#include <types.h>

typedef struct {
	vec2 position;
	vec2 previous_position;
	float radius;
} ball;

typedef struct {
	vec2 position;
	vec2 normal;
	float length;
} wall;

typedef struct {
	float depth;
	vec2 normal;
	int hit;
} collision_info;

void set_velocity(ball *body, vec2 velocity);
collision_info check_collision(ball a, wall b);
void resolve_collision(ball *body, collision_info hit_info, float elasticity, float friction, float deltatime);
void check_and_resolve(ball *body, wall collider, float elasticity, float friction, float deltatime);
void update_ball(ball *body);
void distance_constraint(ball *a, ball *b, float length);
void spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime);
void rope_constraint(ball *a, ball *b, float length);
void rope_spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime);

collision_info check_ball_collision(ball a, ball b);
void resolve_ball_collision(ball *a, ball *b, collision_info hit_info);
void check_and_resolve_balls(ball *a, ball *b);
