#include <stdio.h>
#include <stdlib.h>
#include "data_structures.h"
#include "mem_check.h"



#define malloc(x) record_malloc(x, __FILE__, __LINE__, __FUNCTION__)
#define free(x) record_free(x, __FILE__, __LINE__, __FUNCTION__)
#define calloc(n, x) record_calloc(n, x, __FILE__, __LINE__, __FUNCTION__)
#define realloc(p, x) record_realloc(p, x,__FILE__, __LINE__, __FUNCTION__)


int main()
{

    init_mem_file("memory.txt");
    close_mem_file();



    int *array = (int*) malloc(10 * sizeof(int));
    int *array2 = (int*) malloc(10 * sizeof(int));
    int *array3 = (int*) malloc(10 * sizeof(int));

    array3 = (int*) realloc(array3, 100 * sizeof(int));


    free(array);
    free(array2);
    free(array3);




    check_leaks();
    print_mem_info();



    return 0;
}