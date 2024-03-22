#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include "tets.h"

#define FACTOR 300
#define WIDTH (4*FACTOR) 
#define HEIGHT (3*FACTOR)
#define COLOR_BACKGROUND CLITERAL(Color){48, 48, 48, 255}
#define BOARD_HEIGHT (FACTOR*2.5)
#define BOARD_COLS 10
#define BOARD_ROWS 20
#define BOARD_WIDTH (BOARD_HEIGHT/BOARD_ROWS*BOARD_COLS)
#define THICKNESS 4
#define FPS 60
#define SPEED_INCREASE 1.2
#define NUM_FILLS 5
#define HOLDING_FRAME_THRESHOLD 15
#define FRAMES_FOR_HOLDING 1
#define FRAMES_FOR_HOLDING_W 7

//#define SWITCHING

void rotated(int i, int j, int rot, int* ri, int*rj) {
	switch (rot) {
		case 0: *ri = i    ; *rj = j; break;
		case 1: *ri = j    ; *rj = 3 - i; break;
		case 2: *ri = 3 - i; *rj = 3 - j; break;
		case 3: *ri = 3 - j; *rj = i; break;
		default: assert(0 && "UNREACHABLE");
	}
}

int polje[BOARD_ROWS][BOARD_COLS] = { 0 };

#define DrawTile(x, y, z) DrawTileImp(x, y, z, 255,  0);
#define DrawGhost(x, y, z) DrawTileImp(x, y, z,128, 0);
#define DrawHold(x, y, z, h) DrawTileImp(x, y, z, 255, !h);
void DrawTileImp(int i, int j, int type, int alpha, int gray) {
	Color color;
	if (i < 0) return;
	switch (type) {
		case 1: color = SKYBLUE; break;
		case 2: color = BLUE; break;
		case 3: color = ORANGE; break;
		case 4: color = YELLOW; break;
		case 5: color = LIME; break;
		case 6: color = PURPLE; break;
		case 7: color = RED; break;
		default: assert(0 && "unreachable");
	}
	color.a = alpha;
	if (gray) color = GRAY; 
	DrawRectangle((WIDTH-BOARD_WIDTH)/2 + j*BOARD_WIDTH/BOARD_COLS + THICKNESS/2, (HEIGHT-BOARD_HEIGHT)/2 + i*BOARD_WIDTH/BOARD_COLS + THICKNESS/2, BOARD_WIDTH/BOARD_COLS - THICKNESS, BOARD_WIDTH/BOARD_COLS - THICKNESS, color);
	Vector3 HSV = ColorToHSV(color);
	HSV.y /= 2;
	HSV.z /= 2;
	color = ColorFromHSV(HSV.x, HSV.y, HSV.z);
	color.a = alpha;
	DrawLineEx(	CLITERAL(Vector2){(WIDTH-BOARD_WIDTH)/2 + j*BOARD_WIDTH/BOARD_COLS, (HEIGHT-BOARD_HEIGHT)/2 + i*BOARD_WIDTH/BOARD_COLS},
				CLITERAL(Vector2){(WIDTH-BOARD_WIDTH)/2 + (j+1)*BOARD_WIDTH/BOARD_COLS, (HEIGHT-BOARD_HEIGHT)/2 + i*BOARD_WIDTH/BOARD_COLS},
				THICKNESS, color);
	DrawLineEx(	CLITERAL(Vector2){(WIDTH-BOARD_WIDTH)/2 + j*BOARD_WIDTH/BOARD_COLS, (HEIGHT-BOARD_HEIGHT)/2 + i*BOARD_WIDTH/BOARD_COLS},
				CLITERAL(Vector2){(WIDTH-BOARD_WIDTH)/2 + j*BOARD_WIDTH/BOARD_COLS, (HEIGHT-BOARD_HEIGHT)/2 + (i+1)*BOARD_WIDTH/BOARD_COLS},
				THICKNESS, color);
	DrawLineEx(	CLITERAL(Vector2){(WIDTH-BOARD_WIDTH)/2 + (j+1)*BOARD_WIDTH/BOARD_COLS, (HEIGHT-BOARD_HEIGHT)/2 + (i+1)*BOARD_WIDTH/BOARD_COLS},
				CLITERAL(Vector2){(WIDTH-BOARD_WIDTH)/2 + (j+1)*BOARD_WIDTH/BOARD_COLS, (HEIGHT-BOARD_HEIGHT)/2 + i*BOARD_WIDTH/BOARD_COLS},
				THICKNESS, color);
	DrawLineEx(	CLITERAL(Vector2){(WIDTH-BOARD_WIDTH)/2 + (j+1)*BOARD_WIDTH/BOARD_COLS, (HEIGHT-BOARD_HEIGHT)/2 + (i+1)*BOARD_WIDTH/BOARD_COLS},
				CLITERAL(Vector2){(WIDTH-BOARD_WIDTH)/2 + j*BOARD_WIDTH/BOARD_COLS, (HEIGHT-BOARD_HEIGHT)/2 + (i+1)*BOARD_WIDTH/BOARD_COLS},
				THICKNESS, color);
}

int isValid(int i, int j, int rot, int t) {
	int ri, rj;
	for (int di = 0; di < 4; di++) {
		for (int dj = 0; dj < 4; dj++) {
			rotated(di, dj, rot, &ri, &rj);
			if (tets[t][di][dj]) {
				if (ri+i >= BOARD_ROWS || rj+j < 0|| rj+j >= BOARD_COLS) {
					return 0;
				}
				if (ri+i < 0) continue;
				if (polje[ri+i][rj+j]) {
					return 0;
				}
			}
		}
	}
	return 1;
}

int main(void) {
	int holdingS = 0, holdingA = 0, holdingW = 0, holdingD = 0;
	int fills = 0;
	int level = 0;
	int score = 0;
	int scores[] = {0, 40, 100, 300, 1200};
	int playing = 1;
	InitWindow(WIDTH, HEIGHT, "Tetris");
	srand(time(NULL));
	SetTargetFPS(FPS);
	Rectangle board = CLITERAL(Rectangle){(WIDTH-BOARD_WIDTH)/2, (HEIGHT-BOARD_HEIGHT)/2, BOARD_WIDTH, BOARD_HEIGHT};
	int holdable = 1;
	int falli = -4;
	int fallj = BOARD_COLS/2 - 2;
	int fallt = rand() % 7;
	int holdt = rand() % 7;
	int rot = 0;
	int ghosti;
	unsigned long long lastFrames = 0, dframes = FPS;
	unsigned long long frames = 0;
	while (!WindowShouldClose()) {
		printf("%d\n", fills);
		if (level != (fills / NUM_FILLS) + 1) {
			dframes /= SPEED_INCREASE;
			level = (fills / NUM_FILLS) + 1;
		}

		frames++;
		if (playing) {
			if (frames - lastFrames > dframes) {
				lastFrames = frames;
				if (isValid(falli+1, fallj, rot, fallt)) {
					falli++;		
				} else {
					int broken = 0;
					holdable = 1;
					int ri, rj;
					for (int i = 0; i < 4; i++) {
						for (int j = 0; j < 4; j++) {
							rotated(i, j, rot, &ri, &rj);
							if (falli+ri < 0) {
								playing = 0;
								continue;
							}
							if (tets[fallt][i][j]){
								polje[falli+ri][fallj+rj] = fallt+1;	
							}
						}
					}
					int full = 1;
					for (int i = 0; i < BOARD_ROWS; i++) {
						full = 1;
						for (int j = 0; j < BOARD_COLS; j++) {
							if (!polje[BOARD_ROWS - i - 1][j]) full = 0;
						}	
						if (full == 1) {
							broken++;
							for (int j = 0; j < BOARD_COLS; j++) {
								polje[BOARD_ROWS - i - 1][j] = 0;
							}
							for (int k = BOARD_ROWS - i - 1; k > 0; k--) {
								for (int j = 0; j < BOARD_COLS; j++) {
									polje[k][j] = polje[k-1][j];
								}
							} 
							i--;
						}
					}
					falli = -4;
					fallj = BOARD_COLS/2 - 2;
					fallt = holdt;
					holdt = rand() % 7;
					rot = 0;
					assert(0 <= broken && broken <= 4);
					score += scores[broken];
					fills += broken;
				}
			}
#ifdef SWITCHING
			if (IsKeyPressed(KEY_P)) {
				if (isValid(falli, fallj, rot, (fallt+1)%7)) {
					fallt = (fallt + 1) % 7;
				}
			}
#endif 
			if (IsKeyDown(KEY_W)) {holdingW++;} else holdingW = 0;
			if (holdingW > HOLDING_FRAME_THRESHOLD || IsKeyPressed(KEY_W)) {
				if (holdingW > HOLDING_FRAME_THRESHOLD) {
					holdingW = HOLDING_FRAME_THRESHOLD - FRAMES_FOR_HOLDING_W; // Drugacije jer se inace bre brzo vrti
				}
				if (isValid(falli, fallj, (rot+1)%4, fallt)) {
					rot = (rot+1)%4;
				}
				else if (isValid(falli, fallj+1, (rot+1)%4, fallt)) {
					rot = (rot+1)%4;
					fallj = fallj + 1;
				}
				else if (isValid(falli, fallj+2, (rot+1)%4, fallt)) {
					rot = (rot+1)%4;
					fallj = fallj + 2;
				}
				else if (isValid(falli, fallj-1, (rot+1)%4, fallt)) {
					rot = (rot+1)%4;
					fallj = fallj - 1;
				}
				else if (isValid(falli, fallj-2, (rot+1)%4, fallt)) {
					rot = (rot+1)%4;
					fallj = fallj - 2;
				}
			}
			if (IsKeyDown(KEY_A)) {holdingA++;} else holdingA = 0;
			if (holdingA > HOLDING_FRAME_THRESHOLD || IsKeyPressed(KEY_A)) {
				if (holdingA > HOLDING_FRAME_THRESHOLD) {
					holdingA = HOLDING_FRAME_THRESHOLD - FRAMES_FOR_HOLDING;
				}
				if (isValid(falli, fallj-1, rot, fallt)) {
					fallj--;
				}
			}
			if (IsKeyDown(KEY_D)) {holdingD++;} else holdingD = 0;
			if (holdingD > HOLDING_FRAME_THRESHOLD || IsKeyPressed(KEY_D)) {
				if (holdingD > HOLDING_FRAME_THRESHOLD) {
					holdingD = HOLDING_FRAME_THRESHOLD - FRAMES_FOR_HOLDING;
				}
				if (isValid(falli, fallj+1, rot, fallt)) {
					fallj++;
				}
			}
			if (IsKeyDown(KEY_S)) {holdingS++;} else holdingS = 0;
			if (holdingS > HOLDING_FRAME_THRESHOLD || IsKeyPressed(KEY_S)) {
				if (holdingS > HOLDING_FRAME_THRESHOLD) {
					holdingS = HOLDING_FRAME_THRESHOLD - FRAMES_FOR_HOLDING;
				}
				if (isValid(falli+1, fallj, rot, fallt)) {
					falli++;
				}
			}
			if (IsKeyPressed(KEY_SPACE)) {
				while (isValid(falli+1, fallj, rot, fallt)) {
					falli++;
				}
				lastFrames = 0;
			}

			if (IsKeyPressed(KEY_H)) {
				if (isValid(falli, fallj, rot, holdt) && holdable) {
					int tmp = fallt;
					fallt = holdt;
					holdt = tmp;
					holdable = 0;
					falli = -4;
				}
			}
			ghosti = falli;
			while (isValid(ghosti+1, fallj, rot, fallt)) {
				ghosti++;
			}
		} else { //not playing
			if (IsKeyPressed(KEY_R)) {
				playing = 1;
				for (int i = 0; i < BOARD_ROWS; i++) {
					for (int j = 0; j < BOARD_COLS; j++) {
						polje[i][j] = 0;
					}
				}
			}
		}
		BeginDrawing();
			ClearBackground(COLOR_BACKGROUND);
		if (playing) {
			DrawRectangleRec(board, GRAY);	
			for (int i = 0; i <= BOARD_ROWS; i++) {
				DrawLineEx(CLITERAL(Vector2){(WIDTH - board.width)/2, board.y  + i*BOARD_HEIGHT/BOARD_ROWS}, CLITERAL(Vector2){WIDTH/2 + board.width/2, board.y+i*BOARD_HEIGHT/BOARD_ROWS}, THICKNESS, RAYWHITE);
			}
			for (int j = 0; j <= BOARD_COLS; j++) {
				DrawLineEx(CLITERAL(Vector2){(WIDTH-board.width)/2 + j*BOARD_WIDTH/BOARD_COLS,(HEIGHT-board.height)/2}, CLITERAL(Vector2){(WIDTH-board.width)/2 + j*BOARD_WIDTH/BOARD_COLS, HEIGHT/2 + board.height/2}, 3, RAYWHITE);
			}
			for (int i = 0; i < BOARD_ROWS; i++) {
				for (int j = 0; j < BOARD_COLS; j++) {
					if (polje[i][j]) {
						DrawTile(i, j, polje[i][j]);
					}
				}
			}
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					int ri, rj;
					rotated(i, j, rot, &ri, &rj);
					if (tets[fallt][i][j]) DrawTile(ri+falli,rj+fallj, fallt+1);
				}	
			}
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					int ri, rj;
					rotated(i, j, rot, &ri, &rj);
					if (tets[fallt][i][j]) DrawGhost(ri+ghosti,rj+fallj, fallt+1);
				}	
			}
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (tets[holdt][i][j]) DrawHold(i+2,j+15, holdt+1, holdable);
				}	
			}
			char str[sizeof(int)];
			sprintf(str, "%d", score);
			DrawText(str, 50, 50, 50, WHITE);
		}
		else {

			char str[sizeof(int)];
			sprintf(str, "%d", score);
			DrawText("GAME_OVER", 50, 50, 50, WHITE);
			DrawText(str, 50, 200, 50, WHITE);
		}
		EndDrawing();
	}
	CloseWindow();
}


