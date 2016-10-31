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

#ifndef PP_H
#define PP_H

/**
 * \file PP.h
 * \author Muratet.M
 * \date March 5, 2013
 *
 * \brief Defines main types of Prog&Play API.
 */
 
#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* Data definitions                                                          */
/*****************************************************************************/

/**
 * \brief Number of available coalitions
 * \see PP_Coalition
 */
#define NB_COALITIONS 3

/**
 * \brief Defines coalition ids.
 *
 * A coalition is a set of units.
 */
typedef enum {
  MY_COALITION,    /**< Coalition id for units controlled by the player. */
  ALLY_COALITION,  /**< Coalition id for units controlled by ally players. */
  ENEMY_COALITION  /**< Coalition id for units controlled by enemy players. */
} PP_Coalition;

/**
 * \brief Represents a game unit.
 * 
 * A PP_Unit is a unique id that represents a game unit in Prog&Play API.
 */
typedef int PP_Unit;

/**
 * \brief Represents a resource.
 * 
 * A PP_Resource is a unique id that represents a game resource in Prog&Play API.
 * \see constantList_KP4.1.h - available resources list
 */
typedef int PP_Resource;

/**
 * \brief Represents a position.
 * 
 * Defines a position representing a location (x, y) coordinate space.
 */
typedef struct {
  float x;
  float y;
} PP_Pos;

#ifdef __cplusplus
}
#endif

#endif /* PP_H */
