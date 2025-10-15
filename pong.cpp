#define _USE_MATH_DEFINES
#include <cmath>

#include "pong.h"

using namespace std;

#define UP_1 0
#define DOWN_1 1
#define UP_2 2
#define DOWN_2 3

PongGame::PongGame(FrameBuffer* fb) {
	this->fb = fb;
	pos1 = 0;
	pos2 = 0;
	p1_up = false;
	p1_down = false;
	p2_up = false;
	p2_down = false;
	ball_u = (float)fb->w / 2;
	ball_v = (float)fb->h / 2;
	ball_travel_dir = true; //moves to the right first
	ball_angle = 0;

	redraw();
}

void PongGame::redraw() {
	fb->set(0xFF000000);
	fb->draw_rectangle(20, pos1, 10, 60, 0xFFFFFFFF);
	fb->draw_rectangle(fb->w - 30, pos2, 10, 60, 0xFFFFFFFF);
	fb->draw_circle((int)(ball_u + .5), (int)(ball_v + .5), 8, 0xFFFFFFFF);
	fb->redraw();
}

void PongGame::move_p1_up() {
	p1_up = true;
	p1_down = false;
}

void PongGame::move_p1_down() {
	p1_down = true;
	p1_up = false;
}

void PongGame::move_p2_up() {
	p2_up = true;
	p2_down = false;
}

void PongGame::move_p2_down() {
	p2_down = true;
	p2_up = false;
}

int PongGame::game_loop() {
	//Paddle movement
	if (p1_up) {
		if (pos1 >= 20) pos1 -= 20;
		p1_up = false;
	}
	else if (p1_down) {
		if (pos1 < fb->h - 80) pos1 += 20;
		p1_down = false;
	}

	if (p2_up) {
		if (pos2 >= 20) pos2 -= 20;
		p2_up = false;
	}
	else if (p2_down) {
		if (pos2 < fb->h - 80) pos2 += 20;
		p2_down = false;
	}

	//Collison detection
	if (!ball_travel_dir && ball_u <= 30) {
		if (ball_v >= pos1 && ball_v <= pos1 + 60) {
			ball_travel_dir = true;
			float intersect = (float)((pos1 + 30) - (int)(ball_v + .5)) / 30.0f;
			ball_angle = intersect * ((float)M_PI / 4.0f); //Max deflection of 45 degrees
		}
		else {
			return 2; //Player 2 scores
		}
	}
	else if (ball_travel_dir && ball_u >= fb->w - 30) {
		if (ball_v >= pos2 && ball_v <= pos2 + 60) {
			ball_travel_dir = false;
			float intersect = (float)((pos2 + 30) - (int)(ball_v + .5)) / 30.0f;
			ball_angle = intersect * ((float)M_PI / 4.0f); //Max deflection of 45 degrees
		}
		else {
			return 1; //Player 1 scores
		}
	}

	//Upper and lower wall collision
	if (ball_v <= 8) {
		ball_v = 8;
		ball_angle = -ball_angle;
	}
	else if (ball_v >= fb->h - 8) {
		ball_v = (float)fb->h - 8;
		ball_angle = -ball_angle;
	}

	//Ball movement
	if (ball_travel_dir) {
		ball_u += (.5f * cosf(ball_angle));
		ball_v -= (.5f * sinf(ball_angle));
	}
	else {
		ball_u -= (.5f * cosf(ball_angle));
		ball_v += (.5f * sinf(ball_angle));
	}

	redraw();
	return 0;
}
