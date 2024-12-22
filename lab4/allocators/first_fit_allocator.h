#ifndef FIRST_FIT_ALLOCATOR_H
#define FIRST_FIT_ALLOCATOR_H

#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <stddef.h>

void* my_malloc(size_t size);

void my_free(void* ptr);

void print_free_list();

#endif // FIRST_FIT_ALLOCATOR_H
