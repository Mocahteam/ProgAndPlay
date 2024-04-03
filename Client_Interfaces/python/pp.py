# File : pp.py
# Author : Muratet.M
# Date : March 20, 2024
#
# Brief : Interface to connect with 32 bits library from Python x64
#

import socket
import subprocess
import json

pp_wrapper = None
pp_client = None
pp_socket = None

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
class PP_Pos:
    def __init__(self, newX=0, newY=0):
        self.x = newX
        self.y = newY

# Maximum number of parameters of a command.
# PP_Cmd
MAX_PARAMS = 3

class PP_Cmd:
    def __init__(self, code=0, params=[]):
        self.code = code
        self.params = params

#################################
# Functions to manage Prog&Play #
#################################

def PP_Open ():
    """ -> int
    Opens Prog&Play API. This must be called before using other functions in this library.
    Returns 0 on success. -1 is returned on errors.

    """
    global pp_socket
    global pp_client
    global pp_wrapper
    pp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    pp_socket.bind(('127.0.0.1', 12012))
    pp_wrapper = subprocess.Popen(["32bitWrapper.exe"])
    pp_socket.listen()
    pp_client, _address = pp_socket.accept()
    # send request
    request = "PP_Open"
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

def PP_Close ():
    """ -> int
    Shutdowns and cleans up Prog&Play API. After calling this Prog&Play functions should
    not be used. You may, of course, use PP_Open to use the functionality again.
    
    Returns 0 on success. -1 is returned on errors.

    """
    # send request
    global pp_socket
    global pp_client
    global pp_wrapper
    request = "PP_Close"
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    pp_client.close()
    pp_socket.close()
    pp_wrapper.kill()
    return int(response)

#####################################
# Functions to manage game elements #
#####################################

def PP_IsGameOver ():
    """ -> int
    Checks if the game is over.
    
    Returns positive value if the game is ended and 0 if not. -1 is returned on errors.

    """
    # call C function and return result
    global pp_client
    # send request
    request = "PP_IsGameOver"
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

def PP_IsGamePaused ():
    """ -> int
    Checks if the game is paused.
    
    Returns positive value if the game is paused and 0 if not. -1 is returned on errors.

    """
    # call C function and return result
    global pp_client
    # send request
    request = "PP_IsGamePaused"
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

def PP_GetMapSize ():
    """ -> PP_Pos
    Map size is store into a PP_Pos as the max values available.

    Returns map size as a PP_Pos on success. A position containing -1.0 is returned on
            errors.

    """
    global pp_client
    # send request
    request = "PP_GetMapSize"
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    responses = msg.decode().split("#")
    pos = PP_Pos(float(responses[0]), float(responses[1]))
    return pos

def PP_GetStartPosition ():
    """ -> PP_Pos
    The start position is the position where the player's army is located at the
    beginning of the game.

    Returns start position on success. A position containing -1.0 is returned on errors.

    """
    global pp_client
    # send request
    request = "PP_GetStartPosition"
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    responses = msg.decode().split("#")
    pos = PP_Pos(float(responses[0]), float(responses[1]))
    return pos

def PP_GetNumSpecialAreas ():
    """ -> int
    With Kernel Panic, a special area is a position where building is available.
    
    Returns the number of special areas on success. -1 is returned on errors.

    """
    # call C function and return result
    global pp_client
    # send request
    request = "PP_GetNumSpecialAreas"
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

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
    global pp_client
    # send request
    request = "PP_GetSpecialAreaPosition#"+str(specialAreaId)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    responses = msg.decode().split("#")
    pos = PP_Pos(float(responses[0]), float(responses[1]))
    return pos

def PP_GetResource (resourceId):
    """ int -> int

    Keyword arguments:
    resourceId -- id of resource that you want know level.
    
    Returns the current level of specified resource. -1 is returned on errors.
    
    See constantList_KP4_1.py - available resource ids list

    """
    global pp_client
    # send request
    request = "PP_GetResource#"+str(resourceId)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

def PP_GetNumUnits (coalition) :
    """ int -> int

    Keyword arguments:
    coalition -- coalition to consult.
    
    Returns the number of units (visible by the player) from this coalition. -1 is
            returned on errors.
    
    See available coalition ids list

    """
    global pp_client
    # send request
    request = "PP_GetNumUnits#"+str(coalition)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

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
    global pp_client
    # send request
    request = "PP_GetUnitAt#"+str(coalition)+"#"+str(index)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

# ###########################
# # Function to manage unit #
# ###########################

def PP_Unit_GetCoalition (unit):
    """ int -> int

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns coalition of the specified unit on success. -1 is returned on errors.
    
    See PP_GetUnitAt

    """
    global pp_client
    # send request
    request = "PP_Unit_GetCoalition#"+str(unit)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

def PP_Unit_GetType (unit):
    """ int -> int

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns type id of the specified unit on success. -1 is returned on errors.
    
    See constantList_KP4_1.py - available units type ids
    See PP_GetUnitAt

    """
    global pp_client
    # send request
    request = "PP_Unit_GetType#"+str(unit)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

def PP_Unit_GetPosition (unit):
    """ int -> PP_Pos

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns position of the specified unit on success. A position containing -1.0 is
            returned on errors.
    
    See PP_GetUnitAt

    """
    global pp_client
    # send request
    request = "PP_Unit_GetPosition#"+str(unit)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    responses = msg.decode().split("#")
    pos = PP_Pos(float(responses[0]), float(responses[1]))
    return pos

def PP_Unit_GetHealth (unit):
    """ int -> float

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns health of the specified unit on success. -1.0 is returned on errors.
    
    See PP_GetUnitAt

    """
    global pp_client
    # send request
    request = "PP_Unit_GetHealth#"+str(unit)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return float(response)

def PP_Unit_GetMaxHealth (unit):
    """ int -> float

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns maximum health of the specified unit can reach on success. -1.0 is returned on
            errors.
            
    See PP_GetUnitAt

    """
    global pp_client
    # send request
    request = "PP_Unit_GetMaxHealth#"+str(unit)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return float(response)

def PP_Unit_GetGroup (unit):
    """ int -> int

    Keyword arguments:
    unit -- unit id (returned by PP_GetUnitAt).
    
    Returns the allocation group number of the specified unit on success. -2 is returned,
            if specified unit isn't associated to a group. -1 is returned on errors.
            
    See PP_GetUnitAt

    """
    global pp_client
    # send request
    request = "PP_Unit_GetGroup#"+str(unit)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)
    
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
    global pp_client
    # send request
    request = "PP_Unit_SetGroup#"+str(unit)+"#"+str(groupId)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

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
    global pp_client
    # send request
    request = "PP_Unit_GetPendingCommands#"+str(unit)
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    if response == "-1":
        return None

    result = json.loads(response)

    pendingCmds = []
    for r in result:
        pendingCmds.append(PP_Cmd(r["code"], r["params"]))

    return pendingCmds

def PP_Unit_ActionOnUnit (unit, action, target, synchronized=False):
    """ int * int * int * bool -> int
    Commands a unit to carry out action on a specified unit. Only units controled by the
    player can receive this command.

    Keyword arguments:
    unit -- unit id to command (returned by PP_GetUnitAt with coalition == MY_COALITION).
    action -- action id to carry out (See list form constantList_KP4_1.py of available
              order ids depending on unit ids).
    target -- target unit id (returned by PP_GetUnitAt).
    synchronized -- true means this function is synchronized this call is blocking until
                    the order is over, false means this function is non-blocking (when the
                    function returns this means that the command is sent and not that the
                    order is carried out).
    
    Returns 0 on success. -1 is returned on errors.
    
    See constantList_KP4_1.py - available unit actions depending on the unit type id.
    See PP_GetUnitAt

    """
    global pp_client
    # send request
    request = "PP_Unit_ActionOnUnit#"+str(unit)+"#"+str(action)+"#"+str(target)+"#"+("1" if synchronized else "0")
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)    
    
def PP_Unit_ActionOnPosition (unit, action, pos, synchronized=False):
    """ int * int * PP_Pos * bool -> int
    Commands a unit to carry out action on a specified position. Only units controled by
    the player can receive this command.
    
    Keyword arguments:
    unit -- unit id to command (returned by PP_GetUnitAt with coalition == MY_COALITION).
    action -- action id to carry out (See list form constantList_KP4_1.py of available
              order ids depending on unit ids).
    pos -- target position.
    synchronized -- true means this function is synchronized this call is blocking until
                    the order is over, false means this function is non-blocking (when the
                    function returns this means that the command is sent and not that the
                    order is carried out).

    Returns 0 on success. -1 is returned on errors.

    See constantList_KP4_1.py - available unit actions depending on the unit type id.
    See PP_GetUnitAt"""
    global pp_client
    # send request
    request = "PP_Unit_ActionOnPosition#"+str(unit)+"#"+str(action)+"#"+str(pos.x)+"#"+str(pos.y)+"#"+("1" if synchronized else "0")
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)
    
def PP_Unit_UntargetedAction (unit, action, param=-1.0, synchronized=False):
    """ int * int * float * bool -> int
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
    synchronized -- true means this function is synchronized this call is blocking until
                    the order is over, false means this function is non-blocking (when the
                    function returns this means that the command is sent and not that the
                    order is carried out).
    
    Returns 0 on success. -1 is returned on errors.
    
    See constantList_KP4_1.py - available unit actions depending on the unit type id.
    See PP_GetUnitAt

    """
    global pp_client
    # send request
    request = "PP_Unit_UntargetedAction#"+str(unit)+"#"+str(action)+"#"+str(param)+"#"+("1" if synchronized else "0")
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    response = msg.decode()
    return int(response)

# #####################################################
# # Functions to manage errors generated by Prog&Play #
# #####################################################

def PP_GetError ():
    """ -> string
    Returns last error generated by Prog&Play API.

    """
    global pp_client
    # send request
    request = "PP_GetError"
    pp_client.send(request.encode())
    # get answer
    msg = pp_client.recv(2048)
    return msg.decode()

def PP_ClearError ():
    """ ->
    Cleans the last error. Deletes all informations from the last internal error. Useful
    if error has been processed.

    """
    global pp_client
    # send request
    request = "PP_ClearError"
    pp_client.send(request.encode())
    # get answer
    pp_client.recv(2048)