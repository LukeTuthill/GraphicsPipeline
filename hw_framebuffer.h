#pragma once

#include "framebuffer.h"
#include "tm.h"
#include "CGInterface.h"

#include <chrono>
#include <FL/Fl_Box.H>

struct Billboard {
	V3 V0;  // Bottom-left corner
	V3 V1;  // Bottom-right corner
	V3 V3;  // Top-left corner
	GLuint texture_id;
};

class HWFrameBuffer : public FrameBuffer {
public:
	PPC* ppc;
	TM* tms;
	int num_tms;

	bool* tm_is_reflector;

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
	bool billboards_initialized;

	bool render_wireframe;

	std::chrono::steady_clock::time_point last_frame_time;

	CGInterface* cgi;
	ShaderOneInterface* soi;

	int num_billboards;
	std::vector<Billboard> billboards;
	V3* billboard_vertices;
	GLuint* billboard_texture_ids;
	int billboard_texture_size;

	std::vector<int> ground_plane_tm_indices;
	std::vector<int> regular_billboard_tm_indices;

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

	void init_billboard_from_tm(int tm_index);
	void create_billboard_from_tm(int tm_index);

	void create_ground_plane_billboard(int tm_index);
	void init_ground_plane_billboard(int tm_index);

	void clear_billboards();
	void init_billboards();
	GLuint render_tm_to_texture(int tm_index, int texture_size);
	GLuint create_solid_color_texture(V3 color_vec, int texture_size);
};

