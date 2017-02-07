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

#ifndef PP_CLIENT_PRIVATE_H
#define PP_CLIENT_PRIVATE_H

#include "PP.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UNIT_NOT_FOUND -1
#define NOT_UNIT_OWNER -2
#define INVALID_COALITION -3
#define TARGET_NOT_FOUND -4
#define OUT_OF_RANGE -5
#define INVALID_GROUP -6
#define POSITION_OUT_OF_BOUNDS -7
#define FEEDBACK_COUNT_LIMIT -8
#define SHARED_MEMORY_ERROR -100
#define NOT_OPENED -101
#define RELOAD_FAILURE -102
#define INCONSISTENT_SHARED_MEMORY -103
#define MUTEX_NOT_LOCKED -104
#define TYPE_UNDEF -105
#define NO_GROUP -200

/******************************************************************************/
/* Functions to manage Prog&Play                                              */
/******************************************************************************/

/**
 * \brief Opens Prog&Play API.
 *
 * This must be called before using other functions in this library.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Open_prim(void);

/**
 * \brief Shutdowns and cleans up Prog&Play API.
 *
 * After calling this Prog&Play functions should not be used. You may, of
 * course, use PP_Open to use the functionality again.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Close_prim(void);

/******************************************************************************/
/* Functions to manage game elements                                          */
/******************************************************************************/

/**
 * \brief Checks if the game is over.
 * \return positive value if the game is ended and 0 if not. Negative value is
 *         returned on errors.
 */
int PP_IsGameOver_prim(void);

/**
 * \brief Checks if the game is paused.
 * \return positive value if the game is paused and 0 if not. Negative value is
 *         returned on errors.
 */
int PP_IsGamePaused_prim(void);

/**
 * \brief Retrieve the timestamp updated by the game engine in the shared memory
 *
 * \return the timestamp value on success.
 *         -1 is returned on errors.
 */
int PP_GetTimestamp_prim(void);

/**
 * \brief Sets map size.
 *
 * Sets map size as a PP_Pos. Fields of PP_Pos are the max values available
 * for the current map.
 * \param mapSize points to the PP_Pos that will be filled in. It doesn't need
 *      to be set before calling this, but it must be allocated in memory.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_GetMapSize_prim(PP_Pos * mapSize);

/**
 * \brief Sets start position.
 *
 * The start position is the position where the player's army is located at the
 * beginning of the game.
 * \param startPos points to the PP_Pos that will be filled in. It doesn't need
 *      to be set before calling this, but it must be allocated in memory.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_GetStartPosition_prim(PP_Pos * startPos);

/**
 * \brief Returns number of special areas
 *
 * \return the number of special areas on success. Negative value is returned on
 *      errors.
 */
int PP_GetNumSpecialAreas_prim(void);

/**
 * \brief Sets position of a special area.
 *
 * \param num : an id of special area, it must be included meanwhile [0;n[ where
*         n is the number of special areas.
* \param pos points to the PP_Pos that will be filled in. It doesn't need to be
*         set before calling this, but it must be allocated in memory.
 * \return 0 on success.  Negative value is returned on errors.
 * \see PP_GetNumSpecialAreas_prim
 */
int PP_GetSpecialAreaPosition_prim(int num, PP_Pos * pos);

/**
 * \brief Returns the current level of a resource.
 *
 * \param id of resource that you want know level.
 * \return the current level of specified resource. Negative value is returned
 *        on errors.
 */
int PP_GetResource_prim(PP_Resource id);

/**
 * \brief Returns the number of units from a coalition.
 *
 * Only visible units by the player are counted.
 * \param c : coalition to consult.
 * \return the number of units (visible by the player) from this coalition.
 *         Negative value is returned on errors.
 */
int PP_GetNumUnits_prim(PP_Coalition c);

/**
 * \brief Returns the nth visible unit of a coalition.
 *
 * \param c the coalition to consult.
 * \param index of a unit in the specified coalition, must be included
 *        meanwhile [0;n[ where n is the number of units of this coalition.
 * \return unit at the specified index on success. Negative value is returned on
 *        errors.
 * \see PP_GetNumUnits_prim
 */
PP_Unit PP_GetUnitAt_prim(PP_Coalition c, int index);

/******************************************************************************/
/* Functions to manage unit                                                   */
/******************************************************************************/

/**
 * \brief Returns the unit coalition.
 *
 * \param unit to consult.
 * \return coalition of the specified unit on success. Negative value is
 *         returned on errors.
 */
PP_Coalition PP_Unit_GetCoalition_prim(PP_Unit unit);

/**
 * \brief Returns the unit type.
 *
 * \param unit to consult.
 * \return type id of the specified unit on success. Negative value is returned
 *          on errors.
 */
int PP_Unit_GetType_prim(PP_Unit unit);

/**
 * \brief Sets position of a unit.
 *
 * \param unit to consult.
 * \param pos points to the PP_Pos that will be filled in. It doesn't need to be
 *         set before calling this, but it must be allocated in memory.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Unit_GetPosition_prim(PP_Unit unit, PP_Pos * pos);

/**
 * \brief Returns the unit health.
 *
 * \param unit to consult.
 * \return health of the specified unit on success. Negative value is returned
 *         on errors.
 */
float PP_Unit_GetHealth_prim(PP_Unit unit);

/**
 * \brief Returns the unit max health
 *
 * \param unit to consult.
 * \return maximum health that the specified unit can reach on success. Negative
 *         value is returned on errors.
 */
float PP_Unit_GetMaxHealth_prim(PP_Unit unit);

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
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Unit_GetPendingCommands_prim(PP_Unit unit, PP_PendingCommands * cmd);

/**
 * \brief Returns the allocation group of a unit
 *
 * \param unit to consult.
 * \return group number of the specified unit on success. Negative value is
 *         returned on errors. -200 code means unit isn't associated to a group.
 */
int PP_Unit_GetGroup_prim(PP_Unit unit);

/**
 * \brief Allocates a unit to a specified group.
 *
 * Only units controled by the player can receive this command.
 * \param unit to command.
 * \param group: allocation group. group >= -1. If group == -1 then the
 *               specified unit is freed from its group.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Unit_SetGroup_prim(PP_Unit unit, int group);

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
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Unit_ActionOnUnit_prim(PP_Unit unit, int action, PP_Unit target);

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
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Unit_ActionOnPosition_prim(PP_Unit unit, int action, PP_Pos pos);

/**
 * \brief Commands a unit to carry out an untargeted action.
 *
 * Only units controled by the player can receive this command.
 * \warning This function call is not blocking. When the function returns this
 * means that the command is sent and not that the order is carried out. Carry
 * out an action can take time, keep this in mind.
 * \param unit to command.
 * \param action to carry out.
 * \param param : parameter to the action. If any parameter required, put -1.0
 *        instead.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Unit_UntargetedAction_prim(PP_Unit unit, int action, float param);

/******************************************************************************/
/* Critical section management                                                */
/******************************************************************************/

/*
 * This function has to be called before entering in critical section.
 * Don't forget to call "exitCriticalSection()" after critical section.
 */
void enterCriticalSection(void);

/*
 * This function has to be called at the end of a critical section.
 */
void exitCriticalSection(void);

/******************************************************************************/
/* WARNING !!! Following function has to be used after a first call of        */
/* "enterCriticalSection()". Don't forget to call "exitCriticalSection()" when*/
/* you finish to use these functions.                                         */
/******************************************************************************/

/*
 * \brief Returns the number of pending commands for a unit. Only units
 *        controled by the player can give this data.
 * \param unit: unit to consult.
 * \return the number of pending commands of this unit. Negative value is
 *         returned on errors.
 */
int PP_Unit_GetNumPdgCmds_prim(PP_Unit unit);

/*
 * \brief Sets the nth pending command code of a unit. Only units controled
 *        by the player can give this data.
 * \param unit: unit to consult.
 * \param idCmd: id of the pending command of unit "unit", must be included
 *        meanwhile [0;n[ where n is the number of pending commands of this
 *        unit.
 * \param cmdCode points to the Integer that will be filled in. It doesn't need
 *        to be set before calling this, but it must be allocated in memory.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Unit_PdgCmd_GetCode_prim(PP_Unit unit, int idCmd, int * cmdCode);

/*
 * \brief Returns the number of parameters of the nth pending command code of a
 *        unit. Only units controled by the player can give this data.
 * \param unit: unit to consult.
 * \param idCmd: id of the pending command of unit "unit", must be included
 *        meanwhile [0;n[ where n is the number of pending commands of this
 *        unit.
 * \return number of parameters of command at the specified index on success.
 *         Negative value is returned on errors.
 */
int PP_Unit_PdgCmd_GetNumParams_prim(PP_Unit unit, int idCmd);

/*
 * \brief Sets the nth parameter of a pending command code of a unit. Only units
 *        controled by the player can give this data.
 * \param unit: unit to consult.
 * \param idCmd: id of the pending command of unit "unit", must be included
 *        meanwhile [0;n[ where n is the number of pending commands of this
 *        unit.
 * \param idParam: id of the parameter of the command identified by "idCmd",
 *        must be included meanwhile [0;n[ where n is the number of parameters
 *        for this pending command of this unit.
 * \param paramValue points to the float that will be filled in. It doesn't need
 *        to be set before calling this, but it must be allocated in memory.
 * \return 0 on success. Negative value is returned on errors.
 */
int PP_Unit_PdgCmd_GetParam_prim(PP_Unit unit, int idCmd, int idParam,
		float * paramValue);

/*
 * \brief Push a message to the game engine in order to write it into log file
 * \param msg: message to log.
 * \param error: error code to log provided by other functions of this library.
 *        If the error code is a positive or null value, the error code will be
 *        ignored
 * \return 0 is returned on success. Negative value is returned on errors.
 */
int PP_PushMessage_prim(const char * msg, const int * error);

#ifdef __cplusplus
}
#endif

#endif /* PP_CLIENT_PRIVATE_H */
