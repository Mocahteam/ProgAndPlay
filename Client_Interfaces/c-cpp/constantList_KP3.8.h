#ifndef CONSTANT_LIST_KP38
#define CONSTANT_LIST_KP38
/*
 * List of constants for System units of Kernel Panic 3.8
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
#define POINTER           37
#define SIGNAL            40
#define SOCKET            41
#define TERMINAL          42

/******************************************************************************
 * Orders available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and *
 * SOCKET                                                                     *
 ******************************************************************************/
#define STOP              0     /* expect 0 parameters */
#define WAIT              5     /* expect 0 parameters */
#define FIRE_STATE        45    /* expect 1 parameter:
                                      0.0 => Hold fire
                                      1.0 => Return fire
                                      2.0 => Fire at will */
#define SELF_DESTRUCTION  65    /* expect 0 parameters */
#define REPEAT            115   /* expect 1 parameter:
                                      0.0 => Repeat off
                                      1.0 => Repeat on */
/*************************************************************************
 * Orders available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET *
 *************************************************************************/
#define MOVE              10    /* expect 1 parameter: a position or a unit */
#define PATROL            15    /* expect 1 parameter: a position or a unit */
#define FIGHT             16    /* expect 1 parameter: a position or a unit */
#define GUARD             25    /* expect 1 parameter: a position or a unit */
#define MOVE_STATE        50    /* expect 1 parameter:
                                      0.0 => Hold pos
                                      1.0 => Maneuver
                                      2.0 => Roam */
/**************************************************************************
 * Orders available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET *
 **************************************************************************/
#define ATTACK            20    /* expect 1 parameter: a position or a unit */
/**********************************
 * Orders available for ASSEMBLER *
 **********************************/
#define REPAIR            40    /* expect 1 parameter: a position or a unit */
#define RECLAIM           90    /* expect 1 parameter: a position or a unit */
#define RESTORE           110   /* expect 1 parameter: a position or a unit */
#define BUILD_BADBLOCK    -3    /* expect 1 parameter: a position or a unit */
#define BUILD_LOGIC_BOMB  -25   /* expect 1 parameter: a position or a unit */
#define BUILD_SOCKET      -41   /* expect 1 parameter: a position or a unit */
#define BUILD_TERMINAL    -42   /* expect 1 parameter: a position or a unit */
#define DEBUG             -34   /* expect 1 parameter: a position or a unit */
/*******************************
 * Orders available for KERNEL *
 *******************************/
#define BUILD_ASSEMBLER   -2    /* expect 1 parameter: a position or a unit */
#define BUILD_BYTE        -7    /* expect 1 parameter: a position or a unit */
#define BUILD_POINTER     -37   /* expect 1 parameter: a position or a unit */
/******************************************
 * Orders available for KERNEL and SOCKET *
 ******************************************/
#define BUILD_BIT         -4    /* expect 1 parameter: a position or a unit */
/*****************************
 * Orders available for BYTE *
 *****************************/
#define LAUNCH_MINE       33395 /* expect 0 parameters */
/********************************
 * Orders available for POINTER *
 ********************************/
#define NX_FLAG           33389 /* expect 1 parameter: a position or a unit */
/*********************************
 * Orders available for TERMINAL *
 *********************************/
#define SIGTERM           35126 /* expect 1 parameter: a position or a unit */

/****************/
/* Resources id */
/****************/
#define METAL  0
#define ENERGY 1

#endif
