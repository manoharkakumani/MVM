#include "bool.h"
#include "../memory.h"
#include "class.h"
#include "nil.h"
#include "../vm.h"

MyMoBool *TrueBool;
MyMoBool *FalseBool;

void printBool(MyMoBool *boolean)
{
    if (boolean->value)
    {
        printf("True");
    }
    else
    {
        printf("False");
    }
}

void boolean(MVM *vm)
{
    FalseBool = AllocateObject(vm, MyMoBool, OBJ_BOOL);
    FalseBool->value = false;
    TrueBool = AllocateObject(vm, MyMoBool, OBJ_BOOL);
    TrueBool->value = true;
}

MyMoObject *newBoolMethod(MVM *vm, uint argc, MyMoObject *args[])
{
    if (argc > 1)
    {
        runtimeError(vm, "bool() takes  1 argument (%d given)", argc);
        return NEW_EMPTY;
    }
    else if (argc == 0)
    {
        return NEW_BOOL(0);
    }
    else
    {
        if (IS_INT(args[0]))
        {
            long value = INT_VAL(pop(vm));
            if (value)
            {
                return NEW_BOOL(1);
            }
            return NEW_BOOL(0);
        }
        else if (IS_DOUBLE(args[0]))
        {
            double value = DOUBLE_VAL(pop(vm));
            if (value)
            {
                return NEW_BOOL(1);
            }
            return NEW_BOOL(0);
        }
        else if (IS_STRING(args[0]))
        {
            MyMoString *string = (MyMoString *)pop(vm);
            if (string->length)
            {
                return NEW_BOOL(1);
            }
            return NEW_BOOL(0);
        }
        else if (IS_BOOL(args[0]))
        {
            return pop(vm);
        }
        else
        {
            return NEW_BOOL(1);
        }
    }
}

void defineBoolMethods(MVM *vm)
{
    defineMethod(vm, OBJ_BOOL, "__new__", newBoolMethod);
}

void defineBoolClass(MVM *vm)
{
    MyMoString *name = newString(vm, "bool", 4);
    MyMoBuiltInClass *boolClass = newBuiltInClass(vm, name);
    vm->builtInClasses[OBJ_BOOL] = boolClass;
    setEntry(vm, &vm->builtins, AS_OBJECT(name), AS_OBJECT(boolClass));
    defineBoolMethods(vm);
}
