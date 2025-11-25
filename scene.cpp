#include "GL/glew.h"
#include "scene.h"
#include "m33.h"

Scene *scene;

#include <iostream>
#include <fstream>
#include <strstream>

using namespace std;

Scene::Scene() {
	int u0 = 20;
	int v0 = 40;
	int h = 480;
	int w = 640;

	fb = new FrameBuffer(u0, v0, w, h);
	fb->position(u0, v0);
	fb->label("SW Framebuffer");
	fb->show();
	fb->redraw();

	hw_fb = new HWFrameBuffer(u0, v0, w, h);
	hw_fb->position(u0 + w + u0, v0);
	hw_fb->label("HW Framebuffer");
	hw_fb->show();
	hw_fb->redraw();

	ppc = new PPC(60.0f, w, h);
	render_light = true;
	ambient_factor = .4f;
	specular_exp = 200;

	num_tms = 3;
	tms = new TM[num_tms];
	tms[0] = TM("geometry/teapot1K.bin");
	if (num_tms > 1) {
		tms[1] = TM();
		tms[1].set_as_quad(V3(-100.0f, 0.0f, -100.0f),
			V3(-100.0f, 0.0f, 100.0f),
			V3(100.0f, 0.0f, -100.0f),
			V3(100.0f, 0.0f, 100.0f),
			0xFF0000FF);
	}
	if (num_tms > 2) {
		tms[2] = make_texture_tms()[1];
		tms[2].position(V3(0.0f, 30.0f, 100.0f));
		tms[2].scale(.75f);
	}

	shadow_map = new ShadowMap(512, 512, V3());
	cube_map = nullptr;
	point_light = new V3();

	pong_game = nullptr;

	gui = new GUI();
	gui->show();
	gui->uiw->position(u0, v0 + fb->h + v0);

	hw_fb->ppc = ppc;
	hw_fb->set_tms(tms, num_tms);
	//hw_fb->set_shadow_map(*point_light, 512);
	//hw_fb->set_lighting(*point_light);
}

void Scene::render_cameras_as_frames() {
	PPC* ppcs;
	int num_ppcs = load_from_file(&ppcs, "cameras.bin");
	if (num_ppcs < 1) {
		cerr << "ERROR: cannot load cameras from file cameras.bin" << endl;
		return;
	}
	if (num_ppcs == 1) {
		cerr << "ERROR: only one camera in cameras.bin, need at least two" << endl;
		return;
	}

	TM tm("geometry/teapot1K.bin");

	int num_frames = 1500;
	int frames_per_camera = num_frames / (num_ppcs - 1);
	int frame_counter = 0;

	render_type rt = render_type::NOT_LIGHTED;
	bool save_to_file = false;

	// Create frames directory
	if (save_to_file)
		system("mkdir frames");

	PPC* scene_ppc = ppc; // Save original ppc

	for (int p = 0; p < num_ppcs - 1; p++) {
		PPC ppc_start = ppcs[p];
		PPC ppc_end = ppcs[p + 1];
		for (int f = 0; f < frames_per_camera; f++) {
			float t = (float)f / (float)frames_per_camera;

			ppc = &ppc_start.interpolate(&ppc_end, t);
			render(rt);
			
			// Save frame to TIFF file
			if (save_to_file) {
				char filename[256];
				sprintf_s(filename, "frames/frame_%03d.tiff", frame_counter++);
				fb->save_as_tiff(filename);
			}
		}
	}
	ppc = scene_ppc; // Restore original ppc
}

void Scene::render_shadows() {
	shadow_map->clear();
	for (int i = 0; i < num_tms; i++) {
		shadow_map->add_tm(&tms[i]);
	}
}


void Scene::render(render_type rt) {
	fb->clear();

	for (int i = 0; i < num_tms; i++) {
		render(tms[i], rt);
	}

	if (render_light && rt == render_type::LIGHTED) {
		fb->visualize_point_light(*point_light, ppc);
		if (hw_fb) {
			hw_fb->move_light(*point_light);
		}
	}

	if (cube_map)
		cube_map->render_as_environment(ppc, fb);

	if (hw_fb) {
		hw_fb->ppc = ppc;
		hw_fb->render_wireframe = render_wireframe;
		if (rt == render_type::LIGHTED)
			hw_fb->use_lighting = render_light;
		else
			hw_fb->use_lighting = false;
		hw_fb->redraw();
	}

	fb->redraw();
	Fl::check();
}

void Scene::render(TM& tm, render_type rt) {
	if (!tm.tex && render_light && rt == render_type::LIGHTED) {
		tm.light_point(shadow_map, ppc->C, ambient_factor, specular_exp);
	}
	if (tm.tex)
		tm.rasterize(ppc, fb, cube_map, render_type::NORMAL_TILING_TEXTURED);
	else
		tm.rasterize(ppc, fb, cube_map, rt);
}

void Scene::DBG() {
	cerr << endl;
	int choice = 7;
	
	switch (choice) {
	case 7: { //HW Rendering for billboard imposter
		ppc->translate(V3(0.0f, 75.0f, 300.0f));
		fb->hide();

		hw_fb->tm_is_reflector[0] = true;
		
		hw_fb->create_ground_plane_billboard(1);
		hw_fb->create_billboard_from_tm(2);
	
		while (true) {
			hw_fb->ppc = ppc;
			hw_fb->redraw();
			Fl::check();
		}
		return;
	}
	case 6: { //HW rendering only
		ppc->translate(V3(0.0f, 0.0f, 300.0f));
		TM* tex_tms = make_texture_tms();
		tms[1] = tex_tms[1];
		hw_fb->set_tms(tms, 2);

		fb->hide();

		while (true) {
			hw_fb->ppc = ppc;
			hw_fb->render_wireframe = render_wireframe;
			hw_fb->redraw();
			Fl::check();
		}
		return;
	}
	case 5: { //Environment cube map with reflective object test
		cube_map = new CubeMap("textures/uffizi_cross.tiff");
		ppc->translate(V3(0.0f, 0.0f, 250.0f));
		tms[0].position(V3(0.0f, 0.0f, 0.0f));
		render_type rt = render_type::MIRROR_ONLY;
		render_light = false;

		if (hw_fb) {
			hw_fb->set_environment_map(cube_map);
			hw_fb->tm_is_reflector[0] = true;
		}

		while (true) {
			render(rt);
		}
	}
	case 4: { //Texture test
		ppc->translate(V3(0.0f, 0.0f, 250.0f));
		TM* texture_tms = make_texture_tms();

		render_type rt = render_type::NORMAL_TILING_TEXTURED;

		if (hw_fb) {
			hw_fb->set_tms(texture_tms, 4);
			hw_fb->use_lighting = false;
		}

		while (true) {
			fb->clear();

			if (mirror_tiling)
				rt = render_type::MIRRORED_TILING_TEXTURED;
			else
				rt = render_type::NORMAL_TILING_TEXTURED;

			if (hw_fb) {
				hw_fb->render_wireframe = render_wireframe;
				hw_fb->redraw();
			}

			texture_tms[0].rasterize(ppc, fb, cube_map, rt);
			texture_tms[1].rasterize(ppc, fb, cube_map, rt);
			texture_tms[2].rasterize(ppc, fb, cube_map, rt);
			texture_tms[3].rasterize(ppc, fb, cube_map, rt);

			fb->redraw();
			Fl::check();
		}
	}
	case 3: { //Moving triangle mesh with lighting
		ppc->translate(V3(0.0f, 50.0f, 250.0f));
		*point_light = tms[0].get_center() + V3(0.0f, 50.0f, 150.0f);
		*point_light = point_light->rotate_point(tms[0].get_center(), V3(0.0f, 1.0f, 0.0f), 90.0f);
		shadow_map->set_pos(*point_light);

		render_type rt = render_type::LIGHTED;
		
		while (true) {
			tms[1].rotate_about_arbitrary_axis(tms[0].get_center(), V3(0.0f, 1.0f, 0.0f), 1.0f);
			shadow_map->set_pos(*point_light);
			render_shadows();
			render(rt);
		}
		return;
	}
	case 2: { //Moving point light with shadows
		ppc->translate(V3(0.0f, 50.0f, 250.0f));
		*point_light = tms[0].get_center() + V3(0.0f, 50.0f, 150.0f);

		render_type rt = render_type::LIGHTED;
		
		while (true) {
			*point_light = point_light->rotate_point(tms[0].get_center(), V3(0.0f, 1.0f, 0.0f), 2.0f);
			shadow_map->set_pos(*point_light);
			if (hw_fb)
				hw_fb->move_light(*point_light);
			render_shadows();
			render(rt);
		}
		return;
	}
	case 1: { //Static render with lighting, loop allows camera movement
		ppc->translate(V3(0.0f, 50.0f, 300.0f));
		*point_light = tms[0].get_center() + V3(0.0f, 50.0f, 150.0f);
		*point_light = point_light->rotate_point(tms[0].get_center(), V3(0.0f, 1.0f, 0.0f), 90.0f);
		if (hw_fb) {
			hw_fb->move_light(*point_light);
		}

		render_light = true;

		render_type rt = render_type::LIGHTED;
		while (true) {
			shadow_map->set_pos(*point_light);
			render_shadows();
			render(rt);
		}
		return;
	}
	case 0: { //Static render, loop allows camera movement
		ppc->translate(V3(0.0f, 75.0f, 300.0f));
		render_light = false;

		render_type rt = render_type::NOT_LIGHTED;
		while (true) {
			render(rt);
		}
		return;
	}
	case -1: { //PPC visualization test
		PPC ppc(60.0f, fb->w, fb->h);
		PPC ppc2(60.0f, fb->w, fb->h);
		ppc.translate_forward(200.0f);
		ppc.pan(-50.0f);
		ppc.visualize(fb, &ppc2, 100.0f);
		fb->redraw();
		return;
	}
	case -2: { //Pong Game
		pong_game = new PongGame(fb);
		pong = true;

		int score1 = 0;
		int score2 = 0;

		//Game Loop
		while (true) {
			int result = pong_game->game_loop();
			Fl::check();

			//Resetting game if someone scored
			if (result != 0) {
				delete pong_game;
				pong_game = new PongGame(fb);
				if (result == 1) {
					cerr << "Player 1 scores!" << endl;
					score1++;
				}
				else if (result == 2) {
					cerr << "Player 2 scores!" << endl;
					score2++;
				}
				cerr << "Score: " << score1 << " - " << score2 << endl;
			}
		}
		return;
	}
	}
}

void Scene::NewButton() {
	cerr << "INFO: pressed New button on GUI" << endl;
}

static void WriteName(int u, FrameBuffer *fb) {
	//L
	fb->draw_line(u, 100, u, 300, 0xFF000000);
	fb->draw_line(u, 300, u + 100, 300, 0xFF000000);

	//U
	fb->draw_line(u + 150, 100, u + 150, 300, 0xFF000000);
	fb->draw_line(u + 150, 300, u + 250, 300, 0xFF000000);
	fb->draw_line(u + 250, 300, u + 250, 100, 0xFF000000);

	//K
	fb->draw_line(u + 300, 100, u + 300, 300, 0xFF000000);
	fb->draw_line(u + 300, 200, u + 400, 100, 0xFF000000);
	fb->draw_line(u + 300, 200, u + 400, 300, 0xFF000000);

	//E	
	fb->draw_line(u + 450, 100, u + 450, 300, 0xFF000000);
	fb->draw_line(u + 450, 100, u + 550, 100, 0xFF000000);
	fb->draw_line(u + 450, 200, u + 525, 200, 0xFF000000);
	fb->draw_line(u + 450, 300, u + 550, 300, 0xFF000000);
}

TM* make_texture_tms() {
	TM bricks_tm = TM();
	bricks_tm.set_as_quad(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 50.0f, 100.0f),
		V3(100.0f, 50.0f, 0.0f), V3(100.0f, 0.0f, 100.0f), 0xFF000000);

	FrameBuffer* bricks_tex = new FrameBuffer(0, 0, 100, 100);
	bricks_tex->load_tiff("textures/bricks.tiff");
	
	TM flag_tm = TM();
	flag_tm.set_as_quad(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 50.0f, 100.0f),
		V3(100.0f, 50.0f, 0.0f), V3(100.0f, 0.0f, 100.0f), 0xFF000000);

	FrameBuffer* flag_tex = new FrameBuffer(0, 0, 100, 100);
	flag_tex->load_tiff("textures/purdue.tiff");

	TM face_tm = TM();
	face_tm.set_as_quad(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 50.0f, 100.0f),
		V3(100.0f, 50.0f, 0.0f), V3(100.0f, 0.0f, 100.0f), 0xFF000000);

	FrameBuffer* face_tex = new FrameBuffer(0, 0, 100, 100);
	face_tex->load_tiff("textures/popescu.tiff");

	TM box_tm = TM();
	box_tm.set_as_quad(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 50.0f, 100.0f),
		V3(100.0f, 50.0f, 0.0f), V3(100.0f, 0.0f, 100.0f), 0xFF000000);

	FrameBuffer* box_tex = new FrameBuffer(0, 0, 100, 100);
	box_tex->load_tiff("textures/amazon.tiff");

	V3* tiling_tcs = new V3[4]{V3(0.0f, 0.0f, 0.0f), V3(0.0f, 4.0f, 0.0f), V3(4.0f, 0.0f, 0.0f), V3(4.0f, 4.0f, 0.0f)};
	V3* tcs = new V3[4]{V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), V3(1.0f, 0.0f, 0.0f), V3(1.0f, 1.0f, 0.0f)};

	bricks_tm.set_tex(bricks_tex, tiling_tcs);
	flag_tm.set_tex(flag_tex, tcs);
	face_tm.set_tex(face_tex, tiling_tcs);
	box_tm.set_tex(box_tex, tcs);

	flag_tm.translate(V3(-150.0f, 0.0f, 0.0f));
	face_tm.translate(V3(0.0f, 150.0f, 0.0f));
	box_tm.translate(V3(150.0f, 0.0f, 0.0f));

	TM* tms = new TM[4];

	tms[0] = bricks_tm;
	tms[1] = flag_tm;
	tms[2] = face_tm;
	tms[3] = box_tm;

	return tms;
}

