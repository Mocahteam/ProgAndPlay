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
	 * @throws PPException on errors linked with Prog&Play.
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
	 * @throws PPException on errors linked with Prog&Play.
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
	 * @throws PPException on errors linked with Prog&Play.
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
	 * @throws PPException on errors linked with Prog&Play.
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
	 * @throws PPException on errors linked with Prog&Play.
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
	 * @throws PPException on errors linked with Prog&Play.
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
	 * @param g allocation group (<code>g >= 0</code>).
	 *
	 * @throws PPException on errors linked with Prog&Play.
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
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public void removeFromGroup () throws PPException{
		if (PPNative.Unit_SetGroup (id, -1) == -1)
			throw new PPException ("removeFromGroup -> "+PPNative.GetError());
	}

	/**
	 * Returns pending commands of this unit. Only units controled
	 * by the player can give this data.
	 * 
	 * @return the vector of pending commands of this unit.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public ArrayList<PendingCommand> getPendingCommands() throws PPException {
		PPNative.EnterCriticalSection();
			int size = PPNative.Unit_GetNumPdgCmds(this.id);
			if (size == -1)
				throw new PPException ("getPendingCommands -> "+PPNative.GetError());
			ArrayList<PendingCommand> pdgCmds = new ArrayList<PendingCommand>();
			for (int cmd = 0 ; cmd < size ; cmd++){
				int code = PPNative.Unit_PdgCmd_GetCode (this.id, cmd);
				if (code == -1)
					throw new PPException ("getPendingCommands -> "+PPNative.GetError());
				pdgCmds.add(new PendingCommand(code));
				for (int param = 0 ; param < PPNative.Unit_PdgCmd_GetNumParams(this.id, cmd) ; param++){
					float p = PPNative.Unit_PdgCmd_GetParam(this.id, cmd, param);
					if (p == -1.0)
						throw new PPException ("getPendingCommands -> "+PPNative.GetError());
					pdgCmds.get(cmd).addParam(p);
				}
			}
		PPNative.ExitCriticalSection();
		return pdgCmds;
	}

	/**
	 * Commands the unit to carry out action on a specified unit. Only units
	 * controled by the player can receive this command.
	 *
	 * @param action action to carry out.
	 * @param target target unit.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public void command (int action, Unit target) throws PPException{
		if (PPNative.Unit_ActionOnUnit (id, action, target.getId()) == -1)
			throw new PPException ("command -> "+PPNative.GetError());
	}

	/**
	 * Commands a unit to carry out action on a specified position. Only units
	 * controled by the player can receive this command.
	 *
	 * @param action action to carry out.
	 * @param target target position.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public void command (int action, Point2D target) throws PPException{
		if (PPNative.Unit_ActionOnPosition (id, action, (float)target.getX(), (float)target.getY()) == -1)
			throw new PPException ("command -> "+PPNative.GetError());
	}

	/**
	 * Commands a unit to carry out an untargeted action. Only units controled by
	 * the player can receive this command.
	 *
	 * @param action action to carry out.
	 * @param param parameter to the action. If any parameter required, put -1.0
	 *              instead.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public void command (int action, float param) throws PPException{
		if (PPNative.Unit_UntargetedAction (id, action, param) == -1)
			throw new PPException ("command -> "+PPNative.GetError());
	}
}