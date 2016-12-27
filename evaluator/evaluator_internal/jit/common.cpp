#include "common.h"

#if defined(_WIN32) || defined(_WIN64)
    #if !defined(NOMINMAX)
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <sys/mman.h>
#endif
#include <cstdlib>

// Executable memory allocation and deallocation

namespace evaluator_internal_jit
{

void * exec_alloc(std::size_t size)
{
#if defined(_WIN32) || defined(_WIN64)
    void * data = malloc(size);
    DWORD tmp;
    VirtualProtect(data, size, PAGE_EXECUTE_READWRITE, &tmp);
#else
    const int prot = PROT_READ | PROT_WRITE | PROT_EXEC;
#if defined(__APPLE__)
    const int flags = MAP_PRIVATE | MAP_ANON;
#else
    const int flags = MAP_PRIVATE | MAP_ANONYMOUS;
#endif
    void * data = mmap(NULL, size, prot, flags, -1, 0);
#endif
    return data;
}

void exec_dealloc(void * data, std::size_t size)
{
#if defined(_WIN32) || defined(_WIN64)
    free(data);
    (void)(size);
#else
    munmap(data, size);
#endif
}

}

