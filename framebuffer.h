#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

#include "ppc.h"
 
class CubeMap;

class FrameBuffer : public Fl_Gl_Window {
public:
	unsigned int* pix;
	float* zb;
	int w, h;
	bool move_light;
	bool revolve_around_center;
	FrameBuffer(int u0, int v0, int _w, int _h);
	void draw();
	int handle(int guievent);
	void load_tiff(char* fname);
	void save_as_tiff(char* fname);
	void KeyboardHandle();

	void clear();
	void set(unsigned int color);
	void set(int u, int v, unsigned int color);
	void set_safe(int u, int v, unsigned int color);

	void set_zb(float z);
	void set_zb(int u, int v, float z);
	void set_zb_safe(int u, int v, float z);

	void set_with_zb(int u, int v, unsigned int color, float z);
	void set_with_zb_safe(int u, int v, unsigned int color, float z);

	void set_checker(int cw, unsigned int col0, unsigned int col1);

	unsigned int get(int u, int v);
	unsigned int get(float tu, float tv); //tu is [0, 1], tv is [0, 1]

	float get_zb(int u, int v);

	bool is_farther(int u, int v, float z);
	bool is_farther_safe(int u, int v, float z);

	void draw_rectangle(int u, int v, int width, int height, unsigned int color);
	void draw_circle(int u, int v, int radius, unsigned int color);
	void draw_line(int u1, int v1, int u2, int v2, unsigned int color);
	void draw_line_safe(int u1, int v1, int u2, int v2, unsigned int color);

	void draw_2d_point(V3 p, int psize, unsigned int color);
	void draw_3d_point(V3 p, PPC* ppc, int psize, unsigned int color);
	void visualize_point_light(V3 l, PPC* ppc);

	void draw_2d_segment(V3 V0, V3 V1, V3 C0, V3 C1);
	void draw_3d_segment(V3 V0, V3 V1, V3 C0, V3 C1, PPC* ppc);

	void draw_2d_triangle(V3 V0, V3 V1, V3 V2, V3 C0, V3 C1, V3 C2);
	void draw_3d_triangle(V3 V0, V3 V1, V3 V2, V3 C0, V3 C1, V3 C2, PPC* ppc);

	void draw_2d_texture_triangle(V3 V0, V3 V1, V3 V2, V3 C0, V3 C1, V3 C2, bool mirror_tiling, FrameBuffer* tex);

	void draw_2d_mirrored_triangle(V3 V0, V3 V1, V3 V2, V3 N0, V3 N1, V3 N2, PPC* ppc, CubeMap* cube_map);
};