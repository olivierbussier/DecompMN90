#include <stdio.h>
#include <stdarg.h>
//#include <stdlib.h>
#include <string.h>

#include "uutil.h"
#include "ualloc.h"

// Globals

char *BufStr=NULL;

/*******************************************************************************************/
void strinit (void)
/*******************************************************************************************/
{
  BufStr=(char *)Alloc(1);
  *BufStr=0;
}

/*******************************************************************************************/
char *strget (void)
/*******************************************************************************************/
{
  return BufStr;
}

/*******************************************************************************************/
void str (const char *format, ...)
/*******************************************************************************************/
{
  va_list args;
  char *tempstring=(char *)Alloc(65000);
  int len;

  va_start (args, format);
  len = vsprintf (tempstring,format, args);
  va_end (args);
  BufStr = (char *)Realloc(BufStr,strlen(BufStr)+len+1);
  strcat(BufStr,tempstring);
  Free(tempstring);
}

/*******************************************************************************************/
void strend (void)
/*******************************************************************************************/
{
  if (BufStr!=NULL) {
    Free(BufStr);
    BufStr=NULL;
  }
}
