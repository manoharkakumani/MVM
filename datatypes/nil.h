#ifndef __NIL_H__
#define __NIL_H__

#include "object.h"

#define NEW_NIL (MyMoObject *)NilObject
#define AS_NIL(object) ((MyMoNil *)object)
#define NIL_VAL(object) ((MyMoNil *)object)->value
#define IS_NIL(object) (object->type == OBJ_NIL)

#define NEW_EMPTY (MyMoObject *)EmptyObject
#define AS_EMPTY(object) ((MyMoEmpty *)object)
#define IS_EMPTY(object) (object->type == OBJ_OBJECT)

typedef struct MyMoNil
{
    MyMoObject object;
    bool value;
} MyMoNil;

typedef struct MyMoEmpty
{
    MyMoObject object;
} MyMoEmpty;

extern MyMoNil *NilObject;
extern MyMoEmpty *EmptyObject;
void nil(MVM *vm);

void defineNilClass(MVM *vm);

#endif