#ifndef __LIST_H__
#define __LIST_H__

#include "object.h"

#define NEW_LIST(vm) AS_OBJECT(newList(vm))
#define AS_LIST(object) ((MyMoList *)object)
#define IS_LIST(object) (object->type == OBJ_LIST)

typedef struct MyMoList
{
    MyMoObject object;
    MyMoObjectArray values;
} MyMoList;

MyMoList *newList(MVM *vm);

void printList(MyMoList *list);

MyMoObject *getValueByIndex(MyMoList *list, uint index);

void defineListClass(MVM *vm);

#endif