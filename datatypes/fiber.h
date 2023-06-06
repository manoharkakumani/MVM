#ifndef __FIBER_H__
#define __FIBER_H__

#include "object.h"
#include "function.h"
#include "dict.h"

#define AS_FIBER(object) ((MyMoFiber *)object)
#define IS_FIBER(object) (object->type == OBJ_FIBER)
#define IS_FIBER_RUNNING(fiber) (fiber->state == FIBER_RUNNING)
#define IS_FIBER_READY(fiber) (fiber->state == FIBER_READY)
#define IS_FIBER_SUSPENDED(fiber) (fiber->state == FIBER_SUSPENDED)
#define IS_FIBER_DEAD(fiber) (fiber->state == FIBER_DEAD)
#define IS_FIBER_CHILD(fiber) (fiber->type == FIBER_CHILD)
#define IS_FIBER_ROOT(fiber) (fiber->type == FIBER_ROOT)

#define NEW_FIBER(vm, value) AS_OBJECT(newFiber(vm, value))

typedef struct CallFrame
{
    MyMoFunction *function;
    MyMoDict locals;
    u8 *ip;
    struct CallFrame *parent;
} CallFrame;

typedef enum
{
    FIBER_READY,
    FIBER_RUNNING,
    FIBER_YIELD,
    FIBER_DEAD
} FiberState;

typedef enum
{
    FIBER_ROOT,
    FIBER_CHILD
} FiberType;

typedef struct fiber
{
    MyMoObject object;
    FiberState state;
    FiberType type;
    MyMoObjectArray stack;
    CallFrame **callFrames;
    uint frameCount;
    uint frameCapacity;
    struct fiber *parent;
} MyMoFiber;

MyMoFiber *newFiber(MVM *vm, MyMoFunction *function);
void printFiber(MyMoFiber *fiber);
void freeFiber(MVM *vm, MyMoFiber *fiber);

void defineFiberClass(MVM *vm);

#endif
