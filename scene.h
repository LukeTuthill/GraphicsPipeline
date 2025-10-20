#pragma once
#include "gui.h"
#include "framebuffer.h"
#include "ppc.h"
#include "tm.h"
#include "pong.h"

class Scene {
public:

	GUI *gui;
	FrameBuffer *fb;
	PPC* ppc;
	V3* point_light;
	int num_tms;
	TM* tms;
	ShadowMap* shadow_map;

	bool render_light;
	float ambient_factor;
	int specular_exp;

	bool mirror = false;

	bool pong = false;
	PongGame* pong_game;

	Scene();
	void DBG();
	void NewButton();
	void render();
	void render_shadows();
	void render(TM& tm);
	void render_cameras_as_frames();
};

extern Scene *scene;

void WriteName(int offset, FrameBuffer* fb);