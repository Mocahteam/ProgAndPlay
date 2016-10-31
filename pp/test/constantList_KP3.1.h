#ifndef CONSTANT_LIST_KP31
#define CONSTANT_LIST_KP31
/*
 * List of constants for System units of Kernel Panic 3.1
 */

/************
 * Units id *
 ************/
#define ASSEMBLER         2
#define BADBLOCK          3
#define BIT               4
#define BYTE              7
#define KERNEL            24
#define LOGIC_BOMB        25
#define POINTER           30
#define SIGNAL            33
#define SOCKET            34
#define TERMINAL          35

/******************************************************************************
 * Orders available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and *
 * SOCKET                                                                     *
 ******************************************************************************/
#define WAIT              5
/* use a position to set the FIRE_STATE value ({0.0, xxx} => Hold fire;
 * {1.0, xxx} => Return fire; {2.0, 0.0} => Fire at will) */
#define FIRE_STATE        45
#define SELF_DESTRUCTION  65
/* use a position to set the REPEAT value ({0.0, xxx} => Repeat off; {1.0, xxx}
 * => Repeat on) */
#define REPEAT            115
/*************************************************************************
 * Orders available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET *
 *************************************************************************/
#define MOVE              10
#define PATROL            15
#define FIGHT             16
#define GUARD             25
/* use a position to set the MOVE_STATE value ({0.0, xxx} => Hold pos;
 * {1.0, xxx} => Maneuver; {2.0, 0.0} => Roam) */
#define MOVE_STATE        50
/**************************************************************************
 * Orders available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET *
 **************************************************************************/
#define ATTACK            20
/**********************************
 * Orders available for ASSEMBLER *
 **********************************/
#define REPAIR            40
#define RECLAIM           90
#define RESTORE           110
/*****************************
 * Orders available for BYTE *
 *****************************/
#define LAUNCH_MINE       33395
/********************************
 * Orders available for POINTER *
 ********************************/
#define NX_FLAG           33389
/*********************************
 * Orders available for TERMINAL *
 *********************************/
#define SIGTERM           35126

#endif
