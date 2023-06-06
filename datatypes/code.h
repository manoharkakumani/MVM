#ifndef __CODE_H__
#define __CODE_H__

#include "object.h"
#include "function.h"

#define AS_CODE(object) ((MyMoCode *)object)
#define IS_CODE(object) object->type == OBJ_CODE

typedef struct MyMoCode
{
    MyMoObject object;
    MyMoFunction *function;
    MVM *vm;
} MyMoCode;

MyMoCode *newCode(MVM *vm);
void freeCode(MyMoCode *code);
void printCode(MyMoCode *code);

#endif