#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include "reversi.h"

#define ERREUR_getSymbole 2
#define mode 'D'
#define signal 4

//traduction du type case en caractere
char getSymbole(cellule c)
{
  switch (c) {
    case vide : return(' ');
    case bombe :
    if(mode=='J'){
      return(' ');
    }else if(mode=='D'){
      return('o');
    }
    case trou : return('X');
    case vert : return('V');
    case rouge : return('R');
    default : exit(ERREUR_getSymbole);
  }
}

//affichage d'une plateau sur la sortie standard
void affichage(cellule **plateau, int tour, int *N)
{
  //system("clear");
  printf("\nTour : %d\n\n   ",tour);
  for(int k=0;k<*N;k++){
    printf(" %d ",k);
  }
  printf(" y\n");
  for(int i=0;i<*N;i++){
    printf(" %d ",i);
    for(int j=0;j<*N;j++){
      printf("[%c]",getSymbole(plateau[i][j]));
    }
    printf("\n");
  }
  printf(" x\n\n");
}

//pour verifier si l'on va sur un bord
//renvoie 0 si c'est le cas
int checkbords(int i, int j, direction dir, int *N)
{
  if
  ((dir.hori==1 && dir.verti==1 && (i==*N-1 || j==*N-1)) //sudest
  ||(dir.hori==1 && dir.verti==-1 && (i==*N-1 || j==0))  //nordest
  ||(dir.hori==-1 && dir.verti==1 && (i==0 || j==*N-1))  //sudouest
  ||(dir.hori==-1 && dir.verti==-1 && (i==0 || j==0))    //nordouest
  ||(dir.hori==1 && dir.verti==0 && i==*N-1)             //est
  ||(dir.hori==-1 && dir.verti==0 && i==0)               //ouest
  ||(dir.hori==0 && dir.verti==1 && j==*N-1)             //sud
  ||(dir.hori==0 && dir.verti==-1 && j==0)){             //nord
    return(0);
  }
  return(1);
}

//renvoie le nombre de pion captures avec une pose dans une direction donnee
int checkcapture(cellule **plateau, int x, int y, direction dir, cellule c, int *N)
{
  int i=x,j=y;
  cellule suivante;

  if(!checkbords(i,j,dir,N))
  {
    return(0);
  }else{
    suivante=(plateau[i+dir.hori][j+dir.verti]);
    if(suivante==vide || suivante==bombe || suivante==c || suivante==trou){
      return(0);
    }
  }
  while(suivante!=c){
    i+=dir.hori;j+=dir.verti;
    if(!checkbords(i,j,dir,N))
    {
      return(0);
    }else{
      suivante=(plateau[i+dir.hori][j+dir.verti]);
      if(suivante==vide || suivante==bombe || suivante==trou){
        return(0);
      }
    }
  }
  if((i-x)!=0){
    return(abs(x-i));
  }else{
    return(abs(y-j));
  }
}

void init3x3(cellule **plateau, fleche *rose, int x, int y, int **age, int *N)
{
  direction dir;
  for(int i=0;i<8;i++){
    dir=rose[i].dir;
    if(checkbords(x,y,dir,N)){
      plateau[x+dir.hori][y+dir.verti]=vide;
      age[x+dir.hori][y+dir.verti]=0;
    }
  }
}

//explosion d'une bombe a effet aleatoire
void explosion(cellule **plateau, cellule couleur, fleche *rose, int x, int y, int **age, int *N)
{
  direction dir;
  cellule c;
  srand(time(NULL));

  printf("Bombe : ");
  switch(rand()%5){
    case 0 :
            //laser ultra puissant
            dir=rose[rand()%8].dir;
            int i=x,j=y;
            while(checkbords(i,j,dir,N)){
              i+=dir.hori;
              j+=dir.verti;
              c=plateau[i][j];
              if(c!=vide && c!=bombe && c!=trou){
                plateau[i][j]=vide;
                age[i][j]=0;
              }
            }
            plateau[x][y]=couleur;
            age[x][y]=1;
            printf("Laser!");
            break;
    case 1 :
            //change la couleur du pion qui vient d'etre pose
            //REGLE OPTIONNELLE : on capture apres ça
            //XXXXXXXXXXXXXXXXXXXXXXXXXXXXX
            //XXXCOULEURS NON GENERIQUESXXX
            //XXXXXXXXXXXXXXXXXXXXXXXXXXXXX
            if(couleur==vert){
              plateau[x][y]=rouge;
            }else{
              plateau[x][y]=vert;
            }
            age[x][y]=1;
            printf("Changement de couleur!");
            break;
    case 2 :
            //seul reste le pion joue
            init3x3(plateau,rose,x,y,age,N);
            plateau[x][y]=couleur;
            age[x][y]=1;
            printf("Survivant!");
            break;
    case 3 :
            //explosion normale + case inutilisable
            init3x3(plateau,rose,x,y,age,N);
            plateau[x][y]=trou;
            age[x][y]=0;
            printf("Mayhem!");
            break;
    case 4 :
            //explosion normale
            init3x3(plateau,rose,x,y,age,N);
            plateau[x][y]=vide;
            age[x][y]=0;
            printf("Classique!");
            break;
    default :
            printf("explosion() : valeur aleatoire incorrecte");
            exit(0);
  }
  printf("\n");
}

//capture de pions enemies
void capture(cellule **plateau, fleche *rose, int x, int y, int **age, cellule c)
{
  int dir1,dir2;
  for(int i=0;i<8;i++){
    dir1=(rose[i].dir.hori);
    dir2=(rose[i].dir.verti);
    for(int j=0;j<=rose[i].nbcases;j++){
      plateau[x+dir1*j][y+dir2*j]=c;
      age[x+dir1*j][y+dir2*j]=1;
    }
  }
}

//incremente l'age des pions, declenche les trahisons puis en gere les repercussions
//renvoie 0 s'il n'y a pas eu de trahison
int trahison(cellule **plateau, fleche *rose, int **age, int tour, int *N)
{
  int somme=0,i,j,currentage;
  cellule couleur;

  for(i=0;i<*N;i++){
    for(j=0;j<*N;j++){
      if(age[i][j]>0){
        age[i][j]++;
        somme+=age[i][j];
      }
    }
  }
  if(tour>=signal){
    srand(time(NULL));
    for(i=0;i<*N;i++){
      for(j=0;j<*N;j++){
        currentage=age[i][j];
        if(currentage>0){
          if((((float)rand()/INT_MAX)*somme) <= (float)currentage){
            couleur=plateau[i][j];
            //XXXXXXXXXXXXXXXXXXXXXXXXXXXXX
            //XXXCOULEURS NON GENERIQUESXXX
            //XXXXXXXXXXXXXXXXXXXXXXXXXXXXX
            if(couleur==vert){
              plateau[i][j]=couleur=rouge;
            }else{
              plateau[i][j]=couleur=vert;
            }
            age[i][j]=1;
            for(int k=0;k<8;k++){
              rose[k].nbcases=checkcapture(plateau,i,j,rose[k].dir,couleur,N);
            }
            capture(plateau,rose,i,j,age,couleur);
            printf("%d,%d a trahi! JUDAS!\n",i,j);
            return(1);
          }
          somme-=currentage;
        }
      }
    }
  }
  return(0);
}

//pose d'un pion
//renvoie 0 si le joueur n'a pas pu jouer
int pose(cellule **plateau, fleche *rose, joueur j, int **age, int tour, int *N)
{
  int x,y,s=0;
  cellule c=getSymbole(j.couleur);

  printf("C'est a %c de jouer\n",c);

  if(verifcouprestant(plateau,rose,j,N)==0){
    system("clear");
    printf("%c ne peux pas jouer\n",c);
    trahison(plateau,rose,age,tour,N);
    return(0);
  }

  while(s==0){
    printf("Entrez la case ou vous souhaitez jouer %c (au format x,y)\n",getSymbole(j.couleur));
    scanf("%d,%d",&x,&y);
    for(int i=0;i<8;i++){
      rose[i].nbcases=checkcapture(plateau,x,y,rose[i].dir,j.couleur,N);
      s+=rose[i].nbcases;
    }
  }

  system("clear");

  if(plateau[x][y]==bombe){
    explosion(plateau,j.couleur,rose,x,y,age,N);
  }else{
    capture(plateau,rose,x,y,age,j.couleur);
  }

  trahison(plateau,rose,age,tour,N);

  return(1);
}

//renvoie 1 si le joueur a au moins un coup jouable
int verifcouprestant(cellule **plateau, fleche *rose, joueur j, int *N)
{
  direction dir,dirinverse;
  cellule couleur=j.couleur,c;
  int x,y;

  for(int i=0;i<*N;i++){
    for(int j=0;j<*N;j++){
      c=plateau[i][j];
      if(c!=couleur && c!=bombe && c!=vide && c!=trou){
        printf("J'ai trouve un enemie en %d,%d\n",i,j);
        for(int k=0;k<8;k++){
          dir=rose[k].dir;
          if(checkbords(i,j,dir,N)){
            c=plateau[i+dir.hori][j+dir.verti];
            x=i+dir.hori;
            y=j+dir.verti;
            dirinverse=directioninverse(rose,dir);
            if((c==vide || c==bombe) && (checkcapture(plateau,x,y,dirinverse,couleur,N)>0)){
              printf("De %d,%d je peux l'avoir\n",x,y);
              return(1);
            }
            printf("Pas depuis %d,%d  ",x,y);
          }
        }
        printf("\n");
      }
    }
  }
  return(0);
}

//renvoie la direction inverse
direction directioninverse(fleche *rose, direction dir)
{
  direction dirinverse;

  dirinverse.hori=-(dir.hori);
  dirinverse.verti=-(dir.verti);

  return(dirinverse);
}

//renvoie le prochain l'indice du prochain joueur pouvant joueur
//renvoie -1 s'il n'y en a aucun
int checkfin(cellule **plateau, fleche *rose, joueur *tabjoueurs, int cpt, int *N)
{
  int nbjoueurs=2;

  for(int k=cpt;k<nbjoueurs;k++,(cpt++)%2){
    if(verifcouprestant(plateau,rose,tabjoueurs[k],N)!=0){
      return(k);
    };
  }
  return(-1);
}

//affiche le score
void scores(cellule **plateau, joueur *tabjoueurs, int *N)
{
  int nbjoueurs=2,score;
  cellule couleur;

  for(int k=0;k<nbjoueurs;k++){
    couleur=tabjoueurs[k].couleur;
    score=0;
    for(int i=0;i<*N;i++){
      for(int j=0;j<*N;j++){
        if(couleur==plateau[i][j])
          score++;
      }
    }
    printf("Le joueur %c a un score de %d\n",getSymbole(couleur),score);
  }
}
