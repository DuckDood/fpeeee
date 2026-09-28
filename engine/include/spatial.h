#include <physics.h>
#include <constraints.h>
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

typedef struct {
	int min_column,
	min_row,
	max_column,
	max_row;
} spatial_bounds_2d;

typedef struct {
	int min_column,
	min_row,
	min_layer,
	max_column,
	max_row,
	max_layer;
} spatial_bounds_3d;

spatial_grid construct_grid_2d(int grid_width, int grid_height, float element_size);
void destroy_grid_2d(spatial_grid *grid);
void update_grid_2d(spatial_grid *grid, ball_2d *balls, int ball_count);
void spatial_collision_2d(spatial_grid *grid, ball_2d *balls, int ball_count);
void spatial_constraint_2d(spatial_grid *grid, spatial_bounds_2d bounds, int constraint_index, ball_2d *balls, constraint_function_2d constraint, vec2 **vectors, vec2 *gradients, float *inv_weights, int size, void *arguments, float deltatime, float compliance, constraint_types type);


spatial_grid construct_grid_3d(int grid_width, int grid_height, int grid_depth, float element_size);
void destroy_grid_3d(spatial_grid *grid);
void update_grid_3d(spatial_grid *grid, ball_3d *balls, int ball_count);
void spatial_collision_3d(spatial_grid *grid, ball_3d *balls, int ball_count);
void spatial_constraint_3d(spatial_grid *grid, spatial_bounds_3d bounds, int constraint_index, ball_3d *balls, constraint_function_3d constraint, vec3 **vectors, vec3 *gradients, float *inv_weights, int size, void *arguments, float deltatime, float compliance, constraint_types type);
