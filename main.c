#include <stdio.h>
#include "raylib.h"
#include <stdbool.h>


const float window_scale = 3;
const int screen_width = 224 * window_scale;
const int screen_height = 288 * window_scale;
const int rows = 36;
const int cols = 28;
const int cell_width = screen_width /cols;
const int cell_height = screen_height /rows;
float fps = 60;


typedef enum {
    right,
    left,
    up,
    down,
    stop,
}Dir;


typedef enum {
    empty = 0,
    pellet = 1,
    big_pellet = 2,
    wall = 3,
    left_teleport = 8,
    right_teleport = 9,
}map_key;


int map[36][28] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,1,1,1,1,1,1,1,1,1,1,1,1,3,3,1,1,1,1,1,1,1,1,1,1,1,1,3},
    {3,1,3,3,3,3,1,3,3,3,3,3,1,3,3,1,3,3,3,3,3,1,3,3,3,3,1,3},
    {3,2,3,3,3,3,1,3,3,3,3,3,1,3,3,1,3,3,3,3,3,1,3,3,3,3,2,3},
    {3,1,3,3,3,3,1,3,3,3,3,3,1,3,3,1,3,3,3,3,3,1,3,3,3,3,1,3},
    {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
    {3,1,3,3,3,3,1,3,3,1,3,3,3,3,3,3,3,3,1,3,3,1,3,3,3,3,1,3},
    {3,1,3,3,3,3,1,3,3,1,3,3,3,3,3,3,3,3,1,3,3,1,3,3,3,3,1,3},
    {3,1,1,1,1,1,1,3,3,1,1,1,1,3,3,1,1,1,1,3,3,1,1,1,1,1,1,3},
    {3,3,3,3,3,3,1,3,3,3,3,3,0,3,3,0,3,3,3,3,3,1,3,3,3,3,3,3},
    {0,0,0,0,0,3,1,3,3,3,3,3,0,3,3,0,3,3,3,3,3,1,3,0,0,0,0,0},
    {0,0,0,0,0,3,1,3,3,0,0,0,0,0,0,0,0,0,0,3,3,1,3,0,0,0,0,0},
    {0,0,0,0,0,3,1,3,3,0,3,3,3,0,0,3,3,3,0,3,3,1,3,0,0,0,0,0},
    {3,3,3,3,3,3,1,3,3,0,3,0,0,0,0,0,0,3,0,3,3,1,3,3,3,3,3,3},
    {0,0,0,0,0,0,1,0,0,0,3,0,0,0,0,0,0,3,0,0,0,1,0,0,0,0,0,0},
    {3,3,3,3,3,3,1,3,3,0,3,0,0,0,0,0,0,3,0,3,3,1,3,3,3,3,3,3},
    {9,0,0,0,0,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,0,0,0,0,8},
    {0,0,0,0,0,3,1,3,3,0,0,0,0,0,0,0,0,0,0,3,3,1,3,0,0,0,0,0},
    {0,0,0,0,0,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,0,0,0,0,0},
    {3,3,3,3,3,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,3,3,3,3,3},
    {3,1,1,1,1,1,1,1,1,1,1,1,1,3,3,1,1,1,1,1,1,1,1,1,1,1,1,3},
    {3,1,3,3,3,3,1,3,3,3,3,3,1,3,3,1,3,3,3,3,3,1,3,3,3,3,1,3},
    {3,1,3,3,3,3,1,3,3,3,3,3,1,3,3,1,3,3,3,3,3,1,3,3,3,3,1,3},
    {3,2,1,1,3,3,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,3,3,1,1,2,3},
    {3,3,3,1,3,3,1,3,3,1,3,3,3,3,3,3,3,3,1,3,3,1,3,3,1,3,3,3},
    {3,3,3,1,3,3,1,3,3,1,3,3,3,3,3,3,3,3,1,3,3,1,3,3,1,3,3,3},
    {3,1,1,1,1,1,1,3,3,1,1,1,1,3,3,1,1,1,1,3,3,1,1,1,1,1,1,3},
    {3,1,3,3,3,3,3,3,3,3,3,3,1,3,3,1,3,3,3,3,3,3,3,3,3,3,1,3},
    {3,1,3,3,3,3,3,3,3,3,3,3,1,3,3,1,3,3,3,3,3,3,3,3,3,3,1,3},
    {3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};


void draw_map() {
    for (int row = 0; row <rows; ++row) {
        for (int col = 0; col<cols; ++col) {
            int x = col * cell_width;
            int y = row * cell_height;
            if (map[row][col] == 0) {
                DrawRectangle(x, y, cell_width, cell_height, BLACK);
            }
            else if (map[row][col] == 3) {
                DrawRectangle(x, y, cell_width, cell_height, MAROON);
            }
            else if (map[row][col] == 1) {
                DrawRectangle(x, y, cell_width, cell_height, SKYBLUE );
            }
            else if (map[row][col] == 2) {
                DrawRectangle(x, y, cell_width, cell_height, ORANGE);
            }
        }
    }
}


void draw_grid() {
    for (int row = 0; row<rows; ++row) {
        for (int col = 0; col<cols; ++col) {
            int x = col * cell_width;
            int y = row * cell_height;
            DrawRectangleLines(x, y, cell_width, cell_height, PINK);
        }
    }
}

bool wanted_dir_check(int player_x_cell, int player_y_cell, Dir wanted_dir) {
    bool can_move = false;

    switch (wanted_dir) {
        case left:{
                      if (map[player_y_cell][player_x_cell-1] !=3) can_move = true;
                  }break;
        case right:{
                       if (map[player_y_cell][player_x_cell+1] !=3) can_move = true;
                   }break;
        case up: {
                     if (map[player_y_cell-1][player_x_cell] !=3) can_move = true;
                 }break;
        case down:{
                      if (map[player_y_cell+1][player_x_cell] !=3) can_move = true;
                  }break;
    }
    return can_move;
}


int main() {
    InitWindow(screen_width, screen_height, "pacman");
    SetTargetFPS(fps);

    float pacman_x = 14 * cell_width;
    float pacman_y = 26 * cell_height;
    int pacman_x_cell = pacman_x/cell_width;
    int pacman_y_cell = pacman_y/cell_height;
    Dir wanted_dir = right;
    Dir curr_dir   = right;

    float pacman_speed = 100;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        draw_map();
        draw_grid();

        float pacman_dp = pacman_speed * GetFrameTime();

        if (IsKeyDown(KEY_LEFT)) {
            wanted_dir = left;
        }
        else if (IsKeyDown(KEY_RIGHT)) {
            wanted_dir = right;
        }
        else if (IsKeyDown(KEY_UP)) {
            wanted_dir = up;
        }
        else if (IsKeyDown(KEY_DOWN)) {
            wanted_dir = down;
        }


        if (curr_dir == left ) {
            int new_x= (pacman_x_cell * cell_width) - cell_width;
            int new_x_cell = new_x/cell_width;
            //DrawRectangle(new_x, pacman_y, cell_width, cell_height, GREEN);
            if (map[pacman_y_cell][new_x_cell] != 3) {
                if (pacman_x < new_x) {
                    pacman_x = new_x;
                    pacman_x_cell -=1;
                    if (wanted_dir_check(new_x_cell, pacman_y_cell, wanted_dir)) {
                        curr_dir = wanted_dir;
                    }
                    if (map[pacman_y_cell][pacman_x_cell] == 1 || map[pacman_y_cell][pacman_x_cell] == 2) {
                        map[pacman_y_cell][pacman_x_cell] = 0;
                    }
                }
                else {
                    pacman_x  -= pacman_dp;
                }
            }
            else {
                curr_dir = wanted_dir;
            }

        }
        else if (curr_dir == right) {
            int new_x = ((pacman_x_cell * cell_width) + cell_width);
            int new_x_cell = new_x/cell_width;
            //DrawRectangle(new_x, pacman_y, cell_width, cell_height, GREEN);
            if (map[pacman_y_cell][new_x_cell] != 3) {
                if (pacman_x >new_x ) {
                    pacman_x = new_x;
                    pacman_x_cell +=1;
                    if (wanted_dir_check(new_x_cell, pacman_y_cell, wanted_dir)) {
                        curr_dir = wanted_dir;
                    }
                    if (map[pacman_y_cell][pacman_x_cell] == 1 || map[pacman_y_cell][pacman_x_cell] == 2) {
                        map[pacman_y_cell][pacman_x_cell] = 0;
                    }
                } else {
                    pacman_x += pacman_dp;
                }
            }
            else {
                curr_dir = wanted_dir;

            }
        }
        else if (curr_dir == up) {
            int new_y =((pacman_y_cell * cell_height) - cell_height) ;
            int new_y_cell = new_y/cell_height;
            // DrawRectangle(pacman_x, new_y, cell_width, cell_height, GREEN);
            if (map[new_y_cell][pacman_x_cell] != 3) {
                if (pacman_y < new_y) {
                    pacman_y = new_y;
                    pacman_y_cell -=1;
                    if (wanted_dir_check(pacman_x_cell, new_y_cell, wanted_dir)) {
                        curr_dir = wanted_dir;
                    }
                    if (map[pacman_y_cell][pacman_x_cell] == 1 || map[pacman_y_cell][pacman_x_cell] == 2) {
                        map[pacman_y_cell][pacman_x_cell] = 0;
                    }
                } 
                else {
                    pacman_y -= pacman_dp;
                }
            }
            else {
                curr_dir = wanted_dir;
            }
        }
        else if (curr_dir == down) {
            int new_y =((pacman_y_cell * cell_height) + cell_height) ;
            int new_y_cell = new_y/cell_height;
            //DrawRectangle(pacman_x, new_y, cell_width, cell_height, GREEN);
            if (map[new_y_cell][pacman_x_cell] != 3) {
                if (pacman_y > new_y) {
                    pacman_y = new_y;
                    pacman_y_cell +=1;
                    if (wanted_dir_check(pacman_x_cell, new_y_cell, wanted_dir)) {
                        curr_dir = wanted_dir;
                    }
                    if (map[pacman_y_cell][pacman_x_cell] == 1 || map[pacman_y_cell][pacman_x_cell] == 2) {
                        map[pacman_y_cell][pacman_x_cell] = 0;
                    }
                }
                else {
                    pacman_y += pacman_dp;
                }
            }
            else {
                curr_dir = wanted_dir;
            }
        }


        DrawRectangle(pacman_x, pacman_y, cell_width, cell_height, YELLOW);

        EndDrawing();
    }
}
