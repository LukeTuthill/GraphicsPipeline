#include <GL/glew.h>
#include "framebuffer.h"
#include "math.h"
#include "scene.h"
#include "pong.h"
#include "ppc.h"

#include <tiffio.h>

using namespace std;

#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <strstream>
#include <cmath>


FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h) : 
	Fl_Gl_Window(u0, v0, _w, _h, 0) {
	w = _w;
	h = _h;
	pix = new unsigned int[w*h];
}

void FrameBuffer::draw() {

	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);

}

int FrameBuffer::handle(int event) {

	switch (event)
	{
	case FL_KEYBOARD: {
		KeyboardHandle();
		return 0;
	}
	case FL_MOVE: {
		int u = Fl::event_x();
		int v = Fl::event_y();
		if (u < 0 || u > w - 1 || v < 0 || v > h - 1)
			return 0;
		cerr << u << " " << v << "         \r";
		return 0;
	}
	default:
		return 0;
	}
	return 0;
}

void FrameBuffer::KeyboardHandle() {
	int key = Fl::event_key();

	if (!ppc_for_keyboard_control) return;
	float move_constant = 1.0f;
	float rotation_constant = 2.0f;

	switch (key) {
	case 'b':
	case 'B':
		cerr << "Running camera path from cameras.bin" << endl;
		scene->render();
		break;

	case 'u':
	case 'U':
		cerr << "INFO: pressed u key, saving current framebuffer to out.tiff" << endl;
		save_as_tiff((char*)"out.tiff");
		return;
	case 'p':
	case 'P':
		cerr << "INFO: pressed P key, saving current PPC to ppc vector" << endl;
		ppc_for_keyboard_control->save_to_file_vector();
		break;
	case 'o':
	case 'O':
		cerr << "Saving current PPCs to cameras.bin" << endl;
		save_to_file((char*)"cameras.bin");
		break;

	case FL_Up:
		ppc_for_keyboard_control->translate_forward(move_constant);
		break;
	case FL_Down:
		ppc_for_keyboard_control->translate_forward(-move_constant);
		break;
	case FL_Left:
		ppc_for_keyboard_control->translate_right(-move_constant);
		break;
	case FL_Right:
		ppc_for_keyboard_control->translate_right(move_constant);
		break;
	case FL_Page_Up:
		ppc_for_keyboard_control->translate_up(move_constant);
		break;
	case FL_Page_Down:
		ppc_for_keyboard_control->translate_up(-move_constant);
		break;

	case 'w':
	case 'W':
		ppc_for_keyboard_control->tilt(rotation_constant);
		break;
	case 's':
	case 'S':
		ppc_for_keyboard_control->tilt(-rotation_constant);
		break;
	case 'a':
	case 'A':
		ppc_for_keyboard_control->pan(-rotation_constant);
		break;
	case 'd':
	case 'D':
		ppc_for_keyboard_control->pan(rotation_constant);
		break;
	case 'r':
	case 'R':
		ppc_for_keyboard_control->roll(-rotation_constant);
		break;
	case 'f':
	case 'F':
		ppc_for_keyboard_control->roll(rotation_constant);
		break;
	case 't':
	case 'T':
		ppc_for_keyboard_control->zoom(rotation_constant);
		break;
	case 'g':
	case 'G':
		ppc_for_keyboard_control->zoom(-rotation_constant);
		break;
	}

	/* Pong handler
	switch (key) {
	case FL_Up: {
		if (!PongGame::pong_game) break;
		PongGame::pong_game->move_p1_up();
		break;
	}
	case FL_Down: {
		if (!PongGame::pong_game) break;
		PongGame::pong_game->move_p1_down();
		break;
	}
	case FL_Left: {
		if (!PongGame::pong_game) break;
		PongGame::pong_game->move_p2_up();
		break;
	}
	case FL_Right: {
		if (!PongGame::pong_game) break;
		PongGame::pong_game->move_p2_down();
		break;
	}
	default:
		cerr << "INFO: do not understand keypress" << endl;
		return;
	}
	*/
}

// load a tiff image to pixel buffer
void FrameBuffer::load_tiff(char* fname) {
	TIFF* in = TIFFOpen(fname, "r");
	if (in == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	int width, height;
	TIFFGetField(in, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(in, TIFFTAG_IMAGELENGTH, &height);
	if (w != width || h != height) {
		w = width;
		h = height;
		delete[] pix;
		pix = new unsigned int[w*h];
		size(w, h);
		glFlush();
		glFlush();
	}

	if (TIFFReadRGBAImage(in, w, h, pix, 0) == 0) {
		cerr << "failed to load " << fname << endl;
	}

	TIFFClose(in);
}

// save as tiff image
void FrameBuffer::save_as_tiff(char *fname) {

	TIFF* out = TIFFOpen(fname, "w");

	if (out == NULL) {
		cerr << fname << " could not be opened" << endl;
		return;
	}

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 4);
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1); 

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}



void FrameBuffer::set(unsigned int color) {
	for (int uv = 0; uv < w*h; uv++)
		pix[uv] = color;
}


void FrameBuffer::set(int u, int v, unsigned int color) {
	pix[(h - 1 - v) * w + u] = color;
}

void FrameBuffer::set_safe(int u, int v, unsigned int color) {
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1) return;
	set(u, v, color);
}


void FrameBuffer::draw_rectangle(int u, int v, int width, int height, unsigned int color) {
	if (u < 0 || u + width > w - 1 || 
		v < 0 || v + height > h - 1) return;

	for (int uc = u; uc < u + width; uc++) {
		for (int vc = v; vc < v + height; vc++) {
			set(uc, vc, color);
		}
	}
}

void FrameBuffer::draw_line(int u1, int v1, int u2, int v2, unsigned int color) {
	//Ensure first point is to the left of second point
	if (u1 > u2) {
		swap(u1, u2);
		swap(v1, v2);
	}

	if (u1 == u2) {
		if (v1 > v2) {
			swap(v1, v2);
		}
		for (int v = v1; v <= v2; v++) {
			set(u1, v, color);
		}
	}
	else if (v1 == v2) {
		for (int u = u1; u <= u2; u++) {
			set(u, v1, color);
		}
	}
	else {
		int du = u2 - u1;
		int dv = v2 - v1;

		if (abs(du) >= abs(dv)) {
			double slope = (double)dv / (double)du;
			double v = v1;
			for (int u = u1; u <= u2; u++) {
				set(u, (int)(v + .5), color);
				v += slope;
			}
		}
		else {
			if (v1 > v2) {
				swap(u1, u2);
				swap(v1, v2);
			}
			double slope = (double)du / (double)dv;
			double u = u1;
			for (int v = v1; v <= v2; v++) {
				set((int)(u + .5), v, color);
				u += slope;
			}
		}
	}
} 

void FrameBuffer::draw_line_safe(int u1, int v1, int u2, int v2, unsigned int color) {
	if (u1 < 0 || u1 > w - 1 || v1 < 0 || v1 > h - 1) return;
	if (u2 < 0 || u2 > w - 1 || v2 < 0 || v2 > h - 1) return;
	draw_line(u1, v1, u2, v2, color);
}
void FrameBuffer::draw_circle(int u, int v, int radius, unsigned int color) {
	if (u - radius < 0 || u + radius > w - 1 ||
		v - radius < 0 || v + radius > h - 1) return;

	for (int uc = -radius; uc <= radius; uc++) {
		for (int vc = -radius; vc <= radius; vc++) {
			if (uc * uc + vc * vc <= radius * radius) {
				set(u + uc, v + vc, color);
			}
		}
	}
}

void FrameBuffer::draw_triangle(int u1, int v1, int u2, int v2, int u3, int v3, unsigned int color) {
	//Sorts points by u value
	if (u1 > u2) {
		swap(u1, u2);
		swap(v1, v2);
	}
	if (u3 > u1) {
		swap(u3, u1);
		swap(v3, v1);
	}
	if (u2 > u3) {
		swap(u3, u2);
		swap(v3, v2);
	}

	double slope1 = (double)(v2 - v1) / (double)(u2 - u1);
	double slope2 = (double)(v3 - v1) / (double)(u3 - u1);

	for (int uc = u1; uc <= u3; uc++) {
		int v_start = (int)(v1 + slope1 * (uc - u1) + .5);
		int v_end = (int)(v1 + slope2 * (uc - u1) + .5);
		if (v_start > v_end) {
			swap(v_start, v_end);
		}
		for (int vc = v_start; vc <= v_end; vc++) {
			set(uc, vc, color);
		}
	}
}

void FrameBuffer::draw_2d_point(V3 P, int psize, unsigned int color) {
	int up = (int)P[0];
	int vp = (int)P[1];

	for (int u = up - psize / 2; u < up + psize / 2; u++) {
		for (int v = vp - psize / 2; v < vp + psize / 2; v++) {
			set_safe(u, v, color);
		}
	}
}


void FrameBuffer::draw_3d_point(V3 P, PPC *ppc, int psize,
	unsigned int color) {
	V3 PP;
	if (!ppc->project(P, PP))
		return;

	draw_2d_point(PP, psize, color);
}

void FrameBuffer::draw_3d_segment(V3 C0, V3 C1, V3 V0, V3 V1, PPC* ppc) {
	V3 PV0, PV1;
	if (!ppc->project(V0, PV0)) return;
	if (!ppc->project(V1, PV1)) return;
	PV0[2] = 0.0f;
	PV1[2] = 0.0f;
	draw_2d_segment(C0, C1, PV0, PV1, ppc);
}

void FrameBuffer::draw_2d_segment(V3 C0, V3 C1, V3 PV0, V3 PV1, PPC* ppc) {
	int pixn = (int)((PV1 - PV0).length() + 2);
	V3 curr_p = PV0;
	V3 p_diff = (PV1 - PV0) / (float)(pixn - 1);

	V3 curr_c = C0;
	V3 color_diff = (C1 - C0) / (float)(pixn - 1);
	for (int si = 0; si < pixn; si++) {
		unsigned int color = curr_c.convert_to_color_int();
		set_safe((int)curr_p[0], (int)curr_p[1], color);

		curr_p += p_diff;
		curr_c += color_diff;
	}
}
