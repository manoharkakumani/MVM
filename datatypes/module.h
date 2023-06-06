#ifndef __MODULE_H__
#define __MODULE_H__

#include "object.h"
#include "dict.h"
#include "string.h"

#define AS_MODULE(object) ((MyMoModule *)object)
#define IS_MODULE(object) (object->type == OBJ_MODULE)

typedef struct MyMoModule
{
    MyMoObject object;
    MyMoString *name;
    MyMoString *path;
    MyMoDict *variables;
    struct MyMoModule *parent;
} MyMoModule;

MyMoModule *newModule(MVM *vm, MyMoString *name, MyMoString *path);
void printModule(MyMoModule *module);

#endif