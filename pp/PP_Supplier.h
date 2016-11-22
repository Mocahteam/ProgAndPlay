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

#ifndef PP_SUPPLIER_H
#define PP_SUPPLIER_H

#include "PP.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Define a table of positions
 */
typedef struct {
	int size;
	PP_Pos *pos;
} PP_Positions;

typedef PP_Unit PP_UnitId;

/*
 * Representation of a command
 */
typedef struct {
	/* command code */
	int code;
	/* number of parameters */
	int nbParam;
	/* list of parameters of the command */
	float *param;
} PP_Command;

/*
 * Representation of a unit
 */
typedef struct {
	PP_UnitId id;
	PP_Coalition coalition;
	int type;
	PP_Pos pos;
	float health;
	float maxHealth;
	int group;
	int nbCommandQueue;
	PP_Command *commandQueue;
} PP_ShortUnit;

/*
 * Representation of a command
 */
typedef struct{
	/* unit id that must carry out this command */
	PP_UnitId unitId;
	/* affectation group
	 * -2 : unused
	 * -1 : withdraw from the current group
	 * >= 0 : devote to group */
	int group;
	/* comand type
	 * -1 : invalid command
	 * 0 : target positions
	 * 1 : target unit
	 * 2 : untarget */
	int commandType;
	/* the command */
	PP_Command command;
} PP_PendingCommand;

/*
 * Defines a table of commands
 */
typedef struct{
	int size;
	PP_PendingCommand *pendingCommand;
} PP_PendingCommands;

/*
 * Defines a table of resources
 */
typedef struct{
	int size;
	PP_Resource *resource;
} PP_Resources;

/*****************************************************************************/
/* Functions to manage Prog&Play                                             */
/*****************************************************************************/

/*
 * Initializes Prog&Play API. This must be called before using other functions in
 * this library.
 * 
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_Init();

/*
 * Shutdown and cleanup Prog&Play API.
 * After calling this Prog&Play functions should not be used. You may, of
 * course, use PP_Open to use the functionality again.
 * 
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_Quit(void);

/*
 * Updates gameOver state for the client
 *
 * gameOver : the new game over state.
 *            If gameOver != 0, the game is over.
 *            If gameOver = 0, the game is not yet over.
 * 
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_SetGameOver(const bool gameOver);

/*
 * Updates gamePaused state for the client
 *
 * gamePaused : the new game paused state.
 *            If gamePaused != 0, the game is paused.
 *            If gamePaused = 0, the game is not paused.
 * 
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_SetGamePaused(const bool gamePaused);

/*
 * Set tracePlayer state to true for the client
 *
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_SetTracePlayer();

/*
 * Updates the timestamp for the client
 *
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_UpdateTimestamp(int timestamp);

/*
 * Updates static data. In theory, these data should not be updates during
 * simulation. All parameters you pass in must have consistent data.
 * All these data are copied, you are in charge to free them.
 *
 * mapSize : the new map size of the game. If NULL, data is unchanged otherwise
 *           "mapSize" points to the new map size of the game.
 * startPos : the new starting position of the client. If NULL, data is
 *            unchanged otherwise "startPos" points to the new starting position
 *            of the player.
 * specialAreas : the new set of special areas positions. If NULL, data is
 *                unchanged otherwise "specialAreas" points to the new set of
 *                special areas positions.
 *
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_SetStaticData(const PP_Pos mapSize, const PP_Pos startPos,
		const PP_Positions specialAreas);

/*
 * Updates the ressources state for the client. All parameters you pass in must
 * have consistent data. All these data are copied, you are in charge to free
 * them.
 *
 * resources : the new set of resources level.
 *
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_SetRessources(const PP_Resources resources);

/*
 * Makes a unit accessible. All parameters you pass in must have consistent
 * data. All these data are copied, you are in charge to free them.
 *
 * unit : the unit to add.
 *
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_AddUnit(const PP_ShortUnit unit);

/*
 * Updates an accessible unit. All parameters you pass in must have consistent
 * data. All these data are copied, you are in charge to free them.
 *
 * unit : the unit to update.
 *
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_UpdateUnit(const PP_ShortUnit unit);

/*
 * Removes an accessible unit.
 *
 * unitId : the unit id of the unit to remove.
 *
 * Returns : 0 on success. -1 is returned on errors.
 */
int PP_RemoveUnit(const PP_UnitId unitId);

/*
 * Check if a unit with a "unitId" as already stored.
 * 
 * unitId: the id of the unit we have to look for.
 * 
 * Returns: 1 if the unit exists, O if the unit doesn't exist and -1 on errors.
 */
int PP_IsStored(const PP_UnitId unitId);

/*
 * get pending commands set defined by the client. Use "PP_FreePendingCommand"
 * to free the pending commands set from memory
 *
 * Returns : the pending commands set on success. NULL is returned on errors.
 */
PP_PendingCommands* PP_GetPendingCommands(void);

/*
 * Free the pending commands set from memory. Do not reference the set after
 * this call, except to get new one with "PP_GetPendingCommand" function.
 *
 * commands : The pending commands set to free from memory 
 */
void PP_FreePendingCommands(PP_PendingCommands *commands);

/*
 * Pop the next message defines by the client.
 *
 * Returns: the next message as a c-string ended by NULL (free c-string
 *          by yourself). NULL is returned if no message is available.
 */
char * PP_PopMessage();

#ifdef __cplusplus
};
#endif

#endif /* PP_SUPPLIER_H */
