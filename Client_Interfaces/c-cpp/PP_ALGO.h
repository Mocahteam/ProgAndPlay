
#ifndef PP_ALGO
#define PP_ALGO

#include "PP_Client.h"
#include "constantList_KP4.1.h"

/****************************************/
/* Definition des mots clefs du langage */
/****************************************/
#define Debut int main (){
#define Fin return 0;}
#define Si if (
#define Alors ){
#define Sinon } else {
#define FinSi }
#define TantQue while (
#define Faire ){
#define FinTantQue }
#define Et &&
#define Ou ||
#define Non !
#define VRAI 1
#define FAUX 0

/*************************************************************/
/* Definition des operateurs d'interaction avec Kernel Panic */
/*************************************************************/
/* Pour gerer l'unite courante */
PP_Unit current_unit;
int current_id = 0;

/* local functions */
static float getPosX(){
//	PP_Refresh ();
	return PP_Unit_GetPosition(current_unit).x;
}
static float getPosY(){
//	PP_Refresh ();
	return PP_Unit_GetPosition(current_unit).y;
}

/* Ouvre la connexion avec le jeu.
   Cet operateur doit etre appele avant tout autre operateur */
#define OUVRIR_JEU PP_Open()
/* Ferme la connexion avec le jeu.
   Cet operateur doit imperativement etre le dernier operateur a etre appele
   avant la fin du programme */
#define FERMER_JEU PP_Close ()
/* Initialise l'unite courante a la premiere unite controlee par le joueur */
#define PREMIERE_UNITE current_id = 0;\
                       current_unit = PP_GetUnitAt(MY_COALITION, current_id)
/* Fait passer l'unite courante a l'unite suivante controlee par le joueur */
#define UNITE_SUIVANTE current_id ++;\
                       current_unit = PP_GetUnitAt(MY_COALITION, current_id)
/* Fournit la derniere unite controlable par le joueur */
#define DERNIERE_UNITE PP_GetUnitAt(MY_COALITION, PP_GetNumUnits (MY_COALITION)-1)
/* Indique VRAI si l'unite courante est un BIT */
#define EST_UN_BIT (PP_Unit_GetType (current_unit) == BIT)
/* Indique VRAI si l'unite courante est un BYTE */
#define EST_UN_BYTE (PP_Unit_GetType (current_unit) == BYTE)
/* Indique VRAI si l'unite courante est un ASSEMBLEUR */
#define EST_UN_ASSEMBLEUR (PP_Unit_GetType (current_unit) == ASSEMBLER)
/* Infique la coordonnee X de l'unite courante */
#define POS_X getPosX()
/* Infique la coordonnee Y de l'unite courante */
#define POS_Y getPosY()
/* Donne l'ordre a l'unite courante de se deplacer vers les coordonnees indiquees */
#define DEPLACER_VERS(xCible, yCible) {\
                               PP_Pos p;\
                               p.x = xCible;\
                               p.y = yCible;\
                               PP_Unit_ActionOnPosition (current_unit, MOVE, p);\
                            }
/* Donne l'odre a l'unite courante d'attaquer le premier ennemi */
#define ATTAQUER PP_Unit_ActionOnUnit (current_unit, ATTACK, PP_GetUnitAt(ENEMY_COALITION, 0))
					
#endif
