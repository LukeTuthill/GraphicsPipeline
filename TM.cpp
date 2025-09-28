#define _USE_MATH_DEFINES
#include <cmath>

#include "tm.h"
#include <fstream>
#include <iostream>

using namespace std;

TM::TM(char* fname) {
	load_bin(fname);
}

TM::TM(V3 center, float radius, float height, int _num_verts, unsigned int color) {
	num_verts = _num_verts - _num_verts % 2; //Ensures num_verts is even

	int tris_per_circle = num_verts / 2 - 1;

	//2 Circles, two triangles on the height for each two triangles on opposite circles
	num_tris = 4 * tris_per_circle; 

	verts = new V3[num_verts];

	tris = new unsigned int[num_tris * 3];

	colors = new V3[num_verts];
	V3 color_vector;
	color_vector.set_as_color(color);

	// Create vertices
	for (int vi = 0; vi < num_verts / 2 - 1; vi++) {
		float angle = (float)vi / (float)(num_verts / 2 - 1) * 2.0f * (float)M_PI;

		//Bottom circle
		verts[vi] = V3(center[0] + radius * cos(angle), center[1] + radius * sin(angle), center[2] - height / 2.0f);

		//Top circle
		verts[vi + num_verts / 2] = V3(center[0] + radius * cos(angle), center[1] + radius * sin(angle), center[2] + height / 2.0f);
	
		colors[vi] = color_vector;
		colors[vi + num_verts / 2] = color_vector;
	}

	verts[num_verts / 2 - 1] = V3(center[0], center[1], center[2] - height / 2.0f); // Center vertex of bottom circle
	verts[num_verts - 1] = V3(center[0], center[1], center[2] + height / 2.0f); // Center vertex of top circle

	colors[num_verts / 2 - 1] = color_vector;
	colors[num_verts - 1] = color_vector;

	// Create triangles
	for (unsigned int ti = 0; ti < (unsigned int)tris_per_circle; ti++) {
		// Bottom circle
		tris[ti * 3 + 0] = ti;
		tris[ti * 3 + 1] = (ti + 1) % (num_verts / 2 - 1);
		tris[ti * 3 + 2] = num_verts / 2 - 1; // Center vertex of bottom circle

		// Top circle
		tris[(ti + tris_per_circle) * 3 + 0] = ti + num_verts / 2;
		tris[(ti + tris_per_circle) * 3 + 1] = ((ti + 1) % (num_verts / 2 - 1)) + num_verts / 2;
		tris[(ti + tris_per_circle) * 3 + 2] = num_verts - 1; // Center vertex of top circle

		// Side triangles
		tris[(ti + 2 * tris_per_circle) * 3 + 0] = ti;
		tris[(ti + 2 * tris_per_circle) * 3 + 1] = (ti + 1) % (num_verts / 2 - 1);
		tris[(ti + 2 * tris_per_circle) * 3 + 2] = ti + num_verts / 2;
		tris[(ti + 3 * tris_per_circle) * 3 + 0] = (ti + 1) % (num_verts / 2 - 1);
		tris[(ti + 3 * tris_per_circle) * 3 + 1] = ((ti + 1) % (num_verts / 2 - 1)) + num_verts / 2;
		tris[(ti + 3 * tris_per_circle) * 3 + 2] = ti + num_verts / 2;
	}
}

void TM::draw_points(unsigned int color, int psize, PPC *ppc, 
	FrameBuffer *fb) {
	for (int vi = 0; vi < num_verts; vi++) {
		fb->draw_3d_point(verts[vi], ppc, psize, color);
	}
}

V3 TM::get_center() {
	V3 ret(0.0f, 0.0f, 0.0f);
	for (int vi = 0; vi < num_verts; vi++) {
		ret = ret + verts[vi];
	}
	return ret / (float) num_verts;
}

void TM::rotate_about_arbitrary_axis(V3 aO, V3 ad, float theta) {
	for (int vi = 0; vi < num_verts; vi++) {
		verts[vi] = verts[vi].rotate_point(aO, ad, theta);
		normals[vi] = normals[vi].rotate_direction(ad, theta);
	}
}

void TM::set_as_box(V3 p1, V3 p2, unsigned int color) {
	num_verts = 8;
	num_tris = 12;

	if (verts)
		delete[] verts;
	verts = new V3[num_verts];
	
	if (colors)
		delete[] colors;
	colors = new V3[num_verts];

	if (tris)
		delete[] tris;
	tris = new unsigned int[num_tris * 3];

	verts[0] = V3(p1[0], p1[1], p1[2]);
	verts[1] = V3(p1[0], p1[1], p2[2]);
	verts[2] = V3(p1[0], p2[1], p1[2]);
	verts[3] = V3(p1[0], p2[1], p2[2]);
	verts[4] = V3(p2[0], p1[1], p1[2]);
	verts[5] = V3(p2[0], p1[1], p2[2]);
	verts[6] = V3(p2[0], p2[1], p1[2]);
	verts[7] = V3(p2[0], p2[1], p2[2]);

	// front face
	tris[0] = 0; tris[1] = 1; tris[2] = 2;
	tris[3] = 2; tris[4] = 1; tris[5] = 3;

	// back face
	tris[6] = 4; tris[7] = 5; tris[8] = 6;
	tris[9] = 6; tris[10] = 5; tris[11] = 7;

	// left face
	tris[12] = 0; tris[13] = 2; tris[14] = 4;
	tris[15] = 4; tris[16] = 2; tris[17] = 6;

	// right face
	tris[18] = 1; tris[19] = 3; tris[20] = 5;
	tris[21] = 5; tris[22] = 3; tris[23] = 7;

	// top face
	tris[24] = 2; tris[25] = 3; tris[26] = 6;
	tris[27] = 6; tris[28] = 3; tris[29] = 7;

	// bottom face
	tris[30] = 0; tris[31] = 4; tris[32] = 1;
	tris[33] = 1; tris[34] = 4; tris[35] = 5;

	V3 color_vector;
	color_vector.set_as_color(color);
	for (int vi = 0; vi < num_verts; vi++) {
		colors[vi] = color_vector;
	}
}

void TM::get_bounding_box(V3& p1, V3& p2) {
	V3 min = verts[0];
	V3 max = verts[0];

	for (int vi = 0; vi < num_verts; vi++) {
		V3 v = verts[vi];
		if (v[0] < min[0]) min[0] = v[0];
		if (v[1] < min[1]) min[1] = v[1];
		if (v[2] < min[2]) min[2] = v[2];

		if (v[0] > max[0]) max[0] = v[0];
		if (v[1] > max[1]) max[1] = v[1];
		if (v[2] > max[2]) max[2] = v[2];
	}

	p1[0] = min[0];
	p1[1] = min[1];
	p1[2] = min[2];

	p2[0] = max[0];
	p2[1] = max[1];
	p2[2] = max[2];
}

void TM::translate(V3 tv) {
	for (int vi = 0; vi < num_verts; vi++) {
		verts[vi] = verts[vi] + tv;
	}
}

void TM::position(V3 new_center) {
	translate(new_center - get_center());
}

void TM::scale(float s) {
	V3 center = get_center();
	for (int vi = 0; vi < num_verts; vi++) {
		verts[vi] = center + s * (verts[vi] - center);
	}
}

void TM::render_as_wireframe(PPC* ppc, FrameBuffer* fb, bool is_lighted) {
	for (int ti = 0; ti < num_tris; ti++) {
		int v0 = tris[ti * 3 + 0];
		int v1 = tris[ti * 3 + 1];
		int v2 = tris[ti * 3 + 2];
		V3 V0 = verts[v0];
		V3 V1 = verts[v1];
		V3 V2 = verts[v2];
		
		V3 C0, C1, C2;
		if (is_lighted && lighted_colors) {
			C0 = lighted_colors[v0];
			C1 = lighted_colors[v1];
			C2 = lighted_colors[v2];
		}
		else {
			C0 = colors[v0];
			C1 = colors[v1];
			C2 = colors[v2];
		}

		fb->draw_3d_segment(V0, V1, C0, C1, ppc);
		fb->draw_3d_segment(V1, V2, C1, C2, ppc);
		fb->draw_3d_segment(V2, V0, C2, C0, ppc);
	}
}

void TM::rasterize(PPC* ppc, FrameBuffer* fb, bool is_lighted) {
	for (int ti = 0; ti < num_tris; ti++) {
		int v0 = tris[ti * 3 + 0];
		int v1 = tris[ti * 3 + 1];
		int v2 = tris[ti * 3 + 2];
		V3 V0 = verts[v0];
		V3 V1 = verts[v1];
		V3 V2 = verts[v2];

		V3 C0, C1, C2;
		if (is_lighted && lighted_colors) {
			C0 = lighted_colors[v0];
			C1 = lighted_colors[v1];
			C2 = lighted_colors[v2];
		}
		else {
			C0 = colors[v0];
			C1 = colors[v1];
			C2 = colors[v2];
		}

		fb->draw_3d_triangle(V0, V1, V2, C0, C1, C2, ppc);
	}
}

void TM::light_directional(V3 ld, float ka) {
	ld = ld.normalized();

	if (!lighted_colors)
		lighted_colors = new V3[num_verts];

	for (int vi = 0; vi < num_verts; vi++) {
		lighted_colors[vi] = colors[vi].lighted(normals[vi], ld, ka);
	}
}

void TM::visualize_normals(float nl, PPC* ppc, FrameBuffer* fb) {

	if (!normals)
		return;

	for (int vi = 0; vi < num_verts; vi++) {
		fb->draw_3d_segment(verts[vi], verts[vi] + normals[vi].normalized() * nl, 
			V3(1, 0, 0), V3(1, 0, 0), ppc);
	}

}

void TM::light_point(V3 l, float ka) {
	if (!lighted_colors)
		lighted_colors = new V3[num_verts];
	
	for (int vi = 0; vi < num_verts; vi++) {
		V3 ld = (l - verts[vi]).normalized();
		lighted_colors[vi] = colors[vi].lighted(normals[vi], ld, ka);
	}
}

// loading triangle mesh from a binary file, i.e., a .bin file from geometry folder
void TM::load_bin(char *fname) {
	ifstream ifs(fname, ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open file: " << fname << endl;
		return;
	}

	ifs.read((char*)&num_verts, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	if (verts)
		delete[] verts;
	verts = new V3[num_verts];

	ifs.read(&yn, 1); // cols 3 floats
	if (colors)
		delete[] colors;
	colors = 0;
	if (yn == 'y') {
		colors = new V3[num_verts];
	}
	
	if (lighted_colors)
		delete[] lighted_colors;
	lighted_colors = nullptr;

	ifs.read(&yn, 1); // normals 3 floats
	if (normals)
		delete []normals;
	normals = 0;
	if (yn == 'y') {
		normals = new V3[num_verts];
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	float *tcs = 0; // don't have texture coordinates for now
	if (tcs)
		delete []tcs;
	tcs = 0;
	if (yn == 'y') {
		tcs = new float[num_verts * 2];
	}


	ifs.read((char*)verts, num_verts * 3 * sizeof(float)); // load verts

	if (colors) {
		ifs.read((char*)colors, num_verts * 3 * sizeof(float)); // load colors
	}

	if (normals)
		ifs.read((char*)normals, num_verts * 3 * sizeof(float)); // load normals

	if (tcs)
		ifs.read((char*)tcs, num_verts * 2 * sizeof(float)); // load texture coordinates

	ifs.read((char*)&num_tris, sizeof(int));
	if (tris)
		delete tris;
	tris = new unsigned int[num_tris * 3];
	ifs.read((char*)tris, num_tris * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << num_verts << " verts, " << num_tris << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

}
