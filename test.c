 
// #include <stdio.h>
// #include <stdlib.h>
// #include <dlfcn.h>
// #define MAX_PATH 4069

// int main(int argc, char *argv)
// {
//     char path[MAX_PATH + 1];
//     snprintf(path,MAX_PATH,"./modules/mathmod.so");
//     void *handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
//     if (!handle)
//     {
//         fputs (dlerror(), stderr);
//         printf("cannot open module `%s`\n", path);
//         return 1;
//     }
//    return 0;
// }