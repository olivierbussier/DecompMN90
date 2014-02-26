#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/*******************************************************************************************/
void *Alloc(unsigned int size)
/*******************************************************************************************/
{
  char *tmp;
  if (size==0)
    Fatal(2,"Allocation taille nulle");
  tmp=(char *)malloc(size);
  if (tmp==NULL)
    Fatal (2,"Erreur d'allocation memoire");
  return (void *)tmp;
}

/*******************************************************************************************/
void *Realloc(void *buffer,unsigned int size)
/*******************************************************************************************/
{
  char *tmp;
  if (size==0)
    Fatal(2,"Reallocation taille nulle");
  if (buffer==NULL)
    Fatal(2,"Reallocation buffer nul");

  tmp=(char *)realloc(buffer,size);
  if (tmp==NULL)
    Fatal (2,"Erreur de reallocation memoire");
  return (void *)tmp;
}

/*******************************************************************************************/
void Free(void *buffer)
/*******************************************************************************************/
{
  if (buffer==NULL)
    Fatal (2,"Erreur de libération memoire");
  free(buffer);
}

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

/*******************************************************************************************/
void Fatal(int ErrorCode, const char *Message)
/*******************************************************************************************/
{
  printf ("Erreur %3.0i - %s\n",ErrorCode,Message);
  exit(ErrorCode);
}
