#include <float.h>
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

void collide_ball_partition_2d(spatial_partition * restrict partition, spatial_grid *grid, ball_2d *balls, ball_2d *ball) {
	for(int j = 0; j < partition->ball_count; ++j) {
		//if(partition->ball_offset + j < i) continue;
		//ball_2d *check_ball = balls + partition->ball_offset + i;
		ball_2d *check_ball = balls + grid->ball_map[partition->ball_offset + j];

		if(check_ball == ball) continue; // evil evil pointer comparison
		check_and_resolve_balls_2d(ball, check_ball);
	}
}

void spatial_collision_2d(spatial_grid *grid, ball_2d *balls, int ball_count) {
	for(int i = 0; i < ball_count; ++i) {
		//ball_2d *ball = balls + i;
		ball_2d *ball = balls + grid->ball_map[i];
		int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
		int ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;
		int partition_index = ball_row * grid->width + ball_column;
		[[maybe_unused]] spatial_partition * restrict partition = grid->partitions + partition_index;
		//collide_ball_partition_2d(partition, grid, balls, ball);
/*
		for(int row = -1; row < 2; ++row) {
			int total_row = (ball->position.y + ball->radius * row) / grid->element_size + grid->height * 0.5;
			if(total_row < 0 || total_row > grid->height-1) continue;

			for(int column = -1; column < 2; ++column) {
				int total_column = (ball->position.x + ball->radius * column) / grid->element_size + grid->width * 0.5;
				if(total_column < 0 || total_column > grid->width-1) continue;
				if(total_row == ball_row && total_column == ball_column) continue;

				int partition_index = total_row * grid->width + total_column;
				if(partition_index > grid->width * grid->height) {
					partition_index = grid->width * grid->height;
				}

				spatial_partition * restrict partition = grid->partitions + partition_index;
				collide_ball_partition_2d(partition, grid, balls, ball);
				checkcount++;
			}
		}*/
		//printf("ck%i\n", checkcount);

		float error_buffer = ball->radius;

		int min_ball_column = (ball->position.x - ball->radius - error_buffer) / grid->element_size + grid->width * 0.5;
		int min_ball_row = (ball->position.y - ball->radius - error_buffer) / grid->element_size + grid->height * 0.5;

		int max_ball_column = (ball->position.x + ball->radius + error_buffer) / grid->element_size + grid->width * 0.5;
		int max_ball_row = (ball->position.y + ball->radius + error_buffer) / grid->element_size + grid->height * 0.5;

		//printf("mincol: %i, maxcol: %i\n", min_ball_column, max_ball_column);
		//printf("minrow: %i, maxrow: %i\n", min_ball_row, max_ball_row);


		for(int row = min_ball_row; row <= max_ball_row; ++row) {
			if(row >= grid->height - 1) continue;
			if(row < 0) continue;
			for(int column = min_ball_column; column <= max_ball_column; ++column) {
				if(column >= grid->width - 1) continue;
				if(column < 0) continue;
				int partition_index = row * grid->width + column;
				spatial_partition * restrict partition = grid->partitions + partition_index;
				collide_ball_partition_2d(partition, grid, balls, ball);
			}
		}

		/*
		for(int row = -1; row < 2; ++row) {
		//	int total_row = ball_row + row;
			//if(total_row < 0 || total_row > grid->height-1) continue;
			//int ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
			int new_ball_row = (ball->position.y + ball->radius * row) / grid->element_size + grid->height * 0.5;
			//if(new_ball_row == ball_row) continue;

			for(int column = -1; column < 2; ++column) {
				int new_ball_column = (ball->position.x + ball->radius * row) / grid->element_size + grid->width * 0.5;
				//int new_ball_column = ball->position.x / grid->element_size + grid->width * 0.5;
				//if(new_ball_column == ball_column) continue;

				int partition_index = new_ball_row * grid->width + new_ball_column;

				if(new_ball_column > grid->width) {
					printf("columnabove\n");
					goto dontcol;
				}
				if(new_ball_column < 0) {
					printf("columnbelow %i\n", new_ball_column);
					goto dontcol;
				}
				if(new_ball_row > grid->height) {
					printf("rowabove\n");
					goto dontcol;
				}
				if(new_ball_row < 0) {
					printf("rowbelow\n");
					goto dontcol;
				}

				partition = grid->partitions + partition_index;
				collide_ball_partition_2d(partition, grid, balls, ball);
dontcol:
			}
		}*/


		/*

		int min_ball_column = (ball->position.x - ball->radius) / grid->element_size + grid->width * 0.5;
		//if(new_ball_column == ball_column) new_ball_column = (ball->position.x - ball->radius) / grid->element_size + grid->width * 0.5;
		int min_ball_row = (ball->position.y - ball->radius) / grid->element_size + grid->height * 0.5;
		//if(new_ball_row == ball_row) new_ball_row = (ball->position.y - ball->radius) / grid->element_size + grid->height * 0.5;

		int max_ball_column = (ball->position.x + ball->radius) / grid->element_size + grid->width * 0.5;
		//if(new_ball_column == ball_column) new_ball_column = (ball->position.x - ball->radius) / grid->element_size + grid->width * 0.5;
		int max_ball_row = (ball->position.y + ball->radius) / grid->element_size + grid->height * 0.5;
		//if(new_ball_row == ball_row) new_ball_row = (ball->position.y - ball->radius) / grid->element_size + grid->height * 0.5;
		
		
		
		if(min_ball_column != ball_column) {
			partition_index = ball_row * grid->width + min_ball_column;
			partition = grid->partitions + partition_index;
			collide_ball_partition_2d(partition, grid, balls, ball);
		}

		if(max_ball_column != ball_column) {
			partition_index = ball_row * grid->width + max_ball_column;
			partition = grid->partitions + partition_index;
			collide_ball_partition_2d(partition, grid, balls, ball);
		}
    	
		if(max_ball_row != ball_row) {
			partition_index = max_ball_row * grid->width + ball_column;
			partition = grid->partitions + partition_index;
			collide_ball_partition_2d(partition, grid, balls, ball);
		}
    	
		if(min_ball_row != ball_row) {
			partition_index = min_ball_row * grid->width + ball_column;
			partition = grid->partitions + partition_index;
			collide_ball_partition_2d(partition, grid, balls, ball);
		}
    	
		if(min_ball_column != ball_column && min_ball_row != ball_row) {
			partition_index = min_ball_row * grid->width + min_ball_column;
			partition = grid->partitions + partition_index;
			collide_ball_partition_2d(partition, grid, balls, ball);
		}

		if(max_ball_column != ball_column && max_ball_row != ball_row) {
			partition_index = max_ball_row * grid->width + max_ball_column;
			partition = grid->partitions + partition_index;
			collide_ball_partition_2d(partition, grid, balls, ball);
		}
    	
		if(min_ball_column != ball_column && max_ball_row != ball_row) {
			partition_index = max_ball_row * grid->width + min_ball_column;
			partition = grid->partitions + partition_index;
			collide_ball_partition_2d(partition, grid, balls, ball);
		}
    	
		if(max_ball_column != ball_column && min_ball_row != ball_row) {
			partition_index = min_ball_row * grid->width + max_ball_column;
			partition = grid->partitions + partition_index;
			collide_ball_partition_2d(partition, grid, balls, ball);
		}*/



		/*
		for(int j = 0; j < partition->ball_count; ++j) {
			if(partition->ball_offset + j < i) continue;
			//ball_2d *check_ball = balls + partition->ball_offset + i;
			ball_2d *check_ball = balls + grid->ball_map[partition->ball_offset + j];

			if(check_ball == ball) continue; // evil evil pointer comparison
			check_and_resolve_balls_2d(ball, check_ball);
		}*/
		/*
		int ball_column_side = 0;
		int ball_row_side = 0;
		
		int new_ball_column = (ball->position.x + ball->radius) / grid->element_size + grid->width * 0.5;
		if(new_ball_column != ball_column) ball_column_side = 1;

		new_ball_column = (ball->position.x - ball->radius) / grid->element_size + grid->width * 0.5;
		if(new_ball_column != ball_column) ball_column_side = -1;

		int new_ball_row = ( ball->position.y + ball->radius) / grid->element_size + grid->height * 0.5;
		if(new_ball_row != ball_row) ball_row_side = 1;

		new_ball_row = ( ball->position.y - ball->radius) / grid->element_size + grid->height * 0.5;
		if(new_ball_row != ball_row) ball_row_side = -1;

		//if(ball_column < 0 || ball_column > grid->width - 1 || ball_row < 0 || ball_row > grid->height - 1) continue;
		
		//if(new_ball_column < 0 || new_ball_column > grid->width - 1 || new_ball_row < 0 || new_ball_row > grid->height - 1) goto skip1;
		if(!(ball_row + ball_row_side < 0 || ball_row + ball_row_side > grid->height - 1) && ball_row != 0) {
			int new_partition_index = (ball_row + ball_row_side) * grid->width + ball_column;
			spatial_partition * restrict new_partition = grid->partitions + new_partition_index;
			collide_ball_partition_2d(new_partition, grid, balls, ball);
		}
		if(!(ball_column + ball_column_side < 0 || ball_column + ball_column_side > grid->width - 1) && ball_column !=0) {
			int new_partition_index = ball_row * grid->width + ball_column + ball_column_side;
			spatial_partition * restrict new_partition = grid->partitions + new_partition_index;
			collide_ball_partition_2d(new_partition, grid, balls, ball);
		}
		if(!(ball_column + ball_column_side < 0 || ball_column + ball_column_side > grid->width - 1) && !(ball_row + ball_row_side < 0 || ball_row + ball_row_side > grid->height - 1) && ball_column != 0 && ball_row != 0) {
			int new_partition_index = (ball_row + ball_row_side) * grid->width + ball_column + ball_column_side;
			spatial_partition * restrict new_partition = grid->partitions + new_partition_index;
			collide_ball_partition_2d(new_partition, grid, balls, ball);
		}*/

		

		/*int new_ball_column = (ball->position.x + ball->radius) / grid->element_size + grid->width * 0.5;
		int new_ball_row = ball->position.y / grid->element_size + grid->height * 0.5;
		if(new_ball_column != ball_column) {
			if(!(new_ball_column < 0 || new_ball_column > grid->width - 1 || new_ball_row < 0 || new_ball_row > grid->height - 1)) {
				partition_index = new_ball_row * grid->width + new_ball_column;
				partition = grid->partitions + partition_index;
				collide_ball_partition_2d(partition, grid, balls, ball);
			}
		}*/
		//ball_row = ball->position.y / grid->element_size + grid->height * 0.5;

		/*for(int row = -1; row < 2; ++row) {
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
				collide_ball_partition_2d(partition, grid, balls, ball);
			}
		}*/
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

void collide_ball_partition_3d(spatial_partition * restrict partition, spatial_grid *grid, ball_3d *balls, ball_3d *ball) {
	/*for(int j = 0; j < partition->ball_count; ++j) {
		//if(partition->ball_offset + j < i) continue;
		//ball_2d *check_ball = balls + partition->ball_offset + i;
		ball_2d *check_ball = balls + grid->ball_map[partition->ball_offset + j];

		if(check_ball == ball) continue; // evil evil pointer comparison
		check_and_resolve_balls_2d(ball, check_ball);
	}*/

	for(int i = 0; i < partition->ball_count; ++i) {
		//ball_2d *check_ball = balls + partition->ball_offset + i;
		ball_3d *check_ball = balls + grid->ball_map[partition->ball_offset + i];

		if(check_ball == ball) continue; // evil evil pointer comparison
		check_and_resolve_balls_3d(ball, check_ball);
	}
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

		// this doesnt speed up the spatial collisions, but it does allow balls that are bigger than the size of the grid cell to exist just with detriments to check counts
		float error_buffer = ball->radius; // 
		int min_ball_column = (ball->position.x - ball->radius - error_buffer) / grid->element_size + grid->width * 0.5;
		int min_ball_row = (ball->position.y - ball->radius - error_buffer) / grid->element_size + grid->height * 0.5;
		int min_ball_layer = (ball->position.z - ball->radius - error_buffer) / grid->element_size + grid->depth * 0.5;

		int max_ball_column = (ball->position.x + ball->radius + error_buffer) / grid->element_size + grid->width * 0.5;
		int max_ball_row = (ball->position.y + ball->radius + error_buffer) / grid->element_size + grid->height * 0.5;
		int max_ball_layer = (ball->position.z + ball->radius + error_buffer) / grid->element_size + grid->depth * 0.5;

		for(int layer = min_ball_layer; layer <= max_ball_layer; ++layer) {
			if(layer >= grid->depth - 1) continue;
			if(layer < 0) continue;
			for(int row = min_ball_row; row <= max_ball_row; ++row) {
				if(row >= grid->height - 1) continue;
				if(row < 0) continue;
				for(int column = min_ball_column; column <= max_ball_column; ++column) {
					if(column >= grid->width - 1) continue;
					if(column < 0) continue;
					//int partition_index = row * grid->width + column;
					int partition_index = layer * grid->width * grid->height + row * grid->width + column;
					spatial_partition * restrict partition = grid->partitions + partition_index;
					collide_ball_partition_3d(partition, grid, balls, ball);
				}
			}
		}

		/*
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

					collide_ball_partition_3d(partition, grid, balls, ball);
				}
			}
		}*/
	}

}
