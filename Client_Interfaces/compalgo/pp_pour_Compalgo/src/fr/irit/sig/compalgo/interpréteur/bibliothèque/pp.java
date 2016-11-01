package fr.irit.sig.compalgo.interpréteur.bibliothèque;

import fr.irit.sig.compalgo.interpréteur.runtime.wrap.InEntier;
import fr.irit.sig.compalgo.interpréteur.runtime.wrap.InRéel;
import fr.irit.sig.compalgo.interpréteur.runtime.wrap.OutRéel;

import pp.PPNative;

public final class pp extends Thread{

	private boolean ouvert;
	
	public pp(){
		ouvert = false;
		/* permet d'exécuter la fonction run lors le l'arret de la jvm */
		Runtime.getRuntime().addShutdownHook(this);
	}

	/*
	 * fonction executer lors de l'arret de la jvm. Permet de bien
	 * fermer le jeu au cas où l'utilisateur a oublié d'appeler la
	 * fonction fermer()
	 */
	public void run (){
		if (ouvert) {
			close ();
		}
	}
	
	/*
	 * Opération de gestion du jeu
	 */
	/* initialise le jeu. */
	public Integer open(){
		int ret = PPNative.Open();
		if (ret == 0){
			ouvert = true;
		}
		return new Integer (ret);
	}
	/* ferme le jeu. */
	public Integer close (){
		int ret = PPNative.Close ();
		if (ret == 0)
			ouvert = false;
		return new Integer (ret);
	}
	
	/*
	 * Opérations générales du jeu
	 */
	
	/* Indique si le jeu est terminé */
	public Integer isGameOver (){
		return new Integer (PPNative.IsGameOver());
	}
	
	/* fournit la dimension de la carte de jeu. */
	public void getMapSize (OutRéel x, OutRéel y){
		float [] taille;
		taille = PPNative.GetMapSize();
		if (taille == null || taille.length != 2){
			x.assigne((float)-20);
			y.assigne((float)-20);
		}
		else {
			x.assigne(taille[0]);
			y.assigne(taille[1]);
		}
	}
	/* fournit la dimention de la carte de jeu. */
	public void getStartPosition (OutRéel x, OutRéel y){
		float [] taille;
		taille = PPNative.GetStartPosition();
		if (taille == null || taille.length != 2){
			x.assigne((float)-20);
			y.assigne((float)-20);
		}
		else {
			x.assigne(taille[0]);
			y.assigne(taille[1]);
		}
	}
	
	/*
	 * Opérations sur les zones speciales
	 */

	public Integer getNumSpecialAreas(){
		return new Integer(PPNative.GetNumSpecialAreas ());
	}

	public void getSpecialAreaPosition (InEntier zc, OutRéel x, OutRéel y){
		float [] position = PPNative.GetSpecialAreaPosition (zc.valeur().intValue());
		if (position == null || position.length != 2){
			x.assigne((float)-20);
			y.assigne((float)-20);
		}
		else {
			x.assigne(position[0]);
			y.assigne(position[1]);
		}
	}
	
	/*
	 * Opérations sur les ressources
	 */
	public Integer getResource(InEntier id){
		return new Integer(PPNative.GetResource (id.valeur().intValue()));
	}
	
	/*
	 * Opérations sur les coalitions
	 */

	public Integer getNumUnits (InEntier c){
		return new Integer(PPNative.GetNumUnits (c.valeur().intValue()));
	}

	public Integer getUnitAt (InEntier c, InEntier i){
		return new Integer (PPNative.GetUnitAt (c.valeur().intValue(), i.valeur().intValue()));
	}
	
	/*
	 * Opérations sur les unités
	 */

	public Integer unitGetCoalition (InEntier u){
		return new Integer (PPNative.Unit_GetCoalition (u.valeur().intValue()));
	}

	public Integer unitGetType (InEntier u){
		return new Integer (PPNative.Unit_GetType (u.valeur().intValue()));
	}

	public void unitGetPosition (InEntier u, OutRéel x, OutRéel y){
		float [] position = PPNative.Unit_GetPosition (u.valeur().intValue());
		if (position == null || position.length != 2){
			x.assigne((float)-20);
			y.assigne((float)-20);
		}
		else {
			x.assigne(position[0]);
			y.assigne(position[1]);
		}
	}

	public Float unitGetHealth (InEntier u){
		return new Float (PPNative.Unit_GetHealth (u.valeur().intValue()));
	}

	public Float unitGetMaxHealth (InEntier u){
		return new Float (PPNative.Unit_GetMaxHealth (u.valeur().intValue()));
	}

	public Integer unitGetGroup (InEntier u){
		return new Integer (PPNative.Unit_GetGroup (u.valeur().intValue()));
	}

	public Integer unitSetGroup (InEntier u, InEntier g){
		int ret;
		// ce test est nécessaire car ce cas n'est pas pris en compte dans Unit_SetGroup(...)
		if (g.valeur().intValue() == -1)
			ret = -10;
		else
			ret = PPNative.Unit_SetGroup (u.valeur().intValue(), g.valeur().intValue());
		return new Integer(ret);
	}

	public Integer unitRemoveFromGroup (InEntier u){
		return new Integer(PPNative.Unit_SetGroup (u.valeur().intValue(), -1));
	}

	public Integer unitGetNumPdgCmds (InEntier u){
		return new Integer(PPNative.Unit_GetNumPdgCmds (u.valeur().intValue()));
	}
	
	public Integer unitPdgCmdGetCode (InEntier u, InEntier idCmd){
		return new Integer(PPNative.Unit_PdgCmd_GetCode (u.valeur().intValue(), idCmd.valeur().intValue()));
	}
	
	public Integer unitPdgCmdGetNumParam (InEntier u, InEntier idCmd){
		return new Integer(PPNative.Unit_PdgCmd_GetNumParams (u.valeur().intValue(), idCmd.valeur().intValue()));
	}
	
	public Float unitPdgCmdGetParam (InEntier u, InEntier idCmd, InEntier idParam){
		return new Float(PPNative.Unit_PdgCmd_GetParam (u.valeur().intValue(), idCmd.valeur().intValue(), idParam.valeur().intValue()));
	}

	public Integer unitActionOnUnit (InEntier u, InEntier action, InEntier cible){
		return new Integer(PPNative.Unit_ActionOnUnit (u.valeur().intValue(), action.valeur().intValue(), cible.valeur().intValue()));
	}

	public Integer unitActionOnPosition (InEntier u, InEntier action, InRéel x, InRéel y){
		return new Integer(PPNative.Unit_ActionOnPosition (u.valeur().intValue(), action.valeur().intValue(), x.valeur().floatValue(), y.valeur().floatValue()));
	}

	public Integer unitUntargetedAction (InEntier u, InEntier action, InRéel param){
		return new Integer(PPNative.Unit_UntargetedAction (u.valeur().intValue(), action.valeur().intValue(), param.valeur().floatValue()));
	}
	
	public String getError (){
		return new String(PPNative.GetError());
	}

	public void clearError(){
		PPNative.ClearError();
	}
	
	public void enterCriticalSection(){
		PPNative.EnterCriticalSection();
	}
	
	public void exitCriticalSection(){
		PPNative.ExitCriticalSection();
	}
	
	/*
	 * Opérations additionnelles
	 */
	/* Retourne un nombre aléatoire compris entre 0.0 et limite. */
	public Float random (InRéel limite){
		return new Float ((float)Math.random()*limite.valeur().floatValue());
	}
	/* Effectue une pause de t milisecondes. */
	public void delay (InEntier t){
		try {
			Thread.sleep(t.valeur().longValue());
		} catch (InterruptedException e) {
			System.out.println("(delay - Erreur : attente interrompu.)");
		}
	}
	/* convertit un Réel en Entier */
	public Integer réelVersEntier (InRéel r){
		return new Integer (r.valeur().intValue());
	}
}
