// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include <unistd.h>
#include <pthread.h>

volatile int running = 0;
struct world test_world = {0};
struct client playerClient;
uint8_t anim_tick = 0;
uint32_t frameNumber = 0;

#define logThread(...) {\
    if (DEBUG_THREADS) \
        printf( __VA_ARGS__ );\
}
sem_t eventCountMutex;
// Listen for events coming from the server:
pthread_t listenThread;
void* eventListener() {
    logThread("Listen thread enabled!\n");
    while (running) {
        // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP recv goes here
        SDL_Delay(100);
    }
    logThread("Listen thread exiting.\n");
    return 0;
}

// Client loop implemented in client.c
void* clientLoop();
pthread_t clientThread;

// Server thread (simulate the world):
#define logServer(...) {\
    if (DEBUG_SERVER) \
        printf( __VA_ARGS__ );\
}
volatile uint32_t tickStartTime = 0;
volatile float serverDt = 0;
#define TICKS_PER_SECOND 128
pthread_t serverThread;
void* serverLoop() {
    logThread("Server thread enabled!\n");
    mainWorld = &test_world;
    initMainWorld();
     // Place tiles:
    struct chunk* chunk_0 = &test_world.chunks[0][0];
    test_world.chunks[1][0].tiles[4][4].wall_height = 16;
    //chunk_0->set_floors(floor_test);
    chunkSetFloors(chunk_0, tileGold01);
    for (int y=0; y<WORLD_WIDTH; y++) {
        for (int x=0; x<WORLD_WIDTH; x++)
            chunkSetFloors(&test_world.chunks[y][x], tileMetal04);
    }
    for (int y=0; y<CHUNK_WIDTH; y++) {
        chunkSetWall(chunk_0, 0,y, wall_steel,wall_steel_side,16);
        chunkSetWall(chunk_0, CHUNK_WIDTH-1,y, wall_steel,wall_steel_side,16);
    }
    for (int x=0; x<CHUNK_WIDTH; x++) {
        chunkSetWall(chunk_0, x,0, wall_steel,wall_steel_side,16);
        chunkSetWall(chunk_0, x,CHUNK_WIDTH-1, wall_steel,wall_steel_side,16);
    }
    // Spawn entities:
    
    struct ent_player* p = 0;//(struct ent_player*)spawn(player_type, (vec2f){0,0});
    if (!playingDemo) {
        E(FrameStart, SDL_GetTicks(), frameNumber++);
        handle playerHandle = 0;
        SPAWN(player_type, &playerHandle, (vec2f){RSIZE*(CHUNK_WIDTH/2+1), RSIZE*(CHUNK_WIDTH/2+1)});
        SPAWN(zombie_type, 0, (vec2f){RSIZE*(CHUNK_WIDTH/2), RSIZE*(CHUNK_WIDTH+1)});
        SPAWN(zombie_type, 0, (vec2f){RSIZE*(CHUNK_WIDTH+1), RSIZE*(CHUNK_WIDTH/2)});
        SPAWN(rabbit_type, 0, (vec2f){RSIZE*(CHUNK_WIDTH-1), RSIZE*(1)});
        SPAWN(scenery_type, 0, (vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)});
        SPAWN(spawner_type, 0, (vec2f){RSIZE*(CHUNK_WIDTH/4-0.5), RSIZE*(CHUNK_WIDTH/4-0.5)});
        E(FrameEnd, SDL_GetTicks(), frameNumber);
        // Set the initial gamestate via the above events:
        while (serverEvents.count > 0) {
            takeEvent();
        }
        p = (struct ent_player*)getEnt(playerHandle, player_type);
        p->pos = (vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)};
        playerClient.player = (struct ent_player*)p;
        ((struct ent_player*)p)->cl = &playerClient;
    }
    playMusicLoop(spookyWind1);
    
    bool playerConnected = false;
    int deciSecondsToWait = 100;
    while (!playerConnected) {
        struct event* e = &serverEvents.buffer[serverEvents.readHead];
        if (e->type == eventClientHello) {
            struct dClientHello* hello = &e->data.detClientHello;
            E(ConnectClient, hello->clientID, 0);
            break;
        }
        SDL_Delay(100);
        deciSecondsToWait--;
        if (deciSecondsToWait <= 0) {
            printf("*** Waited too long for client to connect!\n");
            exit(1);
        }
    }
    
    while (running) {
        tickStartTime = SDL_GetTicks();
        //
        // Read client events, update the game state, and send server events:
        //
        if (!playingDemo) {
            E(FrameStart, tickStartTime, frameNumber++);
            // Entity updates:
            thinkAllEnts(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            moveAllEnts(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            wallCollision(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            defragEntArray();
            // Record the player's movement for the demo:
            E(PlayerMove, .p=playerClient.player->h, .pos=playerClient.player->pos, .vel=playerClient.player->vel);
            E(SpriteRotate, .h=playerClient.player->h, .index=PLAYER_GUN, .angle=playerClient.aim_dir);
            E(FrameEnd, SDL_GetTicks(), frameNumber);
        }
        // Update gamestate from the server's packets:
        while (serverEvents.count > 0) {
            takeEvent();
        }
        
        // Game state updated, now sleep until it's time for the next tick:
        uint32_t tickEndTime = SDL_GetTicks();
        uint32_t tickTimeElapsed = tickEndTime - tickStartTime;
        uint32_t sleepTime = (1000 / TICKS_PER_SECOND) - tickTimeElapsed; // millis to sleep
        #define MAX_SERVER_SLEEP_TIME (1000 / 30)
        serverDt = ((float)tickTimeElapsed) / 1000.f;
        serverDt = fclamp(serverDt, 0.f, 0.05f);
        sleepTime = fclamp(sleepTime, 0, MAX_SERVER_SLEEP_TIME);
        SDL_Delay(sleepTime);
    }
    logThread("Server thread exiting.\n");
    return 0;
}

int main() {
    //
    // Initialize server:
    //
    sem_init(&eventCountMutex, 0, 1);
    applyConfig((char*)"config/config.txt");
    running = 1;
    
    
    // Demo recording:
    char demoFileName[] = "demos/demo001.bin";
    FILE* demoFile = 0;
    if (recordingDemo)
        demoFile = fopen(demoFileName, "wb");
    if (recordingDemo && !demoFile) {
        printf("*** Failed to create/open %s.\n", demoFileName);
        exit(-1);
    }
    
    // Demo playback:
    if (playingDemo)
        demoFile = fopen(demoFileName, "r");
    if (playingDemo && !demoFile) {
        printf("*** Failed to open %s.\n", demoFileName);
        exit(-1);
    }
    if (playingDemo && recordingDemo) {
        printf("*** playingDemo and recordingDemo at the same time is not allowed!\n");
        exit(-1);
    }
    int demoFileSize = 0;
    if (playingDemo) {
        /* Size of file */
        fseek(demoFile, 0, SEEK_END);
        demoFileSize = ftell(demoFile);
        fseek(demoFile, 0, SEEK_SET);
    }
    int numDemoEvents = demoFileSize / sizeof(serverEvents.buffer[0]);
    int numDemoEventsRead = 0;
    uint32_t nextDemoFrameTime = 0;
    
    #define DEBUG_EVENT_SIZES 0
    #define TO_SIZE_PRINT(name, ...) printf("%32s: %3ld bytes long.\n", #name, sizeof(struct d##name));
    if (DEBUG_EVENT_SIZES) {
        printf("Event packet sizes:\n");
        EVENT_LIST(TO_SIZE_PRINT)
        printf("\n");
    }
    
    // Begin listening for server serverEvents:
    pthread_create(&listenThread, NULL, eventListener, 0); // a thread is born!
    // Begin updating the game state:
    pthread_create(&serverThread, NULL, serverLoop, 0);
    // Begin accepting inputs and rendering the screen:
    pthread_create(&clientThread, NULL, clientLoop, 0);
    
    
    if (timeScale < 0.01)
        timeScale = 1;
    //
    //;;; GAME LOOP:
    //
    while (running) {
        // Record demo:
        if (recordingDemo && demoFile && serverEvents.count > 0) {
            fwrite(serverEvents.buffer, sizeof(serverEvents.buffer[0]), serverEvents.count, demoFile);
        }
        // Play demo:
        if (playingDemo && demoFile) {
            playerClient.player->sprites[PLAYER_CROSSHAIR].flags |= INVISIBLE;
            while (nextDemoFrameTime < curFrameStart && numDemoEventsRead < numDemoEvents && serverEvents.count < EVENT_BUFFER_SIZE-2 && !feof(demoFile)) {
                // peek at the next event's FrameStart time
                int gotAnEvent = fread(&serverEvents.buffer[serverEvents.count], sizeof(serverEvents.buffer[0]), 1, demoFile);
                if (gotAnEvent == 1 && serverEvents.buffer[serverEvents.count].type == eventFrameStart) {
                    nextDemoFrameTime = serverEvents.buffer[serverEvents.count].data.detFrameStart.time;
                }
                serverEvents.count += (gotAnEvent == 1);
                numDemoEventsRead += 1;
            }
            if (numDemoEventsRead >= numDemoEvents) {
                printf("**** END OF DEMO!!!\n");
                running = false;
            }
        }
        
        if (playingDemo && numDemoEventsRead >= numDemoEvents)
            break;
        
        SDL_Delay(100);
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    if (demoFile)
        fclose(demoFile);
    cleanup_graphics();
    cleanup_audio();
    pthread_join(listenThread, 0);
    pthread_join(clientThread, 0);
    pthread_join(serverThread, 0);
    //pthread_cancel(listenThread); // Could use pthread_cancel() for a timer-based emergency thread killing system failsafe.
    return 0;
}
