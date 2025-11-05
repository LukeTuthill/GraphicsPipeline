#define _USE_MATH_DEFINES
#include <cmath>

#include "ppc.h"
#include "framebuffer.h"

#include <fstream>

PPC::PPC() {
	w = 0;
	h = 0;
}

PPC::PPC(float hfov, int _w, int _h) {
	w = _w;
	h = _h;
	C = V3(0.0f, 0.0f, 0.0f);
	a = V3(1.0f, 0.0f, 0.0f);
	b = V3(0.0f, -1.0f, 0.0f);
	float hfovr = hfov * (float)M_PI / 180.0f;
	c = V3(-(float)w / 2.0f, (float)h / 2.0f, 
		-(float)w / (2.0f*tan(hfovr / 2.0f)));

	m.set_column(0, a);
	m.set_column(1, b);
	m.set_column(2, c);
	m_inverted = m.inverted();
}

PPC* ppc_for_keyboard_control = nullptr;
std::vector<PPC> ppcs_to_save;

V3 PPC::get_vd() {
	return (a ^ b).normalized();
}

float PPC::get_focal_length() {
	return get_vd() * c;
}

int PPC::project(V3 P, V3& PP) {
	int ret = 1;

	V3 q = m_inverted*(P - C);

	if (q[2] <= 0.0f) {
		PP = V3(FLT_MAX, 0, 0);
		return 0;
	}

	PP[0] = q[0] / q[2];
	PP[1] = q[1] / q[2];
	PP[2] = 1.0f / q[2];

	return ret;
}

void PPC::translate(V3 tv) {
	C += tv;
}

void PPC::tilt(float angle_degrees) {
	a = a.rotate_point(V3(0, 0, 0), a, angle_degrees);
	b = b.rotate_point(V3(0, 0, 0), a, angle_degrees);
	c = c.rotate_point(V3(0, 0, 0), a, angle_degrees);

	m.set_column(0, a);
	m.set_column(1, b);
	m.set_column(2, c);
	m_inverted = m.inverted();
}

void PPC::pan(float angle_degrees) {
	a = a.rotate_point(V3(0, 0, 0), b, angle_degrees);
	b =	b.rotate_point(V3(0, 0, 0), b, angle_degrees);
	c = c.rotate_point(V3(0, 0, 0), b, angle_degrees);

	m.set_column(0, a);
	m.set_column(1, b);
	m.set_column(2, c);
	m_inverted = m.inverted();
}

void PPC::roll(float angle_degrees) {
	V3 axis = get_vd();
	a = a.rotate_point(V3(0, 0, 0), axis, angle_degrees);
	b = b.rotate_point(V3(0, 0, 0), axis, angle_degrees);
	c = c.rotate_point(V3(0, 0, 0), axis, angle_degrees);

	m.set_column(0, a);
	m.set_column(1, b);
	m.set_column(2, c);
	m_inverted = m.inverted();
}

void PPC::translate_right(float t) {
	C += t * a;
}

void PPC::translate_up(float t) {
	C -= t * b;
}

void PPC::translate_forward(float t) {
	C += t * get_vd();
}

void PPC::zoom(float s) {
	c += s * get_vd();

	m.set_column(2, c);
	m_inverted = m.inverted();
}


void PPC::rotate_about_arbitrary_axis(V3 aO, V3 ad, float angle_degrees) {
	a = a.rotate_point(aO, ad, angle_degrees);
	b = b.rotate_point(aO, ad, angle_degrees);
	c = c.rotate_point(aO, ad, angle_degrees);
	C = C.rotate_point(aO, ad, angle_degrees);

	m.set_column(0, a);
	m.set_column(1, b);
	m.set_column(2, c);
	m_inverted = m.inverted();
}


void PPC::revolve_left_right(V3 center, float angle_degrees) {
	rotate_about_arbitrary_axis(center, b, angle_degrees);
}

void PPC::revolve_up_down(V3 center, float angle_degrees) {
	rotate_about_arbitrary_axis(center, -1 * a, angle_degrees);
}

void PPC::pose(V3 new_C, V3 look_at_point, V3 up_dir) {
	V3 new_a, new_b, new_c;

	V3 new_vd = (look_at_point - new_C).normalized();
	new_a = (new_vd ^ up_dir).normalized();
	new_b = (new_vd ^ new_a).normalized();
	float f = get_focal_length();

	new_c = new_vd * f - new_a * ((float)w / 2.0f) - new_b * ((float)h / 2.0f);

	a = new_a;
	b = new_b;
	c = new_c;
	C = new_C;

	m.set_column(0, a);
	m.set_column(1, b);
	m.set_column(2, c);

	m_inverted = m.inverted();
}

PPC PPC::interpolate(PPC* ppc2, float t) {
	if (t == 0) return *this;
	if (t == 1) return *ppc2;

	PPC ppc_i;
	ppc_i.C = C + t * (ppc2->C - C);
	ppc_i.a = a + t * (ppc2->a - a);
	ppc_i.b = b + t * (ppc2->b - b);
	ppc_i.c = c + t * (ppc2->c - c);
	ppc_i.w = w;
	ppc_i.h = h;
	ppc_i.m.set_column(0, ppc_i.a);
	ppc_i.m.set_column(1, ppc_i.b);
	ppc_i.m.set_column(2, ppc_i.c);
	ppc_i.m_inverted = ppc_i.m.inverted();

	return ppc_i;
}


void PPC::visualize(FrameBuffer* fb, PPC* ppc, float focal_length) {
	float f = fabs(get_vd() * c);
	float scale_factor = focal_length / f;

	// Calculate the 4 corners of the camera frustum at the scaled distance
	V3 corner_offset = c * scale_factor;
	V3 tl = C + corner_offset;
	V3 tr = C + corner_offset + a * (float)w * scale_factor;
	V3 br = C + corner_offset + a * (float)w * scale_factor + b * (float)h * scale_factor;
	V3 bl = C + corner_offset + b * (float)h * scale_factor;
	
	// Project the corners and camera center
	V3 tl_proj, tr_proj, br_proj, bl_proj, pc;
	if (ppc->project(tl, tl_proj) == 0) return;
	if (ppc->project(tr, tr_proj) == 0) return;
	if (ppc->project(br, br_proj) == 0) return;
	if (ppc->project(bl, bl_proj) == 0) return;
	if (ppc->project(C, pc) == 0) return;

	// Draw the camera frustum
	fb->draw_line_safe((int)tl_proj[0], (int)tl_proj[1], (int)tr_proj[0], (int)tr_proj[1], 0xffff0000);
	fb->draw_line_safe((int)tr_proj[0], (int)tr_proj[1], (int)br_proj[0], (int)br_proj[1], 0xffff0000);
	fb->draw_line_safe((int)br_proj[0], (int)br_proj[1], (int)bl_proj[0], (int)bl_proj[1], 0xffff0000);
	fb->draw_line_safe((int)bl_proj[0], (int)bl_proj[1], (int)tl_proj[0], (int)tl_proj[1], 0xffff0000);
	fb->draw_line_safe((int)pc[0], (int)pc[1], (int)tl_proj[0], (int)tl_proj[1], 0xff000000);
	fb->draw_2d_point(pc, 10, 0xff000000);
}

int load_from_file(PPC** ppcs, char* fname) {
	ifstream ifs(fname, ios::binary);

	if (!ifs) return 0;

	int num_ppcs;
	ifs >> num_ppcs;

	*ppcs = new PPC[num_ppcs];

	for (int i = 0; i < num_ppcs; i++) {
		PPC& ppc = (*ppcs)[i];
		ifs >> ppc.a >> ppc.b >> ppc.c >> ppc.C >> ppc.w >> ppc.h;
		
		ppc.m.set_column(0, ppc.a);
		ppc.m.set_column(1, ppc.b);
		ppc.m.set_column(2, ppc.c);
		ppc.m_inverted = ppc.m.inverted();
	}

	ifs.close();
	return num_ppcs;
}

void save_to_file(char* fname) {
	ofstream ofs(fname, ios::binary);
	if (!ofs) return;

	size_t num_ppcs = ppcs_to_save.size();
	ofs << num_ppcs;
	for (int i = 0; i < num_ppcs; i++) {
		PPC& ppc = ppcs_to_save[i];
		ofs << " " << ppc.a << " " << ppc.b << " " << ppc.c << " " << ppc.C << " " << ppc.w << " " << ppc.h;
	}
	ofs.close();
}

void PPC::save_to_file_vector() {
	PPC copy = *this;
	ppcs_to_save.push_back(copy);
}
