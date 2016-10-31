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


/*
 * This function has to be called before entering in critical section.
 * Don't forget to call "exitCriticalSection()" after critical section.
 */
void enterCriticalSection();

/*
 * This function has to be called at the end of a critical section.
 */
void exitCriticalSection();

/******************************************************************************/
/* WARNING !!! Following function has to be used after a first call of        */
/* "enterCriticalSection()". Don't forget to call "exitCriticalSection()" when*/
/* you finish to use these functions.                                         */
/******************************************************************************/

/*
 * Returns the number of pending commands for a unit. Only units controled by
 * the player can give this data.
 * unit : unit to consult.
 * Returns : the number of pending commands of this unit.
 *           -1 is returned on errors.
 */
int PP_Unit_GetNumPdgCmds(PP_Unit unit);

/*
 * Returns the nth pending command code of a unit. Only units controled by the
 * player can give this data.
 * unit : unit to consult.
 * idCmd : id of the pending command of unit "unit", must be included meanwhile
 *         [0;n[ where n is the number of pending commands of this unit.
 * Returns : command code at the specified index on success.
 *           -1 is returned on errors.
 */
int PP_Unit_PdgCmd_GetCode(PP_Unit unit, int idCmd);

/*
 * Returns the number of parameters of the nth pending command code of a unit.
 * Only units controled by the player can give this data.
 * unit : unit to consult.
 * idCmd : id of the pending command of unit "unit", must be included meanwhile
 *         [0;n[ where n is the number of pending commands of this unit.
 * Returns : number of parameters of command at the specified index on success.
 *           -1 is returned on errors.
 */
int PP_Unit_PdgCmd_GetNumParams(PP_Unit unit, int idCmd);

/*
 * Returns the nth parameter of a pending command code of a unit. Only units
 * controled by the player can give this data.
 * unit : unit to consult.
 * idCmd : id of the pending command of unit "unit", must be included meanwhile
 *         [0;n[ where n is the number of pending commands of this unit.
 * idParam : id of the parameter of the command identified by "idCmd", must be
 *           included meanwhile [0;n[ where n is the number of parameters for 
 *           this pending command of this unit.
 * Returns : parameter at the specified index on success.
 *           -1 is returned on errors.
 */
float PP_Unit_PdgCmd_GetParam(PP_Unit unit, int idCmd, int idParam);

/*
 * Push a message to the game engine in order to write it into log file
 * msg : message to log.
 * Returns : 0 is returned on success.
 *           -1 is returned on errors.
 */
int PP_PushMessage(const char * msg);

#ifdef __cplusplus
}
#endif

#endif /* PP_CLIENT_PRIVATE_H */
