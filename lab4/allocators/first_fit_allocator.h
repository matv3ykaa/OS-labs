#ifndef FIRST_FIT_ALLOCATOR_H
#define FIRST_FIT_ALLOCATOR_H

#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <stddef.h>

// Функция для выделения памяти
void* my_malloc(size_t size);

// Функция для освобождения памяти
void my_free(void* ptr);

// Функция для отладки: вывод списка свободных блоков
void print_free_list();

#endif // FIRST_FIT_ALLOCATOR_H
