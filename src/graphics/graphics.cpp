// draw to the game window 
#include "graphics.h"
#include "../client/client.h"
#include <unistd.h>
#include <iostream>
#include <stdlib.h> // snprintf used for tex fname generation
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

float VERTICAL_TILES_VISIBLE = 14;
float tileWidth = RSIZE;
float texelWidth = tileWidth / TILE_PIXEL_DIAMETER;
constexpr int ANIM_FPS = 32;
constexpr int MS_PER_ANIM_FRAME = 1000 / ANIM_FPS;
#define MAX_PATH_LEN 256
char path[MAX_PATH_LEN]; // path to the executable
SDL_Texture* textures[MAX_TEXTURES];
// Details for each animation.
struct anim_info anim_data[NUM_ANIM];
float min_frame_time = 1000/60; // default is 60 fps
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Rect window_size; // Dimensions of the screen. Set in the init_graphics() method.
SDL_Rect background;
int running;
float last_frame_end, cur_frame_start, frame_time, frame_count, last_sec, fps;
float dt = 0;
TTF_Font* font = 0;

constexpr int MAX_ANIM_NAME_LEN = 32;
char animation_names[NUM_ANIM][MAX_ANIM_NAME_LEN] = { ANIMATION_LIST(TO_STRING) };

void skip_comment(FILE* fp) {
    char c = fgetc(fp);
    while (c != '\n' && !feof(fp))
        c = fgetc(fp);
}

bool load_frame(char* tex_filename, SDL_Surface* cur_surf, int* num_frames, int* total_textures) {
    if (access(tex_filename, F_OK) == 0)
        { cur_surf = SDL_LoadBMP(tex_filename); }
    else
        { return false; }
    SDL_SetColorKey(cur_surf, SDL_TRUE, SDL_MapRGB(cur_surf->format, 0, 0, 0));
    textures[*total_textures] = SDL_CreateTextureFromSurface(renderer, cur_surf);
    if (cur_surf != 0) { SDL_FreeSurface(cur_surf); }
    if (!textures[*total_textures])
        { return false; }
    *num_frames = *num_frames + 1;
    *total_textures = *total_textures + 1;
    return true;
}

// get all the textures for each animation loaded into the game
void load_animations() {
    SDL_Surface* cur_surf = 0;
    char tex_filename[64];
    char* anim_name = 0;
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
            snprintf(tex_filename, sizeof(tex_filename), "assets/graphics/animations/%.*s/%d.bmp", anim_name_len, anim_name, num_frames);
            if (access(tex_filename, F_OK) == 0) {
                bool loaded_tex = load_frame(tex_filename, cur_surf, &num_frames, &total_textures);
                if (!loaded_tex) { printf("*** Couldn't load texture %s!\n", tex_filename); exit(-1); }
            }
            else if (num_frames == 0) { //---------------------------------------------------------- No frames! Load the placeholder texture instead!
                snprintf(tex_filename, sizeof(tex_filename), "placeholders/graphics/animations/missing_animation/0.bmp");
                bool loaded_missing_tex = load_frame(tex_filename, cur_surf, &num_frames, &total_textures);
                if (!loaded_missing_tex) { printf("*** Couldn't load %s!\n", tex_filename); exit(-1); }
            }
            else { break; }
        }
        if (total_textures >= MAX_TEXTURES) { printf("*** Error: MAX_TEXTURES (%d) exceeded in load_animations.\n", MAX_TEXTURES); exit(-1); }
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
    (void)!fgets(path, sizeof(path), fp);
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
    char font_path[] = "assets/graphics/fonts/FreeMonoBold.ttf";
    char font_placeholder_path[] = "placeholders/graphics/fonts/KronaOne-Regular.ttf";
    font = TTF_OpenFont(font_path, 24);
    if (font == 0) {
        printf("Failed to load font %s\n%s\n", font_path, TTF_GetError());
        font = TTF_OpenFont(font_placeholder_path, 24);
    }
    if (font == 0) { printf("Failed to load font %s\n%s\n", font_placeholder_path, TTF_GetError()); exit(-1); }
}

void setTileWidth() {
    tileWidth = std::ceil(window_y / VERTICAL_TILES_VISIBLE);
    texelWidth = tileWidth / TILE_PIXEL_DIAMETER;
    std::cout << "tileWidth for res (" << window_x << ", " << window_y << ") = " << tileWidth << "\n";
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
    SDL_SetWindowTitle(window, windowName);
    setTileWidth();
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
    SDL_RenderSetVSync(renderer, vsync);
}

void goFullscreen() {
    SDL_SetWindowSize(window, window_size.w,window_size.h);
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    window_x = window_size.w;
    window_y = window_size.h;
    fullscreen = 1;
    setTileWidth();
}
void goWindowed() {
    SDL_SetWindowFullscreen(window, 0);
    SDL_SetWindowSize(window, 800, 800);
    window_x = 800;
    window_y = 800;
    fullscreen = 0;
    setTileWidth();
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
    ent_render_pos.w = ent_render_pos.h = tileWidth;
    if (DEBUG_GRAPHICS) { printf("Num sprites in %s entity: %d\n", get_type_name(e->type), num_sprites); }
    sprite* sprites = (sprite*)( (char*)e+sizeof(struct ent_basics) );
    sprite* s;
    for (int i=0; i<num_sprites; i++) {
        // Get sprite data: (stored after the basic_ent segments)
        s = &sprites[i];
        anim = s->anim;
        p =    s->pos + e->pos;
        tick = s->anim_tick;
        rotation = s->rotation;
        flags = s->flags;
        //
        // Advance to the next frame if enough time has passed.
        //
        // Pause if on the last frame and not looping:
        bool isLastFrame = (s->frame == (anim_data[anim].len-1));
        if (  isLastFrame && !(s->flags & (uint8_t)LOOPING)) {
            s->flags |= PAUSED;
            //printf("Stopped anim for %s.\n", get_type_name(e->type));
        }
        // Handle anim_tick overflowing back to lower values:
        ms_since_last_frame = anim_tick - tick + (anim_tick < tick)*256;
        // Update the frame if enough anim_ticks have passed since the last one:
        if (!(flags & PAUSED) && (ms_since_last_frame > MS_PER_ANIM_FRAME)) {
            sprites[i].anim_tick = anim_tick;
            sprites[i].frame += 1;
        }
        // Loop if needed:
        if (sprites[i].frame >= anim_data[anim].len) {
            sprites[i].frame = 0;
        }
        // Adjust for screen position:
        ent_render_pos.x = (p.x - camera_pos.x)*(tileWidth/RSIZE);
        ent_render_pos.y = (p.y - camera_pos.y)*(tileWidth/RSIZE);
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
    render_pos.w = render_pos.h = tileWidth;
    render_pos.x = (x*RSIZE - camera_pos.x)*(tileWidth/RSIZE);
    render_pos.y = (y*RSIZE - camera_pos.y)*(tileWidth/RSIZE);
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

// DELTA between layers needs to be less than one TEXEL_WIDTH.
// TEXEL_WIDTH = TILE_WIDTH / 32_TEXELS_ACROSS
// MAX_DISP = 
//
constexpr float TILE_SLIDE_INCREMENT = 20;
//constexpr float growth = TILE_SLIDE_INCREMENT/window_x;
void draw_tile_wall_side(struct tile (*tiles)[CHUNK_WIDTH], int x, int y, vec2f camera_pos, vec2f camera_center) { // ;;
    if (x < 0 || x > CHUNK_WIDTH-1 || y < 0 || y > CHUNK_WIDTH-1 || tiles[y][x].wall_height < 1)
        return;
    //float growth = 2;
    struct tile t = tiles[y][x];
    int cur_anim = 0;
    int cur_frame = 0;
    SDL_Rect render_pos;
    render_pos.w = render_pos.h = tileWidth;
    render_pos.x = (x*RSIZE - camera_pos.x)*(tileWidth/RSIZE);
    render_pos.y = (y*RSIZE - camera_pos.y)*(tileWidth/RSIZE);
    SDL_Rect tile_pos = render_pos;
    // Draw wall:
    cur_anim = anim_data[t.wall_side_anim].texture_index;
    vec2f offset = (vec2f{(float)x*RSIZE, (float)y*RSIZE} - camera_center) * TILE_SLIDE_INCREMENT / (window_size.w);
    for (int i=0; i<t.wall_height; i++) {
        render_pos = tile_pos;
        render_pos.x = render_pos.x + offset.x*(i) - i/2;
        render_pos.y = render_pos.y + offset.y*(i) - i/2;
        render_pos.w = render_pos.h = tileWidth + i;
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
    render_pos.w = render_pos.h = tileWidth;
    render_pos.x = (-camera_pos.x + x * RSIZE)*(tileWidth/RSIZE);
    render_pos.y = (-camera_pos.y + y * RSIZE)*(tileWidth/RSIZE);
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
        render_pos.w = render_pos.h = tileWidth + t.wall_height;
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
void draw_chunk_ents(vec2f camera_pos, vec2f camera_center, chunk* chunk, vec2i chunk_index) {
    vec2i chunk_pos = ( (camera_center/RSIZE) +vec2f{0.5,0.5} ).to_int();
    vec2f old_camera = camera_pos;
    camera_pos = camera_pos - chunk_index.to_float()*RSIZE*CHUNK_WIDTH; //------ Offset by chunk index.
    camera_center = camera_center - chunk_index.to_float()*RSIZE*CHUNK_WIDTH; //------ Offset by chunk index.
    //struct tile (*tiles)[CHUNK_WIDTH] = chunk->tiles;
    SDL_Rect render_pos;
    render_pos.x = 0;
    render_pos.y = 0;
    render_pos.w = render_pos.h = RSIZE;
    int middle_x = chunk_pos.x - chunk_index.x*CHUNK_WIDTH;
    int middle_y = chunk_pos.y - chunk_index.y*CHUNK_WIDTH;
    for (int ring=MAX_DRAW_DISTANCE; ring>-1; ring--) { //--------------------- Draw a diamond loop of tiles.
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
            if (row < middle_y) //----- Diverge before reaching the middle row.
                spread++;
            else
                spread--; //----------- Converge after reaching the middle row.
        }
    }
}
void draw_chunk_walls(vec2f camera_pos, vec2f camera_center, chunk* chunk, vec2i chunk_index) {
    vec2i chunk_pos = ( (camera_center/RSIZE) +vec2f{0.5,0.5} ).to_int();
    //vec2f old_camera = camera_pos;
    camera_pos = camera_pos - chunk_index.to_float()*RSIZE*CHUNK_WIDTH; //------ Offset by chunk index.
    camera_center = camera_center - chunk_index.to_float()*RSIZE*CHUNK_WIDTH; //------ Offset by chunk index.
    struct tile (*tiles)[CHUNK_WIDTH] = chunk->tiles;
    SDL_Rect render_pos;
    render_pos.x = 0;
    render_pos.y = 0;
    render_pos.w = render_pos.h = RSIZE;
    int middle_x = chunk_pos.x - chunk_index.x*CHUNK_WIDTH;
    int middle_y = chunk_pos.y - chunk_index.y*CHUNK_WIDTH;
    for (int ring=MAX_DRAW_DISTANCE; ring>-1; ring--) { //--------------------- Draw a diamond loop of tiles.
        int spread = 0; // Half the width of a given slice of the diamond.
        for (int row=middle_y-ring; row<=middle_y+ring; row++) {
            vec2i left =  vec2i{middle_x-spread, row};
            vec2i right = vec2i{middle_x+spread, row};
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
void renderText(char* text) {
    SDL_Color White = {255, 255, 255};  // this is the color in rgb format, maxing out all would give you the color white, and it will be your text's color
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, text, White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage); //now you can convert it into a texture
    SDL_Rect Message_rect;
    float charWidth = window_y/16;
    Message_rect.x = window_x/2 - strnlen(text, 256)*charWidth/2 /* + sin(last_frame_end / 128) * window_x / 16 */;
    Message_rect.y = window_y/32;
    Message_rect.w = strnlen(text, 256) * charWidth;
    Message_rect.h = window_y/16;
    
    SDL_RenderCopy(renderer, textures[anim_data[black].texture_index], NULL, &Message_rect);
    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);
}
void drawDebugRectangle(int x, int y, int w, int h) {
    SDL_Rect testRect = SDL_Rect {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // set rectangle color
    SDL_RenderFillRect(renderer, &testRect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void drawFps(float fps) {
    #define fpsStringSize 256
    char fpsText[fpsStringSize];
    snprintf(fpsText, fpsStringSize, "fps: %.0f", fps);
    
    
    SDL_Color White = {255, 255, 255};
    int charWidth = window_x / 64;
    int charHeight = charWidth * 2;
    int numChars = strlen(fpsText);
    #define MAX_LINE_BUFFER_SIZE 256
    char lineBuffer[MAX_LINE_BUFFER_SIZE] = {0};
    int maxLineChars = (window_x / charWidth) - 1;
    if (maxLineChars > MAX_LINE_BUFFER_SIZE)
        maxLineChars = MAX_LINE_BUFFER_SIZE;
    int wrapThreshold = maxLineChars * 0.9;
    int lineNumber = 0;
    int curLineChars = 0;
    // Print the text:
    for (int i=0; i<numChars; i++, curLineChars++) {
        if ( (curLineChars >= wrapThreshold && fpsText[i] == ' ') || (curLineChars >= maxLineChars) ) {
            lineNumber++;
            curLineChars = 0;
            //Skip leading whitespace.
            while (fpsText[i] == ' ')
                i++;
        }
        lineBuffer[0] = fpsText[i];
        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, lineBuffer, White);
        SDL_Texture* charTexture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_Rect charBox = SDL_Rect {charWidth*(curLineChars+1), charHeight*lineNumber + window_y/128, charWidth, charHeight};
        SDL_RenderCopy(renderer, textures[anim_data[black].texture_index], NULL, &charBox);
        SDL_RenderCopy(renderer, charTexture, NULL, &charBox);
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(charTexture);
    }
}
void drawTextBox(char* text, int numCharsToPrint) {
    SDL_Color White = {255, 255, 255};
    int charWidth = window_x / 64;
    int charHeight = charWidth * 2;
    int numChars = strlen(text);
    #define MAX_LINE_BUFFER_SIZE 256
    char lineBuffer[MAX_LINE_BUFFER_SIZE] = {0};
    int maxLineChars = (window_x / charWidth) - 1;
    if (maxLineChars > MAX_LINE_BUFFER_SIZE)
        maxLineChars = MAX_LINE_BUFFER_SIZE;
    int wrapThreshold = maxLineChars * 0.8;
    int lineNumber = 0;
    int curLineChars = 0;
    // Print the text:
    for (int i=0; i<numChars; i++, curLineChars++) {
        if (i > numCharsToPrint)
            break;
        if ( (curLineChars >= wrapThreshold && text[i] == ' ') || (curLineChars >= maxLineChars) ) {
            lineNumber++;
            curLineChars = 0;
            //Skip leading whitespace.
            while (text[i] == ' ')
                i++;
        }
        if (text[i] == '\n') {
            lineNumber++;
            curLineChars = 0;
            continue;
        }
        lineBuffer[0] = text[i];
        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, lineBuffer, White);
        SDL_Texture* charTexture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_Rect charBox = SDL_Rect {charWidth*(curLineChars+1), charHeight*lineNumber + window_y*3/4, charWidth, charHeight};
        SDL_RenderCopy(renderer, textures[anim_data[black].texture_index], NULL, &charBox);
        SDL_RenderCopy(renderer, charTexture, NULL, &charBox);
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(charTexture);
    }
}
void drawDialogBox(struct client* cl) {
    char* text = cl->dialogPrintString;
    int numCharsToPrint = cl->dialogCharsPrinted;
    SDL_Color White = {255, 255, 255};
    int charWidth = window_x / 64;
    int charHeight = charWidth * 2;
    int numChars = strlen(text);
    #define MAX_LINE_BUFFER_SIZE 256
    char lineBuffer[MAX_LINE_BUFFER_SIZE] = {0};
    int portraitSize = window_y / 5;
    int portraitPaddingX = window_x / 20;
    int portraitPaddingY = window_y / 20;
    int maxLineChars = (window_x - portraitSize) / charWidth - 1;
    if (maxLineChars > MAX_LINE_BUFFER_SIZE)
        maxLineChars = MAX_LINE_BUFFER_SIZE;
    int wrapThreshold = maxLineChars * 0.8;
    int lineNumber = 0;
    int curLineChars = 0;
    // Print the text:
    for (int i=0; i<numChars; i++) {
        if (i > numCharsToPrint)
            break;
        if (lineNumber > 3) {
            cl->dialogCharsPrinted = numCharsToPrint = 0;
            memset(cl->dialogPrintString, 0, sizeof(cl->dialogPrintString)-1); //TODO handle this in client::dialogUpdate()
            break;
        }
        if ( (curLineChars >= wrapThreshold && text[i] == ' ') || (curLineChars >= maxLineChars) ) {
            lineNumber++;
            curLineChars = 0;
            //Skip leading whitespace.
            while (isspace(text[i]) && i < numChars)
                i++;
        }
        if (text[i] == '\n' && curLineChars > 1) {
            lineNumber++;
            curLineChars = 0;
            continue;
        }
        else if (text[i] == '\n') { // Don't try to draw the newline as a symbol.
            curLineChars = 0;
            continue;
        }
        curLineChars++;
        lineBuffer[0] = text[i];
        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, lineBuffer, White);
        SDL_Texture* charTexture = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_Rect charBox = SDL_Rect {
            charWidth*(curLineChars+1) + portraitSize + portraitPaddingX, 
            charHeight*lineNumber + window_y - portraitSize - portraitPaddingY, 
            charWidth, charHeight
        };
        SDL_RenderCopy(renderer, textures[anim_data[black].texture_index], NULL, &charBox);
        SDL_RenderCopy(renderer, charTexture, NULL, &charBox);
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(charTexture);
    }
    // Draw the portrait:
    SDL_Rect portraitBox = SDL_Rect {
        portraitPaddingX,
        window_y - portraitSize - portraitPaddingY,
        portraitSize, portraitSize
    };
    SDL_RenderCopy(renderer, textures[anim_data[black].texture_index], NULL, &portraitBox);
    int portraitTextureID = anim_data[actors[cl->dialogActorIndex].anim[cl->dialogActorFaceIndex]].texture_index;
    int portraitAnimationLen = anim_data[actors[cl->dialogActorIndex].anim[cl->dialogActorFaceIndex]].len;
    SDL_RenderCopy(renderer, textures[portraitTextureID+ ((cl->dialogActorFrame/2) % portraitAnimationLen)], NULL, &portraitBox);
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
