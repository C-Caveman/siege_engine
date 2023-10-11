// draw to the game window 
#include "graphics.h"
#include <unistd.h>
#include <iostream>
#include <stdlib.h> // snprintf used for tex fname generation
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define MAX_PATH_LEN 256
char path[MAX_PATH_LEN]; // path to the executable
SDL_Texture* textures[MAX_TEXTURES];
char animation_names[NUM_ANIM*ANIM_NAME_LEN];
// Details for each animation.
struct anim_info anim_data[NUM_ANIM];
int animation_lengths[NUM_ANIM];
int animation_index[NUM_ANIM];
float min_frame_time = 1000/60; // second / frames
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Rect background;
int fullscreen, window_x, window_y, fps_cap;
int running;
float last_frame_end, frame_time, frame_count, last_sec, fps;
float dt = 0;
float view_x = 0;
float view_y = 0;
TTF_Font* font = 0;

extern volatile float mouse_angle;

void skip_comment(FILE* fp) {
    char c = fgetc(fp);
    while (c != '\n' && !feof(fp))
        c = fgetc(fp);
}

// read filenames from animation.h's enum
int load_animation_names() {
    FILE* fp;
    char c;
    char name[ANIM_NAME_LEN];
    int num_names = 0;
    int name_char_index = 0;
    int chars_left = ANIM_NAME_LEN;
    fp = fopen("src/graphics/animations.h", "r");
    if (fp == 0) {
        printf("Failed to find textures/animation_names.txt\n");
        return -1; // don't seg fault by tring to close fp here
    }
    c = '?';
    // skip to the animation enum
    while (c != '{') {
        c = fgetc(fp);
    }
    // begin reading the names from the text file
    while (1) {
        c = fgetc(fp);
        if (feof(fp) || c == '}') // end of file
            break;
        else if (c == '/') // comment
            skip_comment(fp);
        else if (c == ' ' || c == '\n' || c == '\t') // skip whitespace
            continue;
        else if (c == ',') { // end of a name
            /*printf("got name: %.*s, idx=%d\n", ANIM_NAME_LEN, 
                                               animation_names+name_char_index-(ANIM_NAME_LEN-chars_left), 
                                               name_char_index);*/
            // keep names starting at multiples of ANIM_NAME_LEN
            // in the names array
            name_char_index += chars_left;
            chars_left = ANIM_NAME_LEN;
            num_names++;
        }
        else {
            animation_names[name_char_index] = c;
            name_char_index++;
            chars_left--;
            // notify the user when their animation name is too long to use
            if (chars_left < 0) {
                printf("Animation name %.*s was longer than max len: %d\n", 
                       ANIM_NAME_LEN, 
                       animation_names+name_char_index-(ANIM_NAME_LEN-chars_left), 
                       ANIM_NAME_LEN
                );
                exit(-1);
            }
        }
    }
    fclose(fp);
    return num_names;
}

// get all the textures for each animation loaded into the game
void load_animations() {
    SDL_Surface* cur_surf;
    char tex_filename[64];
    char fname_folder_name[] = "graphics/animations/";
    char fname_bmp[] = ".bmp";
    char frame_num_string[4];
    char* anim_name = 0;
    int tex_name_end = 0;
    int anim_name_len = 0;
    int num_frames = 0;
    int total_textures = 0;
    for (int i=0; i<NUM_ANIM; i++) { // load each animation
        num_frames = 0;
        animation_index[i] = total_textures;
        anim_name = animation_names+ANIM_NAME_LEN*i;
        // find the len of anim_name
        for (int j=0; j<ANIM_NAME_LEN+1; j++) {
            anim_name_len = j;
            if (anim_name[j] == 0) {
                break;
            }
        }
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
        animation_lengths[i] = num_frames;
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
            sizeof(font_relative_path));
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
        cout << "SDL init failed. " << SDL_GetError() << "\n";
    // Set the logical resolution:
    //SDL_RenderSetLogicalSize(renderer, 1920, 1080);
    // make it fullscreen if config file says so
    if (fullscreen) {
        // Find the screen's resolution:
        SDL_Rect bounds;
        int displayIndex = 0;
        if (SDL_GetDisplayBounds(displayIndex, &bounds) != 0)
            printf("*** init_graphics error: %s\n", SDL_GetError());
        // Set the resolution to that of the screen:
        window_x = bounds.w;
        window_y = bounds.h;
        if (SDL_CreateWindowAndRenderer(window_x, window_y, 0, &window, &renderer) != 0)
            cout << "SDL window/renderer init failed" << SDL_GetError() << "\n";
        printf("\nResolution:\n        %dx%d\n", bounds.w, bounds.h);
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
    else {
        // Make a window with the configured size:
        if (SDL_CreateWindowAndRenderer(window_x, window_y, 0, &window, &renderer) != 0)
            cout << "SDL window/renderer init failed" << SDL_GetError() << "\n";
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
    int anim_count = 0;
    
    // get the names of the animations
    anim_count = load_animation_names();
    if (anim_count != NUM_ANIM+1)
        printf("Found %d animation names, expected %d... \n", anim_count, NUM_ANIM+1);
    
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
        //cout << "One second has passed, fps = " << fps << "\n";
        //cout << "Delta time was: " << dt << "\n";
    }
}

void draw_background() {
    // draw a black background
    //SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    //SDL_RenderFillRect(renderer, &background);
    
    // draw background textures
    background.x = (-1)*(int)ceil((int)view_x % window_x);
    background.y = (-1)*(int)ceil((int)view_y % window_x);
    float center_x = background.x;
    float center_y = background.y;
    
    // central background texture tile (there are nine in total)
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background);
    
    // the other background tiles
    background.x = center_x + window_x;
    background.y = center_y + window_x;
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background); // corner
    background.x = center_x - window_x;
    background.y = center_y + window_x;
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background); // corner
    background.x = center_x + window_x;
    background.y = center_y - window_x;
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background); // corner
    background.x = center_x - window_x;
    background.y = center_y - window_x;
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background); // corner
    background.x = center_x + window_x;
    background.y = center_y;
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background); // right
    background.x = center_x - window_x;
    background.y = center_y;
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background); // left
    background.x = center_x;
    background.y = center_y + window_x;
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background); // up
    background.x = center_x;
    background.y = center_y - window_x;
    SDL_RenderCopy(renderer, textures[BKGRND_TEX], NULL, &background); // down
}

void draw_ent(ent* e) {
    //SDL_SetRenderDrawColor(renderer, e->get_redness(), 0, 0, 255);
    //SDL_RenderFillRect(renderer, e->get_rect());
    SDL_Rect ent_render_pos;
    ent_render_pos.x = e->get_pos().get_x();
    ent_render_pos.y = e->get_pos().get_y();
    ent_render_pos.w = ent_render_pos.h = RSIZE;
    ent_render_pos.x -= view_x - window_x/2 + 64;
    ent_render_pos.y -= view_y - window_y/2 + 64;
    //TODO incorporate animations
    // 10 fps animation
    int r = 4; // animation rate TODO give each animation it's own rate value
    int anim_frame = (int)(last_frame_end/128/r) % animation_lengths[e->get_anim()];
    //SDL_RenderCopy(renderer, textures[animation_index[e->get_anim()]+anim_frame], NULL, &ent_render_pos);
    
    SDL_Texture* current_tex = textures[animation_index[e->get_anim()]+anim_frame];
    //
    // Set rotation based on entity type.
    //
    int rotation = 0;
    switch(e->get_typ()) {
        case ENT_GUN:
            rotation = mouse_angle;
            break;
        default:
            break;
    }
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (/*e->get_vel().get_x() < 0*/ e->get_typ() == ENT_PLAYER && mouse_x < 0)
        flip = SDL_FLIP_HORIZONTAL; //TODO set this using ent flags
    SDL_RenderCopyEx(renderer, 
                     current_tex, 
                     NULL, 
                     &ent_render_pos, 
                     rotation, 
                     NULL, 
                     flip);
}

void draw_ent_sprites(segment* e) { // TODO use the animation flags / update anim loader TODO ;;
    int num_sprites = e->head.num_sprites;
    vec2f p;
    uint32_t anim;
    float rotation;
    anim_info animation;
    SDL_Rect ent_render_pos;
    ent_render_pos.w = ent_render_pos.h = RSIZE;
    vec2f ent_origin = e[pos].pos.pos;
    //segment
    if (DEBUG_GRAPHICS)
        printf("Num sprites in %s entity: %d\n", get_type_name(e->head.type), num_sprites);
    for (int i=0; i<num_sprites; i++) {
        // Get sprite data:
        p =        e[basic_ent_size + i*sprite_size + sprite_position_segment].pos.pos + ent_origin;
        anim =     e[basic_ent_size + i*sprite_size + sprite_animation_segment].anim.anim;
        rotation = e[basic_ent_size + i*sprite_size + sprite_animation_segment].anim.rotation;
        //flags = TODO use the animation flags TODO
        // Adjust for screen position:
        ent_render_pos.x = p.x - view_x;
        ent_render_pos.y = p.y - view_y;
        // Render the sprite:
        SDL_RenderCopyEx(renderer, 
                         textures[animation_index[anim]], 
                         NULL, 
                         &ent_render_pos, 
                         rotation, 
                         NULL, 
                         SDL_FLIP_NONE);
    }
}
void draw_all_ents(segment* array, int array_len) { // ;;
    int i = 0;
    i = get_first_ent(array, array_len);
    while (i != -1) {
        if (array[i].head.header_byte != HEADER_BYTE) {
            if (DEBUG_GRAPHICS)
                printf("*** Invalid index given by get_next_ent() in draw_all_ents()\n");
            break;
        }
        draw_ent_sprites(&array[i]);
        i = get_next_ent(i, array, array_len);
    }
}

void draw_ents(ent* ent_array, float num_ents) {
    //draw_background();
    for (int i=0; i<num_ents; i++) {
        draw_ent(&ent_array[i]);
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

void draw_tile(struct tile (*tiles)[CHUNK_WIDTH], int x, int y, vec2f scaled_view_pos) {
    if (x < 0 || x > CHUNK_WIDTH-1 || y < 0 || y > CHUNK_WIDTH-1)
        return;
    struct tile t = tiles[y][x];
    int cur_anim = 0;
    int cur_frame = 0;
    SDL_Rect render_pos;
    render_pos.w = render_pos.h = RSIZE;
    render_pos.x = scaled_view_pos.x + x * RSIZE;
    render_pos.y = scaled_view_pos.y + y * RSIZE;
    SDL_Rect tile_pos = render_pos;
    // Draw floor:
    cur_anim = animation_index[t.floor_anim];
    SDL_RenderCopy(
        renderer, 
        textures[cur_anim+cur_frame], 
        NULL, 
        &render_pos
    );
    // Draw wall:
    cur_anim = animation_index[t.wall_side_anim];
    float offset_x = ((view_x+window_x/2-RSIZE) - view_x - (tile_pos.x))/(window_x)*10.0f;
    float offset_y = ((view_y+window_y/2-RSIZE) - view_y - (tile_pos.y))/(window_y)*10.0f;
    render_pos.x = (view_x+window_x/2) - view_x;
    render_pos.y = (view_y+window_y/2) - view_y;
    for (int i=0; i<t.wall_height; i++) {
        render_pos = tile_pos;
        render_pos.x = render_pos.x - offset_x*(i);
        render_pos.y = render_pos.y - offset_y*(i);
        SDL_RenderCopy(
            renderer,
            textures[cur_anim+cur_frame],
            NULL,
            &render_pos
        );
    }
    if (t.wall_height > 0) {
        cur_anim = animation_index[t.wall_top_anim];
        render_pos = tile_pos;
        render_pos.x = render_pos.x - offset_x * t.wall_height;
        render_pos.y = render_pos.y - offset_y * t.wall_height;
        SDL_RenderCopy(
            renderer,
            textures[cur_anim],
            NULL,
            &render_pos
        );
    }
}

void draw_chunk(chunk* chunk, vec2f camera_pos) {
    // Convert the camera_pos to a chunk_pos:
    vec2i chunk_pos = vec2i{(int)camera_pos.x/RSIZE, (int)camera_pos.y/RSIZE};
    // Get a pointer to the chunk's tile array.
    struct tile (*tiles)[CHUNK_WIDTH] = chunk->get_tiles();
    SDL_Rect tile_pos;
    SDL_Rect render_pos;
    SDL_Rect offset;
    render_pos.x = 0;
    render_pos.y = 0;
    render_pos.w = render_pos.h = RSIZE;
    float vpos_x = 0 - (view_x - window_x/2 + 64);
    float vpos_y = 0 - (view_y - window_y/2 + 64);
    int cur_anim = 0;
    int cur_frame = 0;
    
    //
    // Draw from the outer edges, work inwards.
    //
    // Top:
    for (int y=0; y<chunk_pos.y; y++) {
        // Left:
        for (int x=0; x<chunk_pos.x; x++) {
            draw_tile(tiles, x, y, vec2f {vpos_x, vpos_y});
        }
        // Right:
        for (int x=CHUNK_WIDTH-1; x>chunk_pos.x; x--) {
            draw_tile(tiles, x, y, vec2f {vpos_x, vpos_y});
        }
    }
    // Bottom:
    for (int y=CHUNK_WIDTH-1; y>chunk_pos.y; y--) {
        // Left:
        for (int x=0; x<chunk_pos.x; x++) {
            draw_tile(tiles, x, y, vec2f {vpos_x, vpos_y});
        }
        // Right:
        for (int x=CHUNK_WIDTH-1; x>chunk_pos.x; x--) {
            draw_tile(tiles, x, y, vec2f {vpos_x, vpos_y});
        }
    }
    // Center left:
    for (int x=0; x<chunk_pos.x; x++) {
        draw_tile(tiles, x, chunk_pos.y, vec2f {vpos_x, vpos_y});
    }
    // Center right:
    for (int x=CHUNK_WIDTH-1; x>chunk_pos.x; x--) {
        draw_tile(tiles, x, chunk_pos.y, vec2f {vpos_x, vpos_y});
    }
    // Center top:
    for (int y=0; y<chunk_pos.y; y++) {
        draw_tile(tiles, chunk_pos.x, y, vec2f {vpos_x, vpos_y});
    }
    // Center bottom:
    for (int y=CHUNK_WIDTH-1; y>chunk_pos.y; y--) {
        draw_tile(tiles, chunk_pos.x, y, vec2f {vpos_x, vpos_y});
    }
    // Center:
    draw_tile(tiles, chunk_pos.x, chunk_pos.y, vec2f {vpos_x, vpos_y});
}


void cleanup_graphics() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    for (int i=0; i<MAX_TEXTURES; i++) {
        if (textures[i] != 0)
            ;//SDL_DestroyTexture(textures[i]);
    }
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}
