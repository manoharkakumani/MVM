#include "module.h"
#include "../modules/modules.h"
#include "../vm.h"
#include "nil.h"

#ifdef _WIN32
  #include <Windows.h>
#else
    #include <dlfcn.h>
#endif

MyMoModule *newModule(MVM *vm, MyMoString *name, MyMoString *path)
{
    MyMoModule *module = AllocateObject(vm, MyMoModule, OBJ_MODULE);
    module->name = name;
    module->path = path;
    module->variables = newDict(vm);
    module->parent = NULL;
    setEntry(vm, &vm->modules, AS_OBJECT(path), AS_OBJECT(module));
    return module;
}

void printModule(MyMoModule *module)
{
    switch (module->type)
    {
    case MODULE_BULTIN:
        printf("<module '%s' (built-in)>", module->name->value);
        break;
    default:
        printf("<module %s at %p>", module->name->value, module);

    }
}

void defineModuleFunctions(MVM *vm, MyMoModule *module, MyMoModuleFunction functions[], size_t size){
    for (int i = 0; i < size; i++)
    {
        MyMoObject *name = NEW_STRING(vm, functions[i].name, strlen(functions[i].name));
        MyMoObject *function = AS_OBJECT(newBuiltInFunction(vm, AS_STRING(name), functions[i].function, OBJ_BUILTIN_FUNCTION));
        setEntry(vm,module->variables, name, function);        
    }
}

void defineModuleVariables(MVM *vm, MyMoModule *module, MyMoModuleVariable variables[], size_t size){
    for (int i = 0; i < size; i++)
    {
        setEntry(vm,module->variables, NEW_STRING(vm, variables[i].name, strlen(variables[i].name)), variables[i].variable);        
    }
}

MyMoObject *defineBuiltInModule(MVM *vm, MyMoModuleDef *moduleDef)
{
    char path[1024];
    snprintf(path, sizeof(path), "module '%s' (built-in)", moduleDef->name);
    MyMoModule *module = newModule(vm,newString(vm, moduleDef->name, strlen(moduleDef->name)), newString(vm, path, strlen(path)));
    defineModuleFunctions(vm, module, moduleDef->functions, moduleDef->totalfn);
    defineModuleVariables(vm, module, moduleDef->variables, moduleDef->totalvar);
    setEntry(vm,&vm->builtInModules,AS_OBJECT(module->name),AS_OBJECT(module));
    return AS_OBJECT(module);
}

const char *getHomeDir(void)
{
  const char *result = "/opt/mymo";
#ifdef _WIN32
  const char *drive = getenv("SystemDrive");
  char *path[MAX_PATH + 1];
  snprintf(path, MAX_PATH, "%s\\mymo", drive);
  strncpy_s(path, MAX_PATH, drive, _TRUNCATE);
  result = (const char *) path;
#endif
  return result;
}
#define GETNAME(n) #n
#define CALL_MODULE(n) GETNAME(n)##Module;
MyMoObject *loadBuiltInModule(MVM *vm, MyMoString *name){

    // char path[MAX_PATH + 1];
    // snprintf(path,MAX_PATH,"./modules/%s%s",name->value,FILE_POSTFIX);

    // #ifdef _WIN32
    //     HINSTANCE handle = LoadLibrary(path);
    // #else
    //     void *handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    // #endif

    // if (!handle)
    // {
    //     fputs (dlerror(), stderr);
    //     return NEW_EMPTY;
    // }
    // loadModule module;
    // char moduleName[MAX_PATH + 1];
    // snprintf(moduleName,MAX_PATH,"%s%s",name->value,"Module");
    
    // #ifdef _WIN32
    // module = (loadModule) GetProcAddress(handle, moduleName);
    // #else
    // *((void **) &module) = dlsym(handle, moduleName);
    // #endif
    // if (!module)
    // {
    //     fputs(dlerror(), stderr);
    //     return NEW_EMPTY;
    // }
   return NEW_EMPTY;
}