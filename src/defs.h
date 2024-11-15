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

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define F_PI 3.14159
//=================== Vectors =========================================// Vectors //
typedef struct vec2f {
    float x;
    float y;
} vec2f;
typedef struct vec2i {
    int x;
    int y;
} vec2i;
//----------------------------------------------------- vec2f functions
float v2fLen(vec2f v);
float v2fDist(vec2f a, vec2f b);
float v2fDot(vec2f a, vec2f b);
void v2fNormalize(vec2f* v);
vec2f v2fNormalized(vec2f v);
vec2f v2fFloor(vec2f v);
vec2i v2fToI(vec2f v);
vec2i v2fToIRoundUp(vec2f v);
void v2fPrint(vec2f v);
vec2f v2fAdd(vec2f a, vec2f b);
vec2f v2fSub(vec2f a, vec2f b);
vec2f v2fMult(vec2f a, vec2f b);
vec2f v2fDiv(vec2f a, vec2f b);
void v2fEq(vec2f a, vec2f b);
bool v2fIsEq(vec2f a, vec2f b);
bool v2fNotIsEq(vec2f a, vec2f b);
char* v2fToString(vec2f v);
// scale vector by a float
vec2f v2fScale(vec2f v, float scale);
vec2f v2fScalarDiv(vec2f v, float scale);
//----------------------------------------------------- vec2i functions
float v2iLen(vec2i v);
float v2iDist(vec2i a, vec2i b);
float v2iDot(vec2i a, vec2i b);
void v2iNormalize(vec2i* v);
vec2i v2iNormalized(vec2i v);
vec2f v2iToF(vec2i v);
void v2iPrint(vec2i v);
vec2i v2iAdd(vec2i a, vec2i b);
vec2i v2iSub(vec2i a, vec2i b);
vec2i v2iMult(vec2i a, vec2i b);
vec2i v2iDiv(vec2i a, vec2i b);
void v2iEq(vec2i a, vec2i b);
bool v2iIsEq(vec2i a, vec2i b);
bool v2iNotIsEq(vec2i a, vec2i b);
char* v2iToString(vec2i v);
vec2i v2iModulo(vec2i v, int modulo);
bool v2iInBounds(vec2i v, int min, int max);
// scale vector by a float
vec2i v2iScale(vec2i v, float scale);
vec2i v2iScalarDiv(vec2i v, float scale);
//===================================================== Data stored for a particular animation.
struct anim_info {
    uint32_t texture_index;  // First frame in texture array.
    uint8_t  len;            // Number of frames.
    uint8_t  keyframe_0;     // Frame where an event occurs.
    uint8_t  keyframe_1;     // Frame where an event occurs.
    uint8_t  keyframe_2;     // Frame where an event occurs.
};
//===================================================== Counter that ticks up after 'interval' milliseconds:
struct counter {
    uint8_t prevTick;
    uint8_t interval;
    uint8_t flags;
    uint8_t count;
};
void counterInc(struct counter* c);
void counterDec(struct counter* c);
//=======================================================================// Entities //
#define RSIZE 80    //--------------- Diameter of entities and tiles.
#define HW (vec2f){RSIZE/2,RSIZE/2} //- Half the width of a tile/entity.
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
typedef struct { ENT_BASICS } ent_basics; //----------------------------------- Generic entity.
enum ent_flags {
    NODRAW =      1,
    NOANIMATION = 2,
    NOMOVE =      2*2,
    NOFRICTION =  2*2*2,
    NOCOLLISION = 2*2*2*2,
    NOTHINK =     2*2*2*2*2,
};
//============================================================================// SPRITES //
enum sprite_flags {
    LOOPING =   1<<0,
    PAUSED  =   1<<1,
    INVISIBLE = 1<<2,
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
#define MAX_ENTS_PER_TILE 8
#define CHUNK_WIDTH 16
#define CHUNK_DIAMETER (RSIZE * CHUNK_WIDTH)
struct tile { //-------------------------------------- Square tile containing a wall/floor.
    unsigned char flags, health, wall_height, unused;
    handle ents[MAX_ENTS_PER_TILE];
    int wall_top_anim, wall_side_anim, floor_anim;
};
struct chunk { //------------------------------------- 16x16 region of tiles
    struct tile tiles[CHUNK_WIDTH][CHUNK_WIDTH];
};
void chunkSetFloors(struct chunk* c, int wall_top_animation);
void chunkSetTile(struct chunk* c, int x, int y, int wall_top_anim, int new_frame);
void chunkSetFloor(struct chunk* c, int x, int y, int floor_anim);
void chunkSetWall(struct chunk* c, int x, int y, int wall_top_anim, int wall_side_anim, unsigned char wall_height);
#define WORLD_WIDTH 16
#define WORLD_DIAMETER (CHUNK_DIAMETER * WORLD_WIDTH)
#define MAX_WORLD_NAME_LEN 64
#define MAX_ENTS 2048 //  temp value
#define MAX_CLIENTS 1 // temp value
#define ENTITY_BYTES_ARRAY_LEN 8192*16
#define MAX_DRAW_DISTANCE 32
struct world { //---------------------------------------------- Collection of chunks and entities.
    char name[MAX_WORLD_NAME_LEN];
    struct chunk chunks[WORLD_WIDTH][WORLD_WIDTH]; //----------------- Tiles.
    char entity_bytes_array[ENTITY_BYTES_ARRAY_LEN]; //-------- Entities.
};
void initMainWorld();
struct tile* worldGetTile(vec2i tile_i);
struct tile* worldTileFromPos(vec2f pos);
extern struct world* mainWorld; //---------------------------- Main world.
extern uint8_t anim_tick; //----------------------------------- Frame counter for animations.
extern struct client playerClient; //-------------------------- Player client.

/////////////////////////////////////////////////////////////////////////////////////////////////////////// ;;
// Events: (defined in ent.cpp)
#define EVENT_LIST(f) \
    f(PlayerMove, handle p; vec2f pos; vec2f vel;) \
    f(PlayerShoot, handle p; vec2f pos; vec2f aimDir;) \
    f(EntMove, handle e; vec2f pos; vec2f vel;) \

#define TO_EVENT_PREFIXED_ENUM(name, detailsUnused) event##name, 
enum EVENT_ENUM {
    EVENT_LIST(TO_EVENT_PREFIXED_ENUM)
    NUM_EVENTS
};
#define TO_EVENT_STRUCT_DECLARATION(name, d) struct details##name { d }; 
EVENT_LIST(TO_EVENT_STRUCT_DECLARATION)
#define TO_EVENT_FUNCTION_PROTOTYPE(name, detailsUnused) void ev##name(struct details##name* d);
EVENT_LIST(TO_EVENT_FUNCTION_PROTOTYPE)
#define TO_EVENT_STRUCT_UNION_MEMBER(name, detailsUnused) struct details##name d##name; 
struct event {
    int type;
    union { // Event details:
        EVENT_LIST(TO_EVENT_STRUCT_UNION_MEMBER)
        int dummy;
    } details;
};
struct packet {
    int time;
    int sequenceNumber;
    struct event e;
};
#define EVENT_BUFFER_SIZE 2048
struct eventsBuffer {
    int count;
    int index;
    int sequenceNumber;
    struct packet buffer[EVENT_BUFFER_SIZE];
};
extern struct eventsBuffer events;
void applyEvent(struct event* ev);
void makeEvent(struct event e);
void takeEvent();
#define EVENT(eventName, ...) makeEvent((struct event) { event##eventName, .d##eventName = { __VA_ARGS__ }})


//////////////////////////////////////////////////////////////////////////////////// ;;
// Utility functions:
float randf(); // Random float in range: [0,1]
float randfn(); // Random float in range: [-1,1]
vec2f angleToVector(float angle); // Convert an angle to a normalized vector.
float vectorToAngle(vec2f v); // Convert a vector to an angle.
float fclamp(float n, float min, float max);
int   iclamp(int n, int min, int max);

#endif
