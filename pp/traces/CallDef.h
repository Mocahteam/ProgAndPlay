#ifndef __CALL_DEF_H__
#define __CALL_DEF_H__

#include <cmath>
#include <boost/lexical_cast.hpp>

#include "Call.h"
#include "TracesAnalyser.h"

namespace CallMisc {

	enum Coalition {
		NONE = -1,
		MY_COALITION,
		ALLY_COALITION,
		ENEMY_COALITION
	};

	struct Unit {
		int id;
		int type;
		
		bool operator!=(const Unit& u) const {
			return id != u.id;
		}
	};

	struct Position {
		float x;
		float y;
		
		bool operator!=(const Position& p) const {
			return x != p.x || y != p.y;
		}
	};

}

class NoParamCall : public Call {
	
public:
		
	NoParamCall(std::string label): Call(label,Call::NONE) {}
	
	NoParamCall(const NoParamCall *c) : Call(c) {}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<NoParamCall>(this);
	}
	
private:

	virtual bool compare(const Call *c) const {
		return true;
	}
	
	virtual void filter(const Call *c) {}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		return std::make_pair<int,int>(0,0);
	}
	
	virtual std::string getParams() const {
		return "";
	}
	
	virtual std::string getReadableParams() const {
		return "";
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		return ids;
	}
	
};

class GetSpecialAreaPositionCall : public Call {

public:

	GetSpecialAreaPositionCall(ErrorType error, int specialAreaId): Call("PP_GetSpecialAreaPosition",error), specialAreaId(specialAreaId) {}
	
	GetSpecialAreaPositionCall(const GetSpecialAreaPositionCall *c) : Call(c) {
		specialAreaId = c->specialAreaId;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<GetSpecialAreaPositionCall>(this);
	}
	
private:
	
	int specialAreaId;
	
	virtual bool compare(const Call *c) const {
		const GetSpecialAreaPositionCall *cc = dynamic_cast<const GetSpecialAreaPositionCall*>(c);
		if (Call::paramsMap.contains(label,"specialAreaId") && specialAreaId != cc->specialAreaId)
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const GetSpecialAreaPositionCall *cc = dynamic_cast<const GetSpecialAreaPositionCall*>(c);
		if (!Call::paramsMap.contains(label,"specialAreaId") && specialAreaId != cc->specialAreaId && specialAreaId != -1)
			specialAreaId = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const GetSpecialAreaPositionCall *cc = dynamic_cast<const GetSpecialAreaPositionCall*>(c);
		if (specialAreaId == cc->specialAreaId)
			return std::make_pair<int,int>(0,1);
		return std::make_pair<int,int>(1,1);
	}
	
	virtual std::string getParams() const {
		if (specialAreaId == -1)
			return "?";
		return boost::lexical_cast<std::string>(specialAreaId);
	}
	
	virtual std::string getReadableParams() const {
		return "(" + ((specialAreaId != -1) ? boost::lexical_cast<std::string>(specialAreaId) : "_") + ")";
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		GetSpecialAreaPositionCall *cc;
		if (c != NULL)
			cc = dynamic_cast<GetSpecialAreaPositionCall*>(c);
		if (error == Call::OUT_OF_RANGE || (c != NULL && specialAreaId != cc->specialAreaId))
			ids.push_back("specialAreaId");
		return ids;
	}
	
};

class GetResourceCall : public Call {
	
public:

	GetResourceCall(ErrorType error, int resourceId): Call("PP_GetResource",error), resourceId(resourceId) {}
	
	GetResourceCall(const GetResourceCall *c) : Call(c) {
		resourceId = c->resourceId;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<GetResourceCall>(this);
	}
	
private:
	
	int resourceId;
	
	virtual bool compare(const Call *c) const {
		const GetResourceCall *cc = dynamic_cast<const GetResourceCall*>(c);
		if (Call::paramsMap.contains(label,"resourceId") && resourceId != cc->resourceId)
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const GetResourceCall *cc = dynamic_cast<const GetResourceCall*>(c);
		if (!Call::paramsMap.contains(label,"resourceId") && resourceId != cc->resourceId && resourceId != -1)
			resourceId = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const GetResourceCall *cc = dynamic_cast<const GetResourceCall*>(c);
		if (resourceId == cc->resourceId)
			return std::make_pair<int,int>(0,1);
		return std::make_pair<int,int>(1,1);
	}
	
	virtual std::string getParams() const {
		if (resourceId == -1)
			return "?";
		return boost::lexical_cast<std::string>(resourceId);
	}
	
	virtual std::string getReadableParams() const {
		return "(" + ((TracesAnalyser::resources_map.find(resourceId) != TracesAnalyser::resources_map.end()) ? TracesAnalyser::resources_map.at(resourceId) : "_") + ")";
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		GetResourceCall *cc;
		if (c != NULL)
			cc = dynamic_cast<GetResourceCall*>(c);
		if (error == Call::OUT_OF_RANGE || (c != NULL && resourceId != cc->resourceId))
			ids.push_back("resourceId");
		return ids;
	}
	
};

class GetNumUnitsCall : public Call {

public:

	GetNumUnitsCall(ErrorType error, CallMisc::Coalition coalition): Call("PP_GetNumUnits",error), coalition(coalition) {}
	
	GetNumUnitsCall(const GetNumUnitsCall *c) : Call(c) {
		coalition = c->coalition;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<GetNumUnitsCall>(this);
	}
	
private:

	CallMisc::Coalition coalition;
	
	virtual bool compare(const Call *c) const {
		const GetNumUnitsCall *cc = dynamic_cast<const GetNumUnitsCall*>(c);
		if (Call::paramsMap.contains(label,"coalition") && coalition != cc->coalition)
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const GetNumUnitsCall *cc = dynamic_cast<const GetNumUnitsCall*>(c);
		if (!Call::paramsMap.contains(label,"coalition") && coalition != cc->coalition && coalition != CallMisc::NONE)
			coalition = CallMisc::NONE;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const GetNumUnitsCall *cc = dynamic_cast<const GetNumUnitsCall*>(c);
		if (coalition == cc->coalition)
			return std::make_pair<int,int>(0,1);
		return std::make_pair<int,int>(1,1);
	}
	
	virtual std::string getParams() const {
		if (coalition == CallMisc::NONE)
			return "?";
		return boost::lexical_cast<std::string>(static_cast<int>(coalition));
	}
	
	virtual std::string getReadableParams() const {
		return "(" + ((error == Call::NONE && coalition != CallMisc::NONE) ? std::string(Call::getEnumLabel<CallMisc::Coalition>(coalition,Call::coalitionsArr)) : "_") + ")";
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		GetNumUnitsCall *cc;
		if (c != NULL)
			cc = dynamic_cast<GetNumUnitsCall*>(c);
		if (error == Call::WRONG_COALITION || (c != NULL && coalition != cc->coalition))
			ids.push_back("coalition");
		return ids;
	}

};

class GetUnitAtCall : public Call {
	
public:

	GetUnitAtCall(ErrorType error, CallMisc::Coalition coalition, int index): Call("PP_GetUnitAt",error), coalition(coalition), index(index) {}
	
	GetUnitAtCall(const GetUnitAtCall *c) : Call(c) {
		coalition = c->coalition;
		index = c->index;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<GetUnitAtCall>(this);
	}
	
private:

	CallMisc::Coalition coalition;
	int index;
	
	virtual bool compare(const Call *c) const {
		const GetUnitAtCall *cc = dynamic_cast<const GetUnitAtCall*>(c);
		if ((Call::paramsMap.contains(label,"coalition") && coalition != cc->coalition) || (Call::paramsMap.contains(label,"index") && index != cc->index))
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const GetUnitAtCall *cc = dynamic_cast<const GetUnitAtCall*>(c);
		if (!Call::paramsMap.contains(label,"coalition") && coalition != cc->coalition && coalition != CallMisc::NONE)
			coalition = CallMisc::NONE;
		if (!Call::paramsMap.contains(label,"index") && index != cc->index && index != -1)
			index = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const GetUnitAtCall *cc = dynamic_cast<const GetUnitAtCall*>(c);
		int sc = 0;
		if (coalition != cc->coalition)
			sc++;
		if (index != cc->index)
			sc++;
		return std::make_pair<int,int>(sc,2);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (coalition == CallMisc::NONE) ? "?" : boost::lexical_cast<std::string>(static_cast<int>(coalition));
		s += " ";
		s += (index == -1) ? "?" : boost::lexical_cast<std::string>(index);
		return s;
	}
	
	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (error != Call::WRONG_COALITION && coalition != CallMisc::NONE) ? Call::getEnumLabel<CallMisc::Coalition>(coalition,Call::coalitionsArr) : "_";
		s += ",";
		s += (index != -1) ? boost::lexical_cast<std::string>(index) : "_";
		s += ")";
		return s;
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		GetUnitAtCall *cc;
		if (c != NULL)
			cc = dynamic_cast<GetUnitAtCall*>(c);
		if (c != NULL && coalition != cc->coalition)
			ids.push_back("coalition");
		if (error == Call::OUT_OF_RANGE || (c != NULL && index != cc->index))
			ids.push_back("index");
		return ids;
	}
	
};

class UnitCall : public Call {

public:

	UnitCall(ErrorType error, std::string label, int unitId, int unitType): Call(label,error) {
		unit.id = unitId;
		unit.type = unitType;
	}
	
	UnitCall(const UnitCall *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<UnitCall>(this);
	}
	
private:

	CallMisc::Unit unit;
	
	virtual bool compare(const Call *c) const {
		const UnitCall *cc = dynamic_cast<const UnitCall*>(c);
		if ((Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id) || (Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type))
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const UnitCall *cc = dynamic_cast<const UnitCall*>(c);
		if (!Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const UnitCall *cc = dynamic_cast<const UnitCall*>(c);
		if (unit.type == cc->unit.type)
			return std::make_pair<int,int>(0,1);
		return std::make_pair<int,int>(1,1);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		return s;		
	}
	
	virtual std::string getReadableParams() const {
		return "(" + ((TracesAnalyser::units_id_map.find(unit.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(unit.type) : "_") + ")";
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		UnitCall *cc;
		if (c != NULL)
			cc = dynamic_cast<UnitCall*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		return ids;
	}

};

class SetGroupCall : public Call {

public:

	SetGroupCall(ErrorType error, int unitId, int unitType, int groupId): Call("PP_Unit_SetGroup",error), groupId(groupId) {
		unit.id = unitId;
		unit.type = unitType;
	}
	
	SetGroupCall(const SetGroupCall *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		groupId = c->groupId;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<SetGroupCall>(this);
	}
	
private:

	CallMisc::Unit unit;
	int groupId;
	
	virtual bool compare(const Call *c) const {
		const SetGroupCall *cc = dynamic_cast<const SetGroupCall*>(c);
		if ((Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id) || (Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type) || (Call::paramsMap.contains(label,"groupId") && groupId != cc->groupId))
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const SetGroupCall *cc = dynamic_cast<const SetGroupCall*>(c);
		if (!Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::paramsMap.contains(label,"groupId") && groupId != cc->groupId && groupId != -1)
			groupId = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const SetGroupCall *cc = dynamic_cast<const SetGroupCall*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (groupId != cc->groupId)
			sc++;
		return std::make_pair<int,int>(sc,2);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (groupId == -1) ? "?" : boost::lexical_cast<std::string>(groupId);
		return s;
	}
	
	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (TracesAnalyser::units_id_map.find(unit.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(unit.type) : "_";
		s += ",";
		s += (groupId != -1) ? boost::lexical_cast<std::string>(groupId) : "_";
		s += ")";
		return s;
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		SetGroupCall *cc;
		if (c != NULL)
			cc = dynamic_cast<SetGroupCall*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		if (error == Call::OUT_OF_RANGE || (c != NULL && groupId != cc->groupId))
			ids.push_back("groupId");
		return ids;
	}
	
};

class ActionOnUnitCall : public Call {
	
public:
		
	ActionOnUnitCall(ErrorType error, int unitId, int unitType, int action, int targetId, int targetType): Call("PP_Unit_ActionOnUnit",error), action(action) {
		unit.id = unitId;
		unit.type = unitType;
		target.id = targetId;
		target.type = targetType;
	}
	
	ActionOnUnitCall(const ActionOnUnitCall *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		action = c->action;
		target.id = c->target.id;
		target.type = c->target.type;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<ActionOnUnitCall>(this);
	}
	
private:
		
	CallMisc::Unit unit;
	int action;
	CallMisc::Unit target;
	
	virtual bool compare(const Call *c) const {
		const ActionOnUnitCall *cc = dynamic_cast<const ActionOnUnitCall*>(c);
		if ((Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id) || (Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type) || (Call::paramsMap.contains(label,"action") && action != cc->action) || (Call::paramsMap.contains(label,"targetId") && target.id != cc->target.id) || (Call::paramsMap.contains(label,"targetType") && target.type != cc->target.type))
			return false;
		return true;	
	}
	
	virtual void filter(const Call *c) {
		const ActionOnUnitCall *cc = dynamic_cast<const ActionOnUnitCall*>(c);
		if (!Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::paramsMap.contains(label,"action") && action != cc->action && action != -1)
			action = -1;
		if (!Call::paramsMap.contains(label,"targetId") && target.id != cc->target.id && target.id != -1)
			target.id = -1;
		if (!Call::paramsMap.contains(label,"targetType") && target.type != cc->target.type && target.type != -1)
			target.type = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const ActionOnUnitCall *cc = dynamic_cast<const ActionOnUnitCall*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (action != cc->action)
			sc++;
		if (target.type != cc->target.type)
			sc++;
		return std::make_pair<int,int>(sc,3);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (action == -1) ? "?" : boost::lexical_cast<std::string>(action);
		s += " ";
		s += (target.id == -1) ? "?" : boost::lexical_cast<std::string>(target.id);
		s += "_";
		s += (target.type == -1) ? "?" : boost::lexical_cast<std::string>(target.type);
		return s;
	}
	
	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (TracesAnalyser::units_id_map.find(unit.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(unit.type) : "_";
		s += ",";
		s += (TracesAnalyser::orders_map.find(action) != TracesAnalyser::orders_map.end()) ? TracesAnalyser::orders_map.at(action) : "_";
		s += ",";
		s += (TracesAnalyser::units_id_map.find(target.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(target.type) : "_";
		s += ")";
		return s;
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		ActionOnUnitCall *cc;
		if (c != NULL)
			cc = dynamic_cast<ActionOnUnitCall*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		if (c != NULL && action != cc->action)
			ids.push_back("action");
		if (c != NULL && target.type != cc->target.type)
			ids.push_back("targetType");
		return ids;
	}

};

class ActionOnPositionCall : public Call {

public:
		
	ActionOnPositionCall(ErrorType error, int unitId, int unitType, int action, float x, float y): Call("PP_Unit_ActionOnPosition",error), action(action) {
		unit.id = unitId;
		unit.type = unitType;
		pos.x = x;
		pos.y = y;
	}
	
	ActionOnPositionCall(const ActionOnPositionCall *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		action = c->action;
		pos.x = c->pos.x;
		pos.y = c->pos.y;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<ActionOnPositionCall>(this);
	}
	
private:
		
	CallMisc::Unit unit;
	int action;
	CallMisc::Position pos;
	
	virtual bool compare(const Call *c) const {
		const ActionOnPositionCall *cc = dynamic_cast<const ActionOnPositionCall*>(c);
		if ((Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id) || (Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type) || (Call::paramsMap.contains(label,"action") && action != cc->action) || (Call::paramsMap.contains(label,"position") && pos != cc->pos))
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const ActionOnPositionCall *cc = dynamic_cast<const ActionOnPositionCall*>(c);
		if (!Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::paramsMap.contains(label,"action") && action != cc->action && action != -1)
			action = -1;
		if (!Call::paramsMap.contains(label,"position") && pos.x != cc->pos.x && pos.x != -1)
			pos.x = -1;
		if (!Call::paramsMap.contains(label,"position") && pos.y != cc->pos.y && pos.y != -1)
			pos.y = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const ActionOnPositionCall *cc = dynamic_cast<const ActionOnPositionCall*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (action != cc->action)
			sc++;
		if (pos != cc->pos)
			sc++;
		return std::make_pair<int,int>(sc,3);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (action == -1) ? "?" : boost::lexical_cast<std::string>(action);
		s += " ";
		s += (pos.x == -1) ? "?" : boost::lexical_cast<std::string>(pos.x);
		s += " ";
		s += (pos.y == -1) ? "?" : boost::lexical_cast<std::string>(pos.y);
		return s;
	}
	
	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (TracesAnalyser::units_id_map.find(unit.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(unit.type) : "_";
		s += ",";
		s += (TracesAnalyser::orders_map.find(action) != TracesAnalyser::orders_map.end()) ? TracesAnalyser::orders_map.at(action) : "_";
		s += ",";
		s += (pos.x != -1) ? boost::lexical_cast<std::string>(pos.x) : "_";
		s += ",";
		s += (pos.y != -1) ? boost::lexical_cast<std::string>(pos.y) : "_";
		s += ")";
		return s;
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		ActionOnPositionCall *cc;
		if (c != NULL)
			cc = dynamic_cast<ActionOnPositionCall*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		if (c != NULL && action != cc->action)
			ids.push_back("action");
		if (c != NULL && pos != cc->pos)
			ids.push_back("position");
		return ids;
	}

};

class UntargetedActionCall : public Call {

public:
		
	UntargetedActionCall(ErrorType error, int unitId, int unitType, int action, float param): Call("PP_Unit_UntargetedAction",error), action(action), param(param) {
		unit.id = unitId;
		unit.type = unitType;
	}
	
	UntargetedActionCall(const UntargetedActionCall *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		action = c->action;
		param = c->param;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<UntargetedActionCall>(this);
	}
	
private:
		
	CallMisc::Unit unit;
	int action;
	float param;
	
	virtual bool compare(const Call *c) const {
		const UntargetedActionCall *cc = dynamic_cast<const UntargetedActionCall*>(c);
		if ((Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id) || (Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type) || (Call::paramsMap.contains(label,"action") && action != cc->action) || (Call::paramsMap.contains(label,"param") && param != cc->param))
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const UntargetedActionCall *cc = dynamic_cast<const UntargetedActionCall*>(c);
		if (!Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::paramsMap.contains(label,"action") && action != cc->action && action != -1)
			action = -1;
		if (!Call::paramsMap.contains(label,"param") && param != cc->param && param != -1)
			param = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const UntargetedActionCall *cc = dynamic_cast<const UntargetedActionCall*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (action != cc->action)
			sc++;
		if (param != cc->param)
			sc++;
		return std::make_pair<int,int>(sc,3);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (action == -1) ? "?" : boost::lexical_cast<std::string>(action);
		s += " ";
		s += (param == -1) ? "?" : boost::lexical_cast<std::string>(param);
		return s;
	}
	
	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (TracesAnalyser::units_id_map.find(unit.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(unit.type) : "_";
		s += ",";
		s += (TracesAnalyser::orders_map.find(action) != TracesAnalyser::orders_map.end()) ? TracesAnalyser::orders_map.at(action) : "_";
		s += ",";
		s += (param != -1) ? boost::lexical_cast<std::string>(param) : "_";
		s += ")";
		return s;
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		UntargetedActionCall *cc;
		if (c != NULL)
			cc = dynamic_cast<UntargetedActionCall*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		if (c != NULL && action != cc->action)
			ids.push_back("action");
		if (c != NULL && param != cc->param)
			ids.push_back("param");
		return ids;
	}

};

class GetCodePdgCmdCall : public Call {

public:
		
	GetCodePdgCmdCall(ErrorType error, int unitId, int unitType, int idCmd): Call("PP_Unit_PdgCmd_GetCode",error), idCmd(idCmd) {
		unit.id = unitId;
		unit.type = unitType;
	}
	
	GetCodePdgCmdCall(const GetCodePdgCmdCall *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		idCmd = c->idCmd;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<GetCodePdgCmdCall>(this);
	}
	
private:
		
	CallMisc::Unit unit;
	int idCmd;
	
	virtual bool compare(const Call *c) const {
		const GetCodePdgCmdCall *cc = dynamic_cast<const GetCodePdgCmdCall*>(c);
		if ((Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id) || (Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type) || (Call::paramsMap.contains(label,"idCmd") && idCmd != cc->idCmd))
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const GetCodePdgCmdCall *cc = dynamic_cast<const GetCodePdgCmdCall*>(c);
		if (!Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::paramsMap.contains(label,"idCmd") && idCmd != cc->idCmd && idCmd != -1)
			idCmd = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const GetCodePdgCmdCall *cc = dynamic_cast<const GetCodePdgCmdCall*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (idCmd != cc->idCmd)
			sc++;
		return std::make_pair<int,int>(sc,2);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (idCmd == -1) ? "?" : boost::lexical_cast<std::string>(idCmd);
		return s;
	}
	
	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (TracesAnalyser::units_id_map.find(unit.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(unit.type) : "_";
		s += ",";
		s += (idCmd != -1) ? boost::lexical_cast<std::string>(idCmd) : "_";
		s += ")";
		return s;
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		GetCodePdgCmdCall *cc;
		if (c != NULL)
			cc = dynamic_cast<GetCodePdgCmdCall*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		if (error == Call::OUT_OF_RANGE || (c != NULL && idCmd != cc->idCmd))
			ids.push_back("idCmd");
		return ids;
	}
	
};

class GetNumParamsPdgCmdCall : public Call {

public:
		
	GetNumParamsPdgCmdCall(ErrorType error, int unitId, int unitType, int idCmd): Call("PP_Unit_PdgCmd_GetNumParams",error), idCmd(idCmd) {
		unit.id = unitId;
		unit.type = unitType;
	}
	
	GetNumParamsPdgCmdCall(const GetNumParamsPdgCmdCall *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		idCmd = c->idCmd;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<GetNumParamsPdgCmdCall>(this);
	}
	
private:
		
	CallMisc::Unit unit;
	int idCmd;
	
	virtual bool compare(const Call *c) const {
		const GetNumParamsPdgCmdCall *cc = dynamic_cast<const GetNumParamsPdgCmdCall*>(c);
		if ((Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id) || (Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type) || (Call::paramsMap.contains(label,"idCmd") && idCmd != cc->idCmd))
			return false;
		return true;
	}
	
	virtual void filter(const Call *c) {
		const GetNumParamsPdgCmdCall *cc = dynamic_cast<const GetNumParamsPdgCmdCall*>(c);
		if (!Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::paramsMap.contains(label,"idCmd") && idCmd != cc->idCmd && idCmd != -1)
			idCmd = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const GetNumParamsPdgCmdCall *cc = dynamic_cast<const GetNumParamsPdgCmdCall*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (idCmd != cc->idCmd)
			sc++;
		return std::make_pair<int,int>(sc,2);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (idCmd == -1) ? "?" : boost::lexical_cast<std::string>(idCmd);
		return s;
	}
	
	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (TracesAnalyser::units_id_map.find(unit.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(unit.type) : "_";
		s += ",";
		s += (idCmd != -1) ? boost::lexical_cast<std::string>(idCmd) : "_";
		s += ")";
		return s;
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		GetNumParamsPdgCmdCall *cc;
		if (c != NULL)
			cc = dynamic_cast<GetNumParamsPdgCmdCall*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		if (error == Call::OUT_OF_RANGE || (c != NULL && idCmd != cc->idCmd))
			ids.push_back("idCmd");
		return ids;
	}

};

class GetParamPdgCmdCall : public Call {

public:
		
	GetParamPdgCmdCall(ErrorType error, int unitId, int unitType, int idCmd, int idParam): Call("PP_Unit_PdgCmd_GetParam",error), idCmd(idCmd), idParam(idParam) {
		unit.id = unitId;
		unit.type = unitType;
	}
	
	GetParamPdgCmdCall(const GetParamPdgCmdCall *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		idCmd = c->idCmd;
		idParam = c->idParam;
	}
	
	virtual Trace::sp_trace clone() const {
		return boost::make_shared<GetParamPdgCmdCall>(this);
	}
	
private:
		
	CallMisc::Unit unit;
	int idCmd;
	int idParam;
	
	virtual bool compare(const Call *c) const {
		const GetParamPdgCmdCall *cc = dynamic_cast<const GetParamPdgCmdCall*>(c);
		if ((Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id) || (Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type) || (Call::paramsMap.contains(label,"idCmd") && idCmd != cc->idCmd) || (Call::paramsMap.contains(label,"idParam") && idParam != cc->idParam))
			return false;
		return true;	
	}
	
	virtual void filter(const Call *c) {
		const GetParamPdgCmdCall *cc = dynamic_cast<const GetParamPdgCmdCall*>(c);
		if (!Call::paramsMap.contains(label,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::paramsMap.contains(label,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::paramsMap.contains(label,"idCmd") && idCmd != cc->idCmd && idCmd != -1)
			idCmd = -1;
		if (!Call::paramsMap.contains(label,"idParam") && idParam != cc->idParam && idParam != -1)
			idParam = -1;
	}
	
	virtual std::pair<int,int> distance(const Call *c) const {
		const GetParamPdgCmdCall *cc = dynamic_cast<const GetParamPdgCmdCall*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (idCmd != cc->idCmd)
			sc++;
		if (idParam != cc->idParam)
			sc++;
		return std::make_pair<int,int>(sc,3);
	}
	
	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (idCmd == -1) ? "?" : boost::lexical_cast<std::string>(idCmd);
		s += " ";
		s += (idParam == -1) ? "?" : boost::lexical_cast<std::string>(idParam);
		return s;
	}
	
	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (TracesAnalyser::units_id_map.find(unit.type) != TracesAnalyser::units_id_map.end()) ? TracesAnalyser::units_id_map.at(unit.type) : "_";
		s += ",";
		s += (idCmd != -1) ? boost::lexical_cast<std::string>(idCmd) : "_";
		s += ",";
		s += (idParam != -1) ? boost::lexical_cast<std::string>(idParam) : "_";
		s += ")";
		return s;
	}
	
	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		GetParamPdgCmdCall *cc;
		if (c != NULL)
			cc = dynamic_cast<GetParamPdgCmdCall*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		if (c != NULL && idCmd != cc->idCmd)
			ids.push_back("idCmd");
		if (error == Call::OUT_OF_RANGE || (c != NULL && idParam != cc->idParam))
			ids.push_back("idParam");
		return ids;
	}
	
};

#endif