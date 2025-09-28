#include "GL/glew.h"
#include "scene.h"
#include "pong.h"
#include "m33.h"
#include "ppc.h"
#include "tm.h"

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
	point_light = new V3();

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

			fb->set(0xFFFFFFFF);
			tm.render_as_wireframe(&ppc_start.interpolate(&ppc_end, t), fb, false);
			fb->redraw();
			
			// Save frame to TIFF file
			char filename[256];
			sprintf_s(filename, "frames/frame_%03d.tiff", frame_counter++);
			fb->save_as_tiff(filename);
			
			Fl::check();
		}
	}
}

void Scene::render() {
	fb->clear();
		
	ppc->translate(V3(0.0f, 25.0f, 150.0f));

	TM tm("geometry/teapot1K.bin");
	tm.rasterize(ppc, fb, false);

	fb->redraw();
}

void Scene::DBG() {
	cerr << endl;
	/*{
		TM tm("geometry/teapot1K.bin");
		*point_light = tm.get_center() + V3(0.0f, 100.0f, 0.0f);
		ppc->translate(V3(0.0f, 25.0f, 150.0f));

		while (true) {
			fb->clear();
			tm.rotate_about_arbitrary_axis(tm.get_center(), V3(0.0f, 1.0f, 0.0f), 1.0f);
			tm.light_point(*point_light, 0.4f);
			tm.rasterize(ppc, fb, true);
			fb->visualize_point_light(*point_light, ppc);
			fb->redraw();
			Fl::check();
		}
		return;
	}*/
	{
		TM tm("geometry/teapot1K.bin");
		*point_light = tm.get_center() + V3(50.0f, 0.0f, 0.0f);
		ppc->translate(V3(0.0f, 25.0f, 150.0f));

		while (true) {
			fb->clear();
			tm.light_point(*point_light, 0.4f);
			tm.rasterize(ppc, fb, true);
			fb->visualize_point_light(*point_light, ppc);
			fb->redraw();
			Fl::check();

			*point_light = point_light->rotate_point(tm.get_center(), V3(0.0f, 1.0f, 0.0f), 1.0f);
		}
		return;
	}

	{	
		ppc->translate(V3(0.0f, 25.0f, 150.0f));
		TM tm("geometry/teapot1K.bin");

		while (true) {
			fb->clear();
			tm.rasterize(ppc, fb, false);
			fb->redraw();
			Fl::check();
		}
		return;
	}

	{
		// Cylinder test
		float hfov = 60.0f;
		ppc = new PPC(hfov, fb->w, fb->h);
		TM tm(V3(0, 0, -250.0f), 50.0f, 100.0f, 100, 0xFF0000FF);
		tm.rotate_about_arbitrary_axis(V3(0, 0, -250.0f), V3(1, 0, 0), 60.0f);
		tm.render_as_wireframe(ppc, fb, false);
		fb->redraw();
		return;
	}

	{
		//PPC visualization test
		PPC ppc(60.0f, fb->w, fb->h);
		PPC ppc2(60.0f, fb->w, fb->h);
		ppc.translate_forward(200.0f);
		ppc.pan(-50.0f);
		ppc.visualize(fb, &ppc2, 100.0f);
		fb->redraw();
		return;
	}

	
	{
		// spin the teapot in place (about a vertical axis passing through its center)
		float hfov = 60.0f;

		ppc = new PPC(hfov, fb->w, fb->h);
		ppc->translate(V3(0.0f, 0.0f, 250.0f));

		TM tm("geometry/teapot1K.bin");
		V3 ad(0.0f, -0.1f, 0.0f);
		V3 aO = tm.get_center();
		for (int fi = 0; fi < 10000; fi++) {
			fb->set(0xFFFFFFFF);
			//tm.draw_points(0xFF000000, 3, &ppc, fb);
			tm.render_as_wireframe(ppc, fb, false);
			fb->redraw();
			Fl::check();
			//tm.scale(.999f);
			//tm.rotate_about_arbitrary_axis(aO, ad, 0.05f);
			/*ppc.zoom(0.5f);
			ppc.pan(0.01f);
			ppc.translate_right(-.1f);*/
			//tm.translate(ad);
		}

		return;
	}

	fb->set(0xFFFFFFFF);

	//Rotation about an arbitrary axis test

	fb->draw_line(0, 100, 360, 100, 0xFF000000); //x axis
	fb->draw_line(180, 0, 180, 200, 0xFF000000); //y axis

	V3 axis(1, 7, 4);
	V3 Oa(5, 2, 0);
	V3 p(1, 100, 12);

	for (int i = 0; i <= 360; i += 2) {
		V3 p_rotated = p.rotate_point(Oa, axis, (float)i);
		fb->set_safe(i, 100 + (int)p_rotated[0], 0xFF00FF00); //green
		fb->set_safe(i, 100 + (int)p_rotated[1], 0xFFFF0000); //blue
		fb->set_safe(i, 100 + (int)p_rotated[2], 0xFF0000FF); //red
	}

	fb->redraw();

	fb->save_as_tiff("GRAPH.tiff");

	return;

	M33 m(V3(2, 5, 2), V3(8, 4, 2), V3(1, 2, 2));
	cout << m << endl;
	M33 m2 = m.inverted();

	cout << m2 << endl;

	cout << m * m2 << endl;

	return;

	int choice = 0;

	if (choice == 0) {
		//Pong Game
		PongGame::pong_game = new PongGame(fb);
		int score1 = 0;
		int score2 = 0;

		//Game Loop
		while (true) {
			int result = PongGame::pong_game->game_loop();
			Fl::check();

			//Resetting game if someone scored
			if (result != 0) {
				PongGame::pong_game = nullptr;
				delete PongGame::pong_game;
				PongGame::pong_game = new PongGame(fb);
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
	else if (choice == 1) {
		fb->set(0xFFFFFFFF);

		//Name scrolling video
		for (int u = 0; u < 10000; u++) {
			fb->set(0xFFFFFFFF);
			WriteName(u, fb);
			fb->redraw();
			Fl::check();
		}
		return;
	}
	else if (choice == 2) {
		//Writing name and saving to tiff file
		WriteName(50, fb);

		fb->redraw();
		fb->save_as_tiff("name.tiff");
		return;
	}

	cerr << endl;
	cerr << "INFO: pressed DBG button on GUI" << endl;
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

