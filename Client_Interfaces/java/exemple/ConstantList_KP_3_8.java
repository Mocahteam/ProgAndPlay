package exemple;

/*
 * List of constants for System units of Kernel Panic 3.8
 */

class ConstantList_KP_3_8 {
	/************
	 * Units id *
	 ************/
	public static final int ASSEMBLER = 2;
	public static final int BADBLOCK = 3;
	public static final int BIT = 4;
	public static final int BYTE = 7;
	public static final int KERNEL = 24;
	public static final int LOGIC_BOMB = 25;
	public static final int POINTER = 37;
	public static final int SIGNAL = 40;
	public static final int SOCKET = 41;
	public static final int TERMINAL = 42;

	/**************************************************************************
	 * Orders available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER *
	 * and SOCKET                                                             *
	 **************************************************************************/
	public static final int STOP = 0;
	public static final int WAIT = 5;
	public static final int FIRE_STATE = 45;
	public static final int SELF_DESTRUCTION = 65;
	public static final int REPEAT = 115;
	/*************************************************************************
	 * Orders available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET *
	 *************************************************************************/
	public static final int MOVE = 10;
	public static final int PATROL = 15;
	public static final int FIGHT = 16;
	public static final int GUARD = 25;
	public static final int MOVE_STATE = 50;
	/**************************************************************************
	 * Orders available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET *
	 **************************************************************************/
	public static final int ATTACK = 20;
	/**********************************
	 * Orders available for ASSEMBLER *
	 **********************************/
	public static final int REPAIR = 40;
	public static final int RECLAIM = 90;
	public static final int RESTORE = 110;
	/*****************************
	 * Orders available for BYTE *
	 *****************************/
	public static final int LAUNCH_MINE = 33395;
	/********************************
	 * Orders available for POINTER *
	 ********************************/
	public static final int NX_FLAG = 33389;
	/*********************************
	 * Orders available for TERMINAL *
	 *********************************/
	public static final int SIGTERM = 35126;
	
	
	/****************
	 * Resources id *
	 ****************/
	public static final int METAL = 0;
	public static final int ENERGY = 1;
}