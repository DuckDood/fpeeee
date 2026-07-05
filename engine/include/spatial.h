#include <physics.h>
#include <stddef.h>


typedef struct {
	int ball_count;
	int ball_offset;
} spatial_partition_2d;

typedef struct {
		float element_size; // should be about the diameter of the biggest ball that will be in this grid
		int width;
		int height;

		spatial_partition_2d *partitions;
} spatial_grid_2d;

spatial_grid_2d construct_grid_2d(int grid_width, int grid_height, int partition_max_particles, float element_size);
void destroy_grid_2d(spatial_grid_2d *grid);
void update_grid_2d(spatial_grid_2d *grid, ball_2d *balls, int ball_count);
void spatial_collision_2d(spatial_grid_2d *grid, ball_2d *balls, int ball_count);
