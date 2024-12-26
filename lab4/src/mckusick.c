#include "library.h"

#define MIN_BLOCK_SIZE 32
#define MAX_BLOCK_SIZE 1024
#define MAX_PAGE_SIZE 4096

typedef struct Page {
    size_t block_size;
    size_t free_blocks;
    struct Page *next;
    uint8_t *bitmap;
    void *data;
} Page;

typedef struct Allocator {
    Page *pages[MAX_BLOCK_SIZE / MIN_BLOCK_SIZE + 1];
    void *memory_start;
    size_t total_size;
} Allocator;

EXPORT Allocator *allocator_create(void *memory, size_t size) {
    if (!memory || size < sizeof(Allocator)) {
        return NULL;
    }

    Allocator *allocator = (Allocator *)memory;
    allocator->memory_start = (char *)memory + sizeof(Allocator);
    allocator->total_size = size - sizeof(Allocator);

    for (size_t i = 0; i <= MAX_BLOCK_SIZE / MIN_BLOCK_SIZE; ++i) {
        allocator->pages[i] = NULL;
    }

    return allocator;
}

EXPORT void allocator_destroy(Allocator *const allocator) {
    if (!allocator) return;

    memset(allocator, 0, allocator->total_size);
}

static Page *create_page(Allocator *allocator, size_t block_size) {
    if (!allocator || block_size > MAX_BLOCK_SIZE || block_size < MIN_BLOCK_SIZE) {
        return NULL;
    }

    size_t page_size = MAX_PAGE_SIZE;
    size_t bitmap_size = page_size / block_size / 8;
    void *page_memory = (void *)((char *)allocator->memory_start + allocator->total_size - page_size);

    Page *page = (Page *)page_memory;
    page->block_size = block_size;
    page->free_blocks = page_size / block_size;
    page->bitmap = (uint8_t *)((char *)page_memory + sizeof(Page));
    page->data = (void *)((char *)page_memory + sizeof(Page) + bitmap_size);

    memset(page->bitmap, 0, bitmap_size);

    return page;
}

EXPORT void *allocator_alloc(Allocator *allocator, size_t size) {
    if (!allocator || size == 0 || size > MAX_BLOCK_SIZE) {
        return NULL;
    }

    size = (size + MIN_BLOCK_SIZE - 1) / MIN_BLOCK_SIZE * MIN_BLOCK_SIZE;
    size_t page_index = size / MIN_BLOCK_SIZE - 1;

    Page *page = allocator->pages[page_index];

    if (!page) {
        page = create_page(allocator, size);
        if (!page) return NULL;

        allocator->pages[page_index] = page;
    }

    for (size_t i = 0; i < page->free_blocks; ++i) {
        if (!(page->bitmap[i / 8] & (1 << (i % 8)))) {
            page->bitmap[i / 8] |= (1 << (i % 8));
            --page->free_blocks;

            return (void *)((char *)page->data + i * size);
        }
    }

    return NULL;
}

EXPORT void allocator_free(Allocator *allocator, void *memory) {
    if (!allocator || !memory) return;

    for (size_t i = 0; i <= MAX_BLOCK_SIZE / MIN_BLOCK_SIZE; ++i) {
        Page *page = allocator->pages[i];

        while (page) {
            if ((char *)memory >= (char *)page->data && (char *)memory < (char *)page->data + MAX_PAGE_SIZE) {
                size_t offset = (char *)memory - (char *)page->data;
                size_t block_index = offset / page->block_size;

                page->bitmap[block_index / 8] &= ~(1 << (block_index % 8));
                ++page->free_blocks;
                return;
            }

            page = page->next;
        }
    }
}
