#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL.h>
#include <brender.h>

void max_resolution(int *width, int *height, int max) {
	if (width == NULL || height == NULL) {
		return;
	}

	if (*width > max || *height > max) {
		float aspect = *width / (float)*height;
		if (*width > *height) {
			*height = (int)(max / aspect);
			*width = max;
		} else if (*width < *height) {
			*width = (int)(max * aspect);
			*height = max;
		} else {
			*width = max;
			*height = max;
		}
	}
}

int main(int argc, char *argv[]) {
	static int const def_width = 640, def_height = 480;

	static int const br_color_fmt = BR_PMT_RGB_888;
	static int const br_depth_fmt = BR_PMT_DEPTH_16;
	static int const br_depth_match_fmt = BR_PMMATCH_DEPTH_16;
	static int const sdl_color_fmt = SDL_PIXELFORMAT_RGB24;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("sdl_init panic! (%s)\n", SDL_GetError());
		return -1;
	}

	SDL_Window *window = SDL_CreateWindow(
		"brenSDL", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		def_width, def_height, 
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);
	if (window == NULL) {
		printf("window panic! (%s)\n", SDL_GetError());
		return -1;
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("renderer panic! (%s)\n", SDL_GetError());
		return -1;
	}

	BrBegin();
	BrZbBegin(br_color_fmt, br_depth_fmt);

	br_pixelmap *color_buffer = BrPixelmapAllocate(br_color_fmt, def_width, def_height, NULL, BR_PMAF_NORMAL);
	br_pixelmap *depth_buffer = BrPixelmapMatch(color_buffer, br_depth_match_fmt);
	SDL_Texture *fb_texture = SDL_CreateTexture(renderer, sdl_color_fmt, SDL_TEXTUREACCESS_STREAMING, def_width, def_height);

	br_model *teapot_model = BrModelLoad("data/teapot.dat");
	if (teapot_model == NULL) {
		printf("unable to load teapot.dat as a model\n");
		return -1;
	}
	teapot_model = BrModelAdd(teapot_model);

	br_actor *world_root = BrActorAllocate(BR_ACTOR_NONE, NULL);

	br_actor *world_camera = BrActorAdd(world_root, BrActorAllocate(BR_ACTOR_CAMERA, NULL));
	br_camera *camera_data = world_camera->type_data;
	camera_data->type = BR_CAMERA_PERSPECTIVE;
	camera_data->field_of_view = BR_ANGLE_DEG(30.0);
	camera_data->hither_z = BR_SCALAR(0.2);
	camera_data->yon_z = BR_SCALAR(40.0);
	camera_data->aspect = BR_DIV((br_scalar)def_width, (br_scalar)def_height);
	BrMatrix34Translate(&world_camera->t.t.mat,BR_SCALAR(0.0),BR_SCALAR(0.0),BR_SCALAR(3.5));

	br_actor *world_light = BrActorAdd(world_root, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
	br_light *light_data = world_light->type_data;
	light_data->type = BR_LIGHT_DIRECT;
	light_data->attenuation_c = BR_SCALAR(1.0);
	BrMatrix34RotateY(&world_light->t.t.mat,BR_ANGLE_DEG(45.0));
	BrMatrix34PostRotateZ(&world_light->t.t.mat,BR_ANGLE_DEG(45.0));
	BrLightEnable(world_light);

	br_material *mat = BrMaterialAllocate(NULL); 
	*mat = (br_material) {
		.identifier = "white",

		.colour = BR_COLOUR_RGB(255, 255, 255),
		.opacity = 255,

		.ka = BR_UFRACTION(0.10),
		.kd = BR_UFRACTION(0.60),
		.ks = BR_UFRACTION(0.60),

		.power = BR_SCALAR(50),

		.flags = BR_MATF_LIGHT | BR_MATF_SMOOTH,

		.map_transform = {{ 
			BR_VECTOR2(1, 0), 
			BR_VECTOR2(0, 1), 
			BR_VECTOR2(0, 0) 
		}},
		
		.index_base = 0, 
		.index_range = 63,
	};
	mat = BrMaterialAdd(mat);

	br_actor *world_teapot = BrActorAdd(world_root, BrActorAllocate(BR_ACTOR_MODEL, NULL));
	world_teapot->model = teapot_model;
	world_teapot->material = mat;
	world_teapot->t.type = BR_TRANSFORM_EULER;
	world_teapot->t.t.euler.e.order = BR_EULER_ZXY_S;
	world_teapot->t.t.euler.e.a = BR_ANGLE_DEG(-45);
	world_teapot->t.t.euler.e.b = BR_ANGLE_DEG(-45);
	BrVector3Set(&world_teapot->t.t.euler.t, 
		BR_SCALAR(0.0), 
		BR_SCALAR(0.0), 
		BR_SCALAR(0.0)
	);

	uint64_t pre_pc = 0, post_pc = 0, delta_pc;
	bool is_running = true;
	while (is_running) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				is_running = false;
				break;
			case SDL_WINDOWEVENT:
				switch (ev.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					int width = ev.window.data1;
					int height = ev.window.data2;
					
					// NOTE(bjrkk): BRender maxes out at 2048x2048. Anything higher will crash
					// the software renderer, so let's account for it here.
					max_resolution(&width, &height, 2048);

					SDL_DestroyTexture(fb_texture);

					color_buffer = BrPixelmapAllocate(br_color_fmt, width, height, NULL, BR_PMAF_NORMAL);
					depth_buffer = BrPixelmapMatch(color_buffer, br_depth_match_fmt);
					fb_texture = SDL_CreateTexture(renderer, sdl_color_fmt, SDL_TEXTUREACCESS_STREAMING, width, height);

					br_camera *camera_data = world_camera->type_data;
					camera_data->aspect = BR_DIV((br_scalar)ev.window.data1, (br_scalar)ev.window.data2);
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		delta_pc = post_pc - pre_pc;
		pre_pc = SDL_GetPerformanceCounter();

		float delta_time = 0;
		delta_time = delta_pc / (float)SDL_GetPerformanceFrequency();

		char info[512];
		snprintf(info, sizeof(info), "fps: %.2f\n", 1.f / delta_time);

		BrPixelmapFill(color_buffer, 0x00000000);
		BrPixelmapFill(depth_buffer, 0xFFFFFFFF);

		BrZbSceneRender(world_root, world_camera, color_buffer, depth_buffer);
		BrPixelmapText(color_buffer, 0, 0, BR_COLOUR_RGB(255, 255, 255), BrFontProp7x9, info);

		SDL_UpdateTexture(fb_texture, NULL, color_buffer->pixels, color_buffer->row_bytes);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, fb_texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		post_pc = SDL_GetPerformanceCounter();

		world_teapot->t.t.euler.e.a += BR_ANGLE_DEG(delta_time * 100);
		world_teapot->t.t.euler.e.b -= BR_ANGLE_DEG(delta_time * 10);
	}

	BrPixelmapFree(color_buffer);
	BrPixelmapFree(depth_buffer);
	SDL_DestroyTexture(fb_texture);

	BrEnd();
	BrZbEnd();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}
