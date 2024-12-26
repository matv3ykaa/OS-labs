#include "library.h"

#define MEMORY_POOL_SIZE 65536

void HandleError(const char *message) {
    write(STDERR_FILENO, message, strlen(message));
    exit(EXIT_FAILURE);
}

static Allocator *allocator_create_stub(void *const memory, const size_t size) {
    HandleError("allocator_create_stub: Fallback to mmap\n");

    void *mapped_memory = mmap(memory, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    if (mapped_memory == MAP_FAILED) {
        HandleError("allocator_create_stub: mmap failed\n");
    }

    return (Allocator *)mapped_memory;
}

static void allocator_destroy_stub(Allocator *const allocator) {
    HandleError("allocator_destroy_stub: Fallback to munmap\n");

    if (allocator) {
        if (munmap(allocator, MEMORY_POOL_SIZE) == -1) {
            HandleError("allocator_destroy_stub: munmap failed\n");
        }
    }
}

static void *allocator_alloc_stub(Allocator *const allocator, const size_t size) {
    HandleError("allocator_alloc_stub: Fallback to mmap\n");

    void *mapped_memory = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapped_memory == MAP_FAILED) {
        HandleError("allocator_alloc_stub: mmap failed\n");
    }

    return mapped_memory;
}

static void allocator_free_stub(Allocator *const allocator, void *const memory) {
    HandleError("allocator_free_stub: Fallback to munmap\n");

    if (memory && munmap(memory, sizeof(memory)) == -1) {
        HandleError("allocator_free_stub: munmap failed\n");
    }
}

static struct {
    allocator_create_f *create;
    allocator_destroy_f *destroy;
    allocator_alloc_f *alloc;
    allocator_free_f *free;
} allocator_funcs;

int main(int argc, char **argv) {
    if (argc < 2) {
        HandleError("Usage: ./main <library_path>\n");
    }

    void *library = dlopen(argv[1], RTLD_LOCAL | RTLD_NOW);
    if (!library) {
        HandleError("Failed to load library\n");
    }

    allocator_funcs.create = dlsym(library, "allocator_create");
    allocator_funcs.destroy = dlsym(library, "allocator_destroy");
    allocator_funcs.alloc = dlsym(library, "allocator_alloc");
    allocator_funcs.free = dlsym(library, "allocator_free");

    if (!allocator_funcs.create) allocator_funcs.create = allocator_create_stub;
    if (!allocator_funcs.destroy) allocator_funcs.destroy = allocator_destroy_stub;
    if (!allocator_funcs.alloc) allocator_funcs.alloc = allocator_alloc_stub;
    if (!allocator_funcs.free) allocator_funcs.free = allocator_free_stub;

    void *memory_pool = mmap(NULL, MEMORY_POOL_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory_pool == MAP_FAILED) {
        HandleError("Memory mapping failed\n");
    }

    Allocator *allocator = allocator_funcs.create(memory_pool, MEMORY_POOL_SIZE);
    if (!allocator) {
        HandleError("Failed to initialize allocator\n");
        munmap(memory_pool, MEMORY_POOL_SIZE);
        dlclose(library);
    }

    int *allocated_integer = (int *)allocator_funcs.alloc(allocator, sizeof(int));
    if (allocated_integer) {
        *allocated_integer = 123;
        write(STDOUT_FILENO, "Memory block allocated: integer with value 123\n", 48);
        allocator_funcs.free(allocator, allocated_integer);
        write(STDOUT_FILENO, "Memory block freed: integer\n", 29);
    }

    float *allocated_float = (float *)allocator_funcs.alloc(allocator, sizeof(float));
    if (allocated_float) {
        *allocated_float = 456.78f;
        write(STDOUT_FILENO, "Memory block allocated: float with value 456.78\n", 49);
        allocator_funcs.free(allocator, allocated_float);
        write(STDOUT_FILENO, "Memory block freed: float\n", 26);
    }

    double *allocated_double = (double *)allocator_funcs.alloc(allocator, sizeof(double));
    if (allocated_double) {
        *allocated_double = 789.123;
        write(STDOUT_FILENO, "Memory block allocated: double with value 789.123\n", 51);
        allocator_funcs.free(allocator, allocated_double);
        write(STDOUT_FILENO, "Memory block freed: double\n", 28);
    }

    int *array = (int *)allocator_funcs.alloc(allocator, 10 * sizeof(int));
    if (array) {
        for (int i = 0; i < 10; i++) {
            array[i] = i;
        }
        write(STDOUT_FILENO, "Memory block allocated: integer array\n", 39);
        allocator_funcs.free(allocator, array);
        write(STDOUT_FILENO, "Memory block freed: integer array\n", 35);
    }

    allocator_funcs.destroy(allocator);
    write(STDOUT_FILENO, "Allocator destroyed\n", 21);

    dlclose(library);
    munmap(memory_pool, MEMORY_POOL_SIZE);

    return EXIT_SUCCESS;
}
