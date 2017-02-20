package pp;

import pp.PPNative;
import pp.exceptions.*;

import java.awt.geom.Point2D;
import java.util.ArrayList;

/**
 * Unit class enables to access data of units. To get an instance of a Unit, use
 * {@link pp.PP#getUnitAt getUnitAt} from PP class.
 * 
 * @author Mathieu Muratet
 */
public class Unit {
	/** Unit id */
	private int id;

	/**
	 * Constructor. This constructor is accessible just into this package. To get
	 * an instance of a Unit, use {@link pp.PP#getUnitAt getUnitAt} from PP class.
	 */
	Unit (int id) {
		this.id = id;
	}
	
	/**
	 * @return unit id.
	 */
	 public int getId () {
		 return id;
	 }
	
	/**
	 * @return coalition of this unit.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public PP.Coalition getCoalition () throws PPException{
		int ret	= PPNative.Unit_GetCoalition (id);
		switch (ret){
			case 2 : return PP.Coalition.ENEMY_COALITION;
			case 1 : return PP.Coalition.ALLY_COALITION;
			case 0 : return PP.Coalition.MY_COALITION;
			case -1 : throw new PPException ("getCoalition -> "+PPNative.GetError());
			default : throw new PPException ("Unknown coalition");
		}
	}

	/**
	 * @return type of this unit.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public int getType () throws PPException{
		int ret = PPNative.Unit_GetType (id);
		if (ret == -1)
			throw new PPException ("getType -> "+PPNative.GetError());
		return ret;
	}

	/**
	 * @return position of this unit.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 * @throws InstantiationException on allocation problem.
	 */
	public Point2D getPosition () throws PPException,
			InstantiationException{
		float [] position = PPNative.Unit_GetPosition (id);
		if (position == null || position.length != 2)
			throw new InstantiationException ("getPosition -> memory allocation error");
		else if (position[0] == -1 && position[1] == -1)
			throw new PPException (" -> "+PPNative.GetError());
		return new Point2D.Float(position[0], position[1]);
	}

	/**
	 * @return Health of this unit.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public float getHealth () throws PPException{
		float ret = PPNative.Unit_GetHealth (id);
		if (ret == -1)
			throw new PPException ("getHealth -> "+PPNative.GetError());
		return ret;
	}

	/**
	 * @return maximum health that this unit can reach.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public float getMaxHealth () throws PPException{
		float ret = PPNative.Unit_GetMaxHealth (id);
		if (ret == -1)
			throw new PPException ("getMaxHealth -> "+PPNative.GetError());
		return ret;
	}

	/**
	 * @return group number of this unit. a negative value is returned,
	 *         if this unit isn't associated to a group.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public int getGroup ()throws PPException{
		int ret = PPNative.Unit_GetGroup (id);
		if (ret == -1)
			throw new PPException ("getGroup -> "+PPNative.GetError());
		return ret; /* retourns -2 if unit isn't associated to a grou */
	}

	/**
	 * Allocates the unit to a specified group. Only units controled by the player
	 * can be affected to a group.
	 *
	 * @param g allocation group (g has to be upper or equal to 0).
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public void setGroup (int g) throws PPException{
		// this test is required because it is not tested in Unit_SetGroup(...)
		if (g < 0)
			throw new PPException ("setGroup -> invalid group, must be upper 0");
		else
			if (PPNative.Unit_SetGroup (id, g) == -1)
				throw new PPException ("setGroup -> "+PPNative.GetError());
	}

	/**
	 * Removes the unit from its group. Only units controled by the player
	 * can be removed from a group.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public void removeFromGroup () throws PPException{
		if (PPNative.Unit_RemoveFromGroup (id) == -1)
			throw new PPException ("removeFromGroup -> "+PPNative.GetError());
	}

	/**
	 * Returns pending commands of this unit. Only units controled
	 * by the player can give this data.
	 * 
	 * @return the vector of pending commands of this unit.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public ArrayList<PendingCommand> getPendingCommands() throws PPException {
		PPNative.EnterCriticalSection();
			int size = PPNative.Unit_GetNumPdgCmds_prim(this.id);
			int ret = size;
			ArrayList<PendingCommand> pdgCmds = new ArrayList<PendingCommand>();
			for (int cmd = 0 ; cmd < size && ret >= 0 ; cmd++){
				int [] cmdCode = PPNative.Unit_PdgCmd_GetCode_prim (this.id, cmd);
				ret = cmdCode[0];
				if (ret == 0){
					pdgCmds.add(new PendingCommand(cmdCode[1]));
					int nbParams = PPNative.Unit_PdgCmd_GetNumParams_prim(this.id, cmd);
					ret = nbParams;
					for (int param = 0 ; param < nbParams && ret >= 0 ; param++){
						float [] paramValue = PPNative.Unit_PdgCmd_GetParam_prim(this.id, cmd, param);
						ret = (int)paramValue[0];
						if (ret == 0)
							pdgCmds.get(cmd).addParam(paramValue[1]);
					}
				}
			}
			PPNative.pushMessageForGetPendingCommands(this.id, ret);
			if (ret < 0)
				throw new PPException ("getPendingCommands -> "+PPNative.GetError());
		PPNative.ExitCriticalSection();
		return pdgCmds;
	}

	/**
	 * Commands the unit to carry out action on a specified unit. Only units
	 * controled by the player can receive this command.
	 *
	 * @param action action to carry out.
	 * @param target target unit.
	 * @param locked: false means this function call is non blocking (when the
     *        function returns this means that the command is sent and not that
	 *        the order is carried out); true means this function call is
	 *        blocking until the order is carried out.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public void command (int action, Unit target, boolean locked) throws PPException{
		if (PPNative.Unit_ActionOnUnit (id, action, target.getId(), locked) == -1)
			throw new PPException ("command -> "+PPNative.GetError());
	}

	/**
	 * Commands a unit to carry out action on a specified position. Only units
	 * controled by the player can receive this command.
	 *
	 * @param action action to carry out.
	 * @param target target position.
	 * @param locked: false means this function call is non blocking (when the
     *        function returns this means that the command is sent and not that
	 *        the order is carried out); true means this function call is
	 *        blocking until the order is carried out.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public void command (int action, Point2D target, boolean locked) throws PPException{
		if (PPNative.Unit_ActionOnPosition (id, action, (float)target.getX(), (float)target.getY(), locked) == -1)
			throw new PPException ("command -> "+PPNative.GetError());
	}

	/**
	 * Commands a unit to carry out an untargeted action. Only units controled by
	 * the player can receive this command.
	 *
	 * @param action action to carry out.
	 * @param param parameter to the action. If any parameter required, put -1.0
	 *              instead.
	 * @param locked: false means this function call is non blocking (when the
     *        function returns this means that the command is sent and not that
	 *        the order is carried out); true means this function call is
	 *        blocking until the order is carried out.
	 *
	 * @throws PPException on errors linked with ProgAndPlay.
	 */
	public void command (int action, float param, boolean locked) throws PPException{
		if (PPNative.Unit_UntargetedAction (id, action, param, locked) == -1)
			throw new PPException ("command -> "+PPNative.GetError());
	}
}