#ifndef ENT
#define ENT

// entities

#include "../defs.h"
#include "../graphics/animations.h"
#include <SDL2/SDL.h>

#define RSIZE 128    // diameter of rectangular entities
#define CIRCLE_R 64  // radius of circular entities


/////////////////////////////////////////////////////////////////////////////////////////
// Segments are the chunks of memory making up an entity. (each one is 64 bits long)
//

enum segment_types {
    SEG_HEAD, SEG_POS, SEG_CHUNK, SEG_VEL, SEG_DIR, SEG_ANIM, SEG_HEALTH, SEG_MOVETYPE,
};

constexpr uint8_t HEADER_BYTE = 255; // Marker for identifying header segments.
enum ent_flags {DRAWABLE, ANIMATABLE, MOVABLE, COLLIDABLE, DAMAGABLE, THINKABLE};

struct seg_head {        // Header segment. Used by all entities.
    uint8_t header_byte; // For ensuring this is indeed a header segment.
    uint8_t type;        // Type of entity.
    uint8_t size;        // Width of this entity (in segments).
    uint8_t num_sprites; // Number of sprites to draw for this entity.
    uint16_t flags;      // Basic qualities of this ent: drawable, movable, collideable, ect.
    uint16_t id;         // Unique entity id number.
};
struct seg_pos {         // Entity position.
    vec2f pos;
};
struct seg_cur_chunk {   // Chunk the entity is in.
    vec2i cur_chunk;
};
struct seg_vel {         // Entity velocity.
    vec2f vel;
};
struct seg_dir {         // Entity direction.
    vec2f dir;
};
struct seg_anim {        // Data for an entity's animation.
    uint16_t anim;       // Enum value of the animation. (animation data is stored elsewhere)
    uint8_t frame;       // current frame of animation.
    uint8_t flags;       // Flags for changing how an animation is displayed.
    float rotation;      // Rotation of the sprite.
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
    struct seg_head      head;
    struct seg_pos       pos;
    struct seg_cur_chunk cur_chunk;
    struct seg_vel       vel;
    struct seg_dir       dir;
    struct seg_anim      anim;
    struct seg_health    health;
    struct seg_movetype  movetype;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Structure of each entity. >>>Implementations in actions.cpp<<<
/*
ENTITY STRUCTURE:
    > header (ent size, flags, num sprites, ect.)

    > cur_chunk
    > position
    > velocity

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
// Define a list of entity types. Can be reformatted by redefining f().
#define MAX_ENTITY_TYPE_NAME_LEN 32
#define ENTITY_TYPES_LIST \
    f(EMPTY) \
    f(PLAYER) \
    f(SCENERY)
#undef f
#define f(x) x, 
enum entity_types {
    ENTITY_TYPES_LIST
    NUM_ENT_TYPES
};

// Segments common to most entities.
enum ent_basics_segments {////////////////// Basics of every entity.
    head, cur_chunk, pos, vel,
    basic_ent_size
};

enum sprite_components {//////////////////// Sprite data (used by most entities).
    sprite_position_segment,
    sprite_animation_segment,
    sprite_size
};

enum empty_ent_sprites {};///////////////// empty entity
enum empty_ent_segments {};
struct ent_EMPTY {
    void init();
    void think();
};

enum player_segments {///////////////////// player
    player_end_of_basics=basic_ent_size-1,
        p_sprite_body_pos,
        p_sprite_body_anim,
        p_sprite_gun_pos,
        p_sprite_gun_anim,
    player_dir,
    player_movetype,
    player_size
};
#define num_player_sprites (p_sprite_gun_anim - player_end_of_basics)/2
struct ent_PLAYER {
    segment data[player_size];
    void init();
    void think();
};

enum scenery_segments {/////////////////// scenery
    scenery_end_of_basics=basic_ent_size-1,
        scenery_sprite_pos,
        scenery_sprite_anim,
    more_scenery_stuff,
    scenery_size
};
#define num_scenery_sprites (scenery_sprite_anim - scenery_end_of_basics)/2
struct ent_SCENERY {
    segment data[scenery_size];
    void init();
    void think();
};

/////////////////////////////////////////////////////////////////////////////////////////
// Entity functions.
//
segment* spawn_ent(int type, segment* array, int array_len);
void despawn_ent(segment* ent);
void think_all_ents(segment* array, int array_len);
char* get_type_name(int type);
int get_ent_size(int type);
int get_first_ent(segment* array, int array_len);
int get_next_ent(int i, segment* array, int array_len);
void move_ent(segment* e);





#endif
