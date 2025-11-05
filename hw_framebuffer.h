#pragma once

#include "framebuffer.h"
#include "tm.h"

#include <chrono>
#include <FL/Fl_Box.H>

class HWFrameBuffer : public FrameBuffer {
public:
	PPC* ppc;
	TM* tms;
	int num_tms;

	bool* tm_is_mirror;

	bool use_lighting;
	bool has_textures;
	
	GLuint shadow_fb; //Framebuffer for shadow map
	GLuint shadow_map; //Depth texture
	int shadow_map_size;
	bool shadows_enabled;
	V3 light_pos;
	PPC* light_ppc;

	CubeMap* env_map;
	GLuint env_cube_map_id;
	bool environment_map_enabled;

	bool initialized;

	bool render_wireframe;

	std::chrono::steady_clock::time_point last_frame_time;

	HWFrameBuffer(int u0, int v0, int _w, int _h);
	
	void init();
	void init_lighting();
	void init_textures();
	void init_shadow_map();
	void init_environment_map();

	void set_tms(TM* tms, int num_tms);
	void set_lighting(V3 light_pos);
	void set_shadow_map(V3 light_pos, int shadow_map_size);
	void set_environment_map(CubeMap* cube_map);

	float* get_modelview_matrix(PPC* ppc);

	void move_light(V3 new_pos);
	
	void draw() override;

	void set_intrinsics(PPC* ppc);
	void set_extrinsics(PPC* ppc);

	void render(TM* tm, bool is_mirror);
	void render_shadow_map();
	void render_shadows();
	void render_environment_map();

	void render_fps_counter();
	void visualize_point_light();
};

