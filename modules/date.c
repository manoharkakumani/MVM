// #include "modules.h"
// #include <time.h>

// MyMoObject *nowfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if(argc != 0){
//         runtimeError(vm, "TypeError: now() takes 0 argument (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     return NEW_INT(vm, time(NULL));
// }

// //format time and date

// MyMoObject *formatfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if(argc != 2){
//         runtimeError(vm, "TypeError: format() takes 2 argument (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if(!IS_STRING(argv[0])){
//         runtimeError(vm, "TypeError: must be <object 'string'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     if(!IS_NUMBER(argv[1])){
//         runtimeError(vm, "TypeError: must be <object 'int'>, not (%s)",getType(argv[1]));
//         return NEW_EMPTY;
//     }
//     MyMoObject *object = pop(vm);
//     MyMoObject *object2 = pop(vm);
//     return NEW_STRING(vm, ctime((time_t *)NUMBER_VAL(object2)));
// }

// //get time and date

// MyMoObject *getfn(MVM *vm, uint argc, MyMoObject *argv[]){
//     if(argc != 1){
//         runtimeError(vm, "TypeError: get() takes 1 argument (%d given).", argc);
//         return NEW_EMPTY;
//     }
//     if(!IS_STRING(argv[0])){
//         runtimeError(vm, "TypeError: must be <object 'string'>, not (%s)",getType(argv[0]));
//         return NEW_EMPTY;
//     }
//     MyMoObject *object = pop(vm);
//     return NEW_STRING(vm, ctime((time_t *)NUMBER_VAL(object2)));
// }

// MODULE(date){
//     MyMoModuleFunction functions [] = {
//                                     {"now",nowfn}
//                                     };

//     MyMoModuleVariable variables [] = {};

// }