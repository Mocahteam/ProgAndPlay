#include "PP_Client.h"
#include "../constantList_KP4.1.h"

int main (){
	int i;
	// définition de la position cible
	PP_Pos p;
	p.x = 100.0;
	p.y = 100.0;
	// ouverture du jeu
	PP_Open();
	// parcours de toutes mes unités
	for (i = 0 ; i < PP_GetNumUnits (MY_COALITION) ; i++){
		// ordonner à mon unité courante de se déplacer
		PP_Unit_ActionOnPosition(PP_GetUnitAt(MY_COALITION, i), MOVE, p);
	}
	// fermer le jeu
	PP_Close();
}
