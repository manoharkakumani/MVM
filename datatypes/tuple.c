#include "tuple.h"
#include "../memory.h"

MyMoTuple *newTuple(MVM *vm)
{
    MyMoTuple *tuple = AllocateObject(vm, MyMoTuple, OBJ_TUPLE);
    initMyMoObjectArray(vm, &tuple->values);
    return tuple;
}

void printTuple(MyMoTuple *tuple)
{
    printf("(");
    for (int i = 0; i < tuple->values.count; i++)
    {
        MyMoObject *object = tuple->values.objects[i];
        printObject(object);
        if (i != tuple->values.count - 1)
            printf(", ");
    }
    printf(")");
}