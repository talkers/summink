#include "player.h"
#ifdef TRACK
int addfunction(char *newfunction)
{
  int loop;

  strcpy(functionhist[funcposition],newfunction);

  funcposition++;
  if (funcposition>19)
    funcposition=0;

}
#endif
