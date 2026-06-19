#pragma once
#include <types.h>

typedef struct {
	vec2 position;
	vec2 previous_position;
	float radius;
	float mass;
} ball;

typedef struct {
	vec2 position;
	vec2 normal;
	float length;
} wall;

typedef enum {
	DISTANCE,
	SPRING,
	ROPE,
	ROPE_SPRING
} linkage_types;

typedef struct {
	ball *a;
	ball *b;
	float length;
	float stiffness;
	linkage_types type;
} linkage;

typedef struct {
	float depth;
	vec2 normal;
	int hit;
} collision_info;

typedef struct {
	ball *balls;
	int ball_count;
	linkage *links;
	int link_count;
} shape;

void set_velocity(ball *body, vec2 velocity);
collision_info check_collision(ball a, wall b);
void resolve_collision(ball *body, collision_info hit_info, float elasticity, float friction, float deltatime);
void check_and_resolve(ball *body, wall collider, float elasticity, float friction, float deltatime);
void update_ball(ball *body);

void distance_constraint(ball *a, ball *b, float length);
void spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime);
void rope_constraint(ball *a, ball *b, float length);
void rope_spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime);

void update_linkage(linkage link, float deltatime);

collision_info check_ball_collision(ball a, ball b);
void resolve_ball_collision(ball *a, ball *b, collision_info hit_info);
void check_and_resolve_balls(ball *a, ball *b);


typedef struct {
	vec3 position;
	vec3 previous_position;
	float radius;
	float mass;
} ball_3d;

typedef struct {
	vec3 vertex_a;
	vec3 vertex_b;
	vec3 vertex_c;

	vec3 normal;
} wall_3d;

typedef struct {
	ball_3d *a;
	ball_3d *b;
	float length;
	float stiffness;
	linkage_types type;
} linkage_3d;

typedef struct {
	float depth;
	vec3 normal;
	int hit;
} collision_info_3d;

typedef struct {
	ball_3d *balls;
	int ball_count;
	linkage_3d *links;
	int link_count;
} shape_3d;

void set_velocity_3d(ball_3d *body, vec3 velocity);
collision_info_3d check_collision_3d(ball_3d a, wall_3d b);
void resolve_collision_3d(ball_3d *body, collision_info_3d hit_info, float elasticity, float friction, float deltatime);
void check_and_resolve_3d(ball_3d *body, wall_3d collider, float elasticity, float friction, float deltatime);
void update_ball_3d(ball_3d *body);

void distance_constraint_3d(ball_3d *a, ball_3d *b, float length);
void spring_constraint_3d(ball_3d *a, ball_3d *b, float length, float stiffness, float deltatime);
void rope_constraint_3d(ball_3d *a, ball_3d *b, float length);
void rope_spring_constraint_3d(ball_3d *a, ball_3d *b, float length, float stiffness, float deltatime);

void update_linkage_3d(linkage_3d link, float deltatime);

collision_info_3d check_ball_collision_3d(ball_3d a, ball_3d b);
void resolve_ball_collision_3d(ball_3d *a, ball_3d *b, collision_info_3d hit_info);
void check_and_resolve_balls_3d(ball_3d *a, ball_3d *b);
