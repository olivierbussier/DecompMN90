#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <getopt.h>

#include "util.h"
#include "decomp.h"

using namespace std;

void Help(void);

/*******************************************************************************************/
void Help(void)
/*******************************************************************************************/
{
  printf ("MN90 - V0.1 - 20/01/2014 (Olivier Bussier)\n");
  printf ("Decomp -P val -T val [-v] [-D val] [-M val] [-m val]\n");
  printf (" - P value : Profondeur max atteinte durant la plongée\n");
  printf (" - T value : Temps entre l'immersion et le début de remontée a 15-17m/mn\n");
  printf (" - D value : Vitesse de descente en m/mn (default=20m/mn)\n");
  printf (" - M value : Vitesse de remontee avant palier en m/mn (default=17m/mn)\n");
  printf (" - m value : Vitesse de remontee entre les palier en m/s (default=3m/mn)\n");
  exit(EXIT_SUCCESS);
}

/*******************************************************************************************/
int main(int argc, char *argv[])
/*******************************************************************************************/
{
  // Analyse des params

  char c;
  int prof=-1,temps=-1,Verbose=false;
  int vDesc=20,vMontA=17,vMontP=3;
  double pAzote=0.79;

  while ((c = getopt (argc, argv, "P:T:D:M:m:hH?vV")) != -1) {
    switch (c) {
      case 'v': // Verbose
      case 'V': // Verbose
        Verbose=true;
        break;
      case 'A': // Pression partielle d'azote en surface
        pAzote = atoi(optarg);
        break;
      case 'D': // Vitesse de descente
        vDesc = atoi(optarg);
        break;
      case 'M': // Vitesse de montée
        vMontA = atoi(optarg);
        break;
      case 'm': // Vitesse de montée
        vMontP = atoi(optarg);
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
  Decomp(prof,temps,Verbose,vDesc,vMontA,vMontP,pAzote);
  //system("PAUSE");
  return EXIT_SUCCESS;
}
