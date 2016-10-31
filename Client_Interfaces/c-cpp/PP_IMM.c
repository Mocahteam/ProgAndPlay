#include "PP_Client.h"
#include "PP_Client_Private.h"
#include "PP_IMM.h"
#include "PP_Error.h"
#include "constantList_KP4.4.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void PP_Initialisation(void){
	// initialisation du tirage aléatoire
	srand ( time(NULL) );
	// initialisation de l'API
	if (PP_Open() == -1){
		printf("Erreur : PP_Initialisation => Le jeu n'est peut être pas lance.\n  \
%s", PP_GetError());
		return;
	}
}

void PP_Fin(void){
	if (PP_Close() == -1){
		printf("Erreur : PP_Fin => Le jeu n'est peut être pas lance.\n  %s",
PP_GetError());
	}
}

float PP_NombreAleatoire(float max){
	return (float)(rand() % (int)max);
}

bool PP_PartieTerminee(void){
	int ret = PP_IsGameOver();
	if (ret == -1)
		printf("  %s", PP_GetError());
	return ret > 0;
}

void PP_ObtenirPions(PP_Pion pions_j [], int* taille_pj,
                     PP_Pion pions_e [], int* taille_pe){
	int i;
	PP_Unit u;
	enterCriticalSection();
		if (pions_j != NULL && taille_pj != NULL){
			*taille_pj = 0;
			for (i = 0 ; i < PP_GetNumUnits(MY_COALITION) && i < NB_MAX_PIONS ; i++){
				u = PP_GetUnitAt(MY_COALITION, i);
				if (u != -1){
					pions_j[*taille_pj] = u;
					(*taille_pj)++;
				}
			}
		}
		if (pions_e != NULL && taille_pe != NULL){
			*taille_pe = 0;
			for (i = 0 ; i < PP_GetNumUnits(ENEMY_COALITION) && i < NB_MAX_PIONS ; i++){
				u = PP_GetUnitAt(ENEMY_COALITION, i);
				if (u != -1){
					pions_e[*taille_pe] = u;
					(*taille_pe)++;
				}
			}
		}
	exitCriticalSection();
}

void PP_Deplacer(PP_Pion pion, PP_Pos pos){
	if (PP_Unit_ActionOnPosition(pion, MOVE, pos) == -1)
		printf("  %s", PP_GetError());
}

void PP_Attaquer(PP_Pion source, PP_Pion cible){
	if (PP_Unit_ActionOnUnit(source, ATTACK, cible) == -1)
		printf("  %s", PP_GetError());
}

void PP_Stopper(PP_Pion p){
	if (PP_Unit_UntargetedAction(p, STOP, -1.0) == -1)
		printf("  %s", PP_GetError());
}

float PP_CapitalSante(PP_Pion p){
	float cur = PP_Unit_GetHealth(p);
	if (cur == -1){
		printf("  %s", PP_GetError());
		return 0.0;
	}
	float max = PP_Unit_GetMaxHealth(p);
	if (max == -1.0){
		printf("  %s", PP_GetError());
		return 0.0;
	}
	return cur/max;
}

PP_Pos PP_Position(PP_Pion p){
	PP_Pos ret = PP_Unit_GetPosition(p);
	if (ret.x == -1.0 || ret.y == -1.0){
		printf("  %s", PP_GetError());
	}
	return ret;
}

bool PP_EnDeplacement(PP_Pion pion, PP_Pos* pos){
	PP_PendingCommands pdgCmds;
	if (PP_Unit_GetPendingCommands(pion, &pdgCmds) == -1){
		printf("  %s", PP_GetError());
		return false;
	}
	if (pdgCmds.nbCmds > 0)
		if (pdgCmds.cmd[0].code == MOVE){
			if (pdgCmds.cmd[0].nbParams == 3){
				if (pos != NULL){
					pos->x = pdgCmds.cmd[0].param[0];
					pos->y = pdgCmds.cmd[0].param[2];
				}
			}
			else{
				if (pos != NULL){
					pos->x = -1.0;
					pos->y = -1.0;
				}
			}
			return true;
		}
	return false;
}

bool PP_EnAttaque(PP_Pion p, PP_Pion* c){
	PP_PendingCommands pdgCmds;
	if (PP_Unit_GetPendingCommands(p, &pdgCmds) == -1){
		printf("  %s", PP_GetError());
		return false;
	}
	if (pdgCmds.nbCmds > 0)
		if (pdgCmds.cmd[0].code == ATTACK){
			if (pdgCmds.cmd[0].nbParams == 1){
				if (c != NULL)
					*c = pdgCmds.cmd[0].param[0];
			}
			else{
				if (c != NULL)
					*c = -1;
			}
			return true;
		}
	return false;
}

bool PP_EnAttente(PP_Pion p){
	PP_PendingCommands pdgCmds;
	if (PP_Unit_GetPendingCommands(p, &pdgCmds) == -1){
		printf("  %s", PP_GetError());
		return false;
	}
	return pdgCmds.nbCmds == 0;
}