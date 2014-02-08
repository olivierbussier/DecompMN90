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
double Ti2Tf      (double ta,int Profondeur,int Temps,int Periode);

void   Decomp     (int prof, int temps,int Verbose);


#endif // DECOMP_H_INCLUDED
