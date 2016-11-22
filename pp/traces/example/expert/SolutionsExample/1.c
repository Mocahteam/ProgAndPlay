#include "PP_Client.h"
#include "../constantList_KP4.1.h"

// déplacer une unité à une position précise
int main (){
	// définition de la position cible
	PP_Pos p;
	p.x = 1983.0;
	p.y = 1279.0;
	// ouverture du jeu
	PP_Open();
	// récupérer la première unité
	PP_Unit u = PP_GetUnitAt(MY_COALITION, 0);
	// ordonner à mon unité courante de se déplacer
	PP_Unit_ActionOnPosition(u, MOVE, p);
	// fermer le jeu
	PP_Close();
}
