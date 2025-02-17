// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include <unistd.h>
#include <pthread.h>

struct client playerClient;
uint8_t anim_tick = 0;


// Listen for events coming from the server:
pthread_t listenThread;
void* eventListener() {
    printf("Listen thread enabled!\n");
    while (1) {
        // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< UDP recv goes here
        sleep(1);
        if (!running) {
            printf("Listen thread exiting.\n");
            break;
        }
    }
    return 0;
}

void gameTick() {
    
}




int main() {
    applyConfig((char*)"config/config.txt");                                                           //===========// Initialize server. //
    running = 1;
    init_graphics();
    init_audio();
    struct world test_world;
    mainWorld = &test_world;
    initMainWorld();
                                                                               //==============// Place tiles. //
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
    }                                                                          //==============// Spawn entities. //
    struct ent_player* p = (struct ent_player*)spawn(player_type, (vec2f){0,0});
    p->pos = (vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)};
    playerClient.player = (struct ent_player*)p;
    ((struct ent_player*)p)->cl = &playerClient;
    //printf("*Type name: '%s'\n", entTypeName(s->type));
    if (!playingDemo) {
        E(EntSpawn, .entType=zombie_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH/2), RSIZE*(CHUNK_WIDTH+1)});
        E(EntSpawn, .entType=zombie_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH+1), RSIZE*(CHUNK_WIDTH/2)});
        E(EntSpawn, .entType=rabbit_type, .pos=(vec2f){RSIZE*5, RSIZE*5});
        E(EntSpawn, .entType=scenery_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)});
        E(EntSpawn, .entType=spawner_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH/4-0.5), RSIZE*(CHUNK_WIDTH/4-0.5)});
    }
    playMusicLoop(spookyWind1);
    
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
    
    pthread_create(&listenThread, NULL, eventListener, 0); // a thread is born!
    
    #define TO_SIZE_PRINT(name, ...) printf("%32s: %3ld bytes long.\n", #name, sizeof(struct d##name));
    EVENT_LIST(TO_SIZE_PRINT)
    
    if (timeScale < 0.01)
        timeScale = 1;
    
    uint32_t frameNumber = 0;
    //;;; GAME LOOP:
    while (running) {
        curFrameStart = SDL_GetTicks()*timeScale;
        dt = ((float)curFrameStart - (float)lastFrameEnd) / 1000.f;
        if (dt > 0.1f) // Cap the delta time.
            dt = 0.05f;
        anim_tick = SDL_GetTicks() % 256; //- 8-bit timestamp for animations.
        lastFrameEnd = SDL_GetTicks()*timeScale;
        track_fps();
        
        if (!playingDemo) {
            E(FrameStart, curFrameStart, frameNumber++);
            // Client input:
            client_input(&playerClient);
            if (playerClient.paused) {
                //memcpy(playerClient.menuText, pauseMenuText, sizeof(playerClient.menuText));
                SDL_RenderClear(renderer);
                renderMenu(&playerClient);
                //renderText(playerClient.menuText[0]);
                present_frame();
                continue;
            }
            // Update the client's local copy of the player entity:
            clientUpdatePlayerEntity();
            // Send the client events to the server events buffer: (singleplayer version)
            while (clientEvents.count > 0 && events.count <= EVENT_BUFFER_SIZE-1) {
                memcpy(&events.buffer[events.writeHead], &clientEvents.buffer[clientEvents.readHead], sizeof(clientEvents.buffer[0]));
                memset(&clientEvents.buffer[clientEvents.readHead], 0, sizeof(clientEvents.buffer[0]));
                clientEvents.readHead++;
                events.writeHead++;
                if (clientEvents.readHead >= EVENT_BUFFER_SIZE-1)
                    clientEvents.readHead = 0;
                if (events.writeHead >= EVENT_BUFFER_SIZE-1)
                    events.writeHead = 0;
                clientEvents.count--;
                events.count++;
            }
        }
        // Entity updates (server):
        if (!playingDemo)
            thinkAllEnts(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        move_all_ents(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        wallCollision(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        defragEntArray();
        // Record the player's movement for the demo:
        if (!playingDemo) {
            E(PlayerMove, .p=playerClient.player->h, .pos=playerClient.player->pos, .vel=playerClient.player->vel);
            E(SpriteRotate, .h=playerClient.player->h, .index=PLAYER_GUN, .angle=playerClient.aim_dir);
            E(FrameEnd, curFrameStart, frameNumber);
        }
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
        // Update gamestate from the server's packets:
        while (events.count > 0) {
            takeEvent();
        }
        if (playingDemo && numDemoEventsRead >= numDemoEvents)
            break;
        
        // Do clientside animations (zombies' per-frame rotation towards the player and rabbit wiggle):
        animateAllEnts(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        
        ////////////////////////////////////////////////////////////////////////
        // Rendering:
        ////////////////////////////////////////////////////////////////////////
        SDL_RenderClear(renderer);
        drawWorld(&test_world);
        // DRAW A HUD!   
        drawInfo((char*)"fps", fps, 0);
        drawInfo((char*)"heat", (float)playerClient.player->heatTracker, 1);
        drawInfo((char*)"zombies", (float)mainWorld->numZombies, 2);
        clientShowDialog();
        present_frame(); // Put the frame on the screen:
    }
    printf("Server was running for %d seconds.\n", SDL_GetTicks() / 1000);
    if (demoFile)
        fclose(demoFile);
    cleanup_graphics();
    cleanup_audio();
    pthread_cancel(listenThread); // Stop listening for server packets.
    return 0;
}
