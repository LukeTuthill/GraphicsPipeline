#include "tetris.h"

using namespace std;

const unsigned int BLACK = 0x00000000;
const unsigned int WHITE = 0xFFFFFFFF;
const unsigned int BLUE = 0xFF0000FF;
const unsigned int RED = 0xFFFF0000;

Tetris::Tetris(FrameBuffer* fb, int width, int height) {
	this->width = width;
	this->height = height;
	this->fb = fb;

	board = new unsigned int[width * height];
	for (int i = 0; i < width * height; i++) {
		board[i] = BLACK;
	}

	//Define starting pieces
	int half_width = width / 2;

	starting_pieces[0] = { half_width, 0,
		half_width + 1, 0,
		half_width - 1, 1,
		half_width, 1, BLUE
	}; //S

	starting_pieces[1] = { half_width - 1, 0,
		half_width, 0,
		half_width, 1,
		half_width + 1, 1, RED
	}; //Z

	starting_pieces[2] = { half_width, 0,
		half_width, 1,
		half_width, 2,
		half_width + 1, 2, RED
	}; //L

	starting_pieces[3] = { half_width, 0,
		half_width, 1,
		half_width, 2,
		half_width - 1, 2, BLUE
	}; //J

	starting_pieces[4] = { half_width - 1, 0,
		half_width, 0,
		half_width + 1, 0,
		half_width, 1, WHITE
	}; //T

	starting_pieces[5] = { half_width, 0,
		half_width, 1,
		half_width, 2,
		half_width, 3, WHITE
	}; //I

	starting_pieces[6] = { half_width, 0,
		half_width + 1, 0,
		half_width, 1,
		half_width + 1, 1, WHITE
	}; //B

	//Set game variables
	score = 0;
	curr_piece = starting_pieces[get_random_piece()];
	draw_piece(curr_piece);
	next_piece = get_random_piece();
	next_move = 0;
	last_frame_time = chrono::steady_clock::now();
}

void Tetris::game_loop() {
	//Controlling down movement
	auto duration = chrono::steady_clock::now() - last_frame_time;
	bool move_down = (chrono::duration_cast<chrono::milliseconds>(duration).count() > 100);
	if (move_down || next_move == 5) {
		last_frame_time = chrono::steady_clock::now();
	}

	draw_board_to_fb();

	if (move_with_collision_check(curr_piece, next_move, move_down)) {
		//Spawn new piece
		curr_piece = starting_pieces[next_piece];
		curr_piece = starting_pieces[6];
		next_piece = get_random_piece();
		
		clear_lines();
	}


	//Always reset player move after processing
	next_move = 0;
}

void Tetris::clear_lines() {
	for (int h = 0; h < height; h++) {
		bool line_full = true;
		for (int w = 0; w < width; w++) {
			if (get_color(w, h) == BLACK) {
				line_full = false;
				break;
			}
		}
		if (line_full) {
			// Clear the line
			for (int w = 0; w < width; w++) {
				set_color(w, h, BLACK);
			}
			score += 100; // Increment score

			//Move lines down
			for (int line = h - 1; line >= 0; line--) {
				for (int w = 0; w < width; w++) {
					set_color(w, line + 1, get_color(w, line));
				}
			}
		}
	}
}

void Tetris::set_move(int move) {
	next_move = move;
}

bool Tetris::move_with_collision_check(Piece& curr_piece, int move, bool move_down) {
	//Clear current piece from board
	clear_piece(curr_piece);

	Piece original_piece = curr_piece; //For collision rollback

	//Piece is virtual until redrawn, so we can check for collisions first
	switch (move) {
		case 1: //Left move
			curr_piece.x1--;
			curr_piece.x2--;
			curr_piece.x3--;
			curr_piece.x4--;
			break;
		case 2: //Right move
			curr_piece.x1++;
			curr_piece.x2++;
			curr_piece.x3++;
			curr_piece.x4++;
			break;
	}

	//Check for collision with other pieces after move
	if ((move != 0 && move != 5) && (bounds_check(curr_piece) || collision_check(curr_piece))) {
		//Rollback move
		curr_piece = original_piece;
	}

	if (move_down || move == 5) {
		curr_piece.y1++;
		curr_piece.y2++;
		curr_piece.y3++;
		curr_piece.y4++;
	}

	//Check for collision on bottom with other pieces
	if (collision_check(curr_piece)) {
		//Place piece back to last valid position
		//curr_piece will be replaced by new piece in game loop
		draw_piece(original_piece);
		return true;
	}

	//Redraw piece
	draw_piece(curr_piece);
	
	return (curr_piece.y4 >= height - 1); //Checks for floor collision
}

bool Tetris::collision_check(Piece piece) {
	return (get_color(piece.x1, piece.y1) != BLACK ||
		get_color(piece.x2, piece.y2) != BLACK ||
		get_color(piece.x3, piece.y3) != BLACK ||
		get_color(piece.x4, piece.y4) != BLACK);
}

bool Tetris::bounds_check(Piece piece) {
	return (piece.x1 < 0 || piece.x2 < 0 || piece.x3 < 0 || piece.x4 < 0 ||
		piece.x1 >= width || piece.x2 >= width || piece.x3 >= width || piece.x4 >= width);
}

void Tetris::reset_board() {
	for (int i = 0; i < width * height; i++) {
		board[i] = BLACK;
	}
}

void Tetris::draw_board_to_fb() {
	int start_v = fb->h / 16;
	int end_v = fb->h - start_v;
	int game_height = end_v - start_v;
	end_v -= game_height % height;

	int step = game_height / height;

	int game_width = step * width;
	int start_u = (fb->w - game_width) / 2;
	int end_u = start_u + game_width;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			fb->draw_rectangle(start_u + x * step, start_v + y * step, step, step, get_color(x, y));
		}
	}
}

unsigned int Tetris::get_color(int x, int y) {
	return board[x * height + y];
}

void Tetris::set_color(int x, int y, unsigned int color) {
	board[x * height + y] = color;
}

void Tetris::draw_piece(Piece piece) {
	set_color(piece.x1, piece.y1, piece.color);
	set_color(piece.x2, piece.y2, piece.color);
	set_color(piece.x3, piece.y3, piece.color);
	set_color(piece.x4, piece.y4, piece.color);
}

void Tetris::clear_piece(Piece piece) {
	set_color(piece.x1, piece.y1, BLACK);
	set_color(piece.x2, piece.y2, BLACK);
	set_color(piece.x3, piece.y3, BLACK);
	set_color(piece.x4, piece.y4, BLACK);
}

int Tetris::get_random_piece() {
	return rand() % 7;
}