#pragma once
#include "framebuffer.h"
#include <random>
#include <chrono>

struct Piece {
	int x1, y1;
	int x2, y2;
	int x3, y3;
	int x4, y4;

	unsigned int color;
};

enum class PieceType {
	S = 0,
	Z = 1,
	L = 2,
	J = 3,
	T = 4,
	I = 5,
	B = 6
};

class Tetris {
public:
	Tetris(FrameBuffer* fb, int width = 10, int height = 20);

	void game_loop();

	//1 for left move, 2 for right move, 3 for left rotate, 4 for right rotate
	void set_move(int move);

private:
	int score;
	unsigned int* board;
	int width, height;

	Piece starting_pieces[7];
	int next_piece;
	Piece curr_piece;

	chrono::time_point<std::chrono::steady_clock> last_frame_time;

	//0 for nothing, 1 for left move, 2 for right move, 
	//3 for left rotate, 4 for right rotate, 5 for push down
	int next_move;

	FrameBuffer* fb;

	//Moves the piece down by one unit and does player input move, checking for collision
	//Returns true if piece under ground
	bool move_with_collision_check(Piece& curr_piece, int move, bool move_down);
	bool collision_check(Piece piece);
	bool bounds_check(Piece piece);
	void clear_lines();

	void draw_board_to_fb();
	void reset_board();
	void draw_piece(Piece piece);
	void clear_piece(Piece piece);

	unsigned int get_color(int x, int y);
	void set_color(int x, int y, unsigned int color);

	int get_random_piece();
};
