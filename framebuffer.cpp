#include <GL/glew.h>
#include "framebuffer.h"
#include "math.h"
#include "scene.h"
#include "pong.h"

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

}

// load a tiff image to pixel buffer
void FrameBuffer::LoadTiff(char* fname) {
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
void FrameBuffer::SaveAsTiff(char *fname) {

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

	for (uint32 row = 0; row < (unsigned int)h; row++) {
		TIFFWriteScanline(out, &pix[(h - row - 1) * w], row);
	}

	TIFFClose(out);
}



void FrameBuffer::Set(unsigned int color) {
	for (int uv = 0; uv < w*h; uv++)
		pix[uv] = color;
}


void FrameBuffer::Set(int u, int v, unsigned int color) {
	pix[(h - 1 - v) * w + u] = color;
}


void FrameBuffer::DrawRectangle(int u, int v, int width, int height, unsigned int color) {
	if (u < 0 || u + width > w - 1 || 
		v < 0 || v + height > h - 1) return;

	for (int uc = u; uc < u + width; uc++) {
		for (int vc = v; vc < v + height; vc++) {
			Set(uc, vc, color);
		}
	}
}

void FrameBuffer::DrawLine(int u1, int v1, int u2, int v2, unsigned int color) {
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
			Set(u1, v, color);
		}
	}
	else if (v1 == v2) {
		for (int u = u1; u <= u2; u++) {
			Set(u, v1, color);
		}
	}
	else {
		int du = u2 - u1;
		int dv = v2 - v1;

		if (abs(du) >= abs(dv)) {
			double slope = (double)dv / (double)du;
			double v = v1;
			for (int u = u1; u <= u2; u++) {
				Set(u, (int)(v + .5), color);
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
				Set((int)(u + .5), v, color);
				u += slope;
			}
		}
	}
} 

void FrameBuffer::DrawCircle(int u, int v, int radius, unsigned int color) {
	if (u - radius < 0 || u + radius > w - 1 ||
		v - radius < 0 || v + radius > h - 1) return;

	for (int uc = -radius; uc <= radius; uc++) {
		for (int vc = -radius; vc <= radius; vc++) {
			if (uc * uc + vc * vc <= radius * radius) {
				Set(u + uc, v + vc, color);
			}
		}
	}
}

void FrameBuffer::DrawTriangle(int u1, int v1, int u2, int v2, int u3, int v3, unsigned int color) {
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
			Set(uc, vc, color);
		}
	}

}
