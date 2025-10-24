#pragma once

#include "ppc.h"

class FrameBuffer;

class CubeMap {
public:
	FrameBuffer* faces[6];
	PPC* ppcs[6];
	int prev_face;

	//Constructor for environment cube map, loads from one tiff image
	CubeMap(char* fname);

	//Constructor for shadow map cube map
	CubeMap(int w, int h, V3 light_pos);

	unsigned int get_color(V3 dir);

	void render_as_environment(PPC* ppc, FrameBuffer* fb);

private:
	void initialize(int w, int h, V3 pos);
};

