#pragma once

#include <physics.h>
#include <SDL3/SDL.h>

typedef struct {
	vec3 position;
	vec3 rotation;
	int width;
	int height;
} camera;

vec3 point_to_screen(vec3 position, camera cam);

void draw_circle(SDL_Renderer *renderer, ball_2d b, int resolution, camera cam);
void draw_circle_3d(SDL_Renderer *renderer, ball_3d b, int resolution, camera cam);
void draw_wall(SDL_Renderer *renderer, wall_2d w, camera cam);
void draw_wall_3d(SDL_Renderer *renderer, wall_3d w, camera cam);
void draw_linkage(SDL_Renderer *renderer, linkage_2d link, camera cam);
void draw_linkage_3d(SDL_Renderer *renderer, linkage_3d link, camera cam);
void set_wall_normal(wall_3d *w);

