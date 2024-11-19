#include <stdio.h>
#include <stdlib.h>
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
}Dir;


typedef enum {
    empty = 0,
    pellet = 1,
    big_pellet = 2,
    wall = 3,
    left_teleport = 8,
    right_teleport = 9,
}map_key;


typedef struct {
    int x;
    int y;
}V2int;


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
    {8,0,0,0,0,0,1,0,0,0,3,0,0,0,0,0,0,3,0,0,0,1,0,0,0,0,0,9},
    {3,3,3,3,3,3,1,3,3,0,3,0,0,0,0,0,0,3,0,3,3,1,3,3,3,3,3,3},
    {0,0,0,0,0,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,0,0,0,0,0},
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
                DrawRectangle(x, y, cell_width, cell_height, DARKBLUE);
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
            DrawRectangleLines(x, y, cell_width, cell_height, LIME);
        }
    }
}


bool wanted_dir_check(int player_x_cell, int player_y_cell, Dir pacman_wanted_dir) {
    bool can_move = false;

    switch (pacman_wanted_dir) {
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


void update_rg_wanted_dir(Dir rg_curr_dir, Dir *rg_wanted_dir, V2int rg_cell, V2int target_cell) {
    
    Dir x_dir = left;
    Dir y_dir = up;

    int x_dest = rg_cell.x - target_cell.x;
    if (x_dest <0)  {
        x_dest *= -1;
        x_dir = right;
    }

    int y_dest = rg_cell.y - target_cell.y;
    if (y_dest <0) {
        y_dest *= -1;
        y_dir = down;
    }

    if (x_dest < y_dest) {
        *rg_wanted_dir = x_dir;
    }
    else {
        *rg_wanted_dir = y_dir;
    }
    
    bool valid_dir_found = false;

    while (!valid_dir_found) {
        if (*rg_wanted_dir == right) {
            if (left == rg_curr_dir) {
                *rg_wanted_dir = left;
            }
        }
        else if (*rg_wanted_dir == left) {
            if (right == rg_curr_dir) {
                *rg_wanted_dir = right;
            }
        }
        else if (*rg_wanted_dir == down) {
            if (up == rg_curr_dir) {
                *rg_wanted_dir = up;
            }
        }
        else if (*rg_wanted_dir == up) {
            if (down == rg_curr_dir) {
                *rg_wanted_dir = down;
            }

        }
        DrawRectangle(rg_cell.x * cell_width, rg_cell.y * cell_height, cell_width, cell_height, GREEN);
        if (!valid_dir_found) *rg_wanted_dir = rand()%4;
        valid_dir_found = true;
    }
}



void move_player(Dir *curr_dir, Dir *wanted_dir, Vector2 *pos, V2int *cell, float dp) {

    if (*curr_dir == left ) {

        int new_x= (cell->x * cell_width) - cell_width;
        int new_x_cell = new_x/cell_width;
        if (map[cell->y][new_x_cell] != 3) {
            if (pos->x < new_x) {
                pos->x = new_x;
                cell->x -=1;
                if (wanted_dir_check(new_x_cell, cell->y, *wanted_dir)) {
                    *curr_dir = *wanted_dir;
                }
            }
            else {
                pos->x  -= dp;
            }
        }
        else if (map[cell->y][cell->x] == 8) {
            cell->x = 27;
            cell->y = 17;
            pos->x = cell->x * cell_width;
            pos->y = cell->y *cell_height;
        }
        else {
            *curr_dir = *wanted_dir;
        }

    }
    else if (*curr_dir == right) {
        int new_x = ((cell->x * cell_width) + cell_width);
        int new_x_cell = new_x/cell_width;
        if (map[cell->y][new_x_cell] != 3) {
            if (pos->x >new_x ) {
                pos->x = new_x;
                cell->x +=1;
                if (wanted_dir_check(new_x_cell, cell->y, *wanted_dir)) {
                    *curr_dir = *wanted_dir;
                }
            } else {
                pos->x += dp;
            }
        }
        else if (map[cell->y][cell->x] == 9) {
            cell->x = 0;
            cell->y = 17;
            pos->x = cell->x * cell_width;
            pos->y = cell->y *cell_height;
        }
        else {
            *curr_dir = *wanted_dir;

        }
    }
    else if (*curr_dir == up) {
        int new_y =((cell->y * cell_height) - cell_height) ;
        int new_y_cell = new_y/cell_height;
        if (map[new_y_cell][cell->x] != 3) {
            if (pos->y < new_y) {
                pos->y = new_y;
                cell->y -=1;
                if (wanted_dir_check(cell->x, new_y_cell, *wanted_dir)) {
                    *curr_dir = *wanted_dir;
                }
            } 
            else {
                pos->y -= dp;
            }
        }
        else {
            *curr_dir = *wanted_dir;
        }
    }
    else if (*curr_dir == down) {
        int new_y =((cell->y * cell_height) + cell_height) ;
        int new_y_cell = new_y/cell_height;
        if (map[new_y_cell][cell->x] != 3) {
            if (pos->y > new_y) {
                pos->y = new_y;
                cell->y +=1;
                if (wanted_dir_check(cell->x, new_y_cell, *wanted_dir)) {
                    *curr_dir = *wanted_dir;
                }
            }
            else {
                pos->y += dp;
            }
        }
        else {
            *curr_dir = *wanted_dir;
        }
    }

}



int main() {
    InitWindow(screen_width, screen_height, "pacman");
    SetTargetFPS(fps);

    Vector2 pacman_pos = {0};
    pacman_pos.x = 14 * cell_width;
    pacman_pos.y = 26 * cell_height;
    V2int  pacman_cell = {0};
    pacman_cell.x = pacman_pos.x/cell_width;
    pacman_cell.y = pacman_pos.y/cell_height;
    Dir pacman_wanted_dir = right;
    Dir pacman_curr_dir   = right;
    float pacman_speed = 100;


    float ghost_speed = 80;
    float eatable_speed = 60;

    Vector2 rg_pos = {0};
    rg_pos.x = 14 * cell_width;
    rg_pos.y = 14 * cell_height;
    V2int  rg_cell = {0};
    rg_cell.x = rg_pos.x/cell_width;
    rg_cell.y = rg_pos.y/cell_height;
    Dir rg_wanted_dir = right;
    Dir rg_curr_dir   = right;
    bool rg_eatable = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        draw_map();
        //draw_grid();

        float pacman_dp = pacman_speed * GetFrameTime();
        float rg_dp     = ghost_speed * GetFrameTime();

        if (IsKeyDown(KEY_LEFT)) {
            pacman_wanted_dir = left;
        }
        else if (IsKeyDown(KEY_RIGHT)) {
            pacman_wanted_dir = right;
        }
        else if (IsKeyDown(KEY_UP)) {
            pacman_wanted_dir = up;
        }
        else if (IsKeyDown(KEY_DOWN)) {
            pacman_wanted_dir = down;
        }

        if (map[pacman_cell.y][pacman_cell.x] == 1 || map[pacman_cell.y][pacman_cell.x] == 2) {
            map[pacman_cell.y][pacman_cell.x] = 0;
        }

        update_rg_wanted_dir(rg_curr_dir, &rg_wanted_dir, rg_cell, pacman_cell);
        
        move_player(&pacman_curr_dir, &pacman_wanted_dir, &pacman_pos, &pacman_cell, pacman_dp);
        move_player(&rg_curr_dir, &rg_wanted_dir, &rg_pos, &rg_cell, rg_dp);

        DrawRectangle(pacman_pos.x, pacman_pos.y, cell_width, cell_height, YELLOW);
        DrawRectangle(rg_pos.x, rg_pos.y, cell_width, cell_height, RED);

        EndDrawing();
    }

    CloseWindow();
}
