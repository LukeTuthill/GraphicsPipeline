#include "framebuffer.h"
#include <cstdlib>

using namespace std;

const unsigned int BLACK = 0x00000000;
const unsigned int WHITE = 0xFFFFFFFF;
const unsigned int BLUE = 0x000000FF;
const unsigned int RED = 0x00FF0000;

enum class Piece {
	S,
	Z,
	L,
	J,
	T,
	I,
	B
};

void play_tetris(FrameBuffer fb) {
	unsigned int play_area_color[10][20];

	//Setting game area to all black

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 20; j++) {
			play_area_color[i][j] = BLACK;
		}
	}

	//Main game loop

	Piece prev_piece = get_random_piece();
	Piece piece;
	unsigned int piece_color;
	bool new_piece = true;

	while (true) {
		redraw(fb, play_area);

		if (new_piece) {
			piece = prev_piece;
			prev_piece = get_random_piece();


		}
	}
}

Piece get_random_piece() {
	return static_cast<Piece>(rand() % 7);
}

void draw_piece(int u, int v, Piece piece, int play_area_color[10][20]) {
	switch (piece) {
		case S:
			play_area_color[5][0] = piece_color;
			break;
		case Z:
			break;
		case L:
			break;
		case J:
			break;
		case T:
			break;
		case I:
			break;
		case B:
			break;
		}
}

void redraw(FrameBuffer fb, unsigned int play_area_color[10][20]) {
	int w_constant = fb->w / 10;
	int h_constant = fb->h / 20;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 20; j++) {
			fb->DrawRectangle(i * w_constant, j * h_constant, w_constant, h_constant, play_area_color[i][j]);
		}
	}

	fb->redraw();
}
