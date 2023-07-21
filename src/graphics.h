#ifndef GRAPHICS
#define GRAPHICS

#include "world.h"
#include "animations.h"
#include <SDL2/SDL.h>

// variables used by graphics.cpp and server.cpp
extern SDL_Texture* textures[];
extern char animation_names[NUM_ANIM*ANIM_NAME_LEN];
extern int animation_lengths[NUM_ANIM];
extern int animation_index[NUM_ANIM];
extern float min_frame_time;
extern SDL_Renderer* renderer;
extern SDL_Window* window;
extern SDL_Rect background;
extern int fullscreen, window_x, window_y, fps_cap;
extern int running;
extern float last_frame_end, frame_time, frame_count, last_sec, fps;
extern float dt;
extern float view_x; // camera position
extern float view_y;
extern int mouse_x;
extern int mouse_y;

void init_graphics();
void track_fps();
void draw_background();
void draw_ent(ent* e);
void draw_ents(ent* ent_array, float num_ents);
void draw_chunk(chunk* chunk);
void draw_text(ent* e);
void cleanup_graphics();

#endif
