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
//typedef int16_t handle; // Defined in defs.h already.
struct handle_info {
    ent_basics* ent; // Entity who owns this handle.
    int16_t copies;         // Num ents using this handle.
    bool claimed;           // Whether the entity is marked for deletion.
};
#define NUM_HANDLES 2048
extern struct handle_info handles[NUM_HANDLES];
ent_basics*  get_ent(handle i); //----------------- Get an entity* from its handle.
handle              copy_handle(handle i); //------------- Copy a handle to another entity.
handle              uncopy_handle(handle i); //----------- Delete a handle to another entity.

//====== ENTITY_TYPES_LIST =============================================================// ENTITY_TYPES_LIST //
#define MAX_ENTITY_TYPE_NAME_LEN 32
#define ENTITY_TYPES_LIST(f) \
    f(player) \
    f(scenery) \
    f(projectile) \
    f(rabbit) \
    f(zombie) \
    f(gib)
#define TO_TYPE_ENUM(name) name##_type, 
enum entity_types {
    ENTITY_TYPES_LIST(TO_TYPE_ENUM)
    NUM_ENT_TYPES
};
enum example_sprites {FIRST_SPRITE, SECOND_SPRITE, NUM_EXAMPLE_SPRITES}; //---------- EXAMPLE ENTITY
struct ent_example {
    ENT_BASICS //---------------------------------------------- !!! REQUIRED !!!
    struct sprite sprites[NUM_EXAMPLE_SPRITES]; //------------- !!! REQUIRED !!!
    //--------------------------------------------------------- <your variables here>
};
enum player_sprites {PLAYER_BODY, PLAYER_FLAMES_EXTRA, PLAYER_FLAMES, PLAYER_GUN, PLAYER_CROSSHAIR, NUM_PLAYER_SPRITES};              // PLAYER
enum move_types {MOVE_SNEAK, MOVE_WALK, MOVE_SPRINT, MOVE_DASH, NUM_MOVE_TYPES};
#define HEAT_MAX 200
#define OVERHEAT_TEMP 50
struct ent_player {
    ENT_BASICS
    struct sprite sprites[NUM_PLAYER_SPRITES];
    uint8_t movetype;   // Selected speed.
    uint8_t movetypes[NUM_MOVE_TYPES];    // Available speeds.
    struct client* cl;
    struct counter heat;
};
enum scenery_sprites {SCENERY_SPRITE_1, NUM_SCENERY_SPRITES};                     // SCENERY
struct ent_scenery {
    ENT_BASICS
    struct sprite sprites[NUM_PLAYER_SPRITES];
    handle fren;
};
enum projectile_sprites {PROJECTILE_SPRITE_1, PROJECTILE_SPRITE_2};             // PROJECTILE
struct ent_projectile {
    ENT_BASICS
    struct sprite sprites[NUM_PLAYER_SPRITES];
    handle parent;
    int lifetime;
    char isExploding;
};
enum rabbit_sprites {RABBIT_SPRITE_1, NUM_RABBIT_SPRITES};                      // RABBIT
struct ent_rabbit {
    ENT_BASICS
    struct sprite sprites[NUM_RABBIT_SPRITES];
    vec2f wanderDir;
    int wanderWait;
};
enum zombie_sprites {ZOMBIE_SPRITE_1, NUM_ZOMBIE_SPRITES};                      // ZOMBIE
struct ent_zombie {
    ENT_BASICS
    struct sprite sprites[NUM_RABBIT_SPRITES];
    handle target;
    vec2f targetPos;
    vec2f wanderDir;
    int wanderWait;
    struct counter walkDelay;
};
enum gib_sprites {GIB_SPRITE_1, NUM_GIB_SPRITES};                            // GIB
struct ent_gib {
    ENT_BASICS
    struct sprite sprites[NUM_GIB_SPRITES];
    int lifetime;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////
#define TO_INIT_PROTOTYPES(name) void name##Init(struct ent_##name* e); 
#define TO_THINK_PROTOTYPES(name) void name##Think(struct ent_##name* e); 
ENTITY_TYPES_LIST(TO_INIT_PROTOTYPES)
ENTITY_TYPES_LIST(TO_THINK_PROTOTYPES)
//======================================================================// Generic entity functions. //
void* spawn_ent(int type, char* array, int array_len);
void* spawn(int type, vec2f pos);
void despawn_ent(ent_basics* ent);
void think_all_ents(char* array, int array_len);
char* get_type_name(int type);
int get_ent_size(int type);
int get_first_ent(char* array, int array_len);
int get_next_ent(int i, char* array, int array_len);
void move_ent(ent_basics* ent);
char* get_type_name(int type);
void collide_wall(ent_basics* e);
void wallCollision(char* array, int array_len);

#endif
