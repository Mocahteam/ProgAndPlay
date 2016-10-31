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

#ifndef PP_PRIVATE_H
#define PP_PRIVATE_H

#include "PP.h"

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/containers/map.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/list.hpp>

/*****************************************************************************/
/* Redefining types of Boost to easier readability                           */
/*****************************************************************************/

typedef boost::interprocess::managed_shared_memory::segment_manager ShmManager;
typedef boost::interprocess::interprocess_mutex ShMutex;

/*****************************************************************************/
/* Definition of a coordinates vector                                        */
/*****************************************************************************/
/* To allocate coordinates through the manager of the shared memory */
typedef boost::interprocess::allocator<PP_Pos, ShmManager> ShPosAllocator;
/* the vector strictly speaking */
typedef boost::interprocess::vector<PP_Pos, ShPosAllocator> ShVectPos;

/*****************************************************************************/
/* Float vector definition                                                   */
/*****************************************************************************/
/* To allocate a float through the manager of the shared memory */
typedef boost::interprocess::allocator<float, ShmManager> ShFloatAllocator;
/* vector of floats */
typedef boost::interprocess::vector<float, ShFloatAllocator> ShFloatVector;

/*****************************************************************************/
/* Int vector definition                                                     */
/*****************************************************************************/
/* To allocate a int through the manager of the shared memory */
typedef boost::interprocess::allocator<int, ShmManager> ShIntAllocator;
/* vector of int */
typedef boost::interprocess::vector<int, ShIntAllocator> ShIntVector;

/*****************************************************************************/
/* String list definition                                                    */
/*****************************************************************************/
/* To allocate a char through the manager of the shared memory */
typedef boost::interprocess::allocator<char, ShmManager> ShCharAllocator;
/* Defines shared String */
typedef boost::interprocess::basic_string<char, std::char_traits<char>, ShCharAllocator> ShString;
/* To allocate a string through the manager of the shared memory */
typedef boost::interprocess::allocator<ShString, ShmManager> ShStringAllocator;
/* list of string */
typedef boost::interprocess::list<ShString, ShStringAllocator> ShStringList;

/*****************************************************************************/
/* Command definition                                                        */
/*****************************************************************************/
struct ShCommand {
	/* command code */
	int code;
	/* list of parameters of the command */
	ShFloatVector * param;
	
	/* builder */
	ShCommand(){
		code = -1;
		param = NULL;
	}
};
/* To allocate a Command through the manager of the shared memory */
typedef boost::interprocess::allocator<ShCommand, ShmManager> ShCommandAllocator;
/* vector of int */
typedef boost::interprocess::vector<ShCommand, ShCommandAllocator> ShCommandVector;

/*****************************************************************************/
/* Unit definition                                                           */
/*****************************************************************************/
/* A unit strictly speaking */
struct ShUnit {
	int id;
	PP_Coalition coalition;
	int type;
	PP_Pos pos;
	float health;
	float maxHealth;
	int group;
	/* list of commands in queue */
	ShCommandVector *commandQueue;
	
	/* builder */
	ShUnit(){
		commandQueue = NULL;
	}
};

/*****************************************************************************/
/* Map of units definition. units are sorted by id                           */
/*****************************************************************************/
/* Data of the map */
typedef std::pair<const int, ShUnit> ShMapData;
/* To allocate a unit into the map through the manager of the shared memory */
typedef boost::interprocess::allocator<ShMapData, ShmManager>
	ShMapDataAllocator;
/* The map strictly speaking */
typedef boost::interprocess::map<int, ShUnit, std::less<int>,
	ShMapDataAllocator> ShMapUnits;

/*****************************************************************************/
/* Pending command definition                                                        */
/*****************************************************************************/
/* A pending command strictly speaking */
struct ShPendingCommand {
	/* affectation group
	 * -2 : unused
	 * -1 : withdraw from the current group
	 * >= 0 : devotes to group */
	int group;
	/* comand type
	 * -1 : invalid command
	 * 0 : target positions
	 * 1 : target unit
	 * 2 : untarget */
	int commandType;
	/* The command */
	ShCommand command;

	/* builder */
	ShPendingCommand() : group(-2), commandType(-1), command(){};
};

/*****************************************************************************/
/* Map of command definition that content pending commands                   */
/*****************************************************************************/
/* Data of the map */
typedef std::pair<const int, ShPendingCommand> ShUnitPendingCommand;
/* To allocate command into the map through the manager of the shared memory */
typedef boost::interprocess::allocator<ShUnitPendingCommand, ShmManager>
	ShPendingCommandAllocator;
/* The map strictly speaking */
typedef boost::interprocess::map<int, ShPendingCommand, std::less<int>,
	ShPendingCommandAllocator> ShPendingCommands;

/*****************************************************************************/
/* Data shared between process                                               */
/*****************************************************************************/
typedef struct {
	/* For synchronisation */
	ShMutex *mutex;
	/* Indicates if the game is over */
	bool *gameOver;
	/* Contains all units visible by the player */
	ShMapUnits *units;
	/* Contains units which are dispatched according to their coalition */
	ShIntVector *coalitions;
	/* Contains list of pending commands */
	ShPendingCommands *pendingCommand;
	/* Contains the map size */
	PP_Pos *mapSize;
	/* Contains the starting point of the player */
	PP_Pos *startPosition;
	/* Contains special areas */
	ShVectPos *specialAreas;
	/* Contains resources */
	ShIntVector *resources;
	/* Contains history of functions called */
	ShStringList *history;
} PP_sharedData;

#endif /* PP_PRIVATE_H */
