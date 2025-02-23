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
uint32_t clientFrame = 0;

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

// Server thread (simulate the world):
#define logServer(...) {\
    if (DEBUG_SERVER) \
        printf( __VA_ARGS__ );\
}
volatile uint32_t tickStartTime = 0;
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
    struct ent_player* p = (struct ent_player*)spawn(player_type, (vec2f){0,0});
    p->pos = (vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)};
    playerClient.player = (struct ent_player*)p;
    ((struct ent_player*)p)->cl = &playerClient;
    //printf("*Type name: '%s'\n", entTypeName(s->type));
    if (!playingDemo) {
        E(FrameStart, curFrameStart, frameNumber++);
        E(EntSpawn, .entType=zombie_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH/2), RSIZE*(CHUNK_WIDTH+1)});
        E(EntSpawn, .entType=zombie_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH+1), RSIZE*(CHUNK_WIDTH/2)});
        E(EntSpawn, .entType=rabbit_type, .pos=(vec2f){RSIZE*5, RSIZE*5});
        E(EntSpawn, .entType=scenery_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)});
        E(EntSpawn, .entType=spawner_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH/4-0.5), RSIZE*(CHUNK_WIDTH/4-0.5)});
        E(FrameEnd, curFrameStart, frameNumber);
    }
    playMusicLoop(spookyWind1);
    
    while (running) {
        //printf("\rtick: % 5d  frame: % 5d  ", frameNumber, clientFrame);
        tickStartTime = SDL_GetTicks();
        //
        // Read client events, update the game state, and send server events:
        //
        if (!playingDemo) {
            E(FrameStart, curFrameStart, frameNumber++);
            // Entity updates:
            thinkAllEnts(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            move_all_ents(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            wallCollision(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            defragEntArray();
            // Record the player's movement for the demo:
            E(PlayerMove, .p=playerClient.player->h, .pos=playerClient.player->pos, .vel=playerClient.player->vel);
            E(SpriteRotate, .h=playerClient.player->h, .index=PLAYER_GUN, .angle=playerClient.aim_dir);
            E(FrameEnd, curFrameStart, frameNumber);
        }
        // Update gamestate from the server's packets:
        while (events.count > 0) {
            takeEvent();
        }
        
        // Game state updated, now sleep until it's time for the next tick:
        uint32_t tickEndTime = SDL_GetTicks();
        uint32_t tickTimeElapsed = tickEndTime - tickStartTime;
        uint32_t sleepTime = (1000 / TICKS_PER_SECOND) - tickTimeElapsed; // millis to sleep
        #define MAX_SERVER_SLEEP_TIME (1000 / 30)
        dt = ((float)tickTimeElapsed) / 1000.f;
        if (dt > 0.1f) // Cap the delta time.
            dt = 0.05f;
        if (sleepTime < 0) {
            sleepTime = 0;
        }
        if (sleepTime > MAX_SERVER_SLEEP_TIME) {
            sleepTime = (1000 / fps_cap);
        }
        SDL_Delay(sleepTime);
    }
    logThread("Server thread exiting.\n");
    return 0;
}

#define logClient(...) {\
    if (DEBUG_CLIENT) \
        printf( __VA_ARGS__ );\
}
// Client thread (draw the screen, read inputs):
volatile float clientDt = 0;
volatile uint32_t frameStartTime = 0;
pthread_t clientThread;
void* clientLoop() {
    logThread("Client thread enabled!\n");
    init_graphics();
    init_audio();
    frameStartTime = SDL_GetTicks();
    while (running) {
        clientDt = ((float)SDL_GetTicks() - (float)frameStartTime) / 1000.f;
        if (clientDt < 0) {
            printf("clientDt: %f\n", clientDt);
            exit(0);
        }
        frameStartTime = SDL_GetTicks();
        //
        // Player input:
        //
        client_input(&playerClient);
        // Update the client's local copy of the player entity:
        clientUpdatePlayerEntity();
        // Send the client events to the server events buffer: (singleplayer version)
        int cmdEventsSent = 0;
        while (clientCmdEvents.count > 0 && events.count <= EVENT_BUFFER_SIZE-1) {
            memcpy(&events.buffer[events.writeHead], &clientCmdEvents.buffer[clientCmdEvents.readHead], sizeof(clientCmdEvents.buffer[0]));
            memset(&clientCmdEvents.buffer[clientCmdEvents.readHead], 0, sizeof(clientCmdEvents.buffer[0]));
            clientCmdEvents.readHead++;
            events.writeHead++;
            if (clientCmdEvents.readHead >= EVENT_BUFFER_SIZE-1)
                clientCmdEvents.readHead = 0;
            if (events.writeHead >= EVENT_BUFFER_SIZE-1)
                events.writeHead = 0;
            clientCmdEvents.count--;
            cmdEventsSent++;
        }
        sem_wait(&eventCountMutex);
        events.count += cmdEventsSent;
        sem_post(&eventCountMutex);
        //
        // Send client events, read server events, and draw the screen:
        //
        SDL_RenderClear(renderer);
        if (!playerClient.paused) {
            // Clientside animations:
            animateAllEnts(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
            drawWorld(&test_world);
            // DRAW A HUD!
            drawInfo((char*)"fps", fps, 0);
            drawInfo((char*)"heat", (float)playerClient.player->heatTracker, 1);
            drawInfo((char*)"zombies", (float)mainWorld->numZombies, 2);
            clientShowDialog();
        }
        else {
            renderMenu(&playerClient);
        }
        SDL_RenderPresent(renderer);
        frame_count++;
        
        // Screen and inputs updated, now sleep until it's time for the next frame:
        uint32_t frameEndTime = SDL_GetTicks();
        uint32_t frameTimeElapsed = frameEndTime - frameStartTime;
        uint32_t sleepTime = (1000 / fps_cap) - frameTimeElapsed; // millis to sleep
        #define MAX_CLIENT_SLEEP_TIME (1000 / 30)
        if (sleepTime < 0) {
            sleepTime = 0;
        }
        if (sleepTime > MAX_CLIENT_SLEEP_TIME) {
            sleepTime = (1000 / fps_cap);
        }
        SDL_Delay(sleepTime);
        clientFrame++;
    }
    logThread("Client thread exiting.\n");
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
    int numDemoEvents = demoFileSize / sizeof(events.buffer[0]);
    int numDemoEventsRead = 0;
    uint32_t nextDemoFrameTime = 0;
    
    // Begin listening for server events:
    pthread_create(&listenThread, NULL, eventListener, 0); // a thread is born!
    // Begin updating the game state:
    pthread_create(&serverThread, NULL, serverLoop, 0);
    // Begin accepting inputs and rendering the screen:
    pthread_create(&clientThread, NULL, clientLoop, 0);
    
    #define TO_SIZE_PRINT(name, ...) printf("%32s: %3ld bytes long.\n", #name, sizeof(struct d##name));
    EVENT_LIST(TO_SIZE_PRINT)
    
    if (timeScale < 0.01)
        timeScale = 1;
    //
    //;;; GAME LOOP:
    //
    while (running) {
        curFrameStart = SDL_GetTicks()*timeScale;
        /*
        dt = ((float)curFrameStart - (float)lastFrameEnd) / 1000.f;
        if (dt > 0.1f) // Cap the delta time.
            dt = 0.05f;
        */
        anim_tick = SDL_GetTicks() % 256; //- 8-bit timestamp for animations.
        lastFrameEnd = SDL_GetTicks()*timeScale;
        track_fps();
        
        
        // Record demo:
        if (recordingDemo && demoFile && events.count > 0) {
            fwrite(events.buffer, sizeof(events.buffer[0]), events.count, demoFile);
        }
        // Play demo:
        if (playingDemo && demoFile) {
            playerClient.player->sprites[PLAYER_CROSSHAIR].flags |= INVISIBLE;
            while (nextDemoFrameTime < curFrameStart && numDemoEventsRead < numDemoEvents && events.count < EVENT_BUFFER_SIZE-2 && !feof(demoFile)) {
                // peek at the next event's FrameStart time
                int gotAnEvent = fread(&events.buffer[events.count], sizeof(events.buffer[0]), 1, demoFile);
                if (gotAnEvent == 1 && events.buffer[events.count].type == eventFrameStart) {
                    nextDemoFrameTime = events.buffer[events.count].data.detFrameStart.time;
                }
                events.count += (gotAnEvent == 1);
                numDemoEventsRead += 1;
            }
            if (numDemoEventsRead >= numDemoEvents) {
                printf("**** END OF DEMO!!!\n");
                running = false;
            }
        }
        
        if (playingDemo && numDemoEventsRead >= numDemoEvents)
            break;
        
        
        
        ////////////////////////////////////////////////////////////////////////
        // Rendering:
        ////////////////////////////////////////////////////////////////////////
        /*
        SDL_RenderClear(renderer);
        drawWorld(&test_world);
        // DRAW A HUD!   
        drawInfo((char*)"fps", fps, 0);
        drawInfo((char*)"heat", (float)playerClient.player->heatTracker, 1);
        drawInfo((char*)"zombies", (float)mainWorld->numZombies, 2);
        clientShowDialog();
        present_frame(); // Put the frame on the screen:
        */
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
    //pthread_cancel(listenThread); // Stop listening for server packets.
    return 0;
}
