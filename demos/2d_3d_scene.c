#include <SDL3/SDL_oldnames.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <math.h>
#include <physics.h>
#include <matrix.h>
#include <shape_generators.h>
#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

//#define WIDTH 800
//#define HEIGHT 600
#define WIDTH 1280
#define HEIGHT 720
//#define WIDTH 400
//#define HEIGHT 300
//#define WIDTH 512
//#define HEIGHT 512
//#define WIDTH 1024
//#define HEIGHT 1024

#define ROPE_DIMENSIONS 20
#define CLOTH_DIMENSIONS 10

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;

	Uint64 fps_ticks[3];
	int frame;
	float deltatime;

	Uint64 frame_start_time;
	Uint64 last_frame_time;

	float mouse_power;
	float mouse_distance;

	int mouse_down;
	float mx, my;
	vec2 mouse_vector;

	camera cam;

	float yaw;
	float pitch;

	ball *balls;
	int ball_count;

	shape rope;
	
	ball_3d *balls_3d;

	shape_3d cloth;
} prog_state;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
	*appstate = malloc(sizeof(prog_state));
	prog_state *app_state = *appstate;

	if(!SDL_Init(SDL_INIT_VIDEO)) {
		printf("Failed to initialize SDL3: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if(!SDL_CreateWindowAndRenderer("physics", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE, &app_state->window, &app_state->renderer)) {
		printf("Failed to create window or renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}


	app_state->fps_ticks[0] = SDL_GetTicks();
	app_state->fps_ticks[1] = SDL_GetTicks();
	app_state->fps_ticks[2] = SDL_GetTicks();
	app_state->frame = 0;
	app_state->deltatime = 0;
	
	app_state->frame_start_time  = SDL_GetPerformanceCounter();
	app_state->last_frame_time = 1;

	float a = -3.14/2.1;


	app_state->yaw = 0;
	app_state->pitch = 0;

	printf("aspect_ratio: %f\n", (float)WIDTH/HEIGHT);

	app_state->mouse_power = 1000;
	app_state->mouse_distance = 100;

	app_state->mouse_down = 0;
	app_state->mx = 0;
	app_state->my = 0;
	app_state->mouse_vector = (vec2){app_state->mx, app_state->my};

	app_state->cam.position = (vec3){0, 0, -1};
	app_state->cam.rotation = (vec3){0, 0, 0};

	app_state->cam.width = WIDTH;
	app_state->cam.height = HEIGHT;

	app_state->ball_count = 0;
	app_state->balls = malloc(app_state->ball_count * sizeof(ball_3d));

	int x = 0, y = 0, z = 0;

	int spawn_grid_limit = 6;
	for(int i = 0; i < app_state->ball_count; ++i) {
		x++;
		if(x > spawn_grid_limit) x = 0, z++;
		if(z > spawn_grid_limit) z = 0, y++;
		ball *current_ball = app_state->balls+i;
		current_ball->position = (vec2){0, 0};
		current_ball->radius = 0.05;
		current_ball->mass = 1;
		set_velocity(current_ball, (vec2){0, 0});
	}

	float aspect_ratio = (float)WIDTH/HEIGHT;
	app_state->rope = generate_rope(1 * aspect_ratio, ROPE_DIMENSIONS, 250, (vec2){0, -0.2});
	app_state->cloth = generate_cloth_3d(1 * aspect_ratio, 1 * aspect_ratio, CLOTH_DIMENSIONS, CLOTH_DIMENSIONS, 5, (vec3){0, 0, -2});

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	prog_state *app_state = appstate;

	Uint64 start_time = SDL_GetTicks();

	app_state->last_frame_time = app_state->frame_start_time;
	app_state->frame_start_time = SDL_GetPerformanceCounter();
	//app_state->deltatime = (double)(app_state->frame_start_time - app_state->last_frame_time)/SDL_GetPerformanceFrequency();
	int framerate = 60;
	int steps_per_frame = 10;
	app_state->deltatime = 1.0/framerate;
	app_state->deltatime*=1.0/steps_per_frame;
	//steps_per_frame = 0;



	SDL_SetRenderDrawColor(app_state->renderer, 190, 190, 230, 1);
	SDL_RenderClear(app_state->renderer);

	SDL_SetRenderDrawColor(app_state->renderer, 255, 0, 0, 1);

	bool const *kbd_state = SDL_GetKeyboardState(NULL);

	if(kbd_state[SDL_SCANCODE_W]) {
		app_state->cam.position.z += 0.2 * cos(app_state->cam.rotation.x) * cos(app_state->cam.rotation.y);
		app_state->cam.position.x += 0.2 * sin(app_state->cam.rotation.x) * cos(app_state->cam.rotation.y);

		app_state->cam.position.y += 0.2 * sin(app_state->cam.rotation.y);
	}
	if(kbd_state[SDL_SCANCODE_S]) {
		app_state->cam.position.z -= 0.2 * cos(app_state->cam.rotation.x) * cos(app_state->cam.rotation.y);
		app_state->cam.position.x -= 0.2 * sin(app_state->cam.rotation.x) * cos(app_state->cam.rotation.y);

		app_state->cam.position.y -= 0.2 * sin(app_state->cam.rotation.y);
	}
	if(kbd_state[SDL_SCANCODE_D]) {
		app_state->cam.position.z -= 0.2 * sin(app_state->cam.rotation.x);
		app_state->cam.position.x += 0.2 * cos(app_state->cam.rotation.x);
	}
	if(kbd_state[SDL_SCANCODE_A]) {
		app_state->cam.position.z += 0.2 * sin(app_state->cam.rotation.x);
		app_state->cam.position.x -= 0.2 * cos(app_state->cam.rotation.x);
	}

	if(kbd_state[SDL_SCANCODE_LEFT]) {
		app_state->yaw+=0.03;
	}
	if(kbd_state[SDL_SCANCODE_RIGHT]) {
		app_state->yaw-=0.03;
	}
	if(kbd_state[SDL_SCANCODE_UP]) {
		app_state->pitch+=0.03;
	}
	if(kbd_state[SDL_SCANCODE_DOWN]) {
		app_state->pitch-=0.03;
	}


	int previous_mouse_down = app_state->mouse_down;
	vec2 previous_mouse_vector = app_state->mouse_vector;
	int mouse_d = SDL_BUTTON_LMASK & SDL_GetRelativeMouseState(&app_state->mx, &app_state->my);
	if(mouse_d) {
		app_state->cam.rotation.x += app_state->mx/ 100;
		app_state->cam.rotation.y -= app_state->my/ 100;
	}
	app_state->mouse_down = SDL_BUTTON_LMASK & SDL_GetMouseState(&app_state->mx, &app_state->my);
	app_state->mouse_vector = (vec2){app_state->mx, app_state->my};


	/*
	//mat3 rot = generate_rotation_matrix(0, app_state->pitch, app_state->yaw);
	//mat3 rot = generate_rotation_matrix(0, 0, app_state->pitch);
	mat3 scale = generate_scale_matrix((vec3){2, 2, 2});
	//mat3 scale = generate_scale_matrix((vec3){1, 1, 1});
	mat3 rot = generate_rotation_matrix(0, app_state->yaw, app_state->pitch);
	mat3 transform = m3_mult(rot, scale);
	//mat3 rot = generate_rotation_matrix(0, 0, 0);

	wall_3d w;
	w.vertex_a = m3_v3_mult(transform, (vec3){-1, 0, -1});
	w.vertex_b = m3_v3_mult(transform, (vec3){-1, 0, 1});
	w.vertex_c = m3_v3_mult(transform, (vec3){1, 0, -1});

	wall_3d w2;

	w2.vertex_a = m3_v3_mult(transform, (vec3){1, 0, 1});
	w2.vertex_b = m3_v3_mult(transform, (vec3){1, 0, -1});
	w2.vertex_c = m3_v3_mult(transform, (vec3){-1, 0, 1});

	set_wall_normal(&w);
	set_wall_normal(&w2);

	wall_3d w_2;
	w_2.vertex_a = m3_v3_mult(transform, (vec3){-1, 1, -1});
	w_2.vertex_b = m3_v3_mult(transform, (vec3){-1, 1, 1});
	w_2.vertex_c = m3_v3_mult(transform, (vec3){-1, 0, -1});

	wall_3d w2_2;

	w2_2.vertex_a = m3_v3_mult(transform, (vec3){-1, 0, -1});
	w2_2.vertex_b = m3_v3_mult(transform, (vec3){-1, 0, 1});
	w2_2.vertex_c = m3_v3_mult(transform, (vec3){-1, 1, 1});

	set_wall_normal(&w_2);
	set_wall_normal(&w2_2);

	wall_3d w_3;
	w_3.vertex_a = m3_v3_mult(transform, (vec3){1, 1, -1});
	w_3.vertex_b = m3_v3_mult(transform, (vec3){1, 1, 1});
	w_3.vertex_c = m3_v3_mult(transform, (vec3){1, 0, -1});

	wall_3d w2_3;

	w2_3.vertex_a = m3_v3_mult(transform, (vec3){1, 0, -1});
	w2_3.vertex_b = m3_v3_mult(transform, (vec3){1, 0, 1});
	w2_3.vertex_c = m3_v3_mult(transform, (vec3){1, 1, 1});

	set_wall_normal(&w_3);
	set_wall_normal(&w2_3);

	wall_3d w_4;
	w_4.vertex_a = m3_v3_mult(transform, (vec3){-1, 0, 1});
	w_4.vertex_b = m3_v3_mult(transform, (vec3){1, 0, 1});
	w_4.vertex_c = m3_v3_mult(transform, (vec3){1, 1, 1});

	wall_3d w2_4;

	w2_4.vertex_a = m3_v3_mult(transform, (vec3){1, 1, 1});
	w2_4.vertex_b = m3_v3_mult(transform, (vec3){-1, 1, 1});
	w2_4.vertex_c = m3_v3_mult(transform, (vec3){-1, 0, 1});

	set_wall_normal(&w_4);
	set_wall_normal(&w2_4);

	wall_3d w_5;
	w_5.vertex_a = m3_v3_mult(transform, (vec3){-1, 0, -1});
	w_5.vertex_b = m3_v3_mult(transform, (vec3){1, 0, -1});
	w_5.vertex_c = m3_v3_mult(transform, (vec3){1, 1, -1});

	wall_3d w2_5;

	w2_5.vertex_a = m3_v3_mult(transform, (vec3){1, 1, -1});
	w2_5.vertex_b = m3_v3_mult(transform, (vec3){-1, 1, -1});
	w2_5.vertex_c = m3_v3_mult(transform, (vec3){-1, 0, -1});

	set_wall_normal(&w_5);
	set_wall_normal(&w2_5);
	*/

	//collision_info_3d hit_info;
	collision_info hit_info;
	float aspect_ratio = (float)WIDTH/HEIGHT;

	wall w_2d; w_2d.length = 2; w_2d.normal = (vec2){sin(3.14159/2), -cos(3.14159/2)}; w_2d.position = (vec2){-1 * aspect_ratio, 0};

	wall w2_2d; w2_2d.length = 2; w2_2d.normal = (vec2){sin(-3.14159/2), -cos(-3.14159/2)}; w2_2d.position = (vec2){1 * aspect_ratio, 0};

	wall w3_2d; w3_2d.length = 2 * aspect_ratio; w3_2d.normal = (vec2){sin(-3.14159), -cos(-3.14159)}; w3_2d.position = (vec2){0, -1};

	wall w4_2d; w4_2d.length = 2 * aspect_ratio; w4_2d.normal = (vec2){sin(3.14159), -cos(3.14159)}; w4_2d.position = (vec2){0, 1};

	for(int step = 0; step < steps_per_frame; ++step) {
		{
			for(int i = 0; i < app_state->ball_count; ++i) {
				ball *current_ball = app_state->balls+i;
				update_ball(current_ball);
				current_ball->position.y -= 60 * 0.98 * 0.1 * app_state->deltatime * app_state->deltatime;
			}

			for(int i = 0; i < app_state->ball_count; ++i) {
				ball *current_ball = app_state->balls+i;

				check_and_resolve(current_ball, w_2d, 0, 0, app_state->deltatime);
				check_and_resolve(current_ball, w2_2d, 0, 0, app_state->deltatime);
				check_and_resolve(current_ball, w3_2d, 0, 0, app_state->deltatime);
				check_and_resolve(current_ball, w4_2d, 0, 0, app_state->deltatime);

				for(int j = 0; j < app_state->ball_count; ++j) {
					if(i == j) continue;
					check_and_resolve_balls(current_ball, app_state->balls + j);
				}

			
				for(int j = 0; j < app_state->rope.ball_count; ++j) {
					check_and_resolve_balls(current_ball, app_state->rope.balls + j);
				}
			}

			for(int i = 0; i < app_state->rope.ball_count; ++i) {
				ball *current_ball = app_state->rope.balls+i;
				update_ball(current_ball);
				current_ball->position.y -= 60 * 0.98 * 0.1 * app_state->deltatime * app_state->deltatime;
			}

			for(int i = 0; i < app_state->rope.ball_count; ++i) {
				ball *current_ball = app_state->rope.balls+i;

				check_and_resolve(current_ball, w_2d, 0, 0, app_state->deltatime);
				check_and_resolve(current_ball, w2_2d, 0, 0, app_state->deltatime);
				check_and_resolve(current_ball, w3_2d, 0, 0, app_state->deltatime);
				check_and_resolve(current_ball, w4_2d, 0, 0, app_state->deltatime);

				app_state->rope.balls[0].position = (vec2){-0.5 * aspect_ratio, -0.25
				};
				app_state->rope.balls[ROPE_DIMENSIONS-1].position = (vec2){0.5 * aspect_ratio, -0.25};

				for(int j = 0; j < app_state->rope.link_count; ++j) {
					update_linkage(app_state->rope.links[j], app_state->deltatime);
				}
			}
		}

		{
			for(int i = 0; i < app_state->ball_count; ++i) {
				ball_3d *current_ball = app_state->balls_3d+i;
				update_ball_3d(current_ball);
				current_ball->position.y -= 60 * 0.98 * 0.1 * app_state->deltatime * app_state->deltatime;
			}

			for(int i = 0; i < app_state->ball_count; ++i) {
				ball_3d *current_ball = app_state->balls_3d+i;

				/*
				check_and_resolve_3d(current_ball, w, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2, 0, 0, app_state->deltatime);

				check_and_resolve_3d(current_ball, w_2, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2_2, 0, 0, app_state->deltatime);

				check_and_resolve_3d(current_ball, w_3, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2_3, 0, 0, app_state->deltatime);

				check_and_resolve_3d(current_ball, w_4, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2_4, 0, 0, app_state->deltatime);

				check_and_resolve_3d(current_ball, w_5, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2_5, 0, 0, app_state->deltatime);
				*/

				for(int j = 0; j < app_state->ball_count; ++j) {
					if(i == j) continue;
					check_and_resolve_balls_3d(current_ball, app_state->balls_3d + j);
				}

			
				for(int j = 0; j < app_state->cloth.ball_count; ++j) {
					check_and_resolve_balls_3d(current_ball, app_state->cloth.balls + j);
				}
			}

			for(int i = 0; i < app_state->cloth.ball_count; ++i) {
				ball_3d *current_ball = app_state->cloth.balls+i;
				update_ball_3d(current_ball);
				current_ball->position.y -= 60 * 0.98 * 0.1 * app_state->deltatime * app_state->deltatime;
			}

			for(int i = 0; i < app_state->cloth.ball_count; ++i) {
				ball_3d *current_ball = app_state->cloth.balls+i;

				/*
				check_and_resolve_3d(current_ball, w, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2, 0, 0, app_state->deltatime);

				check_and_resolve_3d(current_ball, w_2, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2_2, 0, 0, app_state->deltatime);

				check_and_resolve_3d(current_ball, w_3, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2_3, 0, 0, app_state->deltatime);

				check_and_resolve_3d(current_ball, w_4, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2_4, 0, 0, app_state->deltatime);

				check_and_resolve_3d(current_ball, w_5, 0, 0, app_state->deltatime);
				check_and_resolve_3d(current_ball, w2_5, 0, 0, app_state->deltatime);
				*/

				for(int j = 0; j < app_state->cloth.link_count; ++j) {
					update_linkage_3d(app_state->cloth.links[j], app_state->deltatime);
				}
			}

			/*for(int j = 0; j < CLOTH_DIMENSIONS; ++j) {
				app_state->cloth.balls[j].position = (vec3){0.5*(j-CLOTH_DIMENSIONS/2.)/CLOTH_DIMENSIONS, 0, -2};
			}*/
			app_state->cloth.balls[0].position = (vec3){-0.5 * aspect_ratio, 0, -2 * aspect_ratio};
			app_state->cloth.balls[CLOTH_DIMENSIONS-1].position = (vec3){0.5 * aspect_ratio, 0, -2 * aspect_ratio};

			app_state->cloth.balls[app_state->cloth.ball_count-CLOTH_DIMENSIONS].position = (vec3){-0.5 * aspect_ratio, 0, -3 * aspect_ratio};
			app_state->cloth.balls[app_state->cloth.ball_count-1].position = (vec3){0.5 * aspect_ratio, 0, -3 * aspect_ratio};
		}
	}


	
	/*
	draw_wall_3d(app_state->renderer, w, app_state->camera_position, app_state->camera_rotation);
	draw_wall_3d(app_state->renderer, w2, app_state->camera_position, app_state->camera_rotation);

	draw_wall_3d(app_state->renderer, w_2, app_state->camera_position, app_state->camera_rotation);
	draw_wall_3d(app_state->renderer, w2_2, app_state->camera_position, app_state->camera_rotation);

	draw_wall_3d(app_state->renderer, w_3, app_state->camera_position, app_state->camera_rotation);
	draw_wall_3d(app_state->renderer, w2_3, app_state->camera_position, app_state->camera_rotation);

	draw_wall_3d(app_state->renderer, w_4, app_state->camera_position, app_state->camera_rotation);
	draw_wall_3d(app_state->renderer, w2_4, app_state->camera_position, app_state->camera_rotation);

	draw_wall_3d(app_state->renderer, w_5, app_state->camera_position, app_state->camera_rotation);
	draw_wall_3d(app_state->renderer, w2_5, app_state->camera_position, app_state->camera_rotation);
	*/
	draw_wall(app_state->renderer, w_2d, app_state->cam);
	draw_wall(app_state->renderer, w2_2d, app_state->cam);
	draw_wall(app_state->renderer, w3_2d, app_state->cam);
	draw_wall(app_state->renderer, w4_2d, app_state->cam);

	SDL_SetRenderDrawColor(app_state->renderer, 0, 0, 255, 255);

	for(int i = 0; i < app_state->ball_count; ++i) {
		draw_circle(app_state->renderer, app_state->balls[i], 25, app_state->cam);
	}
	for(int i = 0; i < app_state->rope.ball_count; ++i) {
		draw_circle(app_state->renderer, app_state->rope.balls[i], 25, app_state->cam);
	}
	for(int i = 0; i < app_state->rope.link_count; ++i) {
		draw_linkage(app_state->renderer, app_state->rope.links[i], app_state->cam);
	}

	for(int i = 0; i < app_state->ball_count; ++i) {
		draw_circle_3d(app_state->renderer, app_state->balls_3d[i], 25, app_state->cam);
	}
	for(int i = 0; i < app_state->cloth.ball_count; ++i) {
		draw_circle_3d(app_state->renderer, app_state->cloth.balls[i], 25, app_state->cam);
	}
	for(int i = 0; i < app_state->cloth.link_count; ++i) {
		draw_linkage_3d(app_state->renderer, app_state->cloth.links[i], app_state->cam);
	}

	SDL_RenderPresent(app_state->renderer);

	app_state->frame++;
	if(SDL_GetTicks() > app_state->fps_ticks[0] + 1000) {
		printf("fps: %i\n", app_state->frame);
		app_state->fps_ticks[0] = SDL_GetTicks();
		app_state->frame = 0;
	}

	if(kbd_state[SDL_SCANCODE_C]) {
		if(SDL_GetTicks() > app_state->fps_ticks[1] + 250) {
			app_state->fps_ticks[1] = SDL_GetTicks();
			app_state->balls = realloc(app_state->balls, ++app_state->ball_count * sizeof(ball));
			app_state->balls[app_state->ball_count-1].position = (vec2){0, 0.5};
			app_state->balls[app_state->ball_count-1].radius = 0.1;
			app_state->balls[app_state->ball_count-1].mass = 1;
			set_velocity(app_state->balls+app_state->ball_count-1, (vec2){0, -0.01});

			app_state->fps_ticks[1] = SDL_GetTicks();
			app_state->balls_3d = realloc(app_state->balls_3d, app_state->ball_count * sizeof(ball_3d));
			app_state->balls_3d[app_state->ball_count-1].position = (vec3){0, 1, -2.5 * aspect_ratio};
			app_state->balls_3d[app_state->ball_count-1].radius = 0.15;
			app_state->balls_3d[app_state->ball_count-1].mass = 1;
			set_velocity_3d(app_state->balls_3d+app_state->ball_count-1, (vec3){0, -0.01, 0});
		}
	}

	// causes out of bounds error when triggered cause of mixing 2d/3d balls using the same ball count
	/*
	if(SDL_GetTicks() > app_state->fps_ticks[2] + 5000) {
		app_state->fps_ticks[2] = SDL_GetTicks();

		int remaining_balls = 0;
		for(int i = 0; i < app_state->ball_count; ++i) {
			if(app_state->balls[i].position.y > -10) {
				app_state->balls[remaining_balls++] = app_state->balls[i];
			}
		}
		printf("taking out the trash: %i balls\n", app_state->ball_count - remaining_balls);
		app_state->balls = realloc(app_state->balls, remaining_balls * sizeof(ball_3d));
		if(app_state->balls == NULL && remaining_balls != 0) {
			// panic!!
			printf("realloc failed we gotta crash\n");
			fflush(stdout); // just to make sure because we are terminating now
			return SDL_APP_FAILURE;
		}
		app_state->ball_count = remaining_balls;

	}
	*/
	Uint64 time_taken = SDL_GetTicks() - start_time;
	if(time_taken > 0)
		time_taken -= 1;
	if(time_taken > 1000 / framerate) time_taken = 1000 / framerate;
	SDL_Delay(1000 / framerate - time_taken);
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
	prog_state *app_state = appstate;
	SDL_DestroyRenderer(app_state->renderer);
	SDL_DestroyWindow(app_state->window);
	free(app_state->balls);
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
