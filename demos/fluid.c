#include <SDL3/SDL_oldnames.h>
#include <math.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <physics.h>
#include <matrix.h>
#include <shape_generators.h>
#include <stdio.h>
#include <stdlib.h>

#include <helpers.h>

#include <spatial.h>


#define WIDTH 1280
#define HEIGHT 720

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;

	camera cam;

	float deltatime;

	ball_2d *balls;
	int ball_count;

	spatial_grid_2d grid;

	Uint64 tick_count;
	int frame_count;
} prog_state;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
	*appstate = malloc(sizeof(prog_state));
	prog_state *state = *appstate;

	if(!SDL_Init(SDL_INIT_VIDEO)) {
		printf("Failed to initialize SDL3: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if(!SDL_CreateWindowAndRenderer("physics", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &state->window, &state->renderer)) {
		printf("Failed to create window or renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}


	state->cam.position = (vec3){0, 0, -1};
	state->cam.rotation = (vec3){0, 0, 0};

	state->cam.width = WIDTH;
	state->cam.height = HEIGHT;

	state->deltatime = 0;

	state->ball_count = 2000;
	state->balls = malloc(state->ball_count * sizeof(ball_2d));
	printf("ball mem usage (kb): %zu\n", state->ball_count * sizeof(ball_2d) / 1000);
	state->grid = construct_grid_2d(20, 20, 15, 0.1);

	int horizontal_max_spawn = 15;
	float ball_radius = 0.005;
	float spawn_height = -0.5;

	for(int i = 0; i < state->ball_count; ++i) {
		state->balls[i].position = (vec2){((i%horizontal_max_spawn) - (horizontal_max_spawn-1)/2.0) * ball_radius*2, spawn_height + ball_radius*2 * floor((float)i / horizontal_max_spawn)};
		set_velocity_2d(state->balls + i, (vec2){0, 0});
		state->balls[i].mass = 1;
		state->balls[i].radius = ball_radius;
	}

	state->balls[0].position.x += 0.001;

	state->tick_count = SDL_GetTicks();
	state->frame_count = 0;

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
	prog_state *app_state = appstate;
	SDL_DestroyRenderer(app_state->renderer);
	SDL_DestroyWindow(app_state->window);
	free(appstate);
	SDL_Quit();
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	prog_state *app_state = appstate;
	switch(event->type) {
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			app_state->cam.width = event->window.data1;
			app_state->cam.height = event->window.data2;
			break;
		default: 
			break;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	prog_state *state = appstate;

	Uint64 start_time = SDL_GetTicks();

	int framerate = 60;
	int steps_per_frame = 5;
	state->deltatime = 1.0/framerate/steps_per_frame;
	//state->deltatime = 0;

	SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
	SDL_RenderClear(state->renderer);

	SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 1);

	float aspect_ratio = (float)state->cam.width / state->cam.height;

	wall_2d floor;
	floor.length = aspect_ratio * 2;
	floor.normal = (vec2){0, 1};
	floor.position = (vec2){0, -1};

	wall_2d ceiling;
	ceiling.length = aspect_ratio * 2;
	ceiling.normal = (vec2){0, -1};
	ceiling.position = (vec2){0, 1};

	wall_2d left;
	left.length = 2;
	left.normal = (vec2){1, 0};
	left.position = (vec2){-aspect_ratio, 0};

	wall_2d right;
	right.length = 2;
	right.normal = (vec2){1, 0};
	right.position = (vec2){aspect_ratio, 0};

	float mouse_x, mouse_y;

	bool mouse_down = SDL_BUTTON_LMASK & SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_x /= state->cam.width;
	mouse_y /= -state->cam.height;

	mouse_x -= 0.5;
	mouse_y += 0.5;

	mouse_x *= 2;
	mouse_y *= 2;

	float mouse_radius = 0.5;
	float mouse_power = 10;

	vec2 mouse_position = (vec2){mouse_x, mouse_y};

	update_grid_2d(&state->grid, state->balls, state->ball_count);

	for(int steps = 0; steps < steps_per_frame; ++steps) {
		for(int i = 0; i < state->ball_count; ++i) {
			ball_2d *current_ball = state->balls + i;
			update_ball_2d(current_ball);
			current_ball->position.y -= 1 * state->deltatime * state->deltatime;

			if(mouse_down) {
				vec2 relative_to_mouse = v2_sub(current_ball->position, mouse_position);
				float distance_to_mouse = v2_magnitude(relative_to_mouse);
				if(distance_to_mouse < mouse_radius + current_ball->radius) {
					float total_mouse_power = mouse_power * (mouse_radius - distance_to_mouse + current_ball->radius) / mouse_radius;
					current_ball->position = v2_sub(current_ball->position, v2_fmult(v2_normalize(relative_to_mouse), total_mouse_power * state->deltatime * state->deltatime));
				}
			}
		}


		for(int i = 0; i < state->ball_count; ++i) {
			check_and_resolve_2d(state->balls+i, floor, 0, 0, state->deltatime);
			check_and_resolve_2d(state->balls+i, ceiling, 0, 0, state->deltatime);
			check_and_resolve_2d(state->balls+i, left, 0, 0, state->deltatime);
			check_and_resolve_2d(state->balls+i, right, 0, 0, state->deltatime);
		}

		
		
		
		update_grid_2d(&state->grid, state->balls, state->ball_count);
		
		/*
		for(int i = 0; i < state->ball_count; ++i) {
			for(int j = 0; j < state->ball_count; ++j) {
				if(i == j) continue;
				check_and_resolve_balls_2d(state->balls + i, state->balls+j);
			}
		}*/
		spatial_collision_2d(&state->grid, state->balls, state->ball_count);
	}

	for(int i = 0; i < state->ball_count; ++i) {
		draw_circle(state->renderer, state->balls[i], 25, state->cam);
	}
	draw_wall(state->renderer, floor, state->cam);
	draw_wall(state->renderer, ceiling, state->cam);
	draw_wall(state->renderer, left, state->cam);
	draw_wall(state->renderer, right, state->cam);

	ball_2d mouse_ball;
	mouse_ball.position = mouse_position;
	mouse_ball.radius = mouse_radius;
	draw_circle(state->renderer, mouse_ball, 25, state->cam);


	SDL_RenderPresent(state->renderer);

	Uint64 time_taken = SDL_GetTicks() - start_time;
	if(time_taken > 1000 / framerate) time_taken = 1000 / framerate;
	SDL_Delay(1000 / framerate - time_taken);

	state->frame_count++;

	if(SDL_GetTicks() > state->tick_count + 1000) {
		state->tick_count = SDL_GetTicks();
		printf("framerate: %i\n", state->frame_count);
		state->frame_count = 0;
	}

	return SDL_APP_CONTINUE;
}
