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

	ppc = new PPC(60.0f, w, h); 
	render_light = true;
	ambient_factor = .4f;
	specular_exp = 200;

	num_tms = 3;
	tms = new TM[num_tms];
	tms[0] = TM("geometry/teapot57K.bin");
	tms[1] = TM("geometry/teapot57K.bin");
	tms[1].translate(V3(75.0f, 25.0f, 0.0f));
	tms[1].scale(0.5f);
	//tms[1] = TM(V3(-100.0f, 0.0f, -100.0f), V3(100.0f, 0.0f, 100.0f), 0xFFFF00FF);
	//tms[1].rotate_about_arbitrary_axis(tms[1].get_center(), V3(0.0f, 0.0f, 1.0f), 90.0f);
	//tms[1].translate(V3(50.0f, 50.0f, 0.0f));
	tms[2] = TM(V3(-100.0f, 0.0f, -100.0f), V3(100.0f, 0.0f, 100.0f), 0xFF888888);

	shadow_map = new ShadowMap(512, 512, V3());
	point_light = new V3();

	pong_game = nullptr;

	gui = new GUI();
	gui->show();
	gui->uiw->position(u0, v0 + fb->h + v0);
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

	int num_frames = 300;
	int frames_per_camera = num_frames / (num_ppcs - 1);
	int frame_counter = 0;

	// Create frames directory
	system("mkdir frames");

	for (int p = 0; p < num_ppcs - 1; p++) {
		PPC ppc_start = ppcs[p];
		PPC ppc_end = ppcs[p + 1];
		for (int f = 0; f < frames_per_camera; f++) {
			float t = (float)f / (float)frames_per_camera;

			fb->clear();
			tm.rasterize(&ppc_start.interpolate(&ppc_end, t), fb, false, false, false);
			fb->redraw();
			
			// Save frame to TIFF file
			char filename[256];
			sprintf_s(filename, "frames/frame_%03d.tiff", frame_counter++);
			fb->save_as_tiff(filename);
			
			Fl::check();
		}
	}
}

void Scene::render_shadows() {
	shadow_map->clear();
	for (int i = 0; i < num_tms; i++) {
		shadow_map->add_tm(&tms[i]);
	}
}

void Scene::render() {
	fb->clear();

	for (int i = 0; i < num_tms; i++) {
		render(tms[i]);
	}

	if (render_light)
		fb->visualize_point_light(*point_light, ppc);

	fb->redraw();
	Fl::check();
}

void Scene::render(TM& tm) {
	if (render_light)
		tm.light_point(shadow_map, ppc->C, ambient_factor, specular_exp);
	tm.rasterize(ppc, fb, render_light, false, false);
}

void Scene::DBG() {
	cerr << endl;
	int choice = 4;
	
	switch (choice) {
	case 4: { //Texture test
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

		V3 tiling_tcs[4] = {V3(0.0f, 0.0f, 0.0f), V3(0.0f, 4.0f, 0.0f), V3(4.0f, 0.0f, 0.0f), V3(4.0f, 4.0f, 0.0f)};
		V3 tcs[4] = {V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), V3(1.0f, 0.0f, 0.0f), V3(1.0f, 1.0f, 0.0f)};

		bricks_tm.set_tex(bricks_tex, tiling_tcs);
		flag_tm.set_tex(flag_tex, tcs);
		face_tm.set_tex(face_tex, tiling_tcs);
		box_tm.set_tex(box_tex, tcs);

		ppc->translate(V3(0.0f, 50.0f, 250.0f));

		flag_tm.translate(V3(-150.0f, 0.0f, 0.0f));
		face_tm.translate(V3(0.0f, 150.0f, 0.0f));
		box_tm.translate(V3(150.0f, 0.0f, 0.0f));

		while (true) {
			fb->clear();

			bricks_tm.rasterize(ppc, fb, false, true, mirror);
			flag_tm.rasterize(ppc, fb, false, true, mirror);
			face_tm.rasterize(ppc, fb, false, true, mirror);
			box_tm.rasterize(ppc, fb, false, true, mirror);

			fb->redraw();
			Fl::check();
		}
	}
	case 3: { //Moving triangle mesh with lighting
		ppc->translate(V3(0.0f, 50.0f, 250.0f));
		*point_light = tms[0].get_center() + V3(0.0f, 50.0f, 150.0f);
		*point_light = point_light->rotate_point(tms[0].get_center(), V3(0.0f, 1.0f, 0.0f), 90.0f);
		shadow_map->set_pos(*point_light);
		
		while (true) {
			tms[1].rotate_about_arbitrary_axis(tms[0].get_center(), V3(0.0f, 1.0f, 0.0f), 1.0f);
			shadow_map->set_pos(*point_light);
			render_shadows();
			render();
		}
		return;
	}
	case 2: { //Moving point light with shadows
		ppc->translate(V3(0.0f, 50.0f, 250.0f));
		*point_light = tms[0].get_center() + V3(0.0f, 50.0f, 150.0f);
		shadow_map->set_pos(*point_light);
		
		while (true) {
			shadow_map->set_pos(*point_light);
			render_shadows();
			render();
		}
		return;
	}
	case 1: { //Static render with lighting, loop allows camera movement
		ppc->translate(V3(0.0f, 50.0f, 300.0f));
		*point_light = tms[0].get_center() + V3(0.0f, 50.0f, 150.0f);
		*point_light = point_light->rotate_point(tms[0].get_center(), V3(0.0f, 1.0f, 0.0f), 90.0f);

		render_light = true;
		while (true) {
			shadow_map->set_pos(*point_light);
			render_shadows();
			render();
		}
		return;
	}
	case 0: { //Static render, loop allows camera movement
		ppc->translate(V3(0.0f, 0.0f, 300.0f));
		render_light = false;
		while (true) {
			render();
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

