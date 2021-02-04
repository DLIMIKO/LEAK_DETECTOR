//
// Created by Matúš Nečas on 01/02/2021.
//

#include "mem_check.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>




int cmpmem(const void *a,const void *b)
{
    mem_info_t *a_ptr = (mem_info_t*)a;
    mem_info_t *b_ptr = (mem_info_t*)b;

    return (a_ptr->address > b_ptr->address) - (b_ptr->address > a_ptr->address);
}


void *record_malloc(size_t size, const char *file, int line, const char *func)
{

    if(OUT_FILE == NULL)
    {
        printf(RED "An error has occured in record_malloc: Output memory file hasn't been initialized! "
                   "Terminating program... \n" WHITE);
        exit(EXIT_FAILURE);
    }

    void *p = malloc(size);

    fprintf(OUT_FILE, "%p %s %d %s %lld\n", p, file, line, func, (int64_t)size);

    return p;
}


void *record_calloc(size_t n, size_t size_of_el, const char *file, int line, const char *func)
{

    if(OUT_FILE == NULL)
    {
        printf(RED "An error has occured in record_calloc: Output memory file hasn't been initialized! "
               "Terminating program... \n" WHITE);
        exit(EXIT_FAILURE);
    }

    void *p = calloc(n, size_of_el);

    fprintf(OUT_FILE, "%p %s %d %s %lld\n", p, file, line, func, (int64_t)n * size_of_el);

    return p;
}

void *record_realloc(void *ptr, size_t size, const char *file, int line, const char *func)
{

    if(OUT_FILE == NULL)
    {
        printf(RED "An error has occured in record_realloc: Output memory file hasn't been initialized! "
               "Terminating program... \n" WHITE);
        exit(EXIT_FAILURE);
    }

    fprintf(OUT_FILE, "%p %s %d %s %lld\n", ptr, file, line, func, -1ll);

    void *new_ptr = realloc(ptr, size);

    fprintf(OUT_FILE, "%p %s %d %s %lld\n", new_ptr, file, line, func, (int64_t)size);

    return new_ptr;
}

void record_free(void *ptr, const char *file, int line, const char *func)
{

    if(OUT_FILE == NULL)
    {
        printf(RED "An error has occured in record_free: Output memory file hasn't been initialized! "
               "Terminating program... \n" WHITE);
        exit(EXIT_FAILURE);
    }

    fprintf(OUT_FILE, "%p %s %d %s %lld\n", ptr, file, line, func, 0ll);

    free(ptr);
}

void check_leaks()
{


    if ( OUT_FILE == NULL || fseek(OUT_FILE, 0, SEEK_SET))
    {
        perror("Error while reading memory file in check_leaks:");
        exit(EXIT_FAILURE);
    }

    size_t total_leaks = 0;
    size_t total_memory_leaked = 0;
    size_t count = 0;
    size_t count1;
    size_t  capacity = MAX_ARR;

    mem_info_t  * mem_registry = (mem_info_t*)malloc(capacity * sizeof(mem_info_t));
    printf(" Alloc %p %lu\n", mem_registry, capacity * sizeof(mem_info_t));


    int i = 0;

    if(capacity <= 0)
    {
        capacity = 10;
        mem_registry = (mem_info_t *) realloc(mem_registry, capacity * sizeof(mem_info_t));
    }


    while(fscanf(OUT_FILE,"%li", &mem_registry[i].address) == 1 )
    {

        printf("%lu\n", capacity - i - 1);
        if(capacity  - i - 1  <= 0)
        {
            capacity *= 2;

            mem_registry = (mem_info_t*)realloc(mem_registry,capacity * sizeof(mem_info_t));
        }

        fscanf(OUT_FILE, "%s", mem_registry[i].file);
        fscanf(OUT_FILE, "%d", &mem_registry[i].line);
        fscanf(OUT_FILE, "%s", mem_registry[i].function);
        fscanf(OUT_FILE, "%d", &mem_registry[i].size);

       // printf("%lu\n", mem_registry[i].address);

        ++i;
    }
    size_t mem_len = i;


    printf("\nLEAK DETECTOR ANALYSIS:\n");
    mergesort(mem_registry, mem_len, sizeof(mem_info_t), cmpmem);

    size_t J;

    for (size_t I = 1; I <= mem_len; ++I)
    {
        J = I - 1;
        count1 = 1;


        while (I < mem_len && cmpmem(&mem_registry[I], &mem_registry[J]) == 0)
        {
            count1++;
            I++;
        }

        if(count1 & 1u)
        {

           J += count1 - 1;
           count++;
            printf (RED"LEAK %2lu: %s, %i, %s, %p[%d]\n"WHITE,count, mem_registry[J].file, mem_registry[J].line,
                    mem_registry[J].function, (size_t*)mem_registry[J].address, mem_registry[J].size);
            total_leaks++;
            total_memory_leaked += mem_registry[J].size;
        }
    }



    if(total_leaks > 0)
    {
        printf("\nMEMORY FAILURE:\n");
        printf(RED "Total leaks detected:" WHITE "%lu\n", total_leaks);
        printf(RED "Total memory leaked:" WHITE "%lu Bytes\n", total_memory_leaked);
       // exit(EXIT_FAILURE);
    }
    else
    {
        printf("\n\033[0;32mMEMORY OK: Every allocated memory has been successfully freed :)\033[0m\n");
    }

free(mem_registry);

}



void print_mem_info()
{


    if ( OUT_FILE == NULL || fseek(OUT_FILE, 0, SEEK_SET))
    {
        perror("Error while reading memory file in check_leaks:");
        exit(EXIT_FAILURE);
    }


    size_t number_of_re_allocations = 0;
    size_t number_of_allocations = 0;
    size_t number_of_freeds = 0;

    mem_info_t tmp;
    int i = 0;

    printf("\nHISTORY OF HEAP MEMORY\n");
    while( fscanf(OUT_FILE,"%li", &tmp.address) == 1)
    {
        fscanf(OUT_FILE, "%s", tmp.file);
        fscanf(OUT_FILE, "%d", &tmp.line);
        fscanf(OUT_FILE, "%s", tmp.function);
        fscanf(OUT_FILE, "%d", &tmp.size);



       i++;

        if(tmp.size == 0)
        {
            printf (BLUE "%2d.\tFREED        FILE:" WHITE" %s,\t " BLUE "LINE: "WHITE "%i,\t "BLUE"FUNC:" WHITE"%s,\t"
                    BLUE"ADDR[MEM]" WHITE ":%p[%d]\n",i,
                    tmp.file, tmp.line, tmp.function,
                    (size_t*) tmp.address, tmp.size);
                    number_of_freeds += 1;
        }
        else if (tmp.size == -1)
        {

            printf (YELLOW "%2d.\tREALLOCATED  FILE:" WHITE" %s,\t " YELLOW "LINE: "WHITE "%i,\t "YELLOW"FUNC:" WHITE"%s,\t"
                    YELLOW"ADDR[MEM]" WHITE ":%p[%d]\n",i,
                    tmp.file, tmp.line, tmp.function,
                    (size_t*) tmp.address, tmp.size);
                     number_of_re_allocations += 1;
                     number_of_allocations -= 1;

        }
        else
        {
            printf (RED "%2d.\tALLOCATED    FILE:" WHITE" %s,\t " RED "LINE: "WHITE "%i,\t "RED"FUNC:" WHITE"%s,\t"
                    RED"ADDR[MEM]" WHITE ":%p[%d]\n",i,
                    tmp.file, tmp.line, tmp.function,
                    (size_t*) tmp.address, tmp.size);
                    number_of_allocations += 1;
        }
    }

    printf(YELLOW"\nNUMBER OF ALLOCATIONS: %lu\n" WHITE, number_of_allocations);
    printf(YELLOW "NUMBER OF REALLOCATIONS: %lu\n" WHITE, number_of_re_allocations);
    printf(YELLOW "NUMBER OF FREEDS: %lu\n" WHITE, number_of_freeds);



}

void init_mem_file(char *name_of_file)
{
    if(OUT_FILE != NULL)
    {
        perror("Error with initializing memory file: The file hasn't been closed before reinitialization\n");
    }


    OUT_FILE = fopen(name_of_file, "w+");

    if(OUT_FILE == NULL)
    {
        perror("Error with initializing memory file:");
        exit(EXIT_FAILURE);
    }
}

void close_mem_file()
{
    if(OUT_FILE == NULL)
    {
       printf(RED"Error, can't close current file\n");
       exit(EXIT_FAILURE);
    }

    fclose(OUT_FILE);
    OUT_FILE = NULL;
}

