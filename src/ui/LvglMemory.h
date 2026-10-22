// Fixed LVGL 8 pool: prefer PSRAM while keeping an internal-RAM fallback.
#ifndef HOMEGENIE_MINI_LVGL_MEMORY_H
#define HOMEGENIE_MINI_LVGL_MEMORY_H

#if defined(ESP32) && defined(BOARD_HAS_PSRAM)
#include <stdlib.h>
#include <esp_heap_caps.h>

static inline void *homegenie_lvgl_pool_alloc(size_t size)
{
    // LVGL 8 calls this again during lv_mem_deinit(), without freeing the pool.
    // Keep one pool for the application lifetime, like its original static array.
    static void *pool = NULL;
    if (pool == NULL) {
        pool = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (pool == NULL) {
            pool = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        }
        // lv_tlsf_create_with_pool() cannot accept a NULL backing buffer.
        if (pool == NULL) abort();
    }
    return pool;
}
#endif

#endif // HOMEGENIE_MINI_LVGL_MEMORY_H
