#include "util.h"

#include <stdio.h>

//#define __MINGW32__
#include <pthread.h>
//#undef __MINGW32__

int main(int argc, char *argv[])
{
  for (int i = 0; i < argc; ++i)
    printf("%s\n", argv[i]);
  
  return 0;
}
