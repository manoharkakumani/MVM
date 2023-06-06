#ifndef __DICT_H__
#define __DICT_H__

#include "object.h"
#include "string.h"
#include "int.h"
#include "double.h"

#define NEW_DICT(vm) AS_OBJECT(newDict(vm))
#define AS_DICT(object) ((MyMoDict *)object)
#define IS_DICT(object) (object->type == OBJ_DICT)

typedef struct Entry
{
    struct Entry *prev;
    struct Entry *next;
    MyMoObject *key;
    MyMoObject *value;
    int index;
} Entry;

typedef struct MyMoDict
{
    MyMoObject object;
    int count;
    int capacity;
    Entry *entries;
} MyMoDict;

MyMoDict *newDict(MVM *vm);
void freeDictionary(MVM *vm, MyMoDict *dict);

void initDict(MyMoDict *dict);
void copyDict(MVM *vm, MyMoDict *from, MyMoDict *to);
void printDict(MyMoDict *dict);
void freeDict(MVM *vm, MyMoDict *dict);

MyMoObject *getEntry(MyMoDict *dict, MyMoObject *key);
bool setEntry(MVM *vm, MyMoDict *dict, MyMoObject *key, MyMoObject *value);
bool deleteEntry(MVM *vm, MyMoDict *dict, MyMoObject *key);

void setPrimitive(MVM *vm, MyMoDict *dict, MyMoObject *key);

MyMoString *findString(MyMoDict *dict, const char *chars, int length, u32 hash);

MyMoInt *findInt(MyMoDict *dict, long value, int length, u32 hash);

MyMoDouble *findDouble(MyMoDict *dict, double value, int length, u32 hash);

u32 hasher(char *key, size_t len);

#endif
