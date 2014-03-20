#include <math.h>

typedef struct {
    double N2_A;
    double N2_B;
    double He_A;
    double He_B;
    double N2_HalfLife;
    double He_HalfLife;
} BuhlmannParams;

BuhlmannParams  bpars[] = {
  /* N2_A   N2_B    He_A    He_B     HL_N2  HL_He */
  {1.2599, 0.5050, 1.7424, 0.4245,   4.0,   1.51},
//{1.1696, 0.5578,      0, 0.4770,   5.0,   1.88},
  {1.0000, 0.6514, 1.3830, 0.5747,   8.0,   3.02},
  {0.8618, 0.7222, 1.1919, 0.6527,  12.5,   4.72},
  {0.7562, 0.7825, 1.0458, 0.7223,  18.5,   6.99},
  {0.6200, 0.8126, 0.9220, 0.7582,  27.0,  10.21},
  {0.5043, 0.8434, 0.8205, 0.7957,  38.3,  14.48},
  {0.4410, 0.8693, 0.7305, 0.8279,  54.3,  20.53},
  {0.4000, 0.8910, 0.6502, 0.8553,  77.0,  29.11},
  {0.3750, 0.9092, 0.5950, 0.8757, 109.0,  41.20},
  {0.3500, 0.9222, 0.5545, 0.8903, 146.0,  55.19},
  {0.3295, 0.9319, 0.5333, 0.8997, 187.0,  70.69},
  {0.3065, 0.9403, 0.5189, 0.9073, 239.0,  90.34},
  {0.2835, 0.9477, 0.5181, 0.9122, 305.0, 115.29},
  {0.2610, 0.9544, 0.5176, 0.9171, 390.0, 147.42},
  {0.2480, 0.9602, 0.5172, 0.9217, 498.0, 188.24},
  {0.2327, 0.9653, 0.5119, 0.9267, 635.0, 240.03}
};

/*

P_Cf=P_io+R[T-1/k]+[P_io-P_o-R/k][1-e^(-kT) ]

# Pio = pression initiale du gaz inerte respiré (alvéolaire) moins la vapeur d’eau
# P0 = pression du gaz inerte présente au départ dans le compartiment
# R (ou c) variation de la pression du gaz respiré en fonction de la variation de la pression ambiante
c’est simplement le taux de descente ou remontée multiplié par le pourcentage de gaz inerte
# t = temps d’exposition (ou intervalle)
# k = constante liée à la période du compartiment = ln2/ période (idem équation instantanée)

remarque : quand R (ou c) vaut zéro, l’équation ci-dessus revient à l’équation instantanée familière de la
forme : P = P0 + (Pi – P0) ( 1 – e-kt)
*/
int CalcSaturationBuhl(double po,double ProfDepart,double ProfArrivee,int Temps/*, std::list<tCaract>DiveParms*/)
{
  double k[17];
  int i;
  double R=0.79*((ProfArrivee - ProfDepart)/Temps);
  double pio=((ProfDepart/10)+1)*0.79;
  double pcf[17];

  for (i=0;i<17;i++)
    k[i] = log(2.0)/bpars[i].N2_HalfLife;

  for (i=0;i<17;i++)
    pcf[i] = pio+R*(Temps-(1.0/k[i]))+(pio-po-R/k[i])*(1-exp(-k[i]*Temps));
  return (int)pcf[1];//Warning
}
