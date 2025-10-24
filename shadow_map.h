#pragma once

#include "ppc.h"
#include "cube_map.h"

class TM; // Forward declaration

class ShadowMap {
public:
	int w, h;
	CubeMap* cube_map; // 6 faces
	V3 pos;

	ShadowMap(int _w, int _h, V3 _light_pos);

	void set_pos(V3 new_pos);

	void clear(); // Clear all shadow maps
	void check_and_set_zb(int face_idx, int u, int v, float z);
	bool is_farther(int face_idx, int u, int v, float z);

	void project_and_set(V3 P); // Project point P onto the correct face
	bool in_shadow(V3 P); // Check if point P is in shadow on any face

	void add_tm(TM* tm);
	int get_face_index(V3 P);
};
