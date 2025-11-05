#pragma once

#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tm.h"
#include "pong.h"
#include "hw_framebuffer.h"

class Scene {
public:
	GUI* gui;
	FrameBuffer* fb;
	HWFrameBuffer* hw_fb;
	PPC* ppc;
	V3* point_light;
	int num_tms;
	TM* tms;
	ShadowMap* shadow_map;
	CubeMap* cube_map;

	bool render_light;
	float ambient_factor;
	int specular_exp;

	bool mirror_tiling = false;
	bool render_wireframe = false;

	bool pong = false;
	PongGame* pong_game;

	std::chrono::steady_clock::time_point last_frame_time;

	Scene();
	void DBG();
	void NewButton();
	void render(render_type rt);
	void render_shadows();
	void render(TM& tm, render_type rt);
	void render_cameras_as_frames();

};

extern Scene* scene;

void WriteName(int offset, FrameBuffer* fb);
TM* make_texture_tms();