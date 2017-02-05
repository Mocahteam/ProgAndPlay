# File : constantList_KP4_1.py
# Author : Muratet.M
# Date : January 3, 2015
#
# Brief : List of constants for System units of Kernel Panic 4.1.
#

####################################
# List of available units type ids #
####################################

# The assembler is a construction unit.
# It can build sockets, but it cannot assist-build. Slow, little health.
# Equipped with a radar to detect mines and other cloacked units.
# See PP_Unit_GetType on pp.py
ASSEMBLER = 2

# A tiny wall, built by Assembler.
# Blocks small units movement. Do not block shots however. Easily removed by the
# Debug or simply crushing them with a Byte.
# See PP_Unit_GetType on pp.py
BADBLOCK = 3

# Your basic attacking unit.
# Cheap, fast, small, not very much health. Is armed with a SPARCling laser. Can
# be built by a kernel or a socket.
# See PP_Unit_GetType on pp.py
BIT = 4

# A large, strong, and slow attacking unit.
# Can holds it's own against many bits, as it has lots of health and a powerful
# gun. More armored when closed. Can plow through bad blocks. The byte has an
# alternate firing mode, the mine launcher, which throws 5 mines at the cost of
# much health.
# See PP_Unit_GetType on pp.py
BYTE = 7

# Main building of System faction.
# It can build all mobile units in the game. Has rapid auto-heal and lots of
# health.
# See PP_Unit_GetType on pp.py
KERNEL = 25

# A mine.
# It can be built by the Assembler, and also launched by Bytes. It takes out
# Bits in a single blow, has a decent damage radius, and doesn't chain explode.
# Use with care, as the blast hurts your own units too. Limited to 32.
# See PP_Unit_GetType pp.py
LOGIC_BOMB = 26

# An artillery unit.
# Its normal shot is not so useful against moving units, but can kill kernels
# and sockets pretty quickly. Is slow and has little health, so it needs
# protection. The pointer has an alternate firing mode, the NX Flag, which set
# a wide area ablaze for a minute, causing constant damage to all units, or
# buildings within it's range.
# See PP_Unit_GetType on pp.py
POINTER = 39

# A nuclear bomb.
# It is launched by Terminal.
# See PP_Unit_GetType on pp.py
SIGNAL = 44

# A factory.
# It can only be built on special areas. It can solely build Bits, and slower
# than the Kernel can. It autoheals, and has a decent amount of health.
# See PP_Unit_GetType on pp.py
SOCKET = 45 

# A missile launcher.
# It can only be built on special areas. It can dispatch a Signal (nuclear
# bomber) once every 90 seconds that will deal about 16000 damage to a large
# target area, i.e. it destroys everything except factories. It does much less
# damage to the Kernel. The bomber can strike any position on the map, there is
# no defense.
# See PP_Unit_GetType on pp.py
TERMINAL = 46

##########################################################
# List of available order ids depending on unit type ids #
##########################################################

# Orders available for ASSEMBLER, BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and
# SOCKET
STOP = 0 # Stop current action and remove all pending commands.
         # See PP_Unit_UntargetedAction on pp.py
WAIT = 5 # Suspend/Unsuspend current action.
         # See PP_Unit_UntargetedAction on pp.py
FIRE_STATE = 45 # Defines the fire state and expect 1 parameter:
                #    0.0 => Hold fire, the unit doesn't attack automatically
                #    1.0 => Return fire, the unit responds if it is attacked
                #    2.0 => Fire at will, the unit attacks automatically.
                # See PP_Unit_UntargetedAction on pp.py
SELF_DESTRUCTION = 65 # Self destruction of the unit.
                      # See PP_Unit_UntargetedAction on pp.py
REPEAT = 115 # Defines the repeat mode and expect 1 parameter:
             #    0.0 => Repeat off, the unit doesn't repeat pending commands
             #    1.0 => Repeat on, the unit repeats pending commands.
             # See PP_Unit_UntargetedAction on pp.py

# Orders available for ASSEMBLER, BIT, BYTE, KERNEL, POINTER and SOCKET
MOVE = 10 # The unit moves to a target.
          # See PP_Unit_ActionOnUnit and PP_Unit_ActionOnPosition on pp.py
PATROL = 15 # The unit patrols between its current position and a target.
            # See PP_Unit_ActionOnUnit and PP_Unit_ActionOnPosition
FIGHT = 16 # The unit progresses to a target but units will stop to kill enemies
           # along the way.
           # See PP_Unit_ActionOnUnit and PP_Unit_ActionOnPosition on pp.py
GUARD = 25 # The unit guards a target.
           # See PP_Unit_ActionOnUnit and PP_Unit_ActionOnPosition on pp.py
MOVE_STATE = 50 # Defines the move state and expect 1 parameter:
                #    0.0 => Hold pos, the unit doesn't move automatically
                #    1.0 => Maneuver, the unit stays near from its position
                #    2.0 => Roam, the unit is not blocked around its position.
                # See PP_Unit_UntargetedAction on pp.py

# Orders available for BIT, BYTE, KERNEL, LOGIC_BOMB, POINTER and SOCKET
ATTACK = 20 # The unit progresses to a target, attacks ennemy units in its line
            # of sight but doesn't stop its advance.
            # See PP_Unit_ActionOnUnit and PP_Unit_ActionOnPosition on pp.py

# Orders available for ASSEMBLER
REPAIR = 40 # The unit repairs a target.
            # See PP_Unit_ActionOnUnit and PP_Unit_ActionOnPosition on pp.py
BUILD_BADBLOCK = -3 # The unit builds a Bad Block on a target.
                    # See PP_Unit_ActionOnPosition on pp.py
BUILD_LOGIC_BOMB = -26 # The unit builds a Logic Bomb on a target.
                       # See PP_Unit_ActionOnPosition
BUILD_SOCKET = -45 # The unit builds a Socket on a target.
                   # See PP_Unit_ActionOnPosition on pp.py
BUILD_TERMINAL = -46 # The unit builds a Terminal on a target.
                     # See PP_Unit_ActionOnPosition on pp.py
DEBUG = -35 # The unit builds a Debug on a target (clear an area of all mines
            # and walls).
            # See PP_Unit_ActionOnPosition

# Orders available for KERNEL
BUILD_ASSEMBLER = -2 # The unit builds an Assembler.
                     # See PP_Unit_UntargetedAction and PP_Unit_ActionOnPosition
                     # on pp.py
BUILD_BYTE = -7 # The unit builds a Byte.
                # See PP_Unit_UntargetedAction and PP_Unit_ActionOnPosition on
                # pp.py
BUILD_POINTER = -39 # The unit builds a Pointer.
                    # See PP_Unit_UntargetedAction and PP_Unit_ActionOnPosition
                    # on pp.py

# Orders available for KERNEL and SOCKET
BUILD_BIT = -4 # The unit builds a Bit.
               # See PP_Unit_UntargetedAction and PP_Unit_ActionOnPosition on
               # pp.py
STOP_BUILDING = -7658 # The unit stops building units.
                      # See PP_Unit_UntargetedAction on pp.py

# Orders available for BYTE
LAUNCH_MINE = 33395 # The unit launches mines.
                    # See PP_Unit_UntargetedAction on pp.py

# Orders available for POINTER
NX_FLAG = 33389 # The unit launches an NX Flag.
                # See PP_Unit_ActionOnUnit and PP_Unit_ActionOnPosition on pp.py

# Orders available for TERMINAL
SIGTERM = 35126 # The unit launches a Sigterm.
                # See PP_Unit_ActionOnUnit and PP_Unit_ActionOnPosition on pp.py

##################################
# List of available resource ids #
##################################
METAL = 0 # Not used in Kernel Panic. See PP_GetResource on pp.py
ENERGY = 1 # Not used in Kernel Panic. See PP_GetResource on pp.py
