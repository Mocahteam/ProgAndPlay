package pp;

public class PPNative {
	/*
	 * Opens Prog&Play API. This must be called before using other functions in 
	 * this library.
	 * Returns : 0 on success. -1 is returned on errors.
	 */
	public native static int Open();
	
	/*
	 * Shutdown and cleanup Prog&Play API.
	 * After calling this Prog&Play functions should not be used. You may, of
	 * course, use PP_Open to use the functionality again.
	 * Returns : 0 on success. -1 is returned on errors.
	 */
	public native static int Close ();
	
	/*
	 * Returns : positive value if the game is ended and 0 if not. -1 is returned
	 *           on errors.
	 */
	public native static int IsGameOver();
	
	/*
	 * Returns : map size on success. A position containing -1.0 is returned on
	 *           errors. NULL is returned on allocation error.
	 */
	public native static float [] GetMapSize ();
	
	/*
	 * Returns : start position on success. A position containing -1.0 is returned
	 *           on errors. NULL is returned on allocation error.
	 */
	public native static float [] GetStartPosition ();
	
	/*
	 * Returns : the number of special areas on success. -1 is returned on errors.
	 */
	public native static int GetNumSpecialAreas ();
	
	/*
	 * num : id of a special area, it must be included meanwhile [0;n[ where n is
	 *     the number of special areas.
	 * Returns : the position of this special area on success.  A position
	 *           containing -1.0 is returned on errors. NULL is returned on
	 *           allocation error.
	 */
	public native static float [] GetSpecialAreaPosition(int num);
	
	/*
	 * id : id of resource that you want know level.
	 * .
	 * Returns : the current level of "id" resource
	 *           -1 is returned on errors.
	 */
	public native static int GetResource(int id);
	
	/*
	 * Returns the number of units from the "c" coalition. Only visible units by
	 * the player are counted.
	 * c : coalition to consult.
	 * Returns : the number of units (visible by the player) from this coalition.
	 *           -1 is returned on errors.
	 */
	public native static int GetNumUnits (int c);
	
	/*
	 * Returns the nth visible unit of the "c" coalition.
	 * c : coalition to consult.
	 * num : id of a unit in the coalition "c", must be included meanwhile [0;n[
	 *     where n is the number of units of this coalition.
	 * Returns : unit at the specified index on success. -1 is returned en errors.
	 */
	public native static int GetUnitAt (int c, int num);
	
	/*
	 * unit : unit to consult.
	 * Returns : coalition of the specified unit on success. -1 is returned on
	 *           errors.
	 */
	public native static int Unit_GetCoalition (int unit);
	
	/*
	* unit : unit to consult.
	* Returns : type of the specified unit on success. -1 is returned on
	*           errors.
	*/
	public native static int Unit_GetType (int unit);
	
	/*
	 * unit : unit to consult.
	 * Returns : position of the specified unit on success. A position containing
	 *           -1.0 is returned on errors. NULL is returned on allocation error.
	 */
	public native static float [] Unit_GetPosition (int unit);
	
	/*
	 * unit : unit to consult.
	 * Returns : health of the specified unit on success. -1.0 is returned on
	 *           errors.
	 */
	public native static float Unit_GetHealth (int unit);
	
	/*
	 * unit : unit to consult.
	 * Returns : maximum health that the specified unit can reach on success. -1.0
	 *           is returned on errors.
	 */
	public native static float Unit_GetMaxHealth (int unit);
	
	/*
	 * unit : unit to consult.
	 * Returns : group number of the specified unit on success. -2 is returned, if
	 *           specified unit isn't associated to a group. -1 is returned on
	 *           errors.
	 */
	public native static int Unit_GetGroup (int unit);
	
	/*
	 * Allocates a unit to a specified group. Only units controled by the player
	 * can be affected to a group.
	 * unit : unit to command.
	 * group : allocation group. group >= -1. If group == -1 then the specified
	 *         unit is freed from this group.
	 * Returns : 0 on success. -1 is returned on errors
	 */
	public native static int Unit_SetGroup (int unit, int group);
	
	/*
	 * Returns the number of pending commands for a unit. Only units controled by
	 * the player can give this data.
	 * unit : unit to consult.
	 * Returns : the number of pending commands of this unit.
	 *           -1 is returned on errors.
	 */
	public native static int Unit_GetNumPdgCmds(int unit);

	/*
	 * Returns the nth pending command of a unit. Only units controled by the
	 * player can give this data.
	 * unit : unit to consult.
	 * index : id of a pending command of unit "unit", must be included meanwhile
	 *         [0;n[ where n is the number of pending commands of this unit.
	 * Returns : command at the specified index on success.
	 *           -1 is returned on errors.
	 */
	public native static int Unit_PdgCmd_GetCode(int unit, int index);

	/*
	 * Returns the number of parameters of the nth pending command code of a unit.
	 * Only units controled by the player can give this data.
	 * unit : unit to consult.
	 * idCmd : id of the pending command of unit "unit", must be included meanwhile
	 *         [0;n[ where n is the number of pending commands of this unit.
	 * Returns : number of parameters of command at the specified index on success.
	 *           -1 is returned on errors.
	 */
	public native static int Unit_PdgCmd_GetNumParams(int unit, int idCmd);

	/*
	 * Returns the nth parmater of a pending command code of a unit. Only units
	 * controled by the player can give this data.
	 * unit : unit to consult.
	 * idCmd : id of the pending command of unit "unit", must be included meanwhile
	 *         [0;n[ where n is the number of pending commands of this unit.
	 * idParam : id of the parameter of the command identified by "idCmd", must be
	 *           included meanwhile [0;n[ where n is the number of parameters for 
	 *           this pending command of this unit.
	 * Returns : parameter at the specified index on success.
	 *           -1 is returned on errors.
	 */
	public native static float Unit_PdgCmd_GetParam(int unit, int idCmd, int idParam);

	/*
	 * Commands a unit to carry out action on a specified unit. Only units
	 * controled by the player can receive this command.
	 * unit : unit to command.
	 * action : action to carry out.
	 * target : target unit.
	 * Retourne : 0 on success. -1 is returned on errors.
	 */
	public native static int Unit_ActionOnUnit (int unit, int action, int target);
	
	/*
	 * Commands a unit to carry out action on a specified position. Only units
	 * controled by the player can receive this command.
	 * unit : unit to command.
	 * action : action to carry out.
	 * target : target position.
	 * Retourne : 0 on success. -1 is returned on errors.
	 */
	public native static int Unit_ActionOnPosition (int unit, int action, float x,
		float y);
	
	/*
	 * Commands a unit to carry out an untargeted action. Only units controled by
	 * the player can receive this command.
	 *
	 * unit : unit to command.
	 * action : action to carry out.
	 * param : parameter to the action).
	 *
	 * Retourne : 0 on success. -1 is returned on errors.
	 */
	public native static int Unit_UntargetedAction(int unit, int action,
		float param);
	
	/*
	 * Returns the last error set as a string. NULL is returned on allocation
	 * error.
	 */
	public native static String GetError();
	
	/*
	 * Deletes all informations from the last internal error. Useful if error has
	 * been processed.
	 */
	public native static void ClearError();
	
	/*
	 * This function has to be called before entering in critical section.
	 * Don't forget to call "ExitCriticalSection()" after critical section.
	 */
	public native static void EnterCriticalSection();
	
	/*
	 * This function has to be called at the end of a critical section.
	 */
	public native static void ExitCriticalSection();
	
	static {System.loadLibrary("pp_java");}
}
