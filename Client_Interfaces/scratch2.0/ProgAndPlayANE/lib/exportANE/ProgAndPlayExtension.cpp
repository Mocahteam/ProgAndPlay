#include "FlashRuntimeExtensions.h"
#include "PP_Client.h"
#include "PP_Error.h"
#include "stdlib.h"

#include "PP_Client_Private.h"
#include "traces/TraceConstantList.h"
#include <sstream>

#ifdef __cplusplus
extern "C" {
#endif

void ProgAndPlayContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx, uint32_t* numFunctionsToSet, const FRENamedFunction** functionsToSet);
void ProgAndPlayContextFinalizer(FREContext ctx);

FREObject FRE_PP_Open(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Close(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);

FREObject FRE_PP_IsGameOver(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_IsGamePaused(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetMapWidth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetMapHeight(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetStartPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetStartPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetNumSpecialArea(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetSpecialAreaPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetSpecialAreaPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetResource(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetNumUnits(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_GetUnitAt(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_BelongTo(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_IsType(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetPositionX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetPositionY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetHealth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetMaxHealth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetGroup(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_SetGroup(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetNumPendingCmds(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_PdgCmd_IsEqualTo(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_PdgCmd_GetNumParams(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_PdgCmd_GetParamAt(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_ActionOnPosition(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_ActionOnUnit(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_UntargetedAction(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_UntargetedActionWithoutParam(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);

FREObject FRE_PP_GetError(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_ClearError(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);

void ProgAndPlayInitializer(void** extDataToSet, FREContextInitializer* ctxInitializerToSet, FREContextFinalizer* ctxFinalizerToSet) {
	extDataToSet = 0;  // pas de données commune au contexte dans notre cas. 
	*ctxInitializerToSet = &ProgAndPlayContextInitializer;
	*ctxFinalizerToSet = &ProgAndPlayContextFinalizer;
}

void ProgAndPlayFinalizer(void* extData) {
	// Rien à faire
	return;
}

void ProgAndPlayContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx, 
                        uint32_t* numFunctionsToSet, 
                        const FRENamedFunction** functionsToSet) { 
	static FRENamedFunction extensionFunctions[] =
	{
		{ (const uint8_t*) "PP_Open_wrapper",								0, &FRE_PP_Open },
		{ (const uint8_t*) "PP_Close_wrapper",								0, &FRE_PP_Close },
		{ (const uint8_t*) "PP_IsGameOver_wrapper",							0, &FRE_PP_IsGameOver },
		{ (const uint8_t*) "PP_IsGamePaused_wrapper",						0, &FRE_PP_IsGamePaused },
		{ (const uint8_t*) "PP_GetMapWidth_wrapper",						0, &FRE_PP_GetMapWidth },
		{ (const uint8_t*) "PP_GetMapHeight_wrapper",						0, &FRE_PP_GetMapHeight },
		{ (const uint8_t*) "PP_GetStartPosX_wrapper",						0, &FRE_PP_GetStartPosX },
		{ (const uint8_t*) "PP_GetStartPosY_wrapper",						0, &FRE_PP_GetStartPosY },
		{ (const uint8_t*) "PP_GetNumSpecialArea_wrapper",					0, &FRE_PP_GetNumSpecialArea },
		{ (const uint8_t*) "PP_GetSpecialAreaPosX_wrapper",					0, &FRE_PP_GetSpecialAreaPosX },
		{ (const uint8_t*) "PP_GetSpecialAreaPosY_wrapper",					0, &FRE_PP_GetSpecialAreaPosY },
		{ (const uint8_t*) "PP_GetResource_wrapper",						0, &FRE_PP_GetResource },
		{ (const uint8_t*) "PP_GetNumUnits_wrapper",						0, &FRE_PP_GetNumUnits },
		{ (const uint8_t*) "PP_GetUnitAt_wrapper",							0, &FRE_PP_GetUnitAt },
		{ (const uint8_t*) "PP_Unit_BelongTo_wrapper",						0, &FRE_PP_Unit_BelongTo },
		{ (const uint8_t*) "PP_Unit_IsType_wrapper",						0, &FRE_PP_Unit_IsType },
		{ (const uint8_t*) "PP_Unit_GetPositionX_wrapper",					0, &FRE_PP_Unit_GetPositionX },
		{ (const uint8_t*) "PP_Unit_GetPositionY_wrapper",					0, &FRE_PP_Unit_GetPositionY },
		{ (const uint8_t*) "PP_Unit_GetHealth_wrapper",						0, &FRE_PP_Unit_GetHealth },
		{ (const uint8_t*) "PP_Unit_GetMaxHealth_wrapper",					0, &FRE_PP_Unit_GetMaxHealth },
		{ (const uint8_t*) "PP_Unit_GetGroup_wrapper",						0, &FRE_PP_Unit_GetGroup },
		{ (const uint8_t*) "PP_Unit_SetGroup_wrapper",						0, &FRE_PP_Unit_SetGroup },
		{ (const uint8_t*) "PP_Unit_GetNumPendingCmds_wrapper",				0, &FRE_PP_Unit_GetNumPendingCmds },
		{ (const uint8_t*) "PP_Unit_PdgCmd_IsEqualTo_wrapper",				0, &FRE_PP_Unit_PdgCmd_IsEqualTo },
		{ (const uint8_t*) "PP_Unit_PdgCmd_GetNumParams_wrapper",			0, &FRE_PP_Unit_PdgCmd_GetNumParams },
		{ (const uint8_t*) "PP_Unit_PdgCmd_GetParamAt_wrapper",				0, &FRE_PP_Unit_PdgCmd_GetParamAt },
		{ (const uint8_t*) "PP_Unit_ActionOnPosition_wrapper",				0, &FRE_PP_Unit_ActionOnPosition },
		{ (const uint8_t*) "PP_Unit_ActionOnUnit_wrapper",					0, &FRE_PP_Unit_ActionOnUnit },
		{ (const uint8_t*) "PP_Unit_UntargetedAction_wrapper",				0, &FRE_PP_Unit_UntargetedAction },
		{ (const uint8_t*) "PP_Unit_UntargetedActionWithoutParam_wrapper",	0, &FRE_PP_Unit_UntargetedActionWithoutParam },
		{ (const uint8_t*) "PP_GetError_wrapper",							0, &FRE_PP_GetError },
		{ (const uint8_t*) "PP_ClearError_wrapper",							0, &FRE_PP_ClearError }
	};

	// Tell AIR how many functions there are in the array:
	*numFunctionsToSet = sizeof(extensionFunctions) / sizeof(FRENamedFunction);

	// Set the output parameter to point to the array we filled in:
	*functionsToSet = extensionFunctions;
}

void ProgAndPlayContextFinalizer(FREContext ctx){
	// Rien à faire dans notre cas
}

FREObject FRE_PP_Open(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	if (PP_Open_prim() == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss_start(std::ostringstream::out);
			oss_start << EXECUTION_START_TIME << " " << PP_GetTimestamp_prim();
			PP_PushMessage_prim(oss_start.str().c_str(), NULL);
			std::ostringstream oss_lang(std::ostringstream::out);
			oss_lang << PROGRAMMING_LANGUAGE_USED << " Scratch";
			PP_PushMessage_prim(oss_lang.str().c_str(), NULL);
			PP_PushMessage_prim("PP_Open", NULL);
		exitCriticalSection();
	}
	return 0;
}

FREObject FRE_PP_Close(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	PP_Close();
	return 0;
}

FREObject FRE_PP_IsGameOver(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	int isGameOver = PP_IsGameOver() > 0 ? 1 : 0;
	FRENewObjectFromBool(isGameOver, &value);
	return value;
}

FREObject FRE_PP_IsGamePaused(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	int isGamePaused = PP_IsGamePaused() > 0 ? 1 : 0;
	FRENewObjectFromBool(isGamePaused, &value);
	return value;
}

FREObject FRE_PP_GetMapWidth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	PP_Pos tmp;
	if (PP_GetMapSize_prim(&tmp) == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetMapWidth - " << tmp.x;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
		FRENewObjectFromDouble(tmp.x, &value);
	}
	else
		FRENewObjectFromDouble(-1, &value);
	return value;
}

FREObject FRE_PP_GetMapHeight(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	PP_Pos tmp;
	if (PP_GetMapSize_prim(&tmp) == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetMapHeight - " << tmp.y;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
		FRENewObjectFromDouble(tmp.y, &value);
	}
	else
		FRENewObjectFromDouble(-1, &value);
	return value;
}

FREObject FRE_PP_GetStartPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	PP_Pos tmp;
	if (PP_GetStartPosition_prim(&tmp) == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetStartXPosition - " << tmp.x;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
		FRENewObjectFromDouble(tmp.x, &value);
	}
	else
		FRENewObjectFromDouble(-1, &value);
	return value;
}

FREObject FRE_PP_GetStartPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	PP_Pos tmp;
	if (PP_GetStartPosition_prim(&tmp) == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetStartYPosition - " << tmp.y;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
		FRENewObjectFromDouble(tmp.y, &value);
	}
	else
		FRENewObjectFromDouble(-1, &value);
	return value;
}

FREObject FRE_PP_GetNumSpecialArea(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromInt32(PP_GetNumSpecialAreas(), &value);
	return value;
}

FREObject FRE_PP_GetSpecialAreaPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int num;
	FREGetObjectAsInt32(argv[0], &num);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	PP_Pos tmp;
	int ret = PP_GetSpecialAreaPosition_prim(num, &tmp);
	if (ret > FEEDBACK_COUNT_LIMIT){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetSpecialAreaXPosition";
			if (ret >= 0)
				oss <<  " - " << tmp.x;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
		FRENewObjectFromDouble(tmp.x, &value);
	}
	else
		FRENewObjectFromDouble(-1, &value);
	return value;
}

FREObject FRE_PP_GetSpecialAreaPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int num;
	FREGetObjectAsInt32(argv[0], &num);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	PP_Pos tmp;
	int ret = PP_GetSpecialAreaPosition_prim(num, &tmp);
	if (ret > FEEDBACK_COUNT_LIMIT){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetSpecialAreaYPosition";
			if (ret >= 0)
				oss <<  " - " << tmp.y;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
		FRENewObjectFromDouble(tmp.y, &value);
	}
	else
		FRENewObjectFromDouble(-1, &value);
	return value;
}

FREObject FRE_PP_GetResource(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int num;
	FREGetObjectAsInt32(argv[0], &num);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromInt32(PP_GetResource(num), &value);
	return value;
}

FREObject FRE_PP_GetNumUnits(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int coalition;
	FREGetObjectAsInt32(argv[0], &coalition);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromInt32(PP_GetNumUnits((PP_Coalition)coalition), &value);
	return value;
}

FREObject FRE_PP_GetUnitAt(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int id;
	FREGetObjectAsInt32(argv[0], &id);
	int coalition;
	FREGetObjectAsInt32(argv[1], &coalition);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	int ret = PP_GetUnitAt_prim ((PP_Coalition)coalition, id);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_GetUnitAtIndexFirst " << id << " " << coalition;
			if (ret >= 0)
				oss << " - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	FRENewObjectFromInt32(ret, &value);
	return value;
	
}

FREObject FRE_PP_Unit_BelongTo(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	int coalition;
	FREGetObjectAsInt32(argv[1], &coalition);
	// Appel Prog&Play pour tester la coalition
	int ret = PP_Unit_GetCoalition_prim(unitId);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_BelongTo " << unitId;
			int type = PP_Unit_GetType_prim(unitId);
			if (type >= 0)
				oss << "_" << type;
			oss << " " << coalition;
			if (ret >= 0)
				oss << " - " << (ret == coalition);
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	FREObject value;
	FRENewObjectFromBool(ret == coalition, &value);
	return value;
}

FREObject FRE_PP_Unit_IsType(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	int unitType;
	FREGetObjectAsInt32(argv[1], &unitType);
	// Appel Prog&Play pour tester le type de l'unité
	int ret = PP_Unit_GetType_prim(unitId);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_IsType " << unitId;
			if (ret >= 0)
				oss << "_" << ret;
			oss << " " << unitType;
			if (ret >= 0)
				oss << " - " << (ret == unitType);
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	FREObject value;
	FRENewObjectFromBool(ret == unitType, &value);
	return value;
}

FREObject FRE_PP_Unit_GetPositionX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	PP_Pos tmp;
	int ret = PP_Unit_GetPosition_prim(unitId, &tmp);
	if (ret > FEEDBACK_COUNT_LIMIT){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetXPosition " << unitId;
			int type = PP_Unit_GetType_prim(unitId);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss <<  " - " << tmp.x;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
		FRENewObjectFromDouble(tmp.x, &value);
	}
	else
		FRENewObjectFromDouble(-1, &value);
	return value;
}

FREObject FRE_PP_Unit_GetPositionY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	PP_Pos tmp;
	int ret = PP_Unit_GetPosition_prim(unitId, &tmp);
	if (ret > FEEDBACK_COUNT_LIMIT){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetYPosition " << unitId;
			int type = PP_Unit_GetType_prim(unitId);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss <<  " - " << tmp.y;
			PP_PushMessage_prim(oss.str().c_str(), NULL);
		exitCriticalSection();
		FRENewObjectFromDouble(tmp.y, &value);
	}
	else
		FRENewObjectFromDouble(-1, &value);
	return value;
}

FREObject FRE_PP_Unit_GetHealth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromDouble(PP_Unit_GetHealth(unitId), &value);
	return value;
}

FREObject FRE_PP_Unit_GetMaxHealth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromDouble(PP_Unit_GetMaxHealth(unitId), &value);
	return value;
}

FREObject FRE_PP_Unit_GetGroup(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromInt32(PP_Unit_GetGroup(unitId), &value);
	return value;
}

FREObject FRE_PP_Unit_SetGroup(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	int groupId;
	FREGetObjectAsInt32(argv[1], &groupId);
	// Appel Prog&Play
	PP_Unit_SetGroup(unitId, groupId);
	return 0;
}

FREObject FRE_PP_Unit_GetNumPendingCmds(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour
	FREObject value;
	enterCriticalSection();
		int ret = PP_Unit_GetNumPdgCmds_prim(unitId);
		if (ret > FEEDBACK_COUNT_LIMIT) {
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_GetNumPdgCmds " << unitId;
			int type = PP_Unit_GetType_prim(unitId);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss << " - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
			FRENewObjectFromInt32(ret, &value);
		}
		if (ret < 0)
			FRENewObjectFromInt32(-1, &value);
	exitCriticalSection();
	return value;
}

FREObject FRE_PP_Unit_PdgCmd_IsEqualTo(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int cmdId;
	FREGetObjectAsInt32(argv[0], &cmdId);
	int unitId;
	FREGetObjectAsInt32(argv[1], &unitId);
	int cmdCodeToTest;
	FREGetObjectAsInt32(argv[2], &cmdCodeToTest);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	enterCriticalSection();
		int currentCmdCode;
		int ret = PP_Unit_PdgCmd_GetCode_prim(unitId, cmdId, &currentCmdCode);
		if (ret > FEEDBACK_COUNT_LIMIT) {
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_PdgCmd_IsEqualTo " << cmdId << " " << unitId;
			int type = PP_Unit_GetType_prim(unitId);
			if (type >= 0)
				oss << "_" << type;
			oss << " " << cmdCodeToTest;
			if (ret >= 0)
				oss << " - " << (cmdCodeToTest == currentCmdCode);
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		}
		if (ret < 0)
			ret = -1;
	exitCriticalSection();
	FRENewObjectFromBool(cmdCodeToTest == currentCmdCode, &value);
	return value;
}

FREObject FRE_PP_Unit_PdgCmd_GetNumParams(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int cmdId;
	FREGetObjectAsInt32(argv[0], &cmdId);
	int unitId;
	FREGetObjectAsInt32(argv[1], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	enterCriticalSection();
		int ret = PP_Unit_PdgCmd_GetNumParams_prim(unitId, cmdId);
		if (ret > FEEDBACK_COUNT_LIMIT) {
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_PdgCmd_GetNumParams " << cmdId << " " << unitId;
			int type = PP_Unit_GetType_prim(unitId);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss << " - " << ret;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
			FRENewObjectFromInt32(ret, &value);
		}
		if (ret < 0)
			FRENewObjectFromInt32(-1, &value);
	exitCriticalSection();
	return value;
}

FREObject FRE_PP_Unit_PdgCmd_GetParamAt(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int paramId;
	FREGetObjectAsInt32(argv[0], &paramId);
	int cmdId;
	FREGetObjectAsInt32(argv[1], &cmdId);
	int unitId;
	FREGetObjectAsInt32(argv[2], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	enterCriticalSection();
		float paramValue;
		int ret = PP_Unit_PdgCmd_GetParam_prim(unitId, cmdId, paramId, &paramValue);
		if (ret > FEEDBACK_COUNT_LIMIT) {
			// notify function call to Spring
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_PdgCmd_GetParam " << paramId << " " << cmdId << " " << unitId;
			int type = PP_Unit_GetType_prim(unitId);
			if (type >= 0)
				oss << "_" << type;
			if (ret >= 0)
				oss << " - " << paramValue;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
			FRENewObjectFromDouble(paramValue, &value);
		}
		if (ret < 0)
			FRENewObjectFromDouble(-1, &value);
	exitCriticalSection();
	return value;
}

FREObject FRE_PP_Unit_ActionOnPosition(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	int cmdId;
	FREGetObjectAsInt32(argv[1], &cmdId);
	PP_Pos p;
	double val;
	FREGetObjectAsDouble(argv[2], &val);
	p.x = val;
	FREGetObjectAsDouble(argv[3], &val);
	p.y = val;
	int synchro;
	FREGetObjectAsInt32(argv[4], &synchro);
	// Appel Prog&Play
	PP_Unit_ActionOnPosition(unitId, cmdId, p, synchro);
	return 0;
}

FREObject FRE_PP_Unit_ActionOnUnit(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	int cmdId;
	FREGetObjectAsInt32(argv[1], &cmdId);
	int targetId;
	FREGetObjectAsInt32(argv[2], &targetId);
	int synchro;
	FREGetObjectAsInt32(argv[3], &synchro);
	// Appel Prog&Play
	PP_Unit_ActionOnUnit(unitId, cmdId, targetId, synchro);
	return 0;
}

FREObject FRE_PP_Unit_UntargetedAction(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	int cmdId;
	FREGetObjectAsInt32(argv[1], &cmdId);
	double param;
	FREGetObjectAsDouble(argv[2], &param);
	int synchro;
	FREGetObjectAsInt32(argv[3], &synchro);
	// Appel Prog&Play
	PP_Unit_UntargetedAction(unitId, cmdId, param, synchro);
	return 0;
}

FREObject FRE_PP_Unit_UntargetedActionWithoutParam(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération des paramètres
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	int cmdId;
	FREGetObjectAsInt32(argv[1], &cmdId);
	int synchro;
	FREGetObjectAsInt32(argv[3], &synchro);
	// Appel Prog&Play pour la valeur de retour 
	int ret = PP_Unit_UntargetedAction_prim (unitId, cmdId, -1.0);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_UntargetedActionWithoutParam " << unitId;
			int unitType = PP_Unit_GetType_prim(unitId);
			if (unitType >= 0)
				oss << "_" << unitType;
			oss << " " << cmdId;
			// normalize synchronized
			if (synchro == 0)
				oss << " 0";
			else
				oss << " 1";
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
		if (synchro != 0){
			// waiting that the order is adding into pending commands. We wait approximately
			// 1000ms. We know that for each 8 entrance in critical section Prog&Play makes a
			// a pause (1ms) to avoid cpu consuming. In orderWithOneParamFound function only 1 call
			// of Prog&Play was done and so 8000 calls of this function will consume 1000ms in the
			// better case. 
			int cpt = 0;
			while (!orderWithOneParamFound(unitId, cmdId, -1.0) && cpt < 8000)
				cpt++;
			if (cpt < 8000){
				// waiting that the order is over pending commands
				while (orderWithOneParamFound(unitId, cmdId, -1.0));
			}
		}
	}
	return 0;
}

FREObject FRE_PP_GetError(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Appel Prog&Play pour la valeur de retour 
	char * error = PP_GetError();
	int i;
	for (i = 0 ; error[i] != '\0' ; i++);
	FREObject value;
	FRENewObjectFromUTF8(i, (const uint8_t*)error, &value);
	return value;
}

FREObject FRE_PP_ClearError(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	PP_ClearError();
	return 0;
}

#ifdef __cplusplus
}
#endif