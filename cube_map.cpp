#include "cube_map.h"
#include "tiffio.h"
#include "framebuffer.h"

using namespace std;

CubeMap::CubeMap(char* fname) {
	//Only used to load tiff and set faces using FrameBuffer methods
	FrameBuffer temp(0, 0, 1, 1);
	temp.load_tiff(fname);

	/* Assumes this layout for the faces in the image
		[ ][0][ ]
		[1][2][3]
		[ ][4][ ]
		[ ][5][ ]
	*/

	int face_width = temp.w / 3;
	if (temp.h / 4 != face_width) {
		throw new logic_error("Width of faces doesn't equal height of faces");
	}
	
	initialize(face_width, face_width, V3(0.0f, 0.0f, 0.0f));

	for (int u = 0; u < face_width; u++) {
		for (int v = 0; v < face_width; v++) {
			faces[2]->set(u, v, temp.get(u + face_width, v));
			faces[1]->set(u, v, temp.get(u, v + face_width));
			faces[5]->set(u, v, temp.get(u + face_width, v + face_width));
			faces[0]->set(u, v, temp.get(u + 2 * face_width, v + face_width));
			faces[3]->set(u, v, temp.get(u + face_width, v + 2 * face_width));
			faces[4]->set(u, face_width - 1 - v, temp.get(u + face_width, v + 3 * face_width));
		}
	}
}

CubeMap::CubeMap(int w, int h, V3 light_pos) {
	initialize(w, h, light_pos);
}

void CubeMap::initialize(int w, int h, V3 pos) {
	prev_face = 0;

	for (int i = 0; i < 6; i++) {
		ppcs[i] = new PPC(90.0f, w, h);
		ppcs[i]->C = pos;
		faces[i] = new FrameBuffer(0, 0, w, h);
		faces[i]->clear();
	}

	// -Z (face 5): Default orientation, no rotation needed
	
	// +Z (face 4): Pan 180 degrees
	ppcs[4]->pan(180.0f);

	// +X (face 0): Pan 90 degrees
	ppcs[0]->pan(90.0f);

	// -X (face 1): Pan -90 degrees
	ppcs[1]->pan(-90.0f);

	// +Y (face 2): Tilt 90 degrees 
	ppcs[2]->tilt(90.0f);

	// -Y (face 3): Tilt -90 degrees
	ppcs[3]->tilt(-90.0f);
}

unsigned int CubeMap::get_color(V3 dir) {
	for (int i = 0; i < 6; i++) {
		int idx = (prev_face + i) % 6; //Start checking from previous face for speed
		V3 dir_in_camera_space = dir - ppcs[idx]->C;
		V3 PP;

		if(ppcs[idx]->project(dir_in_camera_space, PP)) {
			float x = PP[0];
			float y = PP[1];
			
			int w = faces[idx]->w;
			int h = faces[idx]->h;

			if (x < 0.0f || y < 0.0f || x >= (float)(w) || y >= (float)(h)) {
				continue;
			}

			prev_face = idx;

			//Bilinear Interpolation
			int u0 = (int)x;
			int v0 = (int)y;
			
			int u1 = min(u0 + 1, w - 1);
			int v1 = min(v0 + 1, h - 1);

			float du = x - (float)u0;
			float dv = y - (float)v0;

			float d00 = (1.0f - du) * (1.0f - dv);
			float d10 = du * (1.0f - dv);
			float d01 = (1.0f - du) * dv;
			float d11 = du * dv;

			V3 c00, c01, c10, c11;

			c00.set_as_color(faces[idx]->get(u0, v0));
			c10.set_as_color(faces[idx]->get(u1, v0));
			c01.set_as_color(faces[idx]->get(u0, v1));
			c11.set_as_color(faces[idx]->get(u1, v1));

			V3 color = c00 * d00 + c10 * d10 + c01 * d01 + c11 * d11;
			return color.convert_to_color_int();
		}
	}
	return 0xFF0000FF; //Should never reach here
}

void CubeMap::render_as_environment(PPC* ppc, FrameBuffer* fb) {
	for (int u = 0; u < fb->w; u++) {
		for (int v = 0; v < fb->h; v++) {
			if (fb->get_zb(u, v) != 0.0f) {
				continue;
			}
			V3 dir = ppc->c + (float)u * ppc->a + (float)v * ppc->b;
			unsigned int color = get_color(dir);
			fb->set(u, v, color);
		}
	}
}

