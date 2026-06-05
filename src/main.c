#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <math.h>
#include <stdio.h>
#include <physics.h>
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




int main() {
	if(!SDL_Init(SDL_INIT_VIDEO)) {
		printf("Failed to initialize SDL3: %s\n", SDL_GetError());
	}

	SDL_Window *window;
	SDL_Renderer *renderer;

	if(!SDL_CreateWindowAndRenderer("physics", WIDTH, HEIGHT, 0, &window, &renderer)) {
		printf("Failed to create window or renderer: %s", SDL_GetError());
	}

	Uint64 fpsTicks = SDL_GetTicks();
	int frame = 0;
	float deltatime = 0;
	
	Uint64 frameStartTime  = SDL_GetPerformanceCounter();
	Uint64 lastFrameTime = 1;

	float a = -3.14/2.1;


	wall walls[5];
	int wall_count = 5;
	walls[0] = (wall){{400, 500}, {cos(a), sin(a)}, 300};

	walls[1] = (wall){WIDTH/2., HEIGHT, {0, -1}, WIDTH};
	walls[2] = (wall){WIDTH/2., 0, {0, 1}, WIDTH};

	walls[3] = (wall){0, HEIGHT/2., {1, 0}, HEIGHT};
	walls[4] = (wall){WIDTH, HEIGHT/2., {-1, 0}, HEIGHT};


	int ball_count = 200;
	ball *balls = malloc(ball_count * sizeof(ball));
	for(int i = 0; i < ball_count; i++) {
		balls[i].position = (vec2) {rand() % WIDTH, 50};
		//balls[i].position = (vec2) {i * 20 % WIDTH, 50 + (int)((i * 20.)/WIDTH) * 20};
		balls[i].radius = 10;
		//balls[i].radius = rand() % 15 + 5;

		set_velocity(balls+i, (vec2){0,0});
	}

	//ball *cursor_ball = balls; // commented incase i dont want the range thing anymore

	float mouse_power = 300;
	float mouse_distance = 150;

	int mouse_down = 0;
	float mx = 0, my = 0;
	vec2 mouse_vector = (vec2){mx, my};
	SDL_Event event;
	bool running = true;

	while(running) {
		Uint64 start_time = SDL_GetTicks();
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;
				default: 
					break;
			}
		}

		//a+=0.05;
		walls[0] = (wall){{400, 500}, {cos(a), sin(a)}, 300};

		lastFrameTime = frameStartTime;
		frameStartTime = SDL_GetPerformanceCounter();
		deltatime = (double)(frameStartTime - lastFrameTime)/SDL_GetPerformanceFrequency();
		int steps_per_frame = 3;
		deltatime = 1.0/60; // more consistent and basically the same to the user
		deltatime*=1.0/steps_per_frame;



		SDL_SetRenderDrawColor(renderer, 190, 190, 230, 1);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);

		int previous_mouse_down = mouse_down;
		vec2 previous_mouse_vector = mouse_vector;
		mouse_down = SDL_BUTTON_LMASK & SDL_GetMouseState(&mx, &my);
		mouse_vector = (vec2){mx, my};

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
			for(int i = 0; i < ball_count; ++i) {
				update_ball(balls+i);
			}
			if(mouse_down) {
				//cursor_ball->position = mouse_vector;
				//set_velocity(cursor_ball, v2_fdiv(v2_sub(mouse_vector, previous_mouse_vector), steps_per_frame));
			}
			for(int i = 0; i < ball_count; ++i) {
				balls[i].position = v2_add(balls[i].position, (vec2){0, 800 * deltatime * deltatime});

				if(mouse_down) {
					float dist_to_cursor = magnitude(v2_sub(balls[i].position, mouse_vector)) - balls[i].radius;
					float affect_power = ((mouse_distance - dist_to_cursor) / mouse_distance) * mouse_power;
					if(affect_power > 0) {
						balls[i].position = v2_add(balls[i].position, v2_fmult(v2_sub(mouse_vector,balls[i].position), deltatime * deltatime * affect_power));
					}
				}

				for(int j = 0; j < wall_count; ++j) {
					check_and_resolve(balls+i, walls[j], deltatime);
				}
				for(int j = 0; j < ball_count; ++j) {
					if(i == j) continue;
					check_and_resolve_balls(balls+i, balls+j);
				}
			}
		}


		for(int i = 0; i < wall_count; i++) {
			draw_wall(renderer, walls[i]);
		}
		for(int i = 0; i < ball_count; i++) {
			//if(balls+i == cursor_ball) continue; // kinda evil comparing memory addresses like this but it works
			draw_circle(renderer, balls[i], 10);
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		//draw_circle(renderer, *cursor_ball, 10);

		ball cursor;
		SDL_GetMouseState(&mx, &my);
		cursor.position = (vec2){mx, my};
		cursor.radius = mouse_distance;
		draw_circle(renderer, cursor, 25);

		SDL_RenderPresent(renderer);


		frame++;
		if(SDL_GetTicks() > fpsTicks + 1000) {
			printf("fps: %i\n", frame/1);
			fpsTicks = SDL_GetTicks();
			frame = 0;
		}
		Uint64 time_taken = SDL_GetTicks() - start_time;
		if(time_taken > 0)
			time_taken -= 1;
		if(time_taken > 1000 / 60) time_taken = 1000 / 60;
		//SDL_DelayNS(1000000000/240. - time_taken);
		SDL_Delay(1000 / 60 - time_taken);
		//SDL_Delay(1000 / 60);

	}
	free(balls);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
