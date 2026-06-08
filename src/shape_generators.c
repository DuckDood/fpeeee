#include <shape_generators.h>
#include <stdio.h>
#include <stdlib.h>

shape generate_rectangle(float width, float height, vec2 starting_position) {
	shape rectangle;
	rectangle.balls = malloc(4 * sizeof(ball));
	rectangle.ball_count = 4;
	rectangle.links = malloc(6 * sizeof(linkage));
	rectangle.link_count = 6;

	rectangle.balls[0].position = v2_add((vec2){-width/2, -height/2}, starting_position);
	rectangle.balls[1].position = v2_add((vec2){width/2, -height/2}, starting_position);
	rectangle.balls[2].position = v2_add((vec2){width/2, height/2}, starting_position);
	rectangle.balls[3].position = v2_add((vec2){-width/2, height/2}, starting_position);

	set_velocity(rectangle.balls+0, (vec2){0,0});
	set_velocity(rectangle.balls+1, (vec2){0,0});
	set_velocity(rectangle.balls+2, (vec2){0,0});
	set_velocity(rectangle.balls+3, (vec2){0,0});

	rectangle.balls[0].radius = 10;
	rectangle.balls[1].radius = 10;
	rectangle.balls[2].radius = 10;
	rectangle.balls[3].radius = 10;

	rectangle.links[0] = (linkage){rectangle.balls+0, rectangle.balls+1, width, 0, DISTANCE};
	rectangle.links[1] = (linkage){rectangle.balls+1, rectangle.balls+2, height, 0, DISTANCE};
	rectangle.links[2] = (linkage){rectangle.balls+2, rectangle.balls+3, width, 0, DISTANCE};
	rectangle.links[3] = (linkage){rectangle.balls+3, rectangle.balls+0, height, 0, DISTANCE};

	float corner_distance = v2_magnitude((vec2){width, height});
	rectangle.links[4] = (linkage){rectangle.balls+0, rectangle.balls+2, corner_distance, 0, DISTANCE};
	rectangle.links[5] = (linkage){rectangle.balls+1, rectangle.balls+3, corner_distance, 0, DISTANCE};

	rectangle.self_collision = 1;
	return rectangle;
}

shape generate_cloth(float width, float height, int width_resolution, int height_resolution, vec2 starting_position) {
	shape cloth;

	cloth.ball_count = width_resolution * height_resolution;
	cloth.balls = malloc(cloth.ball_count * sizeof(ball));
	//cloth.link_count = (width_resolution-1) * height_resolution + (height_resolution-1) * width_resolution;
	//2hw - w - h it simplifies down to this
	cloth.link_count = 2 * height_resolution * width_resolution - width_resolution - height_resolution;
	cloth.links = malloc(cloth.link_count * sizeof(linkage));
	int cloth_link_count = 0;

	for(int row = 0; row < height_resolution; ++row) {
		for(int column = 0; column < width_resolution; ++column) {
			ball *active_ball = &cloth.balls[row * width_resolution + column];
			active_ball->position = v2_add((vec2){(column-width_resolution/2.) * width/(width_resolution-1), (row-height_resolution/2.) * height/(height_resolution-1)}, starting_position);
			set_velocity(active_ball, (vec2){0,0});
			active_ball->radius = 5;
			if(column < width_resolution - 1) {
				cloth.links[cloth_link_count++] = (linkage){
					.a = active_ball,
					.b = active_ball + 1,
					.length = width/(width_resolution-1),
					.stiffness = 100,
					.type = ROPE_SPRING
				};
			}
			if(row < height_resolution - 1) {
				cloth.links[cloth_link_count++] = (linkage){
					.a = active_ball,
					.b = active_ball + width_resolution,
					.length = height/(height_resolution-1),
					.stiffness = 100,
					.type = ROPE_SPRING
				};
			}
		}
	}
	
	cloth.self_collision = 0;
	return cloth;
}
