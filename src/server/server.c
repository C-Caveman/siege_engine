// update the world using messages from the clients,
// update the clients on what has happened (if multiplayer)

#include "server.h"
#include <unistd.h> // testing memory leaks with sleep()

// entity and client data are in these arrays
int num_entities = 0;
int num_clients = 0;

struct client playerClient;

// each entity gets a unique ID number
int id = 0;
int new_id() {return ++id;}
uint8_t anim_tick = 0;



/*
> Save the current pos.
> Update the position.
> Check if we entered a new tile.
    > Move our handle from the old to the new tile.
> Check if we entered a new chunk.
    > Update our chunk.
*/
void move_all_ents(char* array, int array_len) {
    entBasics* e;
    for (int i=getFirstEnt(array, array_len); i != -1; i=getNextEnt(i, array, array_len)) {
        if (array[i] != HEADER_BYTE) { printf("*** Invalid index given by getNextEnt() in move_all_ents()\n"); exit(-1); }
                                                                                    //- move the entity, record its position in the chunk
        e = (entBasics*)&array[i];
        vec2i old_tile = e->tile;                                                   //- Old tile.
        vec2i old_chunk = e->chunk;                                                 //- Old chunk.
        moveEnt(e);
        e->chunk = v2fToI(v2fScalarDiv( v2fAdd(e->pos,(vec2f){RSIZE/2,RSIZE/2}), (RSIZE*CHUNK_WIDTH) ));
        vec2f floored = v2fSub(e->pos, v2iToF(v2iScale(e->chunk, RSIZE*CHUNK_WIDTH)));
        e->tile = v2fToI(v2fAdd(v2fScalarDiv(floored, RSIZE), (vec2f){0.5,0.5}));
        bool changed_tile = !v2iIsEq(e->tile, old_tile);                                  //- New tile?
        bool old_tile_was_valid = v2iInBounds(old_tile, 0, CHUNK_WIDTH);
        bool new_tile_was_valid = v2iInBounds(e->tile, 0, CHUNK_WIDTH);
        bool old_chunk_was_valid = v2iInBounds(old_chunk, 0, WORLD_WIDTH) && old_tile_was_valid;
        bool new_chunk_was_valid = v2iInBounds(e->chunk, 0, WORLD_WIDTH) && new_tile_was_valid;
        struct tile* old_tile_ptr = &mainWorld->chunks[old_chunk.y][old_chunk.x].tiles[old_tile.y][old_tile.x];
        struct tile* new_tile_ptr = &mainWorld->chunks[e->chunk.y][e->chunk.x].tiles[e->tile.y][e->tile.x];
        if (changed_tile) {
            if (old_chunk_was_valid)
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) {                               //- Remove handle from old tile.
                if (old_tile_ptr->ents[i] == e->h)
                    old_tile_ptr->ents[i] = 0; /* old_tile_ptr->floor_anim = stonedk; */
            }
            int numGibsInTile = 0;
            if (new_chunk_was_valid)
                for (int i=0; i<MAX_ENTS_PER_TILE; i++) {
                    entBasics* tileEnt = getEnt(new_tile_ptr->ents[i], 0);
                    numGibsInTile += (tileEnt && tileEnt->type == gib_type);
                }
            bool tooManyGibs = (numGibsInTile > MAX_ENTS_PER_TILE*3/4);
            if (new_chunk_was_valid) {
                entBasics* firstTileEnt = getEnt(new_tile_ptr->ents[0], 0);
                if (e->type != gib_type && firstTileEnt && firstTileEnt->type == gib_type && numGibsInTile > 0) {
                    new_tile_ptr->ents[0] = e->h;
                }
            }
            if (new_chunk_was_valid)
                for (int i=0; i<MAX_ENTS_PER_TILE; i++) { //------------------------------------------------------------ Store handle in new tile.
                    entBasics* tileEnt = getEnt(new_tile_ptr->ents[i], 0);
                    if (tileEnt == 0 || (i == MAX_ENTS_PER_TILE-1 && tileEnt && e->type != gib_type && tileEnt->type == gib_type && tooManyGibs)) {
                        new_tile_ptr->ents[i] = e->h;
                        break;
                    }
                } //----- NOTE: copy_handle() isn't used on e->h here. Use it for sharing e->h with other ents.
        }
    }
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
    p->pos = (vec2f){RSIZE,RSIZE};
    playerClient.player = (struct ent_player*)p;
    ((struct ent_player*)p)->cl = &playerClient;
    //printf("*Type name: '%s'\n", entTypeName(s->type));
    if (!playingDemo) {
        E(EntSpawn, .entType=zombie_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH/2), RSIZE*(CHUNK_WIDTH+1)});
        E(EntSpawn, .entType=zombie_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH+1), RSIZE*(CHUNK_WIDTH/2)});
        E(EntSpawn, .entType=rabbit_type, .pos=(vec2f){RSIZE*5, RSIZE*5});
        E(EntSpawn, .entType=scenery_type, .pos=(vec2f){RSIZE*(CHUNK_WIDTH/2-0.5), RSIZE*(CHUNK_WIDTH/2-0.5)});
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
            clientUpdatePlayerEntity();                                   //- Client_Inputs -> Player_Entity.
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
        
        // Do clientside animations (zombies' per-frame rotation towards the player):
        animateAllEnts(mainWorld->entity_bytes_array, ENTITY_BYTES_ARRAY_LEN);
        
        ////////////////////////////////////////////////////////////////////////
        // Rendering:
        ////////////////////////////////////////////////////////////////////////
        SDL_RenderClear(renderer);
        playerClient.camera_pos = v2fSub(v2fAdd(p->pos, HW), v2fScale((vec2f){window_x,window_y}, (RSIZE/tileWidth/2)));
        playerClient.camera_center = p->pos;
        #define OFFSETS 9
        vec2i order[OFFSETS] = {
            {-1,1}, {1,1}, {-1,-1}, {1,-1}, //- Diagonally adjacent chunks.
            {-1,0}, {0,1}, {1,0}, {0,-1}, //--- Directly adjacent chunks.
            {0,0} //--------------------------- Current chunk.
        };
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = v2iAdd(p->chunk, order[i]);
            if ( v2iInBounds(next_chunk, 0, WORLD_WIDTH-1) ) {                        //- Floor pass.
                draw_chunk_floor(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    (vec2i){next_chunk.x, next_chunk.y});
            }
        }
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = v2iAdd(p->chunk, order[i]);
            if ( v2iInBounds(next_chunk, 0,WORLD_WIDTH-1) ) {                        //- Short entity pass.
                chunkDrawShortEnts(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    (vec2i){next_chunk.x, next_chunk.y});
            }
        }
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = v2iAdd(p->chunk, order[i]);
            if ( v2iInBounds(next_chunk, 0,WORLD_WIDTH-1) ) {                        //- Tall entity pass.
                chunkDrawTallEnts(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    (vec2i){next_chunk.x, next_chunk.y});
            }
        }
        for (int i=0; i<OFFSETS; i++) {
            vec2i next_chunk = v2iAdd(p->chunk, order[i]);
            if ( v2iInBounds(next_chunk,0,WORLD_WIDTH-1) ) {                        //- Wall pass.
                draw_chunk_walls(
                    playerClient.camera_pos, playerClient.camera_center,
                    &test_world.chunks[next_chunk.y][next_chunk.x],
                    (vec2i){next_chunk.x, next_chunk.y});
            }
        }
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
    return 0;
}
