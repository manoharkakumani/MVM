#ifndef __DOUBLE_H__
#define __DOUBLE_H__

#include "object.h"

#define NEW_DOUBLE(vm, value) AS_OBJECT(newDouble(vm, value))
#define AS_DOUBLE(object) ((MyMoDouble *)object)
#define DOUBLE_VAL(object) ((MyMoDouble *)object)->value
#define IS_DOUBLE(object) (object->type == OBJ_DOUBLE)

typedef struct
{
    MyMoObject object;
    double value;
    int length;
} MyMoDouble;

MyMoDouble *newDouble(MVM *vm, double value);

void printDouble(MyMoDouble *object);

void defineDoubleClass(MVM *vm);

#endif