#include "Asset.h"
#include <cstring>
#include <cstdlib>
#include <stdlib.h>


Asset::Asset() : assetData(nullptr), dataSize(0) {}

Asset::~Asset()
{
    if (assetData)
    {
        free(assetData);  // Free asset data if it's been allocated
    }
}

void Asset::LoadData(void* data, size_t size)
{
    assetData = malloc(size);
    memcpy(assetData, data, size);  // Copy data into the asset's internal memory
    dataSize = size;
}
