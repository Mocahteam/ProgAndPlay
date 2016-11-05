#include "FlashRuntimeExtensions.h"
#include "PP_Client.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

void ProgAndPlayContextInitializer(void* extData, const uint8_t* ctxType, FREContext ctx, uint32_t* numFunctionsToSet, const FRENamedFunction** functionsToSet);
void ProgAndPlayContextFinalizer(FREContext ctx);

FREObject FRE_PP_Open(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Close(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);

FREObject FRE_PP_IsGameOver(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
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
FREObject FRE_PP_Unit_GetCoalition(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetType(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetPositionX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Unit_GetPositionY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);

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
		{ (const uint8_t*) "PP_Open_wrapper",					0, &FRE_PP_Open },
		{ (const uint8_t*) "PP_Close_wrapper",					0, &FRE_PP_Close },
		{ (const uint8_t*) "PP_IsGameOver_wrapper",				0, &FRE_PP_IsGameOver },
		{ (const uint8_t*) "PP_GetMapWidth_wrapper",			0, &FRE_PP_GetMapWidth },
		{ (const uint8_t*) "PP_GetMapHeight_wrapper",			0, &FRE_PP_GetMapHeight },
		{ (const uint8_t*) "PP_GetStartPosX_wrapper",			0, &FRE_PP_GetStartPosX },
		{ (const uint8_t*) "PP_GetStartPosY_wrapper",			0, &FRE_PP_GetStartPosY },
		{ (const uint8_t*) "PP_GetNumSpecialArea_wrapper",		0, &FRE_PP_GetNumSpecialArea },
		{ (const uint8_t*) "PP_GetSpecialAreaPosX_wrapper",		0, &FRE_PP_GetSpecialAreaPosX },
		{ (const uint8_t*) "PP_GetSpecialAreaPosY_wrapper",		0, &FRE_PP_GetSpecialAreaPosY },
		{ (const uint8_t*) "PP_GetResource_wrapper",			0, &FRE_PP_GetResource },
		{ (const uint8_t*) "PP_GetNumUnits_wrapper",			0, &FRE_PP_GetNumUnits },
		{ (const uint8_t*) "PP_GetUnitAt_wrapper",				0, &FRE_PP_GetUnitAt },
		{ (const uint8_t*) "PP_Unit_GetCoalition_wrapper",		0, &FRE_PP_Unit_GetCoalition },
		{ (const uint8_t*) "PP_Unit_GetType_wrapper",			0, &FRE_PP_Unit_GetType },
		{ (const uint8_t*) "PP_Unit_GetPositionX_wrapper",		0, &FRE_PP_Unit_GetPositionX },
		{ (const uint8_t*) "PP_Unit_GetPositionY_wrapper",		0, &FRE_PP_Unit_GetPositionY }
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
	PP_Open();
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

FREObject FRE_PP_GetMapWidth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromInt32(PP_GetMapSize().x, &value);
	return value;
}

FREObject FRE_PP_GetMapHeight(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromInt32(PP_GetMapSize().y, &value);
	return value;
}

FREObject FRE_PP_GetStartPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromDouble(PP_GetStartPosition().x, &value);
	return value;
}

FREObject FRE_PP_GetStartPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromDouble(PP_GetStartPosition().y, &value);
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
	FRENewObjectFromDouble(PP_GetSpecialAreaPosition(num).x, &value);
	return value;
}

FREObject FRE_PP_GetSpecialAreaPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int num;
	FREGetObjectAsInt32(argv[0], &num);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromDouble(PP_GetSpecialAreaPosition(num).y, &value);
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
	int coalition;
	FREGetObjectAsInt32(argv[0], &coalition);
	int id;
	FREGetObjectAsInt32(argv[1], &id);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromInt32(PP_GetUnitAt((PP_Coalition)coalition, id), &value);
	return value;
	
}

FREObject FRE_PP_Unit_GetCoalition(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromInt32((int)PP_Unit_GetCoalition(unitId), &value);
	return value;
}

FREObject FRE_PP_Unit_GetType(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromInt32((int)PP_Unit_GetType(unitId), &value);
	return value;
}

FREObject FRE_PP_Unit_GetPositionX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromDouble(PP_Unit_GetPosition(unitId).x, &value);
	return value;
}

FREObject FRE_PP_Unit_GetPositionY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int unitId;
	FREGetObjectAsInt32(argv[0], &unitId);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromDouble(PP_Unit_GetPosition(unitId).y, &value);
	return value;
}

#ifdef __cplusplus
}
#endif