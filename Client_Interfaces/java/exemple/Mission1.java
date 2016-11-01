package exemple;

import exemple.ConstantList_KP_4_1;

import pp.PP;
import pp.Unit;
import java.awt.geom.Point2D;

public class Mission1 {
	public static void main (String[] args){
		// definition de la coordonnee ciblee a atteindre
		Point2D.Float p = new Point2D.Float (1983, 1279);
		try {
			// creer une interface avec le jeu
			PP pp = new PP();
			// ouverture du jeu
			pp.open();
			// recuperation de l'unite courante
			Unit u = pp.getUnitAt(PP.Coalition.MY_COALITION, 0);
			// ordonner a cette unite de se deplacer sur la cible
			u.command(ConstantList_KP_4_1.MOVE, p);
			// fermer le jeu
			pp.close();
		} catch (Exception e){
			System.out.println (e.getMessage());
		}
	}
}
