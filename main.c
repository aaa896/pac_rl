#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "raylib.h"

#define GAME_SPEED 8
#define FPS 60
const double update_dir_time = (1.0/FPS) * GAME_SPEED;


typedef struct {
    int window_scale ;
    int window_width ;
    int window_height ;
    int cols ;
    int rows ;
    int cell_width ;
    int cell_height ;
}Window_Details;


typedef enum {
    left,
    right,
    up,
    down,
    stop
}Dir;


typedef struct {
    Dir active_dir;
    Dir past_dir;
    Dir loaded_dir;
    int x;
    int y;
    Color color;
    int id;
}Player;

typedef struct {
    double start_time;
    double len;
}Timer;


//3 = wall
//0 = empty space
//8 teleportys to 9
//9 telepots to 8
//7 path that ghost
//1 pellet
//2 eat ghost big pellet
int map[36][28] =  {
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
    {3,3,3,3,3,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,3,3,3,3,3},
    {8,7,7,7,7,7,1,0,0,0,3,3,3,3,3,3,3,3,0,0,0,1,7,7,7,7,7,9},
    {3,3,3,3,3,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,3,3,3,3,3},
    {0,0,0,0,0,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,0,0,0,0,0},
    {0,0,0,0,0,3,1,3,3,0,0,0,0,0,0,0,0,0,0,3,3,1,3,0,0,0,0,0},
    {0,0,0,0,0,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,0,0,0,0,0},
    {3,3,3,3,3,3,1,3,3,0,3,3,3,3,3,3,3,3,0,3,3,1,3,3,3,3,3,3},
    {3,1,1,1,1,1,1,1,1,1,1,1,1,3,3,1,1,1,1,1,1,1,1,1,1,1,1,3},
    {3,1,3,3,3,3,1,3,3,3,3,3,1,3,3,1,3,3,3,3,3,1,3,3,3,3,1,3},
    {3,1,3,3,3,3,1,3,3,3,3,3,1,3,3,1,3,3,3,3,3,1,3,3,3,3,1,3},
    {3,2,1,1,3,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,3,1,1,2,3},
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


void draw_map(Window_Details *window) {
    for (int row = 0; row < window->rows; ++row) {
        for (int col = 0; col < window->cols; ++col) {
            if (map[row][col] == 3) {
                int start_pos_x =  col * window->cell_width;
                int start_pos_y =  row * window->cell_height;
                DrawRectangle( start_pos_x, start_pos_y, window->cell_width, window->cell_height, BLUE); 
            }
            if (map[row][col] == 1) {
                int start_pos_x =  col * window->cell_width;
                int start_pos_y =  row * window->cell_height;
                DrawRectangle( start_pos_x + window->cell_width/4, start_pos_y + window->cell_height/4, window->cell_width - window->cell_width/2, window->cell_height -window->cell_width/2, WHITE); 
            }
            if (map[row][col] == 2) {
                int start_pos_x =  col * window->cell_width;
                int start_pos_y =  row * window->cell_height;
                DrawRectangle( start_pos_x + window->cell_width/4, start_pos_y + window->cell_height/4, window->cell_width - window->cell_width/2, window->cell_height -window->cell_width/2, ORANGE); 
            }
        }
    }
}


void draw_grid(Window_Details *window) {
    for (int row = 0; row < window->rows; ++row) {
        for (int col = 0; col < window->cols; ++col) {
            int start_pos_x =  col * window->cell_width;
            int start_pos_y =  row * window->cell_height;
            DrawRectangleLines( start_pos_x, start_pos_y, window->cell_width, window->cell_height, RED); 
        }
    }
}


void load_loaded_pacman_dir(Dir *dir) {
    if (IsKeyDown(KEY_LEFT))   *dir = left;
    if (IsKeyDown(KEY_RIGHT))  *dir = right;
    if (IsKeyDown(KEY_UP))     *dir = up;
    if (IsKeyDown(KEY_DOWN))   *dir = down;
}


void draw_player(Player *player, double current_time, double last_time, Window_Details *window) {
    double delta_pos_x = (current_time - last_time)/update_dir_time * window->cell_width;
    double delta_pos_y = (current_time - last_time)/update_dir_time * window->cell_height;

    if (player->active_dir == right)  DrawRectangle(player->x + delta_pos_x - window->cell_width, player->y, window->cell_width, window->cell_height, player->color);
    if (player->active_dir == left)   DrawRectangle(player->x - delta_pos_x + window->cell_width, player->y, window->cell_width, window->cell_height, player->color);
    if (player->active_dir == up)     DrawRectangle(player->x ,  player->y - delta_pos_y + window->cell_height, window->cell_width, window->cell_height, player->color);
    if (player->active_dir == down)   DrawRectangle(player->x , player->y + delta_pos_y - window->cell_height, window->cell_width, window->cell_height, player->color);
    if (player->active_dir == stop)   DrawRectangle(player->x,  player->y, window->cell_width, window->cell_height, player->color);
}


void load_loaded_ghost_dir(Player *ghost, Player *pacman, Window_Details *window) {
    int ghost_x_cell = ghost->x/window->cell_width;
    int ghost_y_cell = ghost->y/window->cell_height;
    int path_count = 0;
    if (map[ghost_y_cell][ ghost_x_cell +1] == 0 || map [ ghost_y_cell][ghost_x_cell +1] == 1) ++path_count;
    if (map[ghost_y_cell][ ghost_x_cell - 1] == 0 || map [ ghost_y_cell][ghost_x_cell -1] == 1) ++path_count;
    if (map[ghost_y_cell +1][ ghost_x_cell ] == 0 || map [ ghost_y_cell +1][ghost_x_cell] == 1) ++path_count;
    if (map[ghost_y_cell -1][ ghost_x_cell ] == 0 || map [ ghost_y_cell - 1][ghost_x_cell] == 1) ++path_count;

    if (path_count >2) {
        int target_x_cell;
        int target_y_cell ;
        if (ghost->id == 1) {
            target_x_cell = pacman->x /window->cell_width;
            target_y_cell = pacman->y / window->cell_height;
        }

        int x_dist = ghost_x_cell - target_x_cell;
        Dir x_dir = (x_dist>0) ? left:right;
        int y_dist = ghost_y_cell - target_y_cell;
        Dir y_dir = (y_dist > 0)? up:down;
        if (x_dist<0) x_dist *=-1;
        if (y_dist<0) y_dist *=-1;

        if (ghost_x_cell == target_x_cell) {
            ghost->loaded_dir = y_dir;
        }
        else if (ghost_y_cell == target_y_cell) {
            ghost->loaded_dir = x_dir;
        }
        else if (x_dist < y_dist) {
            ghost->loaded_dir = x_dir;
        }
        else {
            ghost->loaded_dir = y_dir;
        }


    }



}



void move_player(Player *player, Window_Details *window) {

    player->active_dir = player->loaded_dir;
    int map_x = player->x/window->cell_width;
    int map_y = player->y/window->cell_height;
    bool dir_found = false;

    while (!dir_found) {
        dir_found = true;

        if (player->active_dir == right) {
            bool should_move =  player->x + window->cell_width < window->window_width 
                && map[map_y][map_x+1] != 3;

            if (should_move) {
                if (player->id != 0) {
                    if (player->past_dir == left) {
                        player->active_dir = left;
                        dir_found = false;
                    }
                    else {
                        player->x += window->cell_width;
                    }

                }
                else {
                    player->x += window->cell_width;
                }
            }
            else if (map[map_y][map_x] == 9) {
                player->x = 0;
            }
            else if (player->past_dir == player->active_dir) {
                if (player->id == 0) {
                    player->active_dir = stop;
                }
                else {
                    player->active_dir = rand() %4;
                    if (player->past_dir == right) {
                        while (player->active_dir == left || player->active_dir == right  ) {
                            player->active_dir = rand() %4;
                        }
                    }
                    player->loaded_dir = player->active_dir;
                    dir_found = false;
                }
            }
            else {
                player->active_dir = player->past_dir;
                dir_found = false;
            }

        }
        else if (player->active_dir == left) {

            bool should_move =  player->x>0
                && map[map_y][map_x-1] != 3;

            if (should_move)  {
                if (player->id != 0) {
                    if (player->past_dir == right) {
                        player->active_dir = right;
                        dir_found = false;
                    }
                    else {
                        player->x -= window->cell_width;
                    }
                }
                else {
                        player->x -= window->cell_width;
                }
            }
            else  if (map[map_y][map_x] == 8) {
                player->x = window->window_width - window->cell_width;
            }
            else if (player->past_dir == player->active_dir) {
                if (player->id == 0) {
                    player->active_dir = stop;
                }
                else {
                    player->active_dir = rand() %4;
                    if (player->past_dir == left) {
                        while (player->active_dir == left || player->active_dir == right  ) {
                            player->active_dir = rand() %4;
                        }
                    }
                    player->loaded_dir = player->active_dir;
                    dir_found = false;
                }
            }
            else {
                player->active_dir = player->past_dir;
                dir_found = false;
            }
        }
        else if (player->active_dir == up)    {

            bool should_move =  player->y - window->cell_height >=0
                && map[map_y-1][map_x] !=3;

            if (should_move) {
                if (player->id != 0) {
                    if (player->past_dir == down) {
                        player->active_dir = down;
                        dir_found = false;
                    }
                    else {
                        player->y -= window->cell_height;
                    }
                }
                else {
                    player->y -= window->cell_height;
                }
            }
            else if (player->past_dir == player->active_dir)  {
                if (player->id == 0) {
                    player->active_dir = stop;
                }
                else {
                    player->active_dir = rand() %4;
                    if (player->past_dir == up) {
                        while (player->active_dir == down || player->active_dir == up  ) {
                            player->active_dir = rand() %4;
                        }
                    }
                    player->loaded_dir = player->active_dir;
                    dir_found = false;
                }
            }
            else  {
                player->active_dir = player->past_dir;
                dir_found  = false;
            }
        }
        else if (player->active_dir == down)  {
            bool should_move =  player->y + window->cell_height < window->window_height
                && map[map_y +1][map_x] !=3;

            if (should_move)  {
                if (player->id != 0) {
                    if (player->past_dir == up) {
                        player->active_dir = up;
                        dir_found = false;
                    }
                    else {
                        player->y += window->cell_width;
                    }
                }
                else {
                        player->y += window->cell_width;
                }
            }
            else if (player->past_dir == player->active_dir)  {
                if (player->id == 0) {
                    player->active_dir = stop;
                }
                else {
                    player->active_dir = rand() %4;
                    if (player->past_dir == down) {
                        while (player->active_dir == down || player->active_dir == up  ) {
                            player->active_dir = rand() %4;
                        }
                    }
                    player->loaded_dir = player->active_dir;
                    dir_found = false;
                }
            }
            else {
                player->active_dir = player->past_dir;
                dir_found = false;
            }
        }

    }

    player->past_dir = player->active_dir;
}


int main() {
    Window_Details window = {0};
    window.window_scale = 3;
    window.window_width = 224 * window.window_scale;
    window.window_height = 288 * window.window_scale;
    window.cols = window.window_width/(8 * window.window_scale);
    window.rows = window.window_height/( 8 * window.window_scale);
    window.cell_width = window.window_width / window.cols;
    window.cell_height = window.window_height /window.rows;

    InitWindow(window.window_width, window.window_height, "Pacman");
    SetTargetFPS(FPS);

    Player pacman = {0};
    pacman.x = window.cell_width  * 13;
    pacman.y = window.cell_height * 26;
    pacman.active_dir = right;
    pacman.past_dir   = right;
    pacman.loaded_dir = right;
    pacman.color = YELLOW;
    pacman.id = 0;

    Player red_ghost = {0};
    red_ghost.x = window.cell_width  * 16;
    red_ghost.y = window.cell_height * 8;
    red_ghost.active_dir = right;
    red_ghost.past_dir   = right;
    red_ghost.loaded_dir = right;
    red_ghost.color = RED;
    red_ghost.id = 1;

    Timer eat_ghost_timer = {};


    HideCursor();
    double last_time = GetTime();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        //draw_grid(&window);
        draw_map(&window);
        load_loaded_pacman_dir(&pacman.loaded_dir);
        load_loaded_ghost_dir(&red_ghost, &pacman, &window);

        double current_time = GetTime();
        draw_player(&pacman, current_time, last_time, &window);
        draw_player(&red_ghost, current_time, last_time, &window);
        

        if (current_time >= (last_time + update_dir_time) ) {
            int map_x = pacman.x/window.cell_width;
            int map_y = pacman.y/window.cell_height;
            if (map[map_y][map_x]==1 || map[map_y][map_x] == 2)  map[map_y][map_x] = 0;

            move_player(&pacman, &window);
            move_player(&red_ghost, &window);
            
            last_time = current_time;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
