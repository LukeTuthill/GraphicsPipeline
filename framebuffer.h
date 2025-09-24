#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "ppc.h"
#include "v3.h"

class PPC;

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int *pix;
	int w, h;
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	int handle(int guievent);
	void load_tiff(char* fname);
	void save_as_tiff(char* fname);
	void KeyboardHandle();

	void set(unsigned int color);
	void set(int u, int v, unsigned int color);
	void set_safe(int u, int v, unsigned int color);

	void draw_rectangle(int u, int v, int width, int height, unsigned int color);
	void draw_circle(int u, int v, int radius, unsigned int color);
	void draw_line(int u1, int v1, int u2, int v2, unsigned int color);
	void draw_line_safe(int u1, int v1, int u2, int v2, unsigned int color);
	void draw_triangle(int u1, int v1, int u2, int v2, int u3, int v3, unsigned int color);
	void draw_2d_point(V3 p, int psize, unsigned int color);
	void draw_3d_point(V3 p, PPC* ppc, int psize, unsigned int color);
	void draw_3d_segment(V3 C0, V3 C1, V3 V0, V3 V1, PPC* ppc);
	void draw_2d_segment(V3 C0, V3 C1, V3 V0, V3 V1, PPC* ppc);
};