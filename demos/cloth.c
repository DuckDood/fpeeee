#include <SDL3/SDL_video.h>
#ifndef __EMSCRIPTEN__
#include <glad/gl.h>
#else
#include <GLES3/gl3.h>
#endif
#include <SDL3/SDL_oldnames.h>
#include <math.h>
#include <sys/types.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <physics.h>
#include <matrix.h>
#include <shape_generators.h>
#include <stdio.h>
#include <stdlib.h>

#include <helpers.h>

#include <spatial.h>

int fps = 0;
int ball_count = 0;

char optionstr[256];

char *get_option() {
	snprintf((char*)optionstr, sizeof(optionstr), "Controls:\nEnter to spawn balls\nWASD to move\nArrow keys to rotate camera (sorry)\n\nFPS: %i, Number of balls: %i", fps, ball_count);
	return optionstr;
}

#define CLOTH_DIMENSIONS 25

#define WIDTH 1280
#define HEIGHT 720

typedef struct {
	SDL_Window *window;
	SDL_GLContext gl_context;

	camera cam;

	float deltatime;

	ball_3d *balls;
	int ball_count;

	spatial_grid grid;

	Uint64 frame_tick_count;
	int frame_count;

	Uint64 spawn_tick_count;

	shape_3d cloth;

	GLuint VBO;
	GLuint VAO;

	GLuint instance_VBO;
	int instance_max;

	GLuint shader_program;

	GLuint cloth_VBO;
	GLuint cloth_normals_VBO;
	GLuint cloth_texcoords_VBO;
	GLuint cloth_VAO;
	GLuint cloth_EBO;

	GLuint cloth_texture;

	GLuint cloth_shader_program;
} prog_state;

SDL_AppResult SDL_AppInit(void **appstate, [[maybe_unused]] int argc, [[maybe_unused]] char **argv) {
	*appstate = malloc(sizeof(prog_state));
	prog_state *state = *appstate;

	if(!SDL_Init(SDL_INIT_VIDEO)) {
		printf("Failed to initialize SDL3: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	state->window = SDL_CreateWindow("physics", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if(!state->window) {
		printf("Failed to create window: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	state->gl_context = SDL_GL_CreateContext(state->window);

#ifndef __EMSCRIPTEN__
	if(gladLoadGL(SDL_GL_GetProcAddress) == 0) {
		printf("Failed to load OpenGL callbacks.\n");
		return SDL_APP_FAILURE;
	}
#endif
	SDL_GL_SetSwapInterval(1);


	state->cam.position = (vec3){0, 1, -6};
	state->cam.rotation = (vec3){0, 0, 0};

	state->cam.width = WIDTH;
	state->cam.height = HEIGHT;

	state->deltatime = 0;

	state->ball_count = 0;
	state->balls = malloc(state->ball_count * sizeof(ball_3d));
	printf("ball mem usage (kb): %zu\n", state->ball_count * sizeof(ball_3d) / 1000);
	state->grid = construct_grid_3d(25, 50, 25, 0.3);


	for(int i = 0; i < state->ball_count; ++i) {
		state->balls[i].position = (vec3){0, i * 0.5 + 5, 0};
		set_velocity_3d(state->balls + i, (vec3){0});
		state->balls[i].mass = 1;
		state->balls[i].radius = 0.5;
	}

	state->cloth = generate_cloth_3d(5, 5, CLOTH_DIMENSIONS, CLOTH_DIMENSIONS, 100, (vec3){0});

	state->frame_tick_count = SDL_GetTicks();
	state->frame_count = 0;


	glEnable(GL_DEPTH_TEST);
#define CIRCLE_SIDE_COUNT 20
	GLfloat vertices[CIRCLE_SIDE_COUNT * CIRCLE_SIDE_COUNT * 3 * 3 * 2];
	/*for(int i = 0; i < CIRCLE_SIDE_COUNT; ++i) {
		float angle1 = (float)i / CIRCLE_SIDE_COUNT * 3.14159 * 2;
		float angle2 = (float)(i+1) / CIRCLE_SIDE_COUNT * 3.14159 * 2;
		vertices[i*9 + 0] = 0;
		vertices[i*9 + 1] = 0;
		vertices[i*9 + 2] = 0;

		vertices[i*9 + 3] = cos(angle1);
		vertices[i*9 + 4] = sin(angle1);
		vertices[i*9 + 5] = 0;

		vertices[i*9 + 6] = cos(angle2);
		vertices[i*9 + 7] = sin(angle2);
		vertices[i*9 + 8] = 0;


	} */
	for(int height = 0; height < CIRCLE_SIDE_COUNT; ++height) {
		float height_angle1 = (float)height / CIRCLE_SIDE_COUNT * 3.14159 * 2;
		float height_angle2 = (float)(height+1) / CIRCLE_SIDE_COUNT * 3.14159 * 2;
			for(int i = 0; i < CIRCLE_SIDE_COUNT; ++i) {
			float angle1 = (float)i / CIRCLE_SIDE_COUNT * 3.14159 * 2;
			float angle2 = (float)(i+1) / CIRCLE_SIDE_COUNT * 3.14159 * 2;

			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 0] = cos(angle1) * sin(height_angle1 / 2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 1] = (float)cos(height_angle1 /2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 2] = sin(angle1) * sin(height_angle1 / 2);

			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 3] = cos(angle1) * sin(height_angle2 / 2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 4] = (float)cos(height_angle2 /2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 5] = sin(angle1) * sin(height_angle2 / 2);

			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 6] = cos(angle2) * sin(height_angle1 / 2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 7] = (float)cos(height_angle1 /2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 8] = sin(angle2) * sin(height_angle1 / 2);

			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 9] = cos(angle2) * sin(height_angle1 / 2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 10] = (float)cos(height_angle1 /2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 11] = sin(angle2) * sin(height_angle1 / 2);

			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 12] = cos(angle1) * sin(height_angle2 / 2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 13] = (float)cos(height_angle2 /2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 14] = sin(angle1) * sin(height_angle2 / 2);

			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 15] = cos(angle2) * sin(height_angle2 / 2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 16] = (float)cos(height_angle2 /2);
			vertices[height * CIRCLE_SIDE_COUNT * 18 + i*18 + 17] = sin(angle2) * sin(height_angle2 / 2);
		}
	}

	
	const char *vs_source = "#version 300 es\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 ballPosition;\n"
    "layout (location = 2) in float ballRadius;\n"
    "layout (location = 3) in vec3 prevPos;\n"
	"uniform float aspectRatio;\n"
	"uniform mat3 camRot;\n"
	"uniform vec3 camPos;\n"
	"out vec3 velocity;\n"
	"out vec3 normal;\n"
    "void main()\n"
    "{\n"
	"	vec3 pos = camRot * (vec3((aPos.x * ballRadius * 0.99 + ballPosition.x), aPos.y * ballRadius * 0.99+ ballPosition.y, aPos.z * ballRadius * 0.99 + ballPosition.z) - camPos);\n"
    "   gl_Position = vec4(pos.x * aspectRatio, pos.y, pos.z*pos.z * 1./100., pos.z);\n" // 1./100. so the far clipping plane doesnt come too quick
    "	velocity = ballPosition - prevPos;\n"
	"	normal = normalize(aPos);\n"
    "}\0";

	GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(v_shader, 1, &vs_source, NULL);
	glCompileShader(v_shader);
	int success;
	char infolog[512];
	glGetShaderiv(v_shader, GL_COMPILE_STATUS, &success);

	if(!success) {
		glGetShaderInfoLog(v_shader, 512, NULL, infolog);
		printf("Vertex shader compilation error: %s\n", infolog);
		return SDL_APP_FAILURE;
	}

	const char *fs_source = "#version 300 es\nprecision mediump float;\n"
	"out vec4 FragColor;\n"
	"\n"
	"in vec3 velocity;\n"
	"in vec3 normal;\n"
	"void main()\n"
	"{\n"
		"\nvec3 col = mix(vec3(0., 0., 1.), vec3(1., 0.4, 0.1), length(velocity) * 100.);\n"
		"if(col.r > 1.) col.r = 1.;\n"
		"if(col.g > 1.) col.g = 1.;\n"
		"if(col.b > 1.) col.b = 1.;\n"

		"if(col.r < 0.) col.r = 0.;\n"
		"if(col.g < 0.) col.g = 0.;\n"
		"if(col.b < 0.) col.b = 0.;\n"
		"col *= 0.5*(1. + dot(normal, vec3(0., 1., 0.)));\n"
	    "FragColor = vec4(col, 1.0f);\n"
	"}\0";

	GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(f_shader, 1, &fs_source, NULL);
	glCompileShader(f_shader);
	glGetShaderiv(f_shader, GL_COMPILE_STATUS, &success);

	if(!success) {
		glGetShaderInfoLog(f_shader, 512, NULL, infolog);
		printf("Fragment shader compilation error: %s\n", infolog);
		return SDL_APP_FAILURE;
	}

	state->shader_program = glCreateProgram();
	glAttachShader(state->shader_program, v_shader);
	glAttachShader(state->shader_program, f_shader);
	glLinkProgram(state->shader_program);

	glGetProgramiv(state->shader_program, GL_LINK_STATUS, &success);

	if(!success) {
		glGetProgramInfoLog(state->shader_program, 512, NULL, infolog);
		printf("Shader program link error: %s\n", infolog);
		return SDL_APP_FAILURE;
	}

	glDeleteShader(v_shader);
	glDeleteShader(f_shader);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &state->cloth_texture);
	glBindTexture(GL_TEXTURE_2D, state->cloth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// fabric texture from ambientCG
	int image_size = sizeof((unsigned char[]){
		#embed "../resources/fabric.ppm"
				});
	unsigned char *fabric_texture = malloc(image_size+1); // you just need the +1 for some reason

		/*= {
		#embed "../resources/fabric.ppm"
	};*/
	memcpy(fabric_texture, (unsigned char[]){
		#embed "../resources/fabric.ppm"
			}, image_size);
	strtok((char*)fabric_texture, "\n");
	char *width = strtok(NULL, " ");
	char *height = strtok(NULL, "\n");
	float tex_width = atoi(width);
	float tex_height = atoi(height);
	strtok(NULL, "\n");

	unsigned char *fabric_tex_bin = (unsigned char*)strtok(NULL, "\n");


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)fabric_tex_bin);

	free(fabric_texture);

	glGenerateMipmap(GL_TEXTURE_2D);

	glGenVertexArrays(1, &state->VAO);
	glBindVertexArray(state->VAO);

	glGenBuffers(1, &state->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, state->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	state->instance_max = 1002;
	glGenBuffers(1, &state->instance_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, state->instance_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ball_3d) * state->instance_max, NULL, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ball_3d), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ball_3d), (void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 1);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ball_3d), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);



	glGenVertexArrays(1, &state->cloth_VAO);
	glBindVertexArray(state->cloth_VAO);

	glGenBuffers(1, &state->cloth_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, state->cloth_VBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(ball_3d) * state->cloth.ball_count, state->cloth.balls, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ball_3d), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &state->cloth_normals_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, state->cloth_normals_VBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * state->cloth.ball_count, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &state->cloth_texcoords_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, state->cloth_texcoords_VBO);
	
	vec2 texcoords[CLOTH_DIMENSIONS * CLOTH_DIMENSIONS];
	for(int row = 0; row < CLOTH_DIMENSIONS; ++row) {
		for(int i = 0; i < CLOTH_DIMENSIONS; ++i) {
			texcoords[row * CLOTH_DIMENSIONS + i] = (vec2){(float)i / CLOTH_DIMENSIONS, (float)row / CLOTH_DIMENSIONS};
		}
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &state->cloth_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->cloth_EBO);
	GLuint indices[(CLOTH_DIMENSIONS-1)*(CLOTH_DIMENSIONS-1) * 6];
	int indices_top = 0;
	for(int row = 0; row < CLOTH_DIMENSIONS-1; ++row) {
		for(int i = 0; i < CLOTH_DIMENSIONS-1; ++i) {
			indices[indices_top++] = i+1 + CLOTH_DIMENSIONS*row;
			indices[indices_top++] = i + CLOTH_DIMENSIONS*row;
			indices[indices_top++] = i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row;

			indices[indices_top++] = i+1 + CLOTH_DIMENSIONS*row;
			indices[indices_top++] = i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row;
			indices[indices_top++] = i+CLOTH_DIMENSIONS+1 + CLOTH_DIMENSIONS*row;
		}
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);







	const char *cloth_vs_source = "#version 300 es\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 normal;\n"
    "layout (location = 2) in vec2 texcoords;\n"
	"uniform float aspectRatio;\n"
	"uniform mat3 camRot;\n"
	"uniform vec3 camPos;\n"
	//"out vec3 velocity;\n"
	"out vec3 position;\n"
	"out vec3 normals;\n"
	"out vec2 texcoord;\n"
    "void main()\n"
    "{\n"
	"	vec3 pos = camRot * (vec3((aPos.x), aPos.y, aPos.z) - camPos);\n"
    "   gl_Position = vec4(pos.x * aspectRatio, pos.y, pos.z*pos.z * 1./100., pos.z);\n" // 1./100. so the far clipping plane doesnt come too quick
    //"	velocity = ballPosition - prevPos;\n"
	"	position = pos;\n"
	"	normals = normal;\n"
	"	texcoord = texcoords;\n"
    "}\0";

	v_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(v_shader, 1, &cloth_vs_source, NULL);
	glCompileShader(v_shader);
	glGetShaderiv(v_shader, GL_COMPILE_STATUS, &success);

	if(!success) {
		glGetShaderInfoLog(v_shader, 512, NULL, infolog);
		printf("Cloth vertex shader compilation error: %s\n", infolog);
		return SDL_APP_FAILURE;
	}

	const char *cloth_fs_source = "#version 300 es\nprecision mediump float;\n"
	"out vec4 FragColor;\n"
	"\n"
	//"in vec3 velocity;\n"
	"in vec3 position;\n"
	"in vec3 normals;\n"
	"in vec2 texcoord;\n"
	"uniform sampler2D cloth_texture;\n"
	"void main()\n"
	"{\n"
		//"vec3 velocity = vec3(0.);\n"
		//"\nvec3 col = mix(vec3(0., 0., 1.), vec3(1., 0.4, 0.1), length(velocity) * 100.);\n"
		"\nvec3 col = vec3(texture(cloth_texture, texcoord));\n"
		"if(col.r > 1.) col.r = 1.;\n"
		"if(col.g > 1.) col.g = 1.;\n"
		"if(col.b > 1.) col.b = 1.;\n"

		"if(col.r < 0.) col.r = 0.;\n"
		"if(col.g < 0.) col.g = 0.;\n"
		"if(col.b < 0.) col.b = 0.;\n"
		"vec3 X = dFdx ( position );"
		"vec3 Y = dFdy ( position );"
		"vec3 normal = normalize ( -cross ( X, Y ) );"
		"col *= max(0.5 + 0.5 * dot(normals * ((float(gl_FrontFacing) - 0.5) * 2.), vec3(0., 1., 0.)), 0.3);\n" 
		"col *= max(0.5 + 0.5 * dot(normals * ((float(gl_FrontFacing) - 0.5) * 2.), vec3(0., 1., 0.)), 0.3);\n"
		//"col += vec3(0, 0, 0.1);\n"
	    "FragColor = vec4(col, 1.0f);\n"
		//"FragColor = texture(cloth_texture, texcoord);\n"
	"}\0";

	f_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(f_shader, 1, &cloth_fs_source, NULL);
	glCompileShader(f_shader);
	glGetShaderiv(f_shader, GL_COMPILE_STATUS, &success);

	if(!success) {
		glGetShaderInfoLog(f_shader, 512, NULL, infolog);
		printf("Cloth fragment shader compilation error: %s\n", infolog);
		return SDL_APP_FAILURE;
	}

	state->cloth_shader_program = glCreateProgram();
	glAttachShader(state->cloth_shader_program, v_shader);
	glAttachShader(state->cloth_shader_program, f_shader);
	glLinkProgram(state->cloth_shader_program);

	glGetProgramiv(state->cloth_shader_program, GL_LINK_STATUS, &success);

	if(!success) {
		glGetProgramInfoLog(state->cloth_shader_program, 512, NULL, infolog);
		printf("Cloth shader program link error: %s\n", infolog);
		return SDL_APP_FAILURE;
	}

	glDeleteShader(v_shader);
	glDeleteShader(f_shader);





	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, [[maybe_unused]] SDL_AppResult result) {
	prog_state *state = appstate;
	destroy_grid_3d(&state->grid);
	free_shape_3d(state->cloth);
	free(state->balls);

	SDL_GL_DestroyContext(state->gl_context);
	SDL_DestroyWindow(state->window);
	free(appstate);
	SDL_Quit();
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	prog_state *state = appstate;
	switch(event->type) {
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
			break;

		case SDL_EVENT_WINDOW_RESIZED:
			state->cam.width = event->window.data1;
			state->cam.height = event->window.data2;
			glViewport(0, 0, event->window.data1, event->window.data2);
			break;
		default: 
			break;
	}
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	prog_state *state = appstate;

	Uint64 start_time = SDL_GetTicks();

	const bool * const key_states = SDL_GetKeyboardState(NULL);

	int framerate = 60;
	int steps_per_frame = 10;
	state->deltatime = 1.0/framerate/steps_per_frame;
	//state->deltatime *= 0.5;
	if(key_states[SDL_SCANCODE_RETURN]) {
		if(SDL_GetTicks() > state->spawn_tick_count + 500) {
			int spawn_count = 1;
			float ball_radius = 0.3;
			float spawn_height = 5;
			state->spawn_tick_count = SDL_GetTicks();
			state->balls = realloc(state->balls, (state->ball_count += spawn_count) * sizeof(ball_3d));

			for(int i = 1; i <= spawn_count; ++i) {
				state->balls[state->ball_count-i].position = (vec3){0, spawn_height + i * 2 * ball_radius, 0};
				set_velocity_3d(state->balls + state->ball_count - i, (vec3){0, 0, 0});
				state->balls[state->ball_count - i].mass = 0.5;
				state->balls[state->ball_count - i].radius = ball_radius;
			}
		}
	}

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float aspect_ratio = (float)state->cam.width / state->cam.height;


	mat3 transform;
	transform = generate_rotation_matrix(0, 0, 0);
	transform = m3_mult(transform, generate_scale_matrix((vec3){1, 4, 1}));

	/*
	if(!key_states[SDL_SCANCODE_R]) {
		for(int i = 0; i < state->cloth.ball_count; ++i) {
			state->cloth.balls[i].previous_position = state->cloth.balls[i].position;
		}
	}*/

	if(key_states[SDL_SCANCODE_LEFT]) {
		state->cam.rotation.x-=0.03;
	}
	if(key_states[SDL_SCANCODE_RIGHT]) {
		state->cam.rotation.x+=0.03;
	}
	if(key_states[SDL_SCANCODE_UP]) {
		state->cam.rotation.y-=0.03;
	}
	if(key_states[SDL_SCANCODE_DOWN]) {
		state->cam.rotation.y+=0.03;
	}
	if(key_states[SDL_SCANCODE_W]) {
		state->cam.position.z += 0.2 * cos(state->cam.rotation.x) * cos(state->cam.rotation.y);
		state->cam.position.x += 0.2 * sin(state->cam.rotation.x) * cos(state->cam.rotation.y);

		state->cam.position.y -= 0.2 * sin(state->cam.rotation.y);
	}
	if(key_states[SDL_SCANCODE_S]) {
		state->cam.position.z -= 0.2 * cos(state->cam.rotation.x) * cos(state->cam.rotation.y);
		state->cam.position.x -= 0.2 * sin(state->cam.rotation.x) * cos(state->cam.rotation.y);

		state->cam.position.y += 0.2 * sin(state->cam.rotation.y);
	}
	if(key_states[SDL_SCANCODE_D]) {
		state->cam.position.z -= 0.2 * sin(state->cam.rotation.x);
		state->cam.position.x += 0.2 * cos(state->cam.rotation.x);
	}
	if(key_states[SDL_SCANCODE_A]) {
		state->cam.position.z += 0.2 * sin(state->cam.rotation.x);
		state->cam.position.x -= 0.2 * cos(state->cam.rotation.x);
	}

	for(int steps = 0; steps < steps_per_frame; ++steps) {

		for(int i = 0; i < state->cloth.ball_count; ++i) {
			ball_3d *current_ball = state->cloth.balls + i;
			update_ball_3d(current_ball);
			current_ball->position.y -= 10 * state->deltatime * state->deltatime;
		}

		for(int i = 0; i < state->ball_count; ++i) {
			ball_3d *current_ball = state->balls + i;
			update_ball_3d(current_ball);
			current_ball->position.y -= 10 * state->deltatime * state->deltatime;
		}

		state->cloth.balls[0].position = (vec3){-2.5, 0, 2.5};
		state->cloth.balls[CLOTH_DIMENSIONS-1].position = (vec3){2.5, 0, 2.5};
        
		state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS].position = (vec3){-2.5, 0, -2.5};
		state->cloth.balls[state->cloth.ball_count-1].position = (vec3){2.5, 0, -2.5};
        
		for(int i = 0; i < state->cloth.link_count; ++i) {
				update_linkage_3d(state->cloth.links[i], state->deltatime);
		}
        
		state->cloth.balls[0].position = (vec3){-2.5, 0, 2.5};
		state->cloth.balls[CLOTH_DIMENSIONS-1].position = (vec3){2.5, 0, 2.5};
        
		state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS].position = (vec3){-2.5, 0, -2.5};
		state->cloth.balls[state->cloth.ball_count-1].position = (vec3){2.5, 0, -2.5};
		
		
		
		update_grid_3d(&state->grid, state->balls, state->ball_count);
		spatial_collision_3d(&state->grid, state->balls, state->ball_count);
	
		//collide_wall_3d(&state->cloth.balls[0], &state->cloth.balls[CLOTH_DIMENSIONS-1], &state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS], state->balls + 4);
		//collide_wall_3d(&state->cloth.balls[state->cloth.ball_count-1], &state->cloth.balls[CLOTH_DIMENSIONS-1], &state->cloth.balls[state->cloth.ball_count-CLOTH_DIMENSIONS], state->balls + 4);
		
	
		for(int ball = 0; ball < state->ball_count; ++ball) {
			for(int row = 0; row < CLOTH_DIMENSIONS-1; ++row) {
				for(int i = 0; i < CLOTH_DIMENSIONS-1; ++i) {
					collide_wall_3d(
							&state->cloth.balls[i + CLOTH_DIMENSIONS*row],
							&state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row],
							&state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row],
							state->balls + ball);
					collide_wall_3d(
							&state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row],
							&state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row],
							&state->cloth.balls[i+CLOTH_DIMENSIONS+1 + CLOTH_DIMENSIONS*row],
							state->balls + ball);
				}
			}
		}
	}


	
	
	/*
	SDL_SetRenderDrawColor(state->renderer, 123, 0, 0, 255);
	for(int i = 0; i < state->grid.depth; ++i) {
		for(int j = 0; j < state->grid.height; ++j) {
			for(int k = 0; k < state->grid.width; ++k) {
				//if(state->grid.partitions[i * state->grid.width * state->grid.height + j * state->grid.width + k].ball_count == 0) continue;
				vec3 line_1_1 = (vec3){
					state->grid.element_size*0.5 + state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size* (j-state->grid.height*0.5),
					state->grid.element_size * (i-state->grid.depth*0.5)
				};
				vec3 line_1_2 = (vec3){
					-state->grid.element_size*0.5 + state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size * (j-state->grid.height*0.5),
					state->grid.element_size * (i-state->grid.depth*0.5)
				};
				line_1_1 = point_to_screen(line_1_1, state->cam);
				line_1_2 = point_to_screen(line_1_2, state->cam);
				if(line_1_1.z < 0 || line_1_2.z < 0) continue;

				SDL_RenderLine(state->renderer, line_1_1.x, line_1_1.y, line_1_2.x, line_1_2.y);

				vec3 line_2_1 = (vec3){
					state->grid.element_size * (k-state->grid.width * 0.5),
					-state->grid.element_size*0.5 + state->grid.element_size* (j-state->grid.height*0.5),
					state->grid.element_size * (i-state->grid.depth*0.5)
				};
				vec3 line_2_2 = (vec3){
					state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size*0.5 + state->grid.element_size * (j-state->grid.height*0.5),
					state->grid.element_size * (i-state->grid.depth*0.5)
				};
				line_2_1 = point_to_screen(line_2_1, state->cam);
				line_2_2 = point_to_screen(line_2_2, state->cam);
				if(line_2_1.z < 0 || line_2_2.z < 0) continue;

				SDL_RenderLine(state->renderer, line_2_1.x, line_2_1.y, line_2_2.x, line_2_2.y);

				vec3 line_3_1 = (vec3){
					state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size* (j-state->grid.height*0.5),
					-state->grid.element_size * 0.5 + state->grid.element_size * (i-state->grid.depth*0.5)
				};
				vec3 line_3_2 = (vec3){
					state->grid.element_size * (k-state->grid.width * 0.5),
					state->grid.element_size* (j-state->grid.height*0.5),
					state->grid.element_size * 0.5 + state->grid.element_size * (i-state->grid.depth*0.5)
				};
				line_3_1 = point_to_screen(line_3_1, state->cam);
				line_3_2 = point_to_screen(line_3_2, state->cam);
				if(line_3_1.z < 0 || line_3_2.z < 0) continue;

				SDL_RenderLine(state->renderer, line_3_1.x, line_3_1.y, line_3_2.x, line_3_2.y);
			}
		}
	}*/

	/*draw_wall_3d(state->renderer, (wall_3d) {
		.vertex_a = state->balls[0].position,
		.vertex_b = state->balls[1].position,
		.vertex_c = state->balls[2].position,
	}, state->cam);

	draw_wall_3d(state->renderer, (wall_3d) {
		.vertex_a = state->balls[3].position,
		.vertex_b = state->balls[1].position,
		.vertex_c = state->balls[2].position,
	}, state->cam);*/

	
	/*for(int i = 0; i < state->cloth.ball_count; ++i) {
		draw_circle_3d(state->renderer, state->cloth.balls[i], 25, state->cam);
	}
	for(int i = 0; i < state->cloth.link_count; ++i) {
		draw_linkage_3d(state->renderer, state->cloth.links[i], state->cam);
	}*/

	/*for(int row = 0; row < CLOTH_DIMENSIONS-1; ++row) {
		for(int i = 0; i < CLOTH_DIMENSIONS-1; ++i) {
			draw_wall_3d(state->renderer, (wall_3d) {
				.vertex_a = state->cloth.balls[i + CLOTH_DIMENSIONS*row].position,
				.vertex_b = state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position,
				.vertex_c = state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position,
			}, state->cam);
			draw_wall_3d(state->renderer, (wall_3d) {
				.vertex_a =	state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position,
				.vertex_b =	state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position,
				.vertex_c =	state->cloth.balls[i+CLOTH_DIMENSIONS+1 + CLOTH_DIMENSIONS*row].position,
			}, state->cam);
		}
	}*/

	/*SDL_SetRenderDrawColor(state->renderer, 0, 0, 255, 255);
	for(int row = 0; row < CLOTH_DIMENSIONS-1; ++row) {
		for(int i = 0; i < CLOTH_DIMENSIONS-1; ++i) {
			
			draw_wall_3d(state->renderer, (wall_3d){
					.vertex_a = state->cloth.balls[i + CLOTH_DIMENSIONS*row].position,
					.vertex_b = state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position,
					.vertex_c = state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position}, state->cam
					);
			draw_wall_3d(state->renderer, (wall_3d){
					.vertex_a = state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position,
					.vertex_b = state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position,
					.vertex_c = state->cloth.balls[i+CLOTH_DIMENSIONS+1 + CLOTH_DIMENSIONS*row].position,
					}, state->cam
					);
		}
	}
	SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 255);
	for(int i = 0; i < state->ball_count; ++i) {
		draw_circle_3d(state->renderer, state->balls[i], 25, state->cam);
	}

	SDL_RenderPresent(state->renderer);*/
	vec3 normals[CLOTH_DIMENSIONS * CLOTH_DIMENSIONS];
	memset(normals, 0, sizeof(normals));
	for(int row = 0; row < CLOTH_DIMENSIONS-1; ++row) {
		for(int i = 0; i < CLOTH_DIMENSIONS-1; ++i) {
			/*vec3 side_AB = v3_sub(state->cloth.balls[i + CLOTH_DIMENSIONS*row].position, state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position);
			vec3 side_AC = v3_sub(state->cloth.balls[i + CLOTH_DIMENSIONS*row].position, state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position);
			vec3 tri_normal = v3_normalize(v3_cross(side_AB, side_AC));
			normals[row * CLOTH_DIMENSIONS + i] = v3_add(normals[row * CLOTH_DIMENSIONS + i], tri_normal);
			normals[row * CLOTH_DIMENSIONS + i + 1] = v3_add(normals[row * CLOTH_DIMENSIONS + i+1], tri_normal);
			normals[row * CLOTH_DIMENSIONS + i + CLOTH_DIMENSIONS] = v3_add(normals[row * CLOTH_DIMENSIONS + i+CLOTH_DIMENSIONS], tri_normal);


			side_AB = v3_sub(state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position, state->cloth.balls[i+CLOTH_DIMENSIONS + CLOTH_DIMENSIONS*row].position);
			side_AC = v3_sub(state->cloth.balls[i+1 + CLOTH_DIMENSIONS*row].position, state->cloth.balls[i+CLOTH_DIMENSIONS+1 + CLOTH_DIMENSIONS*row].position);
			tri_normal = v3_normalize(v3_cross(side_AB, side_AC));
			normals[row * CLOTH_DIMENSIONS + i+1] = v3_add(normals[row * CLOTH_DIMENSIONS + i+1], tri_normal);
			normals[row * CLOTH_DIMENSIONS + i + CLOTH_DIMENSIONS] = v3_add(normals[row * CLOTH_DIMENSIONS + i+CLOTH_DIMENSIONS], tri_normal);
			normals[row * CLOTH_DIMENSIONS + i + CLOTH_DIMENSIONS+1] = v3_add(normals[row * CLOTH_DIMENSIONS + i+CLOTH_DIMENSIONS+1], tri_normal);*/ // didnt work
			vec3 position = state->cloth.balls[row * CLOTH_DIMENSIONS + i].position;
			vec3 next_right_position = state->cloth.balls[row * CLOTH_DIMENSIONS + i + 1].position;
			vec3 next_down_position = state->cloth.balls[row * CLOTH_DIMENSIONS + i + CLOTH_DIMENSIONS].position;

			vec3 side_AB = v3_sub(position, next_right_position);
			vec3 side_AC = v3_sub(position, next_down_position);
			vec3 normal = v3_normalize(v3_cross(side_AB, side_AC));

			normals[row * CLOTH_DIMENSIONS + i] = v3_add(normals[row * CLOTH_DIMENSIONS + i], normal);
			normals[row * CLOTH_DIMENSIONS + i + 1] = v3_add(normals[row * CLOTH_DIMENSIONS + i + 1], normal);
			normals[row * CLOTH_DIMENSIONS + i + CLOTH_DIMENSIONS] = v3_add(normals[row * CLOTH_DIMENSIONS + i + CLOTH_DIMENSIONS], normal);

		}
	}
	for(int i = 0; i < CLOTH_DIMENSIONS*CLOTH_DIMENSIONS; ++i) {
		normals[i] = v3_normalize(normals[i]);
	}
	glBindBuffer(GL_ARRAY_BUFFER, state->cloth_normals_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(normals), normals);

	glBindBuffer(GL_ARRAY_BUFFER, state->cloth_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, state->cloth.ball_count * sizeof(ball_3d), state->cloth.balls);

	glBindBuffer(GL_ARRAY_BUFFER, state->instance_VBO);
	if(state->ball_count > state->instance_max) {
		state->instance_max = state->ball_count * 2;
		glBufferData(GL_ARRAY_BUFFER, sizeof(ball_3d) * state->instance_max, NULL, GL_DYNAMIC_DRAW); // we need to subdata anyway because otherwise it will try and read past the end of balls array
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, state->ball_count * sizeof(ball_3d), state->balls);

	

	//SDL_RenderPresent(state->renderer);
	
	glUseProgram(state->shader_program);
	glUniform1f(glGetUniformLocation(state->shader_program, "aspectRatio"), 1/aspect_ratio);
	glUniformMatrix3fv(glGetUniformLocation(state->shader_program, "camRot"), 1, GL_TRUE, generate_rotation_matrix(state->cam.rotation.x, state->cam.rotation.y, state->cam.rotation.z).matrix);
	glUniform3f(glGetUniformLocation(state->shader_program, "camPos"), state->cam.position.x, state->cam.position.y, state->cam.position.z);
	glBindVertexArray(state->VAO);
	glDrawArraysInstanced(GL_TRIANGLES, 0, CIRCLE_SIDE_COUNT * CIRCLE_SIDE_COUNT * 3 * 2, state->ball_count);
	glUseProgram(state->cloth_shader_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state->cloth_texture);
	glUniform1i(glGetUniformLocation(state->cloth_shader_program, "cloth_texture"), 0);


	glUniform1f(glGetUniformLocation(state->cloth_shader_program, "aspectRatio"), 1/aspect_ratio);
	glUniformMatrix3fv(glGetUniformLocation(state->cloth_shader_program, "camRot"), 1, GL_TRUE, generate_rotation_matrix(state->cam.rotation.x, state->cam.rotation.y, state->cam.rotation.z).matrix);
	glUniform3f(glGetUniformLocation(state->cloth_shader_program, "camPos"), state->cam.position.x, state->cam.position.y, state->cam.position.z);
	glBindVertexArray(state->cloth_VAO);
	glDrawElements(GL_TRIANGLES, (CLOTH_DIMENSIONS-1) * (CLOTH_DIMENSIONS-1) * 6, GL_UNSIGNED_INT, 0);
	SDL_GL_SwapWindow(state->window);

	/*vec3 momenta = {0};
	for(int i = 0; i < state->ball_count; ++i) {
		momenta = v3_add(momenta, v3_fmult(v3_sub(state->balls[i].position, state->balls[i].previous_position), state->balls[i].mass));
	}
	printf("momenta: %f\n", v3_magnitude(momenta));*/
	//printf("balls on the walls p1: %f, %f\n", state->balls[3].position.x, state->balls[3].position.y);

	Uint64 time_taken = SDL_GetTicks() - start_time;
	if(time_taken > (Uint64)1000 / framerate) time_taken = 1000 / framerate;
	//SDL_Delay(1000 / framerate - time_taken);


	state->frame_count++;

	
	ball_count = state->ball_count;
	///*
	if(SDL_GetTicks() > state->frame_tick_count + 1000) {
		printf("framerate: %i\nball count: %i\n", state->frame_count, state->ball_count);
		/*if(state->frame_count < 60) {
			printf("wee woo wee woo\n");
		}*/
		fps = state->frame_count;

		state->frame_tick_count = SDL_GetTicks();
		state->frame_count = 0;

		//spatial_partition *bin_partition = state->grid.partitions + state->grid.width * state->grid.height * state->grid.depth;
		/*int push_back = 0;
		for(int i = 0; i < state->ball_count; ++i) {
			ball_3d *ball = state->balls + i;
			int ball_column = ball->position.x / state->grid.element_size + state->grid.width * 0.5;
			int ball_row = ball->position.y / state->grid.element_size + state->grid.height * 0.5;
			int ball_layer = ball->position.z / state->grid.element_size + state->grid.depth * 0.5;

			state->balls[i-push_back] = state->balls[i];

			if(ball_column < 0 || ball_column > state->grid.width - 1
				|| ball_row < 0 || ball_row > state->grid.height - 1
				|| ball_layer < 0 || ball_layer > state->grid.depth - 1)
				++push_back;
		}
		state->ball_count -= push_back;
		state->balls = realloc(state->balls, sizeof(ball_3d) * state->ball_count);*/
	}
	//*/



	return SDL_APP_CONTINUE;
}
