#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix;
	int w, h;
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	int handle(int guievent);
	void LoadTiff(char* fname);
	void SaveAsTiff(char* fname);
	void KeyboardHandle();

	void Set(unsigned int color);
	void Set(int u, int v, unsigned int color);

	void DrawRectangle(int u, int v, int width, int height, unsigned int color);
	void DrawCircle(int u, int v, int radius, unsigned int color);
	void DrawLine(int u1, int v1, int u2, int v2, unsigned int color);
	void DrawTriangle(int u1, int v1, int u2, int v2, int u3, int v3, unsigned int color);
};