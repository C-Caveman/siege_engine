#ifndef DEFS
#define DEFS
// Some useful primitives and constants.

// Debug flags:
#define DEBUG_ENTS 0
#define DEBUG_ENT_SPAWNING 0
#define DEBUG_ENT_HANDLES 0
#define DEBUG_GRAPHICS 0
#define DEBUG_GRAPHICS_LOADING 0

// Expansion macros: (X Macros)
#define TO_ENUM(x) x, 
#define TO_STRING(x) #x, 

#include <iostream>
#include <cstring>
#include <cstdint>
struct vec2f;
struct vec2i;
//=================== Vectors =========================================// Vectors //
struct vec2f {
    float x;
    float y;
    //----------------------------------------------------- vec2f functions
    float vlen();
    float dist(vec2f &b);
    float dot(vec2f& v);
    void normalize();
    void semi_normalize(); // don't round x or y down to zero
    vec2f normalized();
    vec2f semi_normalized();
    vec2f floor();
    vec2i to_int();
    vec2i to_int_round_up();
    void print();
    //---------------------------------------------------- operator overloading
    vec2f operator + (const vec2f& v);
    vec2f operator - (const vec2f& v);
    vec2f operator * (const vec2f& v);
    vec2f operator / (const vec2f& v);
    void operator = (const vec2f& v);
    bool operator == (const vec2f& v);
    bool operator != (const vec2f& v);
    friend std::ostream& operator << (std::ostream& os, const vec2f& v);
    // scale vector by an float
    vec2f operator * (const float& scale);
    vec2f operator / (const float& scale);
};
struct vec2i {
    int x;
    int y;
    //---------------------------------------------------- vec2i functions
    vec2f to_float();
    bool in_bounds(int min, int max);
    //---------------------------------------------------- operator overloading
    vec2i operator + (const vec2i& v);
    vec2i operator - (const vec2i& v);
    vec2i operator * (const vec2i& v);
    vec2i operator / (const vec2i& v);
    vec2i operator % (const int modulo);
    void operator = (const vec2i& v);
    bool operator == (const vec2i& v);
    bool operator != (const vec2i& v);
    friend std::ostream& operator << (std::ostream& os, const vec2i& v);
    // scale vector by an float
    vec2i operator * (const float& scale);
    vec2i operator / (const int& scale);
};
//=======================================================================// Entities //
#define RSIZE 80    //--------------- Diameter of entities and tiles.
//--------------------------- Required components of every entity.
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
    vec2i tile;          \
    vec2i chunk;         \
    int health;
typedef int16_t handle; //-------------------- Entity handle.
struct ent_basics { ENT_BASICS }; //----------------------------------- Generic entity.
enum ent_flags {DRAWABLE, ANIMATABLE, MOVABLE, COLLIDABLE, THINKABLE, };
//============================================================================// SPRITES //
enum sprite_flags {
    LOOPING = 1<<0,
    PAUSED  = 1<<1,
};
#define HEADER_BYTE 'H'
struct sprite {
    uint16_t anim;       // Enum value of the animation. (animation data is stored elsewhere)
    uint8_t frame;       // current frame of animation.
    uint8_t anim_tick;   // Tick the previous frame was drawn on.
    uint8_t flags;       // Flags for sprite animation. Looping, stopped, ect.
    vec2f pos;           // Offset from the ent origin
    float rotation;
};
//================================================================// Tiles, Chunks, and Worlds //
#define MAX_ENTS_PER_TILE 4
#define CHUNK_WIDTH 16
#define CHUNK_DIAMETER (RSIZE * CHUNK_WIDTH)
struct tile { //-------------------------------------- Square tile containing a wall/floor.
    unsigned char flags, health, wall_height, unused;
    handle ents[MAX_ENTS_PER_TILE];
    int wall_top_anim, wall_side_anim, floor_anim;
};
struct chunk { //------------------------------------- 16x16 region of tiles
    struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH];
    void set_floors(int wall_top_animation);
    void set_tile(int x, int y, int wall_top_anim, int new_frame);
    void set_floor(int x, int y, int floor_anim);
    void set_wall(int x, int y, int wall_top_anim, int wall_side_anim, unsigned char wall_height);
};
#define WORLD_WIDTH 16
#define WORLD_DIAMETER (CHUNK_DIAMETER * WORLD_WIDTH)
#define MAX_WORLD_NAME_LEN 64
#define MAX_ENTS 2048 //  temp value
#define MAX_CLIENTS 1 // temp value
#define ENTITY_BYTES_ARRAY_LEN 8192
#define MAX_DRAW_DISTANCE 32
struct world { //---------------------------------------------- Collection of chunks and entities.
    char name[MAX_WORLD_NAME_LEN];
    chunk chunks[WORLD_WIDTH][WORLD_WIDTH]; //----------------- Tiles.
    char entity_bytes_array[ENTITY_BYTES_ARRAY_LEN]; //-------- Entities.

    world();
    tile* get_tile(vec2i tile_i);
};
extern struct world* main_world; //---------------------------- Main world.
#endif
