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

#ifndef PP_CLIENT_H
#define PP_CLIENT_H

/**
 * \file PP_Client.h
 * \brief Interface to interact with the game.
 * \author Muratet.M
 * \date March 5, 2013
 *
 * Provides an interface to interact with the game.
 *
 */
 
#include "PP.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Data definitions                                                           */
/******************************************************************************/
/**
 * \brief Maximum number of parameters of a command.
 * \see PP_Cmd
 */
#define MAX_PARAMS 3

/**
 * \brief Maximum number of pending command available for a unit in the Prog&Play API.
 * 
 * \see PP_PendingCommands
 */
 #define MAX_PDG_CMD 10

/**
 * \brief Represents a pending command.
 * 
 * Defines a pending command in Prog&Play API.
 */
typedef struct {
  int code;                /**< \brief Command code. \see ConstantList_KP4.1.h - available command codes list */
  int nbParams;            /**< \brief Number of available parameters. */
  float param[MAX_PARAMS]; /**< \brief List of command parameters. */
} PP_Cmd;

/**
 * \brief Represents pending commands.
 * 
 * Defines pending commands list associated to a unit in Prog&Play API.
 */
typedef struct {
  int nbCmds;              /**< \brief Number of available pending commands. */
  PP_Cmd cmd[MAX_PDG_CMD]; /**< \brief List of pending commands. */
} PP_PendingCommands;

/******************************************************************************/
/* Functions to manage Prog&Play                                              */
/******************************************************************************/

/**
 * \brief Opens Prog&Play API.
 *
 * This must be called before using other functions in this library.
 * \return 0 on success. -1 is returned on errors.
 */
int PP_Open(void);

/**
 * \brief Shutdowns and cleans up Prog&Play API.
 *
 * After calling this Prog&Play functions should not be used. You may, of
 * course, use PP_Open to use the functionality again.
 * \return 0 on success. -1 is returned on errors.
 */
int PP_Close(void);

/******************************************************************************/
/* Functions to manage game elements                                          */
/******************************************************************************/

/**
 * \brief Checks if the game is over.
 * \return positive value if the game is ended and 0 if not. -1 is returned on
 *         errors.
 */
int PP_IsGameOver(void);

/**
 * \brief Checks if the game is paused.
 * \return positive value if the game is paused and 0 if not. -1 is returned on
 *         errors.
 */
int PP_IsGamePaused(void);

/**
 * \brief Returns map size.
 * 
 * Returns map size as a PP_Pos. Fields of PP_Pos are the max values available
 * for the current map.
 * \return map size on success. A position containing -1.0 is returned on
 *         errors.
 */
PP_Pos PP_GetMapSize(void);

/**
 * \brief Returns start position.
 * 
 * The start position is the position where the player's army is located at the
 * beginning of the game.
 * \return start position on success. A position containing -1.0 is returned
 *         on errors.
 */
PP_Pos PP_GetStartPosition(void);

/**
 * \brief Returns number of special areas
 *
 * With Kernel Panic, a special area is a position where building is available.
 * \return the number of special areas on success. -1 is returned on errors.
 */
int PP_GetNumSpecialAreas(void);

/**
 * \brief Returns position of a special area.
 * 
 * With Kernel Panic, a special area is a position where building is available.
 * \param num : an id of special area, it must be included meanwhile [0;n[ where n is
 *        the number of special areas.
 * \return the position of this special area on success.  A position
 *         containing -1.0 is returned on errors.
 * \see PP_GetNumSpecialAreas
 */
PP_Pos PP_GetSpecialAreaPosition(int num);

/**
 * \brief Returns the current level of a resource.
 *
 * \param id of resource that you want know level.
 * \return the current level of specified resource.
 *         -1 is returned on errors.
 * \see constantList_KP4.1.h - available resources list
 */
int PP_GetResource(PP_Resource id);

/**
 * \brief Returns the number of units from a coalition.
 *
 * Only visible units by the player are counted.
 * \param c : coalition to consult.
 * \return the number of units (visible by the player) from this coalition. -1
 *         is returned on errors.
 */
int PP_GetNumUnits(PP_Coalition c);

/**
 * \brief Returns the nth visible unit of a coalition.
 * \param c the coalition to consult.
 * \param index of a unit in the specified coalition, must be included
 *        meanwhile [0;n[ where n is the number of units of this coalition.
 * \return unit at the specified index on success. -1 is returned en errors.
 * \see PP_GetNumUnits
 */
PP_Unit PP_GetUnitAt(PP_Coalition c, int index);

/******************************************************************************/
/* Functions to manage unit                                                   */
/******************************************************************************/

/**
 * \brief Returns the unit coalition.
 *
 * \param unit to consult.
 * \return coalition of the specified unit on success. -1 is returned on
 *         errors.
 */
PP_Coalition PP_Unit_GetCoalition(PP_Unit unit);

/**
 * \brief Returns the unit type.
 *
 * \param unit to consult.
 * \return type id of the specified unit on success. -1 is returned on
 *          errors.
 * \see constantList_KP4.1.h - available units type ids
 */
int PP_Unit_GetType(PP_Unit unit);

/**
 * \brief Returns the unit position.
 *
 * \param unit to consult.
 * \return position of the specified unit on success. A position containing
 *         -1.0 is returned on errors.
 */
PP_Pos PP_Unit_GetPosition(PP_Unit unit);

/**
 * \brief Returns the unit health.
 *
 * \param unit to consult.
 * \return health of the specified unit on success. -1.0 is returned on
 *         errors.
 */
float PP_Unit_GetHealth(PP_Unit unit);

/**
 * \brief Returns the unit max health
 *
 * \param unit to consult.
 * \return maximum health that the specified unit can reach on success. -1.0
 *         is returned on errors.
 */
float PP_Unit_GetMaxHealth(PP_Unit unit);

/**
 * \brief Returns the allocation group of a unit
 *
 * \param unit to consult.
 * \return group number of the specified unit on success. -2 is returned, if
 *         specified unit isn't associated to a group. -1 is returned on
 *         errors.
 */
int PP_Unit_GetGroup(PP_Unit unit);

/**
 * \brief Allocates a unit to a specified group.
 *
 * Only units controled by the player can receive this command.
 * \param unit to command.
 * \param group : allocation group. group >= -1. If group == -1 then the specified unit
 *        is freed from its group.
 * \return 0 on success. -1 is returned on errors.
 */
int PP_Unit_SetGroup(PP_Unit unit, int group);

/**
 * \brief Read the first pending commands from a unit.
 * 
 * Reads the first #MAX_PDG_CMD pending commands from a unit and stores into the
 * locations pointed by "cmd". Only units controled by the player can give this
 * data.
 * \param unit to consult
 * \param cmd points to the PP_PendingCommands that will be filled in. It
 *      doesn't need to be set before calling this, but it must be allocated in
 *      memory.
 * \return 0 on success. -1 is returned on errors.
 */
int PP_Unit_GetPendingCommands(PP_Unit unit, PP_PendingCommands * cmd);

/**
 * \brief Commands a unit to carry out action on a specified unit.
 *
 * Only units controled by the player can receive this command. 
 * \warning This function call is not blocking. When the function returns this
 * means that the command is sent and not that the order is carried out. Carry
 * out an action can take time, keep this in mind.
 * \param unit to command.
 * \param action to carry out.
 * \param target unit.
 * \return 0 on success. -1 is returned on errors.
 * \see constantList_KP4.1.h - available unit actions depending on the unit type id.
 */
int PP_Unit_ActionOnUnit(PP_Unit unit, int action, PP_Unit target);

/**
 * \brief Commands a unit to carry out action on a specified position.
 *
 * Only units controled by the player can receive this command.
 * \warning This function call is not blocking. When the function returns this
 * means that the command is sent and not that the order is carried out. Carry
 * out an action can take time, keep this in mind.
 * \param unit to command.
 * \param action to carry out.
 * \param pos target position.
 * \return 0 on success. -1 is returned on errors.
 * \see constantList_KP4.1.h - available unit actions depending on the unit type id.
 */
int PP_Unit_ActionOnPosition(PP_Unit unit, int action, PP_Pos pos);

/**
 * \brief Commands a unit to carry out an untargeted action.
 *
 * Only units controled by the player can receive this command.
 * \warning This function call is not blocking. When the function returns this
 * means that the command is sent and not that the order is carried out. Carry
 * out an action can take time, keep this in mind.
 * \param unit to command.
 * \param action to carry out.
 * \param param : parameter to the action. If any parameter required, put -1.0 instead.
 * \return 0 on success. -1 is returned on errors.
 * \see constantList_KP4.1.h - available unit actions depending on the unit type id.
 */
int PP_Unit_UntargetedAction(PP_Unit unit, int action, float param);

#ifdef __cplusplus
}
#endif

#endif /* PP_CLIENT_H */
