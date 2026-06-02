#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <math.h>
#include <stdio.h>
#include <physics.h>

#define WIDTH 800
#define HEIGHT 600

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

	ball mainBall;

	mainBall.position = (vec2){100, 200};
	mainBall.previous_position = (vec2){100, 200};
	mainBall.radius = 25;
	
	ball mainBall2;

	mainBall2.position = (vec2){200, 200};
	mainBall2.previous_position = (vec2){200, 200};
	mainBall2.radius = 25;

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

	SDL_Event event;
	bool running = true;
	while(running) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;
				default: 
					break;
			}
		}
		lastFrameTime = frameStartTime;
		frameStartTime = SDL_GetPerformanceCounter();
		deltatime = (double)(frameStartTime - lastFrameTime)/SDL_GetPerformanceFrequency();
		deltatime*=1/10.;



		SDL_SetRenderDrawColor(renderer, 190, 190, 230, 1);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);


		
		for(int i = 0; i < 10; i++) {
			update_ball(&mainBall);
			mainBall.position = v2_add(mainBall.position, v2_fmult((vec2){0, 300}, deltatime * deltatime));
			update_ball(&mainBall2);
			mainBall2.position = v2_add(mainBall2.position, v2_fmult((vec2){0, 300}, deltatime * deltatime));

			float mx, my;
			int mouse_down = SDL_BUTTON_LMASK & SDL_GetMouseState(&mx, &my);
			if(mouse_down) {
				//mainBall.previous_position = v2_sub(mainBall.position,(vec2){mx/2, my/2});
				//mainBall.position = v2_add(mainBall.position,(vec2){mx/2, my/2});
				mainBall.position = (vec2){mx, my};
			}


			for(int wall = 0; wall < wall_count; wall++) {
				check_and_resolve(&mainBall, walls[wall]);
				distance_constraint(&mainBall, &mainBall2, 150);
				check_and_resolve(&mainBall2, walls[wall]);
				distance_constraint(&mainBall, &mainBall2, 150);
			}
		}
		/*if(mainBall.position.y + mainBall.radius > HEIGHT) {
			//mainBall.velocity.y = -mainBall.velocity.y * 0.5;
			mainBall.position.y = HEIGHT - mainBall.radius;

			vec2 normal = {0, -1};

			vec2 vn = v2_fmult(normal, dot(normal, mainBall.velocity));
			vec2 vt = v2_sub(mainBall.velocity, vn);

			float elasticity = 0.8;
			vn = v2_fmult(vn, -elasticity);
			float friction = 1;

			vt = v2_fmult(vt, exp(-friction));

			mainBall.velocity = (vec2){vt.x + vn.x, vt.y + vn.y};
		}*/

		SDL_FRect ballRect = {mainBall.position.x-mainBall.radius, mainBall.position.y-mainBall.radius, mainBall.radius*2, mainBall.radius*2};
		//SDL_RenderFillRect(renderer, &ballRect);
		draw_circle(renderer, mainBall, 25);
		draw_circle(renderer, mainBall2, 25);

		for(int i = 0; i < wall_count; i++) {
			draw_wall(renderer, walls[i]);
		}
		SDL_RenderLine(renderer, 0, 200, 500, 200);
		SDL_RenderLine(renderer, mainBall.position.x, mainBall.position.y, mainBall2.position.x, mainBall2.position.y);



		SDL_RenderPresent(renderer);
		frame++;
		if(SDL_GetTicks() > fpsTicks + 3000) {
			printf("fps: %i\n", frame/3);
			fpsTicks = SDL_GetTicks();
			frame = 0;
		}
		SDL_Delay(1000/60. - deltatime*100);

	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
