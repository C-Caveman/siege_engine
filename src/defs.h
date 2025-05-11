#ifndef DEFS
#define DEFS
// Some useful primitives and constants.

// Debug logging:
#define dlog(category, ...) {\
    if (LOG_##category) \
        printf( __VA_ARGS__ );\
}
// Debug flags:
#define DEBUG_ENTS 0
#define DEBUG_ENT_SPAWNING 0
#define DEBUG_ENT_HANDLES 0
#define DEBUG_GRAPHICS 0
#define DEBUG_GRAPHICS_LOADING 0
#define LOG_THREAD 0
#define DEBUG_CLIENT 1
#define DEBUG_SERVER 0
#define DEBUG_EVENTS 0
#define DEBUG_DIALOG 0
#define LOG_MENU 0
// Print an error message with file/lineNum, then kill the program.
#define fatal(message) { fprintf(stderr, "*** %s:%d *** " message "\n", __FILE__, __LINE__); exit(1); }
// Temporary hack for singleplayer movement smoothness:
#define SINGLEPLAYER_HACK 1

// Expansion macros: (X Macros)
#define TO_ENUM(x) x, 
#define TO_STRING(x) #x, 

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
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
//===================================================== Track number of times X milliseconds have passed.
struct timer { // Framerate-independent timer.
    uint32_t start;
    int count;
};
void timerStart(struct timer* t);
void timerUpdate(struct timer* t, uint32_t intervalMillis);
void timerUpdate30FPS(struct timer* t);
bool passedTimestamp(uint32_t t);
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
    uint16_t flags;      \
    uint16_t health;     \
    uint32_t nextThink; \
    vec2f pos;           \
    vec2f vel;           \
    vec2f dir;           \
    vec2i tile;          \
    vec2i chunk;         \

typedef uint16_t handle; //-------------------- Entity handle.
typedef struct { ENT_BASICS } entBasics; //----------------------------------- Generic entity.
enum ent_flags {
    NODRAW =       1<<0,
    NO_ANIMATION = 1<<1,
    NOMOVE =       1<<2,
    NOFRICTION =   1<<3,
    NOCOLLISION =  1<<4,
    NOTHINK =      1<<5,
    SHOOTABLE =    1<<6,
    HEIGHT_LOWEST =1<<7,
    HEIGHT_LOW =   1<<8,
    HEIGHT_HIGH =  1<<9,
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
#define MAX_ENTS_PER_TILE 16
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
#define MAX_CLIENTS 32 // temp value
#define ENTITY_BYTES_ARRAY_LEN 2000000
#define MAX_DRAW_DISTANCE 32
#define MAX_GIBS 512
struct world { //---------------------------------------------- Collection of chunks and entities.
    char name[MAX_WORLD_NAME_LEN];
    struct chunk chunks[WORLD_WIDTH][WORLD_WIDTH]; //----------------- Tiles.
    char entity_bytes_array[ENTITY_BYTES_ARRAY_LEN]; //-------- Entities.
    int entArraySpace;
    int numGibs;
    int numZombies;
};
void initMainWorld();
struct tile* worldGetTile(vec2i tile_i);
struct tile* worldTileFromPos(vec2f pos);
vec2i worldTileIndexFromPos(vec2f pos);
vec2i tileNumberToIndex(uint32_t tileNumber);
uint32_t tileIndexToNumber(vec2i tileIndex);
extern struct world* mainWorld; //---------------------------- Main world.
extern uint8_t anim_tick; //----------------------------------- Frame counter for animations.
extern struct client playerClient; //-------------------------- Player client (for the client side).

// Server details:
struct serverState {
    bool running;
    bool paused;
    int numClients;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////// ;;
// Events: (defined in ent.cpp)
#define EVENT_LIST(f) \
    f(Invalid, int foobarScoobar;) \
    f(FrameStart, uint32_t time; uint32_t frameNumber;) \
    f(FrameEnd, uint32_t time; uint32_t frameNumber;) \
    f(ClientHello, uint32_t clientID; uint32_t clientAddress; uint16_t clientFlags;) \
    f(SpawnPlayer, uint32_t clientID; handle playerHandle;) \
    f(ServerHello, uint32_t clientID; handle playerHandle; uint32_t clientAddress;) \
    f(ClientReady, uint32_t clientID;) \
    f(ClientPause, uint32_t clientID;) \
    f(ClientQuit, uint32_t clientID;) \
    f(PlaySound, uint32_t sound; uint32_t channel;) \
    f(Use, handle user; handle target;) \
    f(Explode, handle h;) \
    f(ZombieDie, handle h;) \
    f(ZombieWindShieldSplatter, handle h;) \
    f(PlayerMove, handle p; vec2f pos; vec2f vel;) \
    f(PlayerShoot, handle p; vec2f shootPos; float shootDir;) \
    f(ChangeTile, uint32_t tileNumber; uint32_t wall; uint32_t wallSide; uint32_t height; uint32_t floor;) \
    f(EntMove, handle h; vec2f pos; vec2f vel;) \
    f(ForceSpawn, int entType; vec2f pos; handle h;) \
    f(Despawn, handle h;) \
    f(TriggerDialog, handle p; char fileName[16];) \
    f(SpriteRotate, handle h; int index; float angle;) \
    f(SpriteSetAnim, handle h; int index; uint16_t anim; uint16_t frame;) \

#define TO_EVENT_PREFIXED_ENUM(name, detailsUnused) event##name, 
enum EVENT_ENUM {
    EVENT_LIST(TO_EVENT_PREFIXED_ENUM)
    NUM_EVENTS
};
char* eventName(int eventType);
#define EVENT_PARAMS 7
struct param {
    union {
        int i;
        float f;
        char c[4];
    } intSizedItem;
};
#define TO_STRUCTS(eventName, details) struct d##eventName { int type; details };
EVENT_LIST(TO_STRUCTS)
#define TO_EVENT_FUNCTION_PROTOTYPE(name, detailsUnused) void ev##name(struct d##name* d);
EVENT_LIST(TO_EVENT_FUNCTION_PROTOTYPE)
#define TO_STRUCT_UNION(eventName, details) struct d##eventName det##eventName;
struct event {
    int type;
    union {
        struct param params[EVENT_PARAMS];
        EVENT_LIST(TO_STRUCT_UNION)
    } data;
};
#define EVENT_BUFFER_SIZE 8192*4
struct eventBufferFlat {
    int count;
    int sequenceNumber;
    struct event buffer[EVENT_BUFFER_SIZE];
};
struct eventBufferCircular {
    int count;
    int readHead;
    int writeHead;
    int sequenceNumber;
    struct event buffer[EVENT_BUFFER_SIZE];
};
#define MAX_PACKET_EVENTS (508 / sizeof(struct event))
extern volatile uint32_t curFrameStart;
extern volatile uint32_t tickStartTime;
extern volatile uint32_t frameStartTime;
extern volatile float clientDt;
extern volatile float serverDt;
extern struct eventBufferFlat       serverEventBuffer;
extern struct eventBufferCircular   serverEventListenBuffer;
extern sem_t                        serverListenerCountMutex;
extern struct eventBufferFlat       clientEventBuffer;
extern struct eventBufferCircular   clientEventListenBuffer;
extern sem_t                        clientListenerCountMutex;
extern struct eventBufferFlat       clientCommandEventsBuffer;
void applyEvent(struct event* ev);
void makeEvent(struct event e);
void processEvents();
void sendEvents(struct eventBufferFlat* eBuff, int* inSocket, int* outSocket);
// Queue up a server event: (to be sent to the client)
#define E(eventName, ...) {\
    if (serverEventBuffer.count < EVENT_BUFFER_SIZE-2) { \
        serverEventBuffer.buffer[serverEventBuffer.count].data.det##eventName = (struct d##eventName) { event##eventName, __VA_ARGS__ }; \
        serverEventBuffer.buffer[serverEventBuffer.count].type = event##eventName;\
        serverEventBuffer.count++; \
    } \
    else { \
        printf("*** Too many events to add to serverEventBuffer this frame!!\n"); \
        exit(-1); \
    } \
}
// Run an event immediately instead of putting it in the queue:
#define E_IMMEDIATE(eventName, ...) { \
    struct d##eventName immediateEventDetails = { event##eventName, __VA_ARGS__ }; \
    ev##eventName(&immediateEventDetails); \
}
// Queue up a client event: (to be sent to the server)
#define CE(eventName, ...) {\
    if (clientCommandEventsBuffer.count < EVENT_BUFFER_SIZE-2) { \
        clientCommandEventsBuffer.buffer[clientCommandEventsBuffer.count].data.det##eventName = (struct d##eventName) { event##eventName, __VA_ARGS__ }; \
        clientCommandEventsBuffer.buffer[clientCommandEventsBuffer.count].type = event##eventName;\
        clientCommandEventsBuffer.count++; \
    } \
    else { \
        printf("*** Too many events to add to clientCommandEventsBuffer this frame!!\n"); \
        exit(-1); \
    } \
}
handle reserveHandle(uint16_t entType); //-------- Set aside a handle to be assigned an entity later.
// Spawn an entity by queing up a spawn event (only to be called by the server)
#define SPAWN(typeOfEntity, handleRef, ...) {\
    handle newHandle = reserveHandle(typeOfEntity); \
    if (handleRef != 0) {\
        *((handle*)handleRef) = newHandle;\
    }\
    E(ForceSpawn, .entType=typeOfEntity, .pos=__VA_ARGS__, .h=newHandle);\
}
// Spawn an entity immediately (only to be called by the server)
#define SPAWN_IMMEDIATE(typeOfEntity, handleRef, ...) {\
    handle newHandle = reserveHandle(typeOfEntity); \
    if (handleRef != 0) {\
        *((handle*)handleRef) = newHandle;\
    }\
    E_IMMEDIATE(ForceSpawn, .entType=typeOfEntity, .pos=__VA_ARGS__, .h=newHandle);\
}

//////////////////////////////////////////////////////////////////////////////////// ;;
// Utility functions:
float randf(); // Random float in range: [0,1]
float randfs(); // Random float in range: [0,1] (squared, so biased towards zero)
float randfn(); // Random float in range: [-1,1]
float randfns(); // Random float in range: [-1,1] (biased toward zero; squared without changing the sign)
vec2f angleToVector(float angle); // Convert an angle to a normalized vector.
float vectorToAngle(vec2f v); // Convert a vector to an angle.
float fclamp(float n, float min, float max);
int   iclamp(int n, int min, int max);

#endif
