#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <math.h>
#include <stdio.h>
#include <types.h>

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

void resolve_collision(ball *body, collision_info hit_info, float deltatime) {
	if(hit_info.hit) {
		body->position = v2_sub(body->position, v2_fmult(hit_info.normal, hit_info.depth));
		if(dot(body->velocity, hit_info.normal) < 0) {

		float bounce_dot = dot(hit_info.normal, body->velocity);

		float elasticity = 0.2;
		float friction = 0;
		//vec2 bounce = v2_fmult(hit_info.normal, -bounce_dot * (1+elasticity));
		vec2 bounce_velocity = v2_fmult(hit_info.normal, -bounce_dot);
		vec2 slide_velocity = v2_add(body->velocity, bounce_velocity); // to negate downward velocity and only have sliding
		//vec2 resistance = v2_fmult(v2_sub(mainBall.velocity, bounce), 1-friction);
		
		//body->velocity = v2_add(body->velocity, bounce);
		body->velocity = v2_add(v2_fmult(bounce_velocity, elasticity), v2_fmult(slide_velocity, exp(-friction * deltatime)));
		//mainBall.velocity = v2_add(resistance, bounce);
		//mainBall.velocity = v2_add(mainBall.velocity, resistance);
		}
	}

}

void spring_constraint(ball *a, ball *b, float length, float deltatime) {
	vec2 avg_position = {(a->position.x + b->position.x) * 0.5, (a->position.y + b->position.y) * 0.5};

	vec2 relative_position = v2_sub(avg_position, a->position);
	float distance = sqrt(relative_position.x * relative_position.x + relative_position.y * relative_position.y);
	if(distance < 0.01) return;
	vec2 rel_position_normal = v2_fdiv(relative_position, distance);
	float spring_power = 90 * deltatime;
	spring_power *= distance-length;

	vec2 relative_velocity = v2_sub(b->velocity, a->velocity);

	float damp_power = 0.1 * deltatime;
	damp_power *= dot(rel_position_normal, relative_velocity);

	float total_power = spring_power + damp_power;

	a->velocity = v2_add(a->velocity, v2_fmult(rel_position_normal, total_power));
	b->velocity = v2_add(b->velocity, v2_fmult(rel_position_normal, -total_power));
	//a->position = v2_add(v2_fmult(rel_position_normal, -length), avg_position);
	//b->position = v2_add(v2_fmult(rel_position_normal, length), avg_position);
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
	//mainBall.velocity = (vec2){250, -350};
	mainBall.velocity = (vec2){0,0};
	mainBall.radius = 25;

	ball mainBall2;

	mainBall2.position = (vec2){400, 300};
	//mainBall2.velocity = (vec2){250, 0};
	mainBall2.velocity = (vec2){0, 0};
	mainBall2.radius = 25;

	Uint64 fpsTicks = SDL_GetTicks();
	int frame = 0;
	float deltatime = 0;
	
	Uint64 frameStartTime  = SDL_GetPerformanceCounter();
	Uint64 lastFrameTime = 1;

	float a = -3.14/2.1;

	wall bottom_wall = {WIDTH/2., HEIGHT, {0, -1}, WIDTH};
	wall top_wall = {WIDTH/2., 0, {0, 1}, WIDTH};

	wall left_wall = {0, HEIGHT/2., {1, 0}, HEIGHT};
	wall right_wall = {WIDTH, HEIGHT/2., {-1, 0}, HEIGHT};

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
		deltatime*=1;


		wall test_wall = {{400, 500}, {cos(a), sin(a)}, 300};
		//a+=0.028;

		SDL_SetRenderDrawColor(renderer, 190, 190, 230, 1);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);


		float mx, my;
		int mouse_down = SDL_BUTTON_LMASK & SDL_GetRelativeMouseState(&mx, &my);
		if(mouse_down) {
			mainBall.velocity = (vec2){mx/deltatime, my/deltatime};
			
		}

		mainBall.velocity.y += 800 * deltatime;
		mainBall.position = v2_add(mainBall.position, v2_fmult(mainBall.velocity, deltatime));

		mainBall2.velocity.y += 800 * deltatime;
		mainBall2.position = v2_add(mainBall2.position, v2_fmult(mainBall2.velocity, deltatime));

		collision_info hit_info = check_collision(mainBall, test_wall);
		resolve_collision(&mainBall, hit_info, deltatime);
/*
		if(hit_info.hit) {
			mainBall.position = v2_sub(mainBall.position, v2_fmult(hit_info.normal, hit_info.depth));
			if(dot(mainBall.velocity, hit_info.normal) < 0) {

			float bounce_dot = dot(hit_info.normal, mainBall.velocity);

			float elasticity = 0.2;
			float friction = 0;
			vec2 bounce = v2_fmult(hit_info.normal, -bounce_dot * (1+elasticity));
			//vec2 resistance = v2_fmult(v2_sub(mainBall.velocity, bounce), 1-friction);
			
			mainBall.velocity = v2_add(mainBall.velocity, bounce);
			//mainBall.velocity = v2_add(resistance, bounce);
			//mainBall.velocity = v2_add(mainBall.velocity, resistance);
			}
		}*/


		collision_info hit_info_b = check_collision(mainBall, bottom_wall);
		resolve_collision(&mainBall, hit_info_b, deltatime);
		collision_info hit_info_t = check_collision(mainBall, top_wall);
		resolve_collision(&mainBall, hit_info_t, deltatime);
		collision_info hit_info_l = check_collision(mainBall, left_wall);
		resolve_collision(&mainBall, hit_info_l, deltatime);
		collision_info hit_info_r = check_collision(mainBall, right_wall);
		resolve_collision(&mainBall, hit_info_r, deltatime);

		collision_info hit_info2 = check_collision(mainBall2, test_wall);
		resolve_collision(&mainBall2, hit_info2, deltatime);

		collision_info hit_info2_b = check_collision(mainBall2, bottom_wall);
		resolve_collision(&mainBall2, hit_info2_b, deltatime);
		collision_info hit_info2_t = check_collision(mainBall2, top_wall);
		resolve_collision(&mainBall2, hit_info2_t, deltatime);
		collision_info hit_info2_l = check_collision(mainBall2, left_wall);
		resolve_collision(&mainBall2, hit_info2_l, deltatime);
		collision_info hit_info2_r = check_collision(mainBall2, right_wall);
		resolve_collision(&mainBall2, hit_info2_r, deltatime);

		spring_constraint(&mainBall, &mainBall2, 100, deltatime);

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
		draw_wall(renderer, test_wall);
		draw_wall(renderer, bottom_wall);
		draw_wall(renderer, top_wall);
		draw_wall(renderer, left_wall);
		draw_wall(renderer, right_wall);
		SDL_RenderLine(renderer, mainBall.position.x, mainBall.position.y, mainBall2.position.x, mainBall2.position.y);



		SDL_RenderPresent(renderer);
		frame++;
		if(SDL_GetTicks() > fpsTicks + 3000) {
			printf("fps: %i\n", frame/3);
			fpsTicks = SDL_GetTicks();
			frame = 0;
		}
		SDL_Delay(1000/60);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
