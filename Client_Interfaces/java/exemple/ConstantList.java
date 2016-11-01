package exemple;

/*
 * Liste de constante pour le jeu Kernel Panic 2.2 Div 0
 */

class ConstantList {
  /*******************
   * Ordres basiques *
   *******************/
  public static final int MOVE = 10;        // code de l'action déplacement
  public static final int PATROL = 15;      // code de l'action patrouille
  public static final int FIGHT = 16;       // code de l'action combat
  public static final int ATTACK = 20;      // code de l'action attaque
  public static final int GUARD = 25;       // code de l'action garde
  public static final int REPAIR = 40;      // code de l'action réparation
  public static final int RECLAIM = 90;     // code de l'action réclamation
  public static final int RESTORE = 110;    // code de l'action restauration
  
  /*************************************
  * Identifiants des types des unités *
  *************************************/
  // Commun à tout les camps
  public static final int BADBLOCK = -2;          // code de l'unité BADBLOCK
  public static final int LOGICBOMB = -18;        // code de l'unité LOGICBOMB
  public static final int DEBUG = -19;            // code de l'unité DEBUG
  // Pour le camp System
  public static final int KERNEL = -17;           // code de l'unité KERNEL
  public static final int SOCKET = -23;           // code de l'unité SOCKET
  public static final int TERMINAL = -24;         // code de l'unité TERMINAL
  public static final int ASSEMBLER = -1;         // code de l'unité ASSEMBLER
  public static final int BIT = -3;               // code de l'unité BIT
  public static final int BYTE = -6;              // code de l'unité BYTE
  public static final int POINTER = -21;          // code de l'unité POINTER
  // Pour le camp Hacker
  public static final int HOLE = -15;             // code de l'unité HOLE
  public static final int WINDOW = -28;           // code de l'unité WINDOW
  public static final int OBELISK = -20;          // code de l'unité OBELISK
  public static final int TROJAN = -25;           // code de l'unité TROJAN
  public static final int VIRUS = -27;            // code de l'unité VIRUS
  public static final int BUG = -4;               // code de l'unité BUG
  public static final int EXPLOIT = -12;          // code de l'unité EXPLOIT
  public static final int WORM = -30;             // code de l'unité WORM
  public static final int DENIALOFSSERVICE = -8;  // code de l'unité 
                                                  // DENIALOFSERVICE
  
  
  /*******************
  * Armes spéciales *
  *******************/
  public static final int NXFALG = 33389;       // code de l'arme spéciale du
                                                // POINTER
  public static final int LAUNCHMINES = 33395;  // code de l'arme spéciale du
                                                // BYTE
  public static final int SIGTERM = 35126;      // code de l'arme du SIGTERM
  public static final int DEPLOY = 33390;       // code de l'action spéciale du
                                                // BUG
  public static final int BOMBARD = 33394;      // code de l'arme spéciale du
                                                // BUG
  public static final int UNDEPLOY = 33391;     // code de l'action spéciale de
                                                // l'EXPLOIT
  public static final int AUTOHOLD = 32103;     // code de l'arme de l'OBELISK
}