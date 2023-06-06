#ifndef __CHUNK_H__
#define __CHUNK_H__

#include "common.h"
#include "opcodes.h"
#include "datatypes/object.h"

typedef struct chunk
{
    int count;
    int capacity;
    u32 *lines;
    u32 *cols;
    u8 *code;
    MyMoObjectArray constants;
} Chunk;

Chunk *newChunk(MVM *vm);
void initChunk(MVM *vm, Chunk *chunk);
void writeChunk(MVM *vm, Chunk *chunk, u8 byte, u32 line, u32 col);
void freeChunk(MVM *vm, Chunk *chunk);
void printChunk(Chunk *chunk);

int addConstant(MVM *vm, Chunk *chunk, MyMoObject *object);

#endif