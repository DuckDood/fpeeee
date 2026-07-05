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

	grid.partitions = malloc(sizeof(spatial_partition_2d) * (grid_width * grid_height + 1)); // +1 for an out of bounds partition
	for(int i = 0; i < grid_width * grid_height + 1; ++i) {
		grid.partitions[i].ball_count = -1;
		grid.partitions[i].ball_offset = -1;
	}
	printf("partition memory footprint (kb): %zu\n", (sizeof(spatial_partition_2d) * (grid_width * grid_height + 1))/1000);

	return grid;
}

void destroy_grid_2d(spatial_grid_2d *grid) {
	for(int i = 0; i < grid->width * grid->height + 1; ++i) {
		grid->partitions[i].ball_count = -1;
		grid->partitions[i].ball_offset = -1;
	}
	free(grid->partitions);
}

void update_grid_2d(spatial_grid_2d *grid, ball_2d *balls, int ball_count) {
	int *ball_counts = calloc(grid->width * grid->height + 1, sizeof(int));

	ball_2d *new_balls = malloc(sizeof(ball_2d) * ball_count);
	for(int i = 0; i < ball_count; ++i) {
		ball_2d *ball = balls + i;
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		//if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;
		int partition_index = ball_row * grid->width + ball_column;
		if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) partition_index = grid->width * grid->height;

		spatial_partition_2d *partition = grid->partitions + partition_index;
		ball_counts[partition_index]++;
	}
	int values_before = 0;
	for(int i = 0; i < grid->width * grid->height + 1; ++i) {
		grid->partitions[i].ball_offset = values_before;
		grid->partitions[i].ball_count = 0;
		values_before+=ball_counts[i];
	}
	for(int i = 0; i < ball_count; ++i) {
		ball_2d *ball = balls + i;
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		//if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;
		int partition_index = ball_row * grid->width + ball_column;
		if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) partition_index = grid->width * grid->height;

		spatial_partition_2d *partition = grid->partitions + partition_index;
		int offset = partition->ball_offset;
		
		new_balls[offset + grid->partitions[partition_index].ball_count++] = *ball;
	}
	memcpy(balls, new_balls, sizeof(ball_2d) * ball_count); // sheesh

	free(new_balls);
	free(ball_counts);
}

void spatial_collision_2d(spatial_grid_2d *grid, ball_2d *balls, int ball_count) {
	for(int i = 0; i < ball_count; ++i) {
		ball_2d *ball = balls + i;
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;

		for(int row = -1; row < 2; ++row) {
			int total_row = ball_row + row;
			if(total_row < 0 || total_row > grid->height-1) continue;

			for(int column = -1; column < 2; ++column) {
				int total_column = ball_column + column;
				if(total_column < 0 || total_column > grid->width-1) continue;

				int partition_index = total_row * grid->width + total_column;
				if(partition_index > grid->width * grid->height) {
					partition_index = grid->width * grid->height;
				}

				spatial_partition_2d *partition = grid->partitions + partition_index;

				for(int i = 0; i < partition->ball_count; ++i) {
					ball_2d *check_ball = balls + partition->ball_offset + i;
					if(check_ball == ball) continue; // evil evil pointer comparison
					check_and_resolve_balls_2d(ball, check_ball);
				}
			}
		}
	}

}
