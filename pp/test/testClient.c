#include "../PP_Client.h"
#include "../PP_Error.h"
#include "constantList_KP4.1.h"
#include <iostream>
#include "stdio.h"

int main (){
	PP_Open();
	while (!PP_IsGameOver()){
		// parcours de toutes mes unités
		for (int i = 0 ; i < PP_GetNumUnits(MY_COALITION) ; i++){
			// récupération de l'unité courante
			PP_Unit u = PP_GetUnitAt(MY_COALITION, i);
			// display command params
			if (PP_Unit_GetNumPdgCmds(u) > 0){
				//std::cout << "code : " << PP_Unit_PdgCmd_GetCode(u, 0) << std::endl;
				//std::cout << "nbParams : " << PP_Unit_PdgCmd_GetNumParams(u, 0) << std::endl;
				std::cout << PP_Unit_PdgCmd_GetParam(u, 0, 0) << std::endl;
			}
		}
	}
/*	// définition de la position cible
	PP_Pos p;
	p.x = 100.0;
	p.y = 100.0;
	// ouverture du jeu
	std::cout << "PP_Open" << std::flush;
	if (PP_Open() == -1) std::cout << " " << PP_GetError();
	else std::cout << std::endl;
 //	std::cout << "PP_Close" << std::flush;
 //	if (PP_Close() == -1) std::cout << " " << PP_GetError();
 //	else std::cout << std::endl;
	std::cout << "PP_IsGameOver : " << std::flush;
	int tmp;
	if ((tmp = PP_IsGameOver()) == -1) std::cout << PP_GetError();
	else std::cout << tmp << std::endl;
	std::cout << "PP_GetMapSize : " << std::flush;
	p = PP_GetMapSize();
	if (p.x == -1.0) std::cout << PP_GetError();
	else std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
	std::cout << "PP_GetStartPosition : " << std::flush;
	p = PP_GetStartPosition();
	if (p.x == -1.0) std::cout << PP_GetError();
	else std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
	std::cout << "PP_GetNumSpecialAreas : " << std::flush;
	int size = PP_GetNumSpecialAreas();
	if (size == -1) std::cout << PP_GetError();
	else{
		std::cout << size << std::endl;
		for (int i = 0 ; i < size ; i++){
			std::cout << "  PP_GetSpecialAreaPosition de " << i << " : " << std::flush;
			p = PP_GetSpecialAreaPosition(i);
			if (p.x == -1.0) std::cout << PP_GetError();
			else std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
		}
	}
	for (int i = 0 ; i < 3 ; i++){
		std::cout << "Coalition " << i << " : " << std::endl;
		std::cout << "  PP_GetNumUnits : " << std::flush;
		size = PP_GetNumUnits((PP_Coalition)i);
		if (size == -1) std::cout << PP_GetError();
		else{
			std::cout << size << std::endl;
			for (int j = 0 ; j < size ; j++){
				std::cout << "    PP_GetUnitAt " << j << " : " << std::flush;
				PP_Unit u = PP_GetUnitAt((PP_Coalition)i, j);
				if (u == -1) std::cout << PP_GetError();
				else{
					std::cout << u << std::endl;
					std::cout << "      PP_Unit_GetCoalition : " << std::flush;
					if ((tmp = PP_Unit_GetCoalition(u)) == -1) std::cout << PP_GetError();
					else std::cout << tmp << std::endl;
					std::cout << "      PP_Unit_GetType : " << std::flush;
					if ((tmp = PP_Unit_GetType(u)) == -1) std::cout << PP_GetError();
					else std::cout << tmp << std::endl;
					std::cout << "      PP_Unit_GetPosition : " << std::flush;
					p = PP_Unit_GetPosition(u);
					if (p.x == -1.0) std::cout << PP_GetError();
					else std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
					std::cout << "      PP_Unit_GetHealth : " << std::flush;
					if ((p.x = PP_Unit_GetHealth(u)) == -1.0) std::cout << PP_GetError();
					else std::cout << p.x << std::endl;
					std::cout << "      PP_Unit_GetMaxHealth : " << std::flush;
					if ((p.x = PP_Unit_GetMaxHealth(u)) == -1.0) std::cout << PP_GetError();
					else std::cout << p.x << std::endl;
					std::cout << "      PP_Unit_GetGroup : " << std::flush;
					if ((tmp = PP_Unit_GetGroup(u)) == -1) std::cout << PP_GetError();
					else std::cout << tmp << std::endl;
					std::cout << "      PP_Unit_SetGroup " << std::flush;
					if (PP_Unit_SetGroup(u, 0) == -1) std::cout << " " << PP_GetError();
					else std::cout << std::endl;
					
					std::cout << "      PP_Unit_GetNumPdgCmds " << std::flush;
					int ret = PP_Unit_GetNumPdgCmds(u);
					if (ret == -1) std::cout << " " << PP_GetError();
					else{
						std::cout << ret << std::endl;
						for (int i =  0 ; i < ret ; i++){
							std::cout << "code : " << PP_Unit_PdgCmd_GetCode(u, i) << "|";
							for (int j = 0 ; j < PP_Unit_PdgCmd_GetNumParams(u, i) ; j++){
								std::cout << " " << PP_Unit_PdgCmd_GetParam(u, i, j);
							}
						}
						std::cout << std::endl;
					}
					
					std::cout << "    PP_GetUnitAt 1 : " << std::flush;
					PP_Unit u1 = PP_GetUnitAt(MY_COALITION, 1);
					if (u1 == -1) std::cout << PP_GetError();
					else{
						std::cout << "      PP_Unit_ActionOnUnit " << std::flush;
						if (PP_Unit_ActionOnUnit(u, GUARD, u1) == -1) std::cout << " " << PP_GetError();
						else std::cout << std::endl;
					}
					std::cout << "      PP_Unit_ActionOnPosition " << std::flush;
					p.x = 50.0;
					p.y = 50.0;
					if (PP_Unit_ActionOnPosition(u, MOVE, p) == -1) std::cout << " " << PP_GetError();
					else std::cout << std::endl;
					//std::cout << "      PP_Unit_UntargetedAction " << std::flush;
					//if (PP_Unit_UntargetedAction(u, FIRE_STATE, 0.0) == -1) std::cout << " " << PP_GetError();
					//else std::cout << std::endl;
				}
			}
		}
	}
	std::cout << "PP_Close" << std::endl;
	PP_Close();*/
}
