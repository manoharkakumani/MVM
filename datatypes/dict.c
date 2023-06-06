#include "memory.h"
#include "dict.h"
#include "../vm.h"
#include "nil.h"
#include "bool.h"
#include "string.h"

#define TABLE_MAX_LOAD 0.75

MyMoDict *newDict(MVM *vm)
{
    MyMoDict *dict = AllocateObject(vm, MyMoDict, OBJ_DICT);
    initDict(dict);
    return dict;
}

void initDict(MyMoDict *dict)
{
    dict->count = 0;
    dict->capacity = -1;
    dict->entries = NULL;
    dict->object.type = OBJ_DICT;
}

void freeDict(MVM *vm, MyMoDict *dict)
{
    FreeArray(vm, Entry, dict->entries, dict->count);
    initDict(dict);
}

void freeDictionary(MVM *vm, MyMoDict *dict)
{
    FreeArray(vm, Entry, dict->entries, dict->count);
    Free(vm, MyMoDict, dict);
}

u32 hasher(char *key, size_t len)
{
    u32 hash = 2166136261u;
    for (size_t i = 0; i < len; i++)
    {
        hash ^= (u8)key[i];
        hash *= 16777619;
    }
    return hash;
}

Entry *findEntry(Entry *entries, int capacity, MyMoObject *key)
{
    u32 index = key->hash & capacity;
    Entry *tombstone = NULL;
    for (;;)
    {
        Entry *entry = &entries[index];
        if (entry->key == NULL)
        {
            if (IS_NIL(entry->value))
            {
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                if (tombstone == NULL)
                    tombstone = entry;
            }
        }
        else if (entry->key == key && entry->key->hash == key->hash)
        {
            return entry;
        }
        index = (index + 1) & capacity;
    }
}

MyMoObject *getEntry(MyMoDict *dict, MyMoObject *key)
{
    // printObject(key);
    // printf("\n");
    if (dict->count == 0)
        return NULL;
    Entry *entry = findEntry(dict->entries, dict->capacity, key);
    if (entry->key == NULL)
        return NULL;
    return entry->value;
}

void adjustCapacity(MVM *vm, MyMoDict *dict, int capacity)
{
    Entry *entries = Allocate(vm, Entry, capacity + 1);
    for (int i = 0; i <= capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NEW_NIL;
    }
    Entry *oldEntries = dict->entries;
    int oldCapacity = dict->capacity;
    dict->count = 0;
    dict->entries = entries;
    dict->capacity = capacity;
    for (int i = 0; i <= oldCapacity; i++)
    {
        Entry *entry = &oldEntries[i];
        if (entry->key == NULL)
            continue;
        setEntry(vm, dict, entry->key, entry->value);
    }
    FreeArray(vm, Entry, oldEntries, oldCapacity + 1);
}

bool setEntry(MVM *vm, MyMoDict *dict, MyMoObject *key, MyMoObject *value)
{
    if (dict->count + 1 > (dict->capacity + 1) * TABLE_MAX_LOAD)
    {
        int capacity = ResizeCapacity(dict->capacity + 1) - 1;
        adjustCapacity(vm, dict, capacity);
    }
    Entry *entry = findEntry(dict->entries, dict->capacity, key);
    bool isNewKey = entry->key == NULL;
    entry->key = key;
    entry->value = value;
    if (isNewKey)
    {
        dict->count++;
    }
    return isNewKey;
}

bool deleteEntry(MVM *vm, MyMoDict *dict, MyMoObject *key)
{
    if (dict->count == 0)
        return false;
    Entry *entry = findEntry(dict->entries, dict->capacity, key);
    if (entry->key == NULL)
        return false;
    dict->count--;
    entry->key = NULL;
    entry->value = NEW_BOOL(false);
    return true;
}

void copyDict(MVM *vm, MyMoDict *from, MyMoDict *to)
{
    for (int i = 0; i <= from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL)
        {
            setEntry(vm, to, entry->key, entry->value);
        }
    }
}

MyMoObject *findKey(MyMoDict *dict, u32 hash)
{
    if (dict->count == 0)
        return NULL;
    u32 index = hash & dict->capacity;
    for (;;)
    {
        Entry *entry = &dict->entries[index];
        if (entry->key == NULL)
        {
            if (IS_NIL(entry->value))
                return NULL;
        }
        else if (entry->key->hash == hash)
        {
            return entry->key;
        }
        index = (index + 1) & dict->capacity;
    }
}

void printDict(MyMoDict *dict)
{
    printf("{");
    for (int i = 0, j = dict->count; i <= dict->capacity; i++)
    {
        Entry *entry = &dict->entries[i];
        if (entry->key != NULL)
        {
            printObject(entry->key);
            printf(": ");
            if (entry->value == AS_OBJECT(dict))
            {
                printf("{...}");
            }
            else
            {
                printObject(entry->value);
            }
            if (--j)
                printf(", ");
        }
    }
    printf("}");
}

MyMoString *findString(MyMoDict *dict, const char *chars, int length, u32 hash)
{
    if (dict->entries == NULL)
        return NULL;
    u32 index = hash & dict->capacity;
    for (;;)
    {
        Entry *entry = &dict->entries[index];
        MyMoString *key = AS_STRING(entry->key);
        if (key == NULL)
            return NULL;
        if (key->length == length &&  entry->key->hash == hash && memcmp(key->value, chars, length) == 0)
        {
            return key;
        }
        index = (index + 1) & dict->capacity;
    }
}

MyMoInt *findInt(MyMoDict *dict, long value, int length, u32 hash)
{
    if (dict->entries == NULL)
        return NULL;
    u32 index = hash & dict->capacity;
    for (;;)
    {
        Entry *entry = &dict->entries[index];
        MyMoInt *key = AS_INT(entry->key);
        if (key == NULL)
            return NULL;
        if (key->length == length  &&  entry->key->hash == hash && key->value == value)
        {
            return key;
        }
        index = (index + 1) & dict->capacity;
    }
}

MyMoDouble *findDouble(MyMoDict *dict, double value, int length, u32 hash)
{
    if (dict->entries == NULL)
        return NULL;
    u32 index = hash & dict->capacity;
    for (;;)
    {
        Entry *entry = &dict->entries[index];
        MyMoDouble *key = AS_DOUBLE(entry->key);
        if (key == NULL)
            return NULL;
        if (key->length == length &&  entry->key->hash == hash  && key->value == value)
        {
            return key;
        }
        index = (index + 1) & dict->capacity;
    }
}

void setPrimitive(MVM *vm, MyMoDict *dict, MyMoObject *key)
{
    if (dict->count + 1 > (dict->capacity + 1) * TABLE_MAX_LOAD)
    {
        int capacity = ResizeCapacity(dict->capacity + 1) - 1;
        adjustCapacity(vm, dict, capacity);
    }

    uint index = key->hash & dict->capacity;
    Entry *entry;
    for (;;) {
        entry = &dict->entries[index];
        if (entry->key == NULL) {
            break;
        } else {
            if (entry->key == key) {
                break;
            }
        }
        index = (index + 1) & dict->capacity;
    }
    entry->key = key;
    entry->value = NEW_NIL; 
    dict->count++;
}