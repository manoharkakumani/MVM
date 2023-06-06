#ifndef __STRING_H__
#define __STRING_H__

#include "object.h"

#define NEW_STRING(vm, value, len) AS_OBJECT(newString(vm, value, len))
#define AS_STRING(object) ((MyMoString *)object)
#define STRING_VAL(object) ((MyMoString *)object)->value
#define IS_STRING(object) (object->type == OBJ_STRING)

typedef struct MyMoString
{
    MyMoObject object;
    int length;
    char *value;
} MyMoString;

MyMoString *newString(MVM *vm, const char *chars, int length);

void printString(MyMoString *string);

void defineStringClass(MVM *vm);

#endif