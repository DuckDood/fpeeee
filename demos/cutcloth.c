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

#define CLOTH_RESOLUTION 75
#define CLOTH_WIDTH 1.5
#define CLOTH_STIFFNESS 5

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;

	camera cam;

	float deltatime;

	Uint64 frame_tick_count;
	Uint64 spawn_tick_count;
	int frame_count;

	shape_2d cloth;
	
	spatial_grid collision_grid;

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

	SDL_SetRenderVSync(state->renderer, 1);


	state->cam.position = (vec3){0, 0, -1};
	state->cam.rotation = (vec3){0, 0, 0};

	state->cam.width = WIDTH;
	state->cam.height = HEIGHT;

	state->deltatime = 0;

	state->frame_tick_count = SDL_GetTicks();
	state->spawn_tick_count = SDL_GetTicks();
	state->frame_count = 0;

	state->cloth = generate_cloth(CLOTH_WIDTH, CLOTH_WIDTH, CLOTH_RESOLUTION, CLOTH_RESOLUTION, CLOTH_STIFFNESS, (vec2){ 0, 0}, 0.5);

	state->collision_grid = construct_grid_2d(250, 150, 0.02);

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, [[maybe_unused]] SDL_AppResult result) {
	prog_state *state = appstate;
	free_shape_2d(state->cloth);

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
		case SDL_EVENT_KEY_DOWN:
			switch(event->key.key) {
				case SDLK_R:
					free_shape_2d(state->cloth);
					state->cloth = generate_cloth(CLOTH_WIDTH, CLOTH_WIDTH, CLOTH_RESOLUTION, CLOTH_RESOLUTION, CLOTH_STIFFNESS, (vec2){ 0, 0}, 0.5);
				default:
					break;
			}
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
	int steps_per_frame = 5;
	state->deltatime = 1.0/framerate/steps_per_frame;

	SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
	SDL_RenderClear(state->renderer);

	SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 1);

	float aspect_ratio = (float)state->cam.width / state->cam.height;

	if(key_states[SDL_SCANCODE_R]) {
	}

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

	float mouse_x, mouse_y;


	SDL_MouseButtonFlags mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	bool mouse_down = SDL_BUTTON_LMASK & mouse_buttons;
	bool mouse_right_down = SDL_BUTTON_RMASK & mouse_buttons;


	mouse_x /= state->cam.width;
	mouse_y /= -state->cam.height;

	mouse_x -= 0.5;
	mouse_y += 0.5;

	mouse_x *= 2;
	mouse_y *= 2;

	mouse_x *= aspect_ratio;
	vec2 mouse_vec = {mouse_x, mouse_y};

	for(int steps = 0; steps < steps_per_frame; ++steps) {
		for(int i = 0; i < state->cloth.ball_count; ++i) {
			ball_2d *current_ball = state->cloth.balls + i;
			update_ball_2d(current_ball);
			current_ball->position.y -= 1 * state->deltatime * state->deltatime;

		}
		if(mouse_down) {
			for(int i = 0; i < state->cloth.link_count; ++i) {/*
				vec2 link_line = v2_normalize(v2_sub(state->cloth.links[i].a->position, state->cloth.links[i].b->position));
				float link_line_dist = v2_magnitude(v2_sub(state->cloth.links[i].a->position, state->cloth.links[i].b->position));
				vec2 link_line_normal = {link_line.y, -link_line.x}; // which way goes where doesnt really matter here

				float side_1 = v2_dot(link_line_normal, v2_sub((vec2){mouse_x, mouse_y}, state->cloth.links[i].a->position));
				float side_2 = v2_dot(link_line_normal, v2_sub((vec2){rel_mx, rel_my}, state->cloth.links[i].a->position));

				float edge = v2_dot(link_line, v2_sub((vec2){mouse_x, mouse_y}, v2_lerp(state->cloth.links[i].a->position, state->cloth.links[i].b->position, 0.5)));
				
				if(edge > link_line_dist/2 || edge < -link_line_dist/2) continue;

				if(side_1 * side_2 < 0) {
					state->cloth.links[i].stiffness = 0;
				}*/

				/*vec2 avg_position = v2_lerp(state->cloth.links[i].a->position, state->cloth.links[i].b->position, 0.5);
				float dist = v2_magnitude(v2_sub(avg_position, (vec2){mouse_x, mouse_y}));
				if(dist < 0.01)
				state->cloth.links[i].stiffness = 0;*/
				vec2 mouse_vec = {mouse_x, mouse_y};
				vec2 relative_a_position = v2_sub(mouse_vec, state->cloth.links[i].a->position);
				vec2 relative_b_position = v2_sub(mouse_vec, state->cloth.links[i].b->position);
			
				float a_position_magnitude = v2_magnitude(relative_a_position);
				float b_position_magnitude = v2_magnitude(relative_b_position);
			
				vec2 a_b_edge = v2_sub(state->cloth.links[i].a->position, state->cloth.links[i].b->position);
				float a_b_side_length = v2_magnitude(a_b_edge);
				float a_b_closest_point_ratio = (a_b_side_length + (a_position_magnitude*a_position_magnitude - b_position_magnitude*b_position_magnitude - a_b_side_length*a_b_side_length)/(2 * a_b_side_length)) / a_b_side_length;
				if(a_b_closest_point_ratio < 0) a_b_closest_point_ratio = 0;
				if(a_b_closest_point_ratio > 1) a_b_closest_point_ratio = 1;
				vec2 closest_position = v2_lerp(state->cloth.links[i].a->position, state->cloth.links[i].b->position, a_b_closest_point_ratio);

				float dist = v2_magnitude(v2_sub(closest_position, (vec2){mouse_x, mouse_y}));
				if(dist < 0.01)
				state->cloth.links[i].stiffness = 0;
			}
		}

		if(mouse_right_down) {
			for(int i = 0; i < state->cloth.ball_count; ++i) {
				vec2 relative_position = v2_sub(state->cloth.balls[i].position,mouse_vec);
				float relative_magnitude = v2_magnitude(relative_position);
				float mouse_radius = 1;
				float mouse_power = 10;
				float power = mouse_power * state->deltatime * state->deltatime * -sqrt((mouse_radius - relative_magnitude)/(mouse_radius*mouse_radius));
				if(relative_magnitude < mouse_radius) {
					#define LERP(a, b, t) a*(1-t)+b*t
					state->cloth.balls[i].position = v2_add(state->cloth.balls[i].position, v2_fmult(relative_position, power));
					#undef LERP
				}
			}
		}

		for(int i = 0; i < state->cloth.ball_count; ++i) {
			check_and_resolve_2d(state->cloth.balls+i, floor, 0, 0, state->deltatime);
			check_and_resolve_2d(state->cloth.balls+i, ceiling, 0, 0, state->deltatime);
			check_and_resolve_2d(state->cloth.balls+i, left, 0, 0, state->deltatime);
			check_and_resolve_2d(state->cloth.balls+i, right, 0, 0, state->deltatime);
		}
		for(int i = 0; i < CLOTH_RESOLUTION; ++i) {
			state->cloth.balls[CLOTH_RESOLUTION*CLOTH_RESOLUTION - 1 - i].position = (vec2){((CLOTH_RESOLUTION-1)*0.5-i) * (CLOTH_WIDTH/(CLOTH_RESOLUTION-1)) - 0.013, CLOTH_WIDTH/2 - 0.013}; // ?????
		}

		for(int i = 0; i < state->cloth.link_count; ++i) {
			update_linkage_2d(state->cloth.links[i], state->deltatime);
		}

		update_grid_2d(&state->collision_grid, state->cloth.balls, state->cloth.ball_count);
		spatial_collision_2d(&state->collision_grid, state->cloth.balls, state->cloth.ball_count);
	}

	
	
	
	
	
	
	
	for(int i = 0; i < state->cloth.ball_count; i+=1) {
		//draw_circle(state->renderer, state->cloth.balls[i], 6, state->cam);
	}

	for(int i = 0; i < state->cloth.link_count; i+=1) {
		if(state->cloth.links[i].stiffness == 0) continue;
		draw_linkage(state->renderer, state->cloth.links[i], state->cam);
	}

	draw_wall(state->renderer, floor, state->cam);
	draw_wall(state->renderer, ceiling, state->cam);
	draw_wall(state->renderer, left, state->cam);
	draw_wall(state->renderer, right, state->cam);

	aspect_ratio = 1/aspect_ratio;
	
	
	
	
	SDL_RenderPresent(state->renderer);


	Uint64 time_taken = SDL_GetTicks() - start_time;
	if(time_taken > (Uint64)1000 / framerate) time_taken = 1000 / framerate;
	//SDL_Delay(1000 / framerate - time_taken);

	state->frame_count++;

	if(SDL_GetTicks() > state->frame_tick_count + 1000) {
		printf("framerate: %i\n", state->frame_count);
		fflush(stdout);
		state->frame_tick_count = SDL_GetTicks();
		state->frame_count = 0;
	}


	return SDL_APP_CONTINUE;
}
