#ifndef DECOMP_H_INCLUDED
#define DECOMP_H_INCLUDED

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

void   Decomp     (double ProfReelle, double temps,int Verbose, int vDesc, int vMontA,int vMontP,double pAzote);


#endif // DECOMP_H_INCLUDED
