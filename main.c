#include <stdio.h>
#include <stdlib.h>
#include "mem_check.h"


#define malloc(x) record_malloc(x, __FILE__, __LINE__, __FUNCTION__)
#define free(x) record_free(x, __FILE__, __LINE__, __FUNCTION__)
#define calloc(n, x) record_calloc(n, x, __FILE__, __LINE__, __FUNCTION__)
#define realloc(p, x) record_realloc(p, x,__FILE__, __LINE__, __FUNCTION__)

int main()
{
    init_mem_file("memory.txt");



    print_mem_info();
    check_leaks();
    close_mem_file();


    return 0;
}