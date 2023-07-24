#include "operations.h"
#include "memory.h"
#include "vm.h"
#include "datatypes/datatypes.h"

MyMoObject *addition(MVM *vm, MyMoObject *a, MyMoObject *b)
{
    if (IS_BOOL(a) && IS_BOOL(b))
    {
        bool a_val = BOOL_VAL(a);
        bool b_val = BOOL_VAL(b);
        return NEW_BOOL(a_val || b_val);
    }
    if (IS_BOOL(a))
    {
        a = NEW_INT(vm, BOOL_VAL(a));
    }
    if (IS_BOOL(b))
    {
        b = NEW_INT(vm, BOOL_VAL(b));
    }
    if (IS_INT(a) && IS_INT(b))
    {
        long a_val = INT_VAL(a);
        long b_val = INT_VAL(b);
        return NEW_INT(vm, a_val + b_val);
    }
    else if (IS_DOUBLE(a) && IS_DOUBLE(b))
    {
        double a_val = DOUBLE_VAL(a);
        double b_val = DOUBLE_VAL(b);
        return NEW_DOUBLE(vm, a_val + b_val);
    }
    else if ((IS_DOUBLE(a) && IS_INT(b)) || (IS_INT(a) && IS_DOUBLE(b)))
    {
        double a_val = IS_DOUBLE(a) ? DOUBLE_VAL(a) : (double)INT_VAL(a);
        double b_val = IS_DOUBLE(b) ? DOUBLE_VAL(b) : (double)INT_VAL(b);
        return NEW_DOUBLE(vm, a_val + b_val);
    }
    else if (IS_STRING(a) && IS_STRING(b))
    {
        MyMoString *a_val = AS_STRING(a);
        MyMoString *b_val = AS_STRING(b);
        int length = a_val->length + b_val->length;
        char *chars = New(char, length + 1);
        memcpy(chars, a_val->value, a_val->length);
        memcpy(chars + a_val->length, b_val->value, b_val->length);
        chars[length] = '\0';
        MyMoObject *result = NEW_STRING(vm, chars, length);
        Free(vm,char,chars);
        return result;
    }
    else
    {
        runtimeError(vm, "TypeError: can't perform + between  %s and %s", getType(a), getType(b));
        return NEW_EMPTY;
    }
}

MyMoObject *subtraction(MVM *vm, MyMoObject *a, MyMoObject *b)
{
    if (IS_BOOL(a) && IS_BOOL(b))
    {
        bool a_val = BOOL_VAL(a);
        bool b_val = BOOL_VAL(b);
        return NEW_BOOL(a_val && !b_val);
    }
    if (IS_BOOL(a))
    {
        a = NEW_INT(vm, BOOL_VAL(a));
    }
    if (IS_BOOL(b))
    {
        b = NEW_INT(vm, BOOL_VAL(b));
    }
    if (IS_INT(a) && IS_INT(b))
    {
        int a_val = INT_VAL(a);
        int b_val = INT_VAL(b);
        return NEW_INT(vm, a_val - b_val);
    }
    else if (IS_DOUBLE(a) && IS_DOUBLE(b))
    {
        double a_val = DOUBLE_VAL(a);
        double b_val = DOUBLE_VAL(b);
        return NEW_DOUBLE(vm, a_val - b_val);
    }
    else if ((IS_DOUBLE(a) && IS_INT(b)) || (IS_INT(a) && IS_DOUBLE(b)))
    {
        double a_val = IS_DOUBLE(a) ? DOUBLE_VAL(a) : (double)INT_VAL(a);
        double b_val = IS_DOUBLE(b) ? DOUBLE_VAL(b) : (double)INT_VAL(b);
        return NEW_DOUBLE(vm, a_val - b_val);
    }
    else
    {
        runtimeError(vm, "TypeError: can't perform - between  %s and %s", getType(a), getType(b));
        return NEW_EMPTY;
    }
}

MyMoObject *stringMultiple(MVM *vm, MyMoString *a, double b)
{
    int length = a->length * b;
    char *chars = New(char, length + 1);
    for (int i = 0; i < length; i++)
    {
        chars[i] = a->value[i % a->length];
    }
    chars[length] = '\0';
    MyMoObject *result = NEW_STRING(vm, chars, length);
    Free(vm,char,chars);
    return result;
}

MyMoObject *multiplication(MVM *vm, MyMoObject *a, MyMoObject *b)
{
    if (IS_BOOL(a) && IS_BOOL(b))
    {
        bool a_val = BOOL_VAL(a);
        bool b_val = BOOL_VAL(b);
        return NEW_BOOL(a_val && b_val);
    }
    if (IS_BOOL(a))
    {
        a = NEW_INT(vm, BOOL_VAL(a));
    }
    if (IS_BOOL(b))
    {
        b = NEW_INT(vm, BOOL_VAL(b));
    }
    if (IS_INT(a) && IS_INT(b))
    {
        int a_val = INT_VAL(a);
        int b_val = INT_VAL(b);
        return NEW_INT(vm, a_val * b_val);
    }
    else if (IS_DOUBLE(a) && IS_DOUBLE(b))
    {
        double a_val = DOUBLE_VAL(a);
        double b_val = DOUBLE_VAL(b);
        return NEW_DOUBLE(vm, a_val * b_val);
    }
    else if ((IS_DOUBLE(a) && IS_INT(b)) || (IS_INT(a) && IS_DOUBLE(b)))
    {
        double a_val = IS_DOUBLE(a) ? DOUBLE_VAL(a) : (double)INT_VAL(a);
        double b_val = IS_DOUBLE(b) ? DOUBLE_VAL(b) : (double)INT_VAL(b);
        return NEW_DOUBLE(vm, a_val * b_val);
    }
    else if (IS_STRING(a) && IS_INT(b))
    {
        return stringMultiple(vm, AS_STRING(a), INT_VAL(b));
    }
    else if (IS_INT(a) && IS_STRING(b))
    {
        return stringMultiple(vm, AS_STRING(b), INT_VAL(a));
    }
    else
    {
        runtimeError(vm, "TypeError: can't perform * between  %s and %s", getType(a), getType(b));
        return NEW_EMPTY;
    }
}

MyMoObject *division(MVM *vm, MyMoObject *a, MyMoObject *b)
{
    if (IS_BOOL(a) && IS_BOOL(b))
    {
        bool a_val = BOOL_VAL(a);
        bool b_val = BOOL_VAL(b);
        return NEW_BOOL(a_val && b_val);
    }
    if (IS_BOOL(a))
    {
        a = NEW_INT(vm, BOOL_VAL(a));
    }
    if (IS_BOOL(b))
    {
        b = NEW_INT(vm, BOOL_VAL(b));
    }
    if (IS_INT(a) && IS_INT(b))
    {
        long a_val = INT_VAL(a);
        double b_val = INT_VAL(b);
        double result = a_val / b_val;
        if (result == (int) result)
        {
            return NEW_INT(vm, (long)result);
        }
        else
        {
            return NEW_DOUBLE(vm, result);
        }
    }
    else if (IS_DOUBLE(a) && IS_DOUBLE(b))
    {
        double a_val = DOUBLE_VAL(a);
        double b_val = DOUBLE_VAL(b);
        return NEW_DOUBLE(vm, a_val / b_val);
    }
    else if ((IS_DOUBLE(a) && IS_INT(b)) || (IS_INT(a) && IS_DOUBLE(b)))
    {
        double a_val = IS_DOUBLE(a) ? DOUBLE_VAL(a) : (double)INT_VAL(a);
        double b_val = IS_DOUBLE(b) ? DOUBLE_VAL(b) : (double)INT_VAL(b);
        return NEW_DOUBLE(vm, a_val / b_val);
    }
    else
    {
        runtimeError(vm, "TypeError: can't perform / between  %s and %s", getType(a), getType(b));
        return NEW_EMPTY;
    }
}
