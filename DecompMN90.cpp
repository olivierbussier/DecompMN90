/******************************************************************************/
/******************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ufatal.h"
#include "uutil.h"

#include "DecompMN90.h"

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
#define C120 11

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
  {0.84,"A" },
  {0.89,"B" },
  {0.93,"C" },
  {0.98,"D" },
  {1.02,"E" },
  {1.07,"F" },
  {1.11,"G" },
  {1.16,"H" },
  {1.20,"I" },
  {1.24,"J" },
  {1.29,"K" },
  {1.33,"L" },
  {1.38,"M" },
  {1.42,"N" },
  {1.47,"O" },
  {1.51,"P" },
  {1.55,"**" }, // Normalement, on ne peut pas sortir de l'eau avec un C120>1,54
};
#define NBGROUPS (sizeof(Group)/sizeof(Group[0]))

/*******************************************************************************************/
const char *Groupe(double AzoteC120)
/*******************************************************************************************/
{
  unsigned int i;

  for (i=0;i<NBGROUPS;i++) {
    if (AzoteC120<Group[i].MaxValAzote)
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

/*******************************************************************************************/
int RechercheDirecteur(tCaract *Caract)
/*******************************************************************************************/
{
  int ProfPalier=0;
  int TissusDirecteur=-1;
  int DureePalier=0;
  int i;

  for (i=0;i<(int)nbcompart;i++) {
    if (Caract->profMN90[i]>0) {
      if (ProfPalier<Caract->profMN90[i]) {
        ProfPalier  = Caract->profMN90[i];
        DureePalier = Caract->DureePalier[i];
        TissusDirecteur = i;
      }
      if (ProfPalier == Caract->profMN90[i]) {
        if (DureePalier<Caract->DureePalier[i]) {
          DureePalier = Caract->DureePalier[i];
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

      Caract->pN2Prof[i]      = (ProfFinale/10+1)*AzotSurf;
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
    TempsPalier = ceil(fdpalier(prf, Caract->tn2[i],sursatcrit[i],periode[i])*60);
    if (prf>0 && TempsPalier>0) {
      // TempsPalier = ceil(fdpalier(prf, Caract->tn2[i],sursatcrit[i],periode[i])*60);
      Caract->DureePalier[i] = TempsPalier;
    } else {
      Caract->DureePalier[i] = 0;
    }
  }
  return RechercheDirecteur(Caract);
}

/*******************************************************************************************/
char *Decomp (double ProfReelle, double Temps,int Verbose, int vDesc, int vMontA,int vMontP,double pAzote)
/*******************************************************************************************/
{
  double ppN2s=pAzote;
  int i,pt,td;
  double ProfFict = ProfReelle*(AzotSurf/ppN2s); // profondeur fictive (prof equivalent Azone et altitude
  tCaract Palier[200];
  int DureeRemont;
  int NextPalier, ProfPalier;
  char *buffer;

  int TempsPalier;
  int TempsSec      = Temps*60;
  int DureeDescente = ProfFict*60/vDesc;

  // init des pressions d'azote de départ

  buffer = strinit();
  buffer = str (buffer, "Paramètres:\r\n");
  buffer = str (buffer," - Profondeur = %6.2fm, Durée = %6.2fmn\r\n",ProfReelle,Temps);
  buffer = str (buffer," - Pression Azote en surface = %5.3f\r\n",pAzote);
  buffer = str (buffer," - Vitesse de Descente = %im/mn\r\n",vDesc);
  buffer = str (buffer," - Vitesse Remontee avant paliers = %im/mn\r\n",vMontA);
  buffer = str (buffer," - Vitesse Remontee pendant les paliers = %im/mn\r\n",vMontP);
  buffer = str (buffer,"----------------------------------------------------------------------------\r\n");
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

  CalcSaturation(0,ProfFict,DureeDescente, &Palier[pt]);
  Palier[pt].Profondeur=ProfFict;
  Palier[pt].Temps     =DureeDescente;
  Palier[pt+1]=Palier[pt];
  pt++;
  // ------------------------------------------------------
  // Calcul de la saturation juste avant la remontée
  // Mémorisé dans Palier 1
  td = CalcSaturation(ProfFict,ProfFict,TempsSec /*- DureeDescente*/, &Palier[pt]);
  if (td==-1)
    ProfPalier = 0;
  else
    ProfPalier = Palier[pt].profMN90[td];
  Palier[pt].Profondeur=ProfFict;
  Palier[pt].Temps     =TempsSec;
  Palier[pt+1]=Palier[pt];

  // ------------------------------------------------------
  // On amorce la remontée jusqu'au palier calculé
  // Calcul temps de remontée au 1er palier (secondes)

  int DTR = 0;
  DureeRemont     = (int)ceil(((double)((ProfFict-ProfPalier)*60.))/vMontA);
  DTR+=DureeRemont;

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
      buffer = str (buffer,"Palier : %imn a %.2im\r\n",(int)ceil((double)TempsPalier/60),ProfPalier);
      if (Verbose)
        for (int x=0;x<(int)nbcompart;x++) {
          if (x==td)
            buffer = str(buffer," ->");
          else
            buffer = str(buffer," - ");
          buffer = str (buffer,"Compartiment%3imn plafond=%6.2fm, palier MN90=%2im, Duree=%4is/%5.1fmn\r\n",periode[x],Palier[pt].profMin[x],Palier[pt].profMN90[x],Palier[pt].DureePalier[x],ceil((double)Palier[pt].DureePalier[x]/60));
        }
      td = CalcSaturation(ProfPalier,ProfPalier,TempsPalier, &Palier[pt]);
      DTR+=(ceil(TempsPalier/60.0)*60);
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
      //buffer = str (buffer,"Remontee a %im\r\n",NextPalier);
      DureeRemont = (int)ceil(((double)((ProfPalier-NextPalier)*60.))/vMont);
      td = CalcSaturation(ProfPalier,NextPalier,DureeRemont, &Palier[pt]);
      DTR+=DureeRemont;
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
  buffer = str (buffer,"DTR = %4.1fmn\r\n",ceil(DTR/60.0));
  buffer = str (buffer,"Azote résiduel C120 = %fmn -> GPS = %s\r\n",Palier[pt].tn2[C120],Groupe(Palier[pt].tn2[C120]));
  return buffer;
}
