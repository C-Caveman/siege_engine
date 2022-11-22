#include <stdio.h>
#include <string.h>

#define VAR_PTRS_TABLE_SIZE 60
#define MAX_VAR_NAME_LEN 15
#define MAX_VAR_VALUE_DIGITS 20

// use this to update set_var_ptrs()
void print_hashes(char* fname);

// fill the var_ptrs table
void set_var_ptrs();

// update vars from file, using the var_ptrs table
void vars_from_file(char* fname);
