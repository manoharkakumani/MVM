#include "code.h"
#include "../vm.h"

MyMoCode *newCode(MVM *vm)
{
    MyMoCode *code = AllocateObject(vm, MyMoCode, OBJ_CODE);
    code->function = NULL;
    code->vm = initVM();
    return code;
}

void freeCode(MyMoCode *code)
{
    freeVM(code->vm);
    free(code);
}

void printCode(MyMoCode *code)
{
    printf("<object code at %p>", code);
}
