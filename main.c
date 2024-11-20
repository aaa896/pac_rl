#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "math.h"
#include <stdbool.h>


const float window_scale = 3;
const int screen_width = 224 * window_scale;
const int screen_height = 288 * window_scale;
const int rows = 36;
const int cols = 28;
const int cell_width = screen_width /cols;
const int cell_height = screen_height /rows;
float fps = 60;

bool draw_next_cell = true;

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
}Ghost_Mode;

V2int rg_scatter_cell = {
    .x = 25,
    .y = 0,
};

V2int jail_cell = {
    .x = 15,
    .y = 14,
};

typedef struct {
    float start_time;
    float len;
}Timer;

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


void flip_dir(Dir *curr_dir, Dir *wanted_dir) {
    if (*curr_dir == right)      *wanted_dir = left;
    else if (*curr_dir == left)  *wanted_dir = right;
    else if (*curr_dir == up)    *wanted_dir = down;
    else if (*curr_dir == down)  *wanted_dir = up;
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


float get_target_cell_dist(V2int cell, V2int target_cell) {
    float x = target_cell.x - cell.x;
    float y = target_cell.y - cell.y;
    float dist = sqrt((x * x) + (y * y));
    return dist;
}


void update_rg_wanted_dir_chase(Dir curr_dir, Dir *wanted_dir, V2int cell, V2int target_cell) {
    
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

        if (!left_available)   left_cell_dist  = unavailable;
        if (!right_available)  right_cell_dist = unavailable;
        if (!up_available)     up_cell_dist    = unavailable;
        if (!down_available)   down_cell_dist  = unavailable;

        float smallest_dist = left_cell_dist;
        if (right_cell_dist < smallest_dist) smallest_dist = right_cell_dist;
        if (down_cell_dist < smallest_dist)  smallest_dist = down_cell_dist;
        if (up_cell_dist < smallest_dist)    smallest_dist = up_cell_dist;

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

    Vector2 pacman_pos = {0};
    pacman_pos.x = 14 * cell_width;
    pacman_pos.y = 26 * cell_height;
    V2int  pacman_cell = {0};
    pacman_cell.x = pacman_pos.x/cell_width;
    pacman_cell.y = pacman_pos.y/cell_height;
    Dir pacman_wanted_dir = right;
    Dir pacman_curr_dir   = right;
    float pacman_speed = 100;


    float chase_speed = 80;
    float scared_speed = 50;
    float eaten_speed = 100;

    Vector2 rg_pos = {0};
    rg_pos.x = 14 * cell_width;
    rg_pos.y = 14 * cell_height;
    V2int  rg_cell = {0};
    rg_cell.x = rg_pos.x/cell_width;
    rg_cell.y = rg_pos.y/cell_height;
    Dir rg_wanted_dir = right;
    Dir rg_curr_dir   = right;

    Ghost_Mode rg_mode = scatter;
    Ghost_Mode all_ghost_mode = scatter;
    Color rg_color = RED;

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

    int eat_timer_len = 8;
    Timer eat_timer = {0};
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        draw_map();
        draw_grid();
        float current_time= GetTime();
        float frame_time= GetFrameTime();

        if ((eat_timer.start_time + eat_timer.len) > current_time) {
            all_ghost_mode = scared;
        }
        else {
            if (all_ghost_mode == scared)  {
                flip_dir(&rg_curr_dir, &rg_wanted_dir);
                all_ghost_mode = chase;
                chase_timer.start_time = current_time;
            }
            else if (all_ghost_mode == scatter) {
                if ((scatter_timer.start_time + scatter_timer.len) < current_time) {
                    all_ghost_mode = chase;
                    chase_timer.start_time = current_time;
                }
            }
            else if(all_ghost_mode == chase) {
                if ((chase_timer.start_time + chase_timer.len) < current_time) {
                    all_ghost_mode = scatter;
                    scatter_timer.start_time = current_time;
                }
            }
        }
        if (rg_mode == eaten) {
        }
        else {
            rg_mode = all_ghost_mode;
        }

        float pacman_dp = pacman_speed * frame_time;
        float rg_dp = 0;
        if (rg_mode == scared) {
            rg_dp  = scared_speed * frame_time ;
        }
        else if (rg_mode == chase || rg_mode == scatter) {
            rg_dp  = chase_speed * frame_time ; 
        }
        else if (rg_mode == eaten) {
            rg_dp = eaten_speed * frame_time;
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

        if (map[pacman_cell.y][pacman_cell.x] == 1 ) {
            map[pacman_cell.y][pacman_cell.x] = 0;
        }
        else if ( map[pacman_cell.y][pacman_cell.x] == 2 ) {
            flip_dir(&rg_curr_dir, &rg_wanted_dir);
            map[pacman_cell.y][pacman_cell.x] = 0;
            eat_timer.len = eat_timer_len;
            eat_timer.start_time = current_time;
            rg_mode = scared;
        }

        if (rg_mode == chase) {
            update_rg_wanted_dir_chase(rg_curr_dir, &rg_wanted_dir, rg_cell, pacman_cell);
            rg_color = RED;
        }
        else if (rg_mode == scatter) {
            update_rg_wanted_dir_chase(rg_curr_dir, &rg_wanted_dir, rg_cell, rg_scatter_cell);
            rg_color = RED;
        }
        else if (rg_mode == scared) {
            V2int rand_target_cell = {
                .x = rand()%cols,
                .y = rand()%rows,
            };
            update_rg_wanted_dir_chase(rg_curr_dir, &rg_wanted_dir, rg_cell, rand_target_cell);
            rg_color = BLUE;
        }
        
        bool collision = ( pacman_cell.x == rg_cell.x && pacman_cell.y == rg_cell.y);
        if (collision) {
            if (all_ghost_mode == scared) {
                rg_mode = eaten;
            }            
        }

        move_player(&pacman_curr_dir, &pacman_wanted_dir, &pacman_pos, &pacman_cell, pacman_dp);
        move_player(&rg_curr_dir, &rg_wanted_dir, &rg_pos, &rg_cell, rg_dp);

        DrawRectangle(pacman_pos.x, pacman_pos.y, cell_width, cell_height, YELLOW);
        DrawRectangle(rg_pos.x, rg_pos.y, cell_width, cell_height, rg_color);

        EndDrawing();
    }

    CloseWindow();
}
