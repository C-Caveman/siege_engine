#ifndef ENT
#define ENT

// entities

#include "../defs.h"
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
    vec2f pos; // position
    vec2f dir; // direction
    vec2f vel; // velocity
    ent* next;
    ent* prev;
    uint8_t state_info[STATE_INFO_SIZE]; // general purpose vars
public:    
    ent();
    ent(int id, int type, int anim, int fr_st, int thk_fr, int an_len, vec2f pos, vec2f dir, vec2f vel);
    ~ent();
    void move_ent(vec2f pos);
    void slide_ent(vec2f distance);
    void collide_ent(ent* ent_b); // square bumping into a square
    void collide_ent_cs(ent* ent_b); // circle bumping into a square
    int get_typ();
    vec2f get_pos();
    void set_pos(vec2f);
    vec2f get_dir();
    void set_dir(vec2f);
    vec2f get_vel();
    void set_vel(vec2f);
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







/////////////////////////////////////////////////////////////////////////////////////////
// Segments are the chunks of memory making up an entity. (each one is 64 bits long)
//

enum segment_types {
    SEG_HEAD, SEG_POS, SEG_CHUNK, SEG_VEL, SEG_DIR, SEG_ANIM, SEG_HEALTH, SEG_MOVETYPE,
};

enum ent_flags {DRAWABLE, MOVABLE, ANIMATABLE, DAMAGABLE, THINKABLE};

struct seg_head {        // Header segment. Used by all entities.
    uint32_t flags;      // Basic qualities of this ent: drawable, movable, collideable, ect.
    uint8_t type;        // Type of entity.
    uint8_t size;        // Width of this entity (in segments).
    uint8_t num_sprites; // Number of sprites to draw for this entity.
};
struct seg_pos {         // Entity position.
    vec2f pos;
};
struct seg_chunkpos {    // Entity chunk position. (index of current chunk)
    vec2i chunkpos;
};
struct seg_vel {         // Entity velocity.
    vec2f vel;
};
struct seg_dir {         // Entity direction.
    vec2f dir;
};
struct seg_anim {        // Data for an entity's animation.
    int anim;            // Index of the animation.
    int anim_flags;      // Qualities of the animation. (???looping???)
};
struct seg_health {      // Entity health.
    int health;          // Health remaining.
    int status_flags;    // Status effects. (???on_fire,poisoned,ect.???)
};
enum MOVE_TYPES {MOVE_SNEAK, MOVE_WALK, MOVE_SPRINT, MOVE_DASH, NUM_MOVE_TYPES};
struct seg_movetype {   // Entity movement speeds. (sneak/walk/sprint/dash)
    uint8_t cur_movetype;   // Selected speed.
    uint8_t movetypes[NUM_MOVE_TYPES];    // Available speeds.
};

/////////////////////////////////////////////////////////////////////////////////////////
// Allow arrays of different segments.
union segment {
    struct seg_head     head;
    struct seg_pos      pos;
    struct seg_chunkpos chunkpos;
    struct seg_vel      vel;
    struct seg_dir      dir;
    struct seg_anim     anim;
    struct seg_health   health;
    struct seg_movetype movetype;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Structure of each entity. >>>Implementations in actions.cpp<<<
/*
ENTITY STRUCTURE:
    > header (general info about the entity)

    > velocity
    > position

    > sprite 0 pos
    > sprite 0 anim
    > sprite 0 rotation (only present when anim.flags &= ROTATABLE)
    > ...
    > sprite n pos
    > sprite n anim
    > sprite n rotation (only present when anim.flags &= ROTATABLE)

    > ...

ENTITY UPDATE CYCLE:
    > Animate
         Update the frame number of each sprite. Trigger think if on a special frame.
    > Think
        Entity-specific functions like pathfinding and shooting.
    > Move
        Apply velocity.
    > Collide
        Tweak position/velocity based on overlap with walls/entities.
    > Draw
        All of the entity's sprites.
*/
enum entity_types {
    EMPTY, PLAYER, SCENERY,
    NUM_ENT_TYPES
};

// Segments common to most entities.
enum ent_basics_segments {                  // basic entity (component of most entitites)
    head, chunkpos, pos, vel, dir, anim, health,
    basic_ent_size
};

enum player_segments {                      // player
    player_end_of_basics=basic_ent_size-1,
    weapon, name, whatever, ect,
    player_size
};
struct ent_player {
    segment data[player_size];
    void init();
};

enum scenery_segments {                     // scenery
    scenery_end_of_basics=basic_ent_size-1,
    scenery_size
};
struct ent_scenery {
    segment data[scenery_size];
    void init();
};

/////////////////////////////////////////////////////////////////////////////////////////
// Spawn an entity (insert a given type of ent to an array of entitity segments).
//
int get_ent_size(int type);
segment* spawn_ent(int type, segment* array, int array_len);
void   despawn_ent(segment* ent);





#endif
