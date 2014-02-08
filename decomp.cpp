/******************************************************************************
' PROGRAMME BASIC CALCULANT LES PALIERS POUR UN TEMPS ET UNE
' PROFONDEUR DONNES (vitesse de remontÃ©e = 17 m/mn) '
' (c) Axel CHAMBILY-CASADESUS '
' variables globales '
' i : compteur de boucle
' j : compteur de paliers ( j=0 : prof max plongÃ©e )
' prof : profondeur maximale atteinte
' temps : temps de plongÃ©e
' periode(i) : pÃ©riode du tissu i : constantes
' sursatcrit(i) : seuil de sursaturation critique du tissu i : constantes
' sc [i][j] : coefficient de sursaturation du tissu i en fin de niveau j
' tn2[i][j] : tension d'azote du tissu i en fin de niveau j
' ppalier(i) : profondeur 1er palier pour le tissu i
' dpalier(i,j) : durÃ©e palier j pour le tissu i
' dpal(i) : copie tableau dpalier pour tri
' dpal : durÃ©e palier tissu directeur
' index%(i) : index de tri du tableau ppalier()
' ppN2s : ppN2 en surface : constante=0.8 ( modifiable en altitude )
' pmoy : profondeur moyenne pour 1er palier thÃ©orique
' pt : 1er palier thÃ©orique puis paliers successifs tous les 3m '
' variables locales '
' p : profondeur dans la fonction tf qui calcule PPN2
' t : temps dans la fonction tn2 qui calcule TN2
' periode : dans la fonction tn2
' ta : tension TN2 Ã  l'Ã©tat prÃ©cÃ©dent
' scm : variable pour passer sursatcrit(i)
*****************************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "decomp.h"

#define TEST

#ifdef TEST
int    periode   []={7    , 30   , 60   , 120 }; // période du tissu i : constantes
double sursatcrit[]={2.56 , 1.84 , 1.6  , 1.6 }; // seuil de sursaturation critique du tissu i : constantes
#else
int    periode   []={5    , 7    , 10   , 15   , 20   , 30   , 40   , 50   , 60   , 80   , 100  , 120  };
double sursatcrit[]={2.72 , 2.54 , 2.38 , 2.20 , 2.04 , 1.82 , 1.68 , 1.61 , 1.58 , 1.56 , 1.55 , 1.54 };
#endif // TEST

#define nbcompart (sizeof(periode)/sizeof(periode[0]))
#define nbpaliers 5
#define VIT_REMONT 17
#define AzotSurf 0.79

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

int Prof2Palier(double ProfActuelle);

/*******************************************************************************************/
double Prof2Press(double Profondeur)
/*******************************************************************************************/
{
  return ((Profondeur / 10) + 1) ;//* AzotSurf;
}

/*******************************************************************************************/
double fppalier(double tN2,double scm)
/*******************************************************************************************/
{
  return ((tN2 / scm) - 1) * 10;
}

/*******************************************************************************************/
double fdpalier(double Profondeur,double tN2,double scm,int period)
/*******************************************************************************************/
{
  //DEFFN dpalier(p,ta,scm,periode)
  //      LOG((tf(p) - ta) / (tf(p) - (((p          - 3) / 10) + 1) * scm)) / LOG(2)) * periode
  double Press = Prof2Press(Profondeur);
  return (log((Press - tN2) / (Press - (((Profondeur /* - 3 */) / 10) + 1) * scm)) / log(2)) * period;
}

/*******************************************************************************************/
double Ti2Tf(double Ti,double Profondeur,double Temps,int Periode)
/*******************************************************************************************/
{
  // Calcul  de la nouvelle tension d'un tissus de période P de tension Ti
  // soumis a une tension Tf pendant une durée T
  // ------------------------------------------------------------------------
  // T = Ti + (Tf-Ti)*(1-0,5^^(T/P))

  double Tf = Prof2Press(Profondeur);
  return Ti + ( Tf - Ti) * (1 - pow(0.5,((double)Temps) / (double)Periode));
}
double Ti2TfIntegr(double Ti,double ProfondeurDebut,double ProfondeurFin,double Temps,int Periode);

/*******************************************************************************************/
double Ti2TfIntegr(double Ti,double ProfondeurDebut,double ProfondeurFin,double Temps,int Periode)
/*******************************************************************************************/
{
  // Calcul  de la nouvelle tension d'un tissus de période P de tension Ti
  // soumis a une tension variable linéaire pendant une durée T
  // ------------------------------------------------------------------------
  // T = Ti + (Tf-Ti)*(1-0,5^^(T/P))
#define NBINTERV 100
  double DeltaT=Temps/NBINTERV;
  int i;

  double TfD = Prof2Press(ProfondeurDebut);
  double TfF = Prof2Press(ProfondeurFin);
  double DeltaTf=(TfF-TfD)/NBINTERV;

  for (i=1;i<=NBINTERV;i++) {
    Ti += ((TfD + i*DeltaTf) - Ti) * (1 - pow(0.5,((double)DeltaT) / (double)Periode));
  }
  return Ti;
}

struct tGroup {
  const double MaxValAzote;
  const char *GroupName;
};
tGroup Group[] = {
  {1.56,"**" },
  {1.51,"P" },
  {1.47,"O" },
  {1.42,"N" },
  {1.38,"M" },
  {1.33,"L" },
  {1.29,"K" },
  {1.24,"J" },
  {1.20,"I" },
  {1.16,"H" },
  {1.11,"G" },
  {1.07,"F" },
  {1.02,"E" },
  {0.98,"D" },
  {0.93,"C" },
  {0.89,"B" },
  {0.84,"A" }
};
#define NBGROUPS (sizeof(Group)/sizeof(Group[0]))

/*******************************************************************************************/
const char *Groupe(double AzoteC120)
/*******************************************************************************************/
{
  unsigned int i;

  for (i=0;i<NBGROUPS;i++) {
    if (AzoteC120>=Group[i].MaxValAzote)
      return Group[i].GroupName;
  }
  Fatal(99,"Groupe erreur");
  return NULL;
}

/*******************************************************************************************/
int Prof2Palier(double ProfActuelle)
/*******************************************************************************************/
{
  // Calcul de la profondeur du palier type MN90 (multiple de 3m)
  // Entre ProfActuelle et la surface, et le plus près de ProfActuelle

  for (int i=ProfActuelle/3;i<30;i+=3)
    if (ProfActuelle<=i)
      return i;
}

typedef struct {
  double sc      [nbcompart];     // Sursat du compartiment
  double tn2     [nbcompart];     // Tension d'azote du tissu i en fin de niveau j
  double dpalier [nbcompart];     // Durée du palier J pour le compartiment i
  double ppalier [nbcompart];     // Profondeur 1er palier pour le compartiment i avant remontée
} tCaract;

double CalcSaturation(double ProfDepart,double ProfArrivee,double Temps, tCaract *Caract);

/*******************************************************************************************/
double CalcSaturation(double ProfDepart,double ProfArrivee,double Temps, tCaract *Caract)
/*******************************************************************************************/
{
  double Vitesse = (ProfArrivee-ProfDepart)/Temps;
  double ProfActuelle = ProfDepart, profpalier;
  double Duree = Temps;
  double PalierDetect=0;
  int    i,pMN90;
  double dMN90;
  bool   fPalier;
  double PasT  = 1.0/60;
  double PasV  = ((ProfArrivee-ProfDepart)/Temps)*PasT;

  while (Duree>0) {
    PalierDetect=0; fPalier=false; // *Caract
    for (i=0;i<(int)nbcompart;i++) {
      // Calcul de la tN2 finale de tous les compartiments après le Pas
      Caract->tn2[i] = Ti2TfIntegr(Caract->tn2[i],ProfActuelle,ProfActuelle+PasV,PasT,periode[i]);
      //Caract->sc [i] = Caract->tn2[i] / (1 + (ProfActuelle/10));
      Caract->sc [i] = Caract->tn2[i] / (1 + (ProfActuelle/10));
      // Ainsi que la profondeur minimum théorique du palier pour chaque compartiment
      profpalier = ((Caract->tn2[i]/sursatcrit[i])-1)*10;
      if (profpalier>0) {
        fPalier=true;
        if (PalierDetect<profpalier)
          PalierDetect=profpalier;
        Caract->ppalier[i] = profpalier;
      } else {
        Caract->ppalier[i] = 0;
      }
    }
    Duree-=PasT;
    ProfActuelle += PasV;
  }
  // Calcul de la durée du palier théorique
  if (fPalier)
    pMN90 = Prof2Palier(PalierDetect);
  else
    pMN90=0;
  for (i=0;i<(int)nbcompart;i++) {
    dMN90 = fdpalier(pMN90,Caract->tn2[i],sursatcrit[i],periode[i]);
    if (dMN90>0)
      Caract->dpalier[i]=dMN90;
  }
  return pMN90;
}

/*******************************************************************************************/
void Decomp (double ProfReelle, double Temps,int Verbose, int vDesc, int vMontA,int vMontP,double pAzote)
/*******************************************************************************************/
{
  double ppN2s=pAzote;
  int i;
  double ProfFict = ProfReelle*(AzotSurf/ppN2s); // profondeur fictive (prof equivalent Azone et altitude
  double DureeDescente=ProfFict/vDesc;
  tCaract Palier[20];


  // init

  int pt=0;
  for (i=0;i<(int)nbcompart;i++) {
    Palier[pt].sc      [i]=0;     // Sursat du compartiment
    Palier[pt].tn2     [i]=1;     // Tension d'azote du tissu i en fin de niveau j
    Palier[pt].dpalier [i]=0;     // Durée du palier J pour le compartiment i
    Palier[pt].ppalier [i]=0;     // Profondeur 1er palier pour le compartiment i avant remontée
  }

  // Calcul des éléments de saturation pour la descente

  // ------------------------------------------------------
  // Calcul de la saturation après de la descente
  // Mémorisé dans Palier 0
  int pp;

  pp = CalcSaturation(0,ProfFict,DureeDescente, &Palier[pt]);

  Palier[pt+1]=Palier[pt];

  // ------------------------------------------------------
  // Calcul de la saturation juste avant la remontée
  // Mémorisé dans Palier 1
  pp = CalcSaturation(ProfFict,ProfFict,Temps-DureeDescente, &Palier[++pt]);

  Palier[pt+1]=Palier[pt];

  pp = CalcSaturation(ProfFict,pp,(ProfFict-pp)/vMontA, &Palier[++pt]);

  Palier[pt+1]=Palier[pt];

  pp = CalcSaturation(ProfFict,pp,(ProfFict-pp)/vMontA, &Palier[++pt]);

  printf ("Palier a %im, Duree=%f\n",pp);
  /*
  for (i=0;i<(int)nbcompart;i++) {
    // Calcul de la tN2 finale de tous les compartiments après la descente
    Palier[.tn2     [i]   = Ti2TfIntegr(0.8,0,ProfFict,DureeDescente,periode[i]);
    Palier.ppalier [i]   = fppalier(tn2[Palier][i],sursatcrit[i]);
    Palier.tn2     [i] = tn2     [Palier][i];
    Palier.sc      [i]   = tn2     [Palier][i] / (1 + (pt / 10));
  }
  temps -= DureeDescente;
  Palier++;

  // ------------------------------------------------------
  // Calcul de la saturation avant la remontée
  // Mémorisé dans Palier 1
  for (i=0;i<(int)nbcompart;i++) {
    // Calcul de la tN2 finale de tous les compartiments avant la remontée
    tn2[Palier][i]  = Ti2TfIntegr(tn2[Palier][i],ProfFict,ProfFict,temps,periode[i]);
    // Ainsi que la profondeur minimum théorique du palier pour chaque compartiment
    ppalier [Palier][i]   = fppalier(tn2[Palier][i],sursatcrit[i]);
    tn2     [Palier+1][i] = tn2     [Palier][i];
    sc      [Palier][i]   = tn2     [Palier][i] / (1 + (pt / 10));
  }
  Palier++;

  // ------------------------------------------------------
  // Fin de la plongée, on commence la remontée
  // Calcul du pas d'intégration (1/5s)
  // pas de remontée (1s) a 17m/mn -> (17/60)m
  // Temps du pas de remontée = 1s

#define PAS 60.0

  bool fStop=false;
  PasV = vMontA/PAS;
  PasT =  1.0/PAS;
  double durtot=0;
  pt = ProfFict; //((int)(pPal / 3) + 1) * 3;
  printf ("durtot|Prof");
    for (i=0;i<nbcompart;i++)
      printf ("|c%i-tn2|c%i-sc|c%i-Stop",periode[i],periode[i],periode[i]);
    printf ("\n");
  while (pt > 0) {

    int ProchainPalier = Prof2Palier(pt);

    // ------------------------------------------------------
    // On Remonte a 17m/mn, par pas de 1/5 de secondes
    // En vérifiant a chaque step si on doit stopper la remontée
    fStop=false;
    for (i=0;i<(int)nbcompart;i++) {
      // Calcul de la tN2 finale de tous les compartiments après le Pas
      tn2[Palier][i]  = Ti2TfIntegr(tn2[Palier][i],pt,pt-PasV,PasT,periode[i]);
      // Ainsi que la profondeur minimum théorique du palier pour chaque compartiment
      ppalier [Palier][i] = fppalier(   tn2[Palier][i],sursatcrit[i]);
      sc [Palier][i] = tn2[Palier][i] / (1 + (pt/10));
      if (tn2[i][Palier] / (1 + ProchainPalier/10) >= sursatcrit[i]) {
        dpalier [Palier][i] = fdpalier(ProchainPalier,tn2[Palier][i],sursatcrit[i],periode[i]);
      } else {
        dpalier [Palier][i] = 0;
      }
    }
    // On doit maintenant vérifier si les coeffs de sursaturation critiques sont
    // Atteints ou dépassés au prochain palier
    printf ("%6.3f|%6.3f",durtot,pt);
    for (i=0;i<nbcompart;i++) {
      printf ("|%5.3f|%5.3f",tn2[Palier][i],sc[Palier][i]);
      if (sc[Palier][i]>=sursatcrit[i]) {
        printf("|Yes");
        fStop=true;
      } else
        printf ("|No");
    }
    printf ("\n");
    if (!fStop)
      pt -= PasV;
    durtot+=PasT;
  }
    // ------------------------------------------------------
    // Recherche si un Palier est a gerer
*/
/*
  VERBOSE ("+-----------------------------------------------------------------------------+\n");
  VERBOSE ("| Parametres de la plongée : Profondeur = %3.0f metres, Temps = %3.0f minutes     |\n",ProfReelle,Temps);
  else {
    printf ("+---------------------------------+\n");
    printf ("| P = %3.0fm, T = %3.0fmn             |\n",ProfReelle,Temps);
    printf ("+ - - - - - - - - - - - - - - - - +\n");
  }

  VERBOSE ("+-----------------------------------------------------------------------------+\n");
  VERBOSE ("| Parametres de la plongée : Profondeur = %3.0f metres, Temps = %3.0f minutes     |\n",ProfReelle,Temps);
  else {
    printf ("+---------------------------------+\n");
    printf ("| P = %3.0fm, T = %3.0fmn             |\n",ProfReelle,Temps);
    printf ("+ - - - - - - - - - - - - - - - - +\n");
  }

  VERBOSE ("+-----------------------------------------------------------------------------+\n");
  VERBOSE ("| Descente : %4im/mn, Duree Descente : %4.2fmn, Duree fond : %4.2fmn          |\n",vDesc,DureeDescente,temps);
  VERBOSE ("+-----------------------------------------------------------------------------+\n");
  VERBOSE ("| Informations de saturation a la fin de la descente                          |\n");
  VERBOSE ("+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +\n");
  for (i=0;i<(int)nbcompart;i++) {
    if (tn2[i][Palier]>sursatcrit[i]) {
      VERBOSE ("| C%3.0i' | TN2 = %5.3f | Coeff Surs. Critique = %4.2f | plafond = %5.2f         |\n",periode[i],tn2[i][Palier],sursatcrit[i],ppalier1[i]*ppN2s/0.8);
    } else {
      VERBOSE ("| C%3.0i' | TN2 = %5.3f | Coeff Surs. Critique = %4.2f | plafond =  N.A          |\n",periode[i],tn2[i][Palier],sursatcrit[i]);
    }
  }
  for (i=0;i<(int)nbcompart;i++) {
    // Calcul de la tN2 finale de tous les compartiments
    // Après exposition a la profondeur 'prof' durant le temps 'temps'
    //tn2[i][Palier]  = Ti2Tf(tn2[i][Palier],ProfFict,temps,periode[i]);
    tn2[i][Palier]  = Ti2TfIntegr(tn2[i][Palier],ProfFict,ProfFict,temps,periode[i]);
//    tn2[i][Palier]  = Ti2Tf(0.8,ProfFict,temps,periode[i]);
    // Ainsi que la profondeur minimum du palier pour chaque compartiment
    // Après l'exposition ci-dessus
    ppalier1[i] = fppalier(tn2[i][Palier],sursatcrit[i]);
    // Recherche du compartiment directeur
    if (ppalier1[i]>pPal1) {
      pPal1=ppalier1[i];
      tDir1=i;
    }
  }
  if (tDir1==-1) Fatal(2,"Erreur recherche compartiment directeur");
*/
  // Calcul du palier le plus profond
/*
  pt1 = ((int)(pPal1 / 3) + 1) * 3;

  // Calcul de la profondeur moyenne
  pmoy = ProfFict - pt1 - 10;

  // ------------------------------------------------
  // Calcul de la désaturation durant la remontée
  // et jusque à l'arrivée au 1er palier théorique
  // Prise en compte de la vitesse de remontée de 15 à 17m/mn (vMontA)

  tDir2=-1;
  pPal2=-100;
  for (i=0;i<(int)nbcompart;i++) {
    // double Ti2Tf         (Ti,       Profondeur, Temps, Periode)
    tn2[i][Palier+1] = Ti2Tf(tn2[i][Palier],pmoy,((double)ProfReelle - (double)pt1) / (double)vMontA,periode[i]);
    //sc [i][Palier+1] = tn2[i][Palier+1] / (1 + (pt1 - 3) / 10);
    ppalier2[i] = fppalier(tn2[i][Palier+1],sursatcrit[i]);
    if (ppalier2[i]>pPal2) {
      pPal2=ppalier2[i];
      tDir2=i;
    }
  }
  if (tDir2==-1) Fatal(2,"Erreur recherche compartiment directeur 2");

  // Calcul du palier le plus profond
  pt2 = ((int)(pPal2 / 3) + 1) * 3;

  // ------------------------------------------------
  // Affichage des données de saturation juste avant
  // de décider de remonter au 1er palier

  VERBOSE ("+-----------------------------------------------------------------------------+\n");
  VERBOSE ("| Informations de saturation juste avant le debut de remontee                 |\n");
  VERBOSE ("+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +\n");
  for (i=0;i<(int)nbcompart;i++) {
    if (tn2[i][Palier]>sursatcrit[i]) {
      VERBOSE ("| C%3.0i' | TN2 = %4.2f | Coeff Surs. Critique = %4.2f | plafond = %5.2f          |\n",periode[i],tn2[i][Palier],sursatcrit[i],ppalier1[i]*ppN2s/0.8);
    } else {
      VERBOSE ("| C%3.0i' | TN2 = %4.2f | Coeff Surs. Critique = %4.2f | plafond =  N.A           |\n",periode[i],tn2[i][Palier],sursatcrit[i]);
    }
  }
  VERBOSE ("+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +\n");
  if (pPal1>0) {
    VERBOSE ("| Compartiment directeur %3.0i'-> Palier a %4.2fm, soit %5.2fm converti MN90     |\n",periode[tDir1],pPal1,pt1);
  } else {
    VERBOSE ("| Pas de palier a effectuer                                                   |\n");
  }
  VERBOSE ("+-----------------------------------------------------------------------------+\n");
  VERBOSE ("| Informations de saturation juste avant le palier theorique                  |\n");
  VERBOSE ("+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +\n");
  for (i=0;i<(int)nbcompart;i++) {
    if (tn2[i][Palier+1]>sursatcrit[i]) {
      VERBOSE ("| C%3.0i' | TN2 = %4.2f | Coeff Surs. Critique = %4.2f | plafond = %5.2f          |\n",periode[i],tn2[i][Palier+1],sursatcrit[i],ppalier2[i]*ppN2s/0.8);
    } else {
      VERBOSE ("| C%3.0i' | TN2 = %4.2f | Coeff Surs. Critique = %4.2f | plafond =  N.A           |\n",periode[i],tn2[i][Palier+1],sursatcrit[i]);
    }
  }
  VERBOSE ("+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +\n");
  if (pPal2>0) {
    VERBOSE ("| Compartiment directeur %3.0i'-> Palier a %4.2fm, soit %5.2fm converti MN90     |\n",periode[tDir2],pPal2,pt2);
  } else {
    VERBOSE ("| Pas de palier a effectuer                                                   |\n");
    pt2=0;
  }
  VERBOSE ("+-----------------------------------------------------------------------------+\n");

   //Palier++;
  while(pt2 > 0) {
    Palier++;
    VERBOSE ("+-----------------------------------------------------------------------------+\n");
    VERBOSE ("| %2i - Palier a %4.1fm                                                         |\n",PalierNum++,pt2);
    VERBOSE ("+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +\n");
    tmax=0;
    tDir2=-1;
    for (i=0;i<(int)nbcompart;i++) {
      if (tn2[i][Palier] / (1 + (pt2 - 3) / 10) >= sursatcrit[i]) {
        dpalier[i][Palier+1] = fdpalier(pt2,tn2[i][Palier],sursatcrit[i],periode[i]);
        VERBOSE ("| C%3.0i' | Coeff Surs. Critique = %4.2f | T=%4.2f | -> palier a %2.0fm : %5.1fmn    |\n",periode[i],sursatcrit[i],tn2[i][Palier],pt2*ppN2s/0.8,dpalier[i][Palier+1]);
      } else {
        dpalier[i][Palier+1] = 0;
        VERBOSE ("| C%3.0i' | Coeff Surs. Critique = %4.2f | T=%4.2f | -> palier a %2.0fm :     N.A    |\n"    ,periode[i],sursatcrit[i],tn2[i][Palier],pt2*ppN2s/0.8);
      }
      if (dpalier[i][Palier+1]>tmax) {
        tmax = dpalier[i][Palier+1];
        tDir2=i;
      }
    }
    if (tDir2==-1) Fatal(3,"Erreur recherche compartiment directeur");
    VERBOSE ("+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +\n");

    DureePaliers+=ceil(tmax);
    // Recherche du tissu directeur
    VERBOSE ("| Compartiment directeur : %3.0i'-> Palier de %6.2fmn, arrondi a %3.0fmn         |\n",periode[tDir2],tmax,ceil(tmax));
    else {
      printf ("| Palier %4.1fm : Duree de %5.1fmn |\n",pt2,ceil(tmax));
    }
    VERBOSE ("+-----------------------------------------------------------------------------+\n");

    // PRINT "Appuyer sur une touche pour continuer."

    for (i=0;i<(int)nbcompart;i++) {
      tn2[i][Palier+1] = Ti2Tf(tn2[i][Palier],pt2,tmax,periode[i]);
      //sc [i][Palier+1] = tn2[i][Palier+1] / (1 + (pt2 - 3) / 10);
    }
    pt2 = pt2 - 3; // Palier suivant
  }

  double DureeTotale   = DureePaliers+ceil((double)ProfReelle/(double)VIT_REMONT);
  double DureeRemontee = (double)ProfReelle/(double)VIT_REMONT;
  VERBOSE ("+-----------------------------------------------------------------------------+\n");
  VERBOSE ("| Le plongeur est sorti avec un coefficient C egal a %4.2f                     |\n",tn2[nbcompart-1][Palier] / 0.8);
  VERBOSE ("| Azote residuel C120 = %4.2f. GPS = %3s                                       |\n",tn2[nbcompart-1][Palier],Groupe(tn2[nbcompart-1][Palier]));
  if (PalierNum>1) {
    VERBOSE ("| Duree totale de remontee : %2i Palier %3imn, Remontee %3.1fmn, arrondi %5.1fmn |\n",PalierNum-1,DureePaliers,DureeRemontee,ceil(DureeTotale));
  } else {
    VERBOSE ("| Duree totale de remontee : Pas de paliers, Remontee %3.1fmn, arrondi %5.1fmn  |\n",DureeRemontee,ceil(DureeTotale));
  }
  VERBOSE ("+-----------------------------------------------------------------------------+\n");
  else {
    printf ("+ - - - - - - - - - - - - - - - - +\n");
    printf ("| C   = %6.2f                    |\n",tn2[nbcompart-1][Palier] / 0.8);
    printf ("| Azote residuel = %4.2f. GPS = %2s |\n",tn2[nbcompart-1][Palier],Groupe(tn2[nbcompart-1][Palier]));
    printf ("| DTR = %5.1fmn                   |\n",ceil(DureeTotale));
    printf ("+---------------------------------+\n");
  }
*/
}
// INPUT "Profondeur de la prochaine plongÃ©e ";prof
// prof = prof * (0.8 / ppN2s) // profondeur fictive
// INPUT "Intervalle ( en minutes ) ";inter
// PRINT
// tfs = FN tn2(tn2(4,j),0,inter,120)
// majo = ROUND((LOG((FN tf(prof) - 0.8) / (FN tf(prof) - tfs)) / LOG(2)) * 120,2)
// PRINT "La majoration sera Ã©gale Ã  ";majo;" mn."
