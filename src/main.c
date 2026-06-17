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

#define CLOTH_DIMENSIONS 10

void draw_circle(SDL_Renderer *renderer, ball b, int resolution) {
	for(float angle = 0; angle < 3.14159*2; angle += (3.14159*2)/resolution) {
		SDL_RenderLine(renderer, b.position.x + cos(angle)*b.radius, b.position.y + sin(angle)*b.radius, b.position.x + cos(angle + (3.14159*2)/resolution)*b.radius, b.position.y + sin(angle + (3.14159*2)/resolution)*b.radius);
	}
}

void draw_circle_3d(SDL_Renderer *renderer, ball_3d b, int resolution, vec3 camera_position, vec3 camera_rotation) {
	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;
	vec3 b_position_relative = v3_sub(b.position, camera_position);
	b_position_relative.y *= -1;
	mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));
	b_position_relative = m3_v3_mult(camera_rotation_matrix, b_position_relative);

	b.radius /= b_position_relative.z;
	if (b_position_relative.z < 0) return;
	b_position_relative.x /= b_position_relative.z;
	b_position_relative.y /= b_position_relative.z;

	//b_position_relative.x /= 2;
	//b_position_relative.y /= 2;
	//b_position_relative.x += 0.5;
	//b_position_relative.y += 0.5;
	//b_position_relative.x *= WIDTH;
	//b_position_relative.y *= HEIGHT;

	for(float angle = 0; angle < 3.14159*2; angle += (3.14159*2)/resolution) {
		SDL_RenderLine(renderer,
				((b_position_relative.x + cos(angle)*b.radius) * inverse_aspect_ratio * 0.5 + 0.5) * WIDTH,
				((b_position_relative.y + sin(angle)*b.radius) * 0.5 + 0.5) * HEIGHT,
				((b_position_relative.x + cos(angle + (3.14159*2)/resolution)*b.radius) * inverse_aspect_ratio * 0.5 + 0.5) * WIDTH,
				((b_position_relative.y + sin(angle + (3.14159*2)/resolution)*b.radius) * 0.5 + 0.5) * HEIGHT);
	}
}

void draw_wall(SDL_Renderer *renderer, wall w) {
	SDL_RenderLine(renderer,
		 w.position.x + w.normal.y * w.length/2,
		 w.position.y + -w.normal.x * w.length/2,
		w.position.x - w.normal.y * w.length/2,
		 w.position.y - -w.normal.x * w.length/2
	 );
}

void draw_wall_3d(SDL_Renderer *renderer, wall_3d w, vec3 camera_position, vec3 camera_rotation) {
	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;

	vec3 a_relative = v3_sub(w.vertex_a, camera_position);
	vec3 b_relative = v3_sub(w.vertex_b, camera_position);
	vec3 c_relative = v3_sub(w.vertex_c, camera_position);

	a_relative.y *= -1;
	b_relative.y *= -1;
	c_relative.y *= -1;

	mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));

	a_relative = m3_v3_mult(camera_rotation_matrix, a_relative);

	if (a_relative.z < 0) return;
	a_relative.x /= a_relative.z;
	a_relative.y /= a_relative.z;

	a_relative.x *= inverse_aspect_ratio;

	a_relative.x /= 2;
	a_relative.y /= 2;
	a_relative.x += 0.5;
	a_relative.y += 0.5;
	a_relative.x *= WIDTH;
	a_relative.y *= HEIGHT;


	b_relative = m3_v3_mult(camera_rotation_matrix, b_relative);

	if (b_relative.z < 0) return;
	b_relative.x /= b_relative.z;
	b_relative.y /= b_relative.z;

	b_relative.x *= inverse_aspect_ratio;

	b_relative.x /= 2;
	b_relative.y /= 2;
	b_relative.x += 0.5;
	b_relative.y += 0.5;
	b_relative.x *= WIDTH;
	b_relative.y *= HEIGHT;

	c_relative = m3_v3_mult(camera_rotation_matrix, c_relative);

	if (c_relative.z < 0) return;
	c_relative.x /= c_relative.z;
	c_relative.y /= c_relative.z;

	c_relative.x *= inverse_aspect_ratio;

	c_relative.x /= 2;
	c_relative.y /= 2;
	c_relative.x += 0.5;
	c_relative.y += 0.5;
	c_relative.x *= WIDTH;
	c_relative.y *= HEIGHT;

	SDL_RenderLine(renderer, a_relative.x, a_relative.y, b_relative.x, b_relative.y);
	SDL_RenderLine(renderer, b_relative.x, b_relative.y, c_relative.x, c_relative.y);
	SDL_RenderLine(renderer, c_relative.x, c_relative.y, a_relative.x, a_relative.y);
}

void draw_linkage(SDL_Renderer *renderer, linkage link) {
	SDL_RenderLine(renderer, link.a->position.x, link.a->position.y, link.b->position.x, link.b->position.y);
}

void draw_linkage_3d(SDL_Renderer *renderer, linkage_3d link, vec3 camera_position, vec3 camera_rotation) {
	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;

	vec3 first = v3_sub(link.a->position, camera_position);
	vec3 last = v3_sub(link.b->position, camera_position);
	first.y *=-1;
	last.y *=-1;
	mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));
	/*first.z /= WIDTH;
	last.z /= WIDTH;

	first.x /= WIDTH;
	first.y /= HEIGHT;
	first.x -= 0.5;
	first.y -= 0.5;
	first.x *= 2;
	first.y *= 2;*/
	first = m3_v3_mult(camera_rotation_matrix, first);

	first.x *= inverse_aspect_ratio;

	if (first.z < 0) return;
	first.x /= first.z;
	first.y /= first.z;

	first.x /= 2;
	first.y /= 2;
	first.x += 0.5;
	first.y += 0.5;
	first.x *= WIDTH;
	first.y *= HEIGHT;

	/*last.x /= WIDTH;
	last.y /= HEIGHT;
	last.x -= 0.5;
	last.y -= 0.5;
	last.x *= 2;
	last.y *= 2;*/
	last = m3_v3_mult(camera_rotation_matrix, last);
	
	last.x *= inverse_aspect_ratio;

	if (last.z < 0) return;
	last.x /= last.z;
	last.y /= last.z;

	last.x /= 2;
	last.y /= 2;
	last.x += 0.5;
	last.y += 0.5;
	last.x *= WIDTH;
	last.y *= HEIGHT;

	SDL_RenderLine(renderer, first.x, first.y, last.x, last.y);
}

void set_wall_normal(wall_3d *w) {
	vec3 a_b_edge = v3_sub(w->vertex_a, w->vertex_b);
	vec3 b_c_edge = v3_sub(w->vertex_b, w->vertex_c);
	w->normal = v3_normalize(v3_cross(b_c_edge, a_b_edge));
}

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

	vec3 camera_position;
	vec3 camera_rotation;

	float yaw;
	float pitch;

	ball_3d *balls;
	int ball_count;

	shape_3d cloth;
} prog_state;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
	*appstate = malloc(sizeof(prog_state));
	prog_state *app_state = *appstate;

	if(!SDL_Init(SDL_INIT_VIDEO)) {
		printf("Failed to initialize SDL3: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if(!SDL_CreateWindowAndRenderer("physics", WIDTH, HEIGHT, 0, &app_state->window, &app_state->renderer)) {
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


	//app_state->yaw = 3.14159/2;
	app_state->yaw = 0;
	app_state->pitch = 0;

	printf("aspect_ratio: %f\n", (float)WIDTH/HEIGHT);

	app_state->mouse_power = 1000;
	app_state->mouse_distance = 100;

	app_state->mouse_down = 0;
	app_state->mx = 0;
	app_state->my = 0;
	app_state->mouse_vector = (vec2){app_state->mx, app_state->my};

	app_state->camera_position = (vec3){0, 1, -3};
	app_state->camera_rotation = (vec3){0, 0, 0};

	//app_state->ball_count = 49;
	//app_state->ball_count = 0;
	app_state->ball_count = 0;
	//app_state->ball_count = 49*9;
	app_state->balls = malloc(app_state->ball_count * sizeof(ball_3d));

	int x = 0, y = 0, z = 0;

	int spawn_grid_limit = 6;
	for(int i = 0; i < app_state->ball_count; ++i) {
		x++;
		if(x > spawn_grid_limit) x = 0, z++;
		if(z > spawn_grid_limit) z = 0, y++;
		ball_3d *current_ball = app_state->balls+i;
		current_ball->position = (vec3){(x - spawn_grid_limit/2.) * 0.2, y * 0.2 +1 + 2, (z - spawn_grid_limit/2.) * 0.2};
		current_ball->radius = 0.09;
		current_ball->mass = 1;
		set_velocity_3d(current_ball, (vec3){0, 0, 0});
	}
	app_state->cloth = generate_cloth_3d(2, 2, CLOTH_DIMENSIONS, CLOTH_DIMENSIONS, 3, (vec3){0, 3, 0});

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
	app_state->deltatime = 1.0/framerate; // more consistent and basically the same to the user if framerate stays consistent
	app_state->deltatime*=1.0/steps_per_frame;



	SDL_SetRenderDrawColor(app_state->renderer, 190, 190, 230, 1);
	SDL_RenderClear(app_state->renderer);

	SDL_SetRenderDrawColor(app_state->renderer, 255, 0, 0, 1);

	bool const *kbd_state = SDL_GetKeyboardState(NULL);

	if(kbd_state[SDL_SCANCODE_W]) {
		app_state->camera_position.z += 0.2 * cos(app_state->camera_rotation.x) * cos(app_state->camera_rotation.y);
		app_state->camera_position.x += 0.2 * sin(app_state->camera_rotation.x) * cos(app_state->camera_rotation.y);

		app_state->camera_position.y += 0.2 * sin(app_state->camera_rotation.y);
	}
	if(kbd_state[SDL_SCANCODE_S]) {
		app_state->camera_position.z -= 0.2 * cos(app_state->camera_rotation.x) * cos(app_state->camera_rotation.y);
		app_state->camera_position.x -= 0.2 * sin(app_state->camera_rotation.x) * cos(app_state->camera_rotation.y);

		app_state->camera_position.y -= 0.2 * sin(app_state->camera_rotation.y);
	}
	if(kbd_state[SDL_SCANCODE_D]) {
		app_state->camera_position.z -= 0.2 * sin(app_state->camera_rotation.x);
		app_state->camera_position.x += 0.2 * cos(app_state->camera_rotation.x);
	}
	if(kbd_state[SDL_SCANCODE_A]) {
		app_state->camera_position.z += 0.2 * sin(app_state->camera_rotation.x);
		app_state->camera_position.x -= 0.2 * cos(app_state->camera_rotation.x);
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
		app_state->camera_rotation.x += app_state->mx/ 100;
		app_state->camera_rotation.y -= app_state->my/ 100;
	}
	app_state->mouse_down = SDL_BUTTON_LMASK & SDL_GetMouseState(&app_state->mx, &app_state->my);
	app_state->mouse_vector = (vec2){app_state->mx, app_state->my};



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

	collision_info_3d hit_info;

	for(int step = 0; step < steps_per_frame; ++step) {
		for(int i = 0; i < app_state->ball_count; ++i) {
			ball_3d *current_ball = app_state->balls+i;
			update_ball_3d(current_ball);
			current_ball->position.y -= 60 * 0.98 * 0.1 * app_state->deltatime * app_state->deltatime;
		}

		for(int i = 0; i < app_state->ball_count; ++i) {
			ball_3d *current_ball = app_state->balls+i;

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


			for(int j = 0; j < app_state->ball_count; ++j) {
				if(i == j) continue;
				check_and_resolve_balls_3d(current_ball, app_state->balls + j);
			}

		
			for(int j = 0; j < app_state->cloth.ball_count; ++j) {
				check_and_resolve_balls_3d(current_ball, app_state->cloth.balls + j);
			}
		}
		//rope_spring_constraint_3d(app_state->balls, app_state->balls+1, 1, 10, app_state->deltatime);
		//rope_constraint_3d(app_state->balls, app_state->balls+1, 1);



		for(int i = 0; i < app_state->cloth.ball_count; ++i) {
			ball_3d *current_ball = app_state->cloth.balls+i;
			update_ball_3d(current_ball);
			current_ball->position.y -= 60 * 0.98 * 0.1 * app_state->deltatime * app_state->deltatime;
		}

		for(int i = 0; i < app_state->cloth.ball_count; ++i) {
			ball_3d *current_ball = app_state->cloth.balls+i;

			
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

			/*
			for(int j = 0; j < 15; j++) {
				float relative_top_position = (j-7.5) / 15. * 2;
					app_state->cloth.balls[j].position = (vec3){cos(app_state->yaw) * relative_top_position + 1, 4, sin(app_state->yaw) * relative_top_position};
			}
			for(int j = 0; j < 15; j++) {
				float relative_bottom_position = (j-7.5) / 15. * -2;
					app_state->cloth.balls[app_state->cloth.ball_count-j-1].position = (vec3){cos(app_state->yaw) * relative_bottom_position - 1, 2, sin(app_state->yaw) * relative_bottom_position};
			}*/
			app_state->cloth.balls[0].position = (vec3){-1, 3, 1};
			app_state->cloth.balls[CLOTH_DIMENSIONS-1].position = (vec3){1, 3, 1};
			app_state->cloth.balls[app_state->cloth.ball_count-CLOTH_DIMENSIONS].position = (vec3){-1, 3, -1};
			app_state->cloth.balls[app_state->cloth.ball_count-1].position = (vec3){1, 3, -1};

			for(int j = 0; j < app_state->cloth.link_count; ++j) {
				update_linkage_3d(app_state->cloth.links[j], app_state->deltatime);
			}
		}
	}


	
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

	SDL_SetRenderDrawColor(app_state->renderer, 0, 0, 255, 255);
	for(int i = 0; i < app_state->ball_count; ++i) {
		draw_circle_3d(app_state->renderer, app_state->balls[i], 25, app_state->camera_position, app_state->camera_rotation);
	}

	//SDL_SetRenderDrawColor(app_state->renderer, 255, 0, 255, 255);
	for(int i = 0; i < app_state->cloth.ball_count; ++i) {
		draw_circle_3d(app_state->renderer, app_state->cloth.balls[i], 25, app_state->camera_position, app_state->camera_rotation);
	}

	for(int i = 0; i < app_state->cloth.link_count; ++i) {
		draw_linkage_3d(app_state->renderer, app_state->cloth.links[i], app_state->camera_position, app_state->camera_rotation);
	}

	SDL_RenderPresent(app_state->renderer);

	app_state->frame++;
	if(SDL_GetTicks() > app_state->fps_ticks[0] + 1000) {
		printf("fps: %i\n", app_state->frame);
		app_state->fps_ticks[0] = SDL_GetTicks();
		app_state->frame = 0;/*
		app_state->balls = realloc(app_state->balls, ++app_state->ball_count * sizeof(ball_3d));
		app_state->balls[app_state->ball_count-1].position = (vec3){-10, 5, sin(app_state->fps_ticks[0] * 0.01) * 0.3};
		//app_state->balls[app_state->ball_count-1].radius = 0.1;
		//app_state->balls[app_state->ball_count-1].radius = (rand() % 10 + 10) * 0.01;
		app_state->balls[app_state->ball_count-1].radius = 0.1;
		//set_velocity_3d(app_state->balls+app_state->ball_count-1, (vec3){0.015, 0.01, 0});
		set_velocity_3d(app_state->balls+app_state->ball_count-1, (vec3){0.03, 0.025, 0});*/
	}

	if(SDL_GetTicks() > app_state->fps_ticks[1] + 250) {
		app_state->fps_ticks[1] = SDL_GetTicks();
		if(kbd_state[SDL_SCANCODE_C]) {
			app_state->balls = realloc(app_state->balls, ++app_state->ball_count * sizeof(ball_3d)); // its bad to call realloc this often but it doesnt impact performance that much right now
			//app_state->balls[app_state->ball_count-1].position = (vec3){-10, 5, sin(app_state->fps_ticks[1] * 0.01) * 0.1};
			app_state->balls[app_state->ball_count-1].position = (vec3){0, 10, 0};
			//app_state->balls[app_state->ball_count-1].position = (vec3){-10, 5, (sin(app_state->fps_ticks[1] * 0.01) - 1) * 0.1};
			//app_state->balls[app_state->ball_count-1].position = (vec3){-10, 5, 0};
			//app_state->balls[app_state->ball_count-1].radius = 0.1;
			//app_state->balls[app_state->ball_count-1].radius = (rand() % 10 + 10) * 0.01;
			//app_state->balls[app_state->ball_count-1].radius = 0.07;
			app_state->balls[app_state->ball_count-1].radius = 0.2;
			app_state->balls[app_state->ball_count-1].mass = 1;
			//app_state->balls[app_state->ball_count-1].radius = (rand() % 5 + 5)/ 100.;
			//app_state->balls[app_state->ball_count-1].radius = (rand() % 3 + 2) * 0.01;
			//set_velocity_3d(app_state->balls+app_state->ball_count-1, (vec3){0.015, 0.01, 0});
			//set_velocity_3d(app_state->balls+app_state->ball_count-1, (vec3){0.03, 0.025, 0});
			//set_velocity_3d(app_state->balls+app_state->ball_count-1, (vec3){0.025, 0.025, 0});
			//set_velocity_3d(app_state->balls+app_state->ball_count-1, (vec3){0.03/ 3.33, 0.025 / 3.33, 0});
			set_velocity_3d(app_state->balls+app_state->ball_count-1, (vec3){0, -0.01, 0});
			//set_velocity_3d(app_state->balls+app_state->ball_count-1, (vec3){0, -0.01, 0});
		}
	}

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
		default: 
			break;
	}
	return SDL_APP_CONTINUE;
}
