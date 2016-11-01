(* Prog&Play interface. *)
(* This module provides functionalities to use Prog&Play system. *)

(* pos type definition (x-axis, y-axis). *)
type pos = float * float
(* entity type definition. This is an abstract type. *)
type entity = int
(* coalition type definition. MyCoalition represents all entities of the player. EnemyCoalition represents all ennemies' entities of the player. AllyCoalition represents all allies' entities of the player.*)
type coalition = MyCoalition | AllyCoalition | EnemyCoalition
(* command type definition. *)
type command = int
(* resource type definition. *)
type resource = int
(* kind type definition. It represents all kind of entities available for a game. *)
type kind = command
(* action type definition. It represents all actions available for the differents kind of entities. *)
type action = command
(* pending command definition. It is represented by an action and a list of parameters *)
type pdgCmd = (action * float list)

(* Returns true if the game is over and false if not. *)
external isGameOver : unit -> bool = "OCaml_IsGameOver"

(* Return the map size of the game as a position (lower right corner). *)
external getMapSize : unit -> pos = "OCaml_GetMapSize"

(* Return the start position of the player. *)
external getStartPosition : unit -> pos = "OCaml_GetStartPosition"

(* Return a list with the position of all special areas. *)
external getSpecialAreas : unit -> pos list = "OCaml_GetSpecialAreas"

(* Return a current level of a specific resource. *)
external getResource : resource -> int = "OCaml_GetResource"

(* Synchronize the game's state and return a list of all entities from the accurate coalition. *)
external getEntities : coalition -> entity list = "OCaml_GetEntities"

(* "getCoalition e" Return the coalition of entity e. *)
external getCoalition : entity -> coalition = "OCaml_Unit_GetCoalition"

(* "getType e" Return the kind of entity of e. *)
external getType : entity -> kind = "OCaml_Unit_GetType"

(* "getPosition e" Return the position of e. *)
external getPosition : entity -> pos = "OCaml_Unit_GetPosition"

(* "getHealth e" Return the health of e. *)
external getHealth : entity -> float = "OCaml_Unit_GetHealth"

(* "getMaxHealth e" Return the maximum health of e. *)
external getMaxHealth : entity -> float = "OCaml_Unit_GetMaxHealth"

(* "getGroup e" Return the group of e. *)
external getGroup : entity -> int = "OCaml_Unit_GetGroup"

(* "setGroup (e, g)" command e to integrate the group g. Return true if the command is set up and false otherwise. *)
external setGroup : entity * int -> bool = "OCaml_Unit_SetGroup"

(* "getPendingCommands e" returns a list of pending commands of e. *)
external getPendingCommands : entity -> pdgCmd list = "OCaml_Unit_GetPendingCommands"

(* "actionOnEntity (e, a, t)" command e to carry out action a on the target t. Return true if the command is set up and false otherwise. *)
external actionOnEntity : entity * command * entity -> bool = "OCaml_Unit_ActionOnUnit"

(* "actionOnPosition (e, a, p)" command e to carry out action a at the position p. Return true if the command is set up and false otherwise. *)
external actionOnPosition : entity * command * pos -> bool = "OCaml_Unit_ActionOnPosition"


(* "untargetedAction (e, a, p)" command e to carry out action a with the parameter p. Return true if the command is set up and false otherwise. *)
external untargetedAction : entity * command * float -> bool = "OCaml_Unit_UntargetedAction"

(* "waitUnitProp u p" returns true when (p u) = true and waits otherwise. Returns false if a problem occurs during waiting. *)
let rec waitUnitProp = function (u, p) ->
	let rec alwaysPresent = function
		(e, []) -> false;
		| (e, t::q) -> e = t or alwaysPresent (e, q) in
	if not (p u) then
		if alwaysPresent (u, (getEntities (getCoalition u))) then
			waitUnitProp (u, p)
		else
			false
	else
		true;;