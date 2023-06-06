#include "module.h"
#include "../vm.h"

MyMoModule *newModule(MVM *vm, MyMoString *name, MyMoString *path)
{
    MyMoModule *module = AllocateObject(vm, MyMoModule, OBJ_MODULE);
    module->name = name;
    module->path = path;
    module->variables = newDict(vm);
    module->parent = NULL;
    setEntry(vm, &vm->modules, AS_OBJECT(path), AS_OBJECT(module));
    return module;
}

void printModule(MyMoModule *module)
{
    printf("<module %s at %p>", module->name->value, module);
}
