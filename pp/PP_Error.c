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

#include "PP_Error_Private.h"
#include "PP_Error.h"

#include <stdio.h>
#include <stdarg.h>

#define MAX_ERROR_SIZE 1024
static char PP_Error[MAX_ERROR_SIZE];

/*
 * Returns the last error set as a string ended by NULL. This string is
 * statically allocated and hasn't to be freed by the user.
 */
char * PP_GetError(void){
  return PP_Error;
}

/*
 * Deletes all informations from the last internal error. Useful if error has
 * been processed.
 */
void PP_ClearError(void){
  PP_Error[0] = '\0';
}

/*
 * Defines an error generated in the Prog&Play API. Possibility to format text
 * like a printf
 */
void PP_SetError(const char *format, ...) {
  int n;
  va_list ap;

  va_start(ap, format);
  n = vsnprintf(PP_Error, MAX_ERROR_SIZE, format, ap);
  va_end(ap);

  if (n <= -1){
    PP_ClearError();
    return;
  }
  if (n >= MAX_ERROR_SIZE){
    PP_Error[MAX_ERROR_SIZE-1] = '\0';
    return;
  }
}
