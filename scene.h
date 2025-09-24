#pragma once
#include "gui.h"
#include "framebuffer.h"

class Scene {
public:

	GUI *gui;
	FrameBuffer *fb;
	Scene();
	void DBG();
	void NewButton();
	void render();
};

extern Scene *scene;

void WriteName(int offset, FrameBuffer* fb);