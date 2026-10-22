#ifndef HOMEGENIE_MINI_DUKTAPE_MEMORY_H
#define HOMEGENIE_MINI_DUKTAPE_MEMORY_H

#include <duktape.h>

#if defined(ESP32) && defined(BOARD_HAS_PSRAM)
#include <esp_heap_caps.h>
#endif

namespace Automation { namespace ScriptMemory {

#if defined(ESP32) && defined(BOARD_HAS_PSRAM)
    inline void *allocate(void *, duk_size_t size) {
        if (size == 0) return nullptr;
        void *memory = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        return memory != nullptr ? memory
            : heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }

    inline void release(void *, void *memory) {
        if (memory != nullptr) heap_caps_free(memory);
    }

    inline void *resize(void *userData, void *memory, duk_size_t size) {
        if (size == 0) {
            release(userData, memory);
            return nullptr;
        }
        if (memory == nullptr) return allocate(userData, size);
        // A failed realloc leaves the original block intact. A successful one
        // may move it between heaps while preserving its contents.
        void *resized = heap_caps_realloc(memory, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        return resized != nullptr ? resized
            : heap_caps_realloc(memory, size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
#endif

    inline duk_context *createHeap() {
#if defined(ESP32) && defined(BOARD_HAS_PSRAM)
        return duk_create_heap(allocate, resize, release, nullptr, nullptr);
#else
        return duk_create_heap_default();
#endif
    }

}}

#endif // HOMEGENIE_MINI_DUKTAPE_MEMORY_H
