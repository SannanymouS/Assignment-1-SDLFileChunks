#ifndef ASSET_H
#define ASSET_H

class Asset
{
public:
    Asset();
    ~Asset();

    // Load data into the asset (you can modify this based on your project's needs)
    void LoadData(void* data, size_t size);

private:
    void* assetData;  // Pointer to the asset's data
    size_t dataSize;
};

#endif  // ASSET_H
