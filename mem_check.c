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

    if (p == NULL)
    {
        printf(RED"Error, memory allocation failed! \n"WHITE);
        exit(EXIT_FAILURE);
    }

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

    if (p == NULL)
    {
        printf(RED"Error, memory allocation failed! \n"WHITE);
        exit(EXIT_FAILURE);
    }

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

    if (new_ptr == NULL)
    {
        printf(RED"Error, memory allocation failed! \n"WHITE);
        exit(EXIT_FAILURE);
    }

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
    // check whether the memory file is opened and set the pointer at the beginning of the stream

    if ( OUT_FILE == NULL || fseek(OUT_FILE, 0, SEEK_SET) )
    {
        perror("Error while reading memory file in check_leaks:");
        exit(EXIT_FAILURE);
    }

    size_t total_leaks = 0;
    size_t total_memory_leaked = 0;
    size_t leak_number = 0;
    size_t count;
    size_t capacity = MAX_ARR;
    size_t mem_len = 0;

    // allocate an array to store memory records from memory file

    mem_info_t  * mem_registry = (mem_info_t*)malloc(capacity * sizeof(mem_info_t));

    // if the capacity is no sufficient for at least 1 element, increase the capacity by 10 and reallocate the array

    if(capacity <= 0)
    {
        capacity = 10;
        mem_registry = (mem_info_t *)realloc(mem_registry, capacity * sizeof(mem_info_t));
    }

    // load memory data from memory file and store them

    while(fscanf(OUT_FILE,"%li", &mem_registry[mem_len].address) == 1 )
    {

       // if the capacity is too low, double the capacity and reallocate the array

        if(capacity - mem_len - 1 <= 0)
        {
            capacity *= 2;
            mem_registry = (mem_info_t*)realloc(mem_registry,capacity * sizeof(mem_info_t));
        }

        fscanf(OUT_FILE, "%s", mem_registry[mem_len].file);
        fscanf(OUT_FILE, "%d", &mem_registry[mem_len].line);
        fscanf(OUT_FILE, "%s", mem_registry[mem_len].function);
        fscanf(OUT_FILE, "%d", &mem_registry[mem_len].size);

        ++mem_len;
    }

    printf("\nLEAK DETECTOR ANALYSIS:\n");

    // sort the records by addresses so, all the records of each address will be continuous

    mergesort(mem_registry, mem_len, sizeof(mem_info_t), cmpmem);

    size_t J; // index of unique address

    // iterate the array and analyze
    for (size_t I = 1; I <= mem_len; ++I)
    {
        J = I - 1;
        count = 1; // number of records for each address


        // skip all the duplicates and count number of records for each address

        while (I < mem_len && cmpmem(&mem_registry[I], &mem_registry[J]) == 0)
        {
            count++;
            I++;
        }

        // if the number of records is odd, a leak occurred

        if(count & 1u)
        {

           J += count - 1; // skip to the last record for that address
           leak_number++;
            printf (RED"LEAK %2lu: %s, %d, %s, %p[%d]\n"WHITE,leak_number, mem_registry[J].file, mem_registry[J].line,
                    mem_registry[J].function, (size_t*)mem_registry[J].address, mem_registry[J].size);
            total_leaks++;
            total_memory_leaked += mem_registry[J].size;
        }
    }

    if(total_leaks > 0)
    {
        printf("\nMEMORY FAILURE:\n");
        printf(RED "Total leaks detected: " WHITE "%lu\n", total_leaks);
        printf(RED "Total memory leaked: " WHITE "%lu B\n", total_memory_leaked);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf(GREEN"MEMORY OK: Every allocated memory has been successfully freed :)"WHITE);
    }

free(mem_registry);

}



void print_mem_info()
{
    // check whether the memory file is opened and set the pointer at the beginning of the stream

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
            printf (BLUE "%2d.\tFREED        FILE:" WHITE" %50s,\t " BLUE "LINE: "WHITE "%i,\t "BLUE"FUNC: "
                    WHITE"%20s,\t"BLUE"ADDR:      " WHITE "%p\n",i,
                    tmp.file, tmp.line, tmp.function, (size_t*) tmp.address);

            number_of_freeds += 1;
        }
        else if (tmp.size == -1)
        {
                printf (YELLOW "%2d.\tREALLOCATED  FILE:" WHITE" %50s,\t " YELLOW "LINE: "WHITE "%i,\t "
                        YELLOW"FUNC: "WHITE"%20s,\t"YELLOW"ADDR:      " WHITE "%p\n",
                        i, tmp.file, tmp.line, tmp.function, (size_t*) tmp.address);

                number_of_re_allocations += 1;
        }
        else
        {
            printf (RED "%2d.\tALLOCATED    FILE:" WHITE" %50s,\t " RED "LINE: "WHITE "%i,\t "RED"FUNC: "
                    WHITE"%20s,\t" RED"ADDR[MEM]: " WHITE "%p[%d]\n",i,
                    tmp.file, tmp.line, tmp.function, (size_t*) tmp.address, tmp.size);

            number_of_allocations += 1;
        }
    }

    printf(YELLOW"\nTOTAL MEMORY OPERATIONS %lu\n", number_of_allocations + number_of_freeds + number_of_re_allocations);
    printf(YELLOW"NUMBER OF ALLOCATIONS: %lu\n" WHITE, number_of_allocations);
    printf(YELLOW"NUMBER OF REALLOCATIONS: %lu\n" WHITE, number_of_re_allocations);
    printf(YELLOW"NUMBER OF FREEDS: %lu\n" WHITE, number_of_freeds);



}

void init_mem_file(char *name_of_file)
{
    if(OUT_FILE != NULL)
    {
        printf("Error with initializing memory file: The memory file hasn't been closed before reinitialization\n");
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
       printf(RED"Error with closing memory file: This file might not been initialized!\n"WHITE);
       exit(EXIT_FAILURE);
    }

    fclose(OUT_FILE);
    OUT_FILE = NULL;
}



