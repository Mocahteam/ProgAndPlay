#include "../PP_Supplier.h"
#include "../PP_Error.h"
#include "constantList_KP4.1.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread.hpp>

int main (){
	int tmp;
	// initialisation du jeu
	std::cout << "PP_Init" << std::flush;
	if (PP_Init() == -1) std::cout << " " << PP_GetError();
	else std::cout << std::endl;

// 	std::cout << "PP_Quit" << std::flush;
// 	if (PP_Quit() == -1) std::cout << " " << PP_GetError();
// 	else std::cout << std::endl;

	std::cout << "PP_SetGameOver : 0" << std::flush;
	if (PP_SetGameOver(0) == -1) std::cout << PP_GetError();
	else std::cout << std::endl;
	
	// définition de position
	std::cout << "PP_SetStaticData : " << std::flush;
	PP_Pos mapSize;
	mapSize.x = 100.0;
	mapSize.y = 100.0;
	PP_Pos startPos;
	startPos.x = 500.0;
	startPos.y = 500.0;
	PP_Positions specialAreas;
	specialAreas.size = 2;
	specialAreas.pos = (PP_Pos*) malloc(2*sizeof(PP_Pos));
	if (specialAreas.pos == NULL)
		std::cout << "Ressources allocation error" << std::endl;
	else{
		specialAreas.pos[0].x = 10.0;
		specialAreas.pos[0].y = 15.0;
		specialAreas.pos[1].x = 20.0;
		specialAreas.pos[1].y = 25.0;
		if (PP_SetStaticData(mapSize, startPos, specialAreas) == -1) std::cout << PP_GetError();
		else std::cout << std::endl;
		free(specialAreas.pos);
	}
	
	std::cout << "PP_SetRessources : " << std::flush;
	PP_Resources resources;
	resources.size = 2;
	resources.resource = (int*) malloc(sizeof(int)*2);
	if (resources.resource == NULL){
		std::cout << "Ressources allocation error" << std::endl;
	} else {
		resources.resource[0] = 900;
		resources.resource[1] = 1050;
		if (PP_SetRessources(resources) == -1) std::cout << PP_GetError();
		else std::cout << std::endl;
		free(resources.resource);
	}
	
	std::cout << "PP_AddUnit : " << std::flush;
	PP_ShortUnit unit;
	unit.id = 37;
	unit.coalition = MY_COALITION;
	unit.type = 5;
	unit.pos.x = 60;
	unit.pos.y = 120;
	unit.health = 100;
	unit.maxHealth = 100;
	unit.group= -2;
	unit.nbCommandQueue = 1;
	unit.commandQueue = (PP_Command*) malloc(sizeof(PP_Command));;
	if (unit.commandQueue != NULL){
		unit.commandQueue[0].code = 99;
		unit.commandQueue[0].nbParam = 1;
		unit.commandQueue[0].param = (float*) malloc(sizeof(float));;
		if (unit.commandQueue[0].param != NULL){
			unit.commandQueue[0].param[0] = 10.3;
			if (PP_AddUnit(unit) == -1) std::cout << PP_GetError();
			else std::cout << std::endl;
		}
		else
			std::cout << "Ressources allocation error 2" << std::endl;
	} else
		std::cout << "Ressources allocation error 1" << std::endl;
	
	int ret;
	
	boost::posix_time::ptime t1 = boost::posix_time::second_clock::local_time();
	boost::posix_time::ptime t2 = t1 + boost::posix_time::seconds(20);
	srand ( time(NULL) );
	while (t1 < t2){
		boost::this_thread::sleep(boost::posix_time::milliseconds(250));
		unit.commandQueue[0].param[0] = rand() * 1000;
		std::cout << "PP_UpdateUnit : " << unit.commandQueue[0].param[0] << std::flush;
		if (PP_UpdateUnit(unit) == -1) std::cout << PP_GetError();
		else std::cout << std::endl;
		t1 = boost::posix_time::second_clock::local_time();
	}
	std::cout << "PP_RemoveUnit : " << std::flush;
	ret = PP_RemoveUnit(unit.id);
	if (ret == -1) std::cout << PP_GetError();
	else std::cout << std::endl;
	
	std::cout << "PP_IsStored : " << std::flush;
	ret = PP_IsStored(unit.id);
	if (ret == -1) std::cout << PP_GetError();
	else std::cout << ret << std::endl;
	
	std::cout << "PP_SetGameOver : 1" << std::flush;
	if (PP_SetGameOver(1) == -1) std::cout << PP_GetError();
	else std::cout << std::endl;
	
/*	std::cout << "Entrer une valeur pour continuer : " << std::flush;
	scanf("%d", &tmp);
	
	std::cout << "PP_GetPendingCommands : " << std::flush;
	PP_PendingCommands* ppc = PP_GetPendingCommands();
	if (ppc == NULL) std::cout << PP_GetError();
	else{
		std::cout << ppc->size <<std::endl;
		for (int i = 0 ; i < ppc->size ; i++){
			std::cout << "    Code of command " << i << " of unit " << ppc->pendingCommand[i].unitId << " : " << ppc->pendingCommand[i].command.code << std::endl;
		}
	}
	
	std::cout << "PP_FreePendingCommands : " << std::flush;
	PP_FreePendingCommands(ppc);
	std::cout << std::endl;
	
	std::cout << "Entrer une seconde valeur pour continuer : " << std::flush;
	scanf("%d", &tmp);*/
	
	std::cout << "PP_Quit" << std::endl;
	if (PP_Quit() == -1) std::cout << PP_GetError();
	else std::cout << ret << std::endl;
}
