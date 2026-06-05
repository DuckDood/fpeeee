#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <math.h>
#include <physics.h>
#include <stdio.h>
#include <stdlib.h>

//#define WIDTH 800
//#define HEIGHT 600
#define WIDTH 1280
#define HEIGHT 720
//#define WIDTH 400
//#define HEIGHT 300

void draw_circle(SDL_Renderer *renderer, ball b, int resolution) {
	for(float angle = 0; angle < 3.14159*2; angle += (3.14159*2)/resolution) {
		SDL_RenderLine(renderer, b.position.x + cos(angle)*b.radius, b.position.y + sin(angle)*b.radius, b.position.x + cos(angle + (3.14159*2)/resolution)*b.radius, b.position.y + sin(angle + (3.14159*2)/resolution)*b.radius);
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

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;

	Uint64 fpsTicks;
	int frame;
	float deltatime;

	Uint64 frameStartTime;
	Uint64 lastFrameTime;

	wall walls[5];
	int wall_count;

	int ball_count;

	ball *balls;

	float mouse_power;
	float mouse_distance;

	int mouse_down;
	float mx, my;
	vec2 mouse_vector;
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

	app_state->fpsTicks = SDL_GetTicks();
	app_state->frame = 0;
	app_state->deltatime = 0;
	
	app_state->frameStartTime  = SDL_GetPerformanceCounter();
	app_state->lastFrameTime = 1;

	float a = -3.14/2.1;


	app_state->wall_count = 5;
	app_state->walls[0] = (wall){{400, 500}, {cos(a), sin(a)}, 300};

	app_state->walls[1] = (wall){WIDTH/2., HEIGHT, {0, -1}, WIDTH};
	app_state->walls[2] = (wall){WIDTH/2., 0, {0, 1}, WIDTH};

	app_state->walls[3] = (wall){0, HEIGHT/2., {1, 0}, HEIGHT};
	app_state->walls[4] = (wall){WIDTH, HEIGHT/2., {-1, 0}, HEIGHT};


	app_state->ball_count = 2;
	app_state->balls = malloc(app_state->ball_count * sizeof(ball));
	for(int i = 0; i < app_state->ball_count; i++) {
		app_state->balls[i].position = (vec2) {rand() % WIDTH, 50};
		app_state->balls[i].position = (vec2) {i * 20 % WIDTH, 50 + (int)((i * 20.)/WIDTH) * 20};
		app_state->balls[i].radius = 25;
		//app_state->balls[i].radius = rand() % 15 + 5;

		set_velocity(app_state->balls+i, (vec2){0,0});
	}

	//ball *cursor_ball = balls; // commented incase i dont want the range thing anymore

	app_state->mouse_power = 300;
	app_state->mouse_distance = 150;

	app_state->mouse_down = 0;
	app_state->mx = 0;
	app_state->my = 0;
	app_state->mouse_vector = (vec2){app_state->mx, app_state->my};

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	prog_state *app_state = appstate;

	Uint64 start_time = SDL_GetTicks();
	/*while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_EVENT_QUIT:
				running = false;
				break;
			default: 
				break;
		}
	}*/

	//a+=0.05;
	//app_state->walls[0] = (wall){{400, 500}, {cos(a), sin(a)}, 300};

	app_state->lastFrameTime = app_state->frameStartTime;
	app_state->frameStartTime = SDL_GetPerformanceCounter();
	app_state->deltatime = (double)(app_state->frameStartTime - app_state->lastFrameTime)/SDL_GetPerformanceFrequency();
	int framerate = 60;
	int steps_per_frame = 3;
	app_state->deltatime = 1.0/framerate; // more consistent and basically the same to the user
	app_state->deltatime*=1.0/steps_per_frame;



	SDL_SetRenderDrawColor(app_state->renderer, 190, 190, 230, 1);
	SDL_RenderClear(app_state->renderer);

	SDL_SetRenderDrawColor(app_state->renderer, 255, 0, 0, 1);

	int previous_mouse_down = app_state->mouse_down;
	vec2 previous_mouse_vector = app_state->mouse_vector;
	app_state->mouse_down = SDL_BUTTON_LMASK & SDL_GetMouseState(&app_state->mx, &app_state->my);
	app_state->mouse_vector = (vec2){app_state->mx, app_state->my};

	/*if(mouse_down && !previous_mouse_down) {
		int current_closest = 0;
		float current_closest_distance = INFINITY;
		for(int i = 0; i < ball_count; ++i) {
			float distance = magnitude(v2_sub(balls[i].position, mouse_vector));
			if(current_closest_distance > distance) {
				current_closest_distance = distance;
				current_closest = i;
			}
		}
		cursor_ball = balls+current_closest;
	}*/

	for(int steps = 0; steps < steps_per_frame; ++steps) {
		for(int i = 0; i < app_state->ball_count; ++i) {
			update_ball(app_state->balls+i);
		}
		if(app_state->mouse_down) {
			//cursor_ball->position = mouse_vector;
			//set_velocity(cursor_ball, v2_fdiv(v2_sub(mouse_vector, previous_mouse_vector), steps_per_frame));
			app_state->balls[0].position = app_state->mouse_vector;
		}
		for(int i = 0; i < app_state->ball_count; ++i) {
			app_state->balls[i].position = v2_add(app_state->balls[i].position, (vec2){0, 800 * app_state->deltatime * app_state->deltatime});

			/*if(app_state->mouse_down) {
				float dist_to_cursor = magnitude(v2_sub(app_state->balls[i].position, app_state->mouse_vector)) - app_state->balls[i].radius;
				float affect_power = ((app_state->mouse_distance - dist_to_cursor) / app_state->mouse_distance) * app_state->mouse_power;
				if(affect_power > 0) {
					app_state->balls[i].position = v2_add(app_state->balls[i].position, v2_fmult(v2_sub(app_state->mouse_vector,app_state->balls[i].position), app_state->deltatime * app_state->deltatime * affect_power));
				}
			}*/

			for(int j = 0; j < app_state->wall_count; ++j) {
				check_and_resolve(app_state->balls+i, app_state->walls[j], 1, 0, app_state->deltatime);
			}
			for(int j = 0; j < app_state->ball_count; ++j) {
				if(i == j) continue;
				check_and_resolve_balls(app_state->balls+i, app_state->balls+j);
			}
		distance_constraint(app_state->balls, app_state->balls+1, 200);
		}
	}


	for(int i = 0; i < app_state->wall_count; i++) {
		draw_wall(app_state->renderer, app_state->walls[i]);
	}
	for(int i = 0; i < app_state->ball_count; i++) {
		//if(balls+i == cursor_ball) continue; // kinda evil comparing memory addresses like this but it works
		draw_circle(app_state->renderer, app_state->balls[i], 25);
	}

	SDL_RenderLine(app_state->renderer, app_state->balls[0].position.x, app_state->balls[0].position.y, app_state->balls[1].position.x, app_state->balls[1].position.y);
	SDL_SetRenderDrawColor(app_state->renderer, 0, 0, 255, 255);
	//draw_circle(renderer, *cursor_ball, 10);

	ball cursor;
	SDL_GetMouseState(&app_state->mx, &app_state->my);
	cursor.position = (vec2){app_state->mx, app_state->my};
	cursor.radius = app_state->mouse_distance;
	draw_circle(app_state->renderer, cursor, 50);

	SDL_RenderLine(app_state->renderer, 0, 50, 100, 50);
	SDL_RenderPresent(app_state->renderer);


	app_state->frame++;
	if(SDL_GetTicks() > app_state->fpsTicks + 1000) {
		printf("fps: %i\n", app_state->frame/1);
		app_state->fpsTicks = SDL_GetTicks();
		app_state->frame = 0;
	}
	Uint64 time_taken = SDL_GetTicks() - start_time;
	if(time_taken > 0)
		time_taken -= 1;
	if(time_taken > 1000 / framerate) time_taken = 1000 / framerate;
	//SDL_DelayNS(1000000000/240. - time_taken);
	SDL_Delay(1000 / framerate - time_taken);
	//SDL_Delay(1000 / 60);
	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
	prog_state *app_state = appstate;
	free(app_state->balls);
	SDL_DestroyRenderer(app_state->renderer);
	SDL_DestroyWindow(app_state->window);
	SDL_Quit();
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	switch(event->type) {
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
			break;
		default: 
			break;
	}
	return SDL_APP_CONTINUE;
}
