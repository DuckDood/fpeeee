#include <SDL3/SDL_oldnames.h>
#include <math.h>
#include <sys/types.h>
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

	spatial_grid grid;

	Uint64 frame_tick_count;
	Uint64 spawn_tick_count;
	int frame_count;

	float spawn_delay;
	vec2 spawn_position;
} prog_state;

SDL_AppResult SDL_AppInit(void **appstate, [[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
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

	state->ball_count = 1000;
	//state->ball_count = 130;
	state->balls = malloc(state->ball_count * sizeof(ball_2d));
	printf("ball mem usage (kb): %zu\n", state->ball_count * sizeof(ball_2d) / 1000);
	printf("ball size: %zu\n", sizeof(ball_2d));
	state->grid = construct_grid_2d(400,200, 0.01);

	int horizontal_max_spawn = 100;
	float ball_radius = 0.005;
	float spawn_height = -0.5;

	for(int i = 0; i < state->ball_count; ++i) {
		state->balls[i].position = (vec2){((i%horizontal_max_spawn) - (horizontal_max_spawn-1)/2.0) * ball_radius*2, spawn_height + ball_radius*2 * floor((float)i / horizontal_max_spawn)};
		set_velocity_2d(state->balls + i, (vec2){0, 0});
		state->balls[i].mass = 0.5;
		state->balls[i].radius = ball_radius;
	}

	state->frame_tick_count = SDL_GetTicks();
	state->spawn_tick_count = SDL_GetTicks();
	state->frame_count = 0;
	state->spawn_delay = 250;
	state->spawn_position = (vec2){0,0};

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, [[maybe_unused]] SDL_AppResult result) {
	prog_state *state = appstate;
	destroy_grid_2d(&state->grid);
	free(state->balls);

	SDL_DestroyRenderer(state->renderer);
	SDL_DestroyWindow(state->window);
	free(appstate);
	SDL_Quit();
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	prog_state *state = appstate;
	switch(event->type) {
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
			break;

		case SDL_EVENT_WINDOW_RESIZED:
			state->cam.width = event->window.data1;
			state->cam.height = event->window.data2;
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			state->spawn_delay += event->wheel.y*5;
		default: 
			break;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	prog_state *state = appstate;

	Uint64 start_time = SDL_GetTicks();

	const bool * const key_states = SDL_GetKeyboardState(NULL);

	int framerate = 60;
	int steps_per_frame = 15;
	state->deltatime = 1.0/framerate/steps_per_frame;
	//state->deltatime = 0;

	SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
	SDL_RenderClear(state->renderer);

	SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 1);

	float aspect_ratio = (float)state->cam.width / state->cam.height;


	if(key_states[SDL_SCANCODE_LEFT]) {
		state->cam.rotation.x+=0.03;
	}
	if(key_states[SDL_SCANCODE_RIGHT]) {
		state->cam.rotation.x-=0.03;
	}
	if(key_states[SDL_SCANCODE_UP]) {
		state->cam.rotation.y+=0.03;
	}
	if(key_states[SDL_SCANCODE_DOWN]) {
		state->cam.rotation.y-=0.03;
	}
	if(key_states[SDL_SCANCODE_W]) {
		state->cam.position.z += 0.2 * cos(state->cam.rotation.x) * cos(state->cam.rotation.y);
		state->cam.position.x += 0.2 * sin(state->cam.rotation.x) * cos(state->cam.rotation.y);

		state->cam.position.y += 0.2 * sin(state->cam.rotation.y);
	}
	if(key_states[SDL_SCANCODE_S]) {
		state->cam.position.z -= 0.2 * cos(state->cam.rotation.x) * cos(state->cam.rotation.y);
		state->cam.position.x -= 0.2 * sin(state->cam.rotation.x) * cos(state->cam.rotation.y);

		state->cam.position.y -= 0.2 * sin(state->cam.rotation.y);
	}
	if(key_states[SDL_SCANCODE_D]) {
		state->cam.position.z -= 0.2 * sin(state->cam.rotation.x);
		state->cam.position.x += 0.2 * cos(state->cam.rotation.x);
	}
	if(key_states[SDL_SCANCODE_A]) {
		state->cam.position.z += 0.2 * sin(state->cam.rotation.x);
		state->cam.position.x -= 0.2 * cos(state->cam.rotation.x);
	}

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

	wall_2d another;
	another.length = 1;
	another.normal = (vec2){cos(1.7), sin(1.7)};
	another.position = (vec2){0.75, 0.1};
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

	vec2 mouse_position = (vec2){mouse_x * aspect_ratio, mouse_y};
	if(key_states[SDL_SCANCODE_P]) {
		state->spawn_position = mouse_position;
	}


	for(int steps = 0; steps < steps_per_frame; ++steps) {
	if(key_states[SDL_SCANCODE_RETURN]) {
		if(SDL_GetTicks() > state->spawn_tick_count + state->spawn_delay) {
			state->spawn_tick_count = SDL_GetTicks();
			state->balls = realloc(state->balls, ++state->ball_count * sizeof(ball_2d));
			state->balls[state->ball_count-1].previous_position = state->spawn_position;
			state->balls[state->ball_count-1].position = mouse_position;
			vec2 velocity = v2_sub(state->spawn_position, mouse_position);
			set_velocity_2d(&state->balls[state->ball_count-1], v2_fdiv(velocity, 100));
			state->balls[state->ball_count-1].mass = 1;
			state->balls[state->ball_count-1].radius = 0.005;
		}
	}
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
			check_and_resolve_2d(state->balls+i, another, 0, 0, state->deltatime);
		}

		
		
		
		update_grid_2d(&state->grid, state->balls, state->ball_count);
		
		spatial_collision_2d(&state->grid, state->balls, state->ball_count);
	}

	
	
	
	
	
	
	
	for(int i = 0; i < state->ball_count; i+=1) {
		draw_circle(state->renderer, state->balls[i], 6, state->cam);
	}
	draw_wall(state->renderer, floor, state->cam);
	draw_wall(state->renderer, ceiling, state->cam);
	draw_wall(state->renderer, left, state->cam);
	draw_wall(state->renderer, right, state->cam);
	draw_wall(state->renderer, another, state->cam);

	ball_2d mouse_ball;
	mouse_ball.position = mouse_position;
	mouse_ball.radius = mouse_radius;
	draw_circle(state->renderer, mouse_ball, 25, state->cam);
	ball_2d spawn_ball;
	spawn_ball.position = state->spawn_position;
	spawn_ball.radius = 0.001;
	draw_circle(state->renderer, spawn_ball, 25, state->cam);

	
	aspect_ratio = 1/aspect_ratio;
	
	
	
	
	/*
	for(int i = 0; i < state->grid.height; ++i) {
		for(int j = 0; j < state->grid.width; ++j) {
			if(state->grid.partitions[i * state->grid.width + j].ball_count == 0) continue;
			vec3 line_1_1 = (vec3){
				state->grid.element_size*0.5 + state->grid.element_size * (j-state->grid.width * 0.5),
				state->grid.element_size* (i-state->grid.height*0.5),
				0
			};
			vec3 line_1_2 = (vec3){
				-state->grid.element_size*0.5 + state->grid.element_size * (j-state->grid.width * 0.5),
				state->grid.element_size * (i-state->grid.height*0.5),
				0
			};
			line_1_1 = point_to_screen(line_1_1, state->cam);
			line_1_2 = point_to_screen(line_1_2, state->cam);
			if(line_1_1.z < 0 || line_1_2.z < 0) continue;

			SDL_RenderLine(state->renderer, line_1_1.x, line_1_1.y, line_1_2.x, line_1_2.y);

			vec3 line_2_1 = (vec3){
				state->grid.element_size * (j-state->grid.width * 0.5),
				-state->grid.element_size*0.5 + state->grid.element_size* (i-state->grid.height*0.5),
				0
			};
			vec3 line_2_2 = (vec3){
				state->grid.element_size * (j-state->grid.width * 0.5),
				state->grid.element_size*0.5 + state->grid.element_size * (i-state->grid.height*0.5),
				0
			};
			line_2_1 = point_to_screen(line_2_1, state->cam);
			line_2_2 = point_to_screen(line_2_2, state->cam);
			if(line_2_1.z < 0 || line_2_2.z < 0) continue;

			SDL_RenderLine(state->renderer, line_2_1.x, line_2_1.y, line_2_2.x, line_2_2.y);
		}
	}
	*/
	

	SDL_RenderPresent(state->renderer);


	Uint64 time_taken = SDL_GetTicks() - start_time;
	if(time_taken > (Uint64)1000 / framerate) time_taken = 1000 / framerate;
	SDL_Delay(1000 / framerate - time_taken);

	state->frame_count++;

	if(SDL_GetTicks() > state->frame_tick_count + 1000) {
		printf("framerate: %i\nball count: %i\n", state->frame_count, state->ball_count);
		state->frame_tick_count = SDL_GetTicks();
		state->frame_count = 0;
	}


	return SDL_APP_CONTINUE;
}
