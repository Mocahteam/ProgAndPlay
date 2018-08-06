
#include "PP_Client.h"
#include "PP_Client_Private.h"
#include "traces/TraceConstantList.h"
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

// analyse le code "code" et lève une exception si besoin
// retourne le code si rien à signaler
int try_catch (int code, const char * source){
	char str[100];
	strcpy (str, source);
	if (code < 0){
		strcat (str, " -> ");
		strcat (str, PP_GetError());
		PP_ClearError();
		caml_failwith(str);
	}
	return code;
}

typedef value (*getData) (value, value);

value getSpecialAreaPosition (value id, value unit){
	CAMLparam2 (id, unit);
	CAMLlocal1 (pos);
	// récupération de la position
	PP_Pos p;
	int ret = PP_GetSpecialAreaPosition_prim (Int_val(id), &p);
	try_catch(ret, "Pp.getSpecialAreas");
	pos = caml_alloc_tuple(2);
	Store_field(pos, 0, caml_copy_double(p.x));
	Store_field(pos, 1, caml_copy_double(p.y));
	CAMLreturn (pos);
}

value getEntity (value id, value coalition){
	CAMLparam2 (id, coalition);
	// récupération de la coalition
	PP_Coalition c = (PP_Coalition)Int_val(coalition);
	int val = try_catch(PP_GetUnitAt_prim(c, Int_val(id)), "Pp.getEntities");
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
		float cParam;
		int ret = PP_Unit_PdgCmd_GetParam_prim((PP_Unit)Int_val(idUnit), Int_val(idCmd), cId, &cParam);
		try_catch(ret, "Pp.getPendingCommands");
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
	int cmdCode;
	int ret = PP_Unit_PdgCmd_GetCode_prim((PP_Unit)Int_val(entity), Int_val(idCmd), &cmdCode);
	try_catch(ret, "Pp.getPendingCommands");
	Store_field(cmd, 0, Val_int(cmdCode));
	// insertion de la liste des paramètres comme second élément du couple
	Store_field(cmd, 1, constructListParam(entity, idCmd, Val_int(0), Val_int(try_catch(PP_Unit_PdgCmd_GetNumParams_prim((PP_Unit)Int_val(entity), Int_val(idCmd)), "Pp.getPendingCommands"))));
	CAMLreturn (cmd);
}

value OCaml_TraceToken (value txt){
	CAMLparam1 (txt);
	printf("%s\n", String_val(txt));
	CAMLreturn (Val_bool(1));
}

value OCaml_OpenConnexion (value unit) {
	CAMLparam1 (unit);
	int retour = try_catch (PP_Open_prim(), "Pp.openConnexion");
	if (retour == 0){
		// notify function call to Spring
		enterCriticalSection();
			char msg [100];
			sprintf(msg, "%s %d", EXECUTION_START_TIME, PP_GetTimestamp_prim());
			PP_PushMessage_prim(msg, NULL);
			sprintf(msg, "%s OCaml", PROGRAMMING_LANGUAGE_USED);
			PP_PushMessage_prim(msg, NULL);
			PP_PushMessage_prim("PP_Open", NULL);
		exitCriticalSection();
	}
	CAMLreturn (Val_bool(retour == 0));
}

value OCaml_CloseConnexion (value unit) {
	CAMLparam1 (unit);
	int retour = try_catch (PP_Close(), "Pp.closeConnexion");
	CAMLreturn (Val_bool(retour == 0));
}

value OCaml_IsGameOver (value unit) {
	CAMLparam1 (unit);
	int retour = try_catch (PP_IsGameOver(), "Pp.isGameOver");
	CAMLreturn (Val_bool(retour > 0));
}

value OCaml_IsGamePaused (value unit) {
	CAMLparam1 (unit);
	int retour = try_catch (PP_IsGamePaused(), "Pp.isGamePaused");
	CAMLreturn (Val_bool(retour > 0));
}

value OCaml_GetMapSize (value unit) {
	CAMLparam1 (unit);
	CAMLlocal1 (pos);
	PP_Pos p = PP_GetMapSize ();
	try_catch ((int)p.x, "Pp.getMapSize");
	pos = caml_alloc_tuple(2);
	Store_field(pos, 0, caml_copy_double(p.x));
	Store_field(pos, 1, caml_copy_double(p.y));
	CAMLreturn (pos);
}

value OCaml_GetStartPosition (value unit) {
	CAMLparam1 (unit);
	CAMLlocal1 (pos);
	PP_Pos p = PP_GetStartPosition ();
	try_catch ((int)p.x, "Pp.getStartPosition");
	pos = caml_alloc_tuple(2);
	Store_field(pos, 0, caml_copy_double(p.x));
	Store_field(pos, 1, caml_copy_double(p.y));
	CAMLreturn (pos);
}

value OCaml_GetSpecialAreas (value unit) {
	CAMLparam1 (unit);
	CAMLlocal1 (list);
	int nbElem = try_catch(PP_GetNumSpecialAreas_prim(), "Pp.getSpecialAreas");
	list = constructList(Val_int(0), Val_int(nbElem), Val_unit,
		getSpecialAreaPosition);
	// notify function call to Spring
	enterCriticalSection();
		PP_PushMessage_prim("PP_GetSpecialAreas", NULL);
	exitCriticalSection();
	CAMLreturn (list);
}

value OCaml_GetResource (value resource) {
	CAMLparam1 (resource);
	int retour = try_catch(PP_GetResource(Int_val(resource)), "Pp.getResource");
	CAMLreturn (Val_int(retour));
}

value OCaml_GetEntities (value coalition) {
	CAMLparam1 (coalition);
	CAMLlocal1 (list);
	// récupération de la coalition
	PP_Coalition c = (PP_Coalition)Int_val(coalition);
	int nbElem = try_catch(PP_GetNumUnits_prim(c), "Pp.getEntities");
	list = constructList(Val_int(0), Val_int(nbElem), coalition, getEntity);
	enterCriticalSection();
		char msg [100];
		sprintf(msg, "PP_GetUnits %d", c);
		PP_PushMessage_prim(msg, NULL);
	exitCriticalSection();
	CAMLreturn (list);
}

value OCaml_Unit_GetCoalition (value entity) {
	CAMLparam1 (entity);
	int retour = try_catch(PP_Unit_GetCoalition(Int_val(entity)), "Pp.getCoalition");
	CAMLreturn (Val_int(retour));
}

value OCaml_Unit_GetType (value entity) {
	CAMLparam1 (entity);
	int val = try_catch(PP_Unit_GetType(Int_val(entity)), "Pp.getType");
	CAMLreturn (Val_int(val));
}

value OCaml_Unit_GetPosition (value entity) {
	CAMLparam1 (entity);
	CAMLlocal1 (pos);
	PP_Pos p = PP_Unit_GetPosition (Int_val(entity));
	try_catch((int)p.x, "Pp.getPosition");
	pos = caml_alloc_tuple(2);
	Store_field(pos, 0, caml_copy_double(p.x));
	Store_field(pos, 1, caml_copy_double(p.y));
	CAMLreturn (pos);
}

value OCaml_Unit_GetHealth (value entity) {
	CAMLparam1 (entity);
	float retour = PP_Unit_GetHealth (Int_val(entity));
	try_catch((int)retour, "Pp.getHealth");
	CAMLreturn (caml_copy_double(retour));
}

value OCaml_Unit_GetMaxHealth (value entity) {
	CAMLparam1 (entity);
	float retour = PP_Unit_GetMaxHealth (Int_val(entity));
	try_catch((int)retour, "Pp.getMaxHealth");
	CAMLreturn (caml_copy_double(retour));
}

value OCaml_Unit_GetGroup (value entity) {
	CAMLparam1 (entity);
	int retour = try_catch(PP_Unit_GetGroup(Int_val(entity)), "Pp.getGroup");
	CAMLreturn (Val_int(retour));
}

value OCaml_Unit_SetGroup (value couple) {
	CAMLparam1 (couple);
	int entity = Int_val(Field(couple, 0));
	int g = Int_val(Field(couple, 1));
	int retour = try_catch(PP_Unit_SetGroup(entity, g), "Pp.setGroup");
	CAMLreturn (Val_bool(retour == 0));
}

value OCaml_Unit_GetPendingCommands (value entity) {
	CAMLparam1 (entity);
	CAMLlocal1 (list);
	// récupération de l'id de l'unité
	PP_Unit e = (PP_Unit)Int_val(entity);
	enterCriticalSection();
	int nbElem = try_catch(PP_Unit_GetNumPdgCmds_prim(e), "Pp.getPendingCommands");
	list = constructList(Val_int(0), Val_int(nbElem), entity, getPendingCommand);
	// notify function call to Spring
	char msg [100];
	int type = PP_Unit_GetType_prim(e);
	if (type >= 0)
		sprintf(msg, "PP_Unit_GetPendingCommands %d_%d", e, type);
	else
		sprintf(msg, "PP_Unit_GetPendingCommands %d", e);
	PP_PushMessage_prim(msg, NULL);
	exitCriticalSection();
	CAMLreturn (list);
}

value OCaml_Unit_ActionOnUnit (value quadruplet) {
	CAMLparam1 (quadruplet);
	int cSrc = Int_val(Field(quadruplet, 0));
	int cAction = Int_val(Field(quadruplet, 1));
	int cTarget = Int_val(Field(quadruplet, 2));
	int synchro = Int_val(Field(quadruplet, 3));
	int retour = try_catch(PP_Unit_ActionOnUnit(cSrc, cAction, cTarget, synchro), "Pp.actionOnEntity");
	CAMLreturn (Val_bool(retour == 0));
	
}

value OCaml_Unit_ActionOnPosition (value quadruplet) {
	CAMLparam1 (quadruplet);
	int cSrc = Int_val(Field(quadruplet, 0));
	int cAction = Int_val(Field(quadruplet, 1));
	PP_Pos p;
	p.x = Double_val(Field(Field(quadruplet, 2), 0));
	p.y = Double_val(Field(Field(quadruplet, 2), 1));
	int synchro = Int_val(Field(quadruplet, 3));
	int retour = try_catch(PP_Unit_ActionOnPosition(cSrc, cAction, p, synchro), "Pp.actionOnPosition");
	CAMLreturn (Val_bool(retour == 0));
}

value OCaml_Unit_UntargetedAction (value quadruplet) {
	CAMLparam1 (quadruplet);
	int cSrc = Int_val(Field(quadruplet, 0));
	int cAction = Int_val(Field(quadruplet, 1));
	float param = Double_val(Field(quadruplet, 2));
	int synchro = Int_val(Field(quadruplet, 3));
	int retour = try_catch(PP_Unit_UntargetedAction(cSrc, cAction, param, synchro), "Pp.untargetedAction");
	CAMLreturn (Val_bool(retour == 0));
}

#ifdef __cplusplus
}
#endif
