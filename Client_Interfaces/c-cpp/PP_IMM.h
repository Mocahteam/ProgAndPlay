#ifndef PP_IMM_H
#define PP_IMM_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NB_MAX_PIONS 48

/* Définition du type synonyme PP_Pion */
typedef int PP_Pion;

#ifndef PP_H

/* Définition de la structure position */
typedef struct {
  float x;
  float y;
} PP_Pos;

#endif

/*****************************************************************************/
/* Gestion de la bibliothèque Prog&Play                                      */
/*****************************************************************************/

/* Initialise la communication avec le jeu. Cette procédure doit être appelée en
début de programme. */
void PP_Initialisation(void);

/* Termine la communication avec le jeu. Cette procédure doit être appelée en
fin de programme. */
void PP_Fin(void);

/* Retourne une valeur tirée aléatoirement dans l'intervale [0.0;max[ */
float PP_NombreAleatoire(float max);

/* Retourne "true" si la partie est terminée et "false" sinon. La partie est
terminée lorsque tous les pions d'un des deux joueurs sont détruits. */
bool PP_PartieTerminee(void);

/*****************************************************************************/
/* Gestion des pions                                                         */
/*****************************************************************************/

/* Initialise les deux tableaux "pions_j" et "pions_e". Les pions du joueur sont
stockés dans le tableau "pions_j". Les pions ennemis VISIBLES par le joueur sont
stockés dans le tableau "pions_e". Les tailles réelles des deux tableaux après
remplissage sont respectivement écrites dans "taille_pj" et "taille_pe".
Précondition : Les tailles des tableaux "pions_j" et "pions_e" doivent être
               égales à NB_MAX_PIONS
Remarque : les données fournies dans les tableaux "pions_j" et "pions_e" restent
           valides jusqu'au prochain appel de cette procédure. */
void PP_ObtenirPions(PP_Pion pions_j[], int * taille_pj,
                     PP_Pion pions_e[], int * taille_pe);

/* Donne l'ordre au pion "pion" de se déplacer vers la position "pos". Seuls les
pions contrôlés par le joueur peuvent recevoir un tel ordre. */
void PP_Deplacer(PP_Pion pion, PP_Pos pos);

/* Donne l'ordre au pion "source" d'attaquer le pion "cible". Seuls les pions
contrôlés par le joueur peuvent recevoir un tel ordre. */
void PP_Attaquer(PP_Pion source, PP_Pion cible);

/* Donne l'ordre au pion "p" de stopper son action en cours. Seuls les pions
contrôlés par le joueur peuvent recevoir un tel ordre. */
void PP_Stopper(PP_Pion p);

/* Retourne dans l'intervalle ]0.0;1.0] le capital santé du pion "p" passé en
paramètre. La valeur retournée représente le capital santé du pion "p" sous la
forme d'un pourcentage. Si la valeur retournée est 1.0 alors le capital santé du
pion "p" est à 100%. */
float PP_CapitalSante(PP_Pion p);

/* Retourne sous la forme d'une PP_Pos la position du pion "p". */
PP_Pos PP_Position(PP_Pion p);

/* Retourne "true" si le pion "pion" est en cours de déplacement et "false"
sinon. Si le pion "pion" est en déplacement et que "destination != NULL", la
position "destination" est initialisée au point d'arrivée. Seuls les pions
contrôlés par le joueur peuvent fournir cette information. */
bool PP_EnDeplacement(PP_Pion pion, PP_Pos * destination);

/* Retourne "true" si le pion "p" a verrouillé une cible à attaquer et "false"
sinon. Si le pion "p" est engagé dans un combat et que "cible != NULL", le pion
cible est enregistré dans "cible". Seuls les pions contrôlés par le joueur
peuvent fournir cette information. */
bool PP_EnAttaque(PP_Pion p, PP_Pion * cible);

/* Retourne "true" si le pion "p" est inactif et false sinon. Seuls les pions
contrôlés par le joueur peuvent fournir cette information. */
bool PP_EnAttente(PP_Pion p);

#ifdef __cplusplus
}
#endif

#endif
