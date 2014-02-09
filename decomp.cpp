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
  return ((Profondeur / 10) + 1) * AzotSurf;
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
  return (log((Press - tN2) / (Press - (((Profondeur - 3) / 10) + 1) * scm)) / log(2)) * period;
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
double Ti2TfIntegr(double Ti,double ProfondeurDebut,double ProfondeurFin,int Temps,int Periode);

/*******************************************************************************************/
double Ti2TfIntegr(double Ti,double ProfondeurDebut,double ProfondeurFin,int Temps,int Periode)
/*******************************************************************************************/
{
  // Calcul  de la nouvelle tension d'un tissus de période Periode
  // Ayant la tension d'azote initiale Ti
  // soumis a une augmentation de pression linéaire
  // pendant une durée Temps
  // ------------------------------------------------------------------------
  // T = Ti + (Tf-Ti)*(1-0,5^^(T/P))

  double TN2Depart  = Prof2Press(ProfondeurDebut);          // Pression d'azote a la profondeur initiale
  double TN2Arrivee = Prof2Press(ProfondeurFin);            // Pression d'azote a la profondeur finale
  double TN2;                                               // Pression d'azote a la profondeur courante

  int    nbinterv   = (ProfondeurFin-ProfondeurDebut)*2;    // Nombre d'intervalles d'intégration - Pas de 50cm

  double DeltaT;                                            // Incrément de temps
  double DeltaTN2;                                          // Incrément de presion

  // Ajustements si la variation de profondeur est nulle
  // Ou si le pas est négatif
  // ---------------------------------------------------

  if (nbinterv  < 0) nbinterv = -nbinterv;
  if (nbinterv == 0) nbinterv = 1;

  DeltaT     = ((double)Temps)/nbinterv/60.0;
  DeltaTN2   = (TN2Arrivee-TN2Depart)/nbinterv;

  // On choisis comme pression la pression de la
  // profondeur médiane entre les deux pas
  // ---------------------------------------------------

  TN2 = TN2Depart+(DeltaTN2/2);

  do {
    Ti   = Ti + ((TN2 - Ti) * (1 - pow(0.5,(DeltaT / (double)Periode))));
    TN2 += DeltaTN2;
  } while(--nbinterv);

  return Ti;
}

/*******************************************************************************************/
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
  int i;
  for (i=0;i<ProfActuelle;i+=3)
    ;
  return i;
}

typedef struct {
  int    TissusDirecteur;
  double tn2         [nbcompart];     // Tension d'azote du tissu i en fin de niveau j
  //double ppalier     [nbcompart];     // Plafond pour le compartiment i

  double pN2Prof     [nbcompart];
  double sursattissus[nbcompart];
  double pN2tissusMin[nbcompart];
  double profMin     [nbcompart];
  int    profMN90    [nbcompart];
  int    DureePalier [nbcompart];
} tCaract;

int    CalcSaturation(double ProfDepart,double ProfArrivee,int Temps, tCaract *Caract);
int    RechercheDirecteur(tCaract *Caract);

/*******************************************************************************************/
int RechercheDirecteur(tCaract *Caract)
/*******************************************************************************************/
{
  double palier=0;
  int TissusDirecteur=-1;
  int DureePalier=0;
  int i;

  for (i=0;i<(int)nbcompart;i++) {
    if (Caract->profMin[i]>0) {
      if (palier<=Caract->profMN90[i]) {
        if (DureePalier<Caract->DureePalier[i]) {
          palier= Caract->profMN90[i];
          TissusDirecteur = i;
        }
      }
    }
  }
  Caract->TissusDirecteur=TissusDirecteur;
  return TissusDirecteur;
}

/*******************************************************************************************/
int CalcSaturation(double ProfDepart,double ProfArrivee,int Temps, tCaract *Caract)
/*******************************************************************************************/
{
#define PAS_T  5  // En secondes

  int    Duree = Temps;                                   // Duree est exprimée en secondes
  double PasV  = ((ProfArrivee-ProfDepart)/Duree)*PAS_T;  // Le pas est la distance parcourue en 1s
  int    PasT  = PAS_T;
  int i,prf;
  double ProfActuelle = ProfDepart;
  double ProfFinale   = ProfDepart+PasV;
  double TempsPalier;

  // Le principe est de vérifier a chaque pas qu'il y a
  // ou pas un palier a faire a ce niveau
  // --------------------------------------------------

  while (Duree>0) {

    // Calcul des saturations pour tous les compartiments

    for (i=0;i<(int)nbcompart;i++) {

      // Calcul de la tN2 finale de tous les compartiments

      Caract->tn2[i] = Ti2TfIntegr(Caract->tn2[i],ProfActuelle,ProfFinale,PasT,periode[i]);

      // Calcul de la sursaturation

      Caract->pN2Prof[i]      = (ProfFinale/10+1)*0.8;
      Caract->sursattissus[i] = Caract->pN2Prof[i]/Caract->tn2[i];
      Caract->pN2tissusMin[i] = Caract->tn2[i]/sursatcrit[i];
      Caract->profMin[i]      = ((Caract->pN2tissusMin[i]/*/0.8*/)-1)*10;
      Caract->profMN90[i]     = Prof2Palier(Caract->profMin[i]);
    }

    Duree-=PAS_T;
    ProfActuelle += PasV;
    ProfFinale   += PasV;
  }
  // Calcul des paliers
  for (i=0;i<(int)nbcompart;i++) {
    // Durée de palier
    prf = Caract->profMN90[i];
    if (prf>0 && TempsPalier>0) {
      TempsPalier = ceil(fdpalier(prf, Caract->tn2[i],sursatcrit[i],periode[i])*60);
      Caract->DureePalier[i] = TempsPalier;
    } else {
      Caract->DureePalier[i] = 0;
    }
  }
  return RechercheDirecteur(Caract);
}

/*******************************************************************************************/
void Decomp (double ProfReelle, double Temps,int Verbose, int vDesc, int vMontA,int vMontP,double pAzote)
/*******************************************************************************************/
{
  double ppN2s=pAzote;
  int i,pt,td;
  double ProfFict = ProfReelle*(AzotSurf/ppN2s); // profondeur fictive (prof equivalent Azone et altitude
  tCaract Palier[20];
  int DureeRemont;
  int NextPalier, ProfPalier;

  int TempsPalier;
  int TempsSec      = Temps*60;
  // int DureeDescente = ProfFict*60/vDesc;

  // init des pressions d'azote de départ

  pt=0;
  for (i=0;i<(int)nbcompart;i++) {
    // Palier[pt].sc      [i]=0;        // Sursat du compartiment
    Palier[pt].tn2     [i]=AzotSurf; // Tension d'azote du tissu i en fin de niveau j
    Palier[pt].profMin [i]=0;
    Palier[pt].DureePalier[i]=0;
    //Palier[pt].ppalier [i]=0;        // Profondeur 1er palier pour le compartiment i avant remontée
  }

  // ------------------------------------------------------
  // Calcul de la saturation après de la descente
  // Mémorisé dans Palier 0

  // ProfPalier = CalcSaturation(0,ProfFict,DureeDescente, &Palier[pt]);

  //Palier[pt+1]=Palier[pt];

  // ------------------------------------------------------
  // Calcul de la saturation juste avant la remontée
  // Mémorisé dans Palier 1
  td = CalcSaturation(ProfFict,ProfFict,TempsSec /*-DureeDescente*/, &Palier[pt]);
  if (td==-1)
    ProfPalier = 0;
  else
    ProfPalier = Palier[pt].profMN90[td];
  Palier[pt+1]=Palier[pt];

  // ------------------------------------------------------
  // On amorce la remontée jusqu'au palier calculé
  // Calcul temps de remontée au 1er palier (secondes)

  DureeRemont     = (int)ceil(((double)((ProfFict-ProfPalier)*60.))/vMontA);
  td = CalcSaturation(ProfFict,ProfPalier,DureeRemont, &Palier[++pt]);
  if (td!=-1) {
    NextPalier  = Palier[pt].profMN90[td];
    TempsPalier = Palier[pt].DureePalier[td];
  } else {
    NextPalier=0;
    TempsPalier=0;
  }

  int vMont = vMontA; // Vitesse de remontée avant le debut des paliers

  do {
    if (NextPalier==ProfPalier) {
      // Palier confirmé
      //int TempsMinutes = (int)ceil((double)TempsPalier/60.);
      printf ("Palier : %imn a %.2im\n",(int)ceil((double)TempsPalier/60),ProfPalier);
      td = CalcSaturation(ProfPalier,ProfPalier,TempsPalier, &Palier[pt]);
      if (td!=-1) {
        NextPalier  = Palier[pt].profMN90[td];
        TempsPalier = Palier[pt].DureePalier[td];
      } else {
        NextPalier=0;
        TempsPalier=0;
      }
      vMont = vMontP; // Vitesse de remontée après le début des paliers
    } else {
      // Palier fini, on remonte au prochain
      DureeRemont = (int)ceil(((double)((ProfPalier-NextPalier)*60.))/vMont);
      td = CalcSaturation(ProfPalier,NextPalier,DureeRemont, &Palier[pt]);
      if (td!=-1) {
        NextPalier  = Palier[pt].profMN90[td];
        TempsPalier = Palier[pt].DureePalier[td];
      } else {
        NextPalier=0;
        TempsPalier=0;
      }
      ProfPalier=NextPalier;
    }
    Palier[pt+1]=Palier[pt];
    pt++;
  } while(ProfPalier>0);

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
  VERBOSE ("| Descente : %4im/mn, Duree Descente : %4.2fmn, Duree fond : %4.2fmn          |\n",vDesc,0.,Temps);
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
