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

#define CLOTH_DIMENSIONS 20

//#define WIDTH 1280
//#define HEIGHT 720
#define WIDTH 600
#define HEIGHT 600

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;

	camera cam;

	float deltatime;

	ball_3d *balls;
	int ball_count;

	spatial_grid grid;

	Uint64 frame_tick_count;
	int frame_count;

	Uint64 spawn_tick_count;

	shape_3d cloth;
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


	state->cam.position = (vec3){0, 1, -6};
	state->cam.rotation = (vec3){0, 0, 0};

	state->cam.width = WIDTH;
	state->cam.height = HEIGHT;

	state->deltatime = 0;

	//state->ball_count = 1000;
	state->ball_count = 5;
	//state->ball_count = 130;
	state->balls = malloc(state->ball_count * sizeof(ball_3d));
	printf("ball mem usage (kb): %zu\n", state->ball_count * sizeof(ball_3d) / 1000);
	state->grid = construct_grid_3d(25, 50, 25, 0.3);


	state->balls[0].position = (vec3){-1, 0, -1};
	state->balls[1].position = (vec3){1, 0, -1};
	state->balls[2].position = (vec3){-1, 0, 1};
	state->balls[3].position = (vec3){1, 0, 1};

	//state->balls[4].position = (vec3){0, 3, 0};
	state->balls[4].position = (vec3){0, 3, 0};

	for(int i = 0; i < state->ball_count; ++i) {
		set_velocity_3d(state->balls + i, (vec3){0});
		state->balls[i].mass = 1;
		state->balls[i].radius = 0.15;
	}

	set_velocity_3d(state->balls + 4, (vec3){0, 0.01, 0});

	state->cloth = generate_cloth_3d(5, 5, CLOTH_DIMENSIONS, CLOTH_DIMENSIONS, 100, (vec3){0});
	//state->cloth.ball_count = 0;

	state->frame_tick_count = SDL_GetTicks();
	state->frame_count = 0;
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, [[maybe_unused]] SDL_AppResult result) {
	prog_state *state = appstate;
	destroy_grid_3d(&state->grid);
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
	int steps_per_frame = 10;
	state->deltatime = 1.0/framerate/steps_per_frame;
	//state->deltatime *= 0.5;
	if(key_states[SDL_SCANCODE_RETURN]) {
		if(SDL_GetTicks() > state->spawn_tick_count + 100) {
			int spawn_count = 1;
			int x = 0, y = 0, z = 0;
			int max_width = 5;
			float ball_radius = 0.15;
			float spawn_height = 5;
			state->spawn_tick_count = SDL_GetTicks();
			state->balls = realloc(state->balls, (state->ball_count += spawn_count) * sizeof(ball_3d));

			for(int i = 1; i <= spawn_count; ++i) {
				if(x >= max_width-1) {
					x = 0;
					z++;
				}
				if(z >= max_width-1) {
					z = 0;
					y++;
				}
				x++;
				//state->balls[state->ball_count-1].previous_position = (vec3){0, 10.01, 0};
				//state->balls[state->ball_count-1].position = (vec3){0, 10, 0};
				//state->balls[state->ball_count-1].mass = 1;
				//state->balls[state->ball_count-1].radius = 0.1;

				state->balls[state->ball_count-i].position = (vec3){(x-max_width*0.5) * 2 * ball_radius, spawn_height + y * 2 * ball_radius, (z-max_width*0.5) * 2 * ball_radius};
				set_velocity_3d(state->balls + state->ball_count - i, (vec3){0, -0.03, 0});
				state->balls[state->ball_count - i].mass = 1;
				state->balls[state->ball_count - i].radius = ball_radius;
				//state->balls[state->ball_count - i].radius = (rand() % 5 + 10)* 0.01;
			}
		}
	}

	SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
	SDL_RenderClear(state->renderer);

	SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 1);


	mat3 transform;
	transform = generate_rotation_matrix(0, 0, 0);
	transform = m3_mult(transform, generate_scale_matrix((vec3){1, 4, 1}));

	if(!key_states[SDL_SCANCODE_R]) {
		for(int i = 0; i < state->cloth.ball_count; ++i) {
			state->cloth.balls[i].previous_position = state->cloth.balls[i].position;
		}
		state->balls[4].previous_position = state->balls[4].position;
	}

	if(key_states[SDL_SCANCODE_LEFT]) {
		state->cam.rotation.x-=0.03;
	}
	if(key_states[SDL_SCANCODE_RIGHT]) {
		state->cam.rotation.x+=0.03;
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

	for(int steps = 0; steps < steps_per_frame; ++steps) {

		for(int i = 0; i < state->cloth.ball_count; ++i) {
			ball_3d *current_ball = state->cloth.balls + i;
			update_ball_3d(current_ball);
			current_ball->position.y -= 10 * state->deltatime * state->deltatime;
		}

		for(int i = 0; i < state->ball_count; ++i) {
			ball_3d *current_ball = state->balls + i;
			update_ball_3d(current_ball);
			current_ball->position.y -= 10 * state->deltatime * state->deltatime;
			//current_ball->position.y -= 9.8 * state->deltatime * state->deltatime;
		}
		state->balls[4].position.y -= 10 * state->deltatime * state->deltatime;
		for(int i = 0; i < state->ball_count; ++i) {
	//		check_and_resolve_3d(state->balls+i, bottom1, 0, 0, state->deltatime);
	//		check_and_resolve_3d(state->balls+i, bottom2, 0, 0, state->deltatime);
    //
	//		check_and_resolve_3d(state->balls+i, back1, 0, 0, state->deltatime);
	//		check_and_resolve_3d(state->balls+i, back2, 0, 0, state->deltatime);
    //
	//		check_and_resolve_3d(state->balls+i, front1, 0, 0, state->deltatime);
	//		check_and_resolve_3d(state->balls+i, front2, 0, 0, state->deltatime);
    //
	//		check_and_resolve_3d(state->balls+i, left1, 0, 0, state->deltatime);
	//		check_and_resolve_3d(state->balls+i, left2, 0, 0, state->deltatime);
    //
	//		check_and_resolve_3d(state->balls+i, right1, 0, 0, state->deltatime);
	//		check_and_resolve_3d(state->balls+i, right2, 0, 0, state->deltatime);
		}

		/*
		update_linkage_3d((linkage_3d){
				.a = state->balls + 0,
				.b = state->balls + 1,
				.length = 2,
				.type = DISTANCE
				}, state->deltatime);

		update_linkage_3d((linkage_3d){
				.a = state->balls + 1,
				.b = state->balls + 2,
				.length = sqrt(8),
				.type = DISTANCE
				}, state->deltatime);

		update_linkage_3d((linkage_3d){
				.a = state->balls + 0,
				.b = state->balls + 2,
				.length = 2,
				.type = DISTANCE
				}, state->deltatime);*/
		state->cloth.balls[0].position = (vec3){-2.5, 0, 2.5};
		state->cloth.balls[CLOTH_DIMENSIONS-1].position = (vec3){2.5, 0, 2.5};
        
		state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS].position = (vec3){-2.5, 0, -2.5};
		state->cloth.balls[state->cloth.ball_count-1].position = (vec3){2.5, 0, -2.5};
        
		for(int i = 0; i < state->cloth.link_count; ++i) {
				update_linkage_3d(state->cloth.links[i], state->deltatime);
		}
        
		//state->cloth.balls[0].position = (vec3){-2.5, 0, 5};
		//state->cloth.balls[CLOTH_DIMENSIONS-1].position = (vec3){2.5, 0, 5};
        
		//state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS].position = (vec3){-2.5, 0, 10};
		//state->cloth.balls[state->cloth.ball_count-1].position = (vec3){2.5, 0, 10};

		state->balls[0].position = (vec3){-1, 0, -1 + 3};
		state->balls[1].position = (vec3){1, 0,  -1 + 3};
		state->balls[2].position = (vec3){-1, 0,  1 + 3};
		state->balls[3].position = (vec3){1, 0,   1 + 3};

		collide_wall_3d(state->balls + 0, state->balls + 1, state->balls + 2, state->balls + 4);
		state->balls[0].position = (vec3){-1, 0, -1 + 3};
		state->balls[1].position = (vec3){1, 0,  -1 + 3};
		state->balls[2].position = (vec3){-1, 0,  1 + 3};
		state->balls[3].position = (vec3){1, 0,   1 + 3};
		collide_wall_3d(state->balls + 3, state->balls + 1, state->balls + 2, state->balls + 4);

		/*distance_constraint_3d(state->balls + 0, state->balls + 1, 2);
		distance_constraint_3d(state->balls + 0, state->balls + 2, 2);
		distance_constraint_3d(state->balls + 1, state->balls + 2, sqrt(8));

		distance_constraint_3d(state->balls + 3, state->balls + 1, 2);
		distance_constraint_3d(state->balls + 3, state->balls + 2, 2);
		distance_constraint_3d(state->balls + 1, state->balls + 2, sqrt(8));*/

		distance_constraint_3d(state->balls + 0, state->balls + 1, 2);
		distance_constraint_3d(state->balls + 0, state->balls + 2, 2);

		distance_constraint_3d(state->balls + 3, state->balls + 1, 2);
		distance_constraint_3d(state->balls + 3, state->balls + 2, 2);

		state->cloth.balls[0].position = (vec3){-2.5, 0, 2.5};
		state->cloth.balls[CLOTH_DIMENSIONS-1].position = (vec3){2.5, 0, 2.5};
        
		state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS].position = (vec3){-2.5, 0, -2.5};
		state->cloth.balls[state->cloth.ball_count-1].position = (vec3){2.5, 0, -2.5};
		
		/*
		for(int i = 0; i < state->ball_count; ++i) {
			for(int j = 0; j < state->ball_count; ++j) {
				if(i == j) continue;
				check_and_resolve_balls_3d(state->balls + i, state->balls + j);
			}
		}*/
		
		
		
		
		update_grid_3d(&state->grid, state->balls, state->ball_count);
		//spatial_collision_3d(&state->grid, state->balls, state->ball_count);
	
		//collide_wall_3d(&state->cloth.balls[0], &state->cloth.balls[CLOTH_DIMENSIONS-1], &state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS], state->balls + 4);
		//collide_wall_3d(&state->cloth.balls[state->cloth.ball_count-1], &state->cloth.balls[CLOTH_DIMENSIONS-1], &state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS], state->balls + 4);
		
	
		for(int row = 0; row < CLOTH_DIMENSIONS-1; ++row) {
			for(int i = 0; i < CLOTH_DIMENSIONS-1; ++i) {
				collide_wall_3d(
						&state->cloth.balls[i + CLOTH_DIMENSIONS*row],
						&state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row],
						&state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row],
						state->balls + 4);
				collide_wall_3d(
						&state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row],
						&state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row],
						&state->cloth.balls[i+CLOTH_DIMENSIONS+1 + CLOTH_DIMENSIONS*row],
						state->balls + 4);
			}
		}
	}


	
	
	/*
	SDL_SetRenderDrawColor(state->renderer, 123, 0, 0, 255);
	for(int i = 0; i < state->grid.depth; ++i) {
		for(int j = 0; j < state->grid.height; ++j) {
			for(int k = 0; k < state->grid.width; ++k) {
				//if(state->grid.partitions[i * state->grid.width * state->grid.height + j * state->grid.width + k].ball_count == 0) continue;
				vec3 line_1_1 = (vec3){
					state->grid.element_size*0.5 + state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size* (j-state->grid.height*0.5),
					state->grid.element_size * (i-state->grid.depth*0.5)
				};
				vec3 line_1_2 = (vec3){
					-state->grid.element_size*0.5 + state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size * (j-state->grid.height*0.5),
					state->grid.element_size * (i-state->grid.depth*0.5)
				};
				line_1_1 = point_to_screen(line_1_1, state->cam);
				line_1_2 = point_to_screen(line_1_2, state->cam);
				if(line_1_1.z < 0 || line_1_2.z < 0) continue;

				SDL_RenderLine(state->renderer, line_1_1.x, line_1_1.y, line_1_2.x, line_1_2.y);

				vec3 line_2_1 = (vec3){
					state->grid.element_size * (k-state->grid.width * 0.5),
					-state->grid.element_size*0.5 + state->grid.element_size* (j-state->grid.height*0.5),
					state->grid.element_size * (i-state->grid.depth*0.5)
				};
				vec3 line_2_2 = (vec3){
					state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size*0.5 + state->grid.element_size * (j-state->grid.height*0.5),
					state->grid.element_size * (i-state->grid.depth*0.5)
				};
				line_2_1 = point_to_screen(line_2_1, state->cam);
				line_2_2 = point_to_screen(line_2_2, state->cam);
				if(line_2_1.z < 0 || line_2_2.z < 0) continue;

				SDL_RenderLine(state->renderer, line_2_1.x, line_2_1.y, line_2_2.x, line_2_2.y);

				vec3 line_3_1 = (vec3){
					state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size* (j-state->grid.height*0.5),
					-state->grid.element_size * 0.5 + state->grid.element_size * (i-state->grid.depth*0.5)
				};
				vec3 line_3_2 = (vec3){
					state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size* (j-state->grid.height*0.5),
					state->grid.element_size * 0.5 + state->grid.element_size * (i-state->grid.depth*0.5)
				};
				line_3_1 = point_to_screen(line_3_1, state->cam);
				line_3_2 = point_to_screen(line_3_2, state->cam);
				if(line_3_1.z < 0 || line_3_2.z < 0) continue;

				SDL_RenderLine(state->renderer, line_3_1.x, line_3_1.y, line_3_2.x, line_3_2.y);
			}
		}
	}*/
	SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 255);
	for(int i = 0; i < state->ball_count; ++i) {
		draw_circle_3d(state->renderer, state->balls[i], 25, state->cam);
	}

	draw_wall_3d(state->renderer, (wall_3d) {
		.vertex_a = state->balls[0].position,
		.vertex_b = state->balls[1].position,
		.vertex_c = state->balls[2].position,
	}, state->cam);

	draw_wall_3d(state->renderer, (wall_3d) {
		.vertex_a = state->balls[3].position,
		.vertex_b = state->balls[1].position,
		.vertex_c = state->balls[2].position,
	}, state->cam);

	
	/*for(int i = 0; i < state->cloth.ball_count; ++i) {
		draw_circle_3d(state->renderer, state->cloth.balls[i], 25, state->cam);
	}
	for(int i = 0; i < state->cloth.link_count; ++i) {
		draw_linkage_3d(state->renderer, state->cloth.links[i], state->cam);
	}*/

	/*for(int row = 0; row < CLOTH_DIMENSIONS-1; ++row) {
		for(int i = 0; i < CLOTH_DIMENSIONS-1; ++i) {
			draw_wall_3d(state->renderer, (wall_3d) {
				.vertex_a = state->cloth.balls[i + CLOTH_DIMENSIONS*row].position,
				.vertex_b = state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position,
				.vertex_c = state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position,
			}, state->cam);
			draw_wall_3d(state->renderer, (wall_3d) {
				.vertex_a =	state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position,
				.vertex_b =	state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position,
				.vertex_c =	state->cloth.balls[i+CLOTH_DIMENSIONS+1 + CLOTH_DIMENSIONS*row].position,
			}, state->cam);
		}
	}*/

	for(int row = 0; row < CLOTH_DIMENSIONS-1; ++row) {
		for(int i = 0; i < CLOTH_DIMENSIONS-1; ++i) {
			
			draw_wall_3d(state->renderer, (wall_3d){
					.vertex_a = state->cloth.balls[i + CLOTH_DIMENSIONS*row].position,
					.vertex_b = state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position,
					.vertex_c = state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position}, state->cam
					);
			draw_wall_3d(state->renderer, (wall_3d){
					.vertex_a = state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position,
					.vertex_b = state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position,
					.vertex_c = state->cloth.balls[i+CLOTH_DIMENSIONS+1 + CLOTH_DIMENSIONS*row].position,
					}, state->cam
					);
		}
	}

	SDL_RenderPresent(state->renderer);

	/*vec3 momenta = {0};
	for(int i = 0; i < state->ball_count; ++i) {
		momenta = v3_add(momenta, v3_fmult(v3_sub(state->balls[i].position, state->balls[i].previous_position), state->balls[i].mass));
	}
	printf("momenta: %f\n", v3_magnitude(momenta));*/
	//printf("balls on the walls p1: %f, %f\n", state->balls[3].position.x, state->balls[3].position.y);

	Uint64 time_taken = SDL_GetTicks() - start_time;
	if(time_taken > (Uint64)1000 / framerate) time_taken = 1000 / framerate;
	SDL_Delay(1000 / framerate - time_taken);


	state->frame_count++;

	/*
	if(SDL_GetTicks() > state->frame_tick_count + 1000) {
		printf("framerate: %i\nball count: %i\n", state->frame_count, state->ball_count);
		if(state->frame_count < 60) {
			printf("wee woo wee woo\n");
		}
		state->frame_tick_count = SDL_GetTicks();
		state->frame_count = 0;

		//spatial_partition *bin_partition = state->grid.partitions + state->grid.width * state->grid.height * state->grid.depth;
		int push_back = 0;
		for(int i = 0; i < state->ball_count; ++i) {
			ball_3d *ball = state->balls + i;
			int ball_column = ball->position.x / state->grid.element_size + state->grid.width * 0.5;
			int ball_row = ball->position.y / state->grid.element_size + state->grid.height * 0.5;
			int ball_layer = ball->position.z / state->grid.element_size + state->grid.depth * 0.5;

			state->balls[i-push_back] = state->balls[i];

			if(ball_column < 0 || ball_column > state->grid.width - 1
				|| ball_row < 0 || ball_row > state->grid.height - 1
				|| ball_layer < 0 || ball_layer > state->grid.depth - 1)
				++push_back;
		}
		state->ball_count -= push_back;
		state->balls = realloc(state->balls, sizeof(ball_3d) * state->ball_count);
	}
	*/



	return SDL_APP_CONTINUE;
}
