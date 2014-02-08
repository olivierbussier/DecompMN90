#include <stdio.h>
#include <stdlib.h>

#include "util.h"


/*******************************************************************************************/
void Fatal(int ErrorCode, const char *Message)
/*******************************************************************************************/
{
  printf ("Erreur %3.0i - %s\n",ErrorCode,Message);
  exit(ErrorCode);
}
