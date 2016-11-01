package pp;

import pp.PPNative;
import pp.exceptions.*;

import java.util.ArrayList;

/**
 * PendingCommand class enables to access data of pending commands. To get an instance of a PendingCommand, use
 * {@link pp.Unit#getPendingCommands getPendingCommands} from Unit class.
 * 
 * @author Mathieu Muratet
 */
public class PendingCommand {
	/** PendingCommand id */
	private int code;
	private ArrayList<Float> parameters;

	/**
	 * Constructor. This constructor is accessible just into this package. To get
	 * an instance of a PendingCommand, use {@link pp.Unit#getPendingCommands getPendingCommands} from Unit class.
	 */
	PendingCommand (int code) {
		this.code = code;
		parameters = new ArrayList<Float>();
	}
	
	/**
	 * Add a parameter to this pending command
	 */
	void addParam(float p){
		parameters.add(new Float(p));
	}
	
	/**
	 * @return PendingCommand code.
	 */
	public int getCode () {
		return code;
	}
	
	/**
	 * @return PendingCommand parameters.
	 */
	public ArrayList<Float> getParams(){
		return parameters;
	}
}