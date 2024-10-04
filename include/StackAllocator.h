#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

#include <cstddef>

class StackAllocator
{
public:
    StackAllocator(size_t totalSize);
    ~StackAllocator();

    void* Allocate(size_t size);
    void FreeToMarker(size_t marker);
    size_t GetMarker() const;

private:
    void* _start;
    size_t _offset;
    size_t _totalSize;
};

#endif
