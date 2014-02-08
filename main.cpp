#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <getopt.h>

#include "util.h"

using namespace std;

void Decomp (int prof, int temps, int Verbose);
void Help(void);

/*******************************************************************************************/
void Help(void)
/*******************************************************************************************/
{
  printf ("MN90 - V0.1\n");
  printf (" - P value : Profondeur max atteinte durant la plongée\n");
  printf (" - T value : Temps entre l'immersion et le début de remontée a 15-17m/mn\n");
  exit(EXIT_SUCCESS);
}

/*******************************************************************************************/
int main(int argc, char *argv[])
/*******************************************************************************************/
{
  // Analyse des params

  char c;
  int prof=-1,temps=-1,Verbose=false;
  while ((c = getopt (argc, argv, "P:T:hH?vV")) != -1) {
    switch (c) {
      case 'v': // Verbose
      case 'V': // Verbose
        Verbose=true;
        break;
      case 'P': // Profondeur Max
        prof = atoi(optarg);
        break;
      case 'T':
        temps = atoi(optarg);
        break;
      case '?':
      case 'h':
      case 'H':
        Help();
        break;
      default:
        abort ();
    }
  }
  if (prof==-1 || temps==-1) {
    Help();
    Fatal(3,"Parametres d'entrée incorercts");
  }
  printf ("Profondeur = %im, Durée = %imn\n\n",prof,temps);
  Decomp(prof,temps,Verbose);
  //system("PAUSE");
  return EXIT_SUCCESS;
}
