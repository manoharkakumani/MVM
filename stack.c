#include "memory.h"
#include "vm.h"

void push(MVM *vm, MyMoObject *object)
{
    writeMyMoObjectArray(vm, &vm->fiber->stack, object);
}

MyMoObject *pop(MVM *vm)
{
    return vm->fiber->stack.objects[--vm->fiber->stack.count];
}

MyMoObject *peek(MVM *vm, int position)
{
    return vm->fiber->stack.objects[vm->fiber->stack.count - position - 1];
}

void printStack(MVM *vm)
{
    int d = 0;
    printf("\n\t\t[ ");
    while (d < vm->fiber->stack.count)
    {
        printf(" ( ");
        if (vm->fiber->stack.objects[d] != NULL)
            printObject(vm->fiber->stack.objects[d]);
        printf(" ) ");
        d++;
    }
    printf(" ]\n\n");
}