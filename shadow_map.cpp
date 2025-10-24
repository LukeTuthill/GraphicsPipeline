#include "shadow_map.h"
#include "tm.h"
#include <cmath>

ShadowMap::ShadowMap(int _w, int _h, V3 _light_pos) {
	w = _w;
	h = _h;
	pos = _light_pos;
	
	cube_map = new CubeMap(w, h, pos);

	clear();
}

void ShadowMap::set_pos(V3 new_pos) {
	pos = new_pos;
	for (int i = 0; i < 6; i++) {
		cube_map->ppcs[i]->C = pos;
	}
}

void ShadowMap::clear() {
	for (int i = 0; i < 6; i++) {
		cube_map->faces[i]->clear();
	}
}

void ShadowMap::check_and_set_zb(int face_idx, int u, int v, float z) {
	if (z > cube_map->faces[face_idx]->zb[(h - 1 - v) * w + u]) {
		cube_map->faces[face_idx]->zb[(h - 1 - v) * w + u] = z;
	}
}

bool ShadowMap::is_farther(int face_idx, int u, int v, float z) {
	return z < cube_map->faces[face_idx]->zb[(h - 1 - v) * w + u] - .01f;
}

int ShadowMap::get_face_index(V3 P) {
	V3 dir = P - pos;
	float absX = fabs(dir[0]);
	float absY = fabs(dir[1]);
	float absZ = fabs(dir[2]);
	if (absX > absY && absX > absZ) {
		return (dir[0] > 0.0f) ? 0 : 1;
	}
	else if (absY > absX && absY > absZ) {
		return (dir[1] > 0.0f) ? 2 : 3;
	}
	else {
		return (dir[2] > 0.0f) ? 4 : 5;
	}
}

void ShadowMap::project_and_set(V3 P) {
	int face_idx = get_face_index(P);
	V3 PP;
	if (!cube_map->ppcs[face_idx]->project(P, PP)) return;
	int u = (int)PP[0];
	int v = (int)PP[1];
	float z = PP[2];
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1) return;
	check_and_set_zb(face_idx, u, v, z);
}

bool ShadowMap::in_shadow(V3 P) {
	int face_idx = get_face_index(P);
	V3 PP;
	if (!cube_map->ppcs[face_idx]->project(P, PP)) return false;
	int u = (int)PP[0];
	int v = (int)PP[1];
	float z = PP[2];
	if (u < 0 || u > w - 1 || v < 0 || v > h - 1) return false;
	return is_farther(face_idx, u, v, z);
}

void ShadowMap::add_tm(TM* tm) {
	for (int vi = 0; vi < tm->num_verts; vi++) {
		project_and_set(tm->verts[vi]);
	}
}
