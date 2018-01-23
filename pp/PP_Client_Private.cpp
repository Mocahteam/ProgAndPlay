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

#include "PP_Private.h"
#include "PP_Client_Private.h"
#include "PP_Error_Private.h"
#include "traces/TraceConstantList.h"

#include <signal.h>
#include <math.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread.hpp>

#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>

/******************************************************************************/
/* Definition of global variables                                             */
/******************************************************************************/

/* List of constants tags that produces specific feedback (see feedbacks.xml file and Call.cpp from trace package) */
static char const *errorsArr[] = {UNIT_NOT_FOUND_LABEL, NOT_UNIT_OWNER_LABEL, INVALID_COALITION_LABEL, TARGET_NOT_FOUND_LABEL, OUT_OF_RANGE_LABEL, INVALID_GROUP_LABEL, POSITION_OUT_OF_BOUNDS_LABEL};

/* Contains all shared data */
static PP_sharedData shd;

/* The shared memory segment */
static boost::interprocess::managed_shared_memory *segment = NULL;

/* Indicates if PP has been opened */
static bool opened = false;
/* Manages Ctrl-C signal in critical sections */
static bool ctrlC = false;
/* Indicates if mutex is already locked and how many time */
static int locked = 0;
/* Stores the number of critical section call */
static int nbCriticalCall = 0;


/******************************************************************************/
/* Internal Functions                                                         */
/******************************************************************************/

/*
 * function that manages signals. On reception of a signal defined in critical
 * section => flag the signal to take into consideration later.
 */
static void manageSignal(int signal){
	if (signal == SIGINT)
		ctrlC = true;
}

/*
 * Checks if PP is initialized.
 * fctName : name of the function that called isInitialised.
 * Returns : 0 on success. Negative value is returned on errors.
 */
static int isInitialized(std::string fctName = ""){
	if (!opened){
		PP_SetError("%s : Prog&Play is not opened\n", fctName.c_str());
		return NOT_OPENED;
	}
	return 0;
}

/*
 * Checks if parameters are correct.
 *
 * fctName : name of the function that called checkParams
 * unit : to test if this unit is valid or not
 * checkOwner : to check if "unit" is property of the player or not
 * coalition : to check if this coalition is valid or not
 * target : to check if "target" is valid or not
 * Returns : 0 on success. Negative value is returned on errors.
 */
static int checkParams(std::string fctName = "",
		const ShMapUnits::iterator * unit = NULL,
		bool checkOwner = false,
		const int * coalition = NULL,
		const ShMapUnits::iterator * target = NULL){
	int ret = isInitialized(fctName);
	if (ret < 0) return ret;
	if (unit){
		if (*unit == shd.units->end()){
			PP_SetError("%s : unit not found\n", fctName.c_str());
			return UNIT_NOT_FOUND;
		}
		if (checkOwner){
			if ((*unit)->second.coalition != MY_COALITION){
				PP_SetError("%s : you are not the unit owner\n",
					fctName.c_str());
				return NOT_UNIT_OWNER;
			}
		}
	}
	if (coalition){
		if (*coalition < 0 || *coalition >= NB_COALITIONS){
			PP_SetError("%s : coalition out of range\n", fctName.c_str());
			return INVALID_COALITION;
		}
	}
	if (target){
		if (*target == shd.units->end()){
			PP_SetError("%s : target not found\n", fctName.c_str());
			return TARGET_NOT_FOUND;
		}
	}
	return 0;
}

/*
 * Adds a command to the list of pending commands.
 * unitID : unit id
 * commandCode : command code
 * params : list of parameters of the command
 * commandType : type of the command
 * setGroup : affectation group number
 */
static void addCommand(PP_Unit unitId, int commandCode,
		const std::vector<float>& params, int commandType, int setGroup = -2){
	// builds a command
	ShPendingCommand &tmp = (*(shd.pendingCommand))[unitId];
	// Checks if this command is an affectation group number
	if (setGroup != -2){
		// set the affectation group
		tmp.group = setGroup;
	}
	if (commandType != -1){
		// set the command type
		tmp.commandType = commandType;
		// set the command code
		tmp.command.code = commandCode;
		// reset params if necessary
		if (tmp.command.param != NULL){
			// destroys old parameters
			segment->destroy_ptr(tmp.command.param);
			tmp.command.param = NULL;
		}
		// creates a name for this parameter
		std::ostringstream oss(std::ostringstream::out);
		oss << "param" << unitId;
		const ShFloatAllocator paramAlloc_inst(segment->get_segment_manager());
		// Builds vector into shared memory and copies elements from parameter
		// vector
		tmp.command.param = segment->construct<ShFloatVector>(oss.str().c_str())
			(params.begin(), params.end(), paramAlloc_inst);
	}
}

/*
 * This function has to be called before entering in critical section.
 * Don't forget to call "exitCriticalSection()" after critical section.
 */
void enterCriticalSection(){
	nbCriticalCall++;
	if (locked == 0){
		// make a pause every 8 call to avoid cpu consuming
		if (nbCriticalCall > 8){
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			nbCriticalCall = 0;
		}
		// intercept SIGINT signal in critical section
		signal(SIGINT, manageSignal);
		// takes mutex
		if (opened) // we don't use "isInitialized(...)" function because we don't want building a PP error message on error here
			shd.mutex->lock();
	}
	locked++;
}

/*
 * This function has to be called at the end of a critical section.
 */
void exitCriticalSection(){
	if (locked == 1){
		// releases the mutex
		if (opened) // we don't use "isInitialized(...)" function because we don't want building a PP error message on error here
			shd.mutex->unlock();
		// putting signal back
		signal (SIGINT, SIG_DFL);
		// if a signal has been received in critical section, resend it now
		if (ctrlC)
			raise(SIGINT);
	}
	locked--;
}

/*
 * Check if position a position is between two others.
 * Returns : true if "p" position is between "b1" and "b2" and false otherwhise
 */
static bool between(PP_Pos p, PP_Pos b1, PP_Pos b2){
	double d = (b1.x-b2.x)*(b1.x-b2.x)+(b1.y-b2.y)*(b1.y-b2.y);
	double d1 = (b1.x-p.x)*(b1.x-p.x)+(b1.y-p.y)*(b1.y-p.y);
	double d2 = (b2.x-p.x)*(b2.x-p.x)+(b2.y-p.y)*(b2.y-p.y);

	return (d1 <= d && d2 <= d);
}

/*
 * Check if unit with id "unitId" has the pending command "actionId" with the
 * parameter "param" defined
 */
int orderWithOneParamFound(PP_Unit unitId, int actionId, float param){
	bool orderFound = false;
	// Check all pending commands
	PP_PendingCommands pdgCmd;
	int ret = PP_Unit_GetPendingCommands_prim(unitId, &pdgCmd);
	for (int i = 0 ; i < pdgCmd.nbCmds ; i++){
		// Check the code of this action
		if (pdgCmd.cmd[i].code == actionId){
			// Check parameter
			if (pdgCmd.cmd[i].nbParams == 1 && pdgCmd.cmd[i].param[0] == param)
				orderFound = true;
		}
	}
	return orderFound;
}

/*
 * Check if unit with id "unitId" has the pending command "actionId" with the
 * target position "target" defined
 */
int orderOnPositionFound(PP_Unit unitId, int actionId, PP_Pos target){
	bool orderFound = false;
	// Check all pending commands
	PP_PendingCommands pdgCmd;
	int ret = PP_Unit_GetPendingCommands_prim(unitId, &pdgCmd);
	for (int i = 0 ; i < pdgCmd.nbCmds ; i++){
		// Check the code of this action
		if (pdgCmd.cmd[i].code == actionId){
			// Check parameters
			if (pdgCmd.cmd[i].nbParams >= 3 && pdgCmd.cmd[i].param[0] == target.x && pdgCmd.cmd[i].param[2] == target.y)
				orderFound = true;
		}
	}
	return orderFound;
}

/******************************************************************************/
/* Functions to communicate with the game                                     */
/******************************************************************************/

int PP_Open_prim(){
	if (!opened){
		nbCriticalCall = 0;
		try{
//#if (defined BOOST_WINDOWS) && !(defined BOOST_DISABLE_WIN32)
			//Tries to delete shared memory in the client side only with Windows
//			std::cout << boost::interprocess::shared_memory_object::remove(
//				"PP_SharedMemory") << std::endl;
//#endif
			// Opening of the shared memory
			segment = new boost::interprocess::managed_shared_memory
				(boost::interprocess::open_only, "PP_SharedMemory");

			// Finds elements in the shared memory
			shd.mutex = segment->find<ShMutex>("mutex").first;
			shd.gameOver = segment->find<bool>("gameOver").first;
			shd.gamePaused = segment->find<bool>("gamePaused").first;
			shd.tracePlayer = segment->find<bool>("tracePlayer").first;
			shd.timestamp = segment->find<int>("timestamp").first;
			shd.units = segment->find<ShMapUnits>("units").first;
			shd.coalitions = segment->find<ShIntVector>("coalitions").first;
			shd.pendingCommand =
				segment->find<ShPendingCommands>("pendingCommands").first;
			shd.mapSize = segment->find<PP_Pos>("mapSize").first;
			shd.startPosition = segment->find<PP_Pos>("startPosition").first;
			shd.specialAreas = segment->find<ShVectPos>("specialAreas").first;
			shd.resources = segment->find<ShIntVector>("resources").first;
			shd.history = segment->find<ShStringList>("history").first;
		} catch (...){
			PP_SetError("PP_Open : open shared memory error. Is the game running?\n");
			delete segment;
			segment = NULL;
//#if (defined BOOST_WINDOWS) && !(defined BOOST_DISABLE_WIN32)
			//Tries to delete shared memory in the client side only with Windows
//			std::cout << boost::interprocess::shared_memory_object::remove(
//				"PP_SharedMemory") << std::endl;
//#endif
			return SHARED_MEMORY_ERROR;
		}
		/* validation of the opening */
		opened = true;

		return 0;
	}
	else{
		if (PP_Close_prim() < 0){
			PP_SetError("PP_Open : Prog&Play is already opened, impossible to \
reload it\n");
			return RELOAD_FAILURE;
		}
		// reopening of Prog&Play
		return PP_Open_prim();
	}
}

int PP_Close_prim (){
	int ret = isInitialized("PP_Close");
	if (ret == 0){
		opened = false;
		/* deletes "segment" that enables access of data */
		delete segment;
		segment = NULL;
//#if (defined BOOST_WINDOWS) && !(defined BOOST_DISABLE_WIN32)
		// Tries to delete shared memory in the client side only with Windows
//		std::cout << boost::interprocess::shared_memory_object::remove(
//			"PP_SharedMemory") << std::endl;
//#endif
	}
	return ret;
}

int PP_IsGameOver_prim(){
	int ret;
	// Checks initialisation
	ret = isInitialized("PP_IsGameOver");
	if (ret == 0){
		enterCriticalSection();
			ret = *(shd.gameOver);
		exitCriticalSection();
	}
	return ret;
}

int PP_IsGamePaused_prim() {
	int ret;
	// Checks initialisation
	ret = isInitialized("PP_IsGamePaused");
	if (ret == 0) {
		enterCriticalSection();
			ret = *(shd.gamePaused);
		exitCriticalSection();
	}
	return ret;
}

int PP_GetTimestamp_prim() {
	int ret;
	// Checks initialisation
	ret = isInitialized("PP_GetTimestamp");
	if (ret == 0) {
		enterCriticalSection();
			ret = *(shd.timestamp);
		exitCriticalSection();
	}
	return ret;
}

int PP_GetMapSize_prim(PP_Pos * mapSize){
	// Checks initialisation
	int ret = isInitialized("PP_GetMapSize");
	if (ret == 0){
		enterCriticalSection();
			*mapSize = *(shd.mapSize);
		exitCriticalSection();
	}
	else{
		mapSize->x = ret;
		mapSize->y = ret;
	}
	return ret;
}

int PP_GetStartPosition_prim(PP_Pos * startPos){
	// Checks initialisation
	int ret = isInitialized("PP_GetStartPosition");
	if (ret == 0){
		enterCriticalSection();
			*startPos = *(shd.startPosition);
		exitCriticalSection();
	}
	else{
		startPos->x = ret;
		startPos->y = ret;
	}
	return ret;
}

int PP_GetNumSpecialAreas_prim(){
	int ret;
	ret = isInitialized("PP_GetNumSpecialAreas");
	if (ret == 0){
		enterCriticalSection();
			ret = shd.specialAreas->size();
		exitCriticalSection();
	}
	return ret;
}

int PP_GetSpecialAreaPosition_prim(int num, PP_Pos * pos){
	// Checks initialisation
	int ret = isInitialized("PP_GetSpecialAreaPosition");
	if (ret == 0){
		enterCriticalSection();
			try {
				*pos = shd.specialAreas->at(num);
			} catch (std::out_of_range e){
				PP_SetError("PP_GetSpecialAreaPosition : special area not \
found\n");
				ret = OUT_OF_RANGE;
			}
		exitCriticalSection();
	}
	if (ret != 0){
		pos->x = ret;
		pos->y = ret;
	}
	return ret;
}

int PP_GetResource_prim(PP_Resource id){
	int ret;
	ret = isInitialized("PP_GetResource");
	if (ret == 0){
		enterCriticalSection();
			try{
				ret = shd.resources->at(id);
			} catch (std::out_of_range e) {
				PP_SetError("PP_GetResource : ressource out of range\n");
				ret = OUT_OF_RANGE;
			}
		exitCriticalSection();
	}
	return ret;
}

int PP_GetNumUnits_prim(PP_Coalition c){
	int ret;
	ret = checkParams("PP_GetNumUnits", NULL, false, (int*)&c);
	if (ret == 0) {
		enterCriticalSection();
			ret = shd.coalitions[c].size();
		exitCriticalSection();
	}
	return ret;
}

PP_Unit PP_GetUnitAt_prim(PP_Coalition c, int index){
	int ret;
	ret = checkParams("PP_GetUnitAt", NULL, false, (int*)&c);
	if (ret == 0) {
		enterCriticalSection();
			try{
				// Returns directly data. If index is inaccurate, an exception
				// will be thrown
				ret = shd.coalitions[c].at(index);
			} catch (std::out_of_range e) {
				PP_SetError("PP_GetUnitAt : index out of range\n");
				ret = OUT_OF_RANGE;
			}
		exitCriticalSection();
	}
	return ret;
}

PP_Coalition PP_Unit_GetCoalition_prim(PP_Unit unit){
	int ret;
	ret = isInitialized("PP_Unit_GetCoalition");
	if (ret == 0){
		enterCriticalSection();
			// Store the unit
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetCoalition", &u);
			if (ret == 0)
				ret = u->second.coalition;
		exitCriticalSection();
	}
	return (PP_Coalition)ret;
}

int PP_Unit_GetType_prim(PP_Unit unit){
	int ret;
	ret = isInitialized("PP_Unit_GetType");
	if (ret == 0){
		enterCriticalSection();
			// Store the unit
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetType", &u);
			if (ret == 0){
				if (u->second.type == -1) {
					PP_SetError("PP_Unit_GetType : type not found\n");
					ret = TYPE_UNDEF;
				}
				else
					ret = u->second.type;
			}
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_GetPosition_prim(PP_Unit unit, PP_Pos * pos){
	int ret = isInitialized("PP_Unit_GetPosition");
	if (ret == 0){
		enterCriticalSection();
			// Store the unit
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetPosition", &u);
			if (ret == 0)
				*pos = u->second.pos;
		exitCriticalSection();
	}
	if (ret != 0){
		pos->x = ret;
		pos->y = ret;
	}
	return ret;
}

float PP_Unit_GetHealth_prim(PP_Unit unit){
	float ret;
	ret = isInitialized("PP_Unit_GetHealth");
	if ((int)ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetHealth", &u);
			if ((int)ret == 0)
				ret = u->second.health;
		exitCriticalSection();
	}
	return ret;
}

float PP_Unit_GetMaxHealth_prim(PP_Unit unit){
	float ret;
	ret = isInitialized("PP_Unit_GetMaxHealth");
	if ((int)ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetMaxHealth", &u);
			if ((int)ret == 0)
				ret = u->second.maxHealth;
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_GetPendingCommands_prim(PP_Unit unit, PP_PendingCommands * pdgCmd) {
	int ret;
	if (pdgCmd == NULL)
		return -1;
	// clean pdgCmd
	pdgCmd->nbCmds=0;
	for (int i = 0 ; i < MAX_PDG_CMD ; i++){
		pdgCmd->cmd[i].code = -1;
		pdgCmd->cmd[i].nbParams = 0;
		for (int j = 0 ; j < MAX_PARAMS ; j++){
			pdgCmd->cmd[i].param[j] = -1.0;
		}
	}
	// store new pending commands
	ret = isInitialized("PP_Unit_GetPendingCommands");
	if (ret == 0) {
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetPendingCommands", &u, true);
			if (ret == 0){
				// Find vector of pendingCommands in shared memory (do not use
				// directly the pointer u->second.commandQueue because it is
				// only significant in process that initialized it (i.e. the
				// supplier))
				std::ostringstream oss(std::ostringstream::out);
				oss << "commandQueue" << unit;
				ShCommandVector *commandQueue = segment->find<ShCommandVector>
					(oss.str().c_str()).first;
				if (commandQueue){
					pdgCmd->nbCmds = commandQueue->size() > MAX_PDG_CMD ? MAX_PDG_CMD : commandQueue->size();
					for (int i = 0 ; i < pdgCmd->nbCmds ; i++){
						pdgCmd->cmd[i].code = commandQueue->at(i).code;
						// Find vector of params for pendingCommands found in
						// shared memory (do not use directly the pointer
						// commandQueue->at(idCmd).param because it is only
						// significant in process that initialized it (i.e. the
						// supplier))
						std::ostringstream ossParams(std::ostringstream::out);
						ossParams << "commandQueue" << unit << "Params" << i;
						ShFloatVector *params = segment->find<ShFloatVector>
							(ossParams.str().c_str()).first;
						if (params){
							pdgCmd->cmd[i].nbParams = params->size() > MAX_PARAMS ? MAX_PARAMS : params->size();
							for (int j = 0 ; j < pdgCmd->cmd[i].nbParams ; j++){
								pdgCmd->cmd[i].param[j] = params->at(j);
							}
							// set others params to -1
							for (int j = pdgCmd->cmd[i].nbParams ; j < MAX_PARAMS ; j++){
								pdgCmd->cmd[i].param[j] = -1.0;
							}
						}
						else{
							PP_SetError("PP_Unit_GetPendingCommands : \
	parameters undefined\n");
							ret = INCONSISTENT_SHARED_MEMORY;
						}
					}
				}
				else{
					PP_SetError("PP_Unit_GetPendingCommands : commandQueue \
	undefined\n");
					ret = INCONSISTENT_SHARED_MEMORY;
				}
			}
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_GetGroup_prim(PP_Unit unit){
	int ret;
	ret = isInitialized("PP_Unit_GetGroup");
	if (ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetGroup", &u);
			if (ret == 0) {
				if (u->second.group == -1)
					ret = NO_GROUP;
				else
					ret = u->second.group;
			}
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_SetGroup_prim(PP_Unit unit, int group){
	int ret;
	ret = isInitialized("PP_Unit_SetGroup");
	if (ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_SetGroup", &u, true);
			if (ret == 0) {
				if (group < -1){
					PP_SetError("PP_Unit_SetGroup : invalid group, must be \
greater than or equal -1\n");
					ret = INVALID_GROUP;
				}
				else {
					std::vector<float> unused; // only to pass a parameter
					addCommand(unit, -1, unused, -1, group);
				}
			}
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_ActionOnUnit_prim(PP_Unit unit, int action, PP_Unit target){
	int ret;
	ret = isInitialized("PP_Unit_ActionOnUnit");
	if (ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ShMapUnits::iterator t = shd.units->find(target);
			ret = checkParams("PP_Unit_ActionOnUnit", &u, true, NULL, &t);
			if (ret == 0){
				// stores command
				std::vector<float> params (1, 0);
				params[0] = target;
				addCommand(unit, action, params, 1);
			}
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_ActionOnPosition_prim(PP_Unit unit, int action, PP_Pos pos){
	int ret;
	ret = isInitialized("PP_Unit_ActionOnPosition");
	if (ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_ActionOnPosition", &u, true);
			if (ret == 0){
				// check if pos is inside map
				PP_Pos max;
				ret = PP_GetMapSize_prim(&max);
				if (pos.x < 0 || pos.x >= max.x || pos.y < 0 || pos.y >= max.y){
					PP_SetError("PP_Unit_ActionOnPosition : position is out of \
bounds\n");
					ret = POSITION_OUT_OF_BOUNDS;

					// compute target position on map border
					PP_Pos p1;
					PP_Unit_GetPosition_prim(unit, &p1);
					PP_Pos p2 = pos;
					if (p1.x == p2.x){
						// move verticaly
						(p2.y < 0.0) ? pos.y = 0.0 : pos.y = max.y;
					}
					else if (p1.y == p2.y){
						// move horizontaly
						(p2.x < 0.0) ? pos.x = 0.0 : pos.x = max.x;
					}
					else{
						float a = (p2.y - p1.y)/(p2.x - p1.x);
						float b = p1.y-a*p1.x;
						PP_Pos tmp;
						// compute intersection with top border => y = 0
						tmp.y = 0.0;
						// because y = a*x+b <=> x = (y-b)/a = (0-b)/a = -b/a
						tmp.x = -b/a;
						if (tmp.x >= 0.0 && tmp.x <= max.x && between(tmp, p1, p2))
							pos = tmp;
						else{
							// compute intersection with left border => x = 0
							tmp.x = 0.0;
							tmp.y = b; // because y = a*x+b = a*0+b = b
							if (tmp.y >= 0.0 && tmp.y <= max.y && between(tmp, p1, p2))
								pos = tmp;
							else{
								// compute intersection with botom border
								// => y = maxY
								tmp.y = max.y;
								// because y = a*x + b <=> x = (y-b)/a
								tmp.x = (max.y-b)/a;
								if (tmp.x >= 0.0 && tmp.x <= max.x && between(tmp, p1, p2))
									pos = tmp;
								else{
									// compute intersection with right border
									// => x = maxX
									tmp.x = max.x;
									tmp.y = a*max.x+b; // because y = a*x+b
									if (tmp.y >= 0.0 && tmp.y <= max.y && between(tmp, p1, p2))
										pos = tmp;
								}
							}
						}
					}
					// new position has to be included into [0.0; max.x[ and
					// [0.0; max.y[ if pos.x or pos.y are equal to max value
					// then substract one unit.
					if (pos.x == max.x) pos.x -= 1.0;
					if (pos.y == max.y) pos.y -= 1.0;
				}
				// stores command
				std::vector<float> params (3, 0);
				params[0] = pos.x;
				// this field is set when command will be executed (i.e. in the
				// game engine)
				params[1] = -1;
				params[2] = pos.y;
				addCommand(unit, action, params, 0);
			}
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_UntargetedAction_prim(PP_Unit unit, int action, float param){
	int ret;
	ret = isInitialized("PP_Unit_UntargetedAction");
	if (ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_UntargetedAction", &u, true);
			if (ret == 0){
				// stores command
				std::vector<float> params (1, param);
				addCommand(unit, action, params, 2);
			}
		exitCriticalSection();
	}
	return ret;
}

/******************************************************************************/
/* None protected functions (from critical section point of view              */
/******************************************************************************/

int PP_Unit_GetNumPdgCmds_prim(PP_Unit unit){
	int ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_Unit_GetNumPdgCmds : Mutex is not locked\n");
		ret = MUTEX_NOT_LOCKED;
	}
	else{
		ret = isInitialized("PP_Unit_GetNumPdgCmds");
		if (ret == 0){
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetNumPdgCmds", &u, true);
			if (ret == 0){
				// Find vector of pendingCommands in shared memory (do not use
				// directly the pointer u->second.commandQueue because it is
				// only significant in process that initialized it (i.e. the
				// supplier))
				std::ostringstream oss(std::ostringstream::out);
				oss << "commandQueue" << unit;
				ShCommandVector *commandQueue = segment->find<ShCommandVector>
					(oss.str().c_str()).first;
				if (commandQueue)
					ret = commandQueue->size();
				else {
					PP_SetError("PP_Unit_GetNumPdgCmds : commandQueue undefined\n");
					ret = INCONSISTENT_SHARED_MEMORY;
				}
			}
		}
	}
	return ret;
}

int PP_Unit_PdgCmd_GetCode_prim(PP_Unit unit, int idCmd, int * cmdCode){
	int ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_Unit_PdgCmd_GetCode : Mutex is not locked\n");
		ret = MUTEX_NOT_LOCKED;
	}
	else{
		ret = isInitialized("PP_Unit_PdgCmd_GetCode");
		if (ret == 0){
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_PdgCmd_GetCode", &u, true);
			if (ret == 0){
				// Find vector of pendingCommands in shared memory (do not use
				// directly the pointer u->second.commandQueue because it is
				// only significant in process that initialized it (i.e. the
				// supplier))
				std::ostringstream oss(std::ostringstream::out);
				oss << "commandQueue" << unit;
				ShCommandVector *commandQueue = segment->find<ShCommandVector>
					(oss.str().c_str()).first;

				if (commandQueue){
					try{
						// Returns directly data. If idCmd is inaccurate, an
						// exception will be thrown
						*cmdCode = commandQueue->at(idCmd).code;
						ret = 0;
					} catch (std::out_of_range e) {
						PP_SetError("PP_Unit_PdgCmd_GetCode : idCmd out of range\n");
						ret = OUT_OF_RANGE;
					}
				}
				else {
					PP_SetError("PP_Unit_PdgCmd_GetCode : commandQueue undefined\n");
					ret = INCONSISTENT_SHARED_MEMORY;
				}
			}
		}
	}
	return ret;
}

int PP_Unit_PdgCmd_GetNumParams_prim(PP_Unit unit, int idCmd){
	int ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_Unit_PdgCmd_GetNumParams : Mutex is not locked\n");
		ret = MUTEX_NOT_LOCKED;
	}
	else{
		ret = isInitialized("PP_Unit_PdgCmd_GetNumParams");
		if (ret == 0) {
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_PdgCmd_GetNumParams", &u, true);
			if (ret == 0) {
				// Find vector of params for pendingCommands found in shared
				// memory (do not use directly the pointer
				// u->second.commandQueue->at(idCmd).param because it is only
				// significant in process that initialized it (i.e. the
				// supplier))
				std::ostringstream ossParams(std::ostringstream::out);
				ossParams << "commandQueue" << unit << "Params" << idCmd;
				ShFloatVector *params = segment->find<ShFloatVector>
					(ossParams.str().c_str()).first;
				if (params)
					ret = params->size();
				else {
					PP_SetError("PP_Unit_PdgCmd_GetNumParams : idCmd out of range\n");
					ret = OUT_OF_RANGE;
				}
			}
		}
	}
	return ret;
}

int PP_Unit_PdgCmd_GetParam_prim(PP_Unit unit, int idCmd, int idParam, float * paramValue){
	int ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_Unit_PdgCmd_GetParam : Mutex is not locked\n");
		ret = MUTEX_NOT_LOCKED;
	}
	else{
		ret = isInitialized("PP_Unit_PdgCmd_GetParam");
		if (ret == 0){
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_PdgCmd_GetParam", &u, true);
			if (ret == 0){
				// Find vector of params for pendingCommands found in shared
				// memory (do not use directly the pointer
				// u->second.commandQueue->at(idCmd).param because it is only
				// significant in process that initialized it (i.e. the
				// supplier))
				std::ostringstream ossParams(std::ostringstream::out);
				ossParams << "commandQueue" << unit << "Params" << idCmd;
				ShFloatVector * params = segment->find<ShFloatVector>
					(ossParams.str().c_str()).first;
				if (params){
					try{
						// Returns directly data. If idParam is inaccurate, an
						// exception will be thrown
						*paramValue = params->at(idParam);
						ret = 0;
					} catch (std::out_of_range e) {
						PP_SetError("PP_Unit_PdgCmd_GetParam : idParam out of range\n");
						ret = OUT_OF_RANGE;
					}
				}
				else {
					PP_SetError("PP_Unit_PdgCmd_GetParam : idCmd out of range\n");
					ret = OUT_OF_RANGE;
				}
			}
		}
	}
	return ret;
}

int PP_PushMessage_prim(const char * msg, const int * error) {
	int ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_PushMessage_prim : Mutex is not locked\n");
		ret = MUTEX_NOT_LOCKED;
	}
	else{
		ret = isInitialized("PP_PushMessage");
		if (ret == 0){
			if (*(shd.tracePlayer) > 0) {
				ret = PP_IsGamePaused_prim();
				if (ret == 0) {
					//Create allocators
					const ShCharAllocator charAlloc_inst(segment->get_segment_manager());
					const ShStringAllocator stringAlloc_inst(segment->get_segment_manager());

					//This string is only in this process (the pointer pointing to the
					//buffer that will hold the text is not in shared memory).
					//But the buffer that will hold "msg" parameter is allocated from
					//shared memory
					ShString sharedMessage(charAlloc_inst);
					// set error
					if (error != NULL && *error < 0){
						sharedMessage.append(errorsArr[(*error+1)*-1]);
						sharedMessage.append(" "); // add space
					}
					// set message
					sharedMessage.append(msg);

					//Store the pointer pointing to the buffer into the shared memory
					shd.history->push_back(sharedMessage);
				}
			}
		}
	}
	return ret;
}
