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

#include <constraints.h>

int fps = 0;
int ball_count = 0;

char optionstr[256];

char *get_option() {
	snprintf((char*)optionstr, sizeof(optionstr), "Controls:\nEnter to spawn balls\nWASD to move\nArrow keys to rotate camera (sorry)\n\nFPS: %i, Number of balls: %i", fps, ball_count);
	return optionstr;
}

void split_draw_debug(SDL_Renderer *renderer) {
	get_option();
	char *split;
	split = strtok(optionstr, "\n");
	for(int i = 5; split != NULL; i+=10) {
		SDL_RenderDebugText(renderer, 5, i, split);
		split = strtok(NULL, "\n");
	}

}

#define WIDTH 1280
#define HEIGHT 720

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


	state->cam.position = (vec3){0, 1, -3};
	state->cam.rotation = (vec3){0, 0, 0};

	state->cam.width = WIDTH;
	state->cam.height = HEIGHT;

	state->deltatime = 0;

	//state->ball_count = 1000;
	state->ball_count = 2;
	//state->ball_count = 130;
	state->balls = malloc(state->ball_count * sizeof(ball_3d));
	printf("ball mem usage (kb): %zu\n", state->ball_count * sizeof(ball_3d) / 1000);
	state->grid = construct_grid_3d(25, 50, 25, 0.3);

	float ball_radius = 0.15;
	float spawn_height = 3;

	int x = 0, y = 0, z = 0;
	int max_width = 11;

	for(int i = 0; i < state->ball_count; ++i) {
		if(x >= max_width-1) {
			x = 0;
			z++;
		}
		if(z >= max_width-1) {
			z = 0;
			y++;
		}
		x++;
		//state->balls[i].position = (vec2){((i%horizontal_max_spawn) - (horizontal_max_spawn-1)/2.0) * ball_radius*2, spawn_height + ball_radius*2 * floor((float)i / horizontal_max_spawn)};
		state->balls[i].position = (vec3){(x-max_width*0.5) * 2 * ball_radius, spawn_height + y * 2 * ball_radius, (z-max_width*0.5) * 2 * ball_radius};
		set_velocity_3d(state->balls + i, (vec3){0, 0, 0});
		state->balls[i].mass = 1;
		state->balls[i].radius = ball_radius;
	}

	state->frame_tick_count = SDL_GetTicks();
	state->frame_count = 0;
	//state->balls[0].position = (vec3){sin(0), 10, cos(0)};
	//state->balls[1].position = (vec3){sin(3.141 * 2 / 3), 110, cos(3.141 * 2 / 3)};
	//state->balls[2].position = (vec3){sin(3.141 * 4 / 3), -1, cos(3.141 * 4 / 3)};
    //
	//state->balls[3].position = (vec3){-0.6, 3, 0};
    //
	//state->balls[3].velocity = (vec3){0, -2, 0};
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
	int steps_per_frame = 5;
	state->deltatime = 1.0/framerate/steps_per_frame;
	//state->deltatime = 0;
	if(key_states[SDL_SCANCODE_RETURN]) {
		if(SDL_GetTicks() > state->spawn_tick_count + 100) {
			int spawn_count = 22;
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

				state->balls[state->ball_count-i].position = (vec3){rand() * 0.0000000000000001 + (x-max_width*0.51) * 2 * ball_radius, spawn_height + y * 2 * ball_radius, (z-max_width*0.51) * 2 * ball_radius};
				set_velocity_3d(state->balls + state->ball_count - i, (vec3){0, -3, 0});
				state->balls[state->ball_count - i].mass = 1;
				state->balls[state->ball_count - i].radius = ball_radius;
				//state->balls[state->ball_count - i].radius = (rand() % 5 + 10)* 0.01;
			}
		}
	}

	SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
	SDL_RenderClear(state->renderer);

	SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 255);


	mat3 transform;
	transform = generate_rotation_matrix(0, 0, 0);
	transform = m3_mult(transform, generate_scale_matrix((vec3){1, 1, 1}));

	wall_3d bottom1 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){-2, 0, -2}),
		.vertex_b = m3_v3_mult(transform, (vec3){-2, 0, 2}),
		.vertex_c = m3_v3_mult(transform, (vec3){2, 0, 2}),
	};
	set_wall_normal(&bottom1);

	wall_3d bottom2 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){-2, 0, -2}),
		.vertex_b = m3_v3_mult(transform, (vec3){2, 0, -2}),
		.vertex_c = m3_v3_mult(transform, (vec3){2, 0, 2}),
	};
	set_wall_normal(&bottom2);


	wall_3d back1 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){-2, 0, 2}),
		.vertex_b = m3_v3_mult(transform, (vec3){2, 0, 2}),
		.vertex_c = m3_v3_mult(transform, (vec3){2, 1, 2}),
	};
	set_wall_normal(&back1);

	wall_3d back2 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){-2, 1, 2}),
		.vertex_b = m3_v3_mult(transform, (vec3){2, 1, 2}),
		.vertex_c = m3_v3_mult(transform, (vec3){-2, 0, 2}),
	};
	set_wall_normal(&back2);


	wall_3d front1 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){-2, 0, -2}),
		.vertex_b = m3_v3_mult(transform, (vec3){2, 0, -2}),
		.vertex_c = m3_v3_mult(transform, (vec3){2, 1, -2}),
	};
	set_wall_normal(&front1);

	wall_3d front2 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){-2, 1, -2}),
		.vertex_b = m3_v3_mult(transform, (vec3){2, 1, -2}),
		.vertex_c = m3_v3_mult(transform, (vec3){-2, 0, -2}),
	};
	set_wall_normal(&front2);


	wall_3d left1 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){-2, 0, -2}),
		.vertex_b = m3_v3_mult(transform, (vec3){-2, 1, -2}),
		.vertex_c = m3_v3_mult(transform, (vec3){-2, 1, 2}),
	};
	set_wall_normal(&left1);

	wall_3d left2 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){-2, 0, -2}),
		.vertex_b = m3_v3_mult(transform, (vec3){-2, 1, 2}),
		.vertex_c = m3_v3_mult(transform, (vec3){-2, 0, 2}),
	};
	set_wall_normal(&left2);


	wall_3d right1 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){2, 0, -2}),
		.vertex_b = m3_v3_mult(transform, (vec3){2, 1, -2}),
		.vertex_c = m3_v3_mult(transform, (vec3){2, 1, 2}),
	};
	set_wall_normal(&right1);

	wall_3d right2 = (wall_3d) {
		.vertex_a = m3_v3_mult(transform, (vec3){2, 0, -2}),
		.vertex_b = m3_v3_mult(transform, (vec3){2, 1, 2}),
		.vertex_c = m3_v3_mult(transform, (vec3){2, 0, 2}),
	};
	set_wall_normal(&right2);



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
		for(int i = 0; i < state->ball_count; ++i) {
			ball_3d *current_ball = state->balls + i;
			current_ball->velocity.y -= 9.8 * state->deltatime;

			//current_ball->velocity.y -= 1 * state->deltatime;
			begin_ball_update_3d(current_ball, state->deltatime);
		}
		/*for(int i = 0; i < state->ball_count; ++i) {
			check_and_resolve_3d(state->balls+i, bottom1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, bottom2, 0, 0, state->deltatime);

			check_and_resolve_3d(state->balls+i, back1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, back2, 0, 0, state->deltatime);

			check_and_resolve_3d(state->balls+i, front1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, front2, 0, 0, state->deltatime);

			check_and_resolve_3d(state->balls+i, left1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, left2, 0, 0, state->deltatime);

			check_and_resolve_3d(state->balls+i, right1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, right2, 0, 0, state->deltatime);
		}*/
		for(int i = 0; i < state->ball_count; ++i) {
		/*	check_and_resolve_3d(state->balls+i, bottom1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, bottom2, 0, 0, state->deltatime);

			check_and_resolve_3d(state->balls+i, back1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, back2, 0, 0, state->deltatime);

			check_and_resolve_3d(state->balls+i, front1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, front2, 0, 0, state->deltatime);

			check_and_resolve_3d(state->balls+i, left1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, left2, 0, 0, state->deltatime);

			check_and_resolve_3d(state->balls+i, right1, 0, 0, state->deltatime);
			check_and_resolve_3d(state->balls+i, right2, 0, 0, state->deltatime);*/
			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &bottom1.vertex_a, &bottom1.vertex_b, &bottom1.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});
			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &bottom2.vertex_a, &bottom2.vertex_b, &bottom2.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});

			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &back1.vertex_a, &back1.vertex_b, &back1.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});
			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &back2.vertex_a, &back2.vertex_b, &back2.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});

			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &front1.vertex_a, &front1.vertex_b, &front1.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});
			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &front2.vertex_a, &front2.vertex_b, &front2.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});

			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &left1.vertex_a, &left1.vertex_b, &left1.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});
			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &left2.vertex_a, &left2.vertex_b, &left2.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});

			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &right1.vertex_a, &right1.vertex_b, &right1.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});
			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &right2.vertex_a, &right2.vertex_b, &right2.vertex_c},
					(float[]){1,0,0,0}, 4, (float[]){state->balls[0].radius});
		/*	for(int j = 0; j < state->ball_count; ++j) {
				if(i==j) continue;
				ball_3d *ball = state->balls + i;
				ball_3d *check_ball = state->balls + j;
				//solve_constraint_3d(dist_constraint_3d, (del_constraint_function_3d[]){dist_constraint_del_a_3d, dist_constraint_del_b_3d}, INEQ_GREATER, (vec3*[]){&state->balls[i].position, &state->balls[j].position}, (float[]){1/state->balls[i].mass, 1/state->balls[j].mass}, 2, (float[]){0.3});
				solve_constraint_3d(dist_constraint_3d, (del_constraint_function_3d[]){dist_constraint_del_a_3d, dist_constraint_del_b_3d}, INEQ_GREATER, (vec3*[]){&ball->position, &check_ball->position}, (float[]){1/ball->mass, 1/check_ball->mass}, 2, (float[]){ball->radius + check_ball->radius});
			}*/
		}   

		/*solve_constraint_3d(dist_constraint_3d, (del_constraint_function_3d[]){dist_constraint_del_a_3d, dist_constraint_del_b_3d}, EQUALITY, (vec3*[]){&state->balls[0].position, &state->balls[1].position}, (float[]){1/state->balls[0].mass, 1/state->balls[1].mass}, 2, (float[]){2});
		solve_constraint_3d(dist_constraint_3d, (del_constraint_function_3d[]){dist_constraint_del_a_3d, dist_constraint_del_b_3d}, EQUALITY, (vec3*[]){&state->balls[1].position, &state->balls[2].position}, (float[]){1/state->balls[0].mass, 1/state->balls[1].mass}, 2, (float[]){2});
		solve_constraint_3d(dist_constraint_3d, (del_constraint_function_3d[]){dist_constraint_del_a_3d, dist_constraint_del_b_3d}, EQUALITY, (vec3*[]){&state->balls[0].position, &state->balls[2].position}, (float[]){1/state->balls[0].mass, 1/state->balls[1].mass}, 2, (float[]){2});
		solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
				(vec3*[]){&state->balls[3].position, &state->balls[0].position, &state->balls[1].position, &state->balls[2].position},
				(float[]){1,1,1,1}, 4, (float[]){state->balls[3].radius});
		for(int i = 4; i < state->ball_count; ++i) {
			solve_constraint_3d(wall_constraint_3d, (del_constraint_function_3d[]){wall_constraint_del_body_3d, wall_constraint_del_a_3d, wall_constraint_del_b_3d, wall_constraint_del_c_3d}, INEQ_GREATER, 
					(vec3*[]){&state->balls[i].position, &state->balls[0].position, &state->balls[1].position, &state->balls[2].position},
					(float[]){1,1,1,1}, 4, (float[]){state->balls[3].radius});
		}*/
		//state->balls[0].position = (vec3){0};
		//solve_constraint_3d(dist_constraint_3d, (del_constraint_function_3d[]){dist_constraint_del_a_3d, dist_constraint_del_b_3d}, INEQ_GREATER, (vec3*[]){&state->balls[0].position, &state->balls[1].position}, (float[]){1/state->balls[0].mass, 1/state->balls[1].mass}, 2, (float[]){2});

		
		/*
		for(int i = 0; i < state->ball_count; ++i) {
			for(int j = 0; j < state->ball_count; ++j) {
				if(i == j) continue;
				check_and_resolve_balls_3d(state->balls + i, state->balls + j);
			}
		}*/
		
		update_grid_3d(&state->grid, state->balls, state->ball_count);
		spatial_collision_3d(&state->grid, state->balls, state->ball_count);
		
		
		for(int i = 0; i < state->ball_count; ++i) {
			ball_3d *current_ball = state->balls + i;

			end_ball_update_3d(current_ball, state->deltatime);
		}
		
	}

	for(int i = 0; i < state->ball_count; ++i) {
		draw_circle_3d(state->renderer, state->balls[i], 25, state->cam);
	}
	SDL_SetRenderScale(state->renderer, 1., 1.);
	//SDL_RenderDebugText(state->renderer, 0, 0, get_option());
	split_draw_debug(state->renderer);
	SDL_SetRenderScale(state->renderer, 1., 1.);
	
	/*
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
	}
	*/
	

	draw_wall_3d(state->renderer, bottom1, state->cam);
	draw_wall_3d(state->renderer, bottom2, state->cam);

	draw_wall_3d(state->renderer, back1, state->cam);
	draw_wall_3d(state->renderer, back2, state->cam);

	draw_wall_3d(state->renderer, front1, state->cam);
	draw_wall_3d(state->renderer, front2, state->cam);


	draw_wall_3d(state->renderer, left1, state->cam);
	draw_wall_3d(state->renderer, left2, state->cam);

	draw_wall_3d(state->renderer, right1, state->cam);
	draw_wall_3d(state->renderer, right2, state->cam);


	SDL_RenderPresent(state->renderer);


	Uint64 time_taken = SDL_GetTicks() - start_time;
	if(time_taken > (Uint64)1000 / framerate) time_taken = 1000 / framerate;
	//SDL_Delay(1000 / framerate - time_taken);

	state->frame_count++;

	ball_count = state->ball_count;

	if(SDL_GetTicks() > state->frame_tick_count + 1000) {
		printf("framerate: %i\nball count: %i\n", state->frame_count, state->ball_count);
		fps = state->frame_count;
		state->frame_tick_count = SDL_GetTicks();
		state->frame_count = 0;

		//spatial_partition *bin_partition = state->grid.partitions + state->grid.width * state->grid.height * state->grid.depth;
		//int push_back = 0;
		//for(int i = 0; i < state->ball_count; ++i) {
		//	ball_3d *ball = state->balls + i;
		//	int ball_column = ball->position.x / state->grid.element_size + state->grid.width * 0.5;
		//	int ball_row = ball->position.y / state->grid.element_size + state->grid.height * 0.5;
		//	int ball_layer = ball->position.z / state->grid.element_size + state->grid.depth * 0.5;

		//	state->balls[i-push_back] = state->balls[i];

		//	if(ball_column < 0 || ball_column > state->grid.width - 1
		//		|| ball_row < 0 || ball_row > state->grid.height - 1
		//		|| ball_layer < 0 || ball_layer > state->grid.depth - 1)
		//		++push_back;
		//}
		//state->ball_count -= push_back;
		//state->balls = realloc(state->balls, sizeof(ball_3d) * state->ball_count);
	}
	/*vec3 momenta = {0};
	for(int i = 0; i < state->ball_count; ++i) {
		momenta = v3_add(momenta, v3_fmult(state->balls[i].velocity, state->balls[i].mass));
	}
	printf("momenta: %f\n", v3_magnitude(momenta));*/



	return SDL_APP_CONTINUE;
}
