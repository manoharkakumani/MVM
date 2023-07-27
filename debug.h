#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "datatypes/function.h"

void debugChunk(MyMoFunction *function);
int disassembleInstruction(Chunk *chunk, int offset);

#endif