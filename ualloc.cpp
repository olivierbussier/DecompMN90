#include <stdlib.h>
#include "ualloc.h"
#include "ufatal.h"

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

