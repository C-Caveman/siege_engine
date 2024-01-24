#ifndef ENT
#define ENT

// entities

#include "../defs.h"
#include "../graphics/animations.h"
#include <SDL2/SDL.h>

#define RSIZE 80    // diameter of rectangular entities

/*
=================================================================== ENTITY STRUCTURE
    1: ENT_BASICS
    2: struct sprite sprites[NUM_SPRITES_THIS_ENT_HAS]
    3: <your vars here>
------------------------------------------------------------------------------------

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

//======================================================================// ENTITY_TYPES_LIST //
#define MAX_ENTITY_TYPE_NAME_LEN 32
// This is an X macro. Redefine expand() to change how the list expands.
#define ENTITY_TYPES_LIST \
    expand(player) \
    expand(scenery)
//===========================================================================================//
enum entity_types { // Using the X macro to get an enum:
    #define expand(name) name##_type, 
    ENTITY_TYPES_LIST
    #undef expand
    NUM_ENT_TYPES
};

//================================================= Required components of every entity.
typedef int16_t handle;
#define ENT_BASICS       \
    char header_byte;    \
    uint8_t type;        \
    uint8_t num_sprites; \
    uint16_t size;       \
    handle h;            \
    int flags;           \
    vec2f pos;           \
    vec2f vel;           \
    vec2f dir;           \
    vec2i chunk;         \
    int health;
struct ent_basics { ENT_BASICS };
enum ent_flags {DRAWABLE, ANIMATABLE, MOVABLE, COLLIDABLE, THINKABLE, };
//--------------------------------------------------------------------------------------
enum sprite_flags {LOOPING, PAUSED, };
#define HEADER_BYTE 'H'
struct sprite {
    uint16_t anim;       // Enum value of the animation. (animation data is stored elsewhere)
    uint8_t frame;       // current frame of animation.
    uint8_t anim_tick;   // Tick the previous frame was drawn on.
    uint8_t flags;       // Flags for sprite animation. Looping, stopped, ect.
    vec2f pos;           // Offset from the ent origin
    float rotation;
};


//==== HANDLE ========================================================= HANDLE
struct handle_info {
    struct ent_basics* ent; // Entity who owns this handle.
    int16_t copies;         // Num ents using this handle.
    bool claimed;           // Whether the entity is marked for deletion.
};
#define NUM_HANDLES 2048
extern struct handle_info handles[NUM_HANDLES];
handle claim_handle(struct ent_basics* e);
handle copy_handle(handle i);
struct ent_basics* get_ent(handle i);
//-----------------------------------------------------------------------------


//------------------------------------------------------------------- Example entity.
enum example_sprites {FIRST_SPRITE, SECOND_SPRITE, NUM_EXAMPLE_SPRITES};
struct ent_example {
    ENT_BASICS
    struct sprite sprites[NUM_EXAMPLE_SPRITES];
    int your_vars_here;
};
//-----------------------------------------------------------------------------------

enum player_sprites {PLAYER_BODY, PLAYER_GUN, NUM_PLAYER_SPRITES}; //---------------- Player entity.
enum move_types {MOVE_SNEAK, MOVE_WALK, MOVE_SPRINT, MOVE_DASH, NUM_MOVE_TYPES};
struct ent_player {
    ENT_BASICS
    struct sprite sprites[NUM_PLAYER_SPRITES];
    uint8_t movetype;   // Selected speed.
    uint8_t movetypes[NUM_MOVE_TYPES];    // Available speeds.

    void init();
    void think();
};
enum scenery_sprites {SCENERY_SPRITE, NUM_SCENERY_SPRITES}; //----------------------- Scenery entity.
struct ent_scenery {
    ENT_BASICS
    struct sprite sprites[NUM_PLAYER_SPRITES];

    void init();
    void think();
};

//============================================================// Generic entity functions. //
void* spawn_ent(int type, char* array, int array_len);
void despawn_ent(struct ent_basics* ent);
void think_all_ents(char* array, int array_len);
char* get_type_name(int type);
int get_ent_size(int type);
int get_first_ent(char* array, int array_len);
int get_next_ent(int i, char* array, int array_len);
void move_ent(struct ent_basics* ent);
//=========================================================================================//




#endif
