// Entities.
#ifndef ENT
#define ENT

#include "../defs.h"
#include "../graphics/animations.h"
#include <SDL2/SDL.h>
/*
//================================================================// ENTITY STRUCTURE //
    1: ENT_BASICS
    2: struct sprite sprites[NUM_SPRITES_THIS_ENT_HAS]
    3: <your vars here>

//====== ENTITY UPDATE CYCLE ====================================// ENTITY UPDATE CYCLE //
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
//==== HANDLES =========================================================================// HANDLES //
typedef int16_t handle;
struct handle_info {
    struct ent_basics* ent; // Entity who owns this handle.
    int16_t copies;         // Num ents using this handle.
    bool claimed;           // Whether the entity is marked for deletion.
};
#define NUM_HANDLES 2048
extern struct handle_info handles[NUM_HANDLES];
struct ent_basics*  get_ent(handle i); //----------------- Get an entity* from its handle.
handle              copy_handle(handle i); //------------- Copy a handle to another entity.
handle              uncopy_handle(handle i); //----------- Delete a handle to another entity.

//====== ENTITY_TYPES_LIST =============================================================// ENTITY_TYPES_LIST //
#define MAX_ENTITY_TYPE_NAME_LEN 32
#define ENTITY_TYPES_LIST \
    expand(player) \
    expand(scenery)
// ENTITY_TYPES_LIST is an X macro. Redefine expand() to change how the list expands.
#define expand(name) name##_type, 
enum entity_types { ENTITY_TYPES_LIST    NUM_ENT_TYPES };
#undef expand
//======= ENTITIES ====================================================================================// ENTITIES //
enum example_sprites {FIRST_SPRITE, SECOND_SPRITE, NUM_EXAMPLE_SPRITES}; //---------- EXAMPLE ENTITY.
struct ent_example {
    ENT_BASICS //---------------------------------------------- !!! REQUIRED !!!
    struct sprite sprites[NUM_EXAMPLE_SPRITES]; //------------- !!! REQUIRED !!!
    //--------------------------------------------------------- <your variables here>

    void init(); //-------------------------------------------- !!! REQUIRED !!!
    void think(); //------------------------------------------- !!! REQUIRED !!!
    //--------------------------------------------------------- <your functions here>
};
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
    handle fren;

    void init();
    void think();
};
//======================================================================// Generic entity functions. //
void* spawn_ent(int type, char* array, int array_len);
void despawn_ent(struct ent_basics* ent);
void think_all_ents(char* array, int array_len);
char* get_type_name(int type);
int get_ent_size(int type);
int get_first_ent(char* array, int array_len);
int get_next_ent(int i, char* array, int array_len);
void move_ent(struct ent_basics* ent);
char* get_type_name(int type);

#endif
