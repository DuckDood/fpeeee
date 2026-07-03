#include <spatial.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#define MINIMUM_VECTOR_SIZE 16

int point_aabb_collision(vec2 position, vec2 bb_position, vec2 bb_dimension) {
	return
		position.x <= bb_position.x + bb_dimension.x &&
		position.x > bb_position.x &&

		position.y <= bb_position.y + bb_dimension.y &&
		position.y > bb_position.y
		;
}

spatial_grid_2d construct_grid_2d(int grid_width, int grid_height, int partition_start_particles, float element_size) {
	spatial_grid_2d grid;
	grid.width = grid_width;
	grid.height = grid_height;
	grid.element_size = element_size;

	grid.partitions = malloc(sizeof(spatial_partition_2d) * grid_width * grid_height);
	for(int i = 0; i < grid_width * grid_height; ++i) {
		grid.partitions[i].ball_capacity = partition_start_particles; // something something minimum vector size
		grid.partitions[i].ball_count = 0;
		grid.partitions[i].ball_list = malloc(sizeof(ball_2d*) * partition_start_particles);
	}

	return grid;
}

void destroy_grid_2d(spatial_grid_2d *grid) {
	for(int i = 0; i < grid->width * grid->height; ++i) {
		free(grid->partitions[i].ball_list);
		grid->partitions[i].ball_count = 0;
		grid->partitions[i].ball_capacity = 0;
	}
	free(grid->partitions);
}

void update_grid_2d(spatial_grid_2d *grid, ball_2d *balls, int ball_count) {
	
	for(int i = 0; i < grid->width * grid->height; ++i) {
		grid->partitions[i].ball_count = 0;
	}
	ball_2d **indices = calloc(ball_count, sizeof(ball_2d*));
	unsigned int indices_size;
	// ball_count * height * width iterations might be a bad idea

	for(int row = 0; row < grid->height; ++row) {
		for(int column = 0; column < grid->width; ++column) {
			spatial_partition_2d *partition = grid->partitions + (row * grid->width + column); // pointer arithmetic if you don't know
			indices_size = 0;
			// make this a 'full update' and then a 'relative update' which only checks for balls in the neighbour
			// might not work though i might need to make a copy of the data to do that because it checks already changed data
			for(int i = 0; i < ball_count; ++i) {
				ball_2d *current_ball = balls + i;

				if(point_aabb_collision(current_ball->position, (vec2){(column - grid->width*0.5) * grid->element_size, (row - grid->height*0.5) * grid->element_size}, (vec2){grid->element_size, grid->element_size})) {
					indices[indices_size++] = current_ball;
				}
			}

			float load_factor = (float)indices_size / partition->ball_capacity;
			if(load_factor > 1) {
				int resize_amount = indices_size * 1.5; // give some wriggle room
				partition->ball_capacity = resize_amount;
				free(partition->ball_list);
				partition->ball_list = malloc(resize_amount * sizeof(ball_2d*));
			}
			else if(load_factor < 0.25 && partition->ball_capacity > MINIMUM_VECTOR_SIZE) {
				int resize_amount = indices_size * 1.5; // idk if this should have wriggle room though
				partition->ball_capacity = resize_amount;
				free(partition->ball_list);
				partition->ball_list = malloc(resize_amount * sizeof(ball_2d*));
			}

			partition->ball_count = indices_size;
			memcpy(partition->ball_list, indices, indices_size * sizeof(ball_2d*));
		}
	}
	free(indices);
	/*
	for(int i = 0; i < grid->width * grid->height; ++i) {
		grid->partitions[i].ball_count = 0;
	}
	for(int i = 0; i < ball_count; ++i) {
		ball_2d *ball = balls + i;
		// maybe?
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;

		spatial_partition_2d *partition = grid->partitions + (ball_row * grid->width + ball_column);
		partition->ball_count++;
		float load_factor = (float)partition->ball_count / partition->ball_capacity;
		if(load_factor > 1) {
			int resize_amount = partition->ball_count * 1.5; // give some wriggle room
			partition->ball_capacity = resize_amount;
			partition->ball_list = realloc(partition->ball_list, resize_amount * sizeof(ball_2d*));
		}
		*/
		/*else if(load_factor < 0.25 && partition->ball_capacity > MINIMUM_VECTOR_SIZE) {
			int resize_amount = partition->ball_count * 1.5; // give some wriggle room
			partition->ball_capacity = resize_amount;
			free(partition->ball_list);
			partition->ball_list = malloc(resize_amount * sizeof(ball_2d*));
		}*/
	/*
		partition->ball_list[partition->ball_count-1] = ball;
	}*/

}

void spatial_collision_2d(spatial_grid_2d *grid, ball_2d *balls, int ball_count) {
	for(int major_row = 0; major_row < grid->height; ++major_row) {
		for(int major_column = 0; major_column < grid->width; ++major_column) {
			spatial_partition_2d *major_partition = grid->partitions + (major_row * grid->width + major_column);
			// maybe dont do this because it might do multiple collisions per two balls but whatever
			// ill figure it out
			for(int i = 0; i < major_partition->ball_count; ++i) {
				ball_2d *major_ball = major_partition->ball_list[i];
				for(int minor_row = -1; minor_row < 2; ++minor_row) {
					int total_row = major_row + minor_row;
					if(total_row < 0 || total_row > grid->height-1) continue;
					for(int minor_column = -1; minor_column < 2; ++minor_column) {
						int total_column = major_column + minor_column;
						if(total_column < 0 || total_column > grid->width-1) continue;
						spatial_partition_2d *minor_partition = grid->partitions + (total_row * grid->width + total_column);
						for(int ball = 0; ball < minor_partition->ball_count; ++ball) {
							ball_2d *minor_ball = minor_partition->ball_list[ball];
							if(major_ball == minor_ball) continue;
							check_and_resolve_balls_2d(major_ball, minor_ball);
						}
					}
				}
			}
		}
	}
}
