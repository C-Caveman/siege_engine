#ifndef ENT
#define ENT

// entities

#include "defs.h"
#include <SDL2/SDL.h>

#define RSIZE 128    // diameter of rectangular entities
#define CIRCLE_R 64  // radius of circular entities

enum ent_types {
    ENT_DEFAULT=0,
    ENT_PLAYER,
    ENT_SCENERY,
    ENT_GUN
};

enum state_info_player { // 8 uints
    state_player_something=0,
    state_player_speed,
    state_player_rotation
};

#define STATE_INFO_SIZE 8
class ent {
private:
    int id;   // unique identifier
    int type; // what kind of entity is it?
    int anim;  // which animation does it use?
    int frame_start; // what sdl_tick did the animation start on?
    int think_frame; // which frame of the animation to think on?
    int anim_len; // how many frames long is the animation?
    vec2 pos; // position
    vec2 dir; // direction
    vec2 vel; // velocity
    ent* next;
    ent* prev;
    uint8_t state_info[STATE_INFO_SIZE]; // general purpose vars
public:    
    ent();
    ent(int id, int type, int anim, int fr_st, int thk_fr, int an_len, vec2 pos, vec2 dir, vec2 vel);
    ~ent();
    void move_ent(vec2 pos);
    void slide_ent(vec2 distance);
    void collide_ent(ent* ent_b); // square bumping into a square
    void collide_ent_cs(ent* ent_b); // circle bumping into a square
    int get_typ();
    vec2 get_pos();
    void set_pos(vec2);
    vec2 get_dir();
    void set_dir(vec2);
    vec2 get_vel();
    void set_vel(vec2);
    int get_id();
    int get_anim();
    void set_anim(int animation);
    friend ostream& operator << (ostream& os, const ent& e);
    ent* get_next();
    ent* get_prev();
    void set_next(ent* n);
    void set_prev(ent* p);
    void set_state(int state_index, int new_state);
    int get_state(int state_index);
};

#endif
