#pragma once

#include "v3.h"
#include "m33.h"
#include <vector>

class FrameBuffer;

class PPC {
public:
	V3 a, b, c, C;
	M33 m, m_inverted; //Save matrix and inversion to save compute time, updated every time a, b, c are updated
	int w, h;
	PPC();
	PPC(float hfov, int _w, int _h);
	int project(V3 P, V3& PP);
	void translate(V3 tv);
	V3 get_vd();

	void tilt(float angle_degrees); //rotate about a
	void pan(float angle_degrees); //rotate about b
	void roll(float angle_degrees); //rotate about c

	void translate_right(float t); //translate along a
	void translate_up(float t); //translate along b
	void translate_forward(float t); //translate along c

	void rotate_about_arbitrary_axis(V3 aO, V3 ad, float angle_degrees);
	void revolve_left_right(V3 center, float angle_degrees); //revolve around b
	void revolve_up_down(V3 center, float angle_degrees); //revolve around -a

	void zoom(float s); //scale c by s, zoom in if s>0, out if s<0

	PPC interpolate(PPC* ppc2, float t); //t in [0,1]

	friend int load_from_file(PPC** ppcs, char* fname);
	friend void save_to_file(char* fname);
	void save_to_file_vector();

	void visualize(FrameBuffer* fb, PPC* ppc, float focal_length);
};

extern std::vector<PPC> ppcs_to_save;
