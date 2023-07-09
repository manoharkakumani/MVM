#include "function.h"
#include "nil.h"
#include "../vm.h"

MyMoBuiltInFunction *newBuiltInFunction(MVM *vm, MyMoString *name, BuiltInfunction function, MyMoObjectType type)
{
    MyMoBuiltInFunction *builtInFunction = AllocateObject(vm, MyMoBuiltInFunction, type);
    builtInFunction->name = name;
    builtInFunction->function = function;
    return builtInFunction;
}

MyMoFunction *newFunction(MVM *vm)
{
    MyMoFunction *function = AllocateObject(vm, MyMoFunction, OBJ_FUNCTION);
    function->name = NULL;
    function->argc = 0;
    function->variables = newDict(vm);
    function->assiginedParameters = newDict(vm);
    function->chunk = newChunk(vm);
    function->outerFunction = NULL;
    function->isargs = false;
    function->klass = AS_OBJECT(vm->builtInClasses[OBJ_OBJECT]);
    return function;
}

MyMoClouser *newClouser(MVM *vm, MyMoFunction *function)
{
    MyMoClouser *clouser = AllocateObject(vm, MyMoClouser, OBJ_CLOUSER);
    clouser->function = function;
    clouser->variables = newDict(vm);
    return clouser;
}

MyMoBoundMethod *newBoundMethod(MVM *vm, MyMoObject *self, MyMoFunction *method)
{
    MyMoBoundMethod *boundMethod = AllocateObject(vm, MyMoBoundMethod, OBJ_BOUND_METHOD);
    boundMethod->self = self;
    boundMethod->method = method;
    return boundMethod;
}

void defineMethod(MVM *vm, MyMoObjectType type, const char *name, BuiltInfunction function)
{
    MyMoObject *methodName = NEW_STRING(vm, name, strlen(name));
    MyMoObject *method = AS_OBJECT(newBuiltInFunction(vm, AS_STRING(methodName), function, OBJ_BUILTIN_METHOD));
    setEntry(vm, vm->builtInClasses[type]->methods, methodName, method);
}

void printFunction(MyMoFunction *function)
{
    switch (function->type)
    {
    case FN_INIT:
        printf("<init method %s at %p>", function->name->value, function);
        break;
    case FN_METHOD:
        printf("<method %s at %p>", function->name->value, function);
        break;
    case FN_OPERATOR:
        printf("<operator %s at %p>", function->name->value, function);
        break;
    case FN_GEN_METHOD:
    case FN_GENERATOR:
        printf("<generator %s at %p>", function->name->value, function);
        break;
    case FN_FUNCTION:
        printf("<function %s at %p>", function->name->value, function);
        break;
    case FN_ARROWFN:
        printf("<annonymos function at %p>", function);
        break;
    case FN_COMPILED:
        printf("<Script %s>", function->name->value);
        break;
    case FN_SCRIPT:
        printf("<Script>");
        break;
    default:
        printf("<unknown function at %p>", function);
        break;
    }
}

void printClouser(MyMoClouser *clouser)
{
    printf("<clouser of function %s at %p>", clouser->function->name->value, clouser);
}

void printBuiltInFunction(MyMoBuiltInFunction *builtInFunction)
{
    printf("<built-in function %s>", builtInFunction->name->value);
}

void printBuiltInMethod(MyMoBuiltInFunction *builtInFunction)
{
    printf("<built-in method %s>", builtInFunction->name->value);
}

void printBoundMethod(MyMoBoundMethod *boundMethod)
{
    printf("<bound method %s.%s at %p>", AS_INSTANCE(boundMethod->self)->klass->name->value, boundMethod->method->name->value, boundMethod);
}