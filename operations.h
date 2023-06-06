#ifndef __OPERATIONS_H__
#define __OPERATIONS_H__

#include "datatypes/object.h"

MyMoObject *addition(MVM *vm, MyMoObject *a, MyMoObject *b);
MyMoObject *subtraction(MVM *vm, MyMoObject *a, MyMoObject *b);
MyMoObject *multiplication(MVM *vm, MyMoObject *a, MyMoObject *b);
MyMoObject *division(MVM *vm, MyMoObject *a, MyMoObject *b);

#endif