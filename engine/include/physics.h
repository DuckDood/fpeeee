#pragma once
#include <types.h>

typedef struct {
	vec2 position;
	vec2 previous_position;
	float radius;
	float mass;
} ball_2d;

typedef struct {
	vec2 position;
	vec2 normal;
	float length;
} wall_2d;

typedef enum {
	DISTANCE,
	SPRING,
	ROPE,
	ROPE_SPRING
} linkage_types;

typedef struct {
	ball_2d *a;
	ball_2d *b;
	float length;
	float stiffness;
	linkage_types type;
} linkage_2d;

typedef struct {
	float depth;
	vec2 normal;
	int hit;
} collision_info_2d;

typedef struct {
	ball_2d *balls;
	int ball_count;
	linkage_2d *links;
	int link_count;
} shape_2d;

void set_velocity_2d(ball_2d *body, vec2 velocity);
collision_info_2d check_collision_2d(ball_2d a, wall_2d b);
void resolve_collision_2d(ball_2d *body, collision_info_2d hit_info, float elasticity, float friction, float deltatime);
void check_and_resolve_2d(ball_2d *body, wall_2d collider, float elasticity, float friction, float deltatime);
void update_ball_2d(ball_2d *body);

void distance_constraint_2d(ball_2d *a, ball_2d *b, float length);
void spring_constraint_2d(ball_2d *a, ball_2d *b, float length, float stiffness, float deltatime);
void rope_constraint_2d(ball_2d *a, ball_2d *b, float length);
void rope_spring_constraint_2d(ball_2d *a, ball_2d *b, float length, float stiffness, float deltatime);

void update_linkage_2d(linkage_2d link, float deltatime);

collision_info_2d check_ball_collision_2d(ball_2d * restrict a, ball_2d * restrict b);
void resolve_ball_collision_2d(ball_2d * restrict a, ball_2d * restrict b, collision_info_2d hit_info);
void check_and_resolve_balls_2d(ball_2d * restrict a, ball_2d * restrict b);

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
