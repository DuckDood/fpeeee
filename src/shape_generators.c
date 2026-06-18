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

	/*rectangle.balls[0].radius = 10;
	rectangle.balls[1].radius = 10;
	rectangle.balls[2].radius = 10;
	rectangle.balls[3].radius = 10;*/

	rectangle.balls[0].radius = 0.1;
	rectangle.balls[1].radius = 0.1;
	rectangle.balls[2].radius = 0.1;
	rectangle.balls[3].radius = 0.1;

	rectangle.balls[0].mass = 1;
	rectangle.balls[1].mass = 1;
	rectangle.balls[2].mass = 1;
	rectangle.balls[3].mass = 1;

	rectangle.links[0] = (linkage){rectangle.balls+0, rectangle.balls+1, width, 0, DISTANCE};
	rectangle.links[1] = (linkage){rectangle.balls+1, rectangle.balls+2, height, 0, DISTANCE};
	rectangle.links[2] = (linkage){rectangle.balls+2, rectangle.balls+3, width, 0, DISTANCE};
	rectangle.links[3] = (linkage){rectangle.balls+3, rectangle.balls+0, height, 0, DISTANCE};

	float corner_distance = v2_magnitude((vec2){width, height});
	rectangle.links[4] = (linkage){rectangle.balls+0, rectangle.balls+2, corner_distance, 0, DISTANCE};
	rectangle.links[5] = (linkage){rectangle.balls+1, rectangle.balls+3, corner_distance, 0, DISTANCE};

	return rectangle;
}

shape generate_cloth(float width, float height, int width_resolution, int height_resolution, float stiffness, vec2 starting_position) {
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
			//active_ball->radius = 5;
			active_ball->radius = 0.01;
			active_ball->mass = 1;
			if(column < width_resolution - 1) {
				cloth.links[cloth_link_count++] = (linkage){
					.a = active_ball,
					.b = active_ball + 1,
					.length = width/(width_resolution-1),
					.stiffness = stiffness,
					.type = ROPE_SPRING
				};
			}
			if(row < height_resolution - 1) {
				cloth.links[cloth_link_count++] = (linkage){
					.a = active_ball,
					.b = active_ball + width_resolution,
					.length = height/(height_resolution-1),
					.stiffness = stiffness,
					.type = ROPE_SPRING
				};
			}
		}
	}
	
	return cloth;
}

shape generate_rope(float length, int resolution, float stiffness, vec2 starting_position) {
	shape cloth;

	cloth.ball_count = resolution;
	cloth.balls = malloc(cloth.ball_count * sizeof(ball));
	//cloth.link_count = (width_resolution-1) * height_resolution + (height_resolution-1) * width_resolution;
	//2hw - w - h it simplifies down to this
	//cloth.link_count = 2 * height_resolution * width_resolution - width_resolution - height_resolution;
	cloth.link_count = resolution-1;
	cloth.links = malloc(cloth.link_count * sizeof(linkage));

	for(int i = 0; i < resolution; i++) {
		cloth.balls[i].position = v2_add((vec2){0, length * ((float)(i-resolution/2.)/(resolution-1))}, starting_position);
		cloth.balls[i].radius = length/resolution * 0.5;
		cloth.balls[i].mass = 1./resolution;
		if(i < resolution-1) {
			cloth.links[i] = (linkage) {
				.a = cloth.balls + i,
				.b = cloth.balls + i + 1,
				.length = length/(resolution-1),
				.stiffness = stiffness,
				.type = ROPE_SPRING
			};
		}
	}

	
	return cloth;
}

shape_3d generate_cube_3d(float width, float height, float depth, vec3 starting_position) {
	shape_3d cube;
	cube.balls = malloc(8 * sizeof(ball_3d));
	cube.ball_count = 8;
	cube.links = malloc(16 * sizeof(linkage_3d));
	cube.link_count = 16;

	cube.balls[0].position = v3_add((vec3){-width/2, -height/2, -depth/2}, starting_position);
	cube.balls[1].position = v3_add((vec3){width/2, -height/2, -depth/2}, starting_position);
	cube.balls[2].position = v3_add((vec3){width/2, height/2, -depth/2}, starting_position);
	cube.balls[3].position = v3_add((vec3){-width/2, height/2, -depth/2}, starting_position);

	cube.balls[4].position = v3_add((vec3){-width/2, -height/2, depth/2}, starting_position);
	cube.balls[5].position = v3_add((vec3){width/2, -height/2, depth/2}, starting_position);
	cube.balls[6].position = v3_add((vec3){width/2, height/2, depth/2}, starting_position);
	cube.balls[7].position = v3_add((vec3){-width/2, height/2, depth/2}, starting_position);

	set_velocity_3d(cube.balls+0, (vec3){0,0,0});
	set_velocity_3d(cube.balls+1, (vec3){0,0,0});
	set_velocity_3d(cube.balls+2, (vec3){0,0,0});
	set_velocity_3d(cube.balls+3, (vec3){0,0,0});
	set_velocity_3d(cube.balls+4, (vec3){0,0,0});
	set_velocity_3d(cube.balls+5, (vec3){0,0,0});
	set_velocity_3d(cube.balls+6, (vec3){0,0,0});
	set_velocity_3d(cube.balls+7, (vec3){0,0,0});

	cube.balls[0].radius = 0.1;
	cube.balls[1].radius = 0.1;
	cube.balls[2].radius = 0.1;
	cube.balls[3].radius = 0.1;

	cube.balls[4].radius = 0.1;
	cube.balls[5].radius = 0.1;
	cube.balls[6].radius = 0.1;
	cube.balls[7].radius = 0.1;

	cube.balls[0].mass = 1;
	cube.balls[1].mass = 1;
	cube.balls[2].mass = 1;
	cube.balls[3].mass = 1;

	cube.balls[4].mass = 1;
	cube.balls[5].mass = 1;
	cube.balls[6].mass = 1;
	cube.balls[7].mass = 1;

	/*
	cube.links[0] = (linkage){cube.balls+0, cube.balls+1, width, 0, DISTANCE};
	cube.links[1] = (linkage){cube.balls+1, cube.balls+2, height, 0, DISTANCE};
	cube.links[2] = (linkage){cube.balls+2, cube.balls+3, width, 0, DISTANCE};
	cube.links[3] = (linkage){cube.balls+3, cube.balls+0, height, 0, DISTANCE};
	*/
	cube.links[0] = (linkage_3d){cube.balls+0, cube.balls+1, width, 0, DISTANCE};
	cube.links[1] = (linkage_3d){cube.balls+1, cube.balls+2, height, 0, DISTANCE};
	cube.links[2] = (linkage_3d){cube.balls+2, cube.balls+3, width, 0, DISTANCE};
	cube.links[3] = (linkage_3d){cube.balls+3, cube.balls+0, height, 0, DISTANCE};

	cube.links[4] = (linkage_3d){cube.balls+4, cube.balls+5, width, 0, DISTANCE};
	cube.links[5] = (linkage_3d){cube.balls+5, cube.balls+6, height, 0, DISTANCE};
	cube.links[6] = (linkage_3d){cube.balls+6, cube.balls+7, width, 0, DISTANCE};
	cube.links[7] = (linkage_3d){cube.balls+7, cube.balls+4, height, 0, DISTANCE};

	cube.links[8] = (linkage_3d){cube.balls+0, cube.balls+4, depth, 0, DISTANCE};
	cube.links[9] = (linkage_3d){cube.balls+1, cube.balls+5, depth, 0, DISTANCE};
	cube.links[10] = (linkage_3d){cube.balls+2, cube.balls+6, depth, 0, DISTANCE};
	cube.links[11] = (linkage_3d){cube.balls+3, cube.balls+7, depth, 0, DISTANCE};

	float corner_distance = v3_magnitude((vec3){width, height, depth});

	cube.links[12] = (linkage_3d){cube.balls+0, cube.balls+6, corner_distance, 0, DISTANCE};
	cube.links[13] = (linkage_3d){cube.balls+1, cube.balls+7, corner_distance, 0, DISTANCE};
	cube.links[14] = (linkage_3d){cube.balls+2, cube.balls+4, corner_distance, 0, DISTANCE};
	cube.links[15] = (linkage_3d){cube.balls+3, cube.balls+5, corner_distance, 0, DISTANCE};

	/*float corner_distance = v2_magnitude((vec2){width, height});
	cube.links[4] = (linkage){cube.balls+0, cube.balls+2, corner_distance, 0, DISTANCE};
	cube.links[5] = (linkage){cube.balls+1, cube.balls+3, corner_distance, 0, DISTANCE};*/

	return cube;
}

// stiffness should be low because there are a lot of springs and the balls are real light!
shape_3d generate_cloth_3d(float width, float height, int width_resolution, int height_resolution, float stiffness, vec3 starting_position) {
	shape_3d cloth;

	cloth.ball_count = width_resolution * height_resolution;
	cloth.balls = malloc(cloth.ball_count * sizeof(ball_3d));
	//cloth.link_count = (width_resolution-1) * height_resolution + (height_resolution-1) * width_resolution;
	//2hw - w - h it simplifies down to this
	cloth.link_count = 2 * height_resolution * width_resolution - width_resolution - height_resolution;
	cloth.links = malloc(cloth.link_count * sizeof(linkage_3d));
	int cloth_link_count = 0;

	for(int row = 0; row < height_resolution; ++row) {
		for(int column = 0; column < width_resolution; ++column) {
			ball_3d *active_ball = &cloth.balls[row * width_resolution + column];
			active_ball->position = v3_add((vec3){(column-width_resolution/2.) * width/(width_resolution-1), (row-height_resolution/2.) * height/(height_resolution-1), 0}, starting_position);
			set_velocity_3d(active_ball, (vec3){0,0,0});
			//active_ball->radius = 1.0 / (width_resolution*height_resolution) * width * height;
			//active_ball->radius = (((width) * (height)) / ((width_resolution) * (height_resolution)));
			active_ball->radius = (((width) * (height)) / ((width_resolution + height_resolution)/2.) * 0.33);
			active_ball->mass = 1.0 / (width_resolution*height_resolution);
			if(column < width_resolution - 1) {
				cloth.links[cloth_link_count++] = (linkage_3d){
					.a = active_ball,
					.b = active_ball + 1,
					.length = width/(width_resolution-1),
					.stiffness = stiffness,
					.type = ROPE_SPRING
				};
			}
			if(row < height_resolution - 1) {
				cloth.links[cloth_link_count++] = (linkage_3d){
					.a = active_ball,
					.b = active_ball + width_resolution,
					.length = height/(height_resolution-1),
					.stiffness = stiffness,
					.type = ROPE_SPRING
				};
			}
		}
	}
	
	return cloth;

}
