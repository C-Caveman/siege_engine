#ifndef GRAPHICS
#define GRAPHICS

#include "../defs.h"
#include "../config/vars.h"
#include "../ent/ent.h"
#include "animations.h"
#include <SDL2/SDL.h>

#define TILE_PIXEL_DIAMETER 32

#define MAX_TEXTURES 2048 // space allocated for textures
#define BKGRND_TEX 8
#define ANIM_NAME_LEN 16 // max length for an animation filename

extern struct anim_info anim_data[];

// variables used by graphics.cpp and server.cpp
extern SDL_Texture* textures[];
extern float min_frame_time;
extern SDL_Renderer* renderer;
extern SDL_Window* window;
extern SDL_Rect window_size;
extern SDL_Rect background;
//extern int fullscreen, window_x, window_y, fps_cap;
extern float tileWidth, VERTICAL_TILES_VISIBLE;
extern int running;
extern uint32_t lastFrameEnd, curFrameStart, frame_time, frame_count, last_sec, fps;
extern float dt;
extern uint8_t anim_tick;
extern float view_x; // camera position
extern float view_y;
extern int mouse_x;
extern int mouse_y;

void init_graphics();
void goFullscreen();
void goWindowed();
void track_fps();
void draw_ent_sprites(vec2f camera_pos, entBasics* ent);
void draw_all_ents(vec2f camera_pos, char* array, int array_len);
void draw_chunk_floor(vec2f camera_pos, vec2f camera_center, struct chunk* chunk, vec2i chunk_index);
void chunkDrawTallEnts(vec2f camera_pos, vec2f camera_center, struct chunk* chunk, vec2i chunk_index);
void chunkDrawShortEnts(vec2f camera_pos, vec2f camera_center, struct chunk* chunk, vec2i chunk_index);
void draw_chunk_walls(vec2f camera_pos, vec2f camera_center, struct chunk* chunk, vec2i chunk_index);
void renderText(char* text);
void drawInfo(char* label, float value, int index);
void drawDebugRectangle(int x, int y, int w, int h);
void drawTextBox(char* text, int numCharsToPrint);
void drawDialogBox(struct client* cl);
void renderMenu(struct client* cl);
void present_frame();
void cleanup_graphics();

#endif
