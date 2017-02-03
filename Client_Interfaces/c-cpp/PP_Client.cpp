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
#include "traces/TraceConstantList.h"
#include <sstream>

int PP_Open(){
	if (PP_Open_prim() == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss_start(std::ostringstream::out);
			oss_start << EXECUTION_START_TIME << " " << PP_GetTimestamp_prim();
			PP_PushMessage_prim(oss_start.str().c_str(), NULL);
			std::ostringstream oss_lang(std::ostringstream::out);
			oss_lang << PROGRAMMING_LANGUAGE_USED << " C";
			PP_PushMessage_prim(oss_lang.str().c_str(), NULL);
			PP_PushMessage_prim("PP_Open", NULL);
		exitCriticalSection();
		return 0;
	}
	else
		return -1;
}

int PP_Close (){
	// notify function call to Spring before closing otherwise the shared memory will be closed
	// and message doesn't be sent
	enterCriticalSection();
		PP_PushMessage_prim("PP_Close", NULL);
		std::ostringstream oss(std::ostringstream::out);
		oss << EXECUTION_END_TIME << " " << PP_GetTimestamp_prim();
		PP_PushMessage_prim(oss.str().c_str(), NULL);
	exitCriticalSection();
	if (PP_Close_prim() == 0){
		return 0;
	} else
		return -1;
}

int PP_IsGameOver(){
	int ret = PP_IsGameOver_prim();
	if (ret >= 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_IsGameOver - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
	} else
		ret = -1;
	return ret;
}

int PP_IsGamePaused() {
	int ret = PP_IsGamePaused_prim();
	if (ret >= 0) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_IsGamePaused - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
	} else
		ret = -1;
	return ret;
}

PP_Pos PP_GetMapSize(){
	PP_Pos tmp;
	if (PP_GetMapSize_prim(&tmp) == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetMapSize - " << tmp.x << " " << tmp.y;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
	}
	else{
		tmp.x = -1;
		tmp.y = -1;
	}
	return tmp;
}

PP_Pos PP_GetStartPosition(){
	PP_Pos tmp;
	if (PP_GetStartPosition_prim(&tmp) == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetStartPosition - " << tmp.x << " " << tmp.y;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
	}
	else{
		tmp.x = -1;
		tmp.y = -1;
	}
	return tmp;
}

int PP_GetNumSpecialAreas(){
	int ret = PP_GetNumSpecialAreas_prim();
	if (ret >= 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetNumSpecialAreas - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
	} else
		ret = -1;
	return ret;
}

PP_Pos PP_GetSpecialAreaPosition(int num){
	PP_Pos tmp;
	int ret = PP_GetSpecialAreaPosition_prim(num, &tmp);
	if (ret > FEEDBACK_COUNT_LIMIT){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetSpecialAreaPosition " << num;
			if (ret == 0)
				oss << " - " << tmp.x << " " << tmp.y;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0) {
		tmp.x = -1;
		tmp.y = -1;
	}
	return tmp;
}

int PP_GetResource(PP_Resource id){
	int ret = PP_GetResource_prim(id);
	if (ret > FEEDBACK_COUNT_LIMIT){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetResource " << id;
			if (ret >= 0)
				oss << " - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

int PP_GetNumUnits(PP_Coalition c){
	int ret = PP_GetNumUnits_prim(c);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetNumUnits " << c;
			if (ret >= 0)
				oss << " - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

PP_Unit PP_GetUnitAt(PP_Coalition c, int index){
	int ret = PP_GetUnitAt_prim (c, index);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetUnitAt " << c << " " << index;
			if (ret >= 0)
				oss << " - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

PP_Coalition PP_Unit_GetCoalition(PP_Unit unit){
	int ret = PP_Unit_GetCoalition_prim (unit);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetCoalition " << unit;
			int type = PP_Unit_GetType_prim(unit);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss << " - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return (PP_Coalition)ret;
}

int PP_Unit_GetType(PP_Unit unit){
	int ret = PP_Unit_GetType_prim(unit);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetType " << unit;
			if (ret >= 0)
				oss << "_" << ret << " - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

PP_Pos PP_Unit_GetPosition(PP_Unit unit){
	PP_Pos tmp;
	int ret = PP_Unit_GetPosition_prim(unit, &tmp);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetPosition " << unit;
			int type = PP_Unit_GetType_prim(unit);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss << " - " << tmp.x << " " << tmp.y;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0){
		tmp.x = -1;
		tmp.y = -1;
	}
	return tmp;
}

float PP_Unit_GetHealth(PP_Unit unit){
	float ret = PP_Unit_GetHealth_prim (unit);
	if ((int)ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetHealth " << unit;
			int type = PP_Unit_GetType_prim(unit);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss << " - " << ret;
			int error = ret;
			PP_PushMessage_prim(oss.str().c_str(), &error);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

float PP_Unit_GetMaxHealth(PP_Unit unit){
	float ret = PP_Unit_GetMaxHealth_prim (unit);
	if ((int)ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetMaxHealth " << unit;
			int type = PP_Unit_GetType_prim(unit);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss << " - " << ret;
			int error = ret;
			PP_PushMessage_prim(oss.str().c_str(), &error);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

int PP_Unit_GetPendingCommands(PP_Unit unit, PP_PendingCommands * pdgCmd) {
	int ret = PP_Unit_GetPendingCommands_prim (unit, pdgCmd);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetPendingCommands " << unit;			
			int type = PP_Unit_GetType_prim (unit);
			if (type >= 0)
				oss << "_" << type;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

int PP_Unit_GetGroup(PP_Unit unit){
	int ret = PP_Unit_GetGroup_prim (unit);
	if (ret > FEEDBACK_COUNT_LIMIT || ret == NO_GROUP) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			if (ret == NO_GROUP)
				ret = -2;
			oss << "PP_Unit_GetGroup " << unit;
			int type = PP_Unit_GetType_prim(unit);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0 || ret == -2){
				oss << " - " << ret;
				PP_PushMessage_prim(oss.str().c_str(), NULL);
			} else
				PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	} else
		ret = -1;
	return ret;
}

int PP_Unit_SetGroup(PP_Unit unit, int group){
	int ret = PP_Unit_SetGroup_prim (unit, group);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_SetGroup " << unit;
			int type = PP_Unit_GetType_prim(unit);
			if (type >= 0)
				oss << "_" << type;
			oss << " " << group;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

bool orderFound(PP_Unit unit, int action){
	bool orderFound = false;
	// Check all pending commands
	PP_PendingCommands pdgCmd;
	int ret = PP_Unit_GetPendingCommands_prim(unit, &pdgCmd);
	for (int i = 0 ; i < pdgCmd.nbCmds ; i++){
		if (pdgCmd.cmd[i].code == action)
			orderFound = true;
	}
	return orderFound;
}

int PP_Unit_ActionOnUnit(PP_Unit unit, int action, PP_Unit target, int synchronized){
	int ret = PP_Unit_ActionOnUnit_prim (unit, action, target);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_ActionOnUnit " << unit;
			int unitType = PP_Unit_GetType_prim(unit);
			if (unitType >= 0)
				oss << "_" << unitType;
			oss << " " << action << " " << target;
			int targetType = PP_Unit_GetType_prim(target);
			if (targetType >= 0)
				oss << "_" << targetType;
			// normalize synchronized
			if (synchronized == 0)
				oss << " false";
			else
				oss << " true";
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
		if (synchronized != 0){
			// waiting that the order is adding into pending commands
			while (!orderFound(unit, action));
			// waiting that the order is over pending commands
			while (orderFound(unit, action));
		}
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

int PP_Unit_ActionOnPosition(PP_Unit unit, int action, PP_Pos pos, int synchronized){
	int ret = PP_Unit_ActionOnPosition_prim (unit, action, pos);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_ActionOnPosition " << unit;
			int unitType = PP_Unit_GetType_prim(unit);
			if (unitType >= 0)
				oss << "_" << unitType;
			oss << " " << action << " " << pos.x << " " << pos.y;
			// normalize synchronized
			if (synchronized == 0)
				oss << " false";
			else
				oss << " true";
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
		if (synchronized != 0){
			// waiting that the order is adding into pending commands
			while (!orderFound(unit, action));
			// waiting that the order is over pending commands
			while (orderFound(unit, action));
		}
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

int PP_Unit_UntargetedAction(PP_Unit unit, int action, float param, int synchronized){
	int ret = PP_Unit_UntargetedAction_prim (unit, action, param);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_UntargetedAction " << unit;
			int unitType = PP_Unit_GetType_prim(unit);
			if (unitType >= 0)
				oss << "_" << unitType;
			oss << " " << action << " " << param;
			// normalize synchronized
			if (synchronized == 0)
				oss << " false";
			else
				oss << " true";
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
		if (synchronized != 0){
			// waiting that the order is adding into pending commands
			while (!orderFound(unit, action));
			// waiting that the order is over pending commands
			while (orderFound(unit, action));
		}
	}
	if (ret < 0)
		ret = -1;
	return ret;
}
