/**
 * @name MEM_CHECK
 * @version 2.0
 * @author Matúš Nečas
 * @date 10.2.2020
 * @brief program for detecting leaks and tracking history of the heap memory
 */

#ifndef UNTITLED7_MEM_CHECK_H
#define UNTITLED7_MEM_CHECK_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// Macros for coloring the terminal output:

#define WHITE "\033[0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"

/**
 * Setters:
 * MAX CHAR -> defines maximal size of strings in type mem_info_t
 * MAX_ARR -> defines initial size of memory_register in function check_leaks()
 */
#define MAX_CHAR 100
#define MAX_ARR 100

/**
 * Global variable OUT_FILE which operates with the memory text file.
 */
FILE *OUT_FILE;

/**
 * Function init_mem_file initializes a memory text file through global variable OUT_FILE.
 * Current memory file will be opened in a write mode.
 * @param name_of_file –> filename of text file to be opened
 */
void init_mem_file(char *name_of_file);

/**
 * Function close_mem_file closes a text memory file through global variable OUT_FILE.
 */
void close_mem_file();

/**
 * Type mem_info_t stores a memory data, such as:
 * allocated/freed addresses, size of allocated memory, filename, line and function
 * where memory functions (malloc, calloc, realloc and free) were actually called.
 */
typedef struct
{
    size_t address;
    char file[MAX_CHAR];
    int line;
    int size;
    char function[MAX_CHAR];
} mem_info_t;

/**
 * Function cmpmem compares two types of mem_info_t and compares their address number:
 * @param a -> void pointer to first element
 * @param b -> void pointer to second element
 * @return 1 if address num of a > address num of b
 *         0 if their address nums are equal
 *        -1 if address num of a < address num of b
 */
int cmpmem(const void *a,const void *b);


/**
 * Function record_malloc calls malloc() form <stdlib.h> and makes a record to the text file.
 * @param size -> size of memory to be dynamically allocated (in Bytes)
 * @param file -> filename of project where this function was called
 * @param line -> line of file where this function was called
 * @param func -> function of file where this function was called
 * @return void pointer to the block of allocated memory (as malloc() does)
 */
void * record_malloc(size_t size, const char *file, int line, const char *func);

/**
 * Function record_calloc calls calloc() form <stdlib.h> and makes a record to the text file.
 * @param n -> nuber of elements to be dynamically allocated
 * @param size_of_el -> size of each element (in Bytes)
 * @param file -> filename of project where this function was called
 * @param line -> line of file where this function was called
 * @param func -> function of file where this function was called
 * @return void pointer to the block of allocated memory (as calloc() does)
 */
void * record_calloc(size_t n, size_t size_of_el, const char *file, int line, const char *func);

/**
 * Function record_realloc calls realloc() form <stdlib.h> and makes a record to the text file.
 * @param ptr -> pointer to the block of memory to be dynamically reallocated
 * @param size -> size of memory to be dynamically allocated (in Bytes)
 * @param file -> filename of project where this function was called
 * @param line -> line of file where this function was called
 * @param func -> function of file where this function was called
 * @return void pointer to the reallocated block of memory (as realloc() does)
 */
void * record_realloc(void *ptr, size_t size, const char *file, int line, const char *func);

/**
 * Function record_free calls free() form <stdlib.h> and makes a record to the text file.
 * @param ptr -> pointer to the block of memory to be freed
 * @param file -> filename of project where this function was called
 * @param line -> line of file where this function was called
 * @param func -> function of file where this function was called
 */
void record_free(void *ptr, const char *file, int line, const char *func);

/**
 * Function check_leaks loads memory records from text file and analyzes potential memory leaks.
 * If number of allocations/reallocations doesn't match with number of of freeds for certain adress,
 * function detects a memory leak. All messages are shown in stdout.
 * @param mem_file_name -> filename of memory file to be reading
 */
void check_leaks();

/**
 * Function print_mem_info loads memory records from text file and prints all the history of heap memory.
 * All messages are shown in stdout.
 * @param mem_file_name -> filename of memory file to be reading
 */
void print_mem_info();

#endif //UNTITLED7_MEM_CHECK_H


