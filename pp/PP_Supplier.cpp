/*
    This file is part of Prog&Play: A library to enable interaction between a
    strategy game and program of a player.
    Copyright 2009 Mathieu Muratet

    Prog&Play is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Prog&Play is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Mathieu Muratet
    mathieu.muratet@lip6.fr
*/

#include "PP_Supplier.h"
#include "PP_Private.h"
#include "PP_Error_Private.h"

/*****************************************************************************/
/* Defines global variables                                                  */
/*****************************************************************************/

/* Contains shared data */
static PP_sharedData shd;

/* The shared memory segment */
static boost::interprocess::managed_shared_memory *segment = NULL;

/* indicates if Prog&Play has been initialized */
static bool initialized = false;

/*****************************************************************************/
/* Internal Functions                                                        */
/*****************************************************************************/

/*
 * Delete a command queue from the shared library
 * commandQueue: the command queue to delete
 */
static void deleteCommandQueue(ShCommandVector * commandQueue){
	// destroy all parameters befor destroying the commandQueue
	ShCommandVector::iterator it2;
	for (it2 = commandQueue->begin() ; it2 != commandQueue->end() ;	it2++)
		segment->destroy_ptr(it2->param);
	segment->destroy_ptr(commandQueue);
}

/*
 * Add a command queue to the shared library
 * commandQueue: the command queue to delete
 */
static ShCommandVector * addCommandQueue(PP_ShortUnit unit){
	// creates a name for the commandQueue
	std::ostringstream oss(std::ostringstream::out);
	oss << "commandQueue" << unit.id;
	const ShCommandAllocator cQAlloc_inst(segment->get_segment_manager());
	// Builds vector into shared memory
	ShCommandVector *commandQueue = segment->construct<ShCommandVector>
		(oss.str().c_str())(cQAlloc_inst);
	// Copies elements from unit commandQueue table into this vector
	for (int j = 0 ; j < unit.nbCommandQueue ; j++){
		ShCommand shTmpCmd;
		shTmpCmd.code = unit.commandQueue[j].code;
		// creates a name for the commandQueue params
		std::ostringstream ossParams(std::ostringstream::out);
		ossParams << "commandQueue" << unit.id << "Params" << j;
		const ShFloatAllocator cQPAlloc_inst(segment->get_segment_manager());
		// Builds vector into shared memory and copies params elements
		shTmpCmd.param = segment->construct<ShFloatVector>
			(ossParams.str().c_str())(cQPAlloc_inst);
		for (int k = 0 ; k < unit.commandQueue[j].nbParam ; k++){
			shTmpCmd.param->push_back(unit.commandQueue[j].param[k]);
		}
		// adds this command to the command queue
		commandQueue->push_back(shTmpCmd);
	}
	return commandQueue;
}

static void removeUnitFromItsCoalition(ShUnit unit){
	ShIntVector &coalition = shd.coalitions[unit.coalition];
	ShIntVector::iterator it2;
	
	for (it2 = coalition.begin() ; it2 != coalition.end() ;	it2++){
		if (*it2 == unit.id){
			coalition.erase(it2);
			it2--;
		}
	}
}

/*****************************************************************************/
/* Functions to communicate with the client                                  */
/*****************************************************************************/

int PP_Init(void){
	if (!initialized){
		// creates the shared memory
		try{
			// deletes the shared memory if it existed
			boost::interprocess::shared_memory_object::remove("PP_SharedMemory");
			// creates the shared memory
			segment = new boost::interprocess::managed_shared_memory
				(boost::interprocess::open_or_create
				,"PP_SharedMemory"  //segment name
				,1000000);          //segment size in bytes
			// constructs elements of the shared memory
			shd.mutex = segment->find_or_construct<ShMutex>("mutex")();
			shd.gameOver = segment->find_or_construct<bool>("gameOver")();
			const ShMapDataAllocator mapDataAlloc_inst
				(segment->get_segment_manager());
			shd.units = segment->find_or_construct<ShMapUnits>("units")
				(std::less<PP_UnitId>(), mapDataAlloc_inst);

			const ShIntAllocator intAlloc_inst
				(segment->get_segment_manager());
			shd.coalitions = segment->find_or_construct<ShIntVector>("coalitions")
				[NB_COALITIONS]
				(intAlloc_inst);

			const ShPendingCommandAllocator commandAlloc_inst
				(segment->get_segment_manager());
			shd.pendingCommand = segment->find_or_construct<ShPendingCommands>
				("pendingCommands") (std::less<int>(), commandAlloc_inst);
				
			shd.mapSize = segment->find_or_construct<PP_Pos>("mapSize")();
			shd.startPosition = segment->find_or_construct<PP_Pos>("startPosition")();
			const ShPosAllocator posAlloc_inst(segment->get_segment_manager());
			shd.specialAreas = segment->find_or_construct<ShVectPos>("specialAreas")
				(posAlloc_inst);
			shd.resources = segment->find_or_construct<ShIntVector>("resources")
				(intAlloc_inst);
			const ShStringAllocator stringAlloc_inst(segment->get_segment_manager());
			shd.history = segment->find_or_construct<ShStringList>("history")
				(stringAlloc_inst);
		} catch (...){
			PP_SetError("PP_Init : construct shared memory error");
			delete segment;
			segment = NULL;
			boost::interprocess::shared_memory_object::remove("PP_SharedMemory");
			return -1;
		}

		// validation of initialisation
		initialized = true;
		return 0;
	}
	else{
		PP_SetError("PP_Init : Prog&Play is already initialized");
		return -1;
	}
}

int PP_Quit(void){
	if (!initialized){
		PP_SetError("PP_Quit : Prog&Play is not initialized");
		return -1;
	}
	initialized = false;
	// deletes the segment
	delete segment;
	segment = NULL;
	// deletes the shared memory
	boost::interprocess::shared_memory_object::remove("PP_SharedMemory");
	return 0;
}

int PP_SetGameOver(bool gameOver){
	if (!initialized){
		PP_SetError("PP_SetGameOver : Prog&Play is not initialized");
		return -1;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// update gameOver in shared memory
	*(shd.gameOver) = gameOver;
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
	return 0;
}

int PP_SetStaticData(PP_Pos mapSize, PP_Pos startPos,
		PP_Positions specialAreas){
	if (!initialized){
		PP_SetError("PP_SetStaticData : Prog&Play is not initialized");
		return -1;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// updates the size of the map
	shd.mapSize->x = mapSize.x;
	shd.mapSize->y = mapSize.y;
	// updates the starting position of the player
	shd.startPosition->x = startPos.x;
	shd.startPosition->y = startPos.y;
	// updates positions of special areas
	// empties all sprecialAreas
	shd.specialAreas->clear();
	PP_Pos tmp;
	for (int i = 0 ; i < specialAreas.size ; i++){
		tmp.x = specialAreas.pos[i].x;
		tmp.y = specialAreas.pos[i].y;
		shd.specialAreas->push_back(tmp);
	}
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
	return 0;
}

int PP_SetRessources(PP_Resources resources){
	if (!initialized){
		PP_SetError("PP_SetRessources : Prog&Play is not initialized");
		return -1;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// empties all resources
	shd.resources->clear();
	// update new ressources
	for (int i = 0 ; i < resources.size ; i++)
		shd.resources->push_back(resources.resource[i]);
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
	return 0;
}

int PP_AddUnit(PP_ShortUnit unit){
	int ret = 0;	
	if (!initialized){
		PP_SetError("PP_AddUnit : Prog&Play is not initialized");
		return -1;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// look for unit already added
	ShMapUnits::iterator it = shd.units->find(unit.id);
	if (it == shd.units->end()){
		// unit not found => it's OK, then add it
		ShUnit shTmpUnit;
		shTmpUnit.id = unit.id;
		shTmpUnit.coalition = unit.coalition;
		shTmpUnit.type = unit.type;
		shTmpUnit.pos = unit.pos;
		shTmpUnit.health = unit.health;
		shTmpUnit.maxHealth = unit.maxHealth;
		shTmpUnit.group = unit.group;
		shTmpUnit.commandQueue = addCommandQueue(unit);
		// inserts this unit
		std::pair<ShMapUnits::iterator,bool> res;
		res = shd.units->insert(std::make_pair(unit.id, shTmpUnit));
		// tests if insertion is ok
		if (res.second){
			// ranks this unit according to its coalition
			shd.coalitions[unit.coalition].push_back(unit.id);
		}
		else{
			PP_SetError("PP_AddUnit : unit adding fail");
			ret = -1;
		}
	}
	else{
		PP_SetError("PP_AddUnit : unit %d already added", unit.id);
		ret = -1;
	}
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
	return ret;
}

int PP_UpdateUnit(PP_ShortUnit unit){
	int ret = 0;
	if (!initialized){
		PP_SetError("PP_UpdateUnit : Prog&Play is not initialized");
		return -1;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// check if unit is already added
	ShMapUnits::iterator it = shd.units->find(unit.id);
	if (it != shd.units->end()){
		// unit found => it's OK, then update fields
		ShUnit& u = it->second;
		if (u.coalition != unit.coalition){
			// delete unit from its previous coalition
			removeUnitFromItsCoalition(u);
			// update unit coalition
			u.coalition = unit.coalition;
			// add unit to its new coalition
			shd.coalitions[u.coalition].push_back(u.id);
		}
		u.type = unit.type;
		u.pos = unit.pos;
		u.health = unit.health;
		u.maxHealth = unit.maxHealth;
		u.group = unit.group;
		// before updating commandQueue, check if it is different because
		// deleting and creating new vector into shared memory is expensive
		bool needUpdate = false;
		if (u.commandQueue->size() != unit.nbCommandQueue)
			needUpdate = true;
		else{
			for (int i = 0 ; i < unit.nbCommandQueue && !needUpdate ; i++){
				ShCommand shCom = u.commandQueue->at(i);
				PP_Command newCom = unit.commandQueue[i];
				if (shCom.code != newCom.code || shCom.param->size() != newCom.nbParam)
					needUpdate = true;
				else{
					for (int j = 0 ; j < newCom.nbParam && !needUpdate ; j++){
						if (shCom.param->at(j) != newCom.param[j])
							needUpdate = true;
					}
				}
			}
		}
		// check if update is needed
		if (needUpdate){
			deleteCommandQueue(u.commandQueue);
			u.commandQueue = addCommandQueue(unit);
		}
	}
	else{
		PP_SetError("PP_UpdateUnit : unit %d is unknown", unit.id);
		ret = -1;
	}
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
	return ret;
}

int PP_RemoveUnit(PP_UnitId unitId){
	int ret = 0;
	if (!initialized){
		PP_SetError("PP_RemoveUnit : Prog&Play is not initialized");
		return -1;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// check if unit is already added
	ShMapUnits::iterator it = shd.units->find(unitId);
	if (it != shd.units->end()){
		// unit found => it's OK, then delete it
		// first update coalition
		removeUnitFromItsCoalition(it->second);
		// second delete command queue
		deleteCommandQueue(it->second.commandQueue);
		// and then remove unit
		shd.units->erase(it);
	}
	else{
		PP_SetError("PP_RemoveUnit : unit %d is unknown", unitId);
		ret = -1;
	}
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
	return ret;
}

int PP_IsStored(PP_UnitId unitId){
	if (!initialized){
		PP_SetError("PP_RemoveUnit : Prog&Play is not initialized");
		return -1;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// check if unit is already added
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
	return (shd.units->find(unitId) != shd.units->end());
}

PP_PendingCommands* PP_GetPendingCommands(void){
	if (!initialized){
		PP_SetError("PP_GetPendingCommand : PP is not initialized");
		return NULL;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// memory allocation
	PP_PendingCommands *commands = (PP_PendingCommands*) malloc(sizeof(PP_PendingCommands));
	if (commands == NULL){
		PP_SetError("PP_GetPendingCommand : memory allocation error");
		return NULL;
	}
	commands->size = shd.pendingCommand->size();
	if (commands->size <= 0)
		commands->pendingCommand = NULL;
	else{
		commands->pendingCommand =
			(PP_PendingCommand*) malloc(commands->size*sizeof(PP_PendingCommand));
		if (commands->pendingCommand == NULL){
			PP_SetError("PP_GetPendingCommand : memory allocation error");
			free(commands);
			return NULL;
		}
		// copy all pending commands
		int i = 0;
		for (ShPendingCommands::iterator c = shd.pendingCommand->begin();
				c != shd.pendingCommand->end() ;
				c++){
			commands->pendingCommand[i].unitId = c->first;
			commands->pendingCommand[i].group = c->second.group;
			commands->pendingCommand[i].commandType = c->second.commandType;
			commands->pendingCommand[i].command.code = c->second.command.code;
			if (c->second.command.param == NULL){
				commands->pendingCommand[i].command.nbParam = 0;
				commands->pendingCommand[i].command.param = NULL;
			}
			else{
				// copy number of parameters and all parameters
				// Find vector of parameters in shared memory (do not use directly the
				// pointer c->second.command.param because it is only significant in
				// process that initialized it (i.e. the client))
				std::ostringstream oss (std::ostringstream::out);
				oss << "param" << c->first;
				ShFloatVector *p = segment->find<ShFloatVector>
					(oss.str().c_str()).first;
				// store the size of the vector
				commands->pendingCommand[i].command.nbParam = p->size();
				// allocation of parameters
				commands->pendingCommand[i].command.param =
					(float*) malloc(p->size()*sizeof(float));
				if (commands->pendingCommand[i].command.param == NULL){
					PP_SetError("PP_GetPendingCommand : memory allocation error");
					for (int j = 0 ; j < i ; j++)
						free(commands->pendingCommand[j].command.param);
					free(commands->pendingCommand);
					free(commands);
					return NULL;
				}
				// allocation ok
				int j = 0;
				for (ShFloatVector::iterator it = p->begin(); it != p->end() ; it++){
					commands->pendingCommand[i].command.param[j] = *it;
					j++;
				}
				// delete vector of parameters
				segment->destroy_ptr(p);
			}
			i++;
		}
		// Clear the vector
		shd.pendingCommand->clear();
	}
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
	return commands;
}

void PP_FreePendingCommands(PP_PendingCommands *commands){
	// free all commands
	if (commands != NULL){
		for (int i = 0 ; i < commands->size ; i++){
			// free all parameters
			free(commands->pendingCommand[i].command.param);
		}
		// free the command
		free(commands->pendingCommand);
		free(commands);
		commands = NULL;
	}
}

char * PP_PopMessage(){
	if (!initialized){
		PP_SetError("PP_PopMessage : Prog&Play is not initialized");
		return NULL;
	}
	// takes mutex
	boost::interprocess::scoped_lock<ShMutex> lock(*(shd.mutex));
	// extract first message from the shared memory
	if (shd.history->size() > 0){
		// We get message from shared memory ...
		ShString msg = shd.history->front();
		// ... this message is currently in shared memory, we only get in this process
		// a pointer to this message. Then we have to copy the content of this
		// message in this process ...
		char * copyMsg = new char [msg.length()+1];
		std::strcpy (copyMsg, msg.c_str());
		// ... We can now deleting the message form the shared memory (we have a
		// local copy) ...
		shd.history->pop_front();
		// ... and returning the local copy of this message
		return copyMsg;
	}
	else{
		return NULL;
	}
	// mutex is automatically freed when the bloc ended (thanks "scoped_lock") usefull if exception thrown
}
