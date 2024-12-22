#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

// Определяем типы функций из динамических библиотек
typedef void* (*malloc_func)(size_t);
typedef void (*free_func)(void*);
typedef void (*print_free_list_func)();

#define MEMORY_POOL_SIZE 1024

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <library_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Загружаем динамическую библиотеку
    void* library = dlopen(argv[1], RTLD_LAZY);
    if (!library) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // Загружаем символы функций
    malloc_func my_malloc = (malloc_func)dlsym(library, "my_malloc");
    free_func my_free = (free_func)dlsym(library, "my_free");
    print_free_list_func print_free_list = (print_free_list_func)dlsym(library, "print_free_list");

    if (!my_malloc || !my_free || !print_free_list) {
        fprintf(stderr, "Error loading symbols: %s\n", dlerror());
        dlclose(library);
        exit(EXIT_FAILURE);
    }

    // Тестируем аллокатор
    printf("[Test] Initial free list:\n");
    print_free_list();

    printf("\n[Test] Allocating memory:\n");
    void* ptr1 = my_malloc(256);
    printf("Allocated 256 bytes at %p\n", ptr1);
    print_free_list();

    void* ptr2 = my_malloc(128);
    printf("Allocated 128 bytes at %p\n", ptr2);
    print_free_list();

    printf("\n[Test] Freeing memory:\n");
    my_free(ptr1);
    printf("Freed memory at %p\n", ptr1);
    print_free_list();

    my_free(ptr2);
    printf("Freed memory at %p\n", ptr2);
    print_free_list();

    // Закрываем библиотеку
    dlclose(library);
    return 0;
}
