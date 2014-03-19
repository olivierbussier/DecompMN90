#include <stdio.h>

enum tCMD {eDIVE};
typedef struct {
  char *Command;
  tCMD Cmd;
} tXML;

tXML xml[]={
  {"dive",eDIVE},
  {"dive",eDIVE}
};

/**********************************************************/
int ReadXmlFile(char *file)
/**********************************************************/
{
  FILE *fi;
  char buffer[16384];

  fi=fopen(file,"rt");
  if (fi==NULL)
    return 2;

  fgets(buffer,sizeof(buffer),fi);
}
