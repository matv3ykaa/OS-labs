#ifndef BEST_FIT_ALLOCATOR_H
#define BEST_FIT_ALLOCATOR_H

#define _GNU_SOURCE
#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>

// Функция для выделения памяти
void* my_malloc(size_t size);

// Функция для освобождения памяти
void my_free(void* ptr);

// Функция для отладки: вывод списка свободных блоков
void print_free_list();

#endif // BEST_FIT_ALLOCATOR_H
