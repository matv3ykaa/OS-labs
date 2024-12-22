#ifndef BEST_FIT_ALLOCATOR_H
#define BEST_FIT_ALLOCATOR_H

#define _GNU_SOURCE
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>

void* my_malloc(size_t size);

void my_free(void* ptr);

void print_free_list();

#endif // BEST_FIT_ALLOCATOR_H
