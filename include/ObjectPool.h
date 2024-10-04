#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include <vector>
#include <iostream>
#include <typeinfo>

template<typename T>
class ObjectPool
{
public:
    // Constructor to initialize the pool with a specific size
    ObjectPool(size_t poolSize)
    {
        pool.reserve(poolSize);  // Reserve space to avoid resizing

        // Create 'poolSize' objects initially
        for (size_t i = 0; i < poolSize; ++i)
        {
            pool.push_back(new T());
        }

        // unit test - Debug print to verify pool size
        //std::cout << "Pool initialized with " << pool.size() << " objects" << std::endl;
    }

    // Acquire an object from the pool
    T* Acquire()
    {
        // Always create a new object for each chunk
        std::cout << "Creating new pool object: " << typeid(T).name() << std::endl;
        return new T();
    }

    // Release object back to the pool
    void Release(T* obj)
    {
        // Return the object back to the pool
        pool.push_back(obj);
        // unit test - seeing when object is released
        //std::cout << "Releasing object back to pool: " << typeid(T).name() << std::endl;
    }

private:
    std::vector<T*> pool;  // Vector to hold available objects in the pool
};

#endif // OBJECTPOOL_H
