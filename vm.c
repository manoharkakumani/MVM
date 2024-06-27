#include "common.h"
#include "vm.h"
#include "stack.h"
#include "operations.h"
#include "datatypes/datatypes.h"
#include "utils.h"
#include "debug.h"

#include <math.h>

// #define DEBUG_STACK_TRACE
// #define DEBUG_PRINT_CODE

MVM *initVM()
{
    MVM *vm = New(MVM, 1);
    vm->objects = NULL;
    vm->currentModule = NULL;
    vm->currentClass = NULL;
    vm->objectClass = NULL;
    vm->classCall = 0;
    vm->fiber = newFiber(vm, NULL);
    initDict(&vm->globals);
    initDict(&vm->builtins);
    initDict(&vm->strings);
    initDict(&vm->numbers);
    initDict(&vm->integers);
    initDict(&vm->doubles);
    initDict(&vm->modules);
    initDict(&vm->builtInModules);
    defineBuiltInClasses(vm);
    defineBuiltInFunctions(vm);
    return vm;
}

void freeVM(MVM *vm)
{
    // printObjectarray(vm->objects);
    // resetStack(vm);
    freeDict(vm, &vm->globals);
    freeDict(vm, &vm->builtins);
    freeDict(vm, &vm->strings);
    freeDict(vm, &vm->numbers);
    freeDict(vm, &vm->integers);
    freeDict(vm, &vm->doubles);
    freeDict(vm, &vm->modules);
    freeDict(vm, &vm->builtInModules);
    freeObjects(vm);
    free(vm);
}

void runtimeError(MVM *vm, const char *format, ...)
{
    printf("Traceback (most recent call last): \n");
    MyMoFiber *fiber = vm->fiber;
    while (fiber != NULL)
    {
        if (fiber->parent != NULL)
        {
            printf("  while running fiber of %s \n", fiber->callFrames[0]->function->name->value);
        }
        for (u32 i = 0; i <= fiber->frameCount; i++)
        {
            CallFrame *frame = fiber->callFrames[i];
            MyMoFunction *function = frame->function;
            size_t instruction = frame->ip - function->chunk->code - 1;
            fprintf(stderr, "  [%d : %d] in ",
                    function->chunk->lines[instruction], function->chunk->cols[instruction]);
            if (function->type == FN_MODULE)
            {
                fprintf(stderr, "<module %s>\n", function->name->value);
            }
            else
            {
                fprintf(stderr, "%s\n", function->name->value);
            }

            if (!fiber->parent && i == fiber->frameCount)
            {
                va_list args;
                va_start(args, format);
                vfprintf(stderr, format, args);
                va_end(args);
                fputs("\n", stderr);
            }
        }
        fiber = fiber->parent;
        // resetStack(vm);
    }
}

bool callFunction(MVM *vm, MyMoFunction *function, int argc)
{
    if (function->argc != argc)
    {
        runtimeError(vm, "TypeError : %s() Takes %d arguments but got %d.", function->name->value, function->argc, argc);
        return false;
    }
    else
    {
        if (vm->fiber->frameCapacity < vm->fiber->frameCount + 1)
        {
            u32 capacity = vm->fiber->frameCapacity;
            vm->fiber->frameCapacity = ResizeCapacity(vm->fiber->frameCapacity);
            vm->fiber->callFrames = ResizeArray(vm, CallFrame *, vm->fiber->callFrames, capacity, vm->fiber->frameCapacity);
        }
        CallFrame *frame = New(CallFrame, 1);
        frame->function = function;
        frame->ip = function->chunk->code;
        initDict(&frame->locals);
        vm->fiber->callFrames[++vm->fiber->frameCount] = frame;
        if (argc && !function->isargs)
        {
            for (int i = argc - 1; i >= 0; i--)
            {
                setEntry(vm, &frame->locals, AS_OBJECT(function->argv[i]), pop(vm));
            }
        }
        return true;
    }
}

bool caller(MVM *vm, MyMoObject *callee, u32 argc)
{
    switch (callee->type)
    {
    case OBJ_BUILTIN_METHOD:
    case OBJ_BUILTIN_FUNCTION:
    {
        BuiltInfunction function = AS_BUILTIN_FUNCTION(callee)->function;
        MyMoObject *result = function(vm, argc, vm->fiber->stack.objects + vm->fiber->stack.count - argc);
        if (IS_EMPTY(result))
            return false;
        pop(vm);
        push(vm, result);
        return true;
    }
    case OBJ_FUNCTION:
    {

        MyMoFunction *function = AS_FUNCTION(callee);
        if (function->type == FN_SCRIPT)
        {
            runtimeError(vm, "TypeError : <Script '%s'> is not callable.", function->name->value);
            return false;
        }
        else if (function->type == FN_GENERATOR || function->type == FN_GEN_METHOD)
        {
            runtimeError(vm, "TypeError : <generator '%s'> is not callable.", function->name->value);
            return false;
        }
        else if (function->type == FN_MODULE)
        {
            runtimeError(vm, "TypeError : <module '%s'> is not callable.", function->name->value);
            return false;
        }
        return callFunction(vm, function, argc);
    }
    case OBJ_CLASS:
    {
        MyMoObject *newMethed = NEW_STRING(vm, "__new__", 7);
        MyMoClass *klass = AS_CLASS(callee);
        MyMoObject *__new__ = getEntry(vm->builtInClasses[OBJ_OBJECT]->methods, newMethed);
        BuiltInfunction function = AS_BUILTIN_FUNCTION(__new__)->function;
        push(vm, callee);
        MyMoObject *result = function(vm, 1, &callee);
        if (IS_EMPTY(result))
            return false;
        if (klass->init)
        {
            vm->classCall++;
            vm->fiber->stack.objects[vm->fiber->stack.count - argc - 1] = result;
            return caller(vm, klass->init, argc + 1);
        }
        else
        {
            if (argc)
            {
                runtimeError(vm, "TypeError : %s() Takes 0 arguments but got %d.", AS_CLASS(callee)->name->value, argc);
                return false;
            }
            pop(vm);
            push(vm, result);
            return true;
        }
    }
    case OBJ_BUILTIN_CLASS:
    {
        MyMoBuiltInClass *klass = AS_BUILTIN_CLASS(callee);
        MyMoObject *newMethed = NEW_STRING(vm, "__new__", 7);
        MyMoObject *__new__ = getEntry(klass->methods, newMethed);
        BuiltInfunction function = AS_BUILTIN_FUNCTION(__new__)->function;
        MyMoObject *result = function(vm, argc, vm->fiber->stack.objects + vm->fiber->stack.count - argc);
        if (IS_EMPTY(result))
            return false;
        pop(vm);
        push(vm, result);
        return true;
    }
    case OBJ_BOUND_METHOD:
    {
        MyMoBoundMethod *bound = AS_BOUND_METHOD(callee);
        vm->fiber->stack.objects[vm->fiber->stack.count - argc - 1] = bound->self;
        return caller(vm, AS_OBJECT(bound->method), argc + 1);
    }
    default:
        printObject(callee);
        runtimeError(vm, "Can only call functions and classes.");
        return false;
    }
}

#define BitwiseOp(a, b, op)                                             \
    do                                                                  \
    {                                                                   \
        if (!IS_INT(a) || !IS_INT(b))                                   \
        {                                                               \
            runtimeError(vm, "TypeError : Operands must be integers."); \
            return false;                                               \
        }                                                               \
        push(vm, NEW_INT(vm, INT_VAL(a) op INT_VAL(b)));                \
    } while (0);

#define UnaryOp(op, a)                                     \
    do                                                     \
    {                                                      \
        if (IS_INT(a))                                     \
            push(vm, NEW_INT(vm, op(INT_VAL(a))));         \
        else if (IS_DOUBLE(a))                             \
            push(vm, NEW_DOUBLE(vm, op(DOUBLE_VAL(a))));   \
        else                                               \
        {                                                  \
            runtimeError(vm, "Operand must be a number."); \
            return RUNTIME_ERROR;                          \
        }                                                  \
    } while (0);

#define OperatorOverLoad(a, b, op)                                                          \
    do                                                                                      \
    {                                                                                       \
        MyMoObject *method = getMethod(vm, a, op);                                          \
        if (IS_EMPTY(method))                                                               \
        {                                                                                   \
            runtimeError(vm, "MethodNotFound: %s does not have method %s", getType(a), op); \
            return RUNTIME_ERROR;                                                           \
        }                                                                                   \
        push(vm, method);                                                                   \
        push(vm, a);                                                                        \
        if (b != NULL)                                                                      \
            push(vm, b);                                                                    \
        if (!caller(vm, method, b != NULL ? 2 : 1))                                         \
        {                                                                                   \
            return RUNTIME_ERROR;                                                           \
        }                                                                                   \
        frame = vm->fiber->callFrames[vm->fiber->frameCount];                               \
        DISPATCH();                                                                         \
    } while (0)

bool isFalsey(MyMoObject *obj)
{
    return IS_NIL(obj) ||
           (IS_BOOL(obj) && !BOOL_VAL(obj)) ||
           (IS_INT(obj) && INT_VAL(obj) == 0) ||
           (IS_DOUBLE(obj) && DOUBLE_VAL(obj) == 0) ||
           (IS_STRING(obj) && STRING_VAL(obj)[0] == '\0');
}

int runMVM(MVM *vm)
{
    int agp = 0;
    CallFrame *frame = vm->fiber->callFrames[vm->fiber->frameCount];
#include "dispatch.h"
#define ReadByte() (*frame->ip++)
#define ReadShort() (frame->ip += 2, (u16)(frame->ip[-2] << 8) | frame->ip[-1])
#define ReadConstant() (frame->function->chunk->constants.objects[ReadByte()])
#define ReadObject() ReadConstant()

#ifdef DEBUG_STACK_TRACE

#define DISPATCH()                                                                                       \
    do                                                                                                   \
    {                                                                                                    \
        printStack(vm);                                                                                  \
        disassembleInstruction(frame->function->chunk, (int)(frame->ip - frame->function->chunk->code)); \
        goto *dispatchTable[ReadByte()];                                                                 \
    } while (0);

#else

#define DISPATCH() \
                   \
    goto *dispatchTable[ReadByte()]

#endif

    for (;;)
    {
    OP_NOP:
    {
        DISPATCH();
    }
    OP_CONST:
    {
        push(vm, ReadConstant());
        DISPATCH();
    }
    OP_NIL:
    {
        push(vm, NEW_NIL);
        DISPATCH();
    }
    OP_TRUE:
    {
        push(vm, NEW_BOOL(true));
        DISPATCH();
    }
    OP_FALSE:
    {
        push(vm, NEW_BOOL(false));
        DISPATCH();
    }
    OP_LIST:
    {
        u32 count = ReadByte();
        MyMoList *list = newList(vm);
        for (u32 i = 0; i < count; i++)
        {
            writeMyMoObjectArray(vm, &list->values, peek(vm, count - i - 1));
        }
        for (u32 i = 0; i < count; i++)
        {
            pop(vm);
        }
        push(vm, AS_OBJECT(list));
        DISPATCH();
    }
    OP_TUPLE:
    {
        u32 count = ReadByte();
        MyMoTuple *tuple = newTuple(vm);
        for (u32 i = 0; i < count; i++)
        {
            writeMyMoObjectArray(vm, &tuple->values, peek(vm, count - i - 1));
        }
        for (u32 i = 0; i < count; i++)
        {
            pop(vm);
        }
        push(vm, AS_OBJECT(tuple));
        DISPATCH();
    }
    OP_DICT:
    {
        u32 count = ReadByte();
        MyMoDict *dict = newDict(vm);
        for (u32 i = 0; i < count; i++)
        {
            MyMoObject *value = pop(vm);
            MyMoObject *key = pop(vm);
            if (!IS_NIL(key) && !IS_BOOL(key) && !IS_INT(key) && !IS_DOUBLE(key) && !IS_STRING(key))
            {
                runtimeError(vm, "TypeError: Dictionary keys must be immutable.");
                return RUNTIME_ERROR;
            }
            setEntry(vm, dict, key, value);
        }
        push(vm, AS_OBJECT(dict));
        DISPATCH();
    }
    OP_SUBSCR:
    {
        MyMoObject *index = pop(vm);
        if (IS_DICT(peek(vm, 0)))
        {
            MyMoDict *dict = AS_DICT(pop(vm));
            MyMoObject *value = getEntry(dict, index);
            if (!(value))
            {
                runtimeError(vm, "KeyError: value not found ");
                return RUNTIME_ERROR;
            }
            push(vm, value);
            DISPATCH();
        }
        if (!IS_INT(index))
        {
            runtimeError(vm, "TypeError: Indices must be integers.");
            return RUNTIME_ERROR;
        }
        int indexValue = INT_VAL(index);
        MyMoObject *object = pop(vm);
        switch (object->type)
        {
        case OBJ_STRING:
        {
            MyMoString *string = AS_STRING(object);
            if (indexValue >= string->length || -indexValue > string->length)
            {
                runtimeError(vm, "TypeError: String index out of bounds.");
                return RUNTIME_ERROR;
            }
            if (indexValue < 0)
            {
                indexValue += string->length;
            }
            push(vm, NEW_STRING(vm, &string->value[indexValue], 1));
            break;
        }
        case OBJ_LIST:
        {
            MyMoList *list = AS_LIST(object);
            if (indexValue >= list->values.count || -indexValue > list->values.count)
            {
                runtimeError(vm, "TypeError: List Index out of bounds.");
                return RUNTIME_ERROR;
            }
            if (indexValue < 0)
            {
                push(vm, list->values.objects[list->values.count + indexValue]);
            }
            else
            {
                push(vm, list->values.objects[indexValue]);
            }
            break;
        }
        case OBJ_TUPLE:
        {
            MyMoTuple *tuple = AS_TUPLE(object);
            if (indexValue >= tuple->values.count || -indexValue > tuple->values.count)
            {
                runtimeError(vm, "Tuple Index out of bounds.");
                return RUNTIME_ERROR;
            }
            if (indexValue < 0)
            {
                push(vm, tuple->values.objects[tuple->values.count + indexValue]);
            }
            else
            {
                push(vm, tuple->values.objects[indexValue]);
            }
            break;
        }
        default:
            runtimeError(vm, "TypeError: Index operator cannot  applied to %s", getType(object));
            return RUNTIME_ERROR;
        }
        DISPATCH();
    }
    OP_SETSUBSCR:
    {
        MyMoObject *value = pop(vm);
        MyMoObject *index = pop(vm);
        MyMoObject *object = pop(vm);
        if (IS_LIST(object))
        {
            if (!IS_INT(index))
            {
                runtimeError(vm, "TypeError: Indices must be integers.");
                return RUNTIME_ERROR;
            }
            int indexValue = INT_VAL(index);
            MyMoList *list = AS_LIST(object);
            if (indexValue >= list->values.count || -indexValue > list->values.count)
            {
                runtimeError(vm, "TypeError: List Index out of bounds.");
                return RUNTIME_ERROR;
            }
            if (indexValue < 0)
            {
                indexValue += list->values.count;
            }
            list->values.objects[indexValue] = value;
            push(vm, value);
            DISPATCH();
        }
        else if (IS_DICT(object))
        {
            MyMoDict *dict = AS_DICT(object);
            if (!IS_STRING(index) && !IS_INT(index) && !IS_BOOL(index) && !IS_NIL(index) && !IS_DOUBLE(index))
            {
                runtimeError(vm, "TypeError: Dictionary keys must be hashable.");
                return RUNTIME_ERROR;
            }
            setEntry(vm, dict, index, value);
            push(vm, value);
            DISPATCH();
        }
        else
        {
            runtimeError(vm, "TypeError: '%s' does not support item assignment.", getType(object));
            return RUNTIME_ERROR;
        }
    }
    OP_UNPACK:
    {
        // u32 count = ReadByte();
        // MyMoObject *object = pop(vm);
        // if (IS_LIST(object))
        // {
        //     MyMoList *list = AS_LIST(object);
        // }
        DISPATCH();
    }
    OP_SLICE:
    {
        MyMoObject *st = pop(vm);
        MyMoObject *ed = pop(vm);
        MyMoObject *sta = pop(vm);
        if (IS_NIL(st) && IS_NIL(ed) && IS_NIL(sta))
        {
            DISPATCH();
        }
        if ((!IS_INT(st) && !IS_NIL(st)) || (!IS_INT(ed) && !IS_NIL(ed)) || (!IS_INT(sta) && !IS_NIL(sta)))
        {
            runtimeError(vm, "TypeError: expect integer as index to slice");
            return RUNTIME_ERROR;
        }
        int step = IS_NIL(st) ? 1 : AS_INT(st)->value;
        if (step < 0)
        {
            st = sta;
            sta = ed;
            ed = st;
        }
        int start = IS_NIL(sta) ? 0 : AS_INT(sta)->value;
        if (!step)
        {
            runtimeError(vm, "TypeError: slice step cannot be zero");
            return RUNTIME_ERROR;
        }
        MyMoObject *object = pop(vm);
        switch (object->type)
        {
        case OBJ_STRING:
        {
            MyMoString *str = AS_STRING(object);
            char _str[str->length];
            int end = IS_NIL(ed) ? (step < 0 ? str->length - 1 : str->length) : AS_INT(ed)->value;
            int j = 0;
            if (start < 0)
            {
                start += str->length;
            }
            if (end < 0)
            {
                end += str->length;
            }
            if (end > str->length)
            {
                end = step < 0 ? str->length - 1 : str->length;
            }
            if (start > end || (start >= str->length && end > str->length) || start >= str->length)
            {
                push(vm, NEW_STRING(vm, "", 0));
                break;
            }
            if (step > 0)
            {
                for (int i = start; i < end; i += step)
                {
                    _str[j++] = str->value[i];
                }
                _str[j] = '\0';
                push(vm, NEW_STRING(vm, _str, j));
                break;
            }
            else
            {
                start = IS_NIL(sta) ? -1 : start;
                for (int i = end; i > start; i += step)
                {
                    _str[j++] = str->value[i];
                }
                _str[j] = '\0';
                push(vm, NEW_STRING(vm, _str, j));
                break;
            }
        }
        case OBJ_LIST:
        {
            MyMoList *lst = AS_LIST(object);
            MyMoList *list = newList(vm);
            int end = IS_NIL(ed) ? (step < 0 ? lst->values.count - 1 : lst->values.count) : AS_INT(ed)->value;
            if (start < 0)
            {
                start += lst->values.count;
            }
            if (end < 0)
            {
                end += lst->values.count;
            }
            else if (end > lst->values.count)
            {
                end = step < 0 ? lst->values.count - 1 : lst->values.count;
            }
            if (start > end || (start >= lst->values.count && end > lst->values.count) || start >= lst->values.count)
            {
                push(vm, AS_OBJECT(list));
                break;
            }
            if (step > 0)
            {
                for (int i = start; i < end; i += step)
                {
                    writeMyMoObjectArray(vm, &list->values, lst->values.objects[i]);
                }
                push(vm, AS_OBJECT(list));
                break;
            }
            else
            {
                start = IS_NIL(sta) ? -1 : start;
                for (int i = end; i > start; i += step)
                {
                    writeMyMoObjectArray(vm, &list->values, lst->values.objects[i]);
                }
                push(vm, AS_OBJECT(list));
                break;
            }
        }
        case OBJ_TUPLE:
        {
            MyMoTuple *tpl = AS_TUPLE(object);
            MyMoTuple *tuple = newTuple(vm);
            int end = IS_NIL(ed) ? (step < 0 ? tpl->values.count - 1 : tpl->values.count) : AS_INT(ed)->value;
            if (start < 0)
            {
                start += tpl->values.count;
            }
            if (end < 0)
            {
                end += tpl->values.count;
            }
            else if (end > tpl->values.count)
            {
                end = step < 0 ? tpl->values.count - 1 : tpl->values.count;
            }

            if (start > end || (start >= tpl->values.count && end > tpl->values.count) || start >= tpl->values.count)
            {
                push(vm, AS_OBJECT(tuple));
                break;
            }
            if (step > 0)
            {
                for (int i = start; i < end; i += step)
                {
                    writeMyMoObjectArray(vm, &tuple->values, tpl->values.objects[i]);
                }
                push(vm, AS_OBJECT(tuple));
                break;
            }
            else
            {
                start = IS_NIL(sta) ? -1 : start;
                for (int i = end; i > start; i += step)
                {
                    writeMyMoObjectArray(vm, &tuple->values, tpl->values.objects[i]);
                }
                push(vm, AS_OBJECT(tuple));
                break;
            }
        }
        default:
            runtimeError(vm, "TypeError: can only slice on List and String but got %s", getType(object));
            return RUNTIME_ERROR;
            break;
        }
        DISPATCH();
    }
    OP_NOT:
    {
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, NULL, "!");
        }
        push(vm, NEW_BOOL(isFalsey(a)));
        DISPATCH();
    }
    OP_DUP:
    {
        push(vm, peek(vm, 0));
        DISPATCH();
    }
    OP_POP:
    {
        pop(vm);
        DISPATCH();
    }
    OP_EQUAL:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            MyMoObject *method = getMethod(vm, a, inplace ? "!=" : "==");
            if (IS_EMPTY(method))
            {
                push(vm, NEW_BOOL(isEqual(a, b)));
                DISPATCH();
            }
            if (inplace)
            {
                UNUSED(ReadByte());
            }
            push(vm, method);
            push(vm, a);
            push(vm, b);
            if (!caller(vm, method, 2))
            {
                return RUNTIME_ERROR;
            }
            frame = vm->fiber->callFrames[vm->fiber->frameCount];
            DISPATCH();
        }
        push(vm, NEW_BOOL(isEqual(a, b)));
        DISPATCH();
    }
    OP_GREATER:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            if (inplace)
            {
                UNUSED(ReadByte());
            }
            OperatorOverLoad(a, b, inplace ? "<=" : ">");
        }
        if (!IS_NUMBER(a) || !IS_NUMBER(b))
        {
            runtimeError(vm, "Operands must be numbers.");
            return RUNTIME_ERROR;
        }
        push(vm, NEW_BOOL(NUMBER_VAL(a) > NUMBER_VAL(b)));
        DISPATCH();
    }
    OP_LESS:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            if (inplace)
            {
                UNUSED(ReadByte());
            }

            OperatorOverLoad(a, b, inplace ? ">=" : "<");
        }
        if (!IS_NUMBER(a) || !IS_NUMBER(b))
        {
            runtimeError(vm, "Operands must be numbers.");
            return RUNTIME_ERROR;
        }
        push(vm, NEW_BOOL(NUMBER_VAL(a) < NUMBER_VAL(b)));
        DISPATCH();
    }
    OP_BAND:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "&=" : "&");
        }
        BitwiseOp(a, b, &);
        DISPATCH();
    }
    OP_BOR:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "|=" : "|");
        }
        BitwiseOp(a, b, |);
        DISPATCH();
    }
    OP_BXOR:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "^=" : "^");
        }
        BitwiseOp(a, b, ^);
        DISPATCH();
    }
    OP_ADD:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {

            OperatorOverLoad(a, b, inplace ? "+=" : "+");
        }
        MyMoObject *result = addition(vm, a, b);
        if (IS_EMPTY(result))
            return RUNTIME_ERROR;
        push(vm, result);
        DISPATCH();
    }
    OP_SUB:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "-=" : "-");
        }
        MyMoObject *result = subtraction(vm, a, b);
        if (IS_EMPTY(result))
            return RUNTIME_ERROR;
        push(vm, result);
        DISPATCH();
    }
    OP_MUL:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "*=" : "*");
        }
        MyMoObject *result = multiplication(vm, a, b);
        if (IS_EMPTY(result))
            return RUNTIME_ERROR;
        push(vm, result);
        DISPATCH();
    }
    OP_DIV:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "/=" : "/");
        }
        MyMoObject *result = division(vm, a, b);
        if (IS_EMPTY(result))
            return RUNTIME_ERROR;
        push(vm, result);
        DISPATCH();
    }
    OP_POW:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "**=" : "**");
        }
        if (!IS_NUMBER(a) || !IS_NUMBER(b))
        {
            runtimeError(vm, "Operands must be numbers.");
            return RUNTIME_ERROR;
        }
        double result = pow(NUMBER_VAL(a), NUMBER_VAL(b));
        // if (isInteger(result))
        // {
        //     push(vm, NEW_INT(vm, (long)result));
        // }
        // else
        {
            push(vm, NEW_DOUBLE(vm, result));
        }
        DISPATCH();
    }
    OP_MOD:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "%=" : "%");
        }
        if (!IS_NUMBER(a) || !IS_NUMBER(b))
        {
            runtimeError(vm, "Operands must be numbers.");
            return RUNTIME_ERROR;
        }
        double result = fmod(NUMBER_VAL(a), NUMBER_VAL(b));
        // if (isInteger(result))
        // {
        //     push(vm, NEW_INT(vm, (int)result));
        // }
        // else
        {
            push(vm, NEW_DOUBLE(vm, result));
        }
        DISPATCH();
    }
    OP_LSFT:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "<<=" : "<<");
        }
        BitwiseOp(a, b, <<);
        DISPATCH();
    }
    OP_RSFT:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? ">>=" : ">>");
        }
        BitwiseOp(a, b, >>);
        DISPATCH();
    }
    OP_IDIV:
    {
        u32 inplace = ReadByte();
        MyMoObject *b = pop(vm);
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, b, inplace ? "//=" : "//");
        }
        if (!IS_NUMBER(a) || !IS_NUMBER(b))
        {
            runtimeError(vm, "Operands must be numbers.");
            return RUNTIME_ERROR;
        }
        long int r = NUMBER_VAL(a) / NUMBER_VAL(b);
        push(vm, NEW_INT(vm, r));
        DISPATCH();
    }
    OP_NEG:
    {
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, NULL, "-@");
        }
        UnaryOp(-, a);
        DISPATCH();
    }
    OP_POS:
    {
        MyMoObject *a = pop(vm);
        if (IS_INSTANCE(a))
        {
            OperatorOverLoad(a, NULL, "+@");
        }
        UnaryOp(+, a);
        DISPATCH();
    }
    OP_JIF:
    {
        u16 offset = ReadShort();
        if (isFalsey(peek(vm, 0)))
        {
            frame->ip += offset;
        }
        DISPATCH();
    }
    OP_JMP:
    {
        u16 offset = ReadShort();
        frame->ip = frame->ip + offset;
        DISPATCH();
    }
    OP_CJMP:
    {
        u16 offset = ReadShort();
        MyMoObject *lhs = pop(vm);
        if (!isEqual(lhs, peek(vm, 0)))
        {
            frame->ip += offset;
        }
        else
        {
            pop(vm);
        }
        DISPATCH();
    }
    OP_MCASE:
    {
        int count = ReadByte();
        MyMoObject *compare = peek(vm, count + 1);
        MyMoObject *match = pop(vm);
        for (int i = 0; i < count; ++i)
        {
            if (isEqual(compare, match))
            {
                i++;
                while (i <= count)
                {
                    pop(vm);
                    i++;
                }
                break;
            }
            match = pop(vm);
        }
        push(vm, match);
        DISPATCH();
    }
    OP_LOOP:
    {
        u16 offset = ReadShort();
        frame->ip -= offset;
        DISPATCH();
    }
    OP_ITER:
    {
        u16 offset = ReadShort();
        MyMoIter *iterator = AS_ITER(peek(vm, 0));
        MyMoObject *object = nextIter(vm, iterator);
        if (IS_EMPTY(object))
        {
            frame->ip = frame->ip + offset;
        }
        else
        {
            push(vm, object);
        }
        DISPATCH();
    }
    OP_GETI:
    {
        MyMoObject *iterator = pop(vm);
        switch (iterator->type)
        {
        case OBJ_STRING:
        case OBJ_LIST:
        case OBJ_TUPLE:
            // case OBJ_DICT:
            // case OBJ_INSTANCE: TODO
            {
                push(vm, AS_OBJECT(newIter(vm, iterator)));
                DISPATCH();
            }
        default:
        {
            runtimeError(vm, "TypeError: cannot iterate on %s.", getType(iterator));
            return RUNTIME_ERROR;
        }
        }
    }
    OP_GETV:
    {
        MyMoObject *variable = ReadObject();
        MyMoObject *value = NULL;
        if (vm->currentClass)
        {
            value = getEntry(vm->currentClass->variables, variable);
            if (value)
            {
                push(vm, value);
                DISPATCH();
            }
            value = getEntry(vm->currentClass->methods, variable);
            if (value)
            {
                push(vm, value);
                DISPATCH();
            }
        }
        if ((IS_FIBER_ROOT(vm->fiber)) && vm->fiber->frameCount == 0)
        {
            value = getEntry(&vm->globals, variable);
            if (value)
            {
                push(vm, value);
                DISPATCH();
            }
            goto builtinvars;
        }
        else
        {
            value = getEntry(&frame->locals, variable);
            if (value)
            {
                push(vm, value);
                DISPATCH();
            }
            CallFrame *parent = frame->function->frame;
            while (parent)
            {
                value = getEntry(&parent->locals, variable);
                if (value)
                {
                    push(vm, value);
                    setEntry(vm, &frame->locals, variable, value);
                    DISPATCH();
                }
                parent = parent->function->frame;
            }
        }
        value = getEntry(&vm->globals, variable);
        if (value)
        {
            push(vm, value);
            DISPATCH();
        }
        else
        {
        builtinvars:
            value = getEntry(&vm->builtins, variable);
            if (value)
            {
                push(vm, value);
                DISPATCH();
            }
        }
        runtimeError(vm, "Name Error: Undefined variable '%s'.", STRING_VAL(variable));
        return RUNTIME_ERROR;
    }
    OP_SETV:
    {
        MyMoObject *variable = ReadObject();
        if (vm->currentClass)
        {
            setEntry(vm, vm->currentClass->variables, variable, peek(vm, 0));
        }
        else if (!vm->fiber->parent && vm->fiber->frameCount == 0)
        {
            setEntry(vm, &vm->globals, variable, peek(vm, 0));
        }
        else
        {
            setEntry(vm, &frame->locals, variable, peek(vm, 0));
        }
        DISPATCH();
    }
    OP_DELV:
    {
        MyMoObject *variable = ReadObject();
        if (deleteEntry(vm, &frame->locals, variable))
        {
            DISPATCH();
        }
        runtimeError(vm, "Name Error: Undefined variable '%s'.", AS_STRING(variable)->value);
        return RUNTIME_ERROR;
    }
    OP_MET:
    {
        MyMoObject *method = ReadObject();
        MyMoObject *name = ReadObject();
        MyMoClass *klass = AS_CLASS(peek(vm, 0));
        if (memcmp(AS_STRING(name)->value, "__init__", 8) == 0)
        {
            klass->init = method;
        }
        setEntry(vm, klass->methods, name, method);
        AS_FUNCTION(method)->klass = AS_OBJECT(klass);
        DISPATCH();
    }
    OP_FN:
    {
        MyMoObject *function = ReadObject();
        AS_FUNCTION(function)->frame = frame;
        push(vm, function);
        DISPATCH();
    }
    OP_CALL:
    {
        u8 argCount = ReadByte();
        if (!caller(vm, peek(vm, argCount), argCount))
            return RUNTIME_ERROR;
        frame = vm->fiber->callFrames[vm->fiber->frameCount];
        DISPATCH();
    }
    OP_PITHRU:
    {
        MyMoObject *fn = pop(vm);
        if (!(IS_FUNCTION(fn) || IS_CLASS(fn) || IS_BUILTIN_FUNCTION(fn) || IS_BUILTIN_METHOD(fn) || IS_BOUND_METHOD(fn) || IS_BUILTIN_CLASS(fn)))
        {
            runtimeError(vm, "Type Error: Cannot Pipe Through '%s'.", getType(fn));
            return RUNTIME_ERROR;
        }
        MyMoObject *arg = pop(vm);
        push(vm, fn);
        push(vm, arg);
        DISPATCH();
    }
    OP_RET:
    {
        if (vm->currentModule && vm->currentModule->parent)
        {
            copyDict(vm, &frame->locals, vm->currentModule->variables);
            freeDict(vm, &frame->locals);
            free(frame);
            vm->currentModule = vm->currentModule->parent;
            frame = vm->fiber->callFrames[--vm->fiber->frameCount];
            DISPATCH();
        }
        return OK;
    }
    OP_FRET:
    {
        MyMoObject *ret = pop(vm);
        if (vm->classCall && frame->function->type == FN_INIT)
        {
            vm->classCall--;
            ret = getEntry(&frame->locals, AS_OBJECT(frame->function->argv[0]));
        }
        // // disassembleChunk(frame->function->chunk,"---");
        if (vm->currentClass || frame->function->type > FN_METHOD)
        {
            pop(vm);
        }
        if (vm->fiber->frameCount == 0)
        {
            // resetStack(vm);
            vm->fiber->state = FIBER_DEAD;
            vm->fiber = vm->fiber->parent;
            pop(vm);
            push(vm, ret);
            frame = vm->fiber->callFrames[vm->fiber->frameCount];
            DISPATCH();
        }
        freeDict(vm, &frame->locals);
        free(frame);
        frame = vm->fiber->callFrames[--vm->fiber->frameCount];
        push(vm, ret);
        DISPATCH();
    }
    OP_CLASS:
    {
        MyMoString *name = AS_STRING(ReadObject());
        MyMoClass *klass = newClass(vm, name);
        klass->enclosing = vm->currentClass;
        vm->currentClass = klass;
        push(vm, AS_OBJECT(klass));
        DISPATCH();
    }
    OP_SUPERARGS:
    {
        int argc = INT_VAL(ReadConstant());
        for (size_t i = 0; i < argc; i++)
        {
            MyMoObject *superClass = pop(vm);
            if (IS_CLASS(superClass) || IS_BUILTIN_CLASS(superClass))
            {
                writeMyMoObjectArray(vm, &vm->currentClass->superClasses, superClass);
                if (IS_CLASS(superClass))
                {
                    copyDict(vm, AS_CLASS(superClass)->methods, vm->currentClass->methods);
                    copyDict(vm, AS_CLASS(superClass)->fields, vm->currentClass->fields);
                    copyDict(vm, AS_CLASS(superClass)->variables, vm->currentClass->variables);
                    vm->currentClass->init = AS_CLASS(superClass)->init;
                }
                else
                {
                    copyDict(vm, AS_BUILTIN_CLASS(superClass)->methods, vm->currentClass->methods);
                }
            }
            else
            {
                runtimeError(vm, "TypeError: only class can be inherated");
            }
        }
        DISPATCH();
    }
    OP_ENDCLASS:
    {
        vm->currentClass = vm->currentClass->enclosing;
        DISPATCH();
    }
    OP_SETP:
    {
        if (IS_INSTANCE(peek(vm, 1)))
        {
            MyMoInstance *instance = AS_INSTANCE(peek(vm, 1));
            setEntry(vm, instance->fields, ReadObject(), peek(vm, 0));
            MyMoObject *value = pop(vm);
            pop(vm);
            push(vm, value);
            DISPATCH();
        }
        else if (IS_CLASS(peek(vm, 1)))
        {
            MyMoClass *klass = AS_CLASS(peek(vm, 1));
            setEntry(vm, klass->variables, ReadObject(), peek(vm, 0));
            MyMoObject *value = pop(vm);
            pop(vm);
            push(vm, value);
            DISPATCH();
        }
        else if (IS_MODULE(peek(vm, 1)))
        {
            MyMoModule *module = AS_MODULE(peek(vm, 1));
            setEntry(vm, module->variables, ReadObject(), peek(vm, 0));
            MyMoObject *value = pop(vm);
            pop(vm);
            push(vm, value);
            DISPATCH();
        }
        else if (IS_BUILTIN_CLASS(peek(vm, 1)))
        {
            runtimeError(vm, "TypeError: can't set attributes of built-in/extension type 'object'");
            return RUNTIME_ERROR;
        }
        runtimeError(vm, "TypeError: Only classes and instances have properties.");
        return RUNTIME_ERROR;
    }
    OP_AGETP:
    {
        agp = 1;
    }
    OP_GETP:
    {
        MyMoObjectType type = peek(vm, 0)->type;
        switch (type)
        {
        case OBJ_INSTANCE:
        {
            MyMoInstance *instance = AS_INSTANCE(peek(vm, 0));
            MyMoObject *variable = ReadObject();
            MyMoObject *value = getEntry(instance->fields, variable);
            if (value)
            {
                if (!agp)
                    pop(vm); // pop the instance
                push(vm, value);
                if (agp)
                    agp = 0;
                DISPATCH();
            }
            value = getEntry(instance->klass->variables, variable);
            if (value)
            {
                if (!agp)
                    pop(vm); // pop the instance
                push(vm, value);
                if (agp)
                    agp = 0;
                DISPATCH();
            }
            value = getEntry(instance->klass->methods, variable);
            if (value)
            {
                if (IS_FUNCTION(value))
                {
                    MyMoBoundMethod *bound = newBoundMethod(vm, peek(vm, 0), AS_FUNCTION(value));
                    setEntry(vm, instance->fields, variable, AS_OBJECT(bound));
                    value = AS_OBJECT(bound);
                }
                if (!agp)
                    pop(vm); // pop the instance
                push(vm, value);
                if (agp)
                    agp = 0;
                DISPATCH();
            }
            value = getEntry(vm->builtInClasses[OBJ_OBJECT]->methods, variable);
            if (value)
            {
                pop(vm); // pop the instance
                push(vm, value);
                if (agp)
                {
                    runtimeError(vm, "TypeError: can't set attributes of built-in/extension type 'object'");
                    return RUNTIME_ERROR;
                }
                DISPATCH();
            }
            runtimeError(vm, "Undefined property '%s'.", STRING_VAL(variable));
            return RUNTIME_ERROR;
        }
        case OBJ_CLASS:
        {
            MyMoClass *klass = AS_CLASS(peek(vm, 0));
            MyMoObject *variable = ReadObject();
            MyMoObject *value = getEntry(klass->variables, variable);
            if (value)
            {
                if (!agp)
                    pop(vm); // pop the class
                if (agp)
                    agp = 0;
                push(vm, value);
                DISPATCH();
            }
            value = getEntry(klass->methods, variable);
            if (value)
            {
                if (!agp)
                    pop(vm); // pop the class
                if (agp)
                    agp = 0;
                push(vm, value);
                DISPATCH();
            }
            value = getEntry(vm->builtInClasses[OBJ_OBJECT]->methods, variable);
            if (value)
            {
                pop(vm); // pop the instance
                push(vm, value);
                if (agp)
                {
                    runtimeError(vm, "TypeError: can't set attributes of built-in/extension type 'object'");
                }
                DISPATCH();
            }
            runtimeError(vm, "Undefined property '%s'.", STRING_VAL(variable));
            return RUNTIME_ERROR;
        }
        case OBJ_MODULE:
        {
            MyMoModule *module = AS_MODULE(peek(vm, 0));
            MyMoObject *variable = ReadObject();
            MyMoObject *value = getEntry(module->variables, variable);
            if (value)
            {
                if (!agp)
                    pop(vm); // pop the module
                if (agp)
                    agp = 0;
                push(vm, value);
                DISPATCH();
            }
            runtimeError(vm, "AttributeError: module '%s' has no attribute '%s'.", module->name->value, STRING_VAL(variable));
            return RUNTIME_ERROR;
        }
        case OBJ_BUILTIN_CLASS:
        {
            MyMoBuiltInClass *klass = AS_BUILTIN_CLASS(peek(vm, 0));
            MyMoObject *variable = ReadObject();
            MyMoObject *value = getEntry(klass->methods, variable);
            if (value)
            {
                if (!agp)
                    pop(vm); // pop the class
                if (agp)
                    agp = 0;
                push(vm, value);
                DISPATCH();
            }
            runtimeError(vm, "AttributeError: built-in/extension type '%s' has no attribute '%s'.", klass->name->value, STRING_VAL(variable));
            return RUNTIME_ERROR;
        }
        default:
        {
            MyMoObject *self = pop(vm);
            MyMoObject *variable = ReadObject();
            MyMoObject *fn = getEntry(vm->builtInClasses[type]->methods, variable);
            if (fn)
            {
                MyMoBuiltInFunction *function = AS_BUILTIN_FUNCTION(fn);
                function->self = self;
                push(vm, fn);
                DISPATCH();
            }
            runtimeError(vm, "AttributeError: %s has no attribute '%s'.", getType(self), STRING_VAL(variable));
            return RUNTIME_ERROR;
        }
        }
    }
    OP_DELP:
        DISPATCH();
    OP_USE:
    {
        MyMoString *modulePathUse = AS_STRING(ReadObject());
        MyMoObject *isUse = pop(vm);
        MyMoString *moduleName = modulePathUse;
        char *path = pathResolver(vm, modulePathUse->value);
        if (path == NULL)
        {
            MyMoObject *module = getEntry(&vm->builtInModules, AS_OBJECT(modulePathUse));
            if (module)
            {
                push(vm, module);
                if (AS_BOOL(isUse)->value)
                {
                    push(vm, AS_OBJECT(moduleName));
                }
                DISPATCH();
            }
            module = loadBuiltInModule(vm, modulePathUse);
            if (IS_EMPTY(module))
            {
                runtimeError(vm, "Module '%s' not found.", modulePathUse->value);
                return RUNTIME_ERROR;
            }
            push(vm, module);
            if (AS_BOOL(isUse)->value)
            {
                push(vm, AS_OBJECT(moduleName));
            }
            DISPATCH();
        }
        char *name = strrchr(modulePathUse->value, '/');
        if (name)
        {
            name++;
            moduleName = newString(vm, name, strlen(name));
        }
        MyMoString *modulePath;
        if (path[strlen(path) - 1] == 'c')
        {
            modulePath = newString(vm, path, strlen(path) - 1);
        }
        else
        {
            modulePath = newString(vm, path, strlen(path));
        }
        MyMoObject *module = getEntry(&vm->modules, AS_OBJECT(modulePath));
        if (module)
        {
            push(vm, module);
            if (AS_BOOL(isUse)->value)
            {
                push(vm, AS_OBJECT(moduleName));
            }
            free(path);
            DISPATCH();
        }
        MyMoFunction *function = runFile(vm, path);
        if (function == NULL)
        {
            runtimeError(vm, "Syntax error in module '%s'.", path);
            free(path);
            return RUNTIME_ERROR;
        }
        free(path);
        function->name = moduleName;
        function->type = FN_MODULE;
        callFunction(vm, function, 0);
        frame = vm->fiber->callFrames[vm->fiber->frameCount];
        setEntry(vm, &frame->locals, NEW_STRING(vm, "__name__", 8), AS_OBJECT(moduleName));
        MyMoModule *currentModule = newModule(vm, moduleName, modulePath);
        currentModule->parent = vm->currentModule;
        vm->currentModule = currentModule;
        module = AS_OBJECT(currentModule);
        push(vm, module);
        if (AS_BOOL(isUse)->value)
        {
            push(vm, AS_OBJECT(AS_MODULE(module)->name));
        }
        DISPATCH();
    }
    OP_SETM:
    {
        MyMoObject *name = pop(vm);
        MyMoObject *value = pop(vm);
        if ((IS_FIBER_ROOT(vm->fiber)) && vm->fiber->frameCount == 0)
        {
            setEntry(vm, &vm->globals, name, value);
        }
        else
        {
            setEntry(vm, &frame->locals, name, value);
        }
        DISPATCH();
    }
    OP_COPY:
    {
        MyMoModule *module = AS_MODULE(pop(vm));
        copyDict(vm, module->variables, (((IS_FIBER_ROOT(vm->fiber)) && vm->fiber->frameCount == 0) ? &vm->globals : &frame->locals));
        DISPATCH();
    }
    }

#undef ReadByte
#undef ReadConstant
#undef DISPATCH
#undef NUMBER_VAL
#undef BitwiseOp
#undef UnaryOp
#undef OperatorOverLoad
    return OK;
}

I_Result interpreter(MVM *vm, MyMoFunction *main_)
{
    if (main_ == NULL)
        return COMPILE_ERROR;
    vm->fiber->callFrames[0]->function = main_;
    vm->fiber->callFrames[0]->ip = main_->chunk->code;
    // set __name__ to __main__
    MyMoObject *name = AS_OBJECT(newString(vm, "__name__", 8));
    MyMoObject *main = AS_OBJECT(newString(vm, "__main__", 8));
    setEntry(vm, &vm->globals, name, main);
    I_Result i = runMVM(vm);
    // free(frame);
    return i;
}
