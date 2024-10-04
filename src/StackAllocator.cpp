#include "StackAllocator.h"
#include <cassert>
#include <cstdlib>

StackAllocator::StackAllocator(size_t totalSize)
{
    _start = malloc(totalSize);
    _offset = 0;
    _totalSize = totalSize;
}

StackAllocator::~StackAllocator()
{
    free(_start);
}

void* StackAllocator::Allocate(size_t size)
{
    assert(_offset + size <= _totalSize && "StackAllocator: Not enough memory.");
    void* ptr = static_cast<unsigned char*>(_start) + _offset;
    _offset += size;
    return ptr;
}


void StackAllocator::FreeToMarker(size_t marker)
{
    assert(marker <= _offset && "StackAllocator: Invalid marker.");
    _offset = marker;
}

size_t StackAllocator::GetMarker() const
{
    return _offset;
}
