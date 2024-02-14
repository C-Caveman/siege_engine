// draw to the game window 
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <stdlib.h> // snprintf used for tex fname generation
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

constexpr int ANIM_FPS = 4;
constexpr int MS_PER_ANIM_FRAME = 1000 / ANIM_FPS;
#define MAX_PATH_LEN 256
char path[MAX_PATH_LEN]; // path to the executable
SDL_Texture* textures[MAX_TEXTURES];
// Details for each animation.
struct anim_info anim_data[NUM_ANIM];
float min_frame_time = 1000/60; // second / frames
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Rect window_size; // Dimensions of the screen. Set in the init_graphics() method.
SDL_Rect background;
int fullscreen, window_x, window_y, fps_cap;
int running;
float last_frame_end, frame_time, frame_count, last_sec, fps;
float dt = 0;
TTF_Font* font = 0;

#define MAKE_STRING(x) #x, 
constexpr int MAX_ANIM_NAME_LEN = 32;
char animation_names[NUM_ANIM][MAX_ANIM_NAME_LEN] = { ANIMATION_LIST(MAKE_STRING) };

void skip_comment(FILE* fp) {
    char c = fgetc(fp);
    while (c != '\n' && !feof(fp))
        c = fgetc(fp);
}

// get all the textures for each animation loaded into the game
void load_animations() {
    SDL_Surface* cur_surf;
    char tex_filename[64];
    //char fname_folder_name[] = "graphics/animations/";
    //char fname_bmp[] = ".bmp";
    //char frame_num_string[4];
    char* anim_name = 0;
    //int tex_name_end = 0;
    int anim_name_len = 0;
    int num_frames = 0;
    int total_textures = 0;
    for (int i=0; i<NUM_ANIM; i++) { // load each animation
        num_frames = 0;
        anim_data[i].texture_index = total_textures;
        // find the len of anim_name
        anim_name = animation_names[i];
        anim_name_len = strnlen(anim_name, MAX_ANIM_NAME_LEN);
        // get every frame of the current animation
        //printf("Loading animation %.*s at idx %d, tex_index=%d\n", anim_name_len, anim_name, total_textures, i);
        while (1) {
            // build the filename for the current animation frame
            snprintf(tex_filename, sizeof(tex_filename), "graphics/animations/%.*s/%d.bmp", anim_name_len, anim_name, num_frames);
            // check if the next frame exists
            if (access(tex_filename, F_OK) == 0) {
                // load the image into a surface
                cur_surf = SDL_LoadBMP(tex_filename);
                //printf("Loading frame %d of %.*s at idx %d, i=%d\n", num_frames, ANIM_NAME_LEN, anim_name, total_textures, i);
            }
            else {
                if (DEBUG_GRAPHICS_LOADING) {
                    printf("Didn't find texture: %s, i=%d\n", tex_filename, i);
                    printf("Loaded %d frames of %.*s, i=%d\n", num_frames, anim_name_len, anim_name, i);
                }
                break;
            }
            if (!cur_surf) {
                printf("*** Texture loading error.\n");
                printf("Loaded %d frames of %s, i=%d\n", num_frames, anim_name, i);
                break;
            }
            // set black pixels as transparent
            SDL_SetColorKey(cur_surf, SDL_TRUE, SDL_MapRGB(cur_surf->format, 0, 0, 0));
            // convert the surface to a texture, then store it
            textures[total_textures] = SDL_CreateTextureFromSurface(renderer, cur_surf);
            if (!textures[total_textures])
                printf("***Failed to load texture %s\n", tex_filename);
            num_frames++;
            total_textures++;
            if (cur_surf != 0) { // free the surface used to load the current texture
                SDL_FreeSurface(cur_surf);
            }
            if (total_textures >= MAX_TEXTURES) {
                printf("*** Error: MAX_TEXTURES exceeded in load_animations.\n");
                exit(-1);
            }
        }
        if (num_frames == 0) {
            printf("*** Error: animation graphics/animations/%.*s is missing!\n", ANIM_NAME_LEN, anim_name);
            exit(-1);
        }
        anim_data[i].len = num_frames;
    }
    if (DEBUG_GRAPHICS_LOADING)
        printf("Loaded %d textures (max is %d).\n", total_textures, MAX_TEXTURES);
}


void get_path() {
    // determine the current working directory
    FILE* fp = popen("/bin/pwd", "r"); //TODO support cross-platform
    if (fp == 0) {
        printf("Failed to run command.\n");
        exit(1);
    }
    fgets(path, sizeof(path), fp);
    pclose(fp);
    if (path[0] == 0) { // make sure a path was obtained
        printf("Failed to run PWD to get current directory.\n");
        exit(1);
    }
    // remove the added newline from fgets
    path[strcspn(path, "\n")] = '\0';
    printf("Path: \"%s\"\n", path);
}

void init_fonts() {
    // init font rendering
    if(TTF_Init() < 0) {
        printf("Failed to load SDL2_ttf library: %s\n", TTF_GetError());
        exit(1);
    }
    // build a path to the font file
    char font_path[MAX_PATH_LEN * 2];
    strncpy(font_path, path, MAX_PATH_LEN);
    int base_path_len = strnlen(font_path, MAX_PATH_LEN);
    char font_relative_path[] = "/graphics/fonts/KronaOne-Regular.ttf";
    strncpy(font_path+base_path_len, 
            font_relative_path, 
            strlen(font_relative_path));
    font = TTF_OpenFont(font_path, 24);
    if (font == 0) {
        printf("Failed to load font: %s\n", TTF_GetError());
        printf("Path to font tried: %s\n", font_path);
    }
}

void init_graphics() {
    // find the file path of the executable, primarily to build an absolute path to the font file
    get_path();
    //
    // initialize the window
    //
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        std::cout << "SDL init failed. " << SDL_GetError() << "\n";
    // Set the logical resolution:
    //SDL_RenderSetLogicalSize(renderer, 1920, 1080);
    constexpr int displayIndex = 0;
    if (SDL_GetDisplayBounds(displayIndex, &window_size) != 0)
            printf("*** init_graphics error: %s\n", SDL_GetError());
    // make it fullscreen if config file says so
    if (fullscreen) {
        // Set the resolution to that of the screen:
        window_x = window_size.w;
        window_y = window_size.h;
        if (SDL_CreateWindowAndRenderer(window_x, window_y, 0, &window, &renderer) != 0)
            std::cout << "SDL window/renderer init failed" << SDL_GetError() << "\n";
        printf("\nResolution:\n        %dx%d\n", window_size.w, window_size.h);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
    else {
        // Make a window with the configured size:
        if (SDL_CreateWindowAndRenderer(window_x, window_y, 0, &window, &renderer) != 0)
            std::cout << "SDL window/renderer init failed" << SDL_GetError() << "\n";
    }
    SDL_SetWindowTitle(window, "Hello SDL!");
    // set the framerate via the config
    if (fps_cap > 0)
        min_frame_time = 1000/fps_cap;
    SDL_ShowCursor(1);
    // initialize the background rectangle
    background.x = background.y = 0;
    background.w = window_x;
    background.h = window_x;
    // init some helpful variables
    last_frame_end = frame_time = frame_count = last_sec = fps = 0;
    
    // turn the images in the graphics folder into GPU-usable textures
    load_animations();
    // init the font rendering
    init_fonts();
}

void track_fps() {
    if (last_frame_end > (last_sec + 1000)) {
        last_sec = last_frame_end;
        fps = frame_count;
        frame_count = 0;
        //std::cout << "One second has passed, fps = " << fps << "\n";
        //std::cout << "Delta time was: " << dt << "\n";
    }
}
// Animate and draw all sprites possesed by an entity. ;;
void draw_ent_sprites(vec2f camera_pos, struct ent_basics* e) {
    //if (e->type == scenery_type) { std::cout << "e->chunk: " << e->chunk << "\ne->tile: " << e->tile << "\n\n";}
    int num_sprites = e->num_sprites;
    vec2f p;
    uint32_t anim;
    uint8_t flags;
    int tick;
    int ms_since_last_frame;
    float rotation;
    SDL_Rect ent_render_pos;
    ent_render_pos.w = ent_render_pos.h = RSIZE;
    if (DEBUG_GRAPHICS) { printf("Num sprites in %s entity: %d\n", get_type_name(e->type), num_sprites); }
    sprite* sprites = (sprite*)( (char*)e+sizeof(struct ent_basics) );
    for (int i=0; i<num_sprites; i++) {
        // Get sprite data: (stored after the basic_ent segments)
        anim = sprites[i].anim;
        p =    sprites[i].pos + e->pos;
        tick = sprites[i].anim_tick;
        rotation = sprites[i].rotation;
        //
        // Advance to the next frame if enough time has passed.
        //
        // Pause if on the last frame and not looping:
        if (  (sprites[i].frame == (anim_data[anim].len-1)) &&
             !(sprites[i].flags & LOOPING))
            { sprites[i].flags |= PAUSED; }
        flags = sprites[i].flags;
        // Handle anim_tick overflowing back to lower values:
        ms_since_last_frame = anim_tick - tick + (anim_tick < tick)*256;
        // Update the frame if enough anim_ticks have passed since the last one:
        if (!(flags & PAUSED) && (ms_since_last_frame > MS_PER_ANIM_FRAME)) {
            sprites[i].anim_tick = anim_tick;
            sprites[i].frame += 1;
        }
        // Loop if needed:
        if (sprites[i].frame > (anim_data[anim].len-1)) { sprites[i].frame = 0; }
        // Adjust for screen position:
        ent_render_pos.x = p.x - camera_pos.x;
        ent_render_pos.y = p.y - camera_pos.y;
        //
        // Render the sprite:
        //
        SDL_RenderCopyEx(renderer, 
                         textures[anim_data[anim].texture_index + sprites[i].frame],
                         NULL, 
                         &ent_render_pos, 
                         rotation, 
                         NULL, 
                         SDL_FLIP_NONE);
    }
}
void draw_all_ents(vec2f camera_pos, char* array, int array_len) { // ;;
    int i = 0;
    i = get_first_ent(array, array_len);
    while (i != -1) {
        if (array[i] != HEADER_BYTE) {
            if (DEBUG_GRAPHICS)
                printf("*** Invalid index given by get_next_ent() in draw_all_ents()\n");
            break;
        }
        draw_ent_sprites(camera_pos, (struct ent_basics*)&array[i]);
        i = get_next_ent(i, array, array_len);
    }
}

void present_frame() {
    //
    // finished rendering a frame, 
    // now make sure we don't exceed 60 fps
    //
    frame_time = SDL_GetTicks() - last_frame_end;
    if (frame_time < min_frame_time) {
        SDL_Delay(min_frame_time - frame_time);
    }
    frame_count++;
    //
    // push the finished frame to the window
    //
    SDL_RenderPresent(renderer);
}

void draw_tile_floor(struct tile (*tiles)[CHUNK_WIDTH], int x, int y, vec2f camera_pos) {
    if (x < 0 || x > CHUNK_WIDTH-1 || y < 0 || y > CHUNK_WIDTH-1)
        return;
    struct tile t = tiles[y][x];
    int cur_anim = 0;
    int cur_frame = 0;
    SDL_Rect render_pos;
    render_pos.w = render_pos.h = RSIZE;
    render_pos.x = x*RSIZE - camera_pos.x;
    render_pos.y = y*RSIZE - camera_pos.y;
    //SDL_Rect tile_pos = render_pos;
    // Draw floor:
    cur_anim = anim_data[t.floor_anim].texture_index;
    SDL_RenderCopy(
        renderer, 
        textures[cur_anim+cur_frame], 
        NULL, 
        &render_pos
    );
}

constexpr float TILE_SLIDE_INCREMENT = 20;
//constexpr float growth = TILE_SLIDE_INCREMENT/window_x;
void draw_tile_wall_side(struct tile (*tiles)[CHUNK_WIDTH], int x, int y, vec2f camera_pos, vec2f camera_center) {
    if (x < 0 || x > CHUNK_WIDTH-1 || y < 0 || y > CHUNK_WIDTH-1 || tiles[y][x].wall_height < 1)
        return;
    //float growth = 2;
    struct tile t = tiles[y][x];
    int cur_anim = 0;
    int cur_frame = 0;
    SDL_Rect render_pos;
    render_pos.w = render_pos.h = RSIZE;
    render_pos.x = x*RSIZE - camera_pos.x;
    render_pos.y = y*RSIZE - camera_pos.y;
    SDL_Rect tile_pos = render_pos;
    // Draw wall:
    cur_anim = anim_data[t.wall_side_anim].texture_index;
    vec2f offset = (vec2f{(float)x*RSIZE, (float)y*RSIZE} - camera_center) * TILE_SLIDE_INCREMENT / (window_size.w);
    for (int i=0; i<t.wall_height; i++) {
        render_pos = tile_pos;
        render_pos.x = render_pos.x + offset.x*(i) - i/2;
        render_pos.y = render_pos.y + offset.y*(i) - i/2;
        render_pos.w = render_pos.h = RSIZE + i;
        SDL_RenderCopy(
            renderer,
            textures[cur_anim+cur_frame],
            NULL,
            &render_pos
        );
    }
}

void draw_tile_wall_top(struct tile (*tiles)[CHUNK_WIDTH], int x, int y, vec2f camera_pos, vec2f camera_center) {
    if (x < 0 || x > CHUNK_WIDTH-1 || y < 0 || y > CHUNK_WIDTH-1 || tiles[y][x].wall_height < 1)
        return;
    //float growth = 1;
    struct tile t = tiles[y][x];
    int cur_anim = 0;
    //int cur_frame = 0;
    SDL_Rect render_pos;
    render_pos.w = render_pos.h = RSIZE;
    render_pos.x = -camera_pos.x + x * RSIZE;
    render_pos.y = -camera_pos.y + y * RSIZE;
    SDL_Rect tile_pos = render_pos;
    // Draw wall:
    cur_anim = anim_data[t.wall_side_anim].texture_index;
    vec2f offset = (vec2f{(float)x*RSIZE, (float)y*RSIZE} - camera_center) * TILE_SLIDE_INCREMENT / (window_size.w);
    // Draw the top of the wall:
    if (t.wall_height > 0) {
        cur_anim = anim_data[t.wall_top_anim].texture_index;
        render_pos = tile_pos;
        render_pos.x = render_pos.x + offset.x * t.wall_height - t.wall_height/2;
        render_pos.y = render_pos.y + offset.y * t.wall_height - t.wall_height/2;
        render_pos.w = render_pos.h = RSIZE + t.wall_height;
        SDL_RenderCopy(
            renderer,
            textures[cur_anim],
            NULL,
            &render_pos
        );
    }
}
void draw_chunk_floor(vec2f camera_pos, vec2f camera_center, chunk* chunk, vec2i chunk_index) {
    camera_pos = camera_pos - chunk_index.to_float()*RSIZE*CHUNK_WIDTH; //------ Offset by chunk index.
    struct tile (*tiles)[CHUNK_WIDTH] = chunk->tiles;
    for (int y=0; y<CHUNK_WIDTH; y++) { //----------------------------------- Draw all the floor.
        for (int x=0; x<CHUNK_WIDTH; x++) { draw_tile_floor(tiles, x, y, camera_pos); }
    }
}
void draw_chunk_other(vec2f camera_pos, vec2f camera_center, chunk* chunk, vec2i chunk_index) {
    vec2i chunk_pos = ( (camera_center/RSIZE) +vec2f{0.5,0.5} ).to_int();
    vec2f old_camera = camera_pos;
    camera_pos = camera_pos - chunk_index.to_float()*RSIZE*CHUNK_WIDTH; //------ Offset by chunk index.
    struct tile (*tiles)[CHUNK_WIDTH] = chunk->tiles;
    SDL_Rect render_pos;
    render_pos.x = 0;
    render_pos.y = 0;
    render_pos.w = render_pos.h = RSIZE;
    /*
    bool has_ent = false;
    for (int y=0; y<CHUNK_WIDTH; y++) { //----------------------------------- Draw all the floor.
        for (int x=0; x<CHUNK_WIDTH; x++) {
            has_ent = false;
            for (int i=0; i<MAX_ENTS_PER_TILE; i++) { if (tiles[y][x].ents[i] != 0) { has_ent = true; } }
            if (!has_ent || true) { draw_tile_floor(tiles, x, y, camera_pos); }
        }
    }
    */
    int middle_x = chunk_pos.x - chunk_index.x*CHUNK_WIDTH;
    int middle_y = chunk_pos.y - chunk_index.y*CHUNK_WIDTH;
    for (int ring=CHUNK_WIDTH*2; ring>-1; ring--) { //--------------------- Draw a diamond loop of tiles.
        int spread = 0; // Half the width of a given slice of the diamond.
        for (int row=middle_y-ring; row<=middle_y+ring; row++) {
            vec2i left =  vec2i{middle_x-spread, row};
            vec2i right = vec2i{middle_x+spread, row};
            if (left.in_bounds(0, CHUNK_WIDTH-1)) {
                for (int i=0; i<MAX_ENTS_PER_TILE; i++) { //------------------------------- Draw left side entities.
                        struct ent_basics* e = get_ent(chunk->tiles[left.y][left.x].ents[i]);
                        if (e != nullptr) { draw_ent_sprites(old_camera, e); }
                }
            }
            if (right.in_bounds(0, CHUNK_WIDTH-1)) {
                for (int i=0; i<MAX_ENTS_PER_TILE; i++) { //------------------------------- Draw right side entities.
                        struct ent_basics* e = get_ent(chunk->tiles[right.y][right.x].ents[i]);
                        if (e != nullptr) { draw_ent_sprites(old_camera, e); }
                }
            }
            draw_tile_wall_side(tiles, left.x,  left.y,  camera_pos, camera_center); //-- Left side.
            draw_tile_wall_top(tiles,  left.x,  left.y,  camera_pos, camera_center);
            draw_tile_wall_side(tiles, right.x, right.y, camera_pos, camera_center); //-- Right side.
            draw_tile_wall_top(tiles,  right.x, right.y, camera_pos, camera_center);
            if (row < middle_y) //----- Diverge before reaching the middle row.
                spread++;
            else
                spread--; //----------- Converge after reaching the middle row.
        }
    }
}
void cleanup_graphics() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    for (int i=0; i<MAX_TEXTURES; i++) {
        if (textures[i] != 0)
            SDL_DestroyTexture(textures[i]);
    }
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}
