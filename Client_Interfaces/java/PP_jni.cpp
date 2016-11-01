
#include "PP_jni.h"
#include "PP_Client.h"
#include "PP_Client_Private.h"
#include "PP_Error.h"

#include <iostream>

JNIEXPORT jint JNICALL Java_pp_PPNative_Open
	(JNIEnv *, jclass){
	return PP_Open();
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Close
	(JNIEnv *, jclass){
	return PP_Close();
}

JNIEXPORT jint JNICALL Java_pp_PPNative_IsGameOver
	(JNIEnv *, jclass){
	return PP_IsGameOver();
}

JNIEXPORT jfloatArray JNICALL Java_pp_PPNative_GetMapSize
	(JNIEnv * env, jclass){
	//allocation du tableau à renvoyer en Java
	jfloatArray result = env->NewFloatArray(2);
	// vérification de l'allocation
	if (result == NULL){
		std::cerr << "(JNI) PP_MapSize : memory allocation error" << std::endl;
		return NULL;
	}
	// transformation de la position en un tableau pour java
	float tmp [2];
	PP_Pos pos = PP_GetMapSize();
	tmp[0] = pos.x;
	tmp[1] = pos.y;
	// recopie dans le tableau Java
	env->SetFloatArrayRegion(result, 0, 2, tmp);
	return result;
}

JNIEXPORT jfloatArray JNICALL Java_pp_PPNative_GetStartPosition
	(JNIEnv * env, jclass){
	//allocation du tableau à renvoyer en Java
	jfloatArray result = env->NewFloatArray(2);
	// vérification de l'allocation
	if (result == NULL){
		std::cerr << "(JNI) PP_StartPosition : memory allocation error"
							<< std::endl;
		return NULL;
	}
	// transformation de la position en un tableau pour java
	float tmp [2];
	PP_Pos pos = PP_GetStartPosition();
	tmp[0] = pos.x;
	tmp[1] = pos.y;
	// recopie dans le tableau Java
	env->SetFloatArrayRegion(result, 0, 2, tmp);
	return result;
}

JNIEXPORT jint JNICALL Java_pp_PPNative_GetNumSpecialAreas (JNIEnv *, jclass){
	return PP_GetNumSpecialAreas();
}

JNIEXPORT jfloatArray JNICALL Java_pp_PPNative_GetSpecialAreaPosition
	(JNIEnv * env, jclass, jint n){
	// allocation du tableau à renvoyer en Java
	jfloatArray result = env->NewFloatArray(2);
	// vérification de l'allocation
	if (result == NULL){
		std::cerr << "(JNI) PP_GetSpecialAreaPosition : memory allocation error"
							<< std::endl;
		return NULL;
	}
	// transformation de la position en un tableau pour java
	float tmp [2];
	PP_Pos pos = PP_GetSpecialAreaPosition(n);
	tmp[0] = pos.x;
	tmp[1] = pos.y;
	// recopie dans le tableau Java
	env->SetFloatArrayRegion(result, 0, 2, tmp);
	return result;

}

JNIEXPORT jint JNICALL Java_pp_PPNative_GetResource
	(JNIEnv *, jclass, jint id){
	return PP_GetResource (id);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_GetNumUnits
	(JNIEnv *, jclass, jint c){
	return PP_GetNumUnits ((PP_Coalition)c);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_GetUnitAt
	(JNIEnv *, jclass, jint c, jint index){
	return PP_GetUnitAt ((PP_Coalition)c, index);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1GetCoalition
	(JNIEnv *, jclass, jint unit){
	return PP_Unit_GetCoalition (unit);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1GetType
	(JNIEnv *, jclass, jint unit){
	return PP_Unit_GetType (unit);
}

JNIEXPORT jfloatArray JNICALL Java_pp_PPNative_Unit_1GetPosition
	(JNIEnv * env, jclass, jint unit){
	// allocation du tableau à renvoyer en Java
	jfloatArray result = env->NewFloatArray(2);
	// vérification de l'allocation
	if (result == NULL){
		std::cerr << "(JNI) PP_Unit_GetPosition : memory allocation error"
							<< std::endl;
		return NULL;
	}
	// transformation de la position en un tableau pour java
	float tmp [2];
	PP_Pos pos = PP_Unit_GetPosition(unit);
	tmp[0] = pos.x;
	tmp[1] = pos.y;
	// recopie dans le tableau Java
	env->SetFloatArrayRegion(result, 0, 2, tmp);
	return result;
}

JNIEXPORT jfloat JNICALL Java_pp_PPNative_Unit_1GetHealth
	(JNIEnv *, jclass, jint unit){
	return PP_Unit_GetHealth (unit);
}

JNIEXPORT jfloat JNICALL Java_pp_PPNative_Unit_1GetMaxHealth
	(JNIEnv *, jclass, jint unit){
	return PP_Unit_GetMaxHealth (unit);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1GetGroup
	(JNIEnv *, jclass, jint unit){
	return PP_Unit_GetGroup (unit);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1SetGroup
	(JNIEnv *, jclass, jint unit, jint group){
	return PP_Unit_SetGroup (unit, group);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1GetNumPdgCmds
	(JNIEnv *, jclass, jint unit){
	return PP_Unit_GetNumPdgCmds(unit);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1PdgCmd_1GetCode
	(JNIEnv *, jclass, jint unit, jint index){
	return PP_Unit_PdgCmd_GetCode(unit, index);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1PdgCmd_1GetNumParams
  (JNIEnv *, jclass, jint unit, jint idCmd){
	return PP_Unit_PdgCmd_GetNumParams(unit, idCmd);
}

JNIEXPORT jfloat JNICALL Java_pp_PPNative_Unit_1PdgCmd_1GetParam
  (JNIEnv *, jclass, jint unit, jint idCmd, jint idParam){
	return PP_Unit_PdgCmd_GetParam(unit, idCmd, idParam);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1ActionOnUnit
	(JNIEnv *, jclass, jint unit, jint command, jint target){
	return PP_Unit_ActionOnUnit(unit, command, target);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1ActionOnPosition
	(JNIEnv *, jclass, jint unit, jint action, jfloat x, jfloat y){
	PP_Pos pos;
	pos.x = x;
	pos.y = y;
	return PP_Unit_ActionOnPosition (unit, action, pos);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1UntargetedAction
	(JNIEnv *, jclass, jint unit, jint action, jfloat param){
	return PP_Unit_UntargetedAction(unit, action, param);
}

JNIEXPORT jstring JNICALL Java_pp_PPNative_GetError
  (JNIEnv * env, jclass){
	return env->NewStringUTF(PP_GetError());
}

JNIEXPORT void JNICALL Java_pp_PPNative_ClearError
  (JNIEnv *, jclass){
	PP_ClearError();
}

JNIEXPORT void JNICALL Java_pp_PPNative_EnterCriticalSection
  (JNIEnv *, jclass){
    enterCriticalSection();
}

JNIEXPORT void JNICALL Java_pp_PPNative_ExitCriticalSection
  (JNIEnv *, jclass){
    exitCriticalSection();
}
