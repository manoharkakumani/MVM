#ifndef __CACHE_H__
#define __CACHE_H__

#include "vm.h"

void functionSerialize(MyMoFunction *function, FILE *stream);
MyMoFunction *functionDeserialize(MVM *vm, FILE *stream);

#endif