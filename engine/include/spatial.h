#include <physics.h>
#include <stddef.h>


typedef struct {
	int ball_count;
	int ball_offset;
} spatial_partition;

typedef struct {
		float element_size; // should be about the diameter of the biggest ball that will be in this grid
		int width;
		int height;
		int depth;

		spatial_partition *partitions;
		int *ball_map;
		int ball_count;

		int *ball_counts;
} spatial_grid;

spatial_grid construct_grid_2d(int grid_width, int grid_height, float element_size);
void destroy_grid_2d(spatial_grid *grid);
void update_grid_2d(spatial_grid *grid, ball_2d *balls, int ball_count);
void spatial_collision_2d(spatial_grid *grid, ball_2d *balls, int ball_count);


spatial_grid construct_grid_3d(int grid_width, int grid_height, int grid_depth, float element_size);
void destroy_grid_3d(spatial_grid *grid);
void update_grid_3d(spatial_grid *grid, ball_3d *balls, int ball_count);
void spatial_collision_3d(spatial_grid *grid, ball_3d *balls, int ball_count);
