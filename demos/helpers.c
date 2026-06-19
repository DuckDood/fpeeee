#include "helpers.h"
#include <matrix.h>
#include <math.h>

void draw_circle(SDL_Renderer *renderer, ball b, int resolution, camera cam) {
	int WIDTH = cam.width;
	int HEIGHT = cam.height;
	vec3 camera_position = cam.position;
	vec3 camera_rotation = cam.rotation;
	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;

	for(float angle = 0; angle < 3.14159*2; angle += (3.14159*2)/resolution) {
		vec2 segment_1;
		segment_1.x = b.position.x + cos(angle)*b.radius;
		segment_1.y = b.position.y + sin(angle)*b.radius;

		vec2 segment_2;
		segment_2.x = b.position.x + cos(angle + (3.14159*2)/resolution)*b.radius;
		segment_2.y = b.position.y + sin(angle + (3.14159*2)/resolution)*b.radius;

		vec3 segment_1_relative = v3_sub((vec3){segment_1.x, segment_1.y, 0}, camera_position);
		segment_1_relative.y *= -1;
		mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));
		segment_1_relative = m3_v3_mult(camera_rotation_matrix, segment_1_relative);

		vec3 segment_2_relative = v3_sub((vec3){segment_2.x, segment_2.y, 0}, camera_position);
		segment_2_relative.y *= -1;
		segment_2_relative = m3_v3_mult(camera_rotation_matrix, segment_2_relative);

		if (segment_1_relative.z < 0) return;
		segment_1_relative.x /= segment_1_relative.z;
		segment_1_relative.y /= segment_1_relative.z;

		segment_1_relative.x *= inverse_aspect_ratio;

		segment_1_relative.x /= 2;
		segment_1_relative.y /= 2;
		segment_1_relative.x += 0.5;
		segment_1_relative.y += 0.5;
		segment_1_relative.x *= WIDTH;
		segment_1_relative.y *= HEIGHT;

		if (segment_2_relative.z < 0) return;
		segment_2_relative.x /= segment_2_relative.z;
		segment_2_relative.y /= segment_2_relative.z;
		
		segment_2_relative.x *= inverse_aspect_ratio;

		segment_2_relative.x /= 2;
		segment_2_relative.y /= 2;
		segment_2_relative.x += 0.5;
		segment_2_relative.y += 0.5;
		segment_2_relative.x *= WIDTH;
		segment_2_relative.y *= HEIGHT;

		SDL_RenderLine(renderer,
				segment_1_relative.x,
				segment_1_relative.y,

				segment_2_relative.x,
				segment_2_relative.y
				);
	}
}

void draw_circle_3d(SDL_Renderer *renderer, ball_3d b, int resolution, camera cam) {
	int WIDTH = cam.width;
	int HEIGHT = cam.height;
	vec3 camera_position = cam.position;
	vec3 camera_rotation = cam.rotation;

	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;
	vec3 b_position_relative = v3_sub(b.position, camera_position);
	b_position_relative.y *= -1;
	mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));
	b_position_relative = m3_v3_mult(camera_rotation_matrix, b_position_relative);

	b.radius /= b_position_relative.z;
	if (b_position_relative.z < 0) return;
	b_position_relative.x /= b_position_relative.z;
	b_position_relative.y /= b_position_relative.z;

	for(float angle = 0; angle < 3.14159*2; angle += (3.14159*2)/resolution) {
		SDL_RenderLine(renderer,
				((b_position_relative.x + cos(angle)*b.radius) * inverse_aspect_ratio * 0.5 + 0.5) * WIDTH,
				((b_position_relative.y + sin(angle)*b.radius) * 0.5 + 0.5) * HEIGHT,
				((b_position_relative.x + cos(angle + (3.14159*2)/resolution)*b.radius) * inverse_aspect_ratio * 0.5 + 0.5) * WIDTH,
				((b_position_relative.y + sin(angle + (3.14159*2)/resolution)*b.radius) * 0.5 + 0.5) * HEIGHT);
	}
}

void draw_wall(SDL_Renderer *renderer, wall w, camera cam) {
	int WIDTH = cam.width;
	int HEIGHT = cam.height;
	vec3 camera_position = cam.position;
	vec3 camera_rotation = cam.rotation;

	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;

	vec3 vertex_a = v3_sub((vec3){w.position.x + w.normal.y * w.length/2, w.position.y + -w.normal.x * w.length/2, 0}, camera_position);
	vec3 vertex_b = v3_sub((vec3){w.position.x - w.normal.y * w.length/2, w.position.y - -w.normal.x * w.length/2, 0}, camera_position);

	vec3 a_relative = vertex_a;
	vec3 b_relative = vertex_b;

	a_relative.y *= -1;
	b_relative.y *= -1;

	mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));

	a_relative = m3_v3_mult(camera_rotation_matrix, a_relative);

	if (a_relative.z < 0) return;
	a_relative.x /= a_relative.z;
	a_relative.y /= a_relative.z;

	a_relative.x *= inverse_aspect_ratio;

	a_relative.x /= 2;
	a_relative.y /= 2;
	a_relative.x += 0.5;
	a_relative.y += 0.5;
	a_relative.x *= WIDTH;
	a_relative.y *= HEIGHT;

	b_relative = m3_v3_mult(camera_rotation_matrix, b_relative);

	if (b_relative.z < 0) return;
	b_relative.x /= b_relative.z;
	b_relative.y /= b_relative.z;

	b_relative.x *= inverse_aspect_ratio;

	b_relative.x /= 2;
	b_relative.y /= 2;
	b_relative.x += 0.5;
	b_relative.y += 0.5;
	b_relative.x *= WIDTH;
	b_relative.y *= HEIGHT;

	SDL_RenderLine(renderer, a_relative.x, a_relative.y, b_relative.x, b_relative.y);
}

void draw_wall_3d(SDL_Renderer *renderer, wall_3d w, camera cam) {
	int WIDTH = cam.width;
	int HEIGHT = cam.height;
	vec3 camera_position = cam.position;
	vec3 camera_rotation = cam.rotation;

	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;

	vec3 a_relative = v3_sub(w.vertex_a, camera_position);
	vec3 b_relative = v3_sub(w.vertex_b, camera_position);
	vec3 c_relative = v3_sub(w.vertex_c, camera_position);

	a_relative.y *= -1;
	b_relative.y *= -1;
	c_relative.y *= -1;

	mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));

	a_relative = m3_v3_mult(camera_rotation_matrix, a_relative);

	if (a_relative.z < 0) return;
	a_relative.x /= a_relative.z;
	a_relative.y /= a_relative.z;

	a_relative.x *= inverse_aspect_ratio;

	a_relative.x /= 2;
	a_relative.y /= 2;
	a_relative.x += 0.5;
	a_relative.y += 0.5;
	a_relative.x *= WIDTH;
	a_relative.y *= HEIGHT;

	b_relative = m3_v3_mult(camera_rotation_matrix, b_relative);

	if (b_relative.z < 0) return;
	b_relative.x /= b_relative.z;
	b_relative.y /= b_relative.z;

	b_relative.x *= inverse_aspect_ratio;

	b_relative.x /= 2;
	b_relative.y /= 2;
	b_relative.x += 0.5;
	b_relative.y += 0.5;
	b_relative.x *= WIDTH;
	b_relative.y *= HEIGHT;

	c_relative = m3_v3_mult(camera_rotation_matrix, c_relative);

	if (c_relative.z < 0) return;
	c_relative.x /= c_relative.z;
	c_relative.y /= c_relative.z;

	c_relative.x *= inverse_aspect_ratio;

	c_relative.x /= 2;
	c_relative.y /= 2;
	c_relative.x += 0.5;
	c_relative.y += 0.5;
	c_relative.x *= WIDTH;
	c_relative.y *= HEIGHT;

	SDL_RenderLine(renderer, a_relative.x, a_relative.y, b_relative.x, b_relative.y);
	SDL_RenderLine(renderer, b_relative.x, b_relative.y, c_relative.x, c_relative.y);
	SDL_RenderLine(renderer, c_relative.x, c_relative.y, a_relative.x, a_relative.y);
}

void draw_linkage(SDL_Renderer *renderer, linkage link, camera cam) {
	int WIDTH = cam.width;
	int HEIGHT = cam.height;
	vec3 camera_position = cam.position;
	vec3 camera_rotation = cam.rotation;

	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;

	vec3 first = v3_sub((vec3){link.a->position.x, link.a->position.y, 0}, camera_position);
	vec3 last = v3_sub((vec3){link.b->position.x, link.b->position.y, 0}, camera_position);
	first.y *=-1;
	last.y *=-1;
	mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));
	first = m3_v3_mult(camera_rotation_matrix, first);

	first.x *= inverse_aspect_ratio;

	if (first.z < 0) return;
	first.x /= first.z;
	first.y /= first.z;

	first.x /= 2;
	first.y /= 2;
	first.x += 0.5;
	first.y += 0.5;
	first.x *= WIDTH;
	first.y *= HEIGHT;

	last = m3_v3_mult(camera_rotation_matrix, last);
	
	last.x *= inverse_aspect_ratio;

	if (last.z < 0) return;
	last.x /= last.z;
	last.y /= last.z;

	last.x /= 2;
	last.y /= 2;
	last.x += 0.5;
	last.y += 0.5;
	last.x *= WIDTH;
	last.y *= HEIGHT;

	SDL_RenderLine(renderer, first.x, first.y, last.x, last.y);
}

void draw_linkage_3d(SDL_Renderer *renderer, linkage_3d link, camera cam) {
	int WIDTH = cam.width;
	int HEIGHT = cam.height;
	vec3 camera_position = cam.position;
	vec3 camera_rotation = cam.rotation;

	float inverse_aspect_ratio = (float)HEIGHT/WIDTH;

	vec3 first = v3_sub(link.a->position, camera_position);
	vec3 last = v3_sub(link.b->position, camera_position);
	first.y *=-1;
	last.y *=-1;
	mat3 camera_rotation_matrix = transpose(generate_rotation_matrix(camera_rotation.x, camera_rotation.y, camera_rotation.z));

	first = m3_v3_mult(camera_rotation_matrix, first);

	first.x *= inverse_aspect_ratio;

	if (first.z < 0) return;
	first.x /= first.z;
	first.y /= first.z;

	first.x /= 2;
	first.y /= 2;
	first.x += 0.5;
	first.y += 0.5;
	first.x *= WIDTH;
	first.y *= HEIGHT;

	last = m3_v3_mult(camera_rotation_matrix, last);
	
	last.x *= inverse_aspect_ratio;

	if (last.z < 0) return;
	last.x /= last.z;
	last.y /= last.z;

	last.x /= 2;
	last.y /= 2;
	last.x += 0.5;
	last.y += 0.5;
	last.x *= WIDTH;
	last.y *= HEIGHT;

	SDL_RenderLine(renderer, first.x, first.y, last.x, last.y);
}

void set_wall_normal(wall_3d *w) {
	vec3 a_b_edge = v3_sub(w->vertex_a, w->vertex_b);
	vec3 b_c_edge = v3_sub(w->vertex_b, w->vertex_c);
	w->normal = v3_normalize(v3_cross(b_c_edge, a_b_edge));
}
