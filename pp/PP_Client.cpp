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

#include "PP_Client.h"
#include "PP_Client_Private.h"
#include "PP_Private.h"
#include "PP_Error_Private.h"

#include <signal.h>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/thread.hpp>

#include <stdio.h>

/******************************************************************************/
/* Definition of global variables                                             */
/******************************************************************************/

/* Contains all shared data */
static PP_sharedData shd;

/* The shared memory segment */
static boost::interprocess::managed_shared_memory *segment = NULL;

/* Indicates if PP has been openned */
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
 * section (PP_Refresh and addCommand) => flag the signal to take into
 * consideration later.
 */
static void manageSignal(int signal){
	if (signal == SIGINT)
		ctrlC = true;
}

/*
 * Checks if PP is initialized.
 * fctName : name of the function that called isInitialised.
 * Returns : 0 on success. -1 is returned on errors.
 */
static int isInitialized(std::string fctName = ""){
	if (!opened){
		PP_SetError("%s : Prog&Play is not opened\n", fctName.c_str());
		return -1;
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
 * Returns : 0 on success. -1 is returned on errors.
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
			return -1;
		}
		if (checkOwner){
			if ((*unit)->second.coalition != MY_COALITION){
				PP_SetError("%s : you are not the unit owner\n",
					fctName.c_str());
				return -1;
			}
		}
	}
	if (coalition){
		if (*coalition < 0 || *coalition >= NB_COALITIONS){
			PP_SetError("%s : coalition out of range\n", fctName.c_str());
			return -1;
		}
	}
	if (target){
		if (*target == shd.units->end()){
			PP_SetError("%s : target not found\n", fctName.c_str());
			return -1;
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
		// make a pause every 4 call to avoid cpu consuming
		if (nbCriticalCall > 4){
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			nbCriticalCall = 0;
		}
		// intercept SIGINT signal in critical section
		signal(SIGINT, manageSignal);
		// takes mutex
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

/******************************************************************************/
/* Functions to communicate with the game                                     */
/******************************************************************************/

int PP_Open(){
	if (!opened){
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
			shd.units = segment->find<ShMapUnits>("units").first;
			shd.coalitions = segment->find<ShIntVector>("coalitions").first;
			shd.pendingCommand =
				segment->find<ShPendingCommands>("pendingCommands").first;
			shd.mapSize = segment->find<PP_Pos>("mapSize").first;
			shd.startPosition = segment->find<PP_Pos>("startPosition").first;
			shd.specialAreas = segment->find<ShVectPos>("specialAreas").first;
			shd.resources = segment->find<ShIntVector>("resources").first;
			shd.history = segment->find<ShStringList>("history").first;
			
			// notify function call to Spring
			enterCriticalSection();
				PP_PushMessage("PP_Open");
			exitCriticalSection();
			
			nbCriticalCall = 0;
		} catch (...){
			PP_SetError("PP_Open : open shared memory error\n");
			delete segment;
			segment = NULL;
//#if (defined BOOST_WINDOWS) && !(defined BOOST_DISABLE_WIN32)
			//Tries to delete shared memory in the client side only with Windows
//			std::cout << boost::interprocess::shared_memory_object::remove(
//				"PP_SharedMemory") << std::endl;
//#endif
			return -1;
		}
		/* validation of the opening */
		opened = true;
		return 0;
	}
	else{
		if (PP_Close() < 0){
			PP_SetError("PP_Open : Prog&Play is already opened, impossible to \
reload it\n");
			return -1;
		}
		// reopening of Prog&Play
		return PP_Open();
	}
}

int PP_Close (){
	int ret = isInitialized("PP_Close");
	if (ret == 0){
		opened = false;
		// notify function call to Spring
		enterCriticalSection();
			PP_PushMessage("PP_Close");
		exitCriticalSection();
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

int PP_IsGameOver(){
	int ret;
	// Checks initialisation
	ret = isInitialized("PP_IsGameOver");
	if (ret == 0){
		enterCriticalSection();
			ret = *(shd.gameOver);
			// notify function call to Spring
			PP_PushMessage("PP_IsGameOver");
		exitCriticalSection();
	}
	return ret;
}

PP_Pos PP_GetMapSize(){
	PP_Pos tmp;
	// Checks initialisation
	int ret = isInitialized("PP_MapSize");
	if (ret == 0){
		enterCriticalSection();
			tmp = *(shd.mapSize);
			// notify function call to Spring
			PP_PushMessage("PP_GetMapSize");
		exitCriticalSection();
	}
	else{
		tmp.x = ret;
		tmp.y = ret;
	}
	return tmp;
}

PP_Pos PP_GetStartPosition(){
	PP_Pos tmp;
	// Checks initialisation
	int ret = isInitialized("PP_StartPosition");
	if (ret == 0){
		enterCriticalSection();
			tmp = *(shd.startPosition);
			// notify function call to Spring
			PP_PushMessage("PP_GetStartPosition");
		exitCriticalSection();
	}
	else{
		tmp.x = ret;
		tmp.y = ret;
	}
	return tmp;
}

int PP_GetNumSpecialAreas(){
	int ret;
	ret = isInitialized("PP_GetNumSpecialAreas");
	if (ret == 0){
		enterCriticalSection();
			ret = shd.specialAreas->size();
			// notify function call to Spring
			PP_PushMessage("PP_GetNumSpecialAreas");
		exitCriticalSection();
	}
	return ret;
}

PP_Pos PP_GetSpecialAreaPosition(int g){
	PP_Pos tmp;
	// Checks initialisation
	int ret = isInitialized("PP_GetSpecialAreaPosition");
	if (ret == 0){
		enterCriticalSection();
			try {
				tmp = shd.specialAreas->at(g);
			} catch (std::out_of_range e){
				PP_SetError("PP_GetSpecialAreaPosition : special area not \
found\n");
				tmp.x = -1;
				tmp.y = -1;
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetSpecialAreaPosition " << g;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	else {
		tmp.x = -1;
		tmp.y = -1;
	}
	return tmp;
}

int PP_GetResource(PP_Resource id){
	int ret;
	ret = isInitialized("PP_GetResource");
	if (ret == 0){
		enterCriticalSection();
			try{
				ret = shd.resources->at(id);
			} catch (std::out_of_range e) {
				PP_SetError("PP_GetResource : ressource out of range\n");
				ret = -1;
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetResource " << id;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

int PP_GetNumUnits(PP_Coalition c){
	int ret;
	ret = checkParams("PP_GetNumUnits", NULL, false, (int*)&c);
	if (ret == 0){
		enterCriticalSection();
			ret = shd.coalitions[c].size();
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetNumUnits " << c;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

PP_Unit PP_GetUnitAt(PP_Coalition c, int index){
	int ret;
	ret = checkParams("PP_GetUnitAt", NULL, false, (int*)&c);
	if (ret == 0){
		enterCriticalSection();
			try{
				// Returns directly data. If index is inaccurate, an exception
				// will be thrown
				ret = shd.coalitions[c].at(index);
			} catch (std::out_of_range e) {
				PP_SetError("PP_GetUnitAt : index out of range\n");
				ret = -1;
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetUnitAt " << c << " " << index;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

PP_Coalition PP_Unit_GetCoalition(PP_Unit unit){
	int ret;
	ret = isInitialized("PP_Unit_GetCoalition");
	if (ret == 0){
		enterCriticalSection();
			// Store the unit
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetCoalition", &u);
			if (ret == 0)
				ret = u->second.coalition;
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetCoalition " << unit;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return (PP_Coalition)ret;
}

int PP_Unit_GetType(PP_Unit unit){
	int ret;
	ret = isInitialized("PP_Unit_GetType");
	if (ret == 0){
		enterCriticalSection();
			// Store the unit
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetType", &u);
			if (ret == 0){
				if (u->second.type == -1){
					PP_SetError("PP_Unit_GetType : type not found\n");
					ret = -1;
				}
				else
					ret = u->second.type;
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetType " << unit;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

PP_Pos PP_Unit_GetPosition(PP_Unit unit){
	PP_Pos tmp;
	int ret = isInitialized("PP_Unit_GetPosition");
	if (ret == 0){
		enterCriticalSection();
			// Store the unit
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetPosition", &u);
			if (ret == 0)
				tmp = u->second.pos;
			else{
				tmp.x = ret;
				tmp.y = ret;
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetPosition " << unit;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	else {
		tmp.x = -1;
		tmp.y = -1;
	}
	return tmp;
}

float PP_Unit_GetHealth(PP_Unit unit){
	float ret;
	ret = isInitialized("PP_Unit_GetHealth");
	if ((int)ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetHealth", &u);
			if ((int)ret == 0)
				ret = u->second.health;
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetHealth " << unit;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

float PP_Unit_GetMaxHealth(PP_Unit unit){
	float ret;
	ret = isInitialized("PP_Unit_GetMaxHealth");
	if ((int)ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetMaxHealth", &u);
			if ((int)ret == 0)
				ret = u->second.maxHealth;
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetMaxHealth " << unit;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_GetPendingCommands(PP_Unit unit, PP_PendingCommands * pdgCmd){
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
	if (ret == 0){
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
							ret = -1;
						}
					}
				}
				else{
					PP_SetError("PP_Unit_GetPendingCommands : commandQueue \
undefined\n");
					ret = -1;
				}
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetPendingCommands " << unit;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_GetGroup(PP_Unit unit){
	int ret;
	ret = isInitialized("PP_Unit_GetGroup");
	if (ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_GetGroup", &u);
			if (ret == 0){
				if (u->second.group == -1)
					ret = -2;
				else
					ret = u->second.group;
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetGroup " << unit;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_SetGroup(PP_Unit unit, int group){
	int ret;
	ret = isInitialized("PP_Unit_SetGroup");
	if (ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_SetGroup", &u, true);
			if (ret == 0){
				if (group < -1){
					PP_SetError("PP_Unit_SetGroup : invalid group, must be \
greater than or equal -1\n");
					ret = -1;
				}
				else{
					std::vector<float> unused; // only to pass a parameter
					addCommand(unit, -1, unused, -1, group);
				}
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_SetGroup " << unit << " " << group;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_ActionOnUnit(PP_Unit unit, int action, PP_Unit target){
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
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_ActionOnUnit " << unit << " " << action << " " << target;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_ActionOnPosition(PP_Unit unit, int action, PP_Pos pos){
	int ret;
	ret = isInitialized("PP_Unit_ActionOnPosition");
	if (ret == 0){
		enterCriticalSection();
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_ActionOnPosition", &u, true);
			if (ret == 0){
				// check if pos is inside map
				PP_Pos max = PP_GetMapSize();
				if (pos.x < 0 || pos.x >= max.x || pos.y < 0 || pos.y >= max.y){
					PP_SetError("PP_Unit_ActionOnPosition : position is out of \
bounds\n");
					ret = -1;
					
					// compute target position on map border
					PP_Pos p1 = PP_Unit_GetPosition(unit);
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
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_ActionOnPosition " << unit << " " << action << " " << pos.x << " " << pos.y;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

int PP_Unit_UntargetedAction(PP_Unit unit, int action, float param){
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
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_UntargetedAction " << unit << " " << action << " " << param;
			PP_PushMessage(oss.str().c_str());
		exitCriticalSection();
	}
	return ret;
}

/******************************************************************************/
/* PP_Client_Private.h implementations                                        */
/******************************************************************************/
 
int PP_Unit_GetNumPdgCmds(PP_Unit unit){
	int ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_Unit_GetNumPdgCmds : Mutex is not locked\n");
		ret = -1;
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
				else{
					PP_SetError("PP_Unit_GetNumPdgCmds : commandQueue undefined\n");
					ret = -1;
				}
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetNumPdgCmds " << unit;
			PP_PushMessage(oss.str().c_str());
		}
	}
	return ret;
}

int PP_Unit_PdgCmd_GetCode(PP_Unit unit, int idCmd){
	int ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_Unit_PdgCmd_GetCode : Mutex is not locked\n");
		ret = -1;
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
						ret = commandQueue->at(idCmd).code;
					} catch (std::out_of_range e) {
						PP_SetError("PP_Unit_PdgCmd_GetCode : idCmd out of range\n");
						ret = -1;
					}
				}
				else{
					PP_SetError("PP_Unit_PdgCmd_GetCode : commandQueue undefined\n");
					ret = -1;
				}
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_PdgCmd_GetCode " << unit << " " << idCmd;
			PP_PushMessage(oss.str().c_str());
		}
	}
	return ret;
}

int PP_Unit_PdgCmd_GetNumParams(PP_Unit unit, int idCmd){
	int ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_Unit_PdgCmd_GetNumParams : Mutex is not locked\n");
		ret = -1;
	}
	else{
		ret = isInitialized("PP_Unit_PdgCmd_GetNumParams");
		if (ret == 0){
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_PdgCmd_GetNumParams", &u, true);
			if (ret == 0){
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
				else{
					PP_SetError("PP_Unit_PdgCmd_GetNumParams : idCmd out of range\n");
					ret = -1;
				}
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_PdgCmd_GetNumParams " << unit << " " << idCmd;
			PP_PushMessage(oss.str().c_str());
		}
	}
	return ret;
}

float PP_Unit_PdgCmd_GetParam(PP_Unit unit, int idCmd, int idParam){
	float ret;
	// check if mutex is locked
	if (locked == 0){
		PP_SetError("PP_Unit_PdgCmd_GetParam : Mutex is not locked\n");
		ret = -1;
	}
	else{
		ret = isInitialized("PP_Unit_PdgCmd_GetParam");
		if ((int)ret == 0){
			ShMapUnits::iterator u = shd.units->find(unit);
			ret = checkParams("PP_Unit_PdgCmd_GetParam", &u, true);
			if ((int)ret == 0){
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
						ret = params->at(idParam);
					} catch (std::out_of_range e) {
						PP_SetError("PP_Unit_PdgCmd_GetParam : idParam out of range\n");
						ret = -1;
					}
				}
				else {
					PP_SetError("PP_Unit_PdgCmd_GetParam : idCmd out of range\n");
					ret = -1;
				}
			}
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_PdgCmd_GetParam " << unit << " " << idCmd << " " << idParam;
			PP_PushMessage(oss.str().c_str());
		}
	}
	return ret;
}


int PP_PushMessage(const char * msg){
	//Create allocators
	const ShCharAllocator charAlloc_inst(segment->get_segment_manager());
	const ShStringAllocator stringAlloc_inst(segment->get_segment_manager());
	
	//This string is only in this process (the pointer pointing to the
	//buffer that will hold the text is not in shared memory).
	//But the buffer that will hold "msg" parameter is allocated from
	//shared memory
	ShString sharedMessage(charAlloc_inst);
	sharedMessage.append(msg);
	
	//Store the pointer pointing to the buffer into the shared memory
	shd.history->push_back(sharedMessage);
}