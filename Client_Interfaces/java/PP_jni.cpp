
#include "PP_jni.h"
#include "PP_Client.h"
#include "PP_Client_Private.h"
#include "traces/TraceConstantList.h"
#include "PP_Error.h"

#include <sstream>
#include <iostream>

JNIEXPORT jint JNICALL Java_pp_PPNative_Open
	(JNIEnv *, jclass){
	if (PP_Open_prim() == 0){
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss_start(std::ostringstream::out);
			oss_start << EXECUTION_START_TIME << " " << PP_GetTimestamp_prim();
			PP_PushMessage_prim(oss_start.str().c_str(), NULL);
			std::ostringstream oss_lang(std::ostringstream::out);
			oss_lang << PROGRAMMING_LANGUAGE_USED << " Java";
			PP_PushMessage_prim(oss_lang.str().c_str(), NULL);
			PP_PushMessage_prim("PP_Open", NULL);
		exitCriticalSection();
		return 0;
	} else
		return -1;
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Close
	(JNIEnv *, jclass){
	return PP_Close();
}

JNIEXPORT jint JNICALL Java_pp_PPNative_IsGameOver
	(JNIEnv *, jclass){
	return PP_IsGameOver();
}

JNIEXPORT jint JNICALL Java_pp_PPNative_IsGamePaused
	(JNIEnv *, jclass){
	return PP_IsGamePaused();
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

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1RemoveFromGroup
	(JNIEnv *, jclass, jint unit){
	int ret = PP_Unit_SetGroup_prim (unit, -1);
	if (ret > FEEDBACK_COUNT_LIMIT) {
		// notify function call to Spring
		enterCriticalSection();
			std::ostringstream oss(std::ostringstream::out);
			oss << "PP_Unit_RemoveFromGroup " << unit;
			int type = PP_Unit_GetType_prim(unit);
			if (type >= 0)
				oss << "_" << type;
			PP_PushMessage_prim(oss.str().c_str(), &ret);
		exitCriticalSection();
	}
	if (ret < 0)
		ret = -1;
	return ret;
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1GetNumPdgCmds_1prim
	(JNIEnv *, jclass, jint unit){
	return PP_Unit_GetNumPdgCmds_prim(unit);
}

JNIEXPORT jintArray JNICALL Java_pp_PPNative_Unit_1PdgCmd_1GetCode_1prim
	(JNIEnv * env, jclass, jint unit, jint index){
	// allocation du tableau à renvoyer en Java
	jintArray result = env->NewIntArray(2);
	// vérification de l'allocation
	if (result == NULL){
		std::cerr << "(JNI) Unit_PdgCmd_GetCode_prim : memory allocation error"
							<< std::endl;
		return NULL;
	}
	// Initialisation du code de retour et du code de la commande
	int ret, cmdCode;
	ret = PP_Unit_PdgCmd_GetCode_prim(unit, index, &cmdCode);
	// construction du tableau en C
	jint tmp[2];
	tmp[0] = ret;
	tmp[1] = cmdCode;
	// recopie dans le tableau Java
	env->SetIntArrayRegion(result, 0, 2, tmp);
	return result;
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1PdgCmd_1GetNumParams_1prim
  (JNIEnv *, jclass, jint unit, jint idCmd){
	return PP_Unit_PdgCmd_GetNumParams_prim(unit, idCmd);
}

JNIEXPORT jfloatArray JNICALL Java_pp_PPNative_Unit_1PdgCmd_1GetParam_1prim
  (JNIEnv * env, jclass, jint unit, jint idCmd, jint idParam){
	// allocation du tableau à renvoyer en Java
	jfloatArray result = env->NewFloatArray(2);
	// vérification de l'allocation
	if (result == NULL){
		std::cerr << "(JNI) Unit_PdgCmd_GetParam_prim : memory allocation error"
							<< std::endl;
		return NULL;
	}
	// Initialisation du code de retour et de la valeur du paramètre de la commande
	float ret, paramValue;
	ret = (float)PP_Unit_PdgCmd_GetParam_prim(unit, idCmd, idParam, &paramValue);
	// construction du tableau en C
	float tmp[2];
	tmp[0] = ret;
	tmp[1] = paramValue;
	// recopie dans le tableau Java
	env->SetFloatArrayRegion(result, 0, 2, tmp);
	return result;
}

JNIEXPORT void JNICALL Java_pp_PPNative_pushMessageForGetPendingCommands
  (JNIEnv *, jclass, jint unit, jint errorCode){
	if (errorCode > FEEDBACK_COUNT_LIMIT) {
		int error = errorCode;
		// notify function call to Spring
		std::ostringstream oss(std::ostringstream::out);
		oss << "PP_Unit_GetPendingCommands " << unit;			
		int type = PP_Unit_GetType_prim (unit);
		if (type >= 0)
			oss << "_" << type;
		PP_PushMessage_prim(oss.str().c_str(), &error);
	}
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1ActionOnUnit
	(JNIEnv *, jclass, jint unit, jint command, jint target, jboolean locked){
	return PP_Unit_ActionOnUnit(unit, command, target, (int)locked);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1ActionOnPosition
	(JNIEnv *, jclass, jint unit, jint action, jfloat x, jfloat y, jboolean locked){
	PP_Pos pos;
	pos.x = x;
	pos.y = y;
	return PP_Unit_ActionOnPosition (unit, action, pos, (int)locked);
}

JNIEXPORT jint JNICALL Java_pp_PPNative_Unit_1UntargetedAction
	(JNIEnv *, jclass, jint unit, jint action, jfloat param, jboolean locked){
	return PP_Unit_UntargetedAction(unit, action, param, (int)locked);
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
