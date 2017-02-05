with Interfaces.C.Strings;
with Ada.Containers.Vectors;     use Ada.Containers;

package PP is
	-- Define Coalition type. A coalition is a set of units
	type Coalition is
		(MyCoalition,    -- Represents my coalition
		AllyCoalition,   -- représente les alliés du joueur
		EnemyCoalition); -- représente les ennemis du joueur
	
	-- Define CommandCode type 
	subtype CommandCode is Integer;
	-- Define Kind type
	subtype Kind is CommandCode;
	-- Define Action type
	subtype Action is CommandCode;
	-- Define Resource type
	subtype Resource is Integer;
	
	-- Define Position type
	type Position is
		record
			x : Float;
			y : Float;
		end record;
	
	-- Define Command type
	package Float_Container is new Vectors (Natural, Float);
	type Command is
		record
			code : Integer;
			param : Float_Container.Vector;
		end record;
	package Cmd_Container is new Vectors (Natural, Command);
	
	-- Define Unit type
	subtype Unit is Integer;
	
	------------------------------------------------------------------------------
	-- Procedure to manage Prog&Play                                            --
	------------------------------------------------------------------------------
	-- Opens Prog&Play API. This must be called before using other functions in
	-- this library.
	procedure Open;
	-- Shutdown and cleanup Prog&Play API.
	-- After calling this, Prog&Play functions should not be used. You may, of
	-- course, use PP_Open to use the functionality again.
	procedure Close;
	
	------------------------------------------------------------------------------
	-- Functions to manage game elements                                        --
	------------------------------------------------------------------------------
	-- Returns: True if the game is ended and False if not
	function IsGameOver return Boolean;
	-- Returns: True if the game is paused and False if not
	function IsGamePaused return Boolean;
	-- Returns: map size on success. A position containing -1.0 is returned on
	--           errors.
	function GetMapSize return Position;
	-- Returns: start position on success. A position containing -1.0 is returned
	--           on errors.
	function GetStartPosition return Position;
	-- Returns: the number of special areas on success. -1 is returned on errors.
	function GetNumSpecialAreas return Integer;
	-- num: id of a special area, it must be included meanwhile [0;n[ where n is
	--      the number of special areas.
	-- Returns: the position of this special area on success.  A position
	--          containing -1.0 is returned on errors.
	function GetSpecialAreaPosition (num : in Integer) return Position;
	-- id: id of resource that you want know level.
	-- Returns: the current level of "id" resource. -1 is returned on errors.
	function GetResource (id : in Resource) return Integer;
	-- Returns the number of units from the "c" coalition. Only visible units by
	-- the player are counted.
	-- c: coalition to consult.
	-- Returns: the number of units (visible by the player) from this coalition.
	--          -1 is returned on errors.
	function GetNumUnits (c : in Coalition) return Integer;
	-- Returns the nth visible unit of the "c" coalition.
	-- c: coalition to consult.
	-- index: id of a unit in the coalition "c", must be included meanwhile [0;n[
	--        where n is the number of units of this coalition.
	-- Returns: unit at the specified index on success. -1 is returned en errors.
	function GetUnitAt (c : in Coalition; index : in Integer) return Unit;
	
	------------------------------------------------------------------------------
	-- Functions and procedures to manage units                                 --
	------------------------------------------------------------------------------
	-- u: unit to consult.
	-- Returns: coalition of the specified unit on success. -1 is returned on
	--           errors.
	function GetCoalition (u : in Unit) return Coalition;
	-- u: unit to consult.
	-- Returns: type of the specified unit on success. -1 is returned on errors.
	function GetType (u : in Unit) return Kind;
	-- u: unit to consult.
	-- Returns: position of the specified unit on success. A position containing
	--          -1.0 is returned on errors.
	function GetPosition (u : in Unit) return Position;
	-- u: unit to consult.
	-- Returns: health of the specified unit on success. -1.0 is returned on
	--          errors.
	function GetHealth (u : in Unit) return Float;
	-- u: unit to consult.
	-- Returns: maximum health that the specified unit can reach on success.
	--         -1.0 is returned on errors.
	function GetMaxHealth (u : in Unit) return Float;
	-- u: unit to consult.
	-- Returns: group number of the specified unit on success.
	--          -2 is returned, if specified unit isn't associated to a group.
	--          -1 is returned on errors.
	function GetGroup (u : in Unit) return Integer;
	-- Allocates a unit to a specified group. Only units controled by the player
	-- can be affected to a group.
	-- u: unit to command.
	-- group: allocation group. group >= -1. If group == -1 then the specified
	--        unit is freed from this group.
	procedure SetGroup (u : in Unit; group : in Integer);
	-- Reads pending commands from a unit. Only units controled by
	-- the player can give this data.
	-- u: unit to consult
	-- Returns: a vector that will be filled by unit's pending commands.
	function GetPendingCommands(u : in Unit) return Cmd_Container.Vector;
	
	-- Commands a unit to carry out action on a specified unit. Only units
	-- controled by the player can receive this command.
	-- u: unit to command.
	-- c: command code to send.
	-- t: target unit.
	-- s: true means this function is synchronized this call is blocking until
	--    the order is over, false means this function is non-blocking (when the
	--    function returns this means that the command is sent and not that the
	--    order is carried out).
	procedure CarryOutCommand (u : in Unit; c : in CommandCode; t : in Unit; s : in Boolean := False);
	-- Commands a unit to carry out action on a specified position. Only units
	-- controled by the player can receive this command.
	-- u: unit to command.
	-- c: command code to send.
	-- t: target position.
	-- s: true means this function is synchronized this call is blocking until
	--    the order is over, false means this function is non-blocking (when the
	--    function returns this means that the command is sent and not that the
	--    order is carried out).
	procedure CarryOutCommand (u : in Unit; c : in CommandCode; t : in Position; s : in Boolean := False);
	-- Commands a unit to carry out an untargeted action. Only units controled by
	-- the player can receive this command.
	-- u: unit to command.
	-- c: command code to send.
	-- p: parameter of the command. If any parameter required, put -1.0 instead.
	-- s: true means this function is synchronized this call is blocking until
	--    the order is over, false means this function is non-blocking (when the
	--    function returns this means that the command is sent and not that the
	--    order is carried out).
	procedure CarryOutCommand (u : in Unit; c : in CommandCode; p : in Float := -1.0; s : in Boolean := False);
	
	-- Print the last error generated by Prog&Play API.
	procedure PrintError;
	
private
	pragma import (C, Open, "Ada_Open");
	pragma import (C, Close, "PP_Close");
	
	pragma import (C, IsGameOver, "PP_IsGameOver");
	pragma import (C, IsGamePaused, "PP_IsGamePaused");
	pragma import (C, GetMapSize, "PP_GetMapSize");
	pragma import (C, GetStartPosition, "PP_GetStartPosition");
	pragma import (C, GetNumSpecialAreas, "PP_GetNumSpecialAreas");
	pragma import (C, GetSpecialAreaPosition, "PP_GetSpecialAreaPosition");
	pragma import (C, GetResource, "PP_GetResource");
	pragma import (C, GetNumUnits, "PP_GetNumUnits");
	pragma import (C, GetUnitAt, "PP_GetUnitAt");
	
	pragma import (C, GetCoalition, "PP_Unit_GetCoalition");
	pragma import (C, GetType, "PP_Unit_GetType");
	pragma import (C, GetPosition, "PP_Unit_GetPosition");
	pragma import (C, GetHealth, "PP_Unit_GetHealth");
	pragma import (C, GetMaxHealth, "PP_Unit_GetMaxHealth");
	pragma import (C, GetGroup, "PP_Unit_GetGroup");
	pragma import (C, SetGroup, "PP_Unit_SetGroup");
	
	procedure PP_Unit_ActionOnUnit (u : in Unit; a : in CommandCode; t : in Unit; s : in Boolean);
	pragma import (C, PP_Unit_ActionOnUnit, "PP_Unit_ActionOnUnit");
	procedure Ada_ActionOnPosition (u : in Unit; a : in CommandCode; x : in Float; y : in Float; s : in Boolean);
	pragma import (C, Ada_ActionOnPosition, "Ada_ActionOnPosition");
	procedure PP_Unit_UntargetedAction (u : in Unit; a : in CommandCode; param : in Float; s : in Boolean);
	pragma import (C, PP_Unit_UntargetedAction, "PP_Unit_UntargetedAction");
	
	procedure enterCriticalSection;
	pragma import (C, enterCriticalSection, "enterCriticalSection");
	procedure exitCriticalSection;
	pragma import (C, exitCriticalSection, "exitCriticalSection");
	function PP_Unit_GetNumPdgCmds_prim (u : in Unit) return Integer;
	pragma import (C, PP_Unit_GetNumPdgCmds_prim, "PP_Unit_GetNumPdgCmds_prim");
	function PP_Unit_PdgCmd_GetCode_prim(u : in Unit; idCmd : in Integer) return Integer;
	pragma import (C, PP_Unit_PdgCmd_GetCode_prim, "PP_Unit_PdgCmd_GetCode_prim");
	function PP_Unit_PdgCmd_GetNumParams_prim(u : in Unit; idCmd : in Integer) return Integer;
	pragma import (C, PP_Unit_PdgCmd_GetNumParams_prim, "PP_Unit_PdgCmd_GetNumParams_prim");
	function PP_Unit_PdgCmd_GetParam_prim(u : in Unit; idCmd : in Integer; idParam : in Integer) return Float;
	pragma import (C, PP_Unit_PdgCmd_GetParam_prim, "PP_Unit_PdgCmd_GetParam_prim");
	
	function PP_PushMessage_prim(msg : in Interfaces.C.Strings.chars_ptr; error : access Interfaces.C.int) return Integer;
	pragma import (C, PP_PushMessage_prim, "PP_PushMessage_prim");
	
	function PP_GetError return Interfaces.C.Strings.chars_ptr;
	pragma import (C, PP_GetError, "PP_GetError");
	procedure PP_ClearError;
	pragma import (C, PP_ClearError, "PP_ClearError");
end PP;