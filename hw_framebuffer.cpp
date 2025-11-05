#include "hw_framebuffer.h"
#include "scene.h"

#include <FL/fl_draw.H>
#include <iostream>

using namespace std;

HWFrameBuffer::HWFrameBuffer(int u0, int v0, int _w, int _h) : FrameBuffer(u0, v0, _w, _h) {
	initialized = false;
	has_textures = false;
	render_wireframe = false;
	ppc = nullptr;
	tms = nullptr;
	num_tms = 0;
	tm_is_mirror = nullptr;

	shadow_fb = 0;
	shadow_map = 0;
	shadow_map_size = 512;
	shadows_enabled = false;

	use_lighting = false;
	light_ppc = nullptr;
	light_pos = V3(0.0f, 100.0f, 0.0f);

	env_map = nullptr;
	env_cube_map_id = 0;
	environment_map_enabled = false;

	last_frame_time = std::chrono::steady_clock::now();
}

void HWFrameBuffer::init() {
	//Enables z-buffering
	glEnable(GL_DEPTH_TEST);

	//Only initialize textures and sets flag if there are any
	init_textures();

	//if (shadows_enabled)
	//	init_shadow_map();

	if (environment_map_enabled)
		init_environment_map();

	if (use_lighting)
		init_lighting();

	initialized = true; 
}

void HWFrameBuffer::init_lighting() {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_shininess[] = { 50.0f };

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

	// Enable color material so vertex colors work with lighting
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	//Auto normalization of normals
	glEnable(GL_NORMALIZE);
}

void HWFrameBuffer::init_textures() {
	glActiveTexture(GL_TEXTURE0);

	for (int tmi = 0; tmi < num_tms; tmi++) {
		TM* tm = &tms[tmi];

		if (!tm->tex) 
			continue;

		has_textures = true;
		glEnable(GL_TEXTURE_2D);

		glGenTextures(1, &tm->tex_id);
		glBindTexture(GL_TEXTURE_2D, tm->tex_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		unsigned char* pixelData = (unsigned char *)tm->tex->get_vert_flipped_pixels();

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tm->tex->w, tm->tex->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
		glBindTexture(GL_TEXTURE_2D, 0); //Unbind texture

		delete[](pixelData);
	}
	glDisable(GL_TEXTURE_2D);
}

void HWFrameBuffer::init_shadow_map() {
	// Generate framebuffer for shadow map
	glGenFramebuffers(1, &shadow_fb);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fb);

	// Create depth texture
	glGenTextures(1, &shadow_map);
	glBindTexture(GL_TEXTURE_2D, shadow_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		shadow_map_size, shadow_map_size,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Compare mode for shadow mapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// Attach depth texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, shadow_map, 0);

	// No color buffer needed for shadow map
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create a PPC for the light's perspective
	if (light_ppc) delete light_ppc;
	light_ppc = new PPC(90.0f, shadow_map_size, shadow_map_size);

	light_ppc->pose(light_pos, scene->tms[0].get_center(), V3(0.0f, 1.0f, 0.0f));
}

void HWFrameBuffer::init_environment_map() {
	if (!env_map || !environment_map_enabled)
		return;

	glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures(1, &env_cube_map_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cube_map_id);

	GLenum faces[6] = {
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, // face 0
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // face 1
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, // face 2
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // face 3
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // face 4
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  // face 5
	};

	for (int i = 0; i < 6; i++) {
		FrameBuffer* face = env_map->faces[i];
		unsigned char* pixelData;
		if (i == 2 || i == 3)
			pixelData = (unsigned char*)face->pix;
		else
			pixelData = (unsigned char*)face->get_vert_and_horiz_flipped_pixels();

		glTexImage2D(faces[i], 0, GL_RGBA,
			face->w, face->h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

		if (i != 2 && i != 3)
			delete[] pixelData;
	}

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void HWFrameBuffer::set_tms(TM* tms, int num_tms) {
	this->tms = tms;
	this->num_tms = num_tms;
	if (tm_is_mirror)
		delete[] tm_is_mirror;
	tm_is_mirror = new bool[num_tms];

	initialized = false;
}

void HWFrameBuffer::set_lighting(V3 light_pos) {
	this->light_pos = light_pos;
	initialized = false;
	use_lighting = true;
}

void HWFrameBuffer::set_shadow_map(V3 light_pos, int shadow_map_size) {
	this->light_pos = light_pos;
	this->shadow_map_size = shadow_map_size;
	initialized = false;
	shadows_enabled = true;
}

void HWFrameBuffer::set_environment_map(CubeMap* cube_map) {
	env_map = cube_map;
	initialized = false;
	environment_map_enabled = true;
}

void HWFrameBuffer::move_light(V3 new_pos) {
	if (!shadows_enabled)
		return;
	this->light_pos = new_pos;
	light_ppc->pose(light_pos, scene->tms[0].get_center(), V3(0.0f, 1.0f, 0.0f));
}

void HWFrameBuffer::set_intrinsics(PPC* ppc) {
	int w = ppc->w;
	int h = ppc->h;

	float nearz = 0.1f;
	float farz = 1000.0f;

	float scalef = nearz / ppc->get_focal_length();
	float wf = ppc->a.length()* (float)w * scalef;
	float hf = ppc->b.length()* (float)h * scalef;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-wf / 2.0f, wf / 2.0f, -hf / 2.0f, hf / 2.0f, nearz, farz);
	glMatrixMode(GL_MODELVIEW);  // default matrix mode
}

void HWFrameBuffer::set_extrinsics(PPC* ppc) {
	V3 C = ppc->C;
	V3 b = ppc->b;
	V3 lap = C + ppc->get_vd();

	glLoadIdentity();
	gluLookAt(C[0], C[1], C[2], lap[0], lap[1], lap[2], -b[0], -b[1], -b[2]);
}

void HWFrameBuffer::draw() {
	make_current();

	if (!valid()) {
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK) {
			fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
			return;
		}
	}

	if (!initialized)
		init();

	if (use_lighting) {
		// Update light position
		GLfloat light_position[] = { light_pos[0], light_pos[1], light_pos[2], 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	}

	/*if (shadows_enabled)
		render_shadow_map();*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	/*if (shadows_enabled)
		render_shadows();*/

	// set the view
	set_intrinsics(ppc);
	set_extrinsics(ppc);

	if (environment_map_enabled)
		render_environment_map();


	// iterate over all triangle meshes and draw current triangle mesh
	for (int tmi = 0; tmi < num_tms; tmi++) {
		render(&tms[tmi], tm_is_mirror[tmi]);
	}

	//if (shadows_enabled) {
	//	glDisable(GL_ALPHA_TEST);
	//	glDisable(GL_TEXTURE_GEN_S);
	//	glDisable(GL_TEXTURE_GEN_T);
	//	glDisable(GL_TEXTURE_GEN_R);
	//	glDisable(GL_TEXTURE_GEN_Q);
	//	glActiveTexture(GL_TEXTURE7);
	//	glDisable(GL_TEXTURE_2D);
	//	glActiveTexture(GL_TEXTURE0);
	//}

	render_fps_counter();

	if (use_lighting)
		visualize_point_light();
}

void HWFrameBuffer::render(TM* tm, bool is_mirror) {
	//Inline check for wireframe or filled in rendering
	glPolygonMode(GL_FRONT_AND_BACK, render_wireframe ? GL_LINE : GL_FILL);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (float*)tm->verts);

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0, (float*)tm->normals);

	//Mirrored surface rendering
	if (is_mirror && environment_map_enabled && env_cube_map_id != 0) {
		glEnable(GL_TEXTURE_CUBE_MAP);
		glBindTexture(GL_TEXTURE_CUBE_MAP, env_cube_map_id);

		// Enable automatic reflection texture coordinate generation
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);

		// Set the texture generation mode to reflection mapping
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
	}
	//Textured surface rendering
	else if (has_textures && tm->tex) {
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tm->tex_id);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(V3), (float*)tm->tcs);
	}
	//Standard color interpolation rendering
	else {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(3, GL_FLOAT, 0, (float*)tm->colors);
	}

	glDrawElements(GL_TRIANGLES, 3 * tm->num_tris, GL_UNSIGNED_INT, tm->tris);

	if (is_mirror && environment_map_enabled && env_cube_map_id != 0) {
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		glDisable(GL_TEXTURE_CUBE_MAP);
	}
	else if (has_textures && tm->tex) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindTexture(GL_TEXTURE_2D, 0); //Unbind texture
		glDisable(GL_TEXTURE_2D);
	}
	else {
		glDisableClientState(GL_COLOR_ARRAY);
	}

	glDisableClientState(GL_NORMAL_ARRAY);

	glDisableClientState(GL_VERTEX_ARRAY);
}

void HWFrameBuffer::render_shadow_map() {
	// Bind shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_fb);
	glViewport(0, 0, shadow_map_size, shadow_map_size);

	// Clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);

	// Set up light's view
	set_intrinsics(light_ppc);
	set_extrinsics(light_ppc);

	// Disable color writes and textures for shadow pass
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Render all meshes to shadow map (depth only)
	for (int tmi = 0; tmi < num_tms; tmi++) {
		TM* tm = &tms[tmi];

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, (float*)tm->verts);
		glDrawElements(GL_TRIANGLES, 3 * tm->num_tris, GL_UNSIGNED_INT, tm->tris);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	// Re-enable color writes
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Unbind shadow framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HWFrameBuffer::render_shadows() {
	// Save current matrices
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Set up texture matrix to transform from world space to light's clip space
	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE7);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, shadow_map);

	glLoadIdentity();
	glTranslatef(0.5f, 0.5f, 0.5f);  // Bias [-1,1] to [0,1]
	glScalef(0.5f, 0.5f, 0.5f);

	// Multiply by light's projection and modelview matrices
	set_intrinsics(light_ppc);
	glMultMatrixf(get_modelview_matrix(light_ppc));

	// Enable automatic texture coordinate generation in object space
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

	// Use identity planes for object linear generation
	// (the texture matrix does all the transformation)
	float sPlane[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
	float tPlane[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
	float rPlane[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
	float qPlane[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glTexGenfv(GL_S, GL_OBJECT_PLANE, sPlane);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, tPlane);
	glTexGenfv(GL_R, GL_OBJECT_PLANE, rPlane);
	glTexGenfv(GL_Q, GL_OBJECT_PLANE, qPlane);

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_GEN_Q);

	// Restore camera matrices for rendering
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// Set up shadow comparison
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	// Use alpha test to create shadows
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.99f);

	// Modulate with lighting
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glMatrixMode(GL_MODELVIEW);
}

void HWFrameBuffer::render_environment_map() {
	if (!environment_map_enabled || env_cube_map_id == 0)
		return;

	// Save current state
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// Disable depth test and lighting
	glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_COLOR_MATERIAL);

	// Enable cube map texture
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, env_cube_map_id);

	// Set texture environment to replace mode (don't modulate with vertex color)
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// Set up 2D orthographic projection for screen-space quad
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set white color to avoid any color modulation
	glColor3f(1.0f, 1.0f, 1.0f);

	// Manually compute texture coordinates for each corner
	// by calculating the view direction through that pixel
	glBegin(GL_QUADS);
	
	//Y coords are flipped because of OpenGL screen coords
	// Bottom-left
	V3 dir_bl = ppc->c + (float)ppc->h * ppc->b;
	dir_bl = dir_bl.normalized();
	glTexCoord3f(dir_bl[0], dir_bl[1], dir_bl[2]);
	glVertex2f(0.0f, 0.0f);

	// Bottom-right
	V3 dir_br = ppc->c + (float)ppc->w * ppc->a + (float)ppc->h * ppc->b;
	dir_br = dir_br.normalized();
	glTexCoord3f(dir_br[0], dir_br[1], dir_br[2]);
	glVertex2f((float)ppc->w, 0.0f);

	// Top-right
	V3 dir_tr = ppc->c + (float)ppc->w * ppc->a;
	dir_tr = dir_tr.normalized();
	glTexCoord3f(dir_tr[0], dir_tr[1], dir_tr[2]);
	glVertex2f((float)ppc->w, (float)ppc->h);

	// Top-left
	V3 dir_tl = ppc->c;
	dir_tl = dir_tl.normalized();
	glTexCoord3f(dir_tl[0], dir_tl[1], dir_tl[2]);
	glVertex2f(0.0f, (float)ppc->h);

	glEnd();

	// Restore matrices
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	// Unbind texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDisable(GL_TEXTURE_CUBE_MAP);

	// Restore state
	glPopAttrib();
}

void HWFrameBuffer::visualize_point_light() {
	glPointSize(10.0f);
	glBegin(GL_POINTS);
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3fv((float*)&light_pos);
	glEnd();
}

void HWFrameBuffer::render_fps_counter() {
	auto current_time = std::chrono::steady_clock::now();

	auto duration = current_time - last_frame_time;
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

	last_frame_time = current_time;

	if (elapsed > 0) {
		float fps = 1e6f / (float)elapsed;

		// Save OpenGL state
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		// Disable things that might interfere
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		// Set up 2D orthographic projection for overlay
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(0, w, 0, h);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// Draw background rectangle
		glColor3f(0.0f, 0.0f, 0.0f);  // Black
		glBegin(GL_QUADS);
		glVertex2f(10, (GLfloat)h - 35);
		glVertex2f(105, (GLfloat)h - 35);
		glVertex2f(105, (GLfloat)h - 10);
		glVertex2f(10, (GLfloat)h - 10);
		glEnd();

		// Draw FPS text using GLUT bitmap font
		glColor3f(1.0f, 1.0f, 1.0f);  // White
		char fps_str[64];
		sprintf_s(fps_str, "FPS: %d", (int)fps);

		glRasterPos2f(15, (GLfloat)h - 30);
		for (char* c = fps_str; *c != '\0'; c++) {
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
		}

		// Restore matrices
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		// Restore OpenGL state
		glPopAttrib();
	}
}

float* HWFrameBuffer::get_modelview_matrix(PPC* ppc) {
	static float matrix[16];
	
	V3 C = ppc->C;
	V3 vd = ppc->get_vd();
	V3 up = -1 * ppc->b.normalized();
	
	// Compute camera basis vectors
	V3 zaxis = (-1 * vd).normalized();  // Forward (negated view direction)
	V3 xaxis = (up ^ zaxis).normalized();  // Right
	V3 yaxis = zaxis ^ xaxis;            // Up
	
	// Build view matrix (column-major for OpenGL)
	matrix[0] = xaxis[0];
	matrix[1] = yaxis[0];
	matrix[2] = zaxis[0];
	matrix[3] = 0.0f;
	
	matrix[4] = xaxis[1];
	matrix[5] = yaxis[1];
	matrix[6] = zaxis[1];
	matrix[7] = 0.0f;
	
	matrix[8] = xaxis[2];
	matrix[9] = yaxis[2];
	matrix[10] = zaxis[2];
	matrix[11] = 0.0f;
	
	matrix[12] = -1 * xaxis * C;
	matrix[13] = -1 * yaxis * C;
	matrix[14] = -1 * zaxis * C;
	matrix[15] = 1.0f;
	
	return matrix;
}
