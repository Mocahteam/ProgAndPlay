package exemple;

import exemple.ConstantList_KP_4_1;

import pp.PP;
import pp.Unit;
import pp.exceptions.*;

import java.awt.geom.Point2D;

public class Exemple {
	public static void main (String[] args){
		// définition de la coordonnée cible à atteindre
		Point2D.Float p = new Point2D.Float (100, 100);
		try {
			// créer une interface avec le jeu
			PP pp = new PP();
			// ouverture du jeu
			pp.open();
			// parcours de toutes mes unités
			for (int i = 0 ; i < pp.getNumUnits (PP.Coalition.MY_COALITION) ; i++){
				// récupération de l'unité courante
				Unit u = pp.getUnitAt(PP.Coalition.MY_COALITION, i);
				// ordonner à cette unité de se déplacer sur la cible
				u.command(ConstantList_KP_4_1.MOVE, p);
			}
			// fermer le jeu
			pp.close();
		} catch (Exception e){
			System.out.println (e.getMessage());
		}
	}
}
