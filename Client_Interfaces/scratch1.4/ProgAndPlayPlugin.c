/* Automatically generated from Squeak on (29 January 2010 10:47:09 pm) */

#if defined(WIN32) || defined(_WIN32) || defined(Win32)
 #ifdef __cplusplus
  #define DLLEXPORT extern "C" __declspec(dllexport)
 #else
  #define DLLEXPORT __declspec(dllexport)
 #endif /* C++ */
#else
 #define DLLEXPORT extern "C" 
#endif /* WIN32 */

#include "sqVirtualMachine.h"

/* memory access macros */
#define byteAt(i) (*((unsigned char *) (i)))
#define byteAtput(i, val) (*((unsigned char *) (i)) = val)
#define longAt(i) (*((int *) (i)))
#define longAtput(i, val) (*((int *) (i)) = val)

#include <string.h>
#include "PP_Client.h"
#include "PP_Client_Private.h"
#include "PP_Error.h"


/*** Variables ***/
struct VirtualMachine* interpreterProxy;
const char *moduleName = "ProgAndPlayPlugin 14 October 2012 (e)";

/*** Functions ***/
DLLEXPORT int primitiveOpen(void);
DLLEXPORT int primitiveClose(void);
DLLEXPORT int primitiveGameOver(void);
DLLEXPORT int primitiveMapWidth(void);
DLLEXPORT int primitiveMapHeight(void);
DLLEXPORT int primitiveStartPositionX(void);
DLLEXPORT int primitiveStartPositionY(void);
DLLEXPORT int primitiveNumSpecialAreas(void);
DLLEXPORT int primitiveSpecialAreasPosX(void);
DLLEXPORT int primitiveSpecialAreasPosY(void);
DLLEXPORT int primitiveResource(void);
DLLEXPORT int primitiveNumUnits(void);
DLLEXPORT int primitiveUnitOf(void);
DLLEXPORT int primitiveUnitGetCoalition(void);
DLLEXPORT int primitiveUnitGetType(void);
DLLEXPORT int primitiveXPosOf(void);
DLLEXPORT int primitiveYPosOf(void);
DLLEXPORT int primitiveHealthOf(void);
DLLEXPORT int primitiveMaxHealthOf(void);
DLLEXPORT int primitiveGroupOf(void);
DLLEXPORT int primitiveUnitSetGroup(void);
DLLEXPORT int primitiveActionOnPosition(void);
DLLEXPORT int primitiveActionOnUnit(void);
DLLEXPORT int primitiveUntargetedAction(void);

DLLEXPORT int primitiveNumPendingCommandsOf(void);
DLLEXPORT int primitiveUnitGetPendingCommandAt(void);
DLLEXPORT int primitivePdgCmdGetNumParams(void);
DLLEXPORT int primitivePdgCmdGetParam(void);

DLLEXPORT int setInterpreter(struct VirtualMachine* anInterpreter);

DLLEXPORT int primitiveOpen(void) {
	interpreterProxy->push((PP_Open() << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveClose(void) {
	interpreterProxy->push((PP_Close() << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveGameOver(void) {
	interpreterProxy->pushBool(PP_IsGameOver());
	return 0;
}

DLLEXPORT int primitiveMapWidth(void) {
	interpreterProxy->pushFloat(PP_GetMapSize().x);
	return 0;
}

DLLEXPORT int primitiveMapHeight(void) {
	interpreterProxy->pushFloat(PP_GetMapSize().y);
	return 0;
}

DLLEXPORT int primitiveStartPositionX(void) {
	interpreterProxy->pushFloat(PP_GetStartPosition().x);
	return 0;
}

DLLEXPORT int primitiveStartPositionY(void) {
	interpreterProxy->pushFloat(PP_GetStartPosition().y);
	return 0;
}

DLLEXPORT int primitiveNumSpecialAreas(void) {
	interpreterProxy->push((PP_GetNumSpecialAreas() << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveSpecialAreasPosX(void) {
	int id;
	
	id = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->pushFloat(PP_GetSpecialAreaPosition(id).x);
	return 0;
}

DLLEXPORT int primitiveSpecialAreasPosY(void) {
	int id;
	
	id = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->pushFloat(PP_GetSpecialAreaPosition(id).y);
	return 0;
}

DLLEXPORT int primitiveResource(void) {
	int id;
	
	id = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->push((PP_GetResource(id) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveNumUnits(void) {
	int idC;
	
	idC = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->push((PP_GetNumUnits((PP_Coalition)idC) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveUnitOf(void) {
	int id;
	int idC;
	int result;
	
	id = interpreterProxy->stackIntegerValue(1); // first parameter
	idC = interpreterProxy->stackIntegerValue(0); // second parameter
	interpreterProxy->push((PP_GetUnitAt((PP_Coalition)idC, id) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveUnitGetCoalition(void) {
	PP_Unit idU;
	
	idU = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->push((PP_Unit_GetCoalition(idU) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveUnitGetType(void) {
	PP_Unit idU;
	
	idU = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->push((PP_Unit_GetType(idU) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveXPosOf(void) {
	PP_Unit idU;
	
	idU = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->pushFloat(PP_Unit_GetPosition(idU).x);
	return 0;
}

DLLEXPORT int primitiveYPosOf(void) {
	PP_Unit idU;
	
	idU = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->pushFloat(PP_Unit_GetPosition(idU).y);
	return 0;
}

DLLEXPORT int primitiveHealthOf(void) {
	PP_Unit idU;
	
	idU = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->pushFloat(PP_Unit_GetHealth(idU));
	return 0;
}

DLLEXPORT int primitiveMaxHealthOf(void) {
	PP_Unit idU;
	
	idU = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->pushFloat(PP_Unit_GetMaxHealth(idU));
	return 0;
}

DLLEXPORT int primitiveGroupOf(void) {
	PP_Unit idU;
	
	idU = interpreterProxy->stackIntegerValue(0);
	interpreterProxy->push((PP_Unit_GetGroup(idU) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveNumPendingCommandsOf(void) {
	PP_Unit idU;
	
	idU = interpreterProxy->stackIntegerValue(0);
	enterCriticalSection();
	interpreterProxy->push((PP_Unit_GetNumPdgCmds(idU) << 1) | 1);
	exitCriticalSection();
	return 0;
}

DLLEXPORT int primitiveUnitGetPendingCommandAt(void) {
	int idPC;
	PP_Unit idU;
	
	idPC = interpreterProxy->stackIntegerValue(1); // first parameter
	idU = interpreterProxy->stackIntegerValue(0); // second parameter
	enterCriticalSection();
	interpreterProxy->push((PP_Unit_PdgCmd_GetCode(idU, idPC) << 1) | 1);
	exitCriticalSection();
	return 0;
}

DLLEXPORT int primitivePdgCmdGetNumParams(void) {
	int idPC;
	PP_Unit idU;
	
	idPC = interpreterProxy->stackIntegerValue(1); // first parameter
	idU = interpreterProxy->stackIntegerValue(0); // second parameter
	enterCriticalSection();
	interpreterProxy->push((PP_Unit_PdgCmd_GetNumParams(idU, idPC) << 1) | 1);
	exitCriticalSection();
	return 0;
}

DLLEXPORT int primitivePdgCmdGetParam(void){
	int idParam;
	int idPC;
	PP_Unit idU;
	
	idParam = interpreterProxy->stackIntegerValue(2); // first parameter
	idPC = interpreterProxy->stackIntegerValue(1); // second parameter
	idU = interpreterProxy->stackIntegerValue(0); // third parameter
	enterCriticalSection();
	interpreterProxy->pushFloat(PP_Unit_PdgCmd_GetParam(idU, idPC, idParam));
	exitCriticalSection();
	return 0;
}

DLLEXPORT int primitiveUnitSetGroup(void) {
	PP_Unit idU;
	int group;
	
	idU = interpreterProxy->stackIntegerValue(1); // first parameter
	group = interpreterProxy->stackIntegerValue(0); // second parameter
	interpreterProxy->push((PP_Unit_SetGroup(idU, group) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveActionOnPosition(void) {
	PP_Unit idU;
	int idA;
	PP_Pos p;
	
	idU = interpreterProxy->stackIntegerValue(3); // first parameter
	idA = interpreterProxy->stackIntegerValue(2); // second parameter
	p.x = interpreterProxy->stackFloatValue(1); // third parameter
	p.y = interpreterProxy->stackFloatValue(0); // fourth parameter
	interpreterProxy->push((PP_Unit_ActionOnPosition(idU, idA, p) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveActionOnUnit(void) {
	PP_Unit idU;
	int idA;
	PP_Unit idUTarget;
	
	idU = interpreterProxy->stackIntegerValue(2); // first parameter
	idA = interpreterProxy->stackIntegerValue(1); // second parameter
	idUTarget = interpreterProxy->stackIntegerValue(0); // third parameter
	interpreterProxy->push(
		(PP_Unit_ActionOnUnit(idU, idA, idUTarget) << 1) | 1);
	return 0;
}

DLLEXPORT int primitiveUntargetedAction(void) {
	PP_Unit idU;
	int idA;
	float param;
	
	idU = interpreterProxy->stackIntegerValue(2); // first parameter
	idA = interpreterProxy->stackIntegerValue(1); // second parameter
	param = interpreterProxy->stackFloatValue(0); // third parameter
	interpreterProxy->push(
		(PP_Unit_UntargetedAction(idU, idA, param) << 1) | 1);
	return 0;
}

DLLEXPORT int setInterpreter(struct VirtualMachine* anInterpreter) {
	int ok;

	interpreterProxy = anInterpreter;
	ok = interpreterProxy->majorVersion() == VM_PROXY_MAJOR;
	if (ok == 0) {
		return 0;
	}
	ok = interpreterProxy->minorVersion() >= VM_PROXY_MINOR;
	return ok;
}
