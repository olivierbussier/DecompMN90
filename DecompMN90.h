#ifndef DECOMP_H_INCLUDED
#define DECOMP_H_INCLUDED

#include <list>

struct tCompart {
  const char *Name;
  const int Periode;
  const double SurSatCrit;
};

double Prof2Press (double Profondeur);
double fppalier   (double ta,double scm);
double fdpalier   (double Profondeur,double ta,double scm,int periode);
double Ti2Tf      (double ta,double Profondeur,double Temps,int Periode);
const char *Groupe(double AzoteC120);


//#define TEST

#ifdef TEST
const int    periode   []={7    , 30   , 60   , 120 }; // période du tissu i : constantes
const double sursatcrit[]={2.56 , 1.84 , 1.6  , 1.6 }; // seuil de sursaturation critique du tissu i : constantes
#else
const int    periode   []={5    , 7    , 10   , 15   , 20   , 30   , 40   , 50   , 60   , 80   , 100  , 120  };
const double sursatcrit[]={2.72 , 2.54 , 2.38 , 2.20 , 2.04 , 1.82 , 1.68 , 1.61 , 1.58 , 1.56 , 1.55 , 1.54 };
#endif // TEST

const int nbcompart = sizeof(periode)/sizeof(periode[0]);

struct tCaract {
  int    TissusDirecteur;
  double Profondeur;
  double Temps;
  double tn2         [nbcompart];     // Tension d'azote du tissu i en fin de niveau j
  double pN2Prof     [nbcompart];
  double sursattissus[nbcompart];
  double pN2tissusMin[nbcompart];
  double profMin     [nbcompart];
  int    profMN90    [nbcompart];
  int    DureePalier [nbcompart];
};

struct tGraph {
  int EchX,EchY;
  int DivX,DivY;
  char *UnitX,*UnitY;
  bool ok;
  std::list<tCaract> *G;
};

int  Decomp     (double ProfReelle, double temps,int Verbose, int vDesc, int vMontA,int vMontP,double pAzote);
int  CalcSaturation(double ProfDepart,double ProfArrivee,int Temps, tCaract *Caract);
int  RechercheDirecteur(tCaract *Caract);
int  DiveXML (char *filename,int Verbose,double pAzote);

#define nbpaliers   5
#define VIT_REMONT 17
#define AzotSurf    0.7808

#endif // DECOMP_H_INCLUDED
