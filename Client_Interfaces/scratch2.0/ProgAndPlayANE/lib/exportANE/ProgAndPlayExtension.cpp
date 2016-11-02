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
FREObject FRE_PP_MapWidth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_MapHeight(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_StartPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_StartPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_NumSpecialArea(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_SpecialAreaPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_SpecialAreaPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_Resource(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_NumUnits(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);
FREObject FRE_PP_UnitAt(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]);

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
		{ (const uint8_t*) "PP_Open_wrapper",        0, &FRE_PP_Open },
		{ (const uint8_t*) "PP_Close_wrapper",        0, &FRE_PP_Close },
		{ (const uint8_t*) "PP_IsGameOver_wrapper",        0, &FRE_PP_IsGameOver },
		{ (const uint8_t*) "PP_MapWidth_wrapper",        0, &FRE_PP_MapWidth },
		{ (const uint8_t*) "PP_MapHeight_wrapper",        0, &FRE_PP_MapHeight },
		{ (const uint8_t*) "PP_StartPosX_wrapper",        0, &FRE_PP_StartPosX },
		{ (const uint8_t*) "PP_StartPosY_wrapper",        0, &FRE_PP_StartPosY },
		{ (const uint8_t*) "PP_NumSpecialArea_wrapper",        0, &FRE_PP_NumSpecialArea },
		{ (const uint8_t*) "PP_SpecialAreaPosX_wrapper",        0, &FRE_PP_SpecialAreaPosX },
		{ (const uint8_t*) "PP_SpecialAreaPosY_wrapper",        0, &FRE_PP_SpecialAreaPosY },
		{ (const uint8_t*) "PP_Resource_wrapper",        0, &FRE_PP_Resource },
		{ (const uint8_t*) "PP_NumUnits_wrapper",        0, &FRE_PP_NumUnits },
		{ (const uint8_t*) "PP_UnitAt_wrapper",        0, &FRE_PP_UnitAt }
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

FREObject FRE_PP_MapWidth(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromInt32(PP_GetMapSize().x, &value);
	return value;
}

FREObject FRE_PP_MapHeight(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromInt32(PP_GetMapSize().y, &value);
	return value;
}

FREObject FRE_PP_StartPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromDouble(PP_GetStartPosition().x, &value);
	return value;
}

FREObject FRE_PP_StartPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromDouble(PP_GetStartPosition().y, &value);
	return value;
}

FREObject FRE_PP_NumSpecialArea(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	FREObject value;
	FRENewObjectFromInt32(PP_GetNumSpecialAreas(), &value);
	return value;
}

FREObject FRE_PP_SpecialAreaPosX(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int num;
	FREGetObjectAsInt32(argv[0], &num);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromDouble(PP_GetSpecialAreaPosition(num).x, &value);
	return value;
}

FREObject FRE_PP_SpecialAreaPosY(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int num;
	FREGetObjectAsInt32(argv[0], &num);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromDouble(PP_GetSpecialAreaPosition(num).y, &value);
	return value;
}

FREObject FRE_PP_Resource(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int num;
	FREGetObjectAsInt32(argv[0], &num);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromInt32(PP_GetResource(num), &value);
	return value;
}

FREObject FRE_PP_NumUnits(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
	// Récupération du paramètre
	int coalition;
	FREGetObjectAsInt32(argv[0], &coalition);
	// Appel Prog&Play pour la valeur de retour 
	FREObject value;
	FRENewObjectFromInt32(PP_GetNumUnits((PP_Coalition)coalition), &value);
	return value;
}

FREObject FRE_PP_UnitAt(FREContext ctx, void* functionData, uint32_t argc, FREObject argv[]){
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

#ifdef __cplusplus
}
#endif