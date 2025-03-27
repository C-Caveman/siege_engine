// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include "../netcode/netcode.h"
#include <unistd.h>
#include <pthread.h>

volatile int running = 0;
struct world test_world = {0};
struct client playerClient;
struct client clients[MAX_CLIENTS] = {0};
uint8_t anim_tick = 0;
uint32_t frameNumber = 0;

char theirIpAddress[] = "192.168.0.237";
int theirPort = 1111;
int myPort = 1111;
int sock = 0;


#define logThread(...) {\
    if (DEBUG_THREADS) \
        printf( __VA_ARGS__ );\
}
sem_t eventCountMutex;
// Listen for events coming from the server:
pthread_t serverListenerThread;
void* serverListener() {
    struct eventsBuffer serverListenBuffer;
    serverListenBuffer.buffer[0].type = 1;
    printf("%d\n", serverListenBuffer.buffer[0].type);
    logThread("Server listener thread enabled!\n");
    char dummyBuf[1024] = {0};
    while (running) {
        /*int messageLen = */udpRecv((char*)&dummyBuf, &sock);
        //printf("serverListener got %d client events.\n", messageLen / (int)sizeof(struct event));
    }
    logThread("Listen thread exiting.\n");
    return 0;
}

#define EVENT_COUNT_BUFFER_SIZE 60
int countBuffer[EVENT_COUNT_BUFFER_SIZE] = {0};
int countBufferPos = 0;
void trackEventCount() {
    if (serverEvents.count > EVENT_BUFFER_SIZE-2) {
        fprintf(stderr, "*** serverEvents buffer overflowing!\n");
    }
    countBuffer[countBufferPos++] = serverEvents.count;
    if (countBufferPos >= EVENT_COUNT_BUFFER_SIZE)
        countBufferPos = 0;
    if (frameNumber < EVENT_COUNT_BUFFER_SIZE) {
        return;
    }
    int average = 0;
    for (int i=0; i<EVENT_COUNT_BUFFER_SIZE; i++) {
        average += countBuffer[i];
    }
    average /= EVENT_COUNT_BUFFER_SIZE;
    //printf("Average events for the last %d frames: %5d\r", EVENT_COUNT_BUFFER_SIZE, average);
}

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
    handle playerHandle = 0;
    if (!playingDemo) {
        E(FrameStart, SDL_GetTicks(), frameNumber++);
        SPAWN(player_type, &playerHandle, (vec2f){RSIZE*(CHUNK_WIDTH/2+1), RSIZE*(CHUNK_WIDTH/2+1)}); // TODO wait for the client to join first
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
        
    }
    /*
    bool playerConnected = false;
    int centiSecondsToWait = 1000;
    E(FrameStart, SDL_GetTicks(), frameNumber++);
    while (!playerConnected) {
        struct event* e = &serverEvents.buffer[serverEvents.readHead];
        if (e->type == eventClientHello) {
            SPAWN(player_type, &playerHandle, (vec2f){RSIZE*(CHUNK_WIDTH/2+1), RSIZE*(CHUNK_WIDTH/2+1)});
            struct dClientHello* hello = &e->data.detClientHello;
            // Add the first player via the above events:
            while (serverEvents.count > 0) {
                takeEvent();
            }
            E(ServerHello, .clientID=hello->clientID, .playerHandle=playerHandle, .clientAddress=hello->clientAddress);
            // Wait for them to ready up:
            centiSecondsToWait = 1000;
        }
        if (e->type == eventClientReady) {
            playerConnected = true;
            break;
        }
        SDL_Delay(10);
        centiSecondsToWait--;
        if (centiSecondsToWait <= 0) {
            printf("*** Server waited too long for client to connect!\n");
            exit(1);
        }
    }
    E(FrameEnd, SDL_GetTicks(), frameNumber);
    */
    p = (struct ent_player*)getEnt(playerHandle, player_type);
    p->pos = (vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)};
    playerClient.player = (struct ent_player*)p;
    ((struct ent_player*)p)->cl = &playerClient;
    
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
            E(FrameEnd, SDL_GetTicks(), frameNumber);
        }
        trackEventCount();
        // Update gamestate from the server's packets:
        while (serverEvents.count > 0) {
            takeEvent();
        }
        
        //TODO send stuff to the client!!
        char sendBuffer[] = "Hello me!!!!!!!!!!!!!!!!!!!!!!!!";
        udpSendN((char*)&sendBuffer, sizeof(sendBuffer), &sock);
        
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

// Client loop implemented in client.c
void* clientLoop();
pthread_t clientThread;

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
    
    // Start networking stuff:
    udpInit(&sock, myPort, theirPort, (char*)theirIpAddress);
    
    // Begin listening for server serverEvents:
    pthread_create(&serverListenerThread, NULL, serverListener, 0); // a thread is born!
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
    //pthread_join(serverListenerThread, 0);
    pthread_join(clientThread, 0);
    pthread_join(serverThread, 0);
    udpShut(&sock);
    //TODO add a listenerShutdown event to join these properly!
    pthread_cancel(serverListenerThread);
    return 0;
}
