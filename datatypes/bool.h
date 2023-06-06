#ifndef __BOOL_H__
#define __BOOL_H__

#include "object.h"

#define NEW_BOOL(value) (MyMoObject *)(value ? TrueBool : FalseBool)
#define AS_BOOL(object) ((MyMoBool *)object)
#define BOOL_VAL(object) ((MyMoBool *)object)->value
#define IS_BOOL(object) (object->type == OBJ_BOOL)

typedef struct MyMoBool
{
    MyMoObject object;
    bool value;
} MyMoBool;

extern MyMoBool *TrueBool;
extern MyMoBool *FalseBool;

void printBool(MyMoBool *boolean);

void boolean(MVM *vm);

void defineBoolClass(MVM *vm);

#endif