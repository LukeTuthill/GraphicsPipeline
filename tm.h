#pragma once

#include "v3.h"
#include "framebuffer.h"
#include "ppc.h"
#include "shadow_map.h"

class CubeMap;

enum class render_type {
	LIGHTED,
	NOT_LIGHTED,
	NORMAL_TILING_TEXTURED,
	MIRRORED_TILING_TEXTURED,
	MIRROR_ONLY
};

class TM {
public:
	V3* verts;
	V3* projected_verts;
	int num_verts;

	V3* colors; // vertex colors in V3 format (one float in [0.0f, 1.0f] per R, G, and B channel)
	V3* lighted_colors; // vertex colors after lighting

	unsigned int *tris; // triples of vertex indices
	int num_tris;

	V3* normals; // per-vertex normals

	V3* tcs; // texture coordinates per vertex
	FrameBuffer* tex; // texture map

	GLuint tex_id; // OpenGL texture ID

	TM() : verts(0), projected_verts(0), num_verts(0), lighted_colors(0), colors(0), tris(0), num_tris(0), normals(0), tcs(0), tex(0) {};
	TM(char* fname);

	//Cylinder constructor
	TM(V3 center, float radius, float height, int _num_verts, unsigned int color);

	//Plane constructor
	TM(V3 p1, V3 p2, unsigned int color);

	void load_bin(char *fname); // load from file

	void set_tex(FrameBuffer* fb, V3* tcs);

	void draw_points(unsigned int color, int psize, PPC *ppc,
		FrameBuffer *fb);
	void rotate_about_arbitrary_axis(V3 aO, V3 ad, float angle_degrees);

	V3 get_center(); // return the average of all vertices

	void set_as_box(V3 p1, V3 p2, unsigned int color); 
	void set_as_plane(V3 p1, V3 p2, unsigned int color);
	void set_as_quad(V3 p1, V3 p2, V3 p3, V3 p4, unsigned int color);
    void get_bounding_box(V3& p1, V3& p2); // return p1, p2 via reference
	void translate(V3 tv);
	void position(V3 new_center);
	void scale(float s);

	void render_as_wireframe(PPC *ppc, FrameBuffer* fb, bool is_lighted);
	void rasterize(PPC* ppc, FrameBuffer* fb, CubeMap* cube_map, render_type rt);

	void set_eeqs(M33 proj_verts, M33& eeqs);

	void visualize_normals(float nl, PPC* ppc, FrameBuffer* fb);

	void light_directional(ShadowMap* shadow_map, V3 eye_pos, float ka, int specular_exp);
	void light_point(ShadowMap* shadow_map, V3 eye_pos, float ka, int specular_exp);

private:
    void create_face(V3 origin, V3 u_dir, V3 v_dir, int u_steps, int v_steps, V3 normal, 
        const V3& color_vector, int& v_idx, int& t_idx);
};

