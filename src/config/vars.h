// Variables you can set via config file(s).
//
// See LICENSE file for copyright and license details.
#ifndef VARS
#define VARS
#include "../defs.h"

#define VAR_NAME_SIZE 64
#define STRING_VAR_SIZE 256
#define MAX_CONFIG_LINE_LEN 512
// Print detailed info about each line of the config file read.
#define DEBUG 0

// Set variables from a file.
void applyConfig(char* fname);
// Print all the variables and their values.
void print_vars();

// List of variables!
#define INT_VARS_LIST(f) \
    f(fullscreen) \
    f(window_x) \
    f(window_y) \
    f(fps_cap) \
    f(vsync) \

#define FLOAT_VARS_LIST(f) \
    f(aimSpeedA) \
    f(aimSpeedB) \
    f(floaty_flops)

#define STRING_VARS_LIST(f) \
    f(welcomeMessage) \
    f(windowName) \
    f(billy_jean) \
    f(lima_bean)
    
#define INPUTS_LIST(f) \
    f(inputKeyUnbound) \
    f(inputQuit) \
    f(inputFullscreen) \
    f(inputMoveUp) \
    f(inputMoveDown) \
    f(inputMoveLeft) \
    f(inputMoveRight) \
    f(inputSprint) \
    f(inputSneak) \
    f(inputAttack) \
    f(inputBuild) \
    f(inputAimLeft) \
    f(inputAimRight) \
    f(inputAimReverse) \
    f(inputAimSpeed) \
    f(inputSpawnZombie) \
    f(inputInteract) \
    f(inputSkipDialog) \
    f(inputDash) \
    f(inputPause) \
    
#define VAR_LIST(f) \
    INT_VARS_LIST(f) \
    FLOAT_VARS_LIST(f) \
    STRING_VARS_LIST(f) \
    
// Make the enums. (each enum listing has 'enum_' in front of it)
#define TO_PREFIXED_ENUM(name) enum_##name, 
enum { INT_VARS_LIST(TO_PREFIXED_ENUM) NUM_INT_VARS };
enum { FLOAT_VARS_LIST(TO_PREFIXED_ENUM) NUM_FLOAT_VARS };
enum { STRING_VARS_LIST(TO_PREFIXED_ENUM) NUM_STRING_VARS };
enum { INPUTS_LIST(TO_PREFIXED_ENUM) NUM_INPUTS };
#define TO_ALTERNATE_PREFIXED_ENUM(name) allVarsEnum_##name, 
enum allVarsEnumeration {
    INT_VARS_LIST(TO_ALTERNATE_PREFIXED_ENUM)
    FLOAT_VARS_LIST(TO_ALTERNATE_PREFIXED_ENUM)
    STRING_VARS_LIST(TO_ALTERNATE_PREFIXED_ENUM)
    INPUTS_LIST(TO_ALTERNATE_PREFIXED_ENUM)
    TOTAL_VARS
};

// Expose all global vars with extern (definitions done config_loader.c).
#define TO_EXTERN_INT(name) extern int name;
#define TO_EXTERN_FLOAT(name) extern float name;
#define TO_EXTERN_STRING(x) extern char x [STRING_VAR_SIZE];
INT_VARS_LIST(TO_EXTERN_INT)
FLOAT_VARS_LIST(TO_EXTERN_FLOAT)
STRING_VARS_LIST(TO_EXTERN_STRING)

#endif
