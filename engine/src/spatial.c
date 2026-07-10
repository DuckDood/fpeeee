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

spatial_grid construct_grid_2d(int grid_width, int grid_height, float element_size) {
	spatial_grid grid;
	grid.width = grid_width;
	grid.height = grid_height;
	grid.element_size = element_size;
	grid.depth = 1;
	grid.ball_counts = malloc((grid.width * grid.height * grid.depth + 1) * sizeof(int));

	grid.partitions = malloc(sizeof(spatial_partition) * (grid_width * grid_height + 1)); // +1 for an out of bounds partition
	for(int i = 0; i < grid_width * grid_height + 1; ++i) {
		grid.partitions[i].ball_count = -1;
		grid.partitions[i].ball_offset = -1;
	}
	grid.ball_map = NULL;
	grid.ball_count = 0;
	printf("partition memory footprint (kb): %zu\n", (sizeof(spatial_partition) * (grid_width * grid_height + 1))/1000);

	return grid;
}

void destroy_grid_2d(spatial_grid *grid) {
	for(int i = 0; i < grid->width * grid->height + 1; ++i) {
		grid->partitions[i].ball_count = -1;
		grid->partitions[i].ball_offset = -1;
	}
	free(grid->partitions);
	free(grid->ball_map);
	free(grid->ball_counts);
	grid->ball_count = -1;
}

void update_grid_2d(spatial_grid *grid, ball_2d *balls, int ball_count) {

	memset(grid->ball_counts, 0, (grid->width * grid->height * grid->depth + 1) * sizeof(int));
	// ball map shenanigans so that the memory positions of balls are stable

	if(grid->ball_count != ball_count) {
		free(grid->ball_map); // should be null on first run so
		grid->ball_map = malloc(sizeof(int) * ball_count);
		grid->ball_count = ball_count;
	}

	for(int i = 0; i < ball_count; ++i) {
		ball_2d *ball = balls + i;
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		//if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;
		int partition_index = ball_row * grid->width + ball_column;
		if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) partition_index = grid->width * grid->height;

		grid->ball_counts[partition_index]++;
	}
	int values_before = 0;
	for(int i = 0; i < grid->width * grid->height + 1; ++i) {
		grid->partitions[i].ball_offset = values_before;
		grid->partitions[i].ball_count = 0;
		values_before+=grid->ball_counts[i];
	}
	for(int i = 0; i < ball_count; ++i) {
		ball_2d *ball = balls + i;
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		//if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;
		int partition_index = ball_row * grid->width + ball_column;
		if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) partition_index = grid->width * grid->height;

		spatial_partition *partition = grid->partitions + partition_index;
		int offset = partition->ball_offset;
		
		grid->ball_map[offset + grid->partitions[partition_index].ball_count++] = i;
	}
}

void spatial_collision_2d(spatial_grid *grid, ball_2d *balls, int ball_count) {
	for(int i = 0; i < ball_count; ++i) {
		//ball_2d *ball = balls + i;
		ball_2d *ball = balls + grid->ball_map[i];
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

				spatial_partition * restrict partition = grid->partitions + partition_index;

				for(int i = 0; i < partition->ball_count; ++i) {
					//ball_2d *check_ball = balls + partition->ball_offset + i;
					ball_2d *check_ball = balls + grid->ball_map[partition->ball_offset + i];

					if(check_ball == ball) continue; // evil evil pointer comparison
					check_and_resolve_balls_2d(ball, check_ball);
				}
			}
		}
	}

}

// 3d

spatial_grid construct_grid_3d(int grid_width, int grid_height, int grid_depth, float element_size) {
	spatial_grid grid;
	grid.width = grid_width;
	grid.height = grid_height;
	grid.depth = grid_depth;
	grid.element_size = element_size;
	grid.ball_counts = malloc((grid.width * grid.height * grid.depth + 1) * sizeof(int));

	grid.partitions = malloc(sizeof(spatial_partition) * (grid_width * grid_height * grid_depth + 1)); // +1 for an out of bounds partition
	for(int i = 0; i < grid_width * grid_height * grid_depth + 1; ++i) {
		grid.partitions[i].ball_count = -1;
		grid.partitions[i].ball_offset = -1;
	}
	grid.ball_map = NULL;
	grid.ball_count = 0;
	printf("partition memory footprint (kb): %zu\n", (sizeof(spatial_partition) * (grid_width * grid_height * grid_depth + 1))/1000);

	return grid;
}

void destroy_grid_3d(spatial_grid *grid) {
	for(int i = 0; i < grid->width * grid->height * grid->depth + 1; ++i) {
		grid->partitions[i].ball_count = -1;
		grid->partitions[i].ball_offset = -1;
	}
	free(grid->partitions);
	free(grid->ball_map);
	free(grid->ball_counts);
	grid->ball_count = -1;
}

void update_grid_3d(spatial_grid *grid, ball_3d *balls, int ball_count) {

	//int *ball_counts = calloc(grid->width * grid->height * grid->depth + 1, sizeof(int));
	memset(grid->ball_counts, 0, (grid->width * grid->height * grid->depth + 1) * sizeof(int));
	// ball map shenanigans so that the memory positions of balls are stable

	if(grid->ball_count != ball_count) {
		free(grid->ball_map); // should be null on first run so
		grid->ball_map = malloc(sizeof(int) * ball_count);
		grid->ball_count = ball_count;
	}

	for(int i = 0; i < ball_count; ++i) {
		ball_3d *ball = balls + i;
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		int ball_layer = ball->position.z / grid->element_size + grid->depth * 0.5;
		//if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;
		int partition_index = ball_layer * grid->width * grid->height + ball_row * grid->width + ball_column;

		if(ball_column < 0 || ball_column > grid->width - 1
			|| ball_row < 0 || ball_row > grid->height - 1
			|| ball_layer < 0 || ball_layer > grid->depth - 1)
			partition_index = grid->width * grid->height * grid->depth;

		grid->ball_counts[partition_index]++;
	}
	int values_before = 0;
	for(int i = 0; i < grid->width * grid->height * grid->depth + 1; ++i) {
		grid->partitions[i].ball_offset = values_before;
		grid->partitions[i].ball_count = 0;
		values_before+=grid->ball_counts[i];
	}
	for(int i = 0; i < ball_count; ++i) {
		ball_3d *ball = balls + i;
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		int ball_layer = ball->position.z / grid->element_size + grid->depth * 0.5;

		int partition_index = ball_layer * grid->width * grid->height + ball_row * grid->width + ball_column;
		if(ball_column < 0 || ball_column > grid->width - 1
			|| ball_row < 0 || ball_row > grid->height - 1
			|| ball_layer < 0 || ball_layer > grid->depth - 1)
			partition_index = grid->width * grid->height * grid->depth;

		spatial_partition *partition = grid->partitions + partition_index;
		int offset = partition->ball_offset;
		
		grid->ball_map[offset + grid->partitions[partition_index].ball_count++] = i;
	}
	//free(ball_counts);
}

void spatial_collision_3d(spatial_grid *grid, ball_3d *balls, int ball_count) {
	for(int i = 0; i < ball_count; ++i) {
		ball_3d *ball = balls + grid->ball_map[i];
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		int ball_layer = ball->position.z / grid->element_size + grid->depth * 0.5;

		if(ball_column < 0 || ball_column > grid->width - 1
			|| ball_row < 0 || ball_row > grid->height - 1
			|| ball_layer < 0 || ball_layer > grid->depth - 1)
			continue;


		for(int layer = -1; layer < 2; ++layer) {
				int total_layer = ball_layer + layer;
				if(total_layer < 0 || total_layer > grid->depth-1) continue;
			for(int row = -1; row < 2; ++row) {
				int total_row = ball_row + row;
				if(total_row < 0 || total_row > grid->height-1) continue;

				for(int column = -1; column < 2; ++column) {
					int total_column = ball_column + column;
					if(total_column < 0 || total_column > grid->width-1) continue;

					int partition_index = total_layer * grid->width * grid->height + total_row * grid->width + total_column;

					if(partition_index > grid->width * grid->height * grid->depth) {
						partition_index = grid->width * grid->height * grid->depth;
					}

					spatial_partition * restrict partition = grid->partitions + partition_index;

					for(int i = 0; i < partition->ball_count; ++i) {
						//ball_2d *check_ball = balls + partition->ball_offset + i;
						ball_3d *check_ball = balls + grid->ball_map[partition->ball_offset + i];

						if(check_ball == ball) continue; // evil evil pointer comparison
						check_and_resolve_balls_3d(ball, check_ball);
					}
				}
			}
		}
	}

}
