#include "first_fit_allocator.h"

// Define a block structure
typedef struct Block {
    size_t size;         // Size of the block
    int free;            // Free (1) or allocated (0)
    struct Block* next;  // Pointer to the next block
} Block;

#define BLOCK_SIZE sizeof(Block)

static Block* free_list = NULL; // Start of the free list

// Align size to the nearest multiple of 8
size_t align8(size_t size) {
    return (size + 7) & ~7;
}

// Request memory from the OS using mmap
Block* request_memory(size_t size) {
    size_t total_size = size + BLOCK_SIZE;
    Block* block = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) {
        return NULL;
    }

    block->size = size;
    block->free = 0;
    block->next = NULL;
    return block;
}

// Find the first fit block
Block* find_first_fit(size_t size) {
    Block* current = free_list;
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Split a block if it's too large
void split_block(Block* block, size_t size) {
    if (block->size >= size + BLOCK_SIZE + 8) {
        Block* new_block = (Block*)((char*)block + BLOCK_SIZE + size);
        new_block->size = block->size - size - BLOCK_SIZE;
        new_block->free = 1;
        new_block->next = block->next;
        block->size = size;
        block->next = new_block;
    }
}

// Allocate memory
void* my_malloc(size_t size) {
    if (size <= 0) {
        return NULL;
    }

    size = align8(size);

    Block* block = find_first_fit(size);
    if (!block) {
        block = request_memory(size);
        if (!block) {
            return NULL;
        }
        if (free_list) {
            Block* current = free_list;
            while (current->next) {
                current = current->next;
            }
            current->next = block;
        } else {
            free_list = block;
        }
    } else {
        block->free = 0;
        split_block(block, size);
    }

    return (void*)((char*)block + BLOCK_SIZE);
}

// Merge adjacent free blocks
void merge_blocks() {
    Block* current = free_list;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += BLOCK_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

// Free allocated memory
void my_free(void* ptr) {
    if (!ptr) {
        return;
    }

    Block* block = (Block*)((char*)ptr - BLOCK_SIZE);
    block->free = 1;
    merge_blocks();
}

// Test the allocator
void print_free_list() {
    Block* current = free_list;
    printf("Free list:\n");
    while (current) {
        printf("Block at %p, size: %zu, free: %d\n", (void*)current, current->size, current->free);
        current = current->next;
    }
}
