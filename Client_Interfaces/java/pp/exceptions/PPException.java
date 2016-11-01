package pp.exceptions;

/**
 * Thrown by method in the Prog&Play class to indicate that something is bad.
 *
 * @author Mathieu Muratet
 */
public class PPException extends Exception {
	/** Constructs a new <code>PPException</code> with null as its detail message. */
	public PPException(){
		super();
	}
	/**
	 * Constructs a new <code>PPException</code> with the specified detail message.
	 *
	 * @param s  the detail message.
	 */
	public PPException(String s){
		super(s);
	}
}