#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "raylib.h"
#include "math.h"

#define GHOST_COUNT 4

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


V2int jail_cell = {
    .x = 13,
    .y = 14,
};

V2int spawn_exit_cell = {
    .x = 13,
    .y = 16,
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
                DrawRectangle(x, y, cell_width, cell_height, DARKGREEN);
            }
            else if (map[row][col] == 1) {
                DrawRectangle(x, y, cell_width, cell_height, BEIGE );
            }
            else if (map[row][col] == 2) {
                DrawRectangle(x, y, cell_width, cell_height, GOLD);
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

    Ghost_Mode chase_scatter_toggle = scatter;

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
    blue_ghost.pos.x = 13 * cell_width;
    blue_ghost.pos.y = 16 * cell_height;
    blue_ghost.cell.x = blue_ghost.pos.x/cell_width;
    blue_ghost.cell.y = blue_ghost.pos.y/cell_height;
    blue_ghost.scatter_cell.x = 27;
    blue_ghost.scatter_cell.y = 35;
    blue_ghost.wanted_dir = right;
    blue_ghost.curr_dir   = right;
    blue_ghost.mode = spawn_wait;
    blue_ghost.color = BLUE;
    blue_ghost.spawn_wait_time = 1;

    Ghost pink_ghost = {0};
    pink_ghost.pos.x = 14 * cell_width;
    pink_ghost.pos.y = 16 * cell_height;
    pink_ghost.cell.x = pink_ghost.pos.x/cell_width;
    pink_ghost.cell.y = pink_ghost.pos.y/cell_height;
    pink_ghost.scatter_cell.x = 2;
    pink_ghost.scatter_cell.y = 0;
    pink_ghost.wanted_dir = left;
    pink_ghost.curr_dir   = left;
    pink_ghost.mode = spawn_wait;
    pink_ghost.color = PINK;
    pink_ghost.spawn_wait_time = 2;

    Ghost orange_ghost = {0};
    orange_ghost.pos.x = 15 * cell_width;
    orange_ghost.pos.y = 16 * cell_height;
    orange_ghost.cell.x = orange_ghost.pos.x/cell_width;
    orange_ghost.cell.y = orange_ghost.pos.y/cell_height;
    orange_ghost.scatter_cell.x = 0;
    orange_ghost.scatter_cell.y = 35;
    orange_ghost.wanted_dir = left;
    orange_ghost.curr_dir   = left;
    orange_ghost.mode = spawn_wait;
    orange_ghost.color = ORANGE;
    orange_ghost.spawn_wait_time = 3;


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
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        draw_map();
        //draw_grid();
        float current_time= GetTime();
        float frame_time= GetFrameTime();

        if ((eat_timer.start_time + eat_timer.len) < current_time) {
            for (int i = 0; i <GHOST_COUNT; ++i) {
                if (ghosts[i]->mode == scared)  {
                    flip_dir(&ghosts[i]->curr_dir, &ghosts[i]->wanted_dir);
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
            else if (ghosts[i]->mode == eaten || ghosts[i]->mode == jail_down || ghosts[i]->mode == jail_up) {
                ghosts[i]->dp = eaten_speed * frame_time;
            }
        }


        if (map[pacman_cell.y][pacman_cell.x] == 1 ) {
            map[pacman_cell.y][pacman_cell.x] = 0;
        }
        else if ( map[pacman_cell.y][pacman_cell.x] == 2 ) {
            for (int i = 0; i<GHOST_COUNT; ++i) {
                flip_dir(&ghosts[i]->curr_dir, &ghosts[i]->wanted_dir);
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
                }            
                else {
                    //RESET and  lose life
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

                bool collision = ( ghosts[i]->cell.x == jail_cell.x && jail_cell.y == ghosts[i]->cell.y);
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
                    if (ghosts[i]->pos.x < spawn_exit_cell.x * cell_width) {
                        ghosts[i]->cell.x = spawn_exit_cell.x;
                        ghosts[i]->pos.x = spawn_exit_cell.x * cell_width;
                        ghosts[i]->mode = jail_up;
                    }
                }
                else if (i == pink) {
                    if (ghosts[i]->pos.x < spawn_exit_cell.x * cell_width) {
                        ghosts[i]->cell.x = spawn_exit_cell.x;
                        ghosts[i]->pos.x = spawn_exit_cell.x * cell_width;
                        ghosts[i]->mode = jail_up;
                    }
                }
                else if (i == blue) {
                    if (ghosts[i]->pos.x > spawn_exit_cell.x * cell_width) {
                        ghosts[i]->cell.x = spawn_exit_cell.x;
                        ghosts[i]->pos.x = spawn_exit_cell.x * cell_width;
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
        move_player(&pacman_curr_dir, &pacman_wanted_dir, &pacman_pos, &pacman_cell, pacman_dp);

        DrawRectangle(pacman_pos.x, pacman_pos.y, cell_width, cell_height, YELLOW);
        for (int i = 0; i <GHOST_COUNT; ++i) {
            DrawRectangle(ghosts[i]->pos.x, ghosts[i]->pos.y, cell_width, cell_height, ghosts[i]->color);
        }

        EndDrawing();
    }

    CloseWindow();
}
