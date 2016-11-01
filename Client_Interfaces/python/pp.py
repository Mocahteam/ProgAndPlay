# File : pp.py
# Author : Muratet.M
# Date : January 3, 2015
#
# Brief : Interface to interact with the game
#

from ctypes import *
pplib = CDLL("./pp-client.dll")

###################################
# List of available coalition ids #
###################################

# Coalition id for units controlled by the player.
MY_COALITION = 0

# Coalition id for units controlled by ally players.
ALLY_COALITION = 1

# Coalition id for units controlled by enemy players.
ENEMY_COALITION = 2

####################
# Data definitions #
####################

# Represents a position.
# Defines a position representing a location (x, y) coordinate space.
class PP_Pos (Structure):
    _fields_ = [("x", c_float),
                ("y", c_float)]

# Maximum number of parameters of a command.
# PP_Cmd
MAX_PARAMS = 3

class PP_Cmd (Structure):
    _fields_ = [("code", c_int),
                ("nbParams", c_int),
                ("param", c_float * MAX_PARAMS)]

#################################
# Functions to manage Prog&Play #
#################################

def PP_Open ():
    """ -> int
    Opens Prog&Play API. This must be called before using other functions in this library.
    Returns 0 on success. -1 is returned on errors.

    """
    # call C function and return result
    return pplib.PP_Open()

def PP_Close ():
    """ -> int
    Shutdowns and cleans up Prog&Play API. After calling this Prog&Play functions should
    not be used. You may, of course, use PP_Open to use the functionality again.
    
    Returns 0 on success. -1 is returned on errors.

    """
    # call C function and return result
    return pplib.PP_Close()

#####################################
# Functions to manage game elements #
#####################################

def PP_IsGameOver ():
    """ -> int
    Checks if the game is over.
    
    Returns positive value if the game is ended and 0 if not. -1 is returned on errors.

    """
    # call C function and return result
    return pplib.PP_IsGameOver()

def PP_GetMapSize ():
    """ -> PP_Pos
    Map size is store into a PP_Pos as the max values available.

    Returns map size as a PP_Pos on success. A position containing -1.0 is returned on
            errors.

    """
    # specify return type of C function
    pplib.PP_GetMapSize.restype = PP_Pos
    # call C function and return result
    return pplib.PP_GetMapSize()

def PP_GetStartPosition ():
    """ -> PP_Pos
    The start position is the position where the player's army is located at the
    beginning of the game.

    Returns start position on success. A position containing -1.0 is returned on errors.

    """
    # specify return type of C function
    pplib.PP_GetStartPosition.restype = PP_Pos
    # call C function and return result
    return pplib.PP_GetStartPosition()

def PP_GetNumSpecialAreas ():
    """ -> int
    With Kernel Panic, a special area is a position where building is available.
    
    Returns the number of special areas on success. -1 is returned on errors.

    """
    # call C function and return result
    return pplib.PP_GetNumSpecialAreas ()

def PP_GetSpecialAreaPosition (specialAreaId):
    """ int -> PP_Pos
    With Kernel Panic, a special area is a position where building is available.

    Keyword arguments:
    specialAreaId -- an id of special area, it must be included meanwhile [0;n[ where n is
                     the number of special areas.
                     
    Returns the position of special area id specified on success. A position containing
            -1.0 is returned on errors.
    
    See PP_GetNumSpecialAreas

    """
    # specify arg types of C function
    pplib.PP_GetSpecialAreaPosition.argtypes = [c_int]
    # specify return type of C function
    pplib.PP_GetSpecialAreaPosition.restype = PP_Pos
    # call C function and return result
    return pplib.PP_GetSpecialAreaPosition(specialAreaId)

def PP_GetResource (resourceId):
    """ int -> int

    Keyword arguments:
    resourceId -- id of resource that you want know level.
    
    Returns the current level of specified resource. -1 is returned on errors.
    
    See constantList_KP4_1.py - available resource ids list

    """
    # specify arg types of C function
    pplib.PP_GetResource.argtypes = [c_int]
    # call C function and return result
    return pplib.PP_GetResource(resourceId)

def PP_GetNumUnits (coalition) :
    """ int -> int

    Keyword arguments:
    coalition -- coalition to consult.
    
    Returns the number of units (visible by the player) from this coalition. -1 is
            returned on errors.
    
    See available coalition ids list

    """
    # specify arg types of C function
    pplib.PP_GetNumUnits.argtypes = [c_int]
    # call C function and return result
    return pplib.PP_GetNumUnits(coalition)

def PP_GetUnitAt (coalition, index):
    """ int * int -> int

    Keyword arguments:
    coalition -- the coalition to consult.
    index -- id of a unit in the specified coalition, index must be included meanwhile
             [0;n[ where n is the number of units of this coalition.
             
    Returns unit (visible by the player) at the specified index on success. -1 is returned
            on errors.
            
    See PP_GetNumUnits
    See available coalition ids list

    """
    # specify arg types of C function
    pplib.PP_GetUnitAt.argtypes = [c_int, c_int]
    # call C function and return result
    return pplib.PP_GetUnitAt(coalition, index)

###########################
# Function to manage unit #
###########################

def PP_Unit_GetCoalition (unit):
    """ int -> int

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns coalition of the specified unit on success. -1 is returned on errors.
    
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_GetCoalition.argtypes = [c_int]
    # call C function and return result
    return pplib.PP_Unit_GetCoalition(unit)

def PP_Unit_GetType (unit):
    """ int -> int

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns type id of the specified unit on success. -1 is returned on errors.
    
    See constantList_KP4_1.py - available units type ids
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_GetType.argtypes = [c_int]
    # call C function and return result
    return pplib.PP_Unit_GetType(unit)

def PP_Unit_GetPosition (unit):
    """ int -> PP_Pos

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns position of the specified unit on success. A position containing -1.0 is
            returned on errors.
    
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_GetPosition.argtypes = [c_int]
    # specify return type of C function
    pplib.PP_Unit_GetPosition.restype = PP_Pos
    # call C function and return result
    return pplib.PP_Unit_GetPosition(unit)

def PP_Unit_GetHealth (unit):
    """ int -> float

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns health of the specified unit on success. -1.0 is returned on errors.
    
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_GetHealth.argtypes = [c_int]
    # specify return type of C function
    pplib.PP_Unit_GetHealth.restype = c_float
    # call C function and return result
    return pplib.PP_Unit_GetHealth(unit)

def PP_Unit_GetMaxHealth (unit):
    """ int -> float

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns maximum health of the specified unit can reach on success. -1.0 is returned on
            errors.
            
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_GetMaxHealth.argtypes = [c_int]
    # specify return type of C function
    pplib.PP_Unit_GetMaxHealth.restype = c_float
    # call C function and return result
    return pplib.PP_Unit_GetMaxHealth(unit)

def PP_Unit_GetGroup (unit):
    """ int -> int

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns the allocation group number of the specified unit on success. -2 is returned,
            if specified unit isn't associated to a group. -1 is returned on errors.
            
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_GetGroup.argtypes = [c_int]
    # call C function and return result
    return pplib.PP_Unit_GetGroup(unit)
    
def PP_Unit_SetGroup (unit, groupId):
    """ int * int -> int
    Allocates a unit to a specified group. Only units controled by the player can receive
    this command. WARNING: this function is not synchronous, result of this call is not
    available in sequence but after the next game update.

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt with coalition == MY_COALITION).
    groupId -- allocation group. group >= -1. If group == -1 then the specified unit is
               freed from its group.
               
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_SetGroup.argtypes = [c_int, c_int]
    # call C function and return result
    return pplib.PP_Unit_SetGroup(unit, groupId)

def PP_Unit_GetPendingCommands (unit):
    """int -> list[PP_Cmd]
    Reads the first 10 pending commands (if they are available) from a unit. Only units
    controled by the player can give this data.

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt with coalition == MY_COALITION).

    Returns a list of the first 10 commands in success. None is returned on errors

    See PP_GetUnitAt
    See PP_Cmd
    
    """
    
    # Maximum number of pending command available for a unit in the Prog&Play API.
    MAX_PDG_CMD = 10
    # C type wrapper to get pending commands
    class PP_PendingCommands (Structure):
        _fields_ = [("nbCmds", c_int),
                    ("cmd", PP_Cmd * MAX_PDG_CMD)]
    # create local pending commands
    c_cmds = PP_PendingCommands()
    # get pointer on this set
    p_cmds = pointer(c_cmds)

    # specify arg types of C function
    pplib.PP_Unit_GetPendingCommands.argtypes = [c_int, POINTER(PP_PendingCommands)]
    # call c function
    ret = pplib.PP_Unit_GetPendingCommands(unit, p_cmds)
    if ret == -1:
        return None
    # parse all commands and store only defined commands
    i = 0
    cmds = []
    while i < c_cmds.nbCmds:
        cmds.append(c_cmds.cmd[i])
        i += 1
    # returns list of available commands
    return cmds

def PP_Unit_ActionOnUnit (unit, action, target):
    """ int * int * int -> int
    Commands a unit to carry out action on a specified unit. Only units controled by the
    player can receive this command. WARNING: This function call is not blocking. When the
    function returns this means that the command is sent and not that the order is carried
    out. Carry out an action can take time, keep this in mind.

    Keyword arguments:
    unit -- unit id to command (returned by PP_GetUnitAt with coalition == MY_COALITION).
    action -- action id to carry out (See list form constantList_KP4_1.py of available
              order ids depending on unit ids).
    target -- target unit id (returned by PP_GetUnitAt).
    
    Returns 0 on success. -1 is returned on errors.
    
    See constantList_KP4_1.py - available unit actions depending on the unit type id.
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_ActionOnUnit.argtypes = [c_int, c_int, c_int]
    # call C function and return result
    return pplib.PP_Unit_ActionOnUnit(unit, action, target)
    
def PP_Unit_ActionOnPosition (unit, action, pos):
    """ int * int * PP_Pos -> int
    Commands a unit to carry out action on a specified position. Only units controled by
    the player can receive this command. WARNING: This function call is not blocking. When
    the function returns this means that the command is sent and not that the order is
    carried out. Carry out an action can take time, keep this in mind.
    
    Keyword arguments:
    unit -- unit id to command (returned by PP_GetUnitAt with coalition == MY_COALITION).
    action -- action id to carry out (See list form constantList_KP4_1.py of available
              order ids depending on unit ids).
    pos -- target position.

    Returns 0 on success. -1 is returned on errors.

    See constantList_KP4_1.py - available unit actions depending on the unit type id.
    See PP_GetUnitAt"""
    # specify arg types of C function
    pplib.PP_Unit_ActionOnPosition.argtypes = [c_int, c_int, PP_Pos]
    # call C function and return result
    return pplib.PP_Unit_ActionOnPosition(unit, action, pos)
    
def PP_Unit_UntargetedAction (unit, action, param=-1.0):
    """ int * int * float -> int
    Commands a unit to carry out an untargeted action. Only units controled by the player
    can receive this command. WARNING: This function call is not blocking. When the
    function returns this means that the command is sent and not that the order is carried
    out. Carry out an action can take time, keep this in mind.

    Keyword arguments:
    unit -- unit id to command (returned by PP_GetUnitAt with coalition == MY_COALITION).
    action -- action id to carry out (See list form constantList_KP4_1.py of available
              order ids depending on unit ids).
    param -- parameter to the action (if required). If any parameter required, put -1.0
    instead.
    
    Returns 0 on success. -1 is returned on errors.
    
    See constantList_KP4_1.py - available unit actions depending on the unit type id.
    See PP_GetUnitAt

    """
    # specify arg types of C function
    pplib.PP_Unit_UntargetedAction.argtypes = [c_int, c_int, c_float]
    # call C function and return result
    return pplib.PP_Unit_UntargetedAction(unit, action, param)

#####################################################
# Functions to manage errors generated by Prog&Play #
#####################################################

def PP_GetError ():
    """ -> string
    Returns last error generated by Prog&Play API.

    """
    pplib.PP_GetError.restype = c_char_p
    return pplib.PP_GetError().decode("utf-8")

def PP_ClearError ():
    """ ->
    Cleans the last error. Deletes all informations from the last internal error. Useful
    if error has been processed.

    """
    pplib.PP_ClearError()
