#include "memory.h"
#include "compiler.h"
#include "utils.h"
#include "vm.h"
#include "./datatypes/datatypes.h"
#include <time.h>
#include "cache.h"
#include <sys/stat.h>

MyMoObject *clockfn(MVM *vm, u32 argc, MyMoObject *args[])
{
    UNUSED(vm);
    UNUSED(args);
    if (argc)
    {
        runtimeError(vm, "clock() takes no arguments (%d given)", argc);
        return NEW_EMPTY;
    }
    return NEW_DOUBLE(vm, (double)clock() / CLOCKS_PER_SEC);
}
MyMoObject *inputfn(MVM *vm, u32 argc, MyMoObject *args[])
{
    if (argc > 1)
    {
        runtimeError(vm, "input() takes 1 argument (%d given).", argc);
        return NEW_EMPTY;
    }
    if (argc != 0)
    {
        MyMoObject *prompt = args[0];
        if (!IS_STRING(prompt))
        {
            runtimeError(vm, "input() only takes a string argument");
            return NEW_EMPTY;
        }
        printf("%s", STRING_VAL(prompt));
    }
    uint64_t currentSize = 128;
    char *line = malloc(currentSize);
    // if (line == NULL) MemoryError("MVM out of memory on input()!");
    int c = EOF;
    uint64_t i = 0;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        line[i++] = (char)c;
        if (i == currentSize)
        {
            currentSize = ResizeCapacity(currentSize);
            line = realloc(line, currentSize);
            // if (line == NULL) MemoryError("MVM out of memory on input()!");
        }
    }
    line[i] = '\0';
    MyMoObject *l = AS_OBJECT(NEW_STRING(vm, line, strlen(line)));
    free(line);
    return l;
}

MyMoObject *printfn(MVM *vm, u32 argc, MyMoObject *args[])
{
    UNUSED(vm);
    if (argc == 0)
    {
        printf("\n");
        return NEW_BOOL(0);
    }
    int i = 0;
    for (i = 0; i < (argc - 1); i++)
    {
        printObject(args[i]);
        pop(vm);
        printf(" ");
    }
    printObject(args[i]);
    pop(vm);
    printf("\n");
    return NEW_BOOL(1);
}

MyMoObject *typefn(MVM *vm, u32 argc, MyMoObject *args[])
{
    UNUSED(vm);
    if (argc != 1)
    {
        runtimeError(vm, "type() only takes 1 argument");
        return NEW_EMPTY;
    }
    char *type = getType(pop(vm));
    return NEW_STRING(vm, type, strlen(type));
}

MyMoObject *globalsfn(MVM *vm, u32 argc, MyMoObject *args[])
{
    if (argc)
    {
        runtimeError(vm, "globals() takes no arguments (%d given).", argc);
        return NEW_EMPTY;
    }
    return AS_OBJECT(&vm->globals);
}

MyMoObject *compilefn(MVM *vm, u32 argc, MyMoObject *args[])
{
    if (argc != 1)
    {
        runtimeError(vm, "compile() takes 1 argument (%d given).", argc);
        return NEW_EMPTY;
    }
    if (!IS_STRING(args[0]))
    {
        runtimeError(vm, "compile() takes a string argument");
        return NEW_EMPTY;
    }
    char *source = STRING_VAL(pop(vm));
    MyMoCode *code = newCode(vm);
    MyMoFunction *function = compile(code->vm, source, "@compile", COMPILE_STRING);
    if (function == NULL)
        return NEW_EMPTY;
    code->function = function;
    return AS_OBJECT(code);
}

MyMoObject *execfn(MVM *vm, u32 argc, MyMoObject *args[])
{
    if (argc != 1)
    {
        runtimeError(vm, "exec() takes 1 argument (%d given).", argc);
        return NEW_EMPTY;
    }
    if (!(IS_CODE(args[0])))
    {
        runtimeError(vm, "exec() takes a code object argument");
        return NEW_EMPTY;
    }
    MyMoCode *code = AS_CODE(pop(vm));
    I_Result result = interpreter(code->vm, code->function);
    if (result == RUNTIME_ERROR)
        return NEW_EMPTY;
    return NEW_BOOL(1);
}

MyMoObject *lenfn(MVM *vm, u32 argc, MyMoObject *args[])
{
    if (argc != 1)
    {
        runtimeError(vm, "TypeError: len() takes 1 argument (%d given).", argc);
        return NEW_EMPTY;
    }
    MyMoObject *obj = args[0];
    switch (obj->type)
    {
    default:
        runtimeError(vm, "TypeError: object of %s has no len()", getType(obj));
        return NEW_EMPTY;
    }
}

MyMoObject *yieldfn(MVM *vm, u32 argc, MyMoObject *args[])
{
    if (argc != 1)
    {
        runtimeError(vm, "yeild() takes 1 argument (%d given).", argc);
        return NEW_EMPTY;
    }
    MyMoObject *obj = pop(vm);
    vm->fiber->state = FIBER_YIELD;
    vm->fiber = vm->fiber->parent;
    return obj;
}

void defineBuiltInFunction(MVM *vm, const char *name, BuiltInfunction function)
{
    MyMoObject *fnName = NEW_STRING(vm, name, strlen(name));
    MyMoObject *fn = AS_OBJECT(newBuiltInFunction(vm, AS_STRING(fnName), function, OBJ_BUILTIN_FUNCTION));
    setEntry(vm, &vm->builtins, fnName, fn);
}

void defineBuiltInFunctions(MVM *vm)
{
    defineBuiltInFunction(vm, "clock", clockfn);
    defineBuiltInFunction(vm, "input", inputfn);
    defineBuiltInFunction(vm, "print", printfn);
    defineBuiltInFunction(vm, "type", typefn);
    defineBuiltInFunction(vm, "globals", globalsfn);
    defineBuiltInFunction(vm, "compile", compilefn);
    defineBuiltInFunction(vm, "exec", execfn);
    defineBuiltInFunction(vm, "len", lenfn);
    defineBuiltInFunction(vm, "yield", yieldfn);
}

MyMoFunction *runFile(MVM *vm, char *path)
{
    MyMoFunction *function;
    FILE *file = fopen(path, "rb");
    if (file == NULL)
    {
        runtimeError(vm, "Module Error: could not open module '%s'.", path);
        exit(74);
    }
    int len = strlen(path);
    if (path[len - 1] == 'c')
    {
        function = functionDeserialize(vm, file);
    }
    else
    {
        fseek(file, 0L, SEEK_END);
        size_t fileSize = ftell(file);
        rewind(file);
        char *buffer = (char *)malloc(fileSize + 1);
        if (buffer == NULL)
        {
            runtimeError(vm, "Not enough memory to read \"%s\".", path);
            exit(74);
        }
        size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
        if (bytesRead < fileSize)
        {
            runtimeError(vm, "Could not read module \"%s\".\n", path);
            exit(74);
        }
        buffer[bytesRead] = '\0';
        function = compile(vm, buffer, path, COMPILE_SCRIPT);
        free(buffer);
    }
    fclose(file);
    return function;
}

char *pathResolver(MVM *vm, char *_path)
{
#define DELIMITER '/'
    size_t len = strlen(_path);
    char *path = New(char, len + 1);
    if (vm->currentModule != NULL)
    {
        if (_path[0] == DELIMITER)
        {
            strcpy(path, _path);
        }
        else
        {
            u32 c = 0;
            path = realloc(path, len + vm->currentModule->path->length + 4);
            memcpy(path, vm->currentModule->path->value, vm->currentModule->path->length);
            path[vm->currentModule->path->length] = '\0';
            char *lastSlash;
        parent:
            lastSlash = strrchr(path, DELIMITER);
            if (lastSlash != NULL)
            {
                lastSlash[1] = '\0';
            }
        current:
            if (_path[c] == '.')
            {
                if (_path[c + 1] == DELIMITER)
                {
                    c += 2;
                    goto current;
                }
                else if (_path[c + 1] == '.' && _path[c + 2] == DELIMITER)
                {
                    c += 3;
                    goto parent;
                }
            }
            int k = len + 1 - c;
            char helper[k];
            int i;
            for (i = 0; i < k; i++)
            {
                helper[i] = _path[c + i];
            }
            helper[i] = '\0';
            strcat(path, helper);
        }
        strcat(path, ".my");
    }
    else
    {
        char actualpath[4096];
        char *__path = realpath(_path, actualpath);
        if (__path)
        {
            size_t _len = strlen(__path);
            path = realloc(path, _len + 1);
            memcpy(path, __path, _len);
            path[_len] = '\0';
        }
        else
        {
            return NULL;
        }
    }
    size_t pathLen = strlen(path);
    path[pathLen] = '\0';
    char *cachePath = New(char, pathLen + 2);
    memcpy(cachePath, path, pathLen);
    cachePath[pathLen] = '\0';
    strcat(cachePath, "c");
    struct stat file;
    struct stat cachefile;
    if (stat(path, &file) == 0 && stat(cachePath, &cachefile) == 0)
    {
        if (cachefile.st_mtime < file.st_mtime)
        {
            free(cachePath);
            return path;
        }
        free(path);
        return cachePath;
    }
    else
    {
        free(cachePath);
        return path;
    }
#undef DELIMITER
}