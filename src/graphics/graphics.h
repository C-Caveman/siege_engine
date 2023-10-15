#ifndef GRAPHICS
#define GRAPHICS

#include "../world/world.h"
#include "animations.h"
#include <SDL2/SDL.h>


// Data stored for a particular animation.
enum anim_flags {ANIM_LOOPING, ANIM_PAUSED,}; // TODO use these flags.
struct anim_info {
    uint32_t texture_index;  // First frame in texture array.
    uint8_t  len;            // Number of frames.
    uint8_t  keyframe_0;     // Frame where an event occurs.
    uint8_t  keyframe_1;     // Frame where an event occurs.
    uint8_t  keyframe_2;     // Frame where an event occurs.
};

// variables used by graphics.cpp and server.cpp
extern SDL_Texture* textures[];
extern float min_frame_time;
extern SDL_Renderer* renderer;
extern SDL_Window* window;
extern SDL_Rect window_size;
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
void draw_ent_sprites(vec2f camera_pos, segment* ent_head);
void draw_all_ents(vec2f camera_pos, segment* array, int array_len);
void draw_chunk(vec2f camera_pos, vec2f camera_center, chunk* chunk);
void present_frame();
void cleanup_graphics();

#endif
