#pragma once
#include "gui.h"
#include "framebuffer.h"

class Scene {
public:

	GUI *gui;
	FrameBuffer *fb;
	PPC* ppc;
	V3* point_light;
	Scene();
	void DBG();
	void NewButton();
	void render();
	void render_cameras_as_frames();
};

extern Scene *scene;

void WriteName(int offset, FrameBuffer* fb);