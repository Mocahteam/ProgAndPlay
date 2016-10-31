/*
 * List of constants for Kernel Panic 2.2 Div 0
 */

/****************
 * Basic orders *
 ****************/
#define MOVE 10
#define PATROL 15
#define FIGHT 16
#define ATTACK 20
#define GUARD 25
#define REPAIR 40
#define RECLAIM 90
#define RESTORE 110

/************
 * Units id *
 ************/
// All side units
#define BADBLOCK 2
#define LOGICBOMB 18
#define DEBUG 19
// System units
#define KERNEL 17
#define SOCKET 23
#define TERMINAL 24
#define ASSEMBLER 1
#define BIT 3
#define BYTE 6
#define POINTER 21
// Hacker units
#define HOLE 15
#define WINDOW 28
#define OBELISK 20
#define TROJAN 25
#define VIRUS 27
#define BUG 4
#define EXPLOIT 12
#define WORM 30
#define DENIALOFSSERVICE 8

/*******************************
 * Special Weapons and actions *
 *******************************/
#define NXFALG 33389         // Special POINTER weapon
#define LAUNCHMINES 33395    // Special BYTE weapon
#define SIGTERM 35126        // TERMINAL weapon
#define DEPLOY 33390         // Special BUG action
#define BOMBARD 33394        // Special BUG action
#define UNDEPLOY 33391       // Special EXPLOIT action
#define AUTOHOLD 32103       // OBELISK weapon
