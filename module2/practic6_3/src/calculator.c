#include "../include/calculator.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>

void include_lib(func_pair functions[])
{
  DIR* lib = opendir("./lib/");
  size_t index = 0;
  struct dirent* file;
  while ((file = readdir(lib)) != NULL) {
    if (file)
    {
      if (strstr(file->d_name, ".so"))
      {
        char fullpath[612];
        snprintf(fullpath, sizeof(fullpath), "./lib/%s", file->d_name);
        void* handle = dlopen(fullpath, RTLD_GLOBAL | RTLD_LAZY);
        if (!handle)
        {
          fprintf(stderr, "%s : %s", fullpath, dlerror());
        }
        else
        {
          if (index < PAIR_LENGH)
          {
            char funcname[64];
            size_t len = strlen(file->d_name);
            strncpy(funcname, file->d_name, strlen(file->d_name) - 3);
            funcname[len - 3] = '\0';
            void* func = (dlsym(handle, funcname));
            functions[index].name = strdup(funcname);
            functions[index].func = (double (*)(double, double))func;
            functions[index].handle = handle;
            index++;
          }
        }
      }
    }
  }
  closedir(lib);
}

void clean_lib(func_pair *functions)
{
  for (int i = 0; i < PAIR_LENGH; i++)
  {
    dlclose(functions[i].handle);
    free(functions[i].name);
  }
}
