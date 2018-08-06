#include "../PP_Client_Private.h"
#include "../PP_Error.h"
#include "constantList_KP4.7.h"
#include <iostream>
#include "stdio.h"

int main (){
	/*std::cout << "Open Prog&Play" << std::endl;
	PP_Open_prim();
	while (!PP_IsGameOver_prim()){
		// parcours de toutes mes unités
		for (int i = 0 ; i < PP_GetNumUnits_prim(MY_COALITION) ; i++){
			// récupération de l'unité courante
			PP_Unit u = PP_GetUnitAt_prim(MY_COALITION, i);
			// display command params
			enterCriticalSection();
				if (PP_Unit_GetNumPdgCmds_prim(u) > 0){
					//std::cout << "code : " << PP_Unit_PdgCmd_GetCode(u, 0) << std::endl;
					//std::cout << "nbParams : " << PP_Unit_PdgCmd_GetNumParams(u, 0) << std::endl;
					std::cout << PP_Unit_PdgCmd_GetParam_prim(u, 0, 0) << std::endl;
				}
			exitCriticalSection();
		}
	}
	std::cout << "Close Prog&Play" << std::endl;
	PP_Close_prim();*/
	
	
	// définition de la position cible
	PP_Pos p;
	p.x = 100.0;
	p.y = 100.0;
	// ouverture du jeu
	std::cout << "PP_Open_prim" << std::flush;
	if (PP_Open_prim() < 0) std::cout << " " << PP_GetError();
	else std::cout << std::endl;
 //	std::cout << "PP_Close_prim" << std::flush;
 //	if (PP_Close_prim() < 0) std::cout << " " << PP_GetError();
 //	else std::cout << std::endl;
	std::cout << "PP_IsGameOver_prim : " << std::flush;
	int tmp;
	if ((tmp = PP_IsGameOver_prim()) < 0) std::cout << PP_GetError();
	else std::cout << tmp << std::endl;
	std::cout << "PP_GetMapSize_prim : " << std::flush;
	tmp = PP_GetMapSize_prim(&p);
	if (tmp < 0) std::cout << PP_GetError();
	else std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
	std::cout << "PP_GetStartPosition_prim : " << std::flush;
	tmp = PP_GetStartPosition_prim(&p);
	if (tmp < 0) std::cout << PP_GetError();
	else std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
	std::cout << "PP_GetNumSpecialAreas_prim : " << std::flush;
	int size = PP_GetNumSpecialAreas_prim();
	if (size < 0) std::cout << PP_GetError();
	else{
		std::cout << size << std::endl;
		for (int i = 0 ; i < size ; i++){
			std::cout << "  PP_GetSpecialAreaPosition_prim de " << i << " : " << std::flush;
			tmp = PP_GetSpecialAreaPosition_prim(i, &p);
			if (tmp < 0) std::cout << PP_GetError();
			else std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
		}
	}
	for (int i = 0 ; i < 3 ; i++){
		std::cout << "Coalition " << i << " : " << std::endl;
		std::cout << "  PP_GetNumUnits_prim : " << std::flush;
		size = PP_GetNumUnits_prim((PP_Coalition)i);
		if (size < 0) std::cout << PP_GetError();
		else{
			std::cout << size << std::endl;
			for (int j = 0 ; j < size ; j++){
				std::cout << "    PP_GetUnitAt_prim " << j << " : " << std::flush;
				PP_Unit u = PP_GetUnitAt_prim((PP_Coalition)i, j);
				if (u < 0) std::cout << PP_GetError();
				else{
					std::cout << u << std::endl;
					std::cout << "      PP_Unit_GetCoalition_prim : " << std::flush;
					if ((tmp = PP_Unit_GetCoalition_prim(u)) < 0) std::cout << PP_GetError();
					else std::cout << tmp << std::endl;
					std::cout << "      PP_Unit_GetType_prim : " << std::flush;
					if ((tmp = PP_Unit_GetType_prim(u)) < 0) std::cout << PP_GetError();
					else std::cout << tmp << std::endl;
					std::cout << "      PP_Unit_GetPosition_prim : " << std::flush;
					if ((tmp = PP_Unit_GetPosition_prim(u, &p)) < 0) std::cout << PP_GetError();
					else std::cout << "(" << p.x << ", " << p.y << ")" << std::endl;
					std::cout << "      PP_Unit_GetHealth_prim : " << std::flush;
					if ((tmp = PP_Unit_GetHealth_prim(u)) < 0) std::cout << PP_GetError();
					else std::cout << tmp << std::endl;
					std::cout << "      PP_Unit_GetMaxHealth_prim : " << std::flush;
					if ((tmp = PP_Unit_GetMaxHealth_prim(u)) < 0) std::cout << PP_GetError();
					else std::cout << tmp << std::endl;
					std::cout << "      PP_Unit_GetGroup_prim : " << std::flush;
					if ((tmp = PP_Unit_GetGroup_prim(u)) < 0) std::cout << PP_GetError();
					else std::cout << tmp << std::endl;
					std::cout << "      PP_Unit_SetGroup_prim " << std::flush;
					if (PP_Unit_SetGroup_prim(u, 0) < 0) std::cout << " " << PP_GetError();
					else std::cout << std::endl;
					
					// Test récupération action en attente global
					std::cout << "      PP_Unit_GetPendingCommands_prim " << std::flush;
					PP_PendingCommands cmd;
					if (PP_Unit_GetPendingCommands_prim(u, &cmd) < 0) std::cout << PP_GetError();
					else{
						std::cout << cmd.nbCmds << std::endl;
						for (int i = 0 ; i < cmd.nbCmds ; i++){
							std::cout << "            code : " << cmd.cmd[i].code << "|";
							for (int j = 0 ; j < cmd.cmd[i].nbParams ; j++){
								std::cout << " " << cmd.cmd[i].param[j];
							}
						}
						std::cout << std::endl;
					}
					/*
					// Test récupération action en attente pas à pas
					std::cout << "      PP_Unit_GetNumPdgCmds_prim " << std::flush;
					enterCriticalSection();
						int ret = PP_Unit_GetNumPdgCmds_prim(u);
						if (ret < 0) std::cout << " " << PP_GetError();
						else{
							std::cout << ret << std::endl;
							for (int i =  0 ; i < ret ; i++){
								std::cout << "            code : " << PP_Unit_PdgCmd_GetCode_prim(u, i) << "|";
								for (int j = 0 ; j < PP_Unit_PdgCmd_GetNumParams_prim(u, i) ; j++){
									std::cout << " " << PP_Unit_PdgCmd_GetParam_prim(u, i, j);
								}
							}
							std::cout << std::endl;
						}
					exitCriticalSection();
					*/
					
					std::cout << "    PP_GetUnitAt_prim 1 : " << std::flush;
					PP_Unit u1 = PP_GetUnitAt_prim(MY_COALITION, 1);
					if (u1 < 0) std::cout << PP_GetError();
					else{
						std::cout << "      PP_Unit_ActionOnUnit_prim " << std::flush;
						if (PP_Unit_ActionOnUnit_prim(u, GUARD, u1) < 0) std::cout << " " << PP_GetError();
						else std::cout << std::endl;
					}
					std::cout << "      PP_Unit_ActionOnPosition_prim " << std::flush;
					p.x = 50.0;
					p.y = 50.0;
					if (PP_Unit_ActionOnPosition_prim(u, MOVE, p) < 0) std::cout << " " << PP_GetError();
					else std::cout << std::endl;
					//std::cout << "      PP_Unit_UntargetedAction_prim " << std::flush;
					//if (PP_Unit_UntargetedAction_prim(u, FIRE_STATE, 0.0) < 0) std::cout << " " << PP_GetError();
					//else std::cout << std::endl;
				}
			}
		}
	}
	std::cout << "Close Prog&Play" << std::endl;
	PP_Close_prim();
}
