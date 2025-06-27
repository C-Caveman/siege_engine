// Update the state of the game world, take in client commands, and tell the clients what changed.

#include "server.h"
#include "../netcode/netcode.h"
#include <unistd.h>
#include <pthread.h>

extern struct world test_world;
extern struct client playerClient;
extern struct client clients[MAX_CLIENTS];
extern uint8_t animTick;
extern uint32_t frameNumber;

extern struct eventBufferFlat      serverEventBuffer;
extern struct eventBufferCircular  serverEventListenBuffer;

extern sem_t serverListenerCountMutex;
extern sem_t clientListenerCountMutex;

extern struct inbox serverInbox;
extern struct outbox outboxToClient;
extern struct outbox outboxToServer;

struct serverState server = {
    .running = false,
    .paused = false,
    .numClients = 0
};

// Listen for events coming from the client:
void* serverListener() {
    struct eventBufferFlat packetBuffer;
    serverInbox.recvBuffer = (char *)packetBuffer.buffer;
    dlog(THREAD, "ServerListener thread enabled!\n");
    while (server.running) {
        int packetLen = inboxRecv(&serverInbox, sizeof(packetBuffer.buffer));
        int numPacketEvents = packetLen / (int)sizeof(struct event);
        packetBuffer.count = numPacketEvents;
        if (numPacketEvents > 0) {
            dlog(SERVER_RECV, "serverListener got %3d events: \n", numPacketEvents);
            for (int i=0; i<numPacketEvents; i++)
                dlog(SERVER_RECV, "%d:%s\n", packetBuffer.buffer[i].type, eventName(packetBuffer.buffer[i].type));
        }
        dlog(SERVER_RECV, "\n");
        
        // Add the events to the ring buffer:
        linearBufferToCircularBuffer(&packetBuffer, &serverEventListenBuffer, packetBuffer.count, &serverListenerCountMutex);
    }
    serverInbox.recvBuffer = 0;
    dlog(THREAD, "ServerListener thread exiting.\n");
    return 0;
}
void recvClientCommands() {
    // Pull client events from the listener's ring buffer:
    circularBufferToFlatBuffer(&serverEventListenBuffer, &serverEventBuffer, &serverListenerCountMutex, 0);
}

#define EVENT_COUNT_BUFFER_SIZE 60
int countBuffer[EVENT_COUNT_BUFFER_SIZE] = {0};
int countBufferPos = 0;
void trackEventCount() {
    if (serverEventBuffer.count > EVENT_BUFFER_SIZE-2)
        fatal( "serverEventBuffer overflowing!\n");
    countBuffer[countBufferPos++] = serverEventBuffer.count;
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
volatile uint32_t tickStartTime = 0;
volatile float serverDt = 0;
#define TICKS_PER_SECOND 128
void* serverLoop() {
    dlog(THREAD, "Server thread enabled!\n");
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
        processEvents();
        
    }
    /*  AND SO IT BEGINS...
    bool playerConnected = false;
    int centiSecondsToWait = 1000;
    E(FrameStart, SDL_GetTicks(), frameNumber++);
    while (!playerConnected) {
        struct event* e = &serverEventBuffer.buffer[serverEventBuffer.count];
        if (e->type == eventClientHello) {
            SPAWN(player_type, &playerHandle, (vec2f){RSIZE*(CHUNK_WIDTH/2+1), RSIZE*(CHUNK_WIDTH/2+1)});
            struct dClientHello* hello = &e->data.detClientHello;
            // Add the first player via the above events:
            processEvents();
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
    if (!playerHandle)
        fatal("Player could not be spawned!");
    p = (struct ent_player*)getEnt(playerHandle, player_type);
    p->pos = (vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)};
    playerClient.player = (struct ent_player*)p;
    ((struct ent_player*)p)->cl = &playerClient;
    
    server.running = true;
    
    while (server.running) {
        tickStartTime = SDL_GetTicks();
        //
        // Read client events, update the game state, and send server events:
        //
        E(FrameStart, tickStartTime, frameNumber++);
        recvClientCommands();
        // Entity updates:
        if (!server.paused) { //TODO add a pauseTime value to prevent glitchyness when unpausing
            thinkAllEnts(mainWorld->entityBytesArray, ENTITY_BYTES_ARRAY_LEN);
            moveAllEnts(mainWorld->entityBytesArray, ENTITY_BYTES_ARRAY_LEN);
            wallCollision(mainWorld->entityBytesArray, ENTITY_BYTES_ARRAY_LEN);
            defragEntArray();
        }
        E(FrameEnd, SDL_GetTicks(), frameNumber);
        trackEventCount();
        // Send the events to the client (TODO do this for ALL CLIENTS, not just the first one!)
        if (!(serverEventBuffer.count == 2 && serverEventBuffer.buffer[0].type == eventFrameStart && serverEventBuffer.buffer[1].type == eventFrameEnd)) {
            dlog(SERVER_SEND, "server sending %3d events: \n", serverEventBuffer.count);
            for (int i=0; i<serverEventBuffer.count; i++)
                dlog(SERVER_SEND, "    %d:%s\n", serverEventBuffer.buffer[i].type, eventName(serverEventBuffer.buffer[i].type));
            dlog(SERVER_SEND, "\n");
            inboxSendAllEvents(&serverInbox, &outboxToClient, serverEventBuffer.count);
        }
        // Update gamestate from the server's packets:
        processEvents();
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
    // Tell the serverListener we are exiting:
    inboxSend(&serverInbox, &outboxToServer, 1*sizeof(serverEventBuffer.buffer[0]));
    dlog(THREAD, "Server thread exiting.\n");
    return 0;
}

