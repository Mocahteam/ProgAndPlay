package exemple;

/*
 * List of constants for System units of Kernel Panic 4.4
 */

public class ConstantList_KP_4_4 {
	/************
	 * Units id *
	 ************/
	public static final int ASSEMBLER = 2;
	public static final int BADBLOCK = 3;
	public static final int BIT = 4;
	public static final int BYTE = 7;
	public static final int KERNEL = 25;
	public static final int LOGIC_BOMB = 26;
	public static final int POINTER = 39;
	public static final int SIGNAL = 44;
	public static final int SOCKET = 45;
	public static final int TERMINAL = 46;

	/**************************************************************************
	 * Orders available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER *
	 * and SOCKET                                                             *
	 **************************************************************************/
	public static final int STOP = 0;              /* expect 0 parameters */
	public static final int WAIT = 5;              /* expect 0 parameters */
	public static final int FIRE_STATE = 45;       /* expect 1 parameters:
	                                                    0.0 => Hold fire
	                                                    1.0 => Return fire
	                                                    2.0 => Fire at will */
	public static final int SELF_DESTRUCTION = 65; /* expect 0 parameters */
	public static final int REPEAT = 115;          /* expect 1 parameter:
	                                                    0.0 => Repeat off
	                                                    1.0 => Repeat on */
	
	/*************************************************************************
	 * Orders available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET *
	 *************************************************************************/
	public static final int MOVE = 10;       /* expect 1 parameter: a position or a unit */
	public static final int PATROL = 15;     /* expect 1 parameter: a position or a unit */
	public static final int FIGHT = 16;      /* expect 1 parameter: a position or a unit */
	public static final int GUARD = 25;      /* expect 1 parameter: a position or a unit */
	public static final int MOVE_STATE = 50; /* expect 1 parameter:
	                                                    0.0 => Hold pos
	                                                    1.0 => Maneuver
	                                                    2.0 => Roam */
	
	/**************************************************************************
	 * Orders available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET *
	 **************************************************************************/
	public static final int ATTACK = 20;  /* expect 1 parameter: a position or a unit */
	
	/**********************************
	 * Orders available for ASSEMBLER *
	 **********************************/
	public static final int REPAIR = 40;            /* expect 1 parameter: a position or a unit */
	public static final int RECLAIM = 90;           /* expect 1 parameter: a position or a unit */
	public static final int RESTORE = 110;          /* expect 1 parameter: a position or a unit */
	public static final int BUILD_BADBLOCK = -3;    /* expect 1 parameter: a position or a unit */
	public static final int BUILD_LOGIC_BOMB = -26; /* expect 1 parameter: a position or a unit */
	public static final int BUILD_SOCKET = -45;     /* expect 1 parameter: a position or a unit */
	public static final int BUILD_TERMINAL = -46;   /* expect 1 parameter: a position or a unit */
	public static final int DEBUG = -35;            /* expect 1 parameter: a position or a unit */
	
	/*******************************
	* Orders available for KERNEL *
	*******************************/
	public static final int BUILD_ASSEMBLER = -2; /* expect 1 parameter: a position or a unit */
	public static final int BUILD_BYTE = -7;      /* expect 1 parameter: a position or a unit */
	public static final int BUILD_POINTER = -39;  /* expect 1 parameter: a position or a unit */
	
	/******************************************
	* Orders available for KERNEL and SOCKET *
	******************************************/
	public static final int BUILD_BIT = -4;        /* expect 1 parameter: a position or a unit */
	public static final int STOP_BUILDING = -7658; /* expect 0 parameters */
	
	/*****************************
	 * Orders available for BYTE *
	 *****************************/
	public static final int LAUNCH_MINE = 33395; /* expect 0 parameters */
	/********************************
	 * Orders available for POINTER *
	 ********************************/
	public static final int NX_FLAG = 33389; /* expect 1 parameter: a position or a unit */
	/*********************************
	 * Orders available for TERMINAL *
	 *********************************/
	public static final int SIGTERM = 35126; /* expect 1 parameter: a position or a unit */
	
	
	/****************
	 * Resources id *
	 ****************/
	public static final int METAL = 0;
	public static final int ENERGY = 1;
}