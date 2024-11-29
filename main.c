#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"
#include "math.h"

#define GHOST_COUNT 4



int level_count = 1;
int score = 0;
int pelletes_remaining = 240;

const int pellet_score = 1;
const int ghost_score = 200;
int wait_time_diff = 5;
int chase_speed_diff = 20;
int max_chase_speed = 200;

const float window_scale = 3;
const int screen_width = 224 * window_scale;
const int screen_height = 288 * window_scale;
const int rows = 36;
const int cols = 28;
const int cell_width = screen_width /cols;
const int cell_height = screen_height /rows;
float fps = 60;

bool draw_next_cell = false;

typedef struct {
    int x;
    int y;
}V2int;

typedef enum {
    right,
    left,
    up,
    down,
}Dir;


typedef enum {
    chase,
    scatter,
    scared,
    eaten,
    jail_down,
    jail_up,
    spawn_wait,
    spawn_exit,
}Ghost_Mode;


typedef struct {
    Vector2 pos;
    V2int cell;
    V2int scatter_cell;
    Dir wanted_dir;
    Dir curr_dir;
    Ghost_Mode mode;
    Color color;
    float dp;
    float spawn_wait_time;
}Ghost;

typedef struct {
    float start_time;
    float len;
}Timer;

typedef struct {
    Texture2D texture;
    Vector2 origin;
    Rectangle source;
    Rectangle dest;
    int frames;
    int current_frame;
    float scale;
    float rot;
    Timer timer;
}Anim;

V2int jail_cell = {
    .x = 13,
    .y = 14,
};


V2int spawn_exit_cell = {
    .x = 13,
    .y = 16,
};

typedef enum {
    empty = 0,
    pellet = 1,
    big_pellet = 2,
    wall = 3,
    left_teleport = 8,
    right_teleport = 9,
}map_key;


int map_backup[36][28] = {
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
    {0,0,0,0,0,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,0,0,0,0,0},
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
    {0,0,0,0,0,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,0,0,0,0,0},
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
                DrawRectangleLines(x, y, cell_width, cell_height, BLACK);
            }
            else if (map[row][col] == 3) {
     //           DrawRectangleLines(x, y, cell_width, cell_height, DARKGREEN);
            }
            else if (map[row][col] == 1) {
                DrawRectangle(x + (cell_width * 0.45), y + (cell_height * 0.45), cell_width * 0.15, cell_height * 0.15, BEIGE );
            }
            else if (map[row][col] == 2) {
                DrawCircle(cell_width/2 + x, cell_width/2 + y, cell_width/3,  BEIGE);
            }
        }
    }
}


void draw_grid() {
    for (int row = 0; row<rows; ++row) {
        for (int col = 0; col<cols; ++col) {
            int x = col * cell_width;
            int y = row * cell_height;
            DrawRectangleLines(x, y, cell_width, cell_height, DARKPURPLE);
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



void flip_ghost_dir(Ghost *ghost) {

    if (ghost->curr_dir == right)      {
        if (wanted_dir_check(ghost->cell.x, ghost->cell.y, left)) {
            ghost->curr_dir = left;
            ghost->wanted_dir = left;
        }
    }
    else if (ghost->curr_dir == left)      {
        if (wanted_dir_check(ghost->cell.x, ghost->cell.y, right)) {
            ghost->curr_dir = right;
            ghost->wanted_dir = right;
        }
    }
    else if (ghost->curr_dir == up)      {
        if (wanted_dir_check(ghost->cell.x, ghost->cell.y, down)) {
            ghost->curr_dir = down;
            ghost->wanted_dir = down;
        }
    }
    else if (ghost->curr_dir == down)      {
        if (wanted_dir_check(ghost->cell.x, ghost->cell.y, up)) {
            ghost->curr_dir = up;
            ghost->wanted_dir = up;
        }
    }
}


float get_target_cell_dist(V2int cell, V2int target_cell) {
    float x = target_cell.x - cell.x;
    float y = target_cell.y - cell.y;
    float dist = sqrt((x * x) + (y * y));
    return dist;
}


void update_wanted_dir(Dir curr_dir, Dir *wanted_dir, V2int cell, V2int target_cell) {

    V2int up_cell = {
        .x = cell.x,
        .y = cell.y -1,
    };
    V2int down_cell = {
        .x = cell.x,
        .y = cell.y +1,
    };
    V2int right_cell = {
        .x = cell.x +1,
        .y = cell.y,
    };
    V2int left_cell = {
        .x = cell.x -1,
        .y = cell.y ,
    };

    bool intersection     = false;
    bool left_available   = false;
    bool right_available  = false;
    bool up_available     = false;
    bool down_available   = false;

    int  paths = 0;
    if (wanted_dir_check(right_cell.x, right_cell.y, *wanted_dir) ) {
        right_available = true;
        ++paths;
    }
    if (wanted_dir_check(left_cell.x, left_cell.y, *wanted_dir) ) {
        left_available = true;
        ++paths;
    }
    if (wanted_dir_check(down_cell.x , down_cell.y, *wanted_dir) ) {
        down_available = true;
        ++paths;
    }
    if (wanted_dir_check(up_cell.x , up_cell.y, *wanted_dir) ) {
        up_available = true;
        ++paths;
    }
    if(paths>= 3) intersection = true;

    bool wall_hit = false;
    if (curr_dir == right) {
        if (!right_available) {
            wall_hit = true;
        }
    }
    else if (curr_dir == left) {
        if (!left_available) {
            wall_hit = true;
        }
    }
    else if (curr_dir == down) {
        if (!down_available) {
            wall_hit = true;
        }
    }
    else if (curr_dir == up) {
        if (!up_available) {
            wall_hit = true;
        }
    }


    float unavailable = rows * cols;
    if (wall_hit || intersection) {
        float left_cell_dist  =  get_target_cell_dist(left_cell, target_cell);
        float right_cell_dist =  get_target_cell_dist(right_cell, target_cell);
        float down_cell_dist  =  get_target_cell_dist(down_cell, target_cell);
        float up_cell_dist    =  get_target_cell_dist(up_cell, target_cell);

        if (curr_dir == left)        right_cell_dist = unavailable; 
        else if (curr_dir == right)  left_cell_dist  = unavailable; 
        else if (curr_dir == up)     down_cell_dist  = unavailable; 
        else if (curr_dir == down)   up_cell_dist    = unavailable; 

        if (!left_available)         left_cell_dist  = unavailable;
        if (!right_available)        right_cell_dist = unavailable;
        if (!up_available)           up_cell_dist    = unavailable;
        if (!down_available)         down_cell_dist  = unavailable;

        float smallest_dist = left_cell_dist;
        if (right_cell_dist < smallest_dist)  smallest_dist = right_cell_dist;
        if (down_cell_dist < smallest_dist)   smallest_dist = down_cell_dist;
        if (up_cell_dist < smallest_dist)     smallest_dist = up_cell_dist;

        if (smallest_dist == left_cell_dist)  *wanted_dir = left;
        if (smallest_dist == right_cell_dist) *wanted_dir = right;
        if (smallest_dist == down_cell_dist)  *wanted_dir = down;
        if (smallest_dist == up_cell_dist)    *wanted_dir = up;
    }
}



void move_player(Dir *curr_dir, Dir *wanted_dir, Vector2 *pos, V2int *cell, float dp) {

    if (*curr_dir == left ) {

        int new_x= (cell->x * cell_width) - cell_width;
        int new_x_cell = new_x/cell_width;
        if (draw_next_cell) DrawRectangle(new_x, pos->y, cell_width, cell_height, GRAY);
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
        if (draw_next_cell) DrawRectangle(new_x, pos->y, cell_width, cell_height, GRAY);
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
        if (draw_next_cell) DrawRectangle(pos->x, new_y, cell_width, cell_height, GRAY);
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
        if (draw_next_cell) DrawRectangle(pos->x, new_y, cell_width, cell_height, GRAY);
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

    Texture2D background_texture = LoadTexture("./data/assets/bg.png");
    Rectangle background_source = {0};
    background_source.width = background_texture.width;
    background_source.height = background_texture.height;
    Rectangle background_dest = {0};
    background_dest.width = screen_width;
    background_dest.height = screen_height;


    Vector2 pacman_pos = {0};
    pacman_pos.x = 14 * cell_width;
    pacman_pos.y = 26 * cell_height;
    V2int  pacman_cell = {0};
    pacman_cell.x = pacman_pos.x/cell_width;
    pacman_cell.y = pacman_pos.y/cell_height;
    Dir pacman_wanted_dir = right;
    Dir pacman_curr_dir   = right;
    float pacman_speed = 170;

    Anim pacman_anim = {0};
    int pacman_lives_remain = 2 ;
    pacman_anim.texture = LoadTexture("./data/assets/pacman_eat.png");
    pacman_anim.dest.width = cell_width;
    pacman_anim.dest.height = cell_height;
    pacman_anim.frames = 4;
    pacman_anim.current_frame = 1;
    pacman_anim.scale = 1.85;
    pacman_anim.source.y = 0;
    pacman_anim.source.width = pacman_anim.texture.width / pacman_anim.frames;
    pacman_anim.source.height = pacman_anim.texture.height;
    pacman_anim.rot = 0;
    pacman_anim.timer.len = 0.07;
    pacman_anim.timer.start_time = GetTime();

    bool pacman_invincible = false;
    Timer pacman_pause_eat = {0};
    pacman_pause_eat.start_time = -3;
    pacman_pause_eat.len = 0.3;

    float chase_speed  = 80;
    float scared_speed = 40;
    float eaten_speed  = 200;

    Ghost_Mode chase_scatter_toggle = scatter;

    Anim rg_anim = {0};
    rg_anim.texture = LoadTexture("./data/assets/red_ghost.png");
    rg_anim.dest.width = cell_width;
    rg_anim.dest.height = cell_height;
    rg_anim.frames = 2;
    rg_anim.current_frame = 1;
    rg_anim.scale = 1.55;
    rg_anim.source.y = 0;
    rg_anim.source.width = rg_anim.texture.width / rg_anim.frames;
    rg_anim.source.height = rg_anim.texture.height/4;
    rg_anim.rot = 0;
    rg_anim.timer.len = 0.07;
    rg_anim.timer.start_time = GetTime();
    
    Anim pg_anim = {0};
    pg_anim.texture = LoadTexture("./data/assets/pink_ghost.png");
    pg_anim.dest.width = cell_width;
    pg_anim.dest.height = cell_height;
    pg_anim.frames = 2;
    pg_anim.current_frame = 1;
    pg_anim.scale = 1.55;
    pg_anim.source.y = 0;
    pg_anim.source.width = pg_anim.texture.width / pg_anim.frames;
    pg_anim.source.height = pg_anim.texture.height/4;
    pg_anim.rot = 0;
    pg_anim.timer.len = 0.07;
    pg_anim.timer.start_time = GetTime();
    Anim eaten_ghost_anim = {0};


    Anim og_anim = {0};
    og_anim.texture = LoadTexture("./data/assets/orange_ghost.png");
    og_anim.dest.width = cell_width;
    og_anim.dest.height = cell_height;
    og_anim.frames = 2;
    og_anim.current_frame = 1;
    og_anim.scale = 1.55;
    og_anim.source.y = 0;
    og_anim.source.width = og_anim.texture.width / og_anim.frames;
    og_anim.source.height = og_anim.texture.height/4;
    og_anim.rot = 0;
    og_anim.timer.len = 0.07;
    og_anim.timer.start_time = GetTime();


    Anim bg_anim = {0};
    bg_anim.texture = LoadTexture("./data/assets/blue_ghost.png");
    bg_anim.dest.width = cell_width;
    bg_anim.dest.height = cell_height;
    bg_anim.frames = 2;
    bg_anim.current_frame = 1;
    bg_anim.scale = 1.55;
    bg_anim.source.y = 0;
    bg_anim.source.width = bg_anim.texture.width / bg_anim.frames;
    bg_anim.source.height = bg_anim.texture.height/4;
    bg_anim.rot = 0;
    bg_anim.timer.len = 0.07;
    bg_anim.timer.start_time = GetTime();


    eaten_ghost_anim.texture = LoadTexture("./data/assets/eaten_ghost.png");
    eaten_ghost_anim.dest.width = cell_width;
    eaten_ghost_anim.dest.height = cell_height;
    eaten_ghost_anim.frames = 2;
    eaten_ghost_anim.current_frame = 1;
    eaten_ghost_anim.scale = 1.55;
    eaten_ghost_anim.source.y = 0;
    eaten_ghost_anim.source.width = eaten_ghost_anim.texture.width / eaten_ghost_anim.frames;
    eaten_ghost_anim.source.height = eaten_ghost_anim.texture.height/4;
    eaten_ghost_anim.rot = 0;
    eaten_ghost_anim.timer.len = 0.07;
    eaten_ghost_anim.timer.start_time = GetTime();

    Anim scared_ghost_anim = {0};
    scared_ghost_anim.texture = LoadTexture("./data/assets/scared_ghost.png");
    scared_ghost_anim.dest.width = cell_width;
    scared_ghost_anim.dest.height = cell_height;
    scared_ghost_anim.frames = 4;
    scared_ghost_anim.current_frame = 0;
    scared_ghost_anim.scale = 1.55;
    scared_ghost_anim.source.y = 0;
    scared_ghost_anim.source.width = scared_ghost_anim.texture.width / scared_ghost_anim.frames;
    scared_ghost_anim.source.height = scared_ghost_anim.texture.height;
    scared_ghost_anim.rot = 0;
    scared_ghost_anim.timer.len = 0.07;
    scared_ghost_anim.timer.start_time = GetTime();

    Ghost *ghosts[GHOST_COUNT ] = {0};
    Ghost red_ghost = {0};
    red_ghost.pos.x = 14 * cell_width;
    red_ghost.pos.y = 14 * cell_height;
    red_ghost.cell.x = red_ghost.pos.x/cell_width;
    red_ghost.cell.y = red_ghost.pos.y/cell_height;
    red_ghost.scatter_cell.x = 25;
    red_ghost.scatter_cell.y = 0;
    red_ghost.wanted_dir = right;
    red_ghost.curr_dir   = right;
    red_ghost.mode = scatter;
    red_ghost.color = RED;
    red_ghost.spawn_wait_time = 0;


    Ghost blue_ghost = {0};
    blue_ghost.pos.x = (12 * cell_width) - (cell_width/3);
    blue_ghost.pos.y = 17 * cell_height;
    blue_ghost.cell.x = blue_ghost.pos.x/cell_width;
    blue_ghost.cell.y = blue_ghost.pos.y/cell_height;
    blue_ghost.scatter_cell.x = 27;
    blue_ghost.scatter_cell.y = 35;
    blue_ghost.wanted_dir = right;
    blue_ghost.curr_dir   = right;
    blue_ghost.mode = spawn_wait;
    blue_ghost.color = BLUE;
    blue_ghost.spawn_wait_time = 15;

    Ghost pink_ghost = {0};
    pink_ghost.pos.x = 14 * cell_width;
    pink_ghost.pos.y = 17 * cell_height;
    pink_ghost.cell.x = pink_ghost.pos.x/cell_width;
    pink_ghost.cell.y = pink_ghost.pos.y/cell_height;
    pink_ghost.scatter_cell.x = 2;
    pink_ghost.scatter_cell.y = 0;
    pink_ghost.wanted_dir = left;
    pink_ghost.curr_dir   = left;
    pink_ghost.mode = spawn_wait;
    pink_ghost.color = PINK;
    pink_ghost.spawn_wait_time = 30;

    Ghost orange_ghost = {0};
    orange_ghost.pos.x = (16 * cell_width) - 3 ;
    orange_ghost.pos.y = 17 * cell_height;
    orange_ghost.cell.x = orange_ghost.pos.x/cell_width;
    orange_ghost.cell.y = orange_ghost.pos.y/cell_height;
    orange_ghost.scatter_cell.x = 0;
    orange_ghost.scatter_cell.y = 35;
    orange_ghost.wanted_dir = left;
    orange_ghost.curr_dir   = left;
    orange_ghost.mode = spawn_wait;
    orange_ghost.color = ORANGE;
    orange_ghost.spawn_wait_time = 45;


    typedef enum {
        red = 0,
        blue,
        pink,  
        orange,
    }Ghosts_Index;

    ghosts[red]    = &red_ghost;
    ghosts[blue]   = &blue_ghost;
    ghosts[pink]   = &pink_ghost;
    ghosts[orange] = &orange_ghost;


    float scatter_time = 7;
    Timer scatter_timer = {
        .start_time = GetTime(),
        .len = scatter_time,
    };

    float chase_time = 20;
    Timer chase_timer = {
        .start_time = 0,
        .len = chase_time,
    };

    Timer eat_timer = {0};
    eat_timer.len = 8;

    float spawn_start = GetTime();
    bool running = true;
    while (!WindowShouldClose() && running) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(background_texture, background_source, background_dest,(Vector2){0,0}, 0,  WHITE);
         draw_map();
        // draw_grid();
        float current_time= GetTime();
        float frame_time= GetFrameTime();

        if ((eat_timer.start_time + eat_timer.len) < current_time) {
            for (int i = 0; i <GHOST_COUNT; ++i) {
                if (ghosts[i]->mode == scared)  {
                    flip_ghost_dir(ghosts[i]);


                    ghosts[i]->mode = chase;
                    chase_timer.start_time = current_time;
                }
            }

            if (chase_scatter_toggle == scatter) {
                if ((scatter_timer.start_time + scatter_timer.len) < current_time) {
                    chase_scatter_toggle = chase;
                    chase_timer.start_time = current_time;
                }
            }
            else if(chase_scatter_toggle == chase) {
                if ((chase_timer.start_time + chase_timer.len) < current_time) {
                    chase_scatter_toggle = scatter;
                    scatter_timer.start_time = current_time;
                }
            }
        }


        float pacman_dp = pacman_speed * frame_time;
        for (int i = 0; i <GHOST_COUNT; ++i) {
            if (ghosts[i]->mode == chase || ghosts[i]->mode == scatter) {
                ghosts[i]->mode = chase_scatter_toggle;
            }

            ghosts[i]->dp = 0;
            if (ghosts[i]->mode == scared) {
                ghosts[i]->dp  = scared_speed * frame_time ;
            }
            else if (ghosts[i]->mode == chase || ghosts[i]->mode == scatter || ghosts[i]->mode == spawn_exit) {
                ghosts[i]->dp  = chase_speed * frame_time ; 
            }
            else if (ghosts[i]->mode == eaten || ghosts[i]->mode == jail_down) {
                ghosts[i]->dp = eaten_speed * frame_time;
            }
            else if (ghosts[i]->mode == jail_down || ghosts[i]->mode == jail_up) {
                ghosts[i]->dp  = chase_speed * frame_time ; 
            }
        }


        if (map[pacman_cell.y][pacman_cell.x] == 1 ) {
            map[pacman_cell.y][pacman_cell.x] = 0;
            --pelletes_remaining;
            score += pellet_score;
        }
        else if ( map[pacman_cell.y][pacman_cell.x] == 2 ) {
            for (int i = 0; i<GHOST_COUNT; ++i) {
                if (ghosts[i]->mode == scatter || ghosts[i]->mode == chase)  flip_ghost_dir(ghosts[i]);
                map[pacman_cell.y][pacman_cell.x] = 0;
                eat_timer.start_time = current_time;
                if (ghosts[i]->mode == scatter || ghosts[i]->mode == chase)  ghosts[i]->mode = scared;
            }

        }


        for (int i = 0; i <GHOST_COUNT; ++i) {
            bool collision = ( pacman_cell.x == ghosts[i]->cell.x && pacman_cell.y == ghosts[i]->cell.y);
            if (collision) {
                if (ghosts[i]->mode == scared) {
                    ghosts[i]->mode = eaten;
                    pacman_pause_eat.start_time = current_time;
                    score += ghost_score;
                }            
                else if (ghosts[i]->mode == chase || ghosts[i]->mode == scatter){
                    pacman_lives_remain -= 1;
                    pacman_pos.x = 14 * cell_width;
                    pacman_pos.y = 26 * cell_height;
                    pacman_cell.x = pacman_pos.x/cell_width;
                    pacman_cell.y = pacman_pos.y/cell_height;
                    pacman_wanted_dir = right;
                    pacman_curr_dir   = right;

                    red_ghost.pos.x = 14 * cell_width;
                    red_ghost.pos.y = 14 * cell_height;
                    red_ghost.cell.x = red_ghost.pos.x/cell_width;
                    red_ghost.cell.y = red_ghost.pos.y/cell_height;
                    red_ghost.wanted_dir = right;
                    red_ghost.curr_dir   = right;
                    red_ghost.spawn_wait_time = 0;
                    red_ghost.mode = scatter; 
                    red_ghost.color = RED;

                    blue_ghost.pos.x = (12 * cell_width) - (cell_width/3);
                    blue_ghost.pos.y = 17 * cell_height;
                    blue_ghost.cell.x = blue_ghost.pos.x/cell_width;
                    blue_ghost.cell.y = blue_ghost.pos.y/cell_height;
                    blue_ghost.wanted_dir = right;
                    blue_ghost.curr_dir   = right;
                    blue_ghost.mode = spawn_wait;
                    blue_ghost.color = BLUE;
                    blue_ghost.spawn_wait_time = 15;
                    blue_ghost.spawn_wait_time -= wait_time_diff;
                    if (blue_ghost.spawn_wait_time < 0) blue_ghost.spawn_wait_time = 0;


                    pink_ghost.pos.x = 14 * cell_width;
                    pink_ghost.pos.y = 17 * cell_height;
                    pink_ghost.cell.x = pink_ghost.pos.x/cell_width;
                    pink_ghost.cell.y = pink_ghost.pos.y/cell_height;
                    pink_ghost.wanted_dir = left;
                    pink_ghost.curr_dir   = left;
                    pink_ghost.mode = spawn_wait;
                    pink_ghost.color = PINK;
                    pink_ghost.spawn_wait_time = 2;
                    pink_ghost.spawn_wait_time = 30;
                    pink_ghost.spawn_wait_time -= wait_time_diff;
                    if (pink_ghost.spawn_wait_time < 0) pink_ghost.spawn_wait_time = 0;

                    orange_ghost.pos.x = (16 * cell_width) - 3 ;
                    orange_ghost.pos.y = 17 * cell_height;
                    orange_ghost.cell.x = orange_ghost.pos.x/cell_width;
                    orange_ghost.cell.y = orange_ghost.pos.y/cell_height;
                    orange_ghost.wanted_dir = left;
                    orange_ghost.curr_dir   = left;
                    orange_ghost.mode = spawn_wait;
                    orange_ghost.color = ORANGE;
                    orange_ghost.spawn_wait_time = 45;
                    orange_ghost.spawn_wait_time -= wait_time_diff;
                    if (orange_ghost.spawn_wait_time < 0) orange_ghost.spawn_wait_time = 0;

                    spawn_start = current_time;


                }
            }

            if (ghosts[i]->mode == chase) {
                if (i == red) {
                    update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, pacman_cell);
                    ghosts[i]->color = RED;
                }
                else if (i == blue) {
                    V2int target_blue_cell = {
                        .x = pacman_cell.x,
                        .y = pacman_cell.y
                    };
                    if (pacman_wanted_dir == left) {
                        target_blue_cell.x -=2;
                    }
                    else if (pacman_wanted_dir == right) {
                        target_blue_cell.x +=2;
                    }
                    else if (pacman_wanted_dir == down) {
                        target_blue_cell.y +=2;
                    }
                    else if (pacman_wanted_dir == up) {
                        target_blue_cell.y -=2;
                    }
                    target_blue_cell.x += (target_blue_cell.x - ghosts[red]->cell.x );
                    target_blue_cell.y += (target_blue_cell.y - ghosts[red]->cell.y );
                    update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, target_blue_cell);
                    ghosts[i]->color = BLUE;
                }
                else if (i == pink) {
                    V2int target_pink_cell = {
                        .x = pacman_cell.x,
                        .y = pacman_cell.y
                    };
                    if (pacman_wanted_dir == left) {
                        target_pink_cell.x -=4;
                    }
                    else if (pacman_wanted_dir == right) {
                        target_pink_cell.x +=4;
                    }
                    else if (pacman_wanted_dir == down) {
                        target_pink_cell.y +=4;
                    }
                    else if (pacman_wanted_dir == up) {
                        target_pink_cell.y -=4;
                    }
                    update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, target_pink_cell);
                    ghosts[i]->color = PINK;
                }
                else if (i == orange) {
                    float rad = sqrt( ((pacman_cell.x - ghosts[i]->cell.x) * (pacman_cell.x - ghosts[i]->cell.x)) +  ((pacman_cell.y - ghosts[i]->cell.y) * (pacman_cell.y - ghosts[i]->cell.y)) );
                    if (rad >8) update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, pacman_cell);
                    else update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, ghosts[i]->scatter_cell);

                    ghosts[i]->color = ORANGE;
                }
            }
            else if (ghosts[i]->mode == scatter) {
                update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, ghosts[i]->scatter_cell);
                if (i == red) {
                    ghosts[i]->color = RED;
                }
                else if (i == blue) {
                    ghosts[i]->color = BLUE;
                }
                else if (i == pink) {
                    ghosts[i]->color = PINK;
                }
                else if (i == orange) {
                    ghosts[i]->color = ORANGE;
                }
            }
            else if (ghosts[i]->mode == scared) {
                V2int rand_target_cell = {
                    .x = rand()%cols,
                    .y = rand()%rows,
                };
                update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, rand_target_cell);
                ghosts[i]->color = DARKBLUE;
            }
            else if (ghosts[i]->mode == eaten) {
                update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, jail_cell);
                ghosts[i]->color = RAYWHITE;

                bool collision = false;
                if (ghosts[i]->curr_dir == left && ghosts[i]->cell.y == jail_cell.y && ghosts[i]->pos.x < ((jail_cell.x *cell_width) + cell_width/1.6)) {
                    collision = true;
                }
                else if (ghosts[i]->curr_dir == right && ghosts[i]->cell.y == jail_cell.y && ghosts[i]->pos.x > ((jail_cell.x *cell_width) + cell_width/2)) {
                    collision = true;
                }
                if (collision) {
                    ghosts[i]->mode = jail_down;
                }
            }
            else if(ghosts[i]->mode == spawn_wait) {
                if ( (spawn_start + ghosts[i]->spawn_wait_time) < current_time) {
                    ghosts[i]->mode = spawn_exit;
                }
            }
            else if (ghosts[i]->mode == spawn_exit) {
                update_wanted_dir(ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, ghosts[i]->cell, spawn_exit_cell);
                if (i == orange) {
                    if (ghosts[i]->pos.x < ((spawn_exit_cell.x * cell_width) + cell_width/2)) {
                        ghosts[i]->cell.x = spawn_exit_cell.x;
                        ghosts[i]->pos.x = (spawn_exit_cell.x * cell_width) + cell_width/2;
                        ghosts[i]->mode = jail_up;
                    }
                }
                else if (i == pink) {
                    if (ghosts[i]->pos.x < ((spawn_exit_cell.x * cell_width) + cell_width/2)) {
                        ghosts[i]->cell.x = spawn_exit_cell.x;
                        ghosts[i]->pos.x = (spawn_exit_cell.x * cell_width) + cell_width/2;
                        ghosts[i]->mode = jail_up;
                    }
                }
                else if (i == blue) {
                    ghosts[i]->curr_dir = right;
                    ghosts[i]->wanted_dir = right;
                    if (ghosts[i]->pos.x > ((spawn_exit_cell.x * cell_width) + cell_width/2)) {
                        ghosts[i]->cell.x = spawn_exit_cell.x;
                        ghosts[i]->pos.x = (spawn_exit_cell.x * cell_width) + cell_width/2;
                        ghosts[i]->mode = jail_up;
                    }
                }
            }

            if (ghosts[i]->mode == jail_down) {
                ghosts[i]->pos.y += ghosts[i]->dp;
                if (ghosts[i]->pos.y > ((jail_cell.y +3) * cell_height)) {
                    ghosts[i]->mode = jail_up;
                }
            }
            else if (ghosts[i]->mode == jail_up) {
                ghosts[i]->pos.y -=  ghosts[i]->dp;
                if (ghosts[i]->pos.y < (jail_cell.y * cell_height)) {
                    ghosts[i]->pos.y = jail_cell.y * cell_height;
                    ghosts[i]->cell.y = jail_cell.y;
                    ghosts[i]->cell.x = jail_cell.x;
                    ghosts[i]->mode = chase;
                }
            }
            else {
                move_player(&ghosts[i]->curr_dir, &ghosts[i]->wanted_dir, &ghosts[i]->pos, &ghosts[i]->cell, ghosts[i]->dp);
            }

        }

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

        if (! ( (pacman_pause_eat.start_time + pacman_pause_eat.len) > current_time)) {
            move_player(&pacman_curr_dir, &pacman_wanted_dir, &pacman_pos, &pacman_cell, pacman_dp);
            pacman_invincible = false;
        }
        else {
            pacman_invincible = true;
        }


        for (int i = 0; i <GHOST_COUNT; ++i) {
            if (i == red) {
                if (ghosts[i]->mode == eaten || ghosts[i]->mode == jail_down ) {
                    eaten_ghost_anim.origin.x = eaten_ghost_anim.scale * (cell_width/2.0);
                    eaten_ghost_anim.origin.y = eaten_ghost_anim.scale * (cell_width/2.0);
                    eaten_ghost_anim.dest.x = ghosts[i]->pos.x + eaten_ghost_anim.origin.x/2;
                    eaten_ghost_anim.dest.y = ghosts[i]->pos.y + eaten_ghost_anim.origin.y/2;
                    eaten_ghost_anim.dest.width = eaten_ghost_anim.scale * (cell_width);
                    eaten_ghost_anim.dest.height =eaten_ghost_anim.scale * ( cell_height);

                    eaten_ghost_anim.source.x = eaten_ghost_anim.current_frame * (eaten_ghost_anim.texture.width/eaten_ghost_anim.frames);
                    if ( (eaten_ghost_anim.timer.start_time + eaten_ghost_anim.timer.len) < current_time) {
                        eaten_ghost_anim.timer.start_time = current_time;
                        ++eaten_ghost_anim.current_frame;
                    }

                    if (eaten_ghost_anim.current_frame > eaten_ghost_anim.frames) eaten_ghost_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      eaten_ghost_anim.source.y = 1 * (eaten_ghost_anim.texture.height/4);
                                      DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       eaten_ghost_anim.source.y = 0 * (eaten_ghost_anim.texture.height/4);
                                       DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    eaten_ghost_anim.source.y = 2 * (eaten_ghost_anim.texture.height/4);
                                    DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      eaten_ghost_anim.source.y = 3 * (eaten_ghost_anim.texture.height/4);
                                      DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                  }break;
                    }
                
                }
                else if (ghosts[i]->mode == scared) {
                    scared_ghost_anim.origin.x = scared_ghost_anim.scale * (cell_width/2.0);
                    scared_ghost_anim.origin.y = scared_ghost_anim.scale * (cell_width/2.0);
                    scared_ghost_anim.dest.x = ghosts[i]->pos.x + scared_ghost_anim.origin.x/2;
                    scared_ghost_anim.dest.y = ghosts[i]->pos.y + scared_ghost_anim.origin.y/2;
                    scared_ghost_anim.dest.width = scared_ghost_anim.scale * (cell_width);
                    scared_ghost_anim.dest.height =scared_ghost_anim.scale * ( cell_height);

                    scared_ghost_anim.source.x = scared_ghost_anim.current_frame * (scared_ghost_anim.texture.width/scared_ghost_anim.frames);
                    if ( (scared_ghost_anim.timer.start_time + scared_ghost_anim.timer.len) < current_time) {
                        scared_ghost_anim.timer.start_time = current_time;
                        ++scared_ghost_anim.current_frame;
                    }

                    if (scared_ghost_anim.current_frame > scared_ghost_anim.frames) scared_ghost_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                  }break;
                    }
                }
                else {
                    rg_anim.origin.x = rg_anim.scale * (cell_width/2.0);
                    rg_anim.origin.y = rg_anim.scale * (cell_width/2.0);
                    rg_anim.dest.x = ghosts[i]->pos.x + rg_anim.origin.x/2;
                    rg_anim.dest.y = ghosts[i]->pos.y + rg_anim.origin.y/2;
                    rg_anim.dest.width = rg_anim.scale * (cell_width);
                    rg_anim.dest.height =rg_anim.scale * ( cell_height);

                    rg_anim.source.x = rg_anim.current_frame * (rg_anim.texture.width/rg_anim.frames);
                    if ( (rg_anim.timer.start_time + rg_anim.timer.len) < current_time) {
                        rg_anim.timer.start_time = current_time;
                        ++rg_anim.current_frame;
                    }

                    if (rg_anim.current_frame > rg_anim.frames) rg_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      rg_anim.source.y = 1 * (rg_anim.texture.height/4);
                                      DrawTexturePro(rg_anim.texture, rg_anim.source, rg_anim.dest, rg_anim.origin,  rg_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       rg_anim.source.y = 0 * (rg_anim.texture.height/4);
                                       DrawTexturePro(rg_anim.texture, rg_anim.source, rg_anim.dest, rg_anim.origin,  rg_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    rg_anim.source.y = 2 * (rg_anim.texture.height/4);
                                    DrawTexturePro(rg_anim.texture, rg_anim.source, rg_anim.dest, rg_anim.origin,  rg_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      rg_anim.source.y = 3 * (rg_anim.texture.height/4);
                                      DrawTexturePro(rg_anim.texture, rg_anim.source, rg_anim.dest, rg_anim.origin,  rg_anim.rot, WHITE); 
                                  }break;
                    }
                }
            }
            else if (i==pink) {
                if (ghosts[i]->mode == eaten || ghosts[i]->mode == jail_down ) {
                    eaten_ghost_anim.origin.x = eaten_ghost_anim.scale * (cell_width/2.0);
                    eaten_ghost_anim.origin.y = eaten_ghost_anim.scale * (cell_width/2.0);
                    eaten_ghost_anim.dest.x = ghosts[i]->pos.x + eaten_ghost_anim.origin.x/2;
                    eaten_ghost_anim.dest.y = ghosts[i]->pos.y + eaten_ghost_anim.origin.y/2;
                    eaten_ghost_anim.dest.width = eaten_ghost_anim.scale * (cell_width);
                    eaten_ghost_anim.dest.height =eaten_ghost_anim.scale * ( cell_height);

                    eaten_ghost_anim.source.x = eaten_ghost_anim.current_frame * (eaten_ghost_anim.texture.width/eaten_ghost_anim.frames);
                    if ( (eaten_ghost_anim.timer.start_time + eaten_ghost_anim.timer.len) < current_time) {
                        eaten_ghost_anim.timer.start_time = current_time;
                        ++eaten_ghost_anim.current_frame;
                    }

                    if (eaten_ghost_anim.current_frame > eaten_ghost_anim.frames) eaten_ghost_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      eaten_ghost_anim.source.y = 1 * (eaten_ghost_anim.texture.height/4);
                                      DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       eaten_ghost_anim.source.y = 0 * (eaten_ghost_anim.texture.height/4);
                                       DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    eaten_ghost_anim.source.y = 2 * (eaten_ghost_anim.texture.height/4);
                                    DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      eaten_ghost_anim.source.y = 3 * (eaten_ghost_anim.texture.height/4);
                                      DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                  }break;
                    }
                
                }
                else if (ghosts[i]->mode == scared) {
                    scared_ghost_anim.origin.x = scared_ghost_anim.scale * (cell_width/2.0);
                    scared_ghost_anim.origin.y = scared_ghost_anim.scale * (cell_width/2.0);
                    scared_ghost_anim.dest.x = ghosts[i]->pos.x + scared_ghost_anim.origin.x/2;
                    scared_ghost_anim.dest.y = ghosts[i]->pos.y + scared_ghost_anim.origin.y/2;
                    scared_ghost_anim.dest.width = scared_ghost_anim.scale * (cell_width);
                    scared_ghost_anim.dest.height =scared_ghost_anim.scale * ( cell_height);

                    scared_ghost_anim.source.x = scared_ghost_anim.current_frame * (scared_ghost_anim.texture.width/scared_ghost_anim.frames);
                    if ( (scared_ghost_anim.timer.start_time + scared_ghost_anim.timer.len) < current_time) {
                        scared_ghost_anim.timer.start_time = current_time;
                        ++scared_ghost_anim.current_frame;
                    }

                    if (scared_ghost_anim.current_frame > scared_ghost_anim.frames) scared_ghost_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                  }break;
                    }
                }
                else {
                    pg_anim.origin.x = pg_anim.scale * (cell_width/2.0);
                    pg_anim.origin.y = pg_anim.scale * (cell_width/2.0);
                    pg_anim.dest.x = ghosts[i]->pos.x + pg_anim.origin.x/2;
                    pg_anim.dest.y = ghosts[i]->pos.y + pg_anim.origin.y/2;
                    pg_anim.dest.width = pg_anim.scale * (cell_width);
                    pg_anim.dest.height =pg_anim.scale * ( cell_height);

                    pg_anim.source.x = pg_anim.current_frame * (pg_anim.texture.width/pg_anim.frames);
                    if ( (pg_anim.timer.start_time + pg_anim.timer.len) < current_time) {
                        pg_anim.timer.start_time = current_time;
                        ++pg_anim.current_frame;
                    }

                    if (pg_anim.current_frame > pg_anim.frames) pg_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      pg_anim.source.y = 1 * (pg_anim.texture.height/4);
                                      DrawTexturePro(pg_anim.texture, pg_anim.source, pg_anim.dest, pg_anim.origin,  pg_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       pg_anim.source.y = 0 * (pg_anim.texture.height/4);
                                       DrawTexturePro(pg_anim.texture, pg_anim.source, pg_anim.dest, pg_anim.origin,  pg_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    pg_anim.source.y = 2 * (pg_anim.texture.height/4);
                                    DrawTexturePro(pg_anim.texture, pg_anim.source, pg_anim.dest, pg_anim.origin,  pg_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      pg_anim.source.y = 3 * (pg_anim.texture.height/4);
                                      DrawTexturePro(pg_anim.texture, pg_anim.source, pg_anim.dest, pg_anim.origin,  pg_anim.rot, WHITE); 
                                  }break;
                    }
                }
            }
            else if (i==blue) {
                if (ghosts[i]->mode == eaten || ghosts[i]->mode == jail_down ) {
                    eaten_ghost_anim.origin.x = eaten_ghost_anim.scale * (cell_width/2.0);
                    eaten_ghost_anim.origin.y = eaten_ghost_anim.scale * (cell_width/2.0);
                    eaten_ghost_anim.dest.x = ghosts[i]->pos.x + eaten_ghost_anim.origin.x/2;
                    eaten_ghost_anim.dest.y = ghosts[i]->pos.y + eaten_ghost_anim.origin.y/2;
                    eaten_ghost_anim.dest.width = eaten_ghost_anim.scale * (cell_width);
                    eaten_ghost_anim.dest.height =eaten_ghost_anim.scale * ( cell_height);

                    eaten_ghost_anim.source.x = eaten_ghost_anim.current_frame * (eaten_ghost_anim.texture.width/eaten_ghost_anim.frames);
                    if ( (eaten_ghost_anim.timer.start_time + eaten_ghost_anim.timer.len) < current_time) {
                        eaten_ghost_anim.timer.start_time = current_time;
                        ++eaten_ghost_anim.current_frame;
                    }

                    if (eaten_ghost_anim.current_frame > eaten_ghost_anim.frames) eaten_ghost_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      eaten_ghost_anim.source.y = 1 * (eaten_ghost_anim.texture.height/4);
                                      DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       eaten_ghost_anim.source.y = 0 * (eaten_ghost_anim.texture.height/4);
                                       DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    eaten_ghost_anim.source.y = 2 * (eaten_ghost_anim.texture.height/4);
                                    DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      eaten_ghost_anim.source.y = 3 * (eaten_ghost_anim.texture.height/4);
                                      DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                  }break;
                    }
                
                }
                else if (ghosts[i]->mode == scared) {
                    scared_ghost_anim.origin.x = scared_ghost_anim.scale * (cell_width/2.0);
                    scared_ghost_anim.origin.y = scared_ghost_anim.scale * (cell_width/2.0);
                    scared_ghost_anim.dest.x = ghosts[i]->pos.x + scared_ghost_anim.origin.x/2;
                    scared_ghost_anim.dest.y = ghosts[i]->pos.y + scared_ghost_anim.origin.y/2;
                    scared_ghost_anim.dest.width = scared_ghost_anim.scale * (cell_width);
                    scared_ghost_anim.dest.height =scared_ghost_anim.scale * ( cell_height);

                    scared_ghost_anim.source.x = scared_ghost_anim.current_frame * (scared_ghost_anim.texture.width/scared_ghost_anim.frames);
                    if ( (scared_ghost_anim.timer.start_time + scared_ghost_anim.timer.len) < current_time) {
                        scared_ghost_anim.timer.start_time = current_time;
                        ++scared_ghost_anim.current_frame;
                    }

                    if (scared_ghost_anim.current_frame > scared_ghost_anim.frames) scared_ghost_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                  }break;
                    }
                }
                else {
                    bg_anim.origin.x = bg_anim.scale * (cell_width/2.0);
                    bg_anim.origin.y = bg_anim.scale * (cell_width/2.0);
                    bg_anim.dest.x = ghosts[i]->pos.x + bg_anim.origin.x/2;
                    bg_anim.dest.y = ghosts[i]->pos.y + bg_anim.origin.y/2;
                    bg_anim.dest.width = bg_anim.scale * (cell_width);
                    bg_anim.dest.height =bg_anim.scale * ( cell_height);

                    bg_anim.source.x = bg_anim.current_frame * (bg_anim.texture.width/bg_anim.frames);
                    if ( (bg_anim.timer.start_time + bg_anim.timer.len) < current_time) {
                        bg_anim.timer.start_time = current_time;
                        ++bg_anim.current_frame;
                    }

                    if (bg_anim.current_frame > bg_anim.frames) bg_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      bg_anim.source.y = 1 * (bg_anim.texture.height/4);
                                      DrawTexturePro(bg_anim.texture, bg_anim.source, bg_anim.dest, bg_anim.origin,  bg_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       bg_anim.source.y = 0 * (bg_anim.texture.height/4);
                                       DrawTexturePro(bg_anim.texture, bg_anim.source, bg_anim.dest, bg_anim.origin,  bg_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    bg_anim.source.y = 2 * (bg_anim.texture.height/4);
                                    DrawTexturePro(bg_anim.texture, bg_anim.source, bg_anim.dest, bg_anim.origin,  bg_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      bg_anim.source.y = 3 * (bg_anim.texture.height/4);
                                      DrawTexturePro(bg_anim.texture, bg_anim.source, bg_anim.dest, bg_anim.origin,  bg_anim.rot, WHITE); 
                                  }break;
                    }
                }
            }
            else if (i == orange) {
                if (ghosts[i]->mode == eaten || ghosts[i]->mode == jail_down ) {
                    eaten_ghost_anim.origin.x = eaten_ghost_anim.scale * (cell_width/2.0);
                    eaten_ghost_anim.origin.y = eaten_ghost_anim.scale * (cell_width/2.0);
                    eaten_ghost_anim.dest.x = ghosts[i]->pos.x + eaten_ghost_anim.origin.x/2;
                    eaten_ghost_anim.dest.y = ghosts[i]->pos.y + eaten_ghost_anim.origin.y/2;
                    eaten_ghost_anim.dest.width = eaten_ghost_anim.scale * (cell_width);
                    eaten_ghost_anim.dest.height =eaten_ghost_anim.scale * ( cell_height);

                    eaten_ghost_anim.source.x = eaten_ghost_anim.current_frame * (eaten_ghost_anim.texture.width/eaten_ghost_anim.frames);
                    if ( (eaten_ghost_anim.timer.start_time + eaten_ghost_anim.timer.len) < current_time) {
                        eaten_ghost_anim.timer.start_time = current_time;
                        ++eaten_ghost_anim.current_frame;
                    }

                    if (eaten_ghost_anim.current_frame > eaten_ghost_anim.frames) eaten_ghost_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      eaten_ghost_anim.source.y = 1 * (eaten_ghost_anim.texture.height/4);
                                      DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       eaten_ghost_anim.source.y = 0 * (eaten_ghost_anim.texture.height/4);
                                       DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    eaten_ghost_anim.source.y = 2 * (eaten_ghost_anim.texture.height/4);
                                    DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      eaten_ghost_anim.source.y = 3 * (eaten_ghost_anim.texture.height/4);
                                      DrawTexturePro(eaten_ghost_anim.texture, eaten_ghost_anim.source, eaten_ghost_anim.dest, eaten_ghost_anim.origin,  eaten_ghost_anim.rot, WHITE); 
                                  }break;
                    }
                
                }
                else if (ghosts[i]->mode == scared) {
                    scared_ghost_anim.origin.x = scared_ghost_anim.scale * (cell_width/2.0);
                    scared_ghost_anim.origin.y = scared_ghost_anim.scale * (cell_width/2.0);
                    scared_ghost_anim.dest.x = ghosts[i]->pos.x + scared_ghost_anim.origin.x/2;
                    scared_ghost_anim.dest.y = ghosts[i]->pos.y + scared_ghost_anim.origin.y/2;
                    scared_ghost_anim.dest.width = scared_ghost_anim.scale * (cell_width);
                    scared_ghost_anim.dest.height =scared_ghost_anim.scale * ( cell_height);

                    scared_ghost_anim.source.x = scared_ghost_anim.current_frame * (scared_ghost_anim.texture.width/scared_ghost_anim.frames);
                    if ( (scared_ghost_anim.timer.start_time + scared_ghost_anim.timer.len) < current_time) {
                        scared_ghost_anim.timer.start_time = current_time;
                        ++scared_ghost_anim.current_frame;
                    }

                    if (scared_ghost_anim.current_frame > scared_ghost_anim.frames) scared_ghost_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      DrawTexturePro(scared_ghost_anim.texture, scared_ghost_anim.source, scared_ghost_anim.dest, scared_ghost_anim.origin,  scared_ghost_anim.rot, WHITE); 
                                  }break;
                    }
                }
                else {
                    og_anim.origin.x = og_anim.scale * (cell_width/2.0);
                    og_anim.origin.y = og_anim.scale * (cell_width/2.0);
                    og_anim.dest.x = ghosts[i]->pos.x + og_anim.origin.x/2;
                    og_anim.dest.y = ghosts[i]->pos.y + og_anim.origin.y/2;
                    og_anim.dest.width = og_anim.scale * (cell_width);
                    og_anim.dest.height =og_anim.scale * ( cell_height);

                    og_anim.source.x = og_anim.current_frame * (og_anim.texture.width/og_anim.frames);
                    if ( (og_anim.timer.start_time + og_anim.timer.len) < current_time) {
                        og_anim.timer.start_time = current_time;
                        ++og_anim.current_frame;
                    }

                    if (og_anim.current_frame > og_anim.frames) og_anim.current_frame = 0;
                    switch (ghosts[i]->curr_dir) {
                        case left:{
                                      og_anim.source.y = 1 * (og_anim.texture.height/4);
                                      DrawTexturePro(og_anim.texture, og_anim.source, og_anim.dest, og_anim.origin,  og_anim.rot, WHITE); 
                                  }break;
                        case right:{
                                       og_anim.source.y = 0 * (og_anim.texture.height/4);
                                       DrawTexturePro(og_anim.texture, og_anim.source, og_anim.dest, og_anim.origin,  og_anim.rot, WHITE); 
                                   }break;
                        case up:{
                                    og_anim.source.y = 2 * (og_anim.texture.height/4);
                                    DrawTexturePro(og_anim.texture, og_anim.source, og_anim.dest, og_anim.origin,  og_anim.rot, WHITE); 
                                }break;
                        case down:{
                                      og_anim.source.y = 3 * (og_anim.texture.height/4);
                                      DrawTexturePro(og_anim.texture, og_anim.source, og_anim.dest, og_anim.origin,  og_anim.rot, WHITE); 
                                  }break;
                    }
                }
            }
        }

        pacman_anim.origin.x = pacman_anim.scale * (cell_width/2.0);
        pacman_anim.origin.y = pacman_anim.scale * (cell_width/2.0);
        pacman_anim.dest.x = pacman_pos.x + pacman_anim.origin.x/2;
        pacman_anim.dest.y = pacman_pos.y + pacman_anim.origin.y/2;
        pacman_anim.dest.width = pacman_anim.scale * (cell_width);
        pacman_anim.dest.height =pacman_anim.scale * ( cell_height);

        pacman_anim.source.x = pacman_anim.current_frame * (pacman_anim.texture.width/pacman_anim.frames);
        if ( (pacman_anim.timer.start_time + pacman_anim.timer.len) < current_time) {

            pacman_anim.timer.start_time = current_time;
            if (!wanted_dir_check(pacman_cell.x, pacman_cell.y, pacman_wanted_dir) && pacman_wanted_dir == pacman_curr_dir) {
            }
            else {
                ++pacman_anim.current_frame;
            }
        }

        if (pacman_anim.current_frame > pacman_anim.frames) pacman_anim.current_frame = 0;
        switch (pacman_curr_dir) {
            case left:{
                          pacman_anim.rot = 180;
                      }break;
            case right:{
                           pacman_anim.rot = 0;
                       }break;
            case up:{
                        pacman_anim.rot = 270;
                    }break;
            case down:{
                          pacman_anim.rot = 90;
                      }break;
        }

        if (! pacman_invincible) {
            DrawTexturePro(pacman_anim.texture, pacman_anim.source, pacman_anim.dest, pacman_anim.origin,  pacman_anim.rot, YELLOW); 
        }
        else {
            DrawText(TextFormat("%d", ghost_score), pacman_pos.x, pacman_pos.y,  20, BLUE);
        }
        for (int i = 0; i <pacman_lives_remain; ++i) {
            int live_x = cell_width * i;
            DrawCircle(live_x + cell_width/2, 35 *  cell_height, cell_width/2.4, YELLOW);                              

        }
        if (pelletes_remaining == 0) {
            pelletes_remaining = 240;

                    pacman_pos.x = 14 * cell_width;
                    pacman_pos.y = 26 * cell_height;
                    pacman_cell.x = pacman_pos.x/cell_width;
                    pacman_cell.y = pacman_pos.y/cell_height;
                    pacman_wanted_dir = right;
                    pacman_curr_dir   = right;

                    red_ghost.pos.x = 14 * cell_width;
                    red_ghost.pos.y = 14 * cell_height;
                    red_ghost.cell.x = red_ghost.pos.x/cell_width;
                    red_ghost.cell.y = red_ghost.pos.y/cell_height;
                    red_ghost.wanted_dir = right;
                    red_ghost.curr_dir   = right;
                    red_ghost.spawn_wait_time = 0;
                    red_ghost.mode = scatter; 
                    red_ghost.color = RED;

                    blue_ghost.pos.x = (12 * cell_width) - (cell_width/3);
                    blue_ghost.pos.y = 17 * cell_height;
                    blue_ghost.cell.x = blue_ghost.pos.x/cell_width;
                    blue_ghost.cell.y = blue_ghost.pos.y/cell_height;
                    blue_ghost.wanted_dir = right;
                    blue_ghost.curr_dir   = right;
                    blue_ghost.mode = spawn_wait;
                    blue_ghost.color = BLUE;
                    blue_ghost.spawn_wait_time = 15;
                    blue_ghost.spawn_wait_time -= wait_time_diff;
                    if (blue_ghost.spawn_wait_time < 0) blue_ghost.spawn_wait_time = 0;

                    pink_ghost.pos.x = 14 * cell_width;
                    pink_ghost.pos.y = 17 * cell_height;
                    pink_ghost.cell.x = pink_ghost.pos.x/cell_width;
                    pink_ghost.cell.y = pink_ghost.pos.y/cell_height;
                    pink_ghost.wanted_dir = left;
                    pink_ghost.curr_dir   = left;
                    pink_ghost.mode = spawn_wait;
                    pink_ghost.color = PINK;
                    pink_ghost.spawn_wait_time = 30;
                    pink_ghost.spawn_wait_time -= wait_time_diff;
                    if (pink_ghost.spawn_wait_time < 0) pink_ghost.spawn_wait_time = 0;

                    orange_ghost.pos.x = (16 * cell_width) - 3 ;
                    orange_ghost.pos.y = 17 * cell_height;
                    orange_ghost.cell.x = orange_ghost.pos.x/cell_width;
                    orange_ghost.cell.y = orange_ghost.pos.y/cell_height;
                    orange_ghost.wanted_dir = left;
                    orange_ghost.curr_dir   = left;
                    orange_ghost.mode = spawn_wait;
                    orange_ghost.color = ORANGE;
                    orange_ghost.spawn_wait_time = 45;
                    orange_ghost.spawn_wait_time -= wait_time_diff;
                    if (orange_ghost.spawn_wait_time < 0) orange_ghost.spawn_wait_time = 0;

                    spawn_start = current_time;
                    for (int row = 0; row < rows; ++row) {
                        for (int col = 0; col < cols; ++col) {
                            map[row][col] = map_backup[row][col];
                        }
                    }
                    wait_time_diff += 5;
                    chase_speed += chase_speed_diff;
                    if (chase_speed > max_chase_speed) chase_speed = max_chase_speed;

            ++level_count;
        }
        DrawText(TextFormat("Level %d", level_count), 0, 0,  30, BLUE);
        DrawText(TextFormat("score %d", score), 0, cell_height,  30, BLUE);
        if (pacman_lives_remain < 0) {
            running  = false;
            printf("you died on level %d with a score of %d\n", level_count, score);
        }
        EndDrawing();
    }

    CloseWindow();
}
