#pragma once
#include <types.h>

void set_velocity(ball *body, vec2 velocity);
collision_info check_collision(ball a, wall b);
void resolve_collision(ball *body, collision_info hit_info, float deltatime);
void check_and_resolve(ball *body, wall collider, float deltatime);
void update_ball(ball *body);
void distance_constraint(ball *a, ball *b, float length);
void spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime);
void rope_constraint(ball *a, ball *b, float length);
void rope_spring_constraint(ball *a, ball *b, float length, float stiffness, float deltatime);

collision_info check_ball_collision(ball a, ball b);
void resolve_ball_collision(ball *a, ball *b, collision_info hit_info);
void check_and_resolve_balls(ball *a, ball *b);
