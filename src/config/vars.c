// Load values from a config file.
// See LICENSE file for copyright and license details.
#include "vars.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define INIT_INT(name) int name = 0; //==============================================// DECLARE VARS //
#define INIT_FLOAT(name) float name = 0;
#define INIT_STRING(name) char name[STRING_VAR_SIZE] = {0};
INT_VARS_LIST(INIT_INT)
FLOAT_VARS_LIST(INIT_FLOAT)
STRING_VARS_LIST(INIT_STRING)

#define TO_ADDRESSES(name) &name, //=================================================// DECLARE VAR ADDRESS ARRAY //
void* VAR_ADDRESSES[] = {
    VAR_LIST(TO_ADDRESSES)
};

#define TO_STRINGS(x) #x, //=========================================================// CONVERT VAR_NAMES TO SEARCHABLE STRING ARRAYS //
char VAR_NAMES[][VAR_NAME_SIZE] = {
    VAR_LIST(TO_STRINGS)
};

//=============================================================================// GET VAR INDEX BY NAME //
int get_var_index(char* name, char names[][VAR_NAME_SIZE], int num_names) { //--- (returns -1 on fail)
    int index = -1;
    for (int i=0; i<num_names; i++) {
        if (strncmp(name, names[i], VAR_NAME_SIZE) == 0) {
            index = i;
            break;
        }
    }
    return index;
}
char c = 0; //============================================// Current line. //
char line[MAX_CONFIG_LINE_LEN];
char var_name[STRING_VAR_SIZE];
char value_string[STRING_VAR_SIZE];
int var_name_len = 0;
int value_string_len = 0;
//========================================================// Digest a line from the config file. //
void read_line(FILE* fp) {
    int len = 0;
    int pos = 0;
    var_name_len = 0;
    value_string_len = 0;
    memset(line, 0, MAX_CONFIG_LINE_LEN*sizeof(char));
    memset(var_name, 0, STRING_VAR_SIZE*sizeof(char));
    memset(value_string, 0, STRING_VAR_SIZE*sizeof(char));
    for (c = fgetc(fp); len<MAX_CONFIG_LINE_LEN  &&  !(feof(fp) || c == '\n'); c = fgetc(fp)) { //---------------------------- Load the line.
        line[len] = c;
        len++;
    }
    if (line[0] == '#') //------------------------------------------ Ignore comments.
        return;
    while (pos<len && isspace(line[pos])) //-------------------------------------------- Trim whitespace.
        pos++;
    while (pos<len && var_name_len < STRING_VAR_SIZE  &&  !(line[pos] == '=' || isspace(line[pos]) || line[pos] == '#')) { //- Get the var_name.
        var_name[var_name_len] = line[pos];
        var_name_len++;
        pos++;
    }
    while (pos<len  &&  !(!isspace(line[pos]) && line[pos] != '=')) //-------------------------------------------- Trim whitespace.
        pos++;
    while (pos<len && value_string_len < STRING_VAR_SIZE  && line[pos] != '#') { //--------- Get the value_string.
        value_string[value_string_len] = line[pos];
        value_string_len++;
        pos++;
    }
}
void applyConfig(char* fname) { //===============================================// Load config variables from file. //
    FILE* fp = fopen(fname, "r");
    if (fp == 0) { printf("File %s not found!\n", fname); exit(-1); }
    int index = -1;
    while (!feof(fp)) { //------------------------------------------------- FOR EACH LINE of "var_name = value_string" 
        read_line(fp);  //----------------------------------------- var_name = value_string
        if (var_name_len == 0) { continue; } // Invalid line.
        index = get_var_index(var_name, VAR_NAMES, TOTAL_VARS); //- Find index of var_name.
        if (index == -1)
            continue;
        else if (index < NUM_INT_VARS)
            *(int*)VAR_ADDRESSES[index] = atoi(value_string); //--------------------- Set int var.
        else if (index < NUM_INT_VARS + NUM_FLOAT_VARS)
            *(float*)VAR_ADDRESSES[index] = atof(value_string); //------------------- Set float var.
        else if (index < NUM_INT_VARS + NUM_FLOAT_VARS + NUM_STRING_VARS)
            memcpy((char**)VAR_ADDRESSES[index], value_string, STRING_VAR_SIZE); //-- Set string var.
    }
    fclose(fp);
}
void print_vars() { //=========================================================// Print the names of each variable. //
    printf("o=====================================================o\n");
    printf("| Summary of Vars:                                    |\n");
    printf("o-----------------------------------------------------o\n");
    printf("| Integers                                            |\n");
    printf("o=====================================================o\n");
    #define LIST_PRINT_INT(x)  printf("| %-32s = %-16d |\n", #x, x); 
    INT_VARS_LIST(LIST_PRINT_INT)
    printf("o-----------------------------------------------------o\n");
    printf("| Floats                                              |\n");
    printf("o=====================================================o\n");
    #define LIST_PRINT_FLOAT(x) printf("| %-32s = %-16f |\n", #x, x); 
    FLOAT_VARS_LIST(LIST_PRINT_FLOAT)
    printf("o-----------------------------------------------------o\n");
    printf("| Strings          (first 16 chars)                   |\n");
    printf("o=====================================================o\n");
    #define LIST_PRINT_STRING(x) printf("| %-32s = %-16.*s |\n", #x, 16, x);
    STRING_VARS_LIST(LIST_PRINT_STRING)
    printf("o=====================================================o\n\n");
}
