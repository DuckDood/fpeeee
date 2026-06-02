#pragma once
#include <types.h>

collision_info check_collision(ball a, wall b);
void resolve_collision(ball *body, collision_info hit_info);
void check_and_resolve(ball *body, wall collider);
void update_ball(ball *body);
void distance_constraint(ball *a, ball *b, float length);
void spring_constraint(ball *a, ball *b, float length);
