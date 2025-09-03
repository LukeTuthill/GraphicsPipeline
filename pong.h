#pragma once

#include "framebuffer.h"

class PongGame {
public:
	int pos1;
	int pos2;
	bool p1_up;
	bool p1_down;
	bool p2_up;
	bool p2_down;
	float ball_u;
	float ball_v;
	bool ball_travel_dir; //true for right, false for left
	float ball_angle; //Radians
	FrameBuffer *fb;

	PongGame(FrameBuffer* fb);

	void redraw();

	void move_p1_up();
	void move_p1_down();
	
	void move_p2_up();
	void move_p2_down();

	int game_loop();

	static PongGame* pong_game;
};

