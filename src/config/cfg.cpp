#include "../config.h"

/*
To change this table,
put all var names in vars_list.txt
and run the print_hashes() function.

Paste the result into the
set_var_ptrs() function.

If a collision occurs, increase the
size of the pointer table.
(print_hashes() will alert you)
*/

// hash table for global variables
int* var_ptrs[VAR_PTRS_TABLE_SIZE] = {0};

//
// configurable global variables
//

extern int fullscreen;
extern int window_x;
extern int window_y;
extern int fps_cap;


void set_var_ptrs() {
    var_ptrs[39] = &fullscreen;
    var_ptrs[31] = &window_x;
    var_ptrs[32] = &window_y;
    var_ptrs[0] = &fps_cap;
}

// function macro for skipping a comment line
// (fptr is a FILE*)
#define skip_line(fptr) {\
    char c = fgetc(fptr);\
    while (c != '\n' && !feof(fptr)) {\
        c = fgetc(fptr);\
    }\
}

// print the hashed index nums for vars
// (used to easily change the set_var_ptrs function)
void print_hashes(char* filename) {
    FILE* fp;
    char c;
    int  name_next = 1;
    char name_chars[MAX_VAR_NAME_LEN] = {0};
    int  name_place = 0;
    int var_name_hash = 0;
    int previous_newline = 1;
    int index_num = 0;
    printf("***************************************\n");
    printf("*** Paste these hashed indicies into\n*** set_var_ptrs() in cfg.cpp\n\n");
    fp = fopen(filename, "r");
    if (fp == 0) {
        printf("*** vars_list config file %s not found.\n", filename);
        return;
    }
    while (1) { // read all characters
        c = fgetc(fp);
        if (feof(fp)) // stop at EOF
            break;
        if (c == '#') { // skip comment lines
            skip_line(fp);
            continue;
        }
        if (previous_newline && c == '\n') // skip empty lines
            continue;
        if (c != '\n')
            previous_newline = 0; 
        if (c == '\n') {
            previous_newline = 1;
            name_next = 1; // read a var name
            index_num = var_name_hash % VAR_PTRS_TABLE_SIZE; // hash -> index #
            if (index_num < 0)
                index_num *= -1; // no negative indicies!
            printf("var_ptrs[%d] = &%.*s;\n", index_num, name_place, name_chars);
            // did we already use this var_ptrs slot?
            if (var_ptrs[index_num] != 0)
                printf("\n***** COLLISION DETECTED! *****\n");
            // mark this var_ptrs slot as in use
            var_ptrs[index_num] = (int*)1;
            var_name_hash = 0;
            name_place = 0;
        }
        if (name_next && c != ' ' && c != '\n') {
            //
            // build the hash from the var's name
            //
            var_name_hash += c + var_name_hash*64;
            name_chars[name_place] = c;
            name_place++;
        }
        if (c == ' ') {
            name_next = 0;
            continue;
        }
    }
    fclose(fp);
    // clear out the var_ptrs table,
    // we only put stuff in it to check
    // for collisions
    memset(var_ptrs, 0, VAR_PTRS_TABLE_SIZE*sizeof(int*));
    printf("***************************************\n");
}

// set vars using data.txt and the pointer_table
void vars_from_file(char* filename) {
    FILE* fp;
    char c;
    int name_next = 1;
    int val_place = 0;
    char value_chars[MAX_VAR_VALUE_DIGITS] = {0};
    int val_sum = 0;
    int var_name_hash = 0;
    int previous_newline = 1;
    int index_num = 0;
    fp = fopen(filename, "r");
    if (fp == 0) {
        printf("*** config file %s not found.\n", filename);
        return;
    }
    while (1) { // read all characters
        c = fgetc(fp);
        if (feof(fp)) // stop at EOF
            break;
        if (c == '#') { // skip comment lines
            skip_line(fp);
            continue;
        }
        if (previous_newline && c == '\n') // skip empty lines
            continue;
        if (c != '\n')
            previous_newline = 0; 
        if (c == '\n') {
            previous_newline = 1;
            name_next = 1; // read a var name
            for (int i=0; i<val_place; i++) {
                // set a var value from the text
                val_sum *= 10;
                val_sum += value_chars[i];
            }
            //
            // Var value obtained in val_sum.
            // Now set the index number.
            //
            index_num = var_name_hash % VAR_PTRS_TABLE_SIZE; // hash -> index #
            if (index_num < 0)
                index_num *= -1; // no negative indicies!
            if (var_ptrs[index_num] == 0) {
                printf("*** Var %d is null in the config table!\n", index_num);
                return;
            }
            *var_ptrs[index_num] = val_sum;
            var_name_hash = 0;
            val_sum = 0;
            val_place = 0;
        }
        //
        // get the var's name or value characters
        //
        if (name_next && c != ' ' && c != '\n') {
            // build the hash from the var's name
            var_name_hash += c + var_name_hash*64;
        }
        if (c == ' ') {
            name_next = 0;
            continue;
        }
        if (!name_next) { // read a var value digit
            value_chars[val_place] = c - '0';
            val_place++;
        }
    }
    fclose(fp);
}

/*
int main() {
    print_hashes("vars_list.txt");
    set_var_ptrs();
    vars_from_file("data.txt"); // set vars using data.txt and the pointer_table
    printf("vid_height %d\n", vid_height);
    printf("cl_sidespeed %d\n", cl_sidespeed);
    printf("fov %d\n", fov);
    return 0;
}
*/
