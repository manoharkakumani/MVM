#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "datatypes/object.h"

#define New(type, n) (type *)malloc(sizeof(type) * (n))

#define Allocate(vm, type, n) \
    (type *)reallocate(vm, NULL, 0, sizeof(type) * (n))

#define ResizeCapacity(c) ((c) < 8 ? 8 : (c) << 1)

#define ResizeArray(vm, type, pointer, oldCount, newCount)     \
    (type *)reallocate(vm, pointer, sizeof(type) * (oldCount), \
                       sizeof(type) * (newCount))

#define Free(vm, type, pointer) reallocate(vm, pointer, sizeof(type), 0)

#define FreeArray(vm, type, pointer, oldCount) \
    reallocate(vm, pointer, sizeof(type) * (oldCount), 0)

#define AllocateObject(vm, type, objectType) \
    (type *)allocateObject(vm, sizeof(type), objectType)

void *reallocate(MVM *vm, void *pointer, size_t oldSize, size_t newSize);
void freeObjects(MVM *vm);

MyMoObject *allocateObject(MVM *vm, size_t size, MyMoObjectType type);


#endif