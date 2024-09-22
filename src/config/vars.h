// Variables you can set via config file(s).
//
// See LICENSE file for copyright and license details.
#ifndef VARS
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
#define INT_VARS_LIST(expand) \
    expand(fullscreen) \
    expand(window_x) \
    expand(window_y) \
    expand(fps_cap) \
    expand(vsync)

#define FLOAT_VARS_LIST(expand) \
    expand(example_float_variable) \
    expand(floaty_mc_floatpants) \
    expand(floaty_flops)

#define STRING_VARS_LIST(expand) \
    expand(welcomeMessage) \
    expand(windowName) \
    expand(billy_jean) \
    expand(lima_bean)

#define VAR_LIST(expand) \
    INT_VARS_LIST(expand) \
    FLOAT_VARS_LIST(expand) \
    STRING_VARS_LIST(expand)

// Make the enums. (each enum listing has 'enum_' in front of it)
#define TO_PREFIXED_ENUM(name) enum_##name, 
enum INTEGERS { INT_VARS_LIST(TO_PREFIXED_ENUM) NUM_INT_VARS };
enum { FLOAT_VARS_LIST(TO_PREFIXED_ENUM) NUM_FLOAT_VARS };
enum { STRING_VARS_LIST(TO_PREFIXED_ENUM) NUM_STRING_VARS };
#define TOTAL_VARS (NUM_INT_VARS + NUM_FLOAT_VARS + NUM_STRING_VARS)

// Expose all global vars with extern (definitions done config_loader.c).
#define TO_EXTERN_INT(name) extern int name;
#define TO_EXTERN_FLOAT(name) extern float name;
#define TO_EXTERN_STRING(x) extern char x [STRING_VAR_SIZE];
INT_VARS_LIST(TO_EXTERN_INT)
FLOAT_VARS_LIST(TO_EXTERN_FLOAT)
STRING_VARS_LIST(TO_EXTERN_STRING)

#endif
