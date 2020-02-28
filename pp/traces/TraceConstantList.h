/**
 * \file TraceConstantList.h
 * \brief Déclaration des constantes utiles pour le codage et le décodage des traces
 * \author muratet
 * \version 0.1
 */

#ifndef __TRACE_CONSTANT_LIST_H__
#define __TRACE_CONSTANT_LIST_H__

#define DEBUG_PARSER
#define LOG_IN_FILE

// Events
#define GAME_START "start"                                    // set by game engine (ProgAndPlay.cpp)
#define MISSION_START_TIME "mission_start_time"               // set by game engine (ProgAndPlay.cpp)
#define EXECUTION_START_TIME "execution_start_time"           // set by client interfaces
#define PROGRAMMING_LANGUAGE_USED "programming_language_used" // set by client interfaces
#define EXECUTION_END_TIME "execution_end_time"               // set by client interfaces
#define MISSION_END_TIME "mission_end_time"                   // set by game engine (ProgAndPlay.cpp)
#define GAME_END "end"                                        // set by game engine (ProgAndPlay.cpp)

#define GAME_PAUSED "game_paused"     // set by game engine (ProgAndPlay.cpp)
#define GAME_UNPAUSED "game_unpaused" // set by game engine (ProgAndPlay.cpp)
#define DELAYED "delayed"             // set by game engine (ProgAndPlay.cpp)

// Call types
#define CALL_WITH_NO_PARAMS "CallWithNoParam"
#define CALL_WITH_INT_PARAM "CallWithIntParam"
#define CALL_WITH_INT_INT_PARAMS "CallWithIntIntParams"
#define CALL_WITH_INT_UNIT_PARAMS "CallWithIntUnitParams"
#define CALL_WITH_INT_INT_UNIT_PARAMS "CallWithIntIntUnitParams"
#define CALL_WITH_INT_UNIT_INT_PARAMS "CallWithIntUnitIntParams"
#define CALL_WITH_UNIT_PARAM "CallWithUnitParam"
#define CALL_WITH_UNIT_INT_PARAMS "CallWithUnitIntParams"
#define CALL_WITH_UNIT_INT_INT_PARAMS "CallWithUnitIntIntParams"
#define CALL_WITH_UNIT_INT_UNIT_INT_PARAMS "CallWithUnitIntUnitIntParams"
#define CALL_WITH_UNIT_INT_POS_INT_PARAMS "CallWithUnitIntPosIntParams"
#define CALL_WITH_UNIT_INT_FLOAT_INT_PARAMS "CallWithUnitIntFloatIntParams"

// Error labels
#define UNIT_NOT_FOUND_LABEL "unit_not_found"
#define NOT_UNIT_OWNER_LABEL "not_unit_owner"
#define INVALID_COALITION_LABEL "invalid_coalition"
#define TARGET_NOT_FOUND_LABEL "target_not_found"
#define OUT_OF_RANGE_LABEL "out_of_range"
#define INVALID_GROUP_LABEL "invalid_group"
#define POSITION_OUT_OF_BOUNDS_LABEL "position_out_of_bounds"

#endif
