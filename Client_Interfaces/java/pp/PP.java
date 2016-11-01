package pp;

import pp.PPNative;
import pp.exceptions.*;

import java.awt.geom.Point2D;

/**
 * PP class enables to manage communication with the game and gives access to
 * the data of the game.
 *
 * @author Mathieu Muratet
 */
public class PP extends Thread{
	/**
	 * Define Coalition type. A coalition is a set of units.
	 */
	public enum Coalition {
		MY_COALITION(0),
		ALLY_COALITION(1),
		ENEMY_COALITION(2);
		
		/** "value" stores data for the enum */
		private final int value;
		
		/** Constructor combines data with the enum */
		private Coalition(int value) {
			this.value = value;
		}
		
		/**
		 * @return the enum as an integer.
		 */
		public int getValue() {
			return this.value;
		}
	}
	
	private boolean open;
	
	/**
	 * Constructor
	 */
	public PP(){
		open = false;
		/* Enable to execute "run" function when jvm stop */
		Runtime.getRuntime().addShutdownHook(this);
	}

	/**
	 * Close Prog&Play API when the jvm stop. This method makes sure of PP is
	 * correctly closed by the user and closes it if not.
	 *
	 * @see #close()
	 */
	public void run (){
		if (open) {
			try{
				close ();
			} catch (Exception e){
				System.out.println (e.getMessage());
			}
		}
	}
	
	/**
	 * Opens Prog&Play API. This must be called before using other functions in
	 * this library.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public void open() throws PPException {
		if (PPNative.Open() == -1)
			throw new PPException ("open -> "+PPNative.GetError());
		open = true;
	}
	/**
	 * Shutdown and cleanup Prog&Play API.
	 * After calling this Prog&Play functions should not be used. You may, of
	 * course, use PP_Open to use the functionality again.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public void close () throws PPException {
		if (PPNative.Close () == -1)
			throw new PPException ("close -> "+PPNative.GetError());
		open = false;
	}
	
	/**
	 * @return <code>true</code> if the game is ended and <code>false</code> if
	 * not.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public boolean isGameOver () throws PPException {
		switch (PPNative.IsGameOver()){
			case 0 : return false;
			case -1 : throw new PPException ("isGameOver -> "+PPNative.GetError());
			default : return true;
		}
	}
	
	/**
	 * @return map size on success.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 * @throws InstantiationException on allocation problem.
	 */
	public Point2D getMapSize () throws InstantiationException, PPException{
		float [] taille;
		taille = PPNative.GetMapSize();
		if (taille == null || taille.length != 2)
			throw new InstantiationException ("getMapSize -> memory allocation error");
		else if (taille[0] == -1 && taille[1] == -1)
			throw new PPException ("getMapSize -> "+PPNative.GetError());
		return new Point2D.Float(taille[0], taille[1]);
	}
	
	/**
	 * @return start position on success.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 * @throws InstantiationException on allocation problem.
	 */
	public Point2D getStartPosition () throws InstantiationException,
			PPException {
		float [] pos;
		pos = PPNative.GetStartPosition();
		if (pos == null || pos.length != 2)
			throw new InstantiationException ("getStartPosition -> memory allocation error");
		else if (pos[0] == -1 && pos[1] == -1)
			throw new PPException ("getStartPosition -> "+PPNative.GetError());
		return new Point2D.Float(pos[0], pos[1]);
	}
	
	/**
	 * @return the number of special areas on success.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public int getNumSpecialAreas() throws PPException {
		int ret = PPNative.GetNumSpecialAreas ();
		if (ret == -1)
			throw new PPException ("getNumSpecialAreas -> "+PPNative.GetError());
		return ret;
	}

	/**
	 * @param num id of a special area, it must be included meanwhile [0;n[ where
	 *            n is the number of special areas returned by
	 *            {@link #getNumSpecialAreas getNumSpecialAreas}.
	 * @return the position of this special area on success.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 * @throws InstantiationException on allocation problem.
	 * @see #getNumSpecialAreas
	 */
	public Point2D getSpecialAreasPosition (int z)
			throws InstantiationException, PPException{
		float [] position = PPNative.GetSpecialAreaPosition (z);
		if (position == null || position.length != 2)
			throw new InstantiationException ("getSpecialAreasPosition -> memory allocation error");
		else if (position[0] == -1 && position[1] == -1)
			throw new PPException ("getSpecialAreasPosition -> "+PPNative.GetError());
		return new Point2D.Float(position[0], position[1]);
	}
	
	/**
	 * @param id id of resource that you want know level.
	 * @return the current level of "id" resource.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public int getResource (int id) throws PPException{
		int ret = PPNative.GetResource (id);
		if (ret == -1)
			throw new PPException ("getResource -> "+PPNative.GetError());
		return ret;
	}
	
	/**
	 * Returns the number of units from the "c" coalition. Only visible units by
	 * the player are counted.
	 * 
	 * @param c coalition to consult.
	 * @return the number of units (visible by the player) from this coalition.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 */
	public int getNumUnits (Coalition c) throws PPException{
		int ret = PPNative.GetNumUnits (c.getValue());
		if (ret == -1)
			throw new PPException ("getNumUnits -> "+PPNative.GetError());
		return ret;
	}

	/**
	 * Returns the nth visible unit of the "c" coalition.
	 * 
	 * @param c coalition to consult.
	 * @param i id of a unit in the coalition "c", must be included meanwhile
	 *          [0;n[ where n is the number of units of this coalition (returned
	 *          by {@link #getNumUnits getNumUnits}).
	 * @return unit at the specified index on success.
	 *
	 * @throws PPException on errors linked with Prog&Play.
	 * @see #getNumUnits
	 */
	public Unit getUnitAt (Coalition c, int i) throws PPException{
		int ret = PPNative.GetUnitAt (c.getValue(), i);
		if (ret == -1)
			throw new PPException ("getUnitAt -> "+PPNative.GetError());
		return new Unit (ret);
	}
}
