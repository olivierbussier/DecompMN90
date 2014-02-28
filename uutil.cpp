#include <stdio.h>
#include <stdarg.h>
//#include <stdlib.h>
#include <string.h>

#include "uutil.h"
#include "ualloc.h"

/*******************************************************************************************/
char *strinit (void)
/*******************************************************************************************/
{
  char *tmp=(char *)Alloc(1);
  *tmp=0;
  return tmp;
}

/*******************************************************************************************/
char *str (char *buffer, const char *format, ...)
/*******************************************************************************************/
{
  va_list args;
  char *tempstring=(char *)Alloc(65000);
  int len;

  va_start (args, format);
  len = vsprintf (tempstring,format, args);
  va_end (args);
  buffer = (char *)Realloc(buffer,strlen(buffer)+len+1);
  strcat(buffer,tempstring);
  Free(tempstring);
  return buffer;
}

/*******************************************************************************************/
void strend (char *buffer)
/*******************************************************************************************/
{
  Free(buffer);
}
