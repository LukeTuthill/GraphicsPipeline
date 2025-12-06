#include <GL/glew.h>
#include <tiffio.h>
#include <FL/fl_ask.h>

#include <iostream>
#include <fstream>
#include <strstream>

#include "framebuffer.h"
#include "scene.h"
#include "pong.h"
#include "cube_map.h"

using namespace std;

FrameBuffer::FrameBuffer(int u0, int v0, int _w, int _h) : 
	Fl_Gl_Window(u0, v0, _w, _h, 0) {
	w = _w;
	h = _h;
	pix = new unsigned int[w*h];
	zb = new float[w*h];
	move_light = false;
	revolve_around_center = false;
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

	if (!scene->ppc) return;
	float move_constant = 5.0f;
	float rotation_constant = 2.0f;

	if (scene->tetris_game) {
		switch (key) {
		case 'r':
		case 'R':
		case '/': {
			scene->tetris_game->set_move(3);
			break;
		}
		case 'f':
		case 'F':
		case '.': {
			scene->tetris_game->set_move(4);
			break;
		}
		case 's':
		case 'S':
		case FL_Down: {
			scene->tetris_game->set_move(5);
			break;
		}
		case 'a':
		case 'A':
		case FL_Left: {
			scene->tetris_game->set_move(1);
			break;
		}
		case 'd':
		case 'D':
		case FL_Right: {
			scene->tetris_game->set_move(2);
			break;
		}
		default:
			cerr << "INFO: do not understand keypress" << endl;
		}
		return;
	}

	if (scene->pong) {
		switch (key) {
		case FL_Up: {
			scene->pong_game->move_p1_up();
			break;
		}
		case FL_Down: {
			scene->pong_game->move_p1_down();
			break;
		}
		case FL_Left: {
			scene->pong_game->move_p2_up();
			break;
		}
		case FL_Right: {
			scene->pong_game->move_p2_down();
			break;
		}
		default:
			cerr << "INFO: do not understand keypress" << endl;
			return;
		}
;
	}

	switch (key) {
	case 'c':
	case 'C':
		scene->render_wireframe = !scene->render_wireframe;
		if (scene->render_wireframe) {
			cerr << "HW rendering in wireframe mode" << endl;
		}
		else {
			cerr << "HW rendering in filled in mode" << endl;
		}
		break;
	case 'n':
	case 'N':
		revolve_around_center = !revolve_around_center;
		if (revolve_around_center) {
			cerr << "Revolve around center ON" << endl;
		}
		else {
			cerr << "Revolve around center OFF" << endl;
		}
		break;
	case 'm':
	case 'M':
		scene->mirror_tiling = !scene->mirror_tiling;
		if (scene->mirror_tiling) {
			cerr << "Mirror mode ON" << endl;
		}
		else {
			cerr << "Mirror mode OFF" << endl;
		}
		break;
	case '1': {
		const char* val = fl_input("New Ambient Factor (0.0-1.0)", "0.4");
		if (val) {
			scene->ambient_factor = (float)atof(val);
			scene->ambient_factor = fminf(fmaxf(0.0f, scene->ambient_factor), 1.0f);
			cerr << "Ambient factor is now " << scene->ambient_factor << endl;
		}
		break;
	}
	case '2': {
		const char* val = fl_input("New Specular Exponent (>0)", "200");
		if (val) {
			scene->specular_exp = atoi(val);
			scene->specular_exp = max(0, scene->specular_exp);
			cerr << "Specular exponent is now " << scene->specular_exp << endl;
		}
		break;
	}

	case 'k':
	case 'K':
		scene->render_light = !scene->render_light;
		if (scene->render_light) {
			cerr << "Rendering with light" << endl;
		}
		else {
			cerr << "Rendering without light" << endl;
		}
		break;
	case 'l':
	case 'L':
		move_light = !move_light;
		if (move_light) {
			cerr << "Light movement enabled" << endl;
		}
		else {
			cerr << "Camera movement enabled" << endl;
		}
		break;
	case 'b':
	case 'B':
		cerr << "Running camera path from cameras.bin" << endl;
		scene->render_cameras_as_frames();
		break;

	case 'u':
	case 'U':
		cerr << "Saving current framebuffer to out.tiff" << endl;
		save_as_tiff((char*)"out.tiff");
		return;
	case 'p':
	case 'P':
		cerr << "Saving current PPC to ppc vector" << endl;
		scene->ppc->save_to_file_vector();
		break;
	case 'o':
	case 'O':
		cerr << "Saving current PPCs to cameras.bin" << endl;
		save_to_file((char*)"cameras.bin");
		break;

	case FL_Up:
		if (move_light) {
            *(scene->point_light) = *(scene->point_light) + V3(0.0f, 0.0f, -move_constant);
			break;
		}
		scene->ppc->translate_forward(move_constant);
		break;
	case FL_Down:
		if (move_light) {
            *(scene->point_light) = *(scene->point_light) + V3(0.0f, 0.0f, move_constant);
			break;
		}
		scene->ppc->translate_forward(-move_constant);
		break;
	case FL_Left:
		if (move_light) {
            *(scene->point_light) = *(scene->point_light) + V3(-move_constant, 0.0f, 0.0f);
			break;
		}
		scene->ppc->translate_right(-move_constant);
		break;
	case FL_Right:
		if (move_light) {
            *(scene->point_light) = *(scene->point_light) + V3(move_constant, 0.0f, 0.0f);
			break;
		}
		scene->ppc->translate_right(move_constant);
		break;
	case '/':
		if (move_light) {
            *(scene->point_light) = *(scene->point_light) + V3(0.0f, move_constant, 0.0f);
			break;
		}
		scene->ppc->translate_up(move_constant);
		break;
	case '.':
		if (move_light) {
            *(scene->point_light) = *(scene->point_light) + V3(0.0f, -move_constant, 0.0f);
			break;
		}
		scene->ppc->translate_up(-move_constant);
		break;

	case 'w':
	case 'W':
		if (revolve_around_center) {
			scene->ppc->revolve_up_down(V3(0.0f, 0.0f, 0.0f), rotation_constant);
			break;
		}
		scene->ppc->tilt(rotation_constant);
		break;
	case 's':
	case 'S':
		if (revolve_around_center) {
			scene->ppc->revolve_up_down(V3(0.0f, 0.0f, 0.0f), -rotation_constant);
			break;
		}
		scene->ppc->tilt(-rotation_constant);
		break;
	case 'a':
	case 'A':
		if (revolve_around_center) {
			scene->ppc->revolve_left_right(V3(0.0f, 0.0f, 0.0f), rotation_constant);
			break;
		}
		scene->ppc->pan(-rotation_constant);
		break;
	case 'd':
	case 'D':
		if (revolve_around_center) {
			scene->ppc->revolve_left_right(V3(0.0f, 0.0f, 0.0f), -rotation_constant);
			break;
		}
		scene->ppc->pan(rotation_constant);
		break;
	case 'r':
	case 'R':
		scene->ppc->roll(-rotation_constant);
		break;
	case 'f':
	case 'F':
		scene->ppc->roll(rotation_constant);
		break;
	case 't':
	case 'T':
		scene->ppc->zoom(rotation_constant);
		break;
	case 'g':
	case 'G':
		scene->ppc->zoom(-rotation_constant);
		break;
	}

	/* Pong handler
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
		throw new runtime_error("TIFF file could not be found");
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

void FrameBuffer::clear() {
	for (int uv = 0; uv < w * h; uv++) {
		pix[uv] = 0xFFFFFFFF;
		zb[uv] = 0.0f;
	}
}

void FrameBuffer::set(unsigned int color) {
	for (int uv = 0; uv < w*h; uv++)
		pix[uv] = color;
}

void FrameBuffer::set_zb(float z) {
	for (int uv = 0; uv < w * h; uv++) {
		zb[uv] = z;
	}
}


void FrameBuffer::set(int u, int v, unsigned int color) {
	pix[(h - 1 - v) * w + u] = color;
}

void FrameBuffer::set_zb(int u, int v, float z) {
	zb[(h - 1 - v) * w + u] = z;
}

void FrameBuffer::set_safe(int u, int v, unsigned int color) {
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1) return;
	set(u, v, color);
}

void FrameBuffer::set_zb_safe(int u, int v, float z) {
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1) return;
	set_zb(u, v, z);
}

void FrameBuffer::set_with_zb(int u, int v, unsigned int color, float z) {
	if (is_farther(u, v, z)) return;
	set(u, v, color);
	set_zb(u, v, z);
}

void FrameBuffer::set_with_zb_safe(int u, int v, unsigned int color, float z) {
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1) return;
	set_with_zb(u, v, color, z);
}

unsigned int FrameBuffer::get(int u, int v) {
	return pix[(h - 1 - v) * w + u];
}

unsigned int FrameBuffer::get(float tu, float tv) {
	int u = (int)(tu * (w - 1));
	int v = (int)(tv * (h - 1));
	return get(u, v);
}

float FrameBuffer::get_zb(int u, int v) {
	return zb[(h - 1 - v) * w + u];
}

bool FrameBuffer::is_farther(int u, int v, float z) {
	return get_zb(u, v) > z;
}

bool FrameBuffer::is_farther_safe(int u, int v, float z) {
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1) return false;
	return is_farther(u, v, z);
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

void FrameBuffer::draw_2d_point(V3 P, int psize, unsigned int color) {
	int up = (int)P[0];
	int vp = (int)P[1];

	for (int u = up - psize / 2; u < up + psize / 2; u++) {
		for (int v = vp - psize / 2; v < vp + psize / 2; v++) {
			set_with_zb_safe(u, v, color, P[2]);
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

void FrameBuffer::visualize_point_light(V3 l, PPC* ppc) {
	draw_3d_point(l, ppc, 7, 0xFFFF0000);
}

void FrameBuffer::draw_3d_segment(V3 V0, V3 V1, V3 C0, V3 C1, PPC* ppc) {
	V3 PV0, PV1;
	if (!ppc->project(V0, PV0)) return;
	if (!ppc->project(V1, PV1)) return;
	draw_2d_segment(PV0, PV1, C0, C1);
}

void FrameBuffer::draw_2d_segment(V3 V0, V3 V1, V3 C0, V3 C1) {
	int pixn = (int)((V1 - V0).length() + 2);
	V3 curr_p = V0;
	V3 p_diff = (V1 - V0) / (float)(pixn - 1);

	V3 curr_c = C0;
	V3 color_diff = (C1 - C0) / (float)(pixn - 1);

	for (int si = 0; si < pixn; si++) {
		unsigned int color = curr_c.convert_to_color_int();
		set_with_zb_safe((int)curr_p[0], (int)curr_p[1], color, curr_p[2]);

		curr_p += p_diff;
		curr_c += color_diff;
	}
}

void FrameBuffer::draw_3d_triangle(V3 V0, V3 V1, V3 V2, V3 C0, V3 C1, V3 C2, PPC* ppc) {
	V3 PV0, PV1, PV2;
	if (!ppc->project(V0, PV0)) return;
	if (!ppc->project(V1, PV1)) return;
	if (!ppc->project(V2, PV2)) return;

	draw_2d_triangle(PV0, PV1, PV2, C0, C1, C2);
}

void FrameBuffer::draw_2d_triangle(V3 V0, V3 V1, V3 V2, V3 C0, V3 C1, V3 C2) {
	V3 a = V3();
	V3 b = V3();
	V3 c = V3();

	//0 to 1
	a[0] = V1[1] - V0[1]; 
	b[0] = -V1[0] + V0[0];
	c[0] = -V1[1] * V0[0] + V0[1] * V1[0];

	//1 to 2
	a[1] = V2[1] - V1[1];
	b[1] = -V2[0] + V1[0];
	c[1] = -V2[1] * V1[0] + V1[1] * V2[0];

	//2 to 0
	a[2] = V0[1] - V2[1];
	b[2] = -V0[0] + V2[0];
	c[2] = -V0[1] * V2[0] + V2[1] * V0[0];

	float sidedness = a[0] * V2[0] + b[0] * V2[1] + c[0];
	if (sidedness < 0) {
		a[0] *= -1;
		b[0] *= -1;
		c[0] *= -1;
	}

	sidedness = a[1] * V0[0] + b[1] * V0[1] + c[1];
	if (sidedness < 0) {
		a[1] *= -1;
		b[1] *= -1;
		c[1] *= -1;
	}

	sidedness = a[2] * V1[0] + b[2] * V1[1] + c[2];
	if (sidedness < 0) {
		a[2] *= -1;
		b[2] *= -1;
		c[2] *= -1;
	}

	float umin = fmax(0.0f, fmin(fmin(V0[0], V1[0]), V2[0]));
	float umax = fmin((float)(w - 1), fmax(fmax(V0[0], V1[0]), V2[0]));
	float vmin = fmax(0.0f, fmin(fmin(V0[1], V1[1]), V2[1]));
	float vmax = fmin((float)(h - 1), fmax(fmax(V0[1], V1[1]), V2[1]));

	int left = (int)(umin + .5f);
	int right = (int)(umax - .5f);
	int top = (int)(vmin + .5f);
	int bottom = (int)(vmax - .5f);

	V3 currEELS = V3();
	V3 currEE = V3();
	
	currEELS = a * (left + .5f) + b * (top + .5f) + c;
	
	//Computes twice signed area of the triangle using edge function V0-V1 and V2
	float area = a[0] * V2[0] + b[0] * V2[1] + c[0]; 

	for (int v = top; v <= bottom; v++) {
		currEE = currEELS;
		
		for (int u = left; u <= right; u++) {
			if (currEE[0] >= 0 && currEE[1] >= 0 && currEE[2] >= 0) {

				//Computes barycentric weights
				float weight_0 = (a[1] * u + b[1] * v + c[1]) / area;
				float weight_1 = (a[2] * u + b[2] * v + c[2]) / area;
				float weight_2 = 1.0f - weight_0 - weight_1;

				//Interpolates depth and color
				float curr_z = weight_0 * V0[2] + weight_1 * V1[2] + weight_2 * V2[2];
				V3 color_vector = weight_0 * C0 + weight_1 * C1 + weight_2 * C2;
				
				set_with_zb(u, v, color_vector.convert_to_color_int(), curr_z);
			}
			currEE += a;
		}
		currEELS += b;
	}

}

void FrameBuffer::draw_2d_texture_triangle(V3 V0, V3 V1, V3 V2, V3 tex0, V3 tex1, V3 tex2, bool mirror_tiling, FrameBuffer* tex) {
    V3 a = V3();
    V3 b = V3();
    V3 c = V3();

    // 0 to 1
    a[0] = V1[1] - V0[1];
    b[0] = -V1[0] + V0[0];
    c[0] = -V1[1] * V0[0] + V0[1] * V1[0];

    // 1 to 2
    a[1] = V2[1] - V1[1];
    b[1] = -V2[0] + V1[0];
    c[1] = -V2[1] * V1[0] + V1[1] * V2[0];

    // 2 to 0
    a[2] = V0[1] - V2[1];
    b[2] = -V0[0] + V2[0];
    c[2] = -V0[1] * V2[0] + V2[1] * V0[0];

    float sidedness = a[0] * V2[0] + b[0] * V2[1] + c[0];
    if (sidedness < 0) { a[0] *= -1; b[0] *= -1; c[0] *= -1; }

    sidedness = a[1] * V0[0] + b[1] * V0[1] + c[1];
    if (sidedness < 0) { a[1] *= -1; b[1] *= -1; c[1] *= -1; }

    sidedness = a[2] * V1[0] + b[2] * V1[1] + c[2];
    if (sidedness < 0) { a[2] *= -1; b[2] *= -1; c[2] *= -1; }

    float umin = fmaxf(0.0f, fminf(fminf(V0[0], V1[0]), V2[0]));
    float umax = fminf((float)(w - 1), fmaxf(fmaxf(V0[0], V1[0]), V2[0]));
    float vmin = fmaxf(0.0f, fminf(fminf(V0[1], V1[1]), V2[1]));
    float vmax = fminf((float)(h - 1), fmaxf(fmaxf(V0[1], V1[1]), V2[1]));

    int left = (int)(umin + .5f);
    int right = (int)(umax - .5f);
    int top = (int)(vmin + .5f);
    int bottom = (int)(vmax - .5f);

    V3 currEELS = V3();
    V3 currEE = V3();
	
	currEELS = a * (left + .5f) + b * (top + .5f) + c;
	
	// Twice the signed area for barycentric normalization
	float area = a[0] * V2[0] + b[0] * V2[1] + c[0];
	if (area == 0.0f) return;

	V3 invz = { V0[2], V1[2], V2[2] };

	V3 tex0_over_z = tex0 * invz[0];
	V3 tex1_over_z = tex1 * invz[1];
	V3 tex2_over_z = tex2 * invz[2];

	V3 u_over_z = { tex0_over_z[0], tex1_over_z[0], tex2_over_z[0] };
	V3 v_over_z = { tex0_over_z[1], tex1_over_z[1], tex2_over_z[1] };

    for (int v = top; v <= bottom; v++) {
        currEE = currEELS;

        for (int u = left; u <= right; u++) {
            if (currEE[0] >= 0 && currEE[1] >= 0 && currEE[2] >= 0) {
                // Barycentric weights (screen-space)
                float w0 = (a[1] * u + b[1] * v + c[1]) / area;
                float w1 = (a[2] * u + b[2] * v + c[2]) / area;
                float w2 = 1.0f - w0 - w1;

				V3 w = { w0, w1, w2 };

                // Interpolate depth for z-buffer (affine is fine for z)
				float curr_z = w * invz + .00001f;

                // Perspective-correct interpolate texture coordinates
				float tu = w * u_over_z / curr_z;
				float tv = w * v_over_z / curr_z;

				if (!mirror_tiling) {
					// Clamp to [0, 1] range while accounting for tiling, no mirroring
					tu -= floor(tu);
					tv -= floor(tv);
				}
				else {
					//Mirroring mode for tiling
					if (int(floor(tu)) % 2 == 1)
						tu = 1.0f - (tu - floor(tu));
					else
						tu -= floor(tu);

					if (int(floor(tv)) % 2 == 1)
						tv = 1.0f - (tv - floor(tv));
					else
						tv -= floor(tv);
				}

				unsigned int color = tex->get(tu, tv);
                set_with_zb(u, v, color, curr_z);
            }
            currEE += a;
        }
        currEELS += b;
    }
}

void FrameBuffer::set_checker(int cw, unsigned int col0, unsigned int col1) {
	for (int v = 0; v < h; v++) {
		for (int u = 0; u < w; u++) {
			int cu, cv;
			cu = u / cw;
			cv = v / cw;
			if ((cu+cv)%2)
				set(u, v, col0);
			else
				set(u, v, col1);
		}
	}
}

void FrameBuffer::draw_2d_mirrored_triangle(V3 V0, V3 V1, V3 V2, V3 N0, V3 N1, V3 N2, PPC* ppc, CubeMap* cube_map) {
    V3 a = V3();
    V3 b = V3();
    V3 c = V3();

    // 0 to 1
    a[0] = V1[1] - V0[1];
    b[0] = -V1[0] + V0[0];
    c[0] = -V1[1] * V0[0] + V0[1] * V1[0];

    // 1 to 2
    a[1] = V2[1] - V1[1];
    b[1] = -V2[0] + V1[0];
    c[1] = -V2[1] * V1[0] + V1[1] * V2[0];

    // 2 to 0
    a[2] = V0[1] - V2[1];
    b[2] = -V0[0] + V2[0];
    c[2] = -V0[1] * V2[0] + V2[1] * V0[0];

    float sidedness = a[0] * V2[0] + b[0] * V2[1] + c[0];
    if (sidedness < 0) { a[0] *= -1; b[0] *= -1; c[0] *= -1; }

    sidedness = a[1] * V0[0] + b[1] * V0[1] + c[1];
    if (sidedness < 0) { a[1] *= -1; b[1] *= -1; c[1] *= -1; }

    sidedness = a[2] * V1[0] + b[2] * V1[1] + c[2];
    if (sidedness < 0) { a[2] *= -1; b[2] *= -1; c[2] *= -1; }

    float umin = fmaxf(0.0f, fminf(fminf(V0[0], V1[0]), V2[0]));
    float umax = fminf((float)(w - 1), fmaxf(fmaxf(V0[0], V1[0]), V2[0]));
    float vmin = fmaxf(0.0f, fminf(fminf(V0[1], V1[1]), V2[1]));
    float vmax = fminf((float)(h - 1), fmaxf(fmaxf(V0[1], V1[1]), V2[1]));

    int left = (int)(umin + .5f);
    int right = (int)(umax - .5f);
    int top = (int)(vmin + .5f);
    int bottom = (int)(vmax - .5f);

    V3 currEELS = V3();
    V3 currEE = V3();
	
	currEELS = a * (left + .5f) + b * (top + .5f) + c;
	
	// Twice the signed area for barycentric normalization
	float area = a[0] * V2[0] + b[0] * V2[1] + c[0];
	if (area == 0.0f) return;

	V3 invz = { V0[2], V1[2], V2[2] };

	V3 n0_over_z = N0 * invz[0];
	V3 n1_over_z = N1 * invz[1];
	V3 n2_over_z = N2 * invz[2];

	M33 n_matrix;
	n_matrix.set_column(0, n0_over_z);
	n_matrix.set_column(1, n1_over_z);
	n_matrix.set_column(2, n2_over_z);
	

    for (int v = top; v <= bottom; v++) {
        currEE = currEELS;

        for (int u = left; u <= right; u++) {
            if (currEE[0] >= 0 && currEE[1] >= 0 && currEE[2] >= 0) {
                // Barycentric weights (screen-space)
                float w0 = (a[1] * u + b[1] * v + c[1]) / area;
                float w1 = (a[2] * u + b[2] * v + c[2]) / area;
                float w2 = 1.0f - w0 - w1;
				V3 w = { w0, w1, w2 };

                // Interpolate depth for z-buffer 
				float curr_z = w * invz + .00001f;

                // Perspective-correct interpolate normal vector (model space)
				V3 n = n_matrix * w / curr_z;

                unsigned int color = cube_map->get_color(n.reflected(ppc->C - V3((float)u, (float)v, curr_z)));
				set_with_zb(u, v, color, curr_z);

            }
            currEE += a;
        }
        currEELS += b;
    }
}


unsigned int* FrameBuffer::get_vert_flipped_pixels() {
	unsigned int* flippedPixels = new unsigned int[w * h];
	for (int row = 0; row < h; row++) {
		int src = row * w;
		int dst = (h - 1 - row) * w;
		for (int col = 0; col < w; col++) {
			flippedPixels[dst + col] = pix[src + col];
		}
	}
	return flippedPixels;
}

unsigned int* FrameBuffer::get_vert_and_horiz_flipped_pixels() {
	unsigned int* flippedPixels = new unsigned int[w * h];
	for (int row = 0; row < h; row++) {
		int src = row * w;
		int dst = (h - 1 - row) * w;
		for (int col = 0; col < w; col++) {
			flippedPixels[dst + (w - 1 - col)] = pix[src + col];
		}
	}
	return flippedPixels;
}
