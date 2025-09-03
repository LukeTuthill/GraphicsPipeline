#include "GL/glew.h"

#include "scene.h"

#include "pong.h"

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

	gui = new GUI();
	gui->show();
	gui->uiw->position(u0, v0 + fb->h + v0);

}


void Scene::DBG() {
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
		fb->Set(0xFFFFFFFF);

		//Name scrolling video
		for (int u = 0; u < 10000; u++) {
			fb->Set(0xFFFFFFFF);
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
		fb->SaveAsTiff("name.tiff");
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
	fb->DrawLine(u, 100, u, 300, 0xFF000000);
	fb->DrawLine(u, 300, u + 100, 300, 0xFF000000);

	//U
	fb->DrawLine(u + 150, 100, u + 150, 300, 0xFF000000);
	fb->DrawLine(u + 150, 300, u + 250, 300, 0xFF000000);
	fb->DrawLine(u + 250, 300, u + 250, 100, 0xFF000000);

	//K
	fb->DrawLine(u + 300, 100, u + 300, 300, 0xFF000000);
	fb->DrawLine(u + 300, 200, u + 400, 100, 0xFF000000);
	fb->DrawLine(u + 300, 200, u + 400, 300, 0xFF000000);

	//E	
	fb->DrawLine(u + 450, 100, u + 450, 300, 0xFF000000);
	fb->DrawLine(u + 450, 100, u + 550, 100, 0xFF000000);
	fb->DrawLine(u + 450, 200, u + 525, 200, 0xFF000000);
	fb->DrawLine(u + 450, 300, u + 550, 300, 0xFF000000);
}

