#ifndef CONSTANT_LIST_KP41
#define CONSTANT_LIST_KP41
/**
 * \file constantList_KP4.1.h
 * \author Muratet.M
 * \date March 5, 2013
 *
 * \brief List of constants for System units of Kernel Panic 4.1.
 */

/** \name Units type id */
/**@{*/
 
/** \brief The assembler is a construction unit.
 * 
 * It can build sockets, but it cannot assist-build. Slow, little health. Equipped with a radar to detect mines and other cloacked units. <img src="units/assembler.png" />
 * \see PP_Unit_GetType */
#define ASSEMBLER         2

/** \brief A tiny wall, built by Assembler.
 * 
 * Blocks small units movement. Do not block shots however. Easily removed by the Debug or simply crushing them with a Byte. <img src="units/badblock.png" /> 
 * \see PP_Unit_GetType */
#define BADBLOCK          3

/** \brief Your basic attacking unit.
 * 
 * Cheap, fast, small, not very much health. Is armed with a SPARCling laser. Can be built by a kernel or a socket. <img src="units/bits.png" />
 * \see PP_Unit_GetType */
#define BIT               4

/** \brief A large, strong, and slow attacking unit.
 * 
 * Can holds it's own against many bits, as it has lots of health and a powerful gun. More armored when closed. Can plow through bad blocks. The byte has an alternate firing mode, the mine launcher, which throws 5 mines at the cost of much health. <img src="units/byte.png" />
 * \see PP_Unit_GetType */
#define BYTE              7

/** \brief Main building of System faction.
 * 
 * It can build all mobile units in the game. Has rapid auto-heal and lots of health. <img src="units/kernel.png" /> 
 * \see PP_Unit_GetType */
#define KERNEL            25

/** \brief A mine.
 * 
 * It can be built by the Assembler, and also launched by Bytes. It takes out Bits in a single blow, has a decent damage radius, and doesn't chain explode. Use with care, as the blast hurts your own units too. Limited to 32. <img src="units/mine.png" /> 
 * \see PP_Unit_GetType */
#define LOGIC_BOMB        26

/** \brief An artillery unit.
 * 
 * Its normal shot is not so useful against moving units, but can kill kernels and sockets pretty quickly. Is slow and has little health, so it needs protection. The pointer has an alternate firing mode, the NX Flag, which set a wide area ablaze for a minute, causing constant damage to all units, or buildings within it's range. <img src="units/pointer.png" />
 * \see PP_Unit_GetType */
#define POINTER           39

/** \brief A nuclear bomb.
 * 
 * It is launched by Terminal. <img src="units/terminal.png" />
 * \see PP_Unit_GetType */
#define SIGNAL            44

/** \brief A factory.
 * 
 * It can only be built on special areas. It can solely build Bits, and slower than the Kernel can. It autoheals, and has a decent amount of health. <img src="units/socket.png" />
 * \see PP_Unit_GetType */
#define SOCKET            45 

/** \brief A missile launcher.
 * 
 * It can only be built on special areas. It can dispatch a Signal (nuclear bomber) once every 90 seconds that will deal about 16000 damage to a large target area, i.e. it destroys everything except factories. It does much less damage to the Kernel. The bomber can strike any position on the map, there is no defense. <img src="units/terminal.png" />
 * \see PP_Unit_GetType */
#define TERMINAL          46
/**@}*/


/** \name Orders available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET */
/**@{*/
#define STOP              0     /**< Stop current action and remove all pending commands. \see PP_Unit_UntargetedAction */
#define WAIT              5     /**< Suspend/Unsuspend current action. \see PP_Unit_UntargetedAction */
#define FIRE_STATE        45    /**< Defines the fire state and expect 1 parameter:\n
                                      0.0 => Hold fire, the unit doesn't attack automatically;\n
                                      1.0 => Return fire, the unit responds if it is attacked;\n
                                      2.0 => Fire at will, the unit attacks automatically. \see PP_Unit_UntargetedAction */
#define SELF_DESTRUCTION  65    /**< Self destruction of the unit. \see PP_Unit_UntargetedAction */
#define REPEAT            115   /**< Defines the repeat mode and expect 1 parameter:\n
                                      0.0 => Repeat off, the unit doesn't repeat pending commands;\n
                                      1.0 => Repeat on, the unit repeats pending commands. \see PP_Unit_UntargetedAction */
/**@}*/

/** \name Orders available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET */
/**@{*/
#define MOVE              10    /**< The unit moves to a target. \see PP_Unit_ActionOnUnit, PP_Unit_ActionOnPosition */
#define PATROL            15    /**< The unit patrols between its current position and a target. \see PP_Unit_ActionOnUnit, PP_Unit_ActionOnPosition */
#define FIGHT             16    /**< The unit progresses to a target but units will stop to kill enemies along the way. \see PP_Unit_ActionOnUnit, PP_Unit_ActionOnPosition */
#define GUARD             25    /**< The unit guards a target. \see PP_Unit_ActionOnUnit, PP_Unit_ActionOnPosition */
#define MOVE_STATE        50    /**< Defines the move state and expect 1 parameter:\n
                                      0.0 => Hold pos, the unit doesn't move automatically;\n
                                      1.0 => Maneuver, the unit stays near from its position;\n
                                      2.0 => Roam, the unit is not blocked around its position. \see PP_Unit_UntargetedAction */
/**@}*/

/** \name Orders available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET */
/**@{*/
#define ATTACK            20    /**< The unit progresses to a target, attacks ennemy units in its line of sight but doesn't stop its advance. \see PP_Unit_ActionOnUnit, PP_Unit_ActionOnPosition */
/**@}*/

/** \name Orders available for ASSEMBLER */
/**@{*/
#define REPAIR            40    /**< The unit repairs a target. \see PP_Unit_ActionOnUnit, PP_Unit_ActionOnPosition */
#define BUILD_BADBLOCK    -3    /**< The unit builds a Bad Block on a target. \see PP_Unit_ActionOnPosition */
#define BUILD_LOGIC_BOMB  -26   /**< The unit builds a Logic Bomb on a target. \see PP_Unit_ActionOnPosition */
#define BUILD_SOCKET      -45   /**< The unit builds a Socket on a target. \see PP_Unit_ActionOnPosition */
#define BUILD_TERMINAL    -46   /**< The unit builds a Terminal on a target. \see PP_Unit_ActionOnPosition */
#define DEBUG             -35   /**< The unit builds a Debug on a target (clear an area of all mines and walls). \see PP_Unit_ActionOnPosition */
/**@}*/

/** \name Orders available for KERNEL */
/**@{*/
#define BUILD_ASSEMBLER   -2    /**< The unit builds an Assembler. \see PP_Unit_UntargetedAction, PP_Unit_ActionOnPosition */
#define BUILD_BYTE        -7    /**< The unit builds a Byte. \see PP_Unit_UntargetedAction, PP_Unit_ActionOnPosition */
#define BUILD_POINTER     -39   /**< The unit builds a Pointer. \see PP_Unit_UntargetedAction, PP_Unit_ActionOnPosition */
/**@}*/

/** \name Orders available for KERNEL and SOCKET */
/**@{*/
#define BUILD_BIT         -4    /**< The unit builds a Bit. \see PP_Unit_UntargetedAction, PP_Unit_ActionOnPosition */
#define STOP_BUILDING     -7658 /**< The unit stops building units. \see PP_Unit_UntargetedAction */
/**@}*/

/** \name Orders available for BYTE */
/**@{*/
#define LAUNCH_MINE       33395 /**< The unit launches mines. \see PP_Unit_UntargetedAction */
/**@}*/

/** \name Orders available for POINTER */
/**@{*/
#define NX_FLAG           33389 /**< The unit launches an NX Flag. \see PP_Unit_ActionOnUnit, PP_Unit_ActionOnPosition */
/**@}*/

/** \name Orders available for TERMINAL */
/**@{*/
#define SIGTERM           35126 /**< The unit launches a Sigterm. \see PP_Unit_ActionOnUnit, PP_Unit_ActionOnPosition */
/**@}*/

/** \name Resources id */
/**@{*/
#define METAL  0 /**< Not used in Kernel Panic. \see PP_GetResource */
#define ENERGY 1 /**< Not used in Kernel Panic. \see PP_GetResource */
/**@}*/

#endif
