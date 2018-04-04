#include <stdio.h>
#include <stdlib.h>
#include "reversi.h"

  cellule **grille=NULL;
  fleche *rose=NULL;
  int cpt=0,res;
  joueur *tabjoueurs;

int main()
{
  rose = initrose();
  grille=initGrille(0);
  tabjoueurs=initJoueurs();
  //ne rien changer avant

  while(cpt!=-1){
    affichage(grille);
    res=pose(grille,rose,tabjoueurs[cpt]);
    cpt=(cpt+1)%2;
    printf("CPT : %d\n",cpt);
    if(res==0){
      printf("AH?\n");
      cpt=checkfin(grille,rose,tabjoueurs,cpt);
    }
  }

  //ne rien changer après
  terminateGrille(grille,rose);
  return (0);
}
