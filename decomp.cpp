/******************************************************************************
' PROGRAMME BASIC CALCULANT LES PALIERS POUR UN TEMPS ET UNE
' PROFONDEUR DONNES (vitesse de remont√©e = 17 m/mn) '
' (c) Axel CHAMBILY-CASADESUS '
' variables globales '
' i : compteur de boucle
' j : compteur de paliers ( j=1 : prof max plong√©e )
' prof : profondeur maximale atteinte
' temps : temps de plong√©e
' periode(i) : p√©riode du tissu i : constantes
' sursatcrit(i) : seuil de sursaturation critique du tissu i : constantes
' sc(i,j) : coefficient de sursaturation du tissu i en fin de
niveau j
' tn2(i,j) : tension d'azote du tissu i en fin de niveau j
' ppalier(i) : profondeur 1er palier pour le tissu i
' dpalier(i,j) : dur√©e palier j pour le tissu i
' dpal(i) : copie tableau dpalier pour tri ' dpal : dur√©e palier tissu directeur
' index%(i) : index de tri du tableau ppalier()
' ppN2s : ppN2 en surface : constante=0.8 ( modifiable en altitude )
' pmoy : profondeur moyenne pour 1er palier th√©orique
' pt : 1er palier th√©orique puis paliers successifs tous les 3m '
' variables locales '
' p : profondeur dans la fonction tf qui calcule PPN2
' t : temps dans la fonction tn2 qui calcule TN2
' periode : dans la fonction tn2
' ta : tension TN2 √† l'√©tat pr√©c√©dent
' scm : variable pour passer sursatcrit(i)
*****************************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "decomp.h"

//#define TEST

#ifdef TEST
int    periode   []={7    , 30   , 60   , 120 };
double sursatcrit[]={2.56 , 1.84 , 1.6  , 1.6 };
#else
int    periode   []={5    , 7    , 10   , 15   , 20   , 30   , 40   , 50   , 60   , 80   , 100  , 120  };
double sursatcrit[]={2.72 , 2.54 , 2.38 , 2.20 , 2.04 , 1.82 , 1.68 , 1.61 , 1.58 , 1.56 , 1.55 , 1.54 };
#endif // TEST

#define nbcompart (sizeof(periode)/sizeof(periode[0]))


tCompart Compartiment[] = {
  {  "C5",   5, 2.72},
  {  "C7",   7, 2.54},
  { "C10",  10, 2.38},
  { "C15",  15, 2.20},
  { "C20",  20, 2.04},
  { "C30",  30, 1.82},
  { "C40",  40, 1.68},
  { "C50",  50, 1.61},
  { "C60",  60, 1.58},
  { "C80",  80, 1.56},
  {"C100", 100, 1.55},
  {"C120", 120, 1.54}
};

// Constantes
double ppN2s=0.8;

double sc     [nbcompart][10];
double tn2    [nbcompart][10];
double dpalier[nbcompart][10];
double ppalier[nbcompart];
double dpal   [nbcompart];
double pt;
double pmoy;
double ddpal;


/*******************************************************************************************/
double Prof2Press(double Profondeur)
/*******************************************************************************************/
{
  return ((Profondeur / 10) + 1) * 0.8;
}

/*******************************************************************************************/
double fppalier(double ta,double scm)
/*******************************************************************************************/
{
  return ((ta / scm) - 1) * 10;
}

/*******************************************************************************************/
double fdpalier(double Profondeur,double ta,double scm,int period)
/*******************************************************************************************/
{
  //DEFFN dpalier(p,ta,scm,periode)
  //      LOG((tf(p) - ta) / (tf(p) - (((p          - 3) / 10) + 1) * scm)) / LOG(2)) * periode
  double Press = Prof2Press(Profondeur);
  return (log((Press - ta) / (Press - (((Profondeur - 3) / 10) + 1) * scm)) / log(2)) * period;
}

/*******************************************************************************************/
double Ti2Tf(double Ti,int Profondeur,int Temps,int Periode)
/*******************************************************************************************/
{
  // Calcul  de la nouvelle tension d'un tissus de pÈriode P de tension Ti
  // soumis a une tension Tf pendant une durÈe T
  // ------------------------------------------------------------------------
  // T = Ti + (Tf-Ti)*(1-0,5^^(T/P))

  double Tf = Prof2Press(Profondeur);
  return Ti + ( Tf - Ti) * (1 - pow(0.5,((double)Temps) / (double)Periode));
}

#define VERBOSE if (Verbose)

/*******************************************************************************************/
void Decomp (int prof, int temps,int Verbose)
/*******************************************************************************************/
{
  int i,j,Tdir;
  double tmax,ppal;
  prof = prof * (0.8 / ppN2s); // profondeur fictive

// Init

  j=0;

  for (i=0;i<(int)nbcompart;i++) {
    tn2[i][j] = Ti2Tf(0.8,prof,temps,periode[i]);
    VERBOSE printf ("On a TN2=%4.2f pour le tissu %i'\n",tn2[i][j],periode[i]);
  }
  VERBOSE printf ("\n");

  for (i=0;i<(int)nbcompart;i++) {
    ppalier[i] = fppalier(tn2[i][j],sursatcrit[i]);
    VERBOSE {
    if (ppalier[i] > 0) {
      printf ("Le tissu %3.0i' impose un palier a %4.2f\n",periode[i],ppalier[i]*ppN2s/0.8);
	} else {
      printf ("Le tissu %3.0i' n'impose pas de palier\n",periode[i]);
    }
    }
  }
  VERBOSE printf ("\n");

  // Recherche du tissu directeur

  Tdir=-1;
  ppal=-1;
  for (i=0;i<(int)nbcompart;i++) {
    if (ppalier[i]>ppal) {
      ppal=ppalier[i];
      Tdir=i;
    }
  }
  if (Tdir==-1) Fatal(2,"Erreur recherche compartiment directeur");

  if (ppal <= 0) {
    printf("On peut remonter sans palier.\n");
	return;
  }

  VERBOSE printf("Le tissu %i' est donc le tissu directeur\n",periode[Tdir]);

  //ppal = ceil(ppal); //if ((int)(ppal / 3) == ppal / 3)    pt = ppal;  else
  pt = ((int)(ppal / 3) + 1) * 3;

  VERBOSE printf ("Il faudrait un palier thÈorique ‡ %4.2fm\n\n",pt*ppN2s/0.8);

  pmoy = prof - pt - 10;
  j++;

  for (i=0;i<(int)nbcompart;i++)
    tn2[i][j] = Ti2Tf(tn2[i][j - 1],pmoy,(prof - pt) / 17,periode[i]);

  sc[Tdir][j] = tn2[Tdir][j] / (1 + (pt - 3) / 10);

  if (sc[Tdir][j] < sursatcrit[Tdir]) {
    VERBOSE {
    printf ("Pour le tissu directeur C%i, sc = %f ‡ %f m\n",periode[Tdir],sc[Tdir][j],(pt - 3)*ppN2s/0.8);
	printf ("Comme sc<%f le palier thÈorique a %f est inutile\n",sursatcrit[Tdir],pt*ppN2s/0.8);
    printf ("et on peut donc remonter directement ‡ %f m\n",(pt - 3)*ppN2s/0.8);
    }
    pt = pt - 3;
  } else {
    VERBOSE {
    printf("Pour le tissu directeur %i', sc = %4.2f ‡ %4.2fm\n",periode[Tdir],sc[Tdir][j],(pt - 3)*ppN2s/0.8);
    printf("Comme sc>=%f, il faut rÈellement effectuer un palier ‡ %f m\n",sursatcrit[Tdir],pt*ppN2s/0.8);
    }
  }
  VERBOSE printf ("\n\n");

// REPEAT
// UNTIL INKEY$ <> "" CLS

  while(pt > 0) {
    VERBOSE printf ("--------------------------------Palier ‡ %2.2f m ----------------------------\n",pt);
    j++;
    for (i=0;i<(int)nbcompart;i++) {
      if (tn2[i][j - 1] / (1 + (pt - 3) / 10) >= sursatcrit[i]) {
        dpalier[i][j] = fdpalier(pt,tn2[i][j - 1],sursatcrit[i],periode[i]);
        VERBOSE printf ("Le tissu %3.0i' (sc=%4.2f/T=%4.2f) -> palier a %3.0fm : %3.2fmn\n",periode[i],sursatcrit[i],tn2[i][j-1],pt*ppN2s/0.8,dpalier[i][j]);
      } else {
        VERBOSE printf ("Le tissu %3.0i' (sc=%4.2f/T=%4.2f) -> palier a %3.0fm : N.A\n"    ,periode[i],sursatcrit[i],tn2[i][j-1],pt*ppN2s/0.8);
      }
    }
    // Recherche du tissu directeur
    tmax=0;
    Tdir=-1;
    for (i=0;i<(int)nbcompart;i++) {
      if (dpalier[i][j]>tmax) {
        tmax = dpalier[i][j];
        Tdir=i;
      }
    }
    if (Tdir==-1) Fatal(3,"Erreur recherche compartiment directeur");

    tmax=ceil(tmax);
    VERBOSE printf("Le tissu directeur est le tissu %i'\n",periode[Tdir]);
    VERBOSE {
    printf("Il faut donc effectuer un palier de %4.1f minutes a %4.1f mËtres\n\n",tmax,pt*ppN2s/0.8);
    } else {
    printf("Palier %4.1fm : %#5.2fmm\n",pt*ppN2s/0.8,tmax);
    }

    // PRINT "Appuyer sur une touche pour continuer."

    for (i=0;i<(int)nbcompart;i++)
      tn2[i][j] = Ti2Tf(tn2[i][j - 1],pt,tmax,periode[i]);
    pt = pt - 3;
  }

  VERBOSE {
  printf("Le plongeur est sorti avec un coefficient C Ègal ‡ : %f\n",tn2[nbcompart-1][j] / 0.8);
  } else {
  printf("C=%f\n",tn2[nbcompart-1][j] / 0.8);
  }
}
// INPUT "Profondeur de la prochaine plong√©e ";prof
// prof = prof * (0.8 / ppN2s) // profondeur fictive
// INPUT "Intervalle ( en minutes ) ";inter
// PRINT
// tfs = FN tn2(tn2(4,j),0,inter,120)
// majo = ROUND((LOG((FN tf(prof) - 0.8) / (FN tf(prof) - tfs)) / LOG(2)) * 120,2)
// PRINT "La majoration sera √©gale √† ";majo;" mn."
