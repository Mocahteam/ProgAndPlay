/**
 * \file CallDef.h
 * \brief Déclaration des classes dérivées de la classe Call
 * \author meresse
 * \version 0.1
 */

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

class CallWithNoParam : public Call {

public:

	CallWithNoParam(std::string key): Call(key,Call::NONE) {}

	CallWithNoParam(const CallWithNoParam *c) : Call(c) {}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithNoParam>(this);
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

class CallWithIntParam : public Call {

public:

	CallWithIntParam(ErrorType error, std::string key, int param): Call(key,error), param(param) {}

	CallWithIntParam(const CallWithIntParam *c) : Call(c) {
		param = c->param;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithIntParam>(this);
	}

private:

	int param;

	virtual bool compare(const Call *c) const {
		const CallWithIntParam *cc = dynamic_cast<const CallWithIntParam*>(c);
		if ((Call::callMaps.contains(key,"specialAreaId") ||
				Call::callMaps.contains(key,"resourceId") ||
				Call::callMaps.contains(key,"coalition")) &&
				param != cc->param)
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithIntParam *cc = dynamic_cast<const CallWithIntParam*>(c);
		if (!Call::callMaps.contains(key,"specialAreaId") &&
				!Call::callMaps.contains(key,"resourceId") &&
				!Call::callMaps.contains(key,"coalition") &&
				 param != cc->param && param != -1)
			param = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithIntParam *cc = dynamic_cast<const CallWithIntParam*>(c);
		if (param == cc->param)
			return std::make_pair<int,int>(0,1);
		return std::make_pair<int,int>(1,1);
	}

	virtual std::string getParams() const {
		if (param == -1)
			return "?";
		return boost::lexical_cast<std::string>(param);
	}

	virtual std::string getReadableParams() const {
		if (Call::callMaps.contains(key,"resourceId"))
			return "(" + ((Call::resources_map.find(param) != Call::resources_map.end()) ? Call::resources_map.at(param) : "_") + ")";
		if (Call::callMaps.contains(key,"coalition"))
			return "(" + ((error == Call::NONE && param != CallMisc::NONE) ? std::string(Call::getEnumLabel<int>(param,Call::coalitionsArr)) : "_") + ")";
		// cas général
		return "(" + ((param != -1) ? boost::lexical_cast<std::string>(param) : "_") + ")";
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithIntParam *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntParam*>(c);
		if (error == Call::OUT_OF_RANGE ||
				error == Call::INVALID_COALITION || (c != NULL && param != cc->param)){
			if (Call::callMaps.contains(key,"specialAreaId"))
				ids.push_back("specialAreaId");
			if (Call::callMaps.contains(key,"resourceId"))
				ids.push_back("resourceId");
			if (Call::callMaps.contains(key,"coalition"))
				ids.push_back("coalition");
			// default
			if (!Call::callMaps.contains(key,"specialAreaId") && !Call::callMaps.contains(key,"resourceId") && !Call::callMaps.contains(key,"coalition"))
				ids.push_back("param");
		}
		return ids;
	}
};

class CallWithIntIntParams : public Call {

public:

	CallWithIntIntParams(ErrorType error, std::string key, int param1, int param2): Call(key,error), param1(param1), param2(param2) {}

	CallWithIntIntParams(const CallWithIntIntParams *c) : Call(c) {
		param1 = c->param1;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithIntIntParams>(this);
	}

private:

	int param1;
	int param2;

	virtual bool compare(const Call *c) const {
		const CallWithIntIntParams *cc = dynamic_cast<const CallWithIntIntParams*>(c);
		if ((Call::callMaps.contains(key,"coalitionFirst") && param1 != cc->param1) || (Call::callMaps.contains(key,"indexSecond") && param2 != cc->param2))
			return false;
		if ((Call::callMaps.contains(key,"indexFirst") && param1 != cc->param1) || (Call::callMaps.contains(key,"coalitionSecond") && param2 != cc->param2))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithIntIntParams *cc = dynamic_cast<const CallWithIntIntParams*>(c);
		if (!Call::callMaps.contains(key,"coalitionFirst") && !Call::callMaps.contains(key,"indexFirst") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key,"coalitionSecond") && !Call::callMaps.contains(key,"indexSecond") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithIntIntParams *cc = dynamic_cast<const CallWithIntIntParams*>(c);
		int sc = 0;
		if (param1 != cc->param1)
			sc++;
		if (param2 != cc->param2)
			sc++;
		return std::make_pair<int,int>(sc,2);
	}

	virtual std::string getParams() const {
		std::string s = "";
		s += (param1 == -1) ? "?" : boost::lexical_cast<std::string>(param1);
		s += " ";
		s += (param2 == -1) ? "?" : boost::lexical_cast<std::string>(param2);
		return s;
	}

	virtual std::string getReadableParams() const {
		std::string s = "(";
		if (Call::callMaps.contains(key,"coalitionFirst") || Call::callMaps.contains(key,"indexSecond")){
			s += (error != Call::INVALID_COALITION && param1 != CallMisc::NONE) ? Call::getEnumLabel<int>(param1,Call::coalitionsArr) : "_";
			s += ",";
			s += (param2 != -1) ? boost::lexical_cast<std::string>(param2) : "_";
		} else if (Call::callMaps.contains(key,"indexFirst") || Call::callMaps.contains(key,"coalitionSecond")){
			s += (param1 != -1) ? boost::lexical_cast<std::string>(param1) : "_";
			s += ",";
			s += (error != Call::INVALID_COALITION && param2 != CallMisc::NONE) ? Call::getEnumLabel<int>(param2,Call::coalitionsArr) : "_";
		} else { // default case
			s += (param1 != -1) ? boost::lexical_cast<std::string>(param1) : "_";
			s += ",";
			s += (param2 != -1) ? boost::lexical_cast<std::string>(param2) : "_";
		}
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithIntIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntIntParams*>(c);
		if (error == Call::INVALID_COALITION ||
				(Call::callMaps.contains(key,"coalitionFirst") && c != NULL && param1 != cc->param1) ||
				(Call::callMaps.contains(key,"coalitionSecond") && c != NULL && param2 != cc->param2))
			ids.push_back("coalition");
		if (error == Call::OUT_OF_RANGE ||
				(Call::callMaps.contains(key,"indexFirst") && c != NULL && param1 != cc->param1) ||
				(Call::callMaps.contains(key,"indexSecond") && c != NULL && param2 != cc->param2))
			ids.push_back("index");
		if (!Call::callMaps.contains(key,"coalitionFirst") && !Call::callMaps.contains(key,"indexFirst") && c != NULL && param1 != cc->param1)
			ids.push_back("param1");
		if (!Call::callMaps.contains(key,"coalitionSecond") && !Call::callMaps.contains(key,"indexSecond") && c != NULL && param2 != cc->param2)
			ids.push_back("param2");
		return ids;
	}
};

class CallWithUnitParam : public Call {

public:

	CallWithUnitParam(ErrorType error, std::string key, int unitId, int unitType): Call(key,error) {
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithUnitParam(const CallWithUnitParam *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithUnitParam>(this);
	}

private:

	CallMisc::Unit unit;

	virtual bool compare(const Call *c) const {
		const CallWithUnitParam *cc = dynamic_cast<const CallWithUnitParam*>(c);
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithUnitParam *cc = dynamic_cast<const CallWithUnitParam*>(c);
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithUnitParam *cc = dynamic_cast<const CallWithUnitParam*>(c);
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
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So do not display
		// unit attributes.
		if (Call::callMaps.getLabel(key).find("Unit::") != std::string::npos)
			return "";
		else
			return "(" + ((Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_") + ")";
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithUnitParam *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitParam*>(c);
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		return ids;
	}
};

class CallWithUnitIntParams : public Call {

public:

	CallWithUnitIntParams(ErrorType error, std::string key, int unitId, int unitType, int param): Call(key,error), param(param) {
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithUnitIntParams(const CallWithUnitIntParams *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param = c->param;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithUnitIntParams>(this);
	}

private:

	CallMisc::Unit unit;
	int param;

	virtual bool compare(const Call *c) const {
		const CallWithUnitIntParams *cc = dynamic_cast<const CallWithUnitIntParams*>(c);
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"groupId") && param != cc->param))
			return false;
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"coalition") && param != cc->param))
			return false;
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"typeToCheck") && param != cc->param))
			return false;
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"action") && param != cc->param))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithUnitIntParams *cc = dynamic_cast<const CallWithUnitIntParams*>(c);
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key,"groupId") && !Call::callMaps.contains(key,"coalition") && !Call::callMaps.contains(key,"typeToCheck") && !Call::callMaps.contains(key,"action") && param != cc->param && param != -1)
			param = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithUnitIntParams *cc = dynamic_cast<const CallWithUnitIntParams*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (param != cc->param)
			sc++;
		return std::make_pair<int,int>(sc,2);
	}

	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (param == -1) ? "?" : boost::lexical_cast<std::string>(param);
		return s;
	}

	virtual std::string getReadableParams() const {
		std::string s = "(";
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So display
		// unit attributes only for non object oriented langages.
		if (Call::callMaps.getLabel(key).find("Unit::") == std::string::npos){
			s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
			s += ",";
		}
		if (Call::callMaps.contains(key,"coalition"))
			s += (error == Call::NONE && param != CallMisc::NONE) ? std::string(Call::getEnumLabel<int>(param,Call::coalitionsArr)) : "_";
		else if (Call::callMaps.contains(key,"typeToCheck"))
			s += (param != -1 && Call::units_id_map.find(param) != Call::units_id_map.end()) ? Call::units_id_map.at(param) : "_";
		else if (Call::callMaps.contains(key,"action"))
			s += (param != -1 && Call::orders_map.find(param) != Call::orders_map.end()) ? Call::orders_map.at(param) : "_";
		else // case for "groupId", and default case
			s += (param != -1) ? boost::lexical_cast<std::string>(param) : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithUnitIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntParams*>(c);
		// check unit
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check param
		if (Call::callMaps.contains(key,"groupId") && (error == Call::OUT_OF_RANGE || (c != NULL && param != cc->param)))
			ids.push_back("groupId");
		else if (Call::callMaps.contains(key,"coalition") && (error == Call::INVALID_COALITION || (c != NULL && param != cc->param)))
			ids.push_back("coalition");
		else if (Call::callMaps.contains(key,"typeToCheck") && c != NULL && param != cc->param)
			ids.push_back("typeToCheck");
		else if (Call::callMaps.contains(key,"action") && c != NULL && param != cc->param)
			ids.push_back("action");
		else if (c != NULL && param != cc->param)
			ids.push_back("param2");
		return ids;
	}

};

class CallWithIntUnitParams : public Call {

public:

	CallWithIntUnitParams(ErrorType error, std::string key, int param, int unitId, int unitType): Call(key,error), param(param) {
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithIntUnitParams(const CallWithIntUnitParams *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param = c->param;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithIntUnitParams>(this);
	}

private:

	CallMisc::Unit unit;
	int param;

	virtual bool compare(const Call *c) const {
		const CallWithIntUnitParams *cc = dynamic_cast<const CallWithIntUnitParams*>(c);
		if ((Call::callMaps.contains(key,"index") && param != cc->param) || (Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithIntUnitParams *cc = dynamic_cast<const CallWithIntUnitParams*>(c);
		if (!Call::callMaps.contains(key,"index") && param != cc->param && param != -1)
			param = -1;
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithIntUnitParams *cc = dynamic_cast<const CallWithIntUnitParams*>(c);
		int sc = 0;
		if (param != cc->param)
			sc++;
		if (unit.type != cc->unit.type)
			sc++;
		return std::make_pair<int,int>(sc,2);
	}

	virtual std::string getParams() const {
		std::string s = "";
		s += (param == -1) ? "?" : boost::lexical_cast<std::string>(param);
		s += " ";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		return s;
	}

	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (param != -1) ? boost::lexical_cast<std::string>(param) : "_";
		s += ",";
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithIntUnitParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntUnitParams*>(c);
		// param
		if (Call::callMaps.contains(key,"index") && (error == Call::OUT_OF_RANGE || (c != NULL && param != cc->param)))
			ids.push_back("index");
		else if (c != NULL && param != cc->param)
			ids.push_back("param1");

		// unit type
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		return ids;
	}

};

class CallWithIntIntUnitParams : public Call {

public:

	CallWithIntIntUnitParams(ErrorType error, std::string key, int param1, int param2, int unitId, int unitType): Call(key,error), param1(param1), param2(param2) {
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithIntIntUnitParams(const CallWithIntIntUnitParams *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithIntIntUnitParams>(this);
	}

private:

	CallMisc::Unit unit;
	int param1;
	int param2;

	virtual bool compare(const Call *c) const {
		const CallWithIntIntUnitParams *cc = dynamic_cast<const CallWithIntIntUnitParams*>(c);
		if ((Call::callMaps.contains(key,"paramIndex") && param1 != cc->param1) || (Call::callMaps.contains(key,"cmdIndex") && param2 != cc->param2) || (Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithIntIntUnitParams *cc = dynamic_cast<const CallWithIntIntUnitParams*>(c);
		if (!Call::callMaps.contains(key,"paramIndex") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key,"cmdIndex") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithIntIntUnitParams *cc = dynamic_cast<const CallWithIntIntUnitParams*>(c);
		int sc = 0;
		if (param1 != cc->param1)
			sc++;
		if (param2 != cc->param2)
			sc++;
		if (unit.type != cc->unit.type)
			sc++;
		return std::make_pair<int,int>(sc,3);
	}

	virtual std::string getParams() const {
		std::string s = "";
		s += (param1 == -1) ? "?" : boost::lexical_cast<std::string>(param1);
		s += " ";
		s += (param2 == -1) ? "?" : boost::lexical_cast<std::string>(param2);
		s += " ";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		return s;
	}

	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (param1 != -1) ? boost::lexical_cast<std::string>(param1) : "_";
		s += ",";
		s += (param2 != -1) ? boost::lexical_cast<std::string>(param2) : "_";
		s += ",";
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+ "unit" : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithIntIntUnitParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntIntUnitParams*>(c);
		// param 1
		if (c != NULL && param1 != cc->param1)
			ids.push_back("param1");

		// param 2
		if (c != NULL && param2 != cc->param2)
			ids.push_back("param2");

		// unit type
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		return ids;
	}

};

class CallWithIntUnitIntParams : public Call {

public:

	CallWithIntUnitIntParams(ErrorType error, std::string key, int param1, int unitId, int unitType, int param3): Call(key,error), param1(param1), param3(param3) {
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithIntUnitIntParams(const CallWithIntUnitIntParams *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		param3 = c->param3;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithIntUnitIntParams>(this);
	}

private:

	CallMisc::Unit unit;
	int param1;
	int param3;

	virtual bool compare(const Call *c) const {
		const CallWithIntUnitIntParams *cc = dynamic_cast<const CallWithIntUnitIntParams*>(c);
		if ((Call::callMaps.contains(key,"index") && param1 != cc->param1) || (Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"action") && param3 != cc->param3))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithIntUnitIntParams *cc = dynamic_cast<const CallWithIntUnitIntParams*>(c);
		if (!Call::callMaps.contains(key,"index") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key,"action") && param3 != cc->param3 && param3 != -1)
			param3 = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithIntUnitIntParams *cc = dynamic_cast<const CallWithIntUnitIntParams*>(c);
		int sc = 0;
		if (param1 != cc->param1)
			sc++;
		if (unit.type != cc->unit.type)
			sc++;
		if (param3 != cc->param3)
			sc++;
		return std::make_pair<int,int>(sc,3);
	}

	virtual std::string getParams() const {
		std::string s = "";
		s += (param1 == -1) ? "?" : boost::lexical_cast<std::string>(param1);
		s += " ";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (param3 == -1) ? "?" : boost::lexical_cast<std::string>(param3);
		return s;
	}

	virtual std::string getReadableParams() const {
		std::string s = "(";
		s += (param1 != -1) ? boost::lexical_cast<std::string>(param1) : "_";
		s += ",";
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
		s += ",";
		s += (param3 != -1) ? boost::lexical_cast<std::string>(param3) : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithIntUnitIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntUnitIntParams*>(c);
		// param 1
		if (Call::callMaps.contains(key,"index") && (error == Call::OUT_OF_RANGE || (c != NULL && param1 != cc->param1)))
			ids.push_back("index");
		else if (c != NULL && param1 != cc->param1)
			ids.push_back("param1");

		// unit type
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");

		// param 3
		if (Call::callMaps.contains(key,"action") && c != NULL && param3 != cc->param3)
			ids.push_back("action");
		else if (c != NULL && param3 != cc->param3)
			ids.push_back("param3");
		return ids;
	}

};

class CallWithUnitIntUnitIntParams : public Call {

public:

	CallWithUnitIntUnitIntParams(ErrorType error, std::string key, int unitId, int unitType, int param1, int targetId, int targetType, int param2): Call(key,error), param1(param1), param2(param2) {
		unit.id = unitId;
		unit.type = unitType;
		target.id = targetId;
		target.type = targetType;
	}

	CallWithUnitIntUnitIntParams(const CallWithUnitIntUnitIntParams *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		target.id = c->target.id;
		target.type = c->target.type;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithUnitIntUnitIntParams>(this);
	}

private:

	CallMisc::Unit unit;
	int param1, param2;
	CallMisc::Unit target;

	virtual bool compare(const Call *c) const {
		const CallWithUnitIntUnitIntParams *cc = dynamic_cast<const CallWithUnitIntUnitIntParams*>(c);
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"action") && param1 != cc->param1) || (Call::callMaps.contains(key,"targetId") && target.id != cc->target.id) || (Call::callMaps.contains(key,"targetType") && target.type != cc->target.type) || (Call::callMaps.contains(key,"synchronized") && param2 != cc->param2))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithUnitIntUnitIntParams *cc = dynamic_cast<const CallWithUnitIntUnitIntParams*>(c);
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key,"action") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key,"targetId") && target.id != cc->target.id && target.id != -1)
			target.id = -1;
		if (!Call::callMaps.contains(key,"targetType") && target.type != cc->target.type && target.type != -1)
			target.type = -1;
		if (!Call::callMaps.contains(key,"synchronized") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithUnitIntUnitIntParams *cc = dynamic_cast<const CallWithUnitIntUnitIntParams*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (param1 != cc->param1)
			sc++;
		if (target.type != cc->target.type)
			sc++;
		if (param2 != cc->param2)
			sc++;
		return std::make_pair<int,int>(sc,3);
	}

	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (param1 == -1) ? "?" : boost::lexical_cast<std::string>(param1);
		s += " ";
		s += (target.id == -1) ? "?" : boost::lexical_cast<std::string>(target.id);
		s += "_";
		s += (target.type == -1) ? "?" : boost::lexical_cast<std::string>(target.type);
		s += " ";
		s += (param2 == -1) ? "?" : boost::lexical_cast<std::string>(param2);
		return s;
	}

	virtual std::string getReadableParams() const {
		std::string s = "(";
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So display
		// unit attributes only for non object oriented langages.
		if (Call::callMaps.getLabel(key).find("Unit::") == std::string::npos){
			s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
			s += ",";
		}
		s += (Call::orders_map.find(param1) != Call::orders_map.end()) ? Call::orders_map.at(param1) : "_";
		s += ",";
		s += (Call::units_id_map.find(target.type) != Call::units_id_map.end()) ? Call::units_id_map.at(target.type)+" unit" : "_";
		s += ",";
		// Ugly!!! only Scratch labels include " _ " token to describe parameters position. We use this
		// trick to define how to display last parameter.
		if (Call::callMaps.getLabel(key).find(" _ ") != std::string::npos)
			s += (param2 != 0) ? "WAIT" : "CONTINUE";
		else
			s += (param2 != 0) ? "true" : "false";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithUnitIntUnitIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntUnitIntParams*>(c);
		// check unit
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check param1
		if (c != NULL && param1 != cc->param1)
			ids.push_back("action");
		// check target
		if (c != NULL && target.type != cc->target.type)
			ids.push_back("targetType");
		// check synchronized
		if (c != NULL && param2 != cc->param2){
			if (param2 == 0)
				ids.push_back("synchronous");
			else
				ids.push_back("asynchronous");
		}
		return ids;
	}
};

class CallWithUnitIntPosIntParams : public Call {

public:

	CallWithUnitIntPosIntParams(ErrorType error, std::string key, int unitId, int unitType, int param1, float x, float y, int param2): Call(key,error), param1(param1), param2(param2) {
		unit.id = unitId;
		unit.type = unitType;
		pos.x = x;
		pos.y = y;
	}

	CallWithUnitIntPosIntParams(const CallWithUnitIntPosIntParams *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		pos.x = c->pos.x;
		pos.y = c->pos.y;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithUnitIntPosIntParams>(this);
	}

private:

	CallMisc::Unit unit;
	int param1, param2;
	CallMisc::Position pos;

	virtual bool compare(const Call *c) const {
		const CallWithUnitIntPosIntParams *cc = dynamic_cast<const CallWithUnitIntPosIntParams*>(c);
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"action") && param1 != cc->param1) || (Call::callMaps.contains(key,"position") && pos != cc->pos) || (Call::callMaps.contains(key,"synchronized") && param2 != cc->param2))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithUnitIntPosIntParams *cc = dynamic_cast<const CallWithUnitIntPosIntParams*>(c);
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key,"action") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key,"position") && pos.x != cc->pos.x && pos.x != -1)
			pos.x = -1;
		if (!Call::callMaps.contains(key,"position") && pos.y != cc->pos.y && pos.y != -1)
			pos.y = -1;
		if (!Call::callMaps.contains(key,"synchronized") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithUnitIntPosIntParams *cc = dynamic_cast<const CallWithUnitIntPosIntParams*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (param1 != cc->param1)
			sc++;
		if (pos.x != cc->pos.x)
			sc++;
		if (pos.y != cc->pos.y)
			sc++;
		if (param2 != cc->param2)
			sc++;
		return std::make_pair<int,int>(sc,4);
	}

	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (param1 == -1) ? "?" : boost::lexical_cast<std::string>(param1);
		s += " ";
		s += (pos.x == -1) ? "?" : boost::lexical_cast<std::string>(pos.x);
		s += " ";
		s += (pos.y == -1) ? "?" : boost::lexical_cast<std::string>(pos.y);
		s += " ";
		s += (param2 == -1) ? "?" : boost::lexical_cast<std::string>(param2);
		return s;
	}

	virtual std::string getReadableParams() const {
		std::string s = "(";
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So display
		// unit attributes only for non object oriented langages.
		if (Call::callMaps.getLabel(key).find("Unit::") == std::string::npos){
			s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
			s += ",";
		}
		s += (Call::orders_map.find(param1) != Call::orders_map.end()) ? Call::orders_map.at(param1) : "_";
		s += ",";
		s += (pos.x != -1) ? boost::lexical_cast<std::string>(pos.x) : "_";
		s += ",";
		s += (pos.y != -1) ? boost::lexical_cast<std::string>(pos.y) : "_";
		s += ",";
		// Ugly!!! only Scratch labels include " _ " token to describe parameters position. We use this
		// trick to define how to display last parameter.
		if (Call::callMaps.getLabel(key).find(" _ ") != std::string::npos)
			s += (param2 != 0) ? "WAIT" : "CONTINUE";
		else
			s += (param2 != 0) ? "true" : "false";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithUnitIntPosIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntPosIntParams*>(c);
		// check unit
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check param1
		if (c != NULL && param1 != cc->param1)
			ids.push_back("action");
		// check position
		if (c != NULL && pos != cc->pos)
			ids.push_back("position");
		// check synchronized
		if (c != NULL && param2 != cc->param2){
			if (param2 == 0)
				ids.push_back("synchronous");
			else
				ids.push_back("asynchronous");
		}
		return ids;
	}
};

class CallWithUnitIntFloatIntParams : public Call {

public:

	CallWithUnitIntFloatIntParams(ErrorType error, std::string key, int unitId, int unitType, int param1, float param2, int param3): Call(key,error), param1(param1), param2(param2), param3(param3) {
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithUnitIntFloatIntParams(const CallWithUnitIntFloatIntParams *c) : Call(c) {
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		param2 = c->param2;
		param3 = c->param3;
	}

	virtual Trace::sp_trace clone() const {
		return boost::make_shared<CallWithUnitIntFloatIntParams>(this);
	}

private:

	CallMisc::Unit unit;
	int param1;
	float param2;
	int param3;

	virtual bool compare(const Call *c) const {
		const CallWithUnitIntFloatIntParams *cc = dynamic_cast<const CallWithUnitIntFloatIntParams*>(c);
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"action") && param1 != cc->param1) || (Call::callMaps.contains(key,"param") && param2 != cc->param2) || (Call::callMaps.contains(key,"synchronized") && param3 != cc->param3))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const CallWithUnitIntFloatIntParams *cc = dynamic_cast<const CallWithUnitIntFloatIntParams*>(c);
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key,"action") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key,"param") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
		if (!Call::callMaps.contains(key,"synchronized") && param3 != cc->param3 && param3 != -1)
			param3 = -1;
	}

	virtual std::pair<int,int> distance(const Call *c) const {
		const CallWithUnitIntFloatIntParams *cc = dynamic_cast<const CallWithUnitIntFloatIntParams*>(c);
		int sc = 0;
		if (unit.type != cc->unit.type)
			sc++;
		if (param1 != cc->param1)
			sc++;
		if (param2 != cc->param2)
			sc++;
		if (param3 != cc->param3)
			sc++;
		return std::make_pair<int,int>(sc,3);
	}

	virtual std::string getParams() const {
		std::string s = "";
		s += (unit.id == -1) ? "?" : boost::lexical_cast<std::string>(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : boost::lexical_cast<std::string>(unit.type);
		s += " ";
		s += (param1 == -1) ? "?" : boost::lexical_cast<std::string>(param1);
		s += " ";
		s += (param2 == -1) ? "?" : boost::lexical_cast<std::string>(param2);
		s += " ";
		s += (param3 == -1) ? "?" : boost::lexical_cast<std::string>(param3);
		return s;
	}

	virtual std::string getReadableParams() const {
		std::string s = "(";
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So display
		// unit attributes only for non object oriented langages.
		if (Call::callMaps.getLabel(key).find("Unit::") == std::string::npos){
			s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
			s += ",";
		}
		s += (Call::orders_map.find(param1) != Call::orders_map.end()) ? Call::orders_map.at(param1) : "_";
		s += ",";
		s += (param2 != -1) ? boost::lexical_cast<std::string>(param2) : "_";
		// Ugly!!! only Scratch labels include " _ " token to describe parameters position. We use this
		// trick to define if we have to display last parameter. For Scratch, we don't want to show it
		if (Call::callMaps.getLabel(key).find(" _ ") == std::string::npos){
			s += ",";
			s += (param3 != 0) ? "true" : "false";
		}
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const {
		std::vector<std::string> ids;
		CallWithUnitIntFloatIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntFloatIntParams*>(c);
		// check unit
		if (c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check action
		if (c != NULL && param1 != cc->param1)
			ids.push_back("action");
		// check param
		if (c != NULL && param2 != cc->param2)
			ids.push_back("param3");
		// check synchronized
		if (c != NULL && param3 != cc->param3){
			if (param3 == 0)
				ids.push_back("synchronous");
			else
				ids.push_back("asynchronous");
		}
		return ids;
	}
};

/* Supprimer code ci-dessous */
/*
class GetCodePdgCmdCall : public Call {

public:

	GetCodePdgCmdCall(ErrorType error, int unitId, int unitType, int idCmd): Call(CALL_UNIT_PDG_CMD_GET_CODE,error), idCmd(idCmd) {
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
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"idCmd") && idCmd != cc->idCmd))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const GetCodePdgCmdCall *cc = dynamic_cast<const GetCodePdgCmdCall*>(c);
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key,"idCmd") && idCmd != cc->idCmd && idCmd != -1)
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
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
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

	GetNumParamsPdgCmdCall(ErrorType error, int unitId, int unitType, int idCmd): Call(CALL_UNIT_PDG_CMD_GET_NUM_PARAM,error), idCmd(idCmd) {
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
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"idCmd") && idCmd != cc->idCmd))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const GetNumParamsPdgCmdCall *cc = dynamic_cast<const GetNumParamsPdgCmdCall*>(c);
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key,"idCmd") && idCmd != cc->idCmd && idCmd != -1)
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
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
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

	GetParamPdgCmdCall(ErrorType error, int unitId, int unitType, int idCmd, int idParam): Call(CALL_UNIT_PDG_CMD_GET_PARAM,error), idCmd(idCmd), idParam(idParam) {
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
		if ((Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type) || (Call::callMaps.contains(key,"idCmd") && idCmd != cc->idCmd) || (Call::callMaps.contains(key,"idParam") && idParam != cc->idParam))
			return false;
		return true;
	}

	virtual void filter(const Call *c) {
		const GetParamPdgCmdCall *cc = dynamic_cast<const GetParamPdgCmdCall*>(c);
		if (!Call::callMaps.contains(key,"unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key,"unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key,"idCmd") && idCmd != cc->idCmd && idCmd != -1)
			idCmd = -1;
		if (!Call::callMaps.contains(key,"idParam") && idParam != cc->idParam && idParam != -1)
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
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type)+" unit" : "_";
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
*/
#endif
