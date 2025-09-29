#pragma once

#include "v3.h"
#include "framebuffer.h"
#include "ppc.h"

class TM {
public:
	V3 *verts;
	int num_verts;

	V3 *colors; // vertex colors in V3 format (one float in [0.0f, 1.0f] per R, G, and B channel)
	V3* lighted_colors; // vertex colors after lighting

	unsigned int *tris; // triples of vertex indices
	int num_tris;

	V3* normals; // per-vertex normals

	TM() : verts(0), num_verts(0), colors(0), tris(0), num_tris(0), normals(0) {};
	TM(char* fname);

	//Cylinder constructor
	TM(V3 center, float radius, float height, int _num_verts, unsigned int color);
	void load_bin(char *fname); // load from file

	void draw_points(unsigned int color, int psize, PPC *ppc,
		FrameBuffer *fb);
	void rotate_about_arbitrary_axis(V3 aO, V3 ad, float angle_degrees);

	V3 get_center(); // return the average of all vertices

	void set_as_box(V3 p1, V3 p2, unsigned int color); 
    void get_bounding_box(V3& p1, V3& p2); // return p1, p2 via reference
	void translate(V3 tv);
	void position(V3 new_center);
	void scale(float s);

	void render_as_wireframe(PPC *ppc, FrameBuffer* fb, bool is_lighted);
	void rasterize(PPC* ppc, FrameBuffer* fb, bool is_lighted);

	void visualize_normals(float nl, PPC* ppc, FrameBuffer* fb);

	void light_directional(V3 ld, V3 eye_pos, float ka, float phong_exp);
	void light_point(V3 l, V3 eye_pos, float ka, float phong_exp);
};
