#include "nil.h"
#include "../memory.h"
#include "../vm.h"

MyMoNil *NilObject;
MyMoEmpty *EmptyObject;

void printNil(MyMoObject *object)
{
    printf("Nil");
}

void nil(MVM *vm)
{
    NilObject = AllocateObject(vm, MyMoNil, OBJ_NIL);
    NilObject->value = false;
    EmptyObject = AllocateObject(vm, MyMoEmpty, OBJ_OBJECT);
}

void defineNilClass(MVM *vm)
{
    MyMoString *name = newString(vm, "Nil", 3);
    MyMoBuiltInClass *nilClass = newBuiltInClass(vm, name);
    vm->builtInClasses[OBJ_NIL] = nilClass;
    // defineNilMethods(vm);
}