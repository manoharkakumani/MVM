#ifndef __STACK_H__
#define __STACK_H__

#include "common.h"
#include "datatypes/object.h"

void printStack(MVM *vm);
void manipulateStack(MVM *vm, MyMoObject *value, int position);
void push(MVM *vm, MyMoObject *object);

MyMoObject *pop(MVM *vm);
MyMoObject *peek(MVM *vm, int position);

#endif