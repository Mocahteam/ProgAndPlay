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

#ifdef __cplusplus
extern "C" {
#endif

void Ada_Open () {
	if (PP_Open_prim() == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss_start(std::ostringstream::out);
			oss_start << EXECUTION_START_TIME << " " << PP_GetTimestamp_prim();
			PP_PushMessage_prim(oss_start.str().c_str(), NULL);
			std::ostringstream oss_lang(std::ostringstream::out);
			oss_lang << PROGRAMMING_LANGUAGE_USED << " Ada";
			PP_PushMessage_prim(oss_lang.str().c_str(), NULL);
			PP_PushMessage_prim("PP_Open", NULL);
		exitCriticalSection();
	}
}

/* Le passage de structure de Ada à C se fait par référence, pour cette raison.
 * Pour éviter les cas d'erreur le type position et passé sous la forme de 2
 * float et est recounstruite pour l'appel final à Prog&Play */
void Ada_ActionOnPosition (PP_Unit unit, int action, float x, float y, int s){
	PP_Pos p;
	p.x = x;
	p.y = y;
	PP_Unit_ActionOnPosition (unit, action, p, s);
}

#ifdef __cplusplus
}
#endif
