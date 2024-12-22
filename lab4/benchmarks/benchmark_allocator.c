#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <time.h>

typedef void* (*malloc_func)(size_t);
typedef void (*free_func)(void*);
typedef void (*print_free_list_func)();

#define NUM_ALLOCATIONS 10000
#define MAX_ALLOCATION_SIZE 1024

void benchmark_allocator(const char* library_path) {
    printf("Testing allocator from library: %s\n", library_path);

    void* library = dlopen(library_path, RTLD_LAZY);
    if (!library) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    malloc_func my_malloc = (malloc_func)dlsym(library, "my_malloc");
    free_func my_free = (free_func)dlsym(library, "my_free");

    if (!my_malloc || !my_free) {
        fprintf(stderr, "Error loading symbols: %s\n", dlerror());
        dlclose(library);
        exit(EXIT_FAILURE);
    }

    void* allocations[NUM_ALLOCATIONS];
    size_t sizes[NUM_ALLOCATIONS];
    clock_t start_time, end_time;

    printf("Allocating memory...\n");
    start_time = clock();
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        sizes[i] = rand() % MAX_ALLOCATION_SIZE + 1; // Размер от 1 до MAX_ALLOCATION_SIZE
        allocations[i] = my_malloc(sizes[i]);
        if (!allocations[i]) {
            fprintf(stderr, "Allocation failed at iteration %d\n", i);
            break;
        }
    }
    end_time = clock();
    printf("Allocation completed in %.6f seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    printf("Freeing memory...\n");
    start_time = clock();
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        if (allocations[i]) {
            my_free(allocations[i]);
        }
    }
    end_time = clock();
    printf("Deallocation completed in %.6f seconds\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    dlclose(library);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <firstfit_library_path> <bestfit_library_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    benchmark_allocator(argv[1]);

    benchmark_allocator(argv[2]);

    return EXIT_SUCCESS;
}
