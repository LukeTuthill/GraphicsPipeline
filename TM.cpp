#define _USE_MATH_DEFINES
#include <cmath>

#include "tm.h"
#include "shadow_map.h"

#include <fstream>
#include <iostream>

using namespace std;

TM::TM(char* fname) : TM() {
	load_bin(fname);
}

TM::TM(V3 center, float radius, float height, int _num_verts, unsigned int color) : TM() {
	num_verts = _num_verts - _num_verts % 2; //Ensures num_verts is even

	int tris_per_circle = num_verts / 2 - 1;

	//2 Circles, two triangles on the height for each two triangles on opposite circles
	num_tris = 4 * tris_per_circle; 

	verts = new V3[num_verts];
	projected_verts = new V3[num_verts];

	tris = new unsigned int[num_tris * 3];

	colors = new V3[num_verts];
	lighted_colors = new V3[num_verts];

	normals = new V3[num_verts];

	V3 color_vector;
	color_vector.set_as_color(color);

	// Create vertices
	for (int vi = 0; vi < num_verts / 2 - 1; vi++) {
		float angle = (float)vi / (float)(num_verts / 2 - 1) * 2.0f * (float)M_PI;

		//Bottom circle
		verts[vi] = V3(center[0] + radius * cos(angle), center[1] - height / 2.0f, center[2] + radius * sin(angle));
		normals[vi] = V3(0.0f, -1.0f, 0.0f);

		//Top circle
		verts[vi + num_verts / 2] = V3(center[0] + radius * cos(angle), center[1] + height / 2.0f, center[2] + radius * sin(angle));
		normals[vi + num_verts / 2] = V3(0.0f, 1.0f, 0.0f);
	
		colors[vi] = color_vector;
		colors[vi + num_verts / 2] = color_vector;
	}

	verts[num_verts / 2 - 1] = V3(center[0], center[1] - height / 2.0f, center[2]); // Center vertex of bottom circle
	verts[num_verts - 1] = V3(center[0], center[1] + height / 2.0f, center[2]); // Center vertex of top circle

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

TM::TM(V3 p1, V3 p2, unsigned int color) : TM(){
	set_as_plane(p1, p2, color);
}

void TM::set_tex(FrameBuffer* tex, V3* tcs) {
	this->tex = tex;
	this->tcs = tcs;
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

void TM::create_face(V3 origin, V3 u_dir, V3 v_dir, int u_steps, int v_steps, V3 normal,
    const V3& color_vector, int& v_idx, int& t_idx) {
    int u_verts = u_steps + 1;
    int v_verts = v_steps + 1;
    int base_v_idx = v_idx;

    for (int j = 0; j < v_verts; ++j) {
        for (int i = 0; i < u_verts; ++i) {
            verts[v_idx] = origin + u_dir * (float)i + v_dir * (float)j;
            normals[v_idx] = normal;
            colors[v_idx] = color_vector;
            v_idx++;
        }
    }

    for (int j = 0; j < v_steps; ++j) {
        for (int i = 0; i < u_steps; ++i) {
            unsigned int p00 = base_v_idx + j * u_verts + i;
            unsigned int p10 = base_v_idx + j * u_verts + i + 1;
            unsigned int p01 = base_v_idx + (j + 1) * u_verts + i;
            unsigned int p11 = base_v_idx + (j + 1) * u_verts + i + 1;

            tris[t_idx * 3 + 0] = p00;
            tris[t_idx * 3 + 1] = p10;
            tris[t_idx * 3 + 2] = p11;
            t_idx++;

            tris[t_idx * 3 + 0] = p00;
            tris[t_idx * 3 + 1] = p11;
            tris[t_idx * 3 + 2] = p01;
            t_idx++;
        }
    }
}

void TM::set_as_box(V3 p1, V3 p2, unsigned int color) {
	V3 min_p(fmin(p1[0], p2[0]), fmin(p1[1], p2[1]), fmin(p1[2], p2[2]));
	V3 max_p(fmax(p1[0], p2[0]), fmax(p1[1], p2[1]), fmax(p1[2], p2[2]));

	int x_steps = (int)(ceil(abs(max_p[0] - min_p[0]) / 2.0f));
	int y_steps = (int)(ceil(abs(max_p[1] - min_p[1]) / 2.0f));
	int z_steps = (int)(ceil(abs(max_p[2] - min_p[2]) / 2.0f));

	x_steps = max(1, x_steps);
	y_steps = max(1, y_steps);
	z_steps = max(1, z_steps);

	int x_verts = x_steps + 1;
	int y_verts = y_steps + 1;
	int z_verts = z_steps + 1;

	num_verts = 2 * (x_verts * y_verts + x_verts * z_verts + y_verts * z_verts);
	num_tris = 2 * (x_steps * y_steps + x_steps * z_steps + y_steps * z_steps) * 2;

	verts = new V3[num_verts];
	projected_verts = new V3[num_verts];
	normals = new V3[num_verts];
	colors = new V3[num_verts];
	lighted_colors = new V3[num_verts];
	tris = new unsigned int[num_tris * 3];

	V3 color_vector;
	color_vector.set_as_color(color);

	int v_idx = 0;
	int t_idx = 0;

	V3 dx((max_p[0] - min_p[0]) / x_steps, 0, 0);
	V3 dy(0, (max_p[1] - min_p[1]) / y_steps, 0);
	V3 dz(0, 0, (max_p[2] - min_p[2]) / z_steps);

	// Front face
	create_face(min_p, dx, dy, x_steps, y_steps, V3(0, 0, -1), color_vector, v_idx, t_idx);
	// Back face
	create_face(V3(min_p[0], min_p[1], max_p[2]), dx, dy, x_steps, y_steps, V3(0, 0, 1), color_vector, v_idx, t_idx);
	// Bottom face
	create_face(min_p, dx, dz, x_steps, z_steps, V3(0, -1, 0), color_vector, v_idx, t_idx);
	// Top face
	create_face(V3(min_p[0], max_p[1], min_p[2]), dx, dz, x_steps, z_steps, V3(0, 1, 0), color_vector, v_idx, t_idx);
	// Left face
	create_face(min_p, dy, dz, y_steps, z_steps, V3(-1, 0, 0), color_vector, v_idx, t_idx);
	// Right face
	create_face(V3(max_p[0], min_p[1], min_p[2]), dy, dz, y_steps, z_steps, V3(1, 0, 0), color_vector, v_idx, t_idx);
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

void TM::set_as_quad(V3 p1, V3 p2, V3 p3, V3 p4, unsigned int color) {
	num_verts = 4;
	num_tris = 2;

	verts = new V3[num_verts];
	projected_verts = new V3[num_verts];
	normals = new V3[num_verts];
	colors = new V3[num_verts];
	lighted_colors = new V3[num_verts];
	tris = new unsigned int[num_tris * 3];

	V3 color_vector;
	color_vector.set_as_color(color);

	verts[0] = p1;
	verts[1] = p2;
	verts[2] = p3;
	verts[3] = p4;

	V3 normal = ((p2 - p1) ^ (p3 - p1)).normalized();

	for (int vi = 0; vi < num_verts; vi++) {
		normals[vi] = normal;
		colors[vi] = color_vector;
	}

	tris[0] = 0;
	tris[1] = 1;
	tris[2] = 2;
	tris[3] = 1;
	tris[4] = 2;
	tris[5] = 3;
}

void TM::set_as_plane(V3 p1, V3 p2, unsigned int color) {
	V3 min_p(fmin(p1[0], p2[0]), p1[1], fmin(p1[2], p2[2]));
	V3 max_p(fmax(p1[0], p2[0]), p1[1], fmax(p1[2], p2[2]));

	int x_steps = (int)(ceil(abs(max_p[0] - min_p[0]) / 2.0f));
	int z_steps = (int)(ceil(abs(max_p[2] - min_p[2]) / 2.0f));

	x_steps = max(1, x_steps);
	z_steps = max(1, z_steps);

	int x_verts = x_steps + 1;
	int z_verts = z_steps + 1;

	num_verts = x_verts * z_verts;
	num_tris = x_steps * z_steps * 2;

	verts = new V3[num_verts];
	projected_verts = new V3[num_verts];
	normals = new V3[num_verts];
	colors = new V3[num_verts];
	lighted_colors = new V3[num_verts];
	tris = new unsigned int[num_tris * 3];

	V3 color_vector;
	color_vector.set_as_color(color);

	float x_len = max_p[0] - min_p[0];
	float z_len = max_p[2] - min_p[2];

	for (int j = 0; j < z_verts; ++j) {
		for (int i = 0; i < x_verts; ++i) {
			float x = min_p[0] + (x_len / x_steps) * i;
			float z = min_p[2] + (z_len / z_steps) * j;
			int idx = j * x_verts + i;
			verts[idx] = V3(x, min_p[1], z);
			normals[idx] = V3(0.0f, 1.0f, 0.0f);
			colors[idx] = color_vector;
		}
	}

	int t_idx = 0;
	for (int j = 0; j < z_steps; ++j) {
		for (int i = 0; i < x_steps; ++i) {
			unsigned int p00 = j * x_verts + i;
			unsigned int p10 = j * x_verts + i + 1;
			unsigned int p01 = (j + 1) * x_verts + i;
			unsigned int p11 = (j + 1) * x_verts + i + 1;

			tris[t_idx * 3 + 0] = p00;
			tris[t_idx * 3 + 1] = p10;
			tris[t_idx * 3 + 2] = p01;
			t_idx++;

			tris[t_idx * 3 + 0] = p10;
			tris[t_idx * 3 + 1] = p11;
			tris[t_idx * 3 + 2] = p01;
			t_idx++;
		}
	}
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

void TM::rasterize(PPC* ppc, FrameBuffer* fb, CubeMap* cube_map, render_type rt) {
	for (int vi = 0; vi < num_verts; vi++) {
		ppc->project(verts[vi], projected_verts[vi]);
	}

	for (int ti = 0; ti < num_tris; ti++) {
		int v0 = tris[ti * 3 + 0];
		int v1 = tris[ti * 3 + 1];
		int v2 = tris[ti * 3 + 2];
		V3 V0 = projected_verts[v0];
		V3 V1 = projected_verts[v1];
		V3 V2 = projected_verts[v2];

		if (V0[0] == FLT_MAX || V1[0] == FLT_MAX || V2[0] == FLT_MAX) // Behind the camera
			continue;

		if (rt == render_type::MIRROR_ONLY) {
			fb->draw_2d_mirrored_triangle(V0, V1, V2, normals[v0], normals[v1], normals[v2], ppc, cube_map);
			continue;
		}

		if (tex && (rt == render_type::NORMAL_TILING_TEXTURED || rt == render_type::MIRRORED_TILING_TEXTURED)) {
			V3 tex0 = tcs[v0];
			V3 tex1 = tcs[v1];
			V3 tex2 = tcs[v2];
			fb->draw_2d_texture_triangle(V0, V1, V2, tex0, tex1, tex2, rt == render_type::MIRRORED_TILING_TEXTURED, tex);
			continue;
		}

		V3 C0, C1, C2;
		if (rt == render_type::LIGHTED && lighted_colors) {
			C0 = lighted_colors[v0];
			C1 = lighted_colors[v1];
			C2 = lighted_colors[v2];
		}
		else {
			C0 = colors[v0];
			C1 = colors[v1];
			C2 = colors[v2];
		}

		fb->draw_2d_triangle(V0, V1, V2, C0, C1, C2);
	}
}

void TM::set_eeqs(M33 proj_verts, M33& eeqs) {

}

void TM::visualize_normals(float nl, PPC* ppc, FrameBuffer* fb) {
	if (!normals)
		return;

	for (int vi = 0; vi < num_verts; vi++) {
		fb->draw_3d_segment(verts[vi], verts[vi] + normals[vi].normalized() * nl, 
			V3(1, 0, 0), V3(1, 0, 0), ppc);
	}
}

void TM::light_point(ShadowMap* shadow_map, V3 eye_pos, float ka, int specular_exp) {
	if (!lighted_colors)
		lighted_colors = new V3[num_verts];

	for (int vi = 0; vi < num_verts; vi++) {
		if (shadow_map->in_shadow(verts[vi])) {
			lighted_colors[vi] = colors[vi] * ka; // ambient only
			continue;
		}
		V3 ld = shadow_map->pos - verts[vi];
		V3 view_dir = eye_pos - verts[vi];
		lighted_colors[vi] = colors[vi].lighted(normals[vi], ld, view_dir, ka, specular_exp);
	}
}

void TM::light_directional(ShadowMap* shadow_map, V3 eye_pos, float ka, int specular_exp) {
	if (!lighted_colors)
		lighted_colors = new V3[num_verts];

	for (int vi = 0; vi < num_verts; vi++) {
		if (shadow_map->in_shadow(verts[vi])) {
			lighted_colors[vi] = colors[vi] * ka; // ambient only
			continue;
		}

		lighted_colors[vi] = colors[vi].lighted(normals[vi], shadow_map->pos, eye_pos, ka, specular_exp);
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
	if (num_verts <= 0) {
		cerr << "INFO: file " << fname << " has no vertices" << endl;
		ifs.close();
		return;
	}
	char yn;
	ifs.read(&yn, 1); // always xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	/*if (verts)
		delete[] verts;*/
	verts = new V3[num_verts];
	
	/*if(projected_verts)
		delete[] projected_verts;*/
	projected_verts = new V3[num_verts];

	ifs.read(&yn, 1); // cols 3 floats
	/*if (colors)
		delete[] colors;*/
	colors = 0;
	if (yn == 'y') {
		colors = new V3[num_verts];
	}
	
	/*if (lighted_colors)
		delete[] lighted_colors;*/
	lighted_colors = nullptr;

	ifs.read(&yn, 1); // normals 3 floats
	/*if (normals)
		delete []normals;*/
	normals = 0;
	if (yn == 'y') {
		normals = new V3[num_verts];
	}

	ifs.read(&yn, 1); // texture coordinates 2 floats
	float* tcs = 0;
	/*if (tcs)
		delete []tcs;*/
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
	/*if (tris)
		delete tris;*/
	tris = new unsigned int[num_tris * 3];
	ifs.read((char*)tris, num_tris * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close();

	cerr << "INFO: loaded " << num_verts << " verts, " << num_tris << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((colors) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

}
