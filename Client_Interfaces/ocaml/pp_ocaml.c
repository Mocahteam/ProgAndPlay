
#include "PP_Client.h"
#include "PP_Client_Private.h"
#include "PP_Error.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/fail.h>

#include <cstring>
#include <cstdio>

int openned = 0;

// analyse le code "code" et lève une exception si besoin
// retourne le code si rien à signaler
int try_catch (int code, const char * source){
	char str[100];
	strcpy (str, source);
	if (code == -1){
		strcat (str, " -> ");
		strcat (str, PP_GetError());
		PP_ClearError();
		caml_failwith(str);
	}
	return code;
}

void needOpenned (const char * source){
	if (!openned){
		try_catch (PP_Open(), source); // ouverture
		openned = 1;
	}
}

void needClosed (const char * source){
	if (openned){
		try_catch (PP_Close(), source); // ouverture
		openned = 0;
	}
}

typedef value (*getData) (value, value);

value getSpecialAreaPosition (value id, value unit){
	CAMLparam2 (id, unit);
	CAMLlocal1 (pos);
	// récupération de la position
	PP_Pos p = PP_GetSpecialAreaPosition (Int_val(id));
	try_catch((int)p.x, "Pp.getSpecialAreas");
	pos = caml_alloc_tuple(2);
	Store_field(pos, 0, caml_copy_double(p.x));
	Store_field(pos, 1, caml_copy_double(p.y));
	CAMLreturn (pos);
}

value getEntity (value id, value coalition){
	CAMLparam2 (id, coalition);
	// récupération de la coalition
	PP_Coalition c = (PP_Coalition)Int_val(coalition);
	int val = try_catch(PP_GetUnitAt(c, Int_val(id)), "Pp.getEntities");
	CAMLreturn (Val_int(val));
}

value constructList (value id, value limite, value optParam, getData fct){
	CAMLparam2 (id, limite);
	CAMLlocal2 (list, pos);
	int cId = Int_val(id);
	if (cId < Int_val(limite)){
		// création du couple représentant la liste
		list = caml_alloc_tuple(2);
		// insertion de la donnée dans le premier élément du couple
		Store_field(list, 0, fct(id, optParam));
		// insertion de la queue de la liste dans le deuxième élément du couple
		Store_field(list, 1, constructList(Val_int(cId+1), limite, optParam, fct));
	}
	else
		list = Val_int(0);
	CAMLreturn (list);
}

value constructListParam (value idUnit, value idCmd, value idParam, value nbParam){
	CAMLparam4 (idUnit, idCmd, idParam, nbParam);
	CAMLlocal1 (list);
	int cId = Int_val(idParam);
	if (cId < Int_val(nbParam)){
		// création du couple représentant la liste
		list = caml_alloc_tuple(2);
		// insertion de la donnée dans le premier élément du couple
		float cParam = PP_Unit_PdgCmd_GetParam((PP_Unit)Int_val(idUnit), Int_val(idCmd), cId);
		try_catch((int)cParam, "Pp.getPendingCommands");
		Store_field(list, 0, caml_copy_double(cParam));
		// insertion de la queue de la liste dans le deuxième élément du couple
		Store_field(list, 1, constructListParam(idUnit, idCmd, Val_int(cId+1), nbParam));
	}
	else
		list = Val_int(0);
	CAMLreturn (list);
}

value getPendingCommand (value idCmd, value entity){
	CAMLparam2 (idCmd, entity);
	CAMLlocal1 (cmd);
	// récupération de l'id de l'entité
	PP_Unit e = (PP_Unit)Int_val(entity);
	cmd = caml_alloc_tuple(2);
	// insertion du code de la commande
	Store_field(cmd, 0, Val_int(try_catch(PP_Unit_PdgCmd_GetCode((PP_Unit)Int_val(entity), Int_val(idCmd)), "Pp.getPendingCommands")));
	// insertion de la liste des paramètres comme second élément du couple
	Store_field(cmd, 1, constructListParam(entity, idCmd, Val_int(0), Val_int(try_catch(PP_Unit_PdgCmd_GetNumParams((PP_Unit)Int_val(entity), Int_val(idCmd)), "Pp.getPendingCommands"))));
	CAMLreturn (cmd);
}

value OCaml_IsGameOver (value unit) {
	CAMLparam1 (unit);
	needOpenned("Pp.isGameOver");
	int retour = try_catch (PP_IsGameOver(), "Pp.isGameOver");
	needClosed("Pp.isGameOver");
	CAMLreturn (Val_bool(retour > 0));
}

value OCaml_GetMapSize (value unit) {
	CAMLparam1 (unit);
	CAMLlocal1 (pos);
	needOpenned("Pp.getMapSize");
	PP_Pos p = PP_GetMapSize ();
	try_catch ((int)p.x, "Pp.getMapSize");
	pos = caml_alloc_tuple(2);
	Store_field(pos, 0, caml_copy_double(p.x));
	Store_field(pos, 1, caml_copy_double(p.y));
	needClosed("Pp.getMapSize");
	CAMLreturn (pos);
}

value OCaml_GetStartPosition (value unit) {
	CAMLparam1 (unit);
	CAMLlocal1 (pos);
	needOpenned("Pp.getStartPosition");
	PP_Pos p = PP_GetStartPosition ();
	try_catch ((int)p.x, "Pp.getStartPosition");
	pos = caml_alloc_tuple(2);
	Store_field(pos, 0, caml_copy_double(p.x));
	Store_field(pos, 1, caml_copy_double(p.y));
	needClosed("Pp.getStartPosition");
	CAMLreturn (pos);
}

value OCaml_GetSpecialAreas (value unit) {
	CAMLparam1 (unit);
	CAMLlocal1 (list);
	needOpenned("Pp.getSpecialAreas");
	int nbElem = try_catch(PP_GetNumSpecialAreas(), "Pp.getSpecialAreas");
	list = constructList(Val_int(0), Val_int(nbElem), Val_unit,
		getSpecialAreaPosition);
	needClosed("Pp.getSpecialAreas");
	CAMLreturn (list);
}

value OCaml_GetResource (value resource) {
	CAMLparam1 (resource);
	needOpenned("Pp.getResource");
	int retour = try_catch(PP_GetResource(Int_val(resource)), "Pp.getResource");
	needClosed("Pp.getResource");
	CAMLreturn (Val_int(retour));
}

value OCaml_GetEntities (value coalition) {
	CAMLparam1 (coalition);
	CAMLlocal1 (list);
	// récupération de la coalition
	PP_Coalition c = (PP_Coalition)Int_val(coalition);
	needOpenned("Pp.getEntities");
	enterCriticalSection();
	int nbElem = try_catch(PP_GetNumUnits(c), "Pp.getEntities");
	list = constructList(Val_int(0), Val_int(nbElem), coalition, getEntity);
	exitCriticalSection();
	needClosed("Pp.getEntities");
	CAMLreturn (list);
}

value OCaml_Unit_GetCoalition (value entity) {
	CAMLparam1 (entity);
	needOpenned("Pp.getCoalition");
	int retour = try_catch(PP_Unit_GetCoalition(Int_val(entity)), "Pp.getCoalition");
	needClosed("Pp.getCoalition");
	CAMLreturn (Val_int(retour));
}

value OCaml_Unit_GetType (value entity) {
	CAMLparam1 (entity);
	needOpenned("Pp.getType");
	char str[100];
	sprintf (str, "Pp.getType %d", Int_val(entity));
	int val = try_catch(PP_Unit_GetType(Int_val(entity)), str);
	needClosed("Pp.getType");
	CAMLreturn (Val_int(val));
}

value OCaml_Unit_GetPosition (value entity) {
	CAMLparam1 (entity);
	CAMLlocal1 (pos);
	needOpenned("Pp.getPosition");
	PP_Pos p = PP_Unit_GetPosition (Int_val(entity));
	try_catch((int)p.x, "Pp.getPosition");
	pos = caml_alloc_tuple(2);
	Store_field(pos, 0, caml_copy_double(p.x));
	Store_field(pos, 1, caml_copy_double(p.y));
	needClosed("Pp.getPosition");
	CAMLreturn (pos);
}

value OCaml_Unit_GetHealth (value entity) {
	CAMLparam1 (entity);
	needOpenned("Pp.getHealth");
	float retour = PP_Unit_GetHealth (Int_val(entity));
	try_catch((int)retour, "Pp.getHealth");
	needClosed("Pp.getHealth");
	CAMLreturn (caml_copy_double(retour));
}

value OCaml_Unit_GetMaxHealth (value entity) {
	CAMLparam1 (entity);
	needOpenned("Pp.getMaxHealth");
	float retour = PP_Unit_GetMaxHealth (Int_val(entity));
	try_catch((int)retour, "Pp.getMaxHealth");
	needClosed("Pp.getMaxHealth");
	CAMLreturn (caml_copy_double(retour));
}

value OCaml_Unit_GetGroup (value entity) {
	CAMLparam1 (entity);
	needOpenned("Pp.getGroup");
	int retour = try_catch(PP_Unit_GetGroup(Int_val(entity)), "Pp.getGroup");
	needClosed("Pp.getGroup");
	CAMLreturn (Val_int(retour));
}

value OCaml_Unit_SetGroup (value couple) {
/*	CAMLparam2 (entity, g);
	CAMLreturn (Val_bool(PP_Unit_SetGroup(Int_val(entity), Int_val(g)) == 0));*/
	CAMLparam1 (couple);
	int entity = Int_val(Field(couple, 0));
	int g = Int_val(Field(couple, 1));
	needOpenned("Pp.setGroup");
	int retour = try_catch(PP_Unit_SetGroup(entity, g), "Pp.setGroup");
	needClosed("Pp.setGroup");
	CAMLreturn (Val_bool(retour == 0));
}

value OCaml_Unit_GetPendingCommands (value entity) {
	CAMLparam1 (entity);
	CAMLlocal1 (list);
	// récupération de l'id de l'unité
	PP_Unit e = (PP_Unit)Int_val(entity);
	needOpenned("Pp.getPendingCommands");
	enterCriticalSection();
	int nbElem = try_catch(PP_Unit_GetNumPdgCmds(e), "Pp.getPendingCommands");
	list = constructList(Val_int(0), Val_int(nbElem), entity, getPendingCommand);
	exitCriticalSection();
	needClosed("Pp.getPendingCommands");
	CAMLreturn (list);
}

value OCaml_Unit_ActionOnUnit (value triplet) {
/*	CAMLparam3 (src, action, target);
	int cAction = Int_val(action);
	int cSrc = Int_val(src);
	int cTarget = Int_val(target);
	CAMLreturn (Val_bool(PP_Unit_ActionOnUnit(cSrc, cAction, cTarget) == 0));*/
	CAMLparam1 (triplet);
	int cSrc = Int_val(Field(triplet, 0));
	int cAction = Int_val(Field(triplet, 1));
	int cTarget = Int_val(Field(triplet, 2));
	needOpenned("Pp.actionOnEntity");
	int retour = try_catch(PP_Unit_ActionOnUnit(cSrc, cAction, cTarget), "Pp.actionOnEntity");
	needClosed("Pp.actionOnEntity");
	CAMLreturn (Val_bool(retour == 0));
	
}

value OCaml_Unit_ActionOnPosition (value triplet) {
/*	CAMLparam3 (src, action, pos);
	PP_Pos p;
	p.x = Double_val(Field(pos, 0));
	p.y = Double_val(Field(pos, 1));
	int cAction = Int_val(action);
	int cSrc = Int_val(src);
	CAMLreturn (Val_bool(PP_Unit_ActionOnPosition(cSrc, cAction, p) == 0));*/
	CAMLparam1 (triplet);
	int cSrc = Int_val(Field(triplet, 0));
	int cAction = Int_val(Field(triplet, 1));
	PP_Pos p;
	p.x = Double_val(Field(Field(triplet, 2), 0));
	p.y = Double_val(Field(Field(triplet, 2), 1));
	needOpenned("Pp.actionOnPosition");
	int retour = try_catch(PP_Unit_ActionOnPosition(cSrc, cAction, p), "Pp.actionOnPosition");
	needClosed("Pp.actionOnPosition");
	CAMLreturn (Val_bool(retour == 0));
}

value OCaml_Unit_UntargetedAction (value triplet) {
	CAMLparam1 (triplet);
	int cSrc = Int_val(Field(triplet, 0));
	int cAction = Int_val(Field(triplet, 1));
	float param = Double_val(Field(triplet, 2));
	needOpenned("Pp.untargetedAction");
	int retour = try_catch(PP_Unit_UntargetedAction(cSrc, cAction, param), "Pp.untargetedAction");
	needClosed("Pp.untargetedAction");
	CAMLreturn (Val_bool(retour == 0));
}

#ifdef __cplusplus
}
#endif
