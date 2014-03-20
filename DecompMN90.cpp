/******************************************************************************/
/******************************************************************************/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>

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
void SetParmsGraph(tGraph *tmp);
void StartGraph (void);
int CalcSaturation(double ProfDepart,double ProfArrivee,int Temps/*, std::list<tCaract>DiveParms*/);
void SetEchelle (tGraph &Graph,double EchelleX, int nbDivX, double EchelleY, int nbDivY);

std::list<tCaract> DiveParms;

/*******************************************************************************************/
void SetEchelle (tGraph *Graph,double EchelleX, int nbDivX, double EchelleY, int nbDivY)
/*******************************************************************************************/
{
  double Echelle;

  // Determine scale

  Graph->EchX = EchelleX;
  Graph->EchY = EchelleY;

  Echelle = Graph->EchX/(double)nbDivX;

  if (Echelle >= 36000)      // 10 Hour
	Echelle = 36000;
  else if (Echelle >= 18000) // 5 Hour
	Echelle = 18000;
  else if (Echelle >= 7200)  // 2 Hour
	Echelle = 7200;
  else if (Echelle >= 3600)  // 1 Hour
	Echelle = 3600;
  else if (Echelle >= 1800)  // 30 mn
	Echelle=1800;
  else if (Echelle > 600)    // 10 mn
	Echelle=600;
  else if (Echelle > 300)    // 5 Min
    Echelle=300;
  else if (Echelle > 120)    // 2 Min
    Echelle=120;
  else if (Echelle > 60)     // 1 Min
    Echelle=60;
  else
    Echelle=30;              // 30 sec

  // --- Construction des Lignes ---
  // Principe : environ 10 lignes verticales

  Graph->DivX = Echelle;
  Graph->UnitX=(char *)"mn";

  Echelle = Graph->EchY/nbDivY;

  if (Echelle >= 1000)      // 1000m
	Echelle = 1000;
  else if (Echelle >= 500)       // 500m
	Echelle = 500;
  else if (Echelle >= 200)  // 200m
	Echelle = 200;
  else if (Echelle >= 100)  // 100m
	Echelle = 100;
  else if (Echelle >= 50)   // 50m
	Echelle=50;
  else if (Echelle > 20)    // 20m
	Echelle=20;
  else if (Echelle > 10)    // 10m
    Echelle=10;
  else if (Echelle > 5)     //  5m
    Echelle=5;
  else if (Echelle > 2)     //  2m
    Echelle=2;
  else
    Echelle=1;              // 1m

  Graph->DivY = Echelle;
  Graph->UnitY=(char *)"m";

}

/*******************************************************************************************/
void StartGraph (void)
/*******************************************************************************************/
{
  tCaract tmp;
  double Profondeur=-1,pp=-1,Temps=-1,pt=-1;
  //int ProfLigne, TempsLigne;
  std::list<tCaract>::iterator it;
  tGraph Graph;

  for (it = DiveParms.begin(); it != DiveParms.end(); it++) {
    tmp = *it;
    if (tmp.Profondeur>Profondeur)
      Profondeur = tmp.Profondeur;
  }
  Temps = tmp.Temps;

  // --- Construction des echelles ---
  // 5% a droite ----
  // 5%m en dessous
  // ---------------------------------

  if (Temps<0 || Profondeur<0)
    return;

  pt = Temps      * 1.05;
  if (pt == Temps)      pt++;
  pp = Profondeur * 1.05;
  if (pp == Profondeur) pp++;

  SetEchelle(&Graph,pt,10,pp,10);

  Graph.G = &DiveParms;
  SetParmsGraph(&Graph);
}

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
int CalcSaturation(double ProfDepart,double ProfArrivee,int Temps/*, std::list<tCaract>DiveParms*/)
/*******************************************************************************************/
{
#define PAS_T  1  // En secondes

  int    Duree = 0;                                   // Duree est exprimée en secondes
  double PasV;
  int    PasT = PAS_T;
  int i,prf;
  double ProfActuelle;
  double ProfFinale;
  double TempsPalier;
  tCaract tmp;

  tmp = DiveParms.back();  // Récup des derniers éléments de saturation

  // Le principe est de vérifier a chaque pas qu'il y a
  // ou pas un palier a faire a ce niveau
  // --------------------------------------------------

  if (Temps>0) {
    PasV = ((ProfArrivee-ProfDepart)/Temps)*PAS_T;  // Le pas est la distance parcourue en 1s
    ProfActuelle = ProfDepart;
    ProfFinale = ProfDepart+PasV;
  } else {
    tmp.Profondeur=ProfArrivee;
    DiveParms.push_back(tmp);
  }

  while (Duree<Temps) {

    // Calcul des saturations pour tous les compartiments

    for (i=0;i<(int)nbcompart;i++) {

      // Calcul de la tN2 finale de tous les compartiments

      tmp.tn2[i] = Ti2TfIntegr(tmp.tn2[i],ProfActuelle,ProfFinale,PasT,periode[i]);

      // Calcul de la sursaturation

      tmp.pN2Prof[i]      = (ProfFinale/10+1)*AzotSurf;
      tmp.sursattissus[i] = tmp.pN2Prof[i]/tmp.tn2[i];
      tmp.pN2tissusMin[i] = tmp.tn2[i]/sursatcrit[i];
      tmp.profMin[i]      = ((tmp.pN2tissusMin[i]/*/0.8*/)-1)*10;
      tmp.profMN90[i]     = Prof2Palier(tmp.profMin[i]);
    }
    // Calcul des paliers
    for (i=0;i<(int)nbcompart;i++) {
      // Durée de palier
      prf = tmp.profMN90[i];
      TempsPalier = ceil(fdpalier(prf, tmp.tn2[i],sursatcrit[i],periode[i])*60);
      if (prf>0 && TempsPalier>0) {
        // TempsPalier = ceil(fdpalier(prf, Caract->tn2[i],sursatcrit[i],periode[i])*60);
        tmp.DureePalier[i] = TempsPalier;
      } else {
        tmp.DureePalier[i] = 0;
      }
    }
    tmp.Profondeur=ProfActuelle;
    tmp.Temps    +=PasT;
    DiveParms.push_back(tmp);

    Duree+=PAS_T;
    ProfActuelle += PasV;
    ProfFinale   += PasV;
  }
  return RechercheDirecteur(&tmp);
}

/*******************************************************************************************/
int Decomp (double ProfReelle, double Temps,int Verbose, int vDesc, int vMontA,int vMontP,double pAzote)
/*******************************************************************************************/
{
  double ppN2s=pAzote;
  int i,td;
  double ProfFict = ProfReelle*(AzotSurf/ppN2s); // profondeur fictive (prof equivalent Azone et altitude
  tCaract Palier;
  int DureeRemont;
  int NextPalier, ProfPalier;

  int TempsPalier;
  int TempsSec      = Temps*60;
  int DureeDescente = ProfFict*60/vDesc;

  // init des pressions d'azote de départ

  strinit();

  if (ProfReelle==0 || Temps == 0) {
    str ("Paramètres invalides\r\n");
    return 0;
  }

  DiveParms.clear();

  str ("Paramètres:\r\n");
  str (" - Profondeur  = %6.2fm\r\n",ProfReelle,Temps);
  str (" - Durée       = %6.2fmn\r\n",Temps);
  str (" - PN2 surface = %6.3f\r\n",pAzote);
  str (" - V Descente  = %3im/mn\r\n",vDesc);
  str (" - V Rem fond  = %3im/mn\r\n",vMontA);
  str (" - V paliers   = %3im/mn\r\n",vMontP);
  str ("-----------------------------------------------------------------------\r\n");

  for (i=0;i<(int)nbcompart;i++) {
    // Palier[pt].sc      [i]=0;        // Sursat du compartiment
    Palier.tn2     [i]=AzotSurf; // Tension d'azote du tissu i en fin de niveau j
    Palier.profMin [i]=0;
    Palier.DureePalier[i]=0;
    //Palier[pt].ppalier [i]=0;        // Profondeur 1er palier pour le compartiment i avant remontée
  }
  Palier.Profondeur=0;
  Palier.Temps     =0;
  DiveParms.push_back(Palier);
  // ------------------------------------------------------
  // Calcul de la saturation après de la descente
  // Mémorisé dans Palier 0

  CalcSaturation(0,ProfFict,DureeDescente/*, DiveParms*/);
  //Palier.Profondeur=ProfFict;
  //Palier.Temps     =DureeDescente;
  //DiveParms.push_back(Palier);

  // ------------------------------------------------------
  // Calcul de la saturation juste avant la remontée
  // Mémorisé dans Palier 1
  td = CalcSaturation(ProfFict,ProfFict,TempsSec /*- DureeDescente*//*, &DiveParms*/);
  Palier=DiveParms.back();
  if (td==-1)
    ProfPalier = 0;
  else
    ProfPalier = Palier.profMN90[td];

  // ------------------------------------------------------
  // On amorce la remontée jusqu'au palier calculé
  // Calcul temps de remontée au 1er palier (secondes)

  int DTR = 0;
  DureeRemont     = (int)ceil(((double)((ProfFict-ProfPalier)*60.))/vMontA);
  DTR+=DureeRemont;

  td = CalcSaturation(ProfFict,ProfPalier,DureeRemont/*, &DiveParms*/);
  Palier=DiveParms.back();

  if (td!=-1) {
    NextPalier  = Palier.profMN90[td];
    TempsPalier = Palier.DureePalier[td];
  } else {
    NextPalier=0;
    TempsPalier=0;
  }

  int vMont = vMontA; // Vitesse de remontée avant le debut des paliers

  do {
    if (NextPalier==ProfPalier) {
      // Palier confirmé
      //int TempsMinutes = (int)ceil((double)TempsPalier/60.);
      str ("Palier : %imn a %.2im\r\n",(int)ceil((double)TempsPalier/60),ProfPalier);
      str ("  DTR:%is, %6.2fmn->%i\r\n",DTR, ((double)TempsPalier)/60.0,(int)ceil(TempsPalier/60.0));
      if (Verbose)
        for (int x=0;x<(int)nbcompart;x++) {
          if (x==td)
            str(" ->");
          else
            str(" - ");
          str ("C%3imn: Min=%6.2fm, pMN90=%2im, T=%4is/%5.1fmn\r\n",periode[x],Palier.profMin[x],Palier.profMN90[x],Palier.DureePalier[x],ceil((double)Palier.DureePalier[x]/60));
        }
      td = CalcSaturation(ProfPalier,ProfPalier,TempsPalier/*, DiveParms*/);
      Palier=DiveParms.back();

      DTR+=(ceil(TempsPalier/60.0)*60);
      if (td!=-1) {
        NextPalier  = Palier.profMN90[td];
        TempsPalier = Palier.DureePalier[td];
      } else {
        NextPalier=0;
        TempsPalier=0;
      }
      vMont = vMontP; // Vitesse de remontée après le début des paliers
    } else {
      // Palier fini, on remonte au prochain
      //buffer = str (buffer,"Remontee a %im\r\n",NextPalier);
      DureeRemont = (int)ceil(((double)((ProfPalier-NextPalier)*60.))/vMont);
      td = CalcSaturation(ProfPalier,NextPalier,DureeRemont/*, DiveParms*/);
      Palier=DiveParms.back();
      DTR+=DureeRemont;
      if (td!=-1) {
        NextPalier  = Palier.profMN90[td];
        TempsPalier = Palier.DureePalier[td];
      } else {
        NextPalier=0;
        TempsPalier=0;
      }
      ProfPalier=NextPalier;
    }
  } while(ProfPalier>0);
  str ("DTR = %4.1fmn\r\n",ceil(DTR/60.0));
  str ("Azote résiduel C120 = %fmn\r\n",Palier.tn2[C120]);
  str (" -> GPS = %s\r\n",Groupe(Palier.tn2[C120]));
  StartGraph();
  return 1;
}

#include <libxml/parser.h>
#include <libxml/tree.h>
int imbric=-1;

/*******************************************************************************************/
const xmlChar *GetAttrValue(xmlAttr *x)
/*******************************************************************************************/
{
  if (x->children!=NULL)
    return x->children->content;
  else
    return (xmlChar *)"";
}

/*******************************************************************************************/
static void ExploreDive(xmlNode *a_node)
/*******************************************************************************************/
{
  xmlNode *cur_node = NULL;
  xmlAttr *cur_attr = NULL;
  char *p;
  double Time,tp=0;
  double Prof;
  int minut,secs;
  double CurProf=0;

  imbric++;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (strcmp((char *)cur_node->name,"sample")==0) {
      cur_attr = cur_node->properties;
      p=(char *)GetAttrValue(cur_attr);
      sscanf(p,"%i:%i min",&minut,&secs);
      Time = minut*60+secs;
      cur_attr = cur_attr->next;
      p=(char *)GetAttrValue(cur_attr);
      Prof = atof(p);
      CalcSaturation(CurProf,Prof,Time-tp);
      CurProf=Prof;
      tp=Time;
    }
    if (cur_node->children)
      ExploreDive(cur_node->children);
  }
  imbric--;
}

/*******************************************************************************************/
int DiveXML (char *filename,int Verbose,double pAzote)
/*******************************************************************************************/
{
  //double ppN2s=pAzote;
  int i;
  //double ProfFict = 0; //ProfReelle*(AzotSurf/ppN2s); // profondeur fictive (prof equivalent Azone et altitude
  tCaract Palier;
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;

  DiveParms.clear();

  strinit();
  str ("Paramètres:\r\n");
  str (" - fichier     = %s\r\n",filename);
  str (" - PN2 surface = %6.3f\r\n",pAzote);
  str ("--------------------------------------\r\n");

  for (i=0;i<(int)nbcompart;i++) {
    // Palier[pt].sc      [i]=0;  // Sursat du compartiment
    Palier.tn2     [i]=AzotSurf;  // Tension d'azote du tissu i en fin de niveau j
    Palier.profMin [i]=0;
    Palier.DureePalier[i]=0;
    //Palier[pt].ppalier [i]=0;   // Profondeur 1er palier pour le compartiment i avant remontée
  }
  Palier.Profondeur=0;
  Palier.Temps     =0;
  DiveParms.push_back(Palier);

  // ------------------------------------------------------
  // Lecture XML

  LIBXML_TEST_VERSION

  /*parse the file and get the DOM */

  doc = xmlReadFile(filename, NULL, XML_PARSE_NOBLANKS);

  if (doc == NULL) {
    str("error: could not parse file %s\n", filename);
    return 0;
  }

  // Get the root element node

  //xmlNode *cur_node = NULL;
  //xmlAttr *cur_attr = NULL;

  root_element = xmlDocGetRootElement(doc);

  ExploreDive(root_element);

  // init des pressions d'azote de départ

  //buffer = str (buffer,"DTR = %4.1fmn\r\n",ceil(DTR/60.0));
  //buffer = str (buffer,"Azote résiduel C120 = %fmn\r\n",Palier.tn2[C120]);
  //buffer = str (buffer," -> GPS = %s\r\n",Groupe(Palier.tn2[C120]));
  StartGraph();
  return 1;
}
