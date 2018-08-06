#include "PP_Client.h"
#include "../constantList_KP4.7.h"

// déplacer une unité à une position précise
int main (){
	// définition de la position cible
	PP_Pos p1;
	p1.x = 1980.0;
	p1.y = 1676.0;
	// ouverture du jeu
	PP_Open();
	// récupérer la première unité
	PP_Unit u = PP_GetUnitAt(MY_COALITION, 0);
	// ordonner à mon unité courante de se déplacer
	PP_Unit_ActionOnPosition(u, MOVE, p1, 0);
	// fermer le jeu
	PP_Close();
}
