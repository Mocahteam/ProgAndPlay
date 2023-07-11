/**
 * \file CallDef.h
 * \brief Déclaration des classes dérivées de la classe Call
 * \author meresse
 * \version 0.1
 */

#ifndef __CALL_DEF_H__
#define __CALL_DEF_H__

#include <cmath>

#include "Call.h"

namespace CallMisc
{

enum Coalition
{
	NONE = -1,
	MY_COALITION,
	ALLY_COALITION,
	ENEMY_COALITION
};

struct Unit
{
	int id;
	int type;

	bool operator!=(const Unit &u) const
	{
		return id != u.id;
	}
};

struct Position
{
	float x;
	float y;

	bool operator!=(const Position &p) const
	{
		return fabs(x - p.x) > FLOAT_EPSILON || fabs(y - p.y) > FLOAT_EPSILON;
	}
};

} // namespace CallMisc

class CallWithNoParam : public Call
{

  public:
	CallWithNoParam(std::string key) : Call(key, key) {}

	CallWithNoParam(const CallWithNoParam *c) : Call(c) {}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithNoParam>(this);
	}

  private:
	virtual bool compare(const Call *c) const
	{
		return true;
	}

	virtual void filter(const Call *c) {}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		return std::pair<int, int>(0, 0);
	}

	virtual std::string getParams() const
	{
		return "";
	}

	virtual std::string getReadableParams() const
	{
		return "";
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		return ids;
	}
};

class CallWithIntParam : public Call
{

  public:
	CallWithIntParam(std::string key, int param) : Call(key, key), param(param) {}

	CallWithIntParam(const CallWithIntParam *c) : Call(c)
	{
		param = c->param;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithIntParam>(this);
	}

  private:
	int param;

	virtual bool compare(const Call *c) const
	{
		const CallWithIntParam *cc = dynamic_cast<const CallWithIntParam *>(c);
		if ((Call::callMaps.contains(key, "specialAreaId") ||
			 Call::callMaps.contains(key, "resourceId") ||
			 Call::callMaps.contains(key, "coalition")) &&
			param != cc->param)
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithIntParam *cc = dynamic_cast<const CallWithIntParam *>(c);
		if (!Call::callMaps.contains(key, "specialAreaId") &&
			!Call::callMaps.contains(key, "resourceId") &&
			!Call::callMaps.contains(key, "coalition") &&
			param != cc->param && param != -1)
			param = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithIntParam *cc = dynamic_cast<const CallWithIntParam *>(c);
		if (Call::callMaps.contains(key, "specialAreaId") ||
			Call::callMaps.contains(key, "resourceId") ||
			Call::callMaps.contains(key, "coalition"))
		{
			if (param != cc->param)
				return std::pair<int, int>(1, 1);
		}
		return std::pair<int, int>(0, 1);
	}

	virtual std::string getParams() const
	{
		if (param == -1)
			return "?";
		return std::to_string(param);
	}

	virtual std::string getReadableParams() const
	{
		if (Call::callMaps.contains(key, "resourceId"))
			return "(" + ((Call::resources_map.find(param) != Call::resources_map.end()) ? Call::resources_map.at(param) : "_") + ")";
		if (Call::callMaps.contains(key, "coalition"))
			return "(" + (param != CallMisc::NONE ? std::string(Call::getEnumLabel<int>(param, Call::coalitionsArr)) : "_") + ")";
		// cas général
		return "(" + ((param != -1) ? std::to_string(param) : "_") + ")";
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithIntParam *cc;
		if (c != NULL){
			cc = dynamic_cast<CallWithIntParam *>(c);
			if (param != cc->param)
			{
				if (Call::callMaps.contains(key, "specialAreaId"))
					ids.push_back("specialAreaId");
				else if (Call::callMaps.contains(key, "resourceId"))
					ids.push_back("resourceId");
				else if (Call::callMaps.contains(key, "coalition"))
					ids.push_back("coalition");
			}
		}
		return ids;
	}
};

class CallWithIntIntParams : public Call
{

  public:
	CallWithIntIntParams(std::string key, int param1, int param2) : Call(key, key), param1(param1), param2(param2) {}

	CallWithIntIntParams(const CallWithIntIntParams *c) : Call(c)
	{
		param1 = c->param1;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithIntIntParams>(this);
	}

  private:
	int param1;
	int param2;

	virtual bool compare(const Call *c) const
	{
		const CallWithIntIntParams *cc = dynamic_cast<const CallWithIntIntParams *>(c);
		if ((Call::callMaps.contains(key, "coalitionFirst") && param1 != cc->param1) || (Call::callMaps.contains(key, "indexSecond") && param2 != cc->param2))
			return false;
		if ((Call::callMaps.contains(key, "indexFirst") && param1 != cc->param1) || (Call::callMaps.contains(key, "coalitionSecond") && param2 != cc->param2))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithIntIntParams *cc = dynamic_cast<const CallWithIntIntParams *>(c);
		if (!Call::callMaps.contains(key, "coalitionFirst") && !Call::callMaps.contains(key, "indexFirst") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key, "coalitionSecond") && !Call::callMaps.contains(key, "indexSecond") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithIntIntParams *cc = dynamic_cast<const CallWithIntIntParams *>(c);
		int sc = 0;
		if ((Call::callMaps.contains(key, "coalitionFirst") || Call::callMaps.contains(key, "indexFirst")) && param1 != cc->param1)
			sc++;
		if ((Call::callMaps.contains(key, "indexSecond") || Call::callMaps.contains(key, "coalitionSecond")) && param2 != cc->param2)
			sc++;
		return std::pair<int, int>(sc, 2);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (param1 == -1) ? "?" : std::to_string(param1);
		s += " ";
		s += (param2 == -1) ? "?" : std::to_string(param2);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		if (Call::callMaps.contains(key, "coalitionFirst") || Call::callMaps.contains(key, "indexSecond"))
		{
			s += (param1 != CallMisc::NONE) ? Call::getEnumLabel<int>(param1, Call::coalitionsArr) : "_";
			s += ", ";
			s += (param2 != -1) ? std::to_string(param2) : "_";
		}
		else if (Call::callMaps.contains(key, "indexFirst") || Call::callMaps.contains(key, "coalitionSecond"))
		{
			s += (param1 != -1) ? std::to_string(param1) : "_";
			s += ", ";
			s += (param2 != CallMisc::NONE) ? Call::getEnumLabel<int>(param2, Call::coalitionsArr) : "_";
		}
		else
		{ // default case
			s += (param1 != -1) ? std::to_string(param1) : "_";
			s += ", ";
			s += (param2 != -1) ? std::to_string(param2) : "_";
		}
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithIntIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntIntParams *>(c);
		if ((Call::callMaps.contains(key, "coalitionFirst") && c != NULL && param1 != cc->param1) ||
			(Call::callMaps.contains(key, "coalitionSecond") && c != NULL && param2 != cc->param2))
			ids.push_back("coalition");
		if ((Call::callMaps.contains(key, "indexFirst") && c != NULL && param1 != cc->param1) ||
			(Call::callMaps.contains(key, "indexSecond") && c != NULL && param2 != cc->param2))
			ids.push_back("index");
		return ids;
	}
};

class CallWithUnitParam : public Call
{

  public:
	CallWithUnitParam(std::string key, int unitId, int unitType) : Call(key, key)
	{
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithUnitParam(const CallWithUnitParam *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithUnitParam>(this);
	}

  private:
	CallMisc::Unit unit;

	virtual bool compare(const Call *c) const
	{
		const CallWithUnitParam *cc = dynamic_cast<const CallWithUnitParam *>(c);
		if ((Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) || (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithUnitParam *cc = dynamic_cast<const CallWithUnitParam *>(c);
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithUnitParam *cc = dynamic_cast<const CallWithUnitParam *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		return std::pair<int, int>(sc, 2);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So do not display
		// unit attributes.
		if (Call::callMaps.getLabel(key).find("Unit::") != std::string::npos)
			return "";
		else
			return "(" + ((Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + " unit" : "_") + ")";
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithUnitParam *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitParam *>(c);
		
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		return ids;
	}
};

class CallWithUnitIntParams : public Call
{

  public:
	CallWithUnitIntParams(std::string key, int unitId, int unitType, int param) : Call(key, key), param(param)
	{
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithUnitIntParams(const CallWithUnitIntParams *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param = c->param;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithUnitIntParams>(this);
	}

  private:
	CallMisc::Unit unit;
	int param;

	virtual bool compare(const Call *c) const
	{
		const CallWithUnitIntParams *cc = dynamic_cast<const CallWithUnitIntParams *>(c);
		if ((Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type) ||
			(Call::callMaps.contains(key, "groupId") && param != cc->param))
			return false;
		if ((Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type) ||
			(Call::callMaps.contains(key, "coalition") && param != cc->param))
			return false;
		if ((Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type) ||
			(Call::callMaps.contains(key, "typeToCheck") && param != cc->param))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithUnitIntParams *cc = dynamic_cast<const CallWithUnitIntParams *>(c);
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key, "groupId") && !Call::callMaps.contains(key, "coalition") &&
			!Call::callMaps.contains(key, "typeToCheck") && param != cc->param && param != -1)
			param = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithUnitIntParams *cc = dynamic_cast<const CallWithUnitIntParams *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		if ((Call::callMaps.contains(key, "groupId") ||
			 Call::callMaps.contains(key, "coalition") ||
			 Call::callMaps.contains(key, "typeToCheck")) &&
			param != cc->param)
			sc++;
		return std::pair<int, int>(sc, 3);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		s += " ";
		s += (param == -1) ? "?" : std::to_string(param);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So display
		// unit attributes only for non object oriented langages.
		if (Call::callMaps.getLabel(key).find("Unit::") == std::string::npos)
		{
			s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + " unit" : "_";
			s += ", ";
		}
		if (Call::callMaps.contains(key, "coalition"))
			s += (param != CallMisc::NONE) ? std::string(Call::getEnumLabel<int>(param, Call::coalitionsArr)) : "_";
		else if (Call::callMaps.contains(key, "typeToCheck"))
			s += (param != -1 && Call::units_id_map.find(param) != Call::units_id_map.end()) ? Call::units_id_map.at(param) : "_";
		else // case for "groupId", and default case
			s += (param != -1) ? std::to_string(param) : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithUnitIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntParams *>(c);
		// check unit
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check param
		if (Call::callMaps.contains(key, "groupId") && c != NULL && param != cc->param)
			ids.push_back("groupId");
		else if (Call::callMaps.contains(key, "coalition") && c != NULL && param != cc->param)
			ids.push_back("coalition");
		else if (Call::callMaps.contains(key, "typeToCheck") && c != NULL && param != cc->param)
			ids.push_back("typeToCheck");
		return ids;
	}
};

class CallWithUnitIntIntParams : public Call
{

  public:
	CallWithUnitIntIntParams(std::string key, int unitId, int unitType, int param1, int param2) : Call(key), param1(param1), param2(param2)
	{
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithUnitIntIntParams(const CallWithUnitIntIntParams *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithUnitIntIntParams>(this);
	}

  private:
	CallMisc::Unit unit;
	int param1, param2;

	virtual bool compare(const Call *c) const
	{
		const CallWithUnitIntIntParams *cc = dynamic_cast<const CallWithUnitIntIntParams *>(c);
		if ((Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type) ||
			(Call::callMaps.contains(key, "action") && param1 != cc->param1) ||
			(Call::callMaps.contains(key, "synchronized") && param2 != cc->param2))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithUnitIntIntParams *cc = dynamic_cast<const CallWithUnitIntIntParams *>(c);
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key, "action") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key, "synchronized") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithUnitIntIntParams *cc = dynamic_cast<const CallWithUnitIntIntParams *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		if (Call::callMaps.contains(key, "action") && param1 != cc->param1)
			sc++;
		if (Call::callMaps.contains(key, "synchronized") && param2 != cc->param2)
			sc++;
		return std::pair<int, int>(sc, 4);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		s += " ";
		s += (param1 == -1) ? "?" : std::to_string(param1);
		s += " ";
		s += (param2 == -1) ? "?" : std::to_string(param2);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + " unit" : "_";
		s += ", ";
		s += (param1 != -1 && Call::orders_map.find(param1) != Call::orders_map.end()) ? Call::orders_map.at(param1) : "_";
		s += ", ";
		s += (param2 != -1) ? std::to_string(param2) : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithUnitIntIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntIntParams *>(c);
		// check unit
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check params
		if (Call::callMaps.contains(key, "action") && c != NULL && param1 != cc->param1)
			ids.push_back("action");
		else if (Call::callMaps.contains(key, "synchronized") && c != NULL && param2 != cc->param2)
			ids.push_back("synchronized");
		return ids;
	}
};

class CallWithIntUnitParams : public Call
{

  public:
	CallWithIntUnitParams(std::string key, int param, int unitId, int unitType) : Call(key), param(param)
	{
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithIntUnitParams(const CallWithIntUnitParams *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param = c->param;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithIntUnitParams>(this);
	}

  private:
	CallMisc::Unit unit;
	int param;

	virtual bool compare(const Call *c) const
	{
		const CallWithIntUnitParams *cc = dynamic_cast<const CallWithIntUnitParams *>(c);
		if ((Call::callMaps.contains(key, "index") && param != cc->param) ||
			(Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithIntUnitParams *cc = dynamic_cast<const CallWithIntUnitParams *>(c);
		if (!Call::callMaps.contains(key, "index") && param != cc->param && param != -1)
			param = -1;
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithIntUnitParams *cc = dynamic_cast<const CallWithIntUnitParams *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "index") && param != cc->param)
			sc++;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		return std::pair<int, int>(sc, 3);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (param == -1) ? "?" : std::to_string(param);
		s += " ";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		s += (param != -1) ? std::to_string(param) : "_";
		s += ", ";
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + " unit" : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithIntUnitParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntUnitParams *>(c);
		// param
		if (Call::callMaps.contains(key, "index") && c != NULL && param != cc->param)
			ids.push_back("index");
		// check unit
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		return ids;
	}
};

class CallWithIntIntUnitParams : public Call
{

  public:
	CallWithIntIntUnitParams(std::string key, int param1, int param2, int unitId, int unitType) : Call(key), param1(param1), param2(param2)
	{
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithIntIntUnitParams(const CallWithIntIntUnitParams *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithIntIntUnitParams>(this);
	}

  private:
	CallMisc::Unit unit;
	int param1;
	int param2;

	virtual bool compare(const Call *c) const
	{
		const CallWithIntIntUnitParams *cc = dynamic_cast<const CallWithIntIntUnitParams *>(c);
		if ((Call::callMaps.contains(key, "paramIndex") && param1 != cc->param1) ||
			(Call::callMaps.contains(key, "cmdIndex") && param2 != cc->param2) ||
			(Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithIntIntUnitParams *cc = dynamic_cast<const CallWithIntIntUnitParams *>(c);
		if (!Call::callMaps.contains(key, "paramIndex") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key, "cmdIndex") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithIntIntUnitParams *cc = dynamic_cast<const CallWithIntIntUnitParams *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "paramIndex") && param1 != cc->param1)
			sc++;
		if (Call::callMaps.contains(key, "cmdIndex") && param2 != cc->param2)
			sc++;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		return std::pair<int, int>(sc, 4);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (param1 == -1) ? "?" : std::to_string(param1);
		s += " ";
		s += (param2 == -1) ? "?" : std::to_string(param2);
		s += " ";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		s += (param1 != -1) ? std::to_string(param1) : "_";
		s += ", ";
		s += (param2 != -1) ? std::to_string(param2) : "_";
		s += ", ";
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + "unit" : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithIntIntUnitParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntIntUnitParams *>(c);
		// param 1
		if (Call::callMaps.contains(key, "paramIndex") && c != NULL && param1 != cc->param1)
			ids.push_back("param1");
		// param 2
		if (Call::callMaps.contains(key, "cmdIndex") && c != NULL && param2 != cc->param2)
			ids.push_back("param2");

		// check unit
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		return ids;
	}
};

class CallWithIntUnitIntParams : public Call
{

  public:
	CallWithIntUnitIntParams(std::string key, int param1, int unitId, int unitType, int param3) : Call(key), param1(param1), param3(param3)
	{
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithIntUnitIntParams(const CallWithIntUnitIntParams *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		param3 = c->param3;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithIntUnitIntParams>(this);
	}

  private:
	CallMisc::Unit unit;
	int param1;
	int param3;

	virtual bool compare(const Call *c) const
	{
		const CallWithIntUnitIntParams *cc = dynamic_cast<const CallWithIntUnitIntParams *>(c);
		if ((Call::callMaps.contains(key, "index") && param1 != cc->param1) ||
			(Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type) ||
			(Call::callMaps.contains(key, "action") && param3 != cc->param3))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithIntUnitIntParams *cc = dynamic_cast<const CallWithIntUnitIntParams *>(c);
		if (!Call::callMaps.contains(key, "index") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key, "action") && param3 != cc->param3 && param3 != -1)
			param3 = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithIntUnitIntParams *cc = dynamic_cast<const CallWithIntUnitIntParams *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "index") && param1 != cc->param1)
			sc++;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		if (Call::callMaps.contains(key, "action") && param3 != cc->param3)
			sc++;
		return std::pair<int, int>(sc, 4);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (param1 == -1) ? "?" : std::to_string(param1);
		s += " ";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		s += " ";
		s += (param3 == -1) ? "?" : std::to_string(param3);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		s += (param1 != -1) ? std::to_string(param1) : "_";
		s += ", ";
		s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + " unit" : "_";
		s += ", ";
		s += (param3 != -1) ? std::to_string(param3) : "_";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithIntUnitIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithIntUnitIntParams *>(c);
		// param 1
		if (Call::callMaps.contains(key, "index") && c != NULL && param1 != cc->param1)
			ids.push_back("index");

		// check unit
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");

		// param 3
		if (Call::callMaps.contains(key, "action") && c != NULL && param3 != cc->param3)
			ids.push_back("action");
		return ids;
	}
};

class CallWithUnitIntUnitIntParams : public Call
{

  public:
	CallWithUnitIntUnitIntParams(std::string key, int unitId, int unitType, int param1, int targetId, int targetType, int param2) : Call(key), param1(param1), param2(param2)
	{
		unit.id = unitId;
		unit.type = unitType;
		target.id = targetId;
		target.type = targetType;
	}

	CallWithUnitIntUnitIntParams(const CallWithUnitIntUnitIntParams *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		target.id = c->target.id;
		target.type = c->target.type;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithUnitIntUnitIntParams>(this);
	}

  private:
	CallMisc::Unit unit;
	int param1, param2;
	CallMisc::Unit target;

	virtual bool compare(const Call *c) const
	{
		const CallWithUnitIntUnitIntParams *cc = dynamic_cast<const CallWithUnitIntUnitIntParams *>(c);
		if ((Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type) ||
			(Call::callMaps.contains(key, "action") && param1 != cc->param1) ||
			(Call::callMaps.contains(key, "targetId") && target.id != cc->target.id) ||
			(Call::callMaps.contains(key, "targetType") && target.type != cc->target.type) ||
			(Call::callMaps.contains(key, "synchronized") && param2 != cc->param2))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithUnitIntUnitIntParams *cc = dynamic_cast<const CallWithUnitIntUnitIntParams *>(c);
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key, "action") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key, "targetId") && target.id != cc->target.id && target.id != -1)
			target.id = -1;
		if (!Call::callMaps.contains(key, "targetType") && target.type != cc->target.type && target.type != -1)
			target.type = -1;
		if (!Call::callMaps.contains(key, "synchronized") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithUnitIntUnitIntParams *cc = dynamic_cast<const CallWithUnitIntUnitIntParams *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		if (Call::callMaps.contains(key, "action") && param1 != cc->param1)
			sc++;
		if (Call::callMaps.contains(key, "targetId") && target.id != cc->target.id)
			sc++;
		if (Call::callMaps.contains(key, "targetType") && target.type != cc->target.type)
			sc++;
		if (Call::callMaps.contains(key, "synchronized") && param2 != cc->param2)
			sc++;
		return std::pair<int, int>(sc, 6);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		s += " ";
		s += (param1 == -1) ? "?" : std::to_string(param1);
		s += " ";
		s += (target.id == -1) ? "?" : std::to_string(target.id);
		s += "_";
		s += (target.type == -1) ? "?" : std::to_string(target.type);
		s += " ";
		s += (param2 == -1) ? "?" : std::to_string(param2);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So display
		// unit attributes only for non object oriented langages.
		if (Call::callMaps.getLabel(key).find("Unit::") == std::string::npos)
		{
			s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + " unit" : "_";
			s += ", ";
		}
		s += (Call::orders_map.find(param1) != Call::orders_map.end()) ? Call::orders_map.at(param1) : "_";
		s += ", ";
		s += (Call::units_id_map.find(target.type) != Call::units_id_map.end()) ? Call::units_id_map.at(target.type) + " unit" : "_";
		s += ", ";
		// Ugly!!! only Scratch labels include " _ " token to describe parameters position. We use this
		// trick to define how to display last parameter.
		if (Call::callMaps.getLabel(key).find(" _ ") != std::string::npos)
			s += (param2 != 0) ? "WAIT" : "CONTINUE";
		else
			s += (param2 != 0) ? "true" : "false";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithUnitIntUnitIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntUnitIntParams *>(c);
		// check unit
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check param1
		if (Call::callMaps.contains(key, "action") && c != NULL && param1 != cc->param1)
			ids.push_back("action");
		// check target
		if (Call::callMaps.contains(key, "targetId") && c != NULL && target.id != cc->target.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "targetType") && c != NULL && target.type != cc->target.type)
			ids.push_back("unitType");
		// check synchronized
		if (Call::callMaps.contains(key, "synchronized") && c != NULL && param2 != cc->param2)
		{
			if (param2 == 0)
				ids.push_back("synchronous");
			else
				ids.push_back("asynchronous");
		}
		return ids;
	}
};

class CallWithUnitIntPosIntParams : public Call
{

  public:
	CallWithUnitIntPosIntParams(std::string key, int unitId, int unitType, int param1, float x, float y, int param2) : Call(key), param1(param1), param2(param2)
	{
		unit.id = unitId;
		unit.type = unitType;
		pos.x = x;
		pos.y = y;
	}

	CallWithUnitIntPosIntParams(const CallWithUnitIntPosIntParams *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		pos.x = c->pos.x;
		pos.y = c->pos.y;
		param2 = c->param2;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithUnitIntPosIntParams>(this);
	}

  private:
	CallMisc::Unit unit;
	int param1, param2;
	CallMisc::Position pos;

	virtual bool compare(const Call *c) const
	{
		const CallWithUnitIntPosIntParams *cc = dynamic_cast<const CallWithUnitIntPosIntParams *>(c);
		if ((Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type) ||
			(Call::callMaps.contains(key, "action") && param1 != cc->param1) ||
			(Call::callMaps.contains(key, "position") && pos != cc->pos) ||
			(Call::callMaps.contains(key, "synchronized") && param2 != cc->param2))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithUnitIntPosIntParams *cc = dynamic_cast<const CallWithUnitIntPosIntParams *>(c);
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key, "action") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key, "position") && fabs(pos.x - cc->pos.x) > FLOAT_EPSILON && pos.x != -1)
			pos.x = -1;
		if (!Call::callMaps.contains(key, "position") && fabs(pos.y - cc->pos.y) > FLOAT_EPSILON && pos.y != -1)
			pos.y = -1;
		if (!Call::callMaps.contains(key, "synchronized") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithUnitIntPosIntParams *cc = dynamic_cast<const CallWithUnitIntPosIntParams *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		if (Call::callMaps.contains(key, "action") && param1 != cc->param1)
			sc++;
		if (Call::callMaps.contains(key, "position"))
		{
			if (fabs(pos.x - cc->pos.x) > FLOAT_EPSILON)
				sc++;
			if (fabs(pos.y - cc->pos.y) > FLOAT_EPSILON)
				sc++;
		}
		if (Call::callMaps.contains(key, "synchronized") && param2 != cc->param2)
			sc++;
		return std::pair<int, int>(sc, 6);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		s += " ";
		s += (param1 == -1) ? "?" : std::to_string(param1);
		s += " ";
		s += (pos.x == -1) ? "?" : std::to_string(pos.x);
		s += " ";
		s += (pos.y == -1) ? "?" : std::to_string(pos.y);
		s += " ";
		s += (param2 == -1) ? "?" : std::to_string(param2);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So display
		// unit attributes only for non object oriented langages.
		if (Call::callMaps.getLabel(key).find("Unit::") == std::string::npos)
		{
			s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + " unit" : "_";
			s += ", ";
		}
		s += (Call::orders_map.find(param1) != Call::orders_map.end()) ? Call::orders_map.at(param1) : "_";
		s += ", ";
		s += (pos.x != -1) ? std::to_string(pos.x) : "_";
		s += ", ";
		s += (pos.y != -1) ? std::to_string(pos.y) : "_";
		s += ", ";
		// Ugly!!! only Scratch labels include " _ " token to describe parameters position. We use this
		// trick to define how to display last parameter.
		if (Call::callMaps.getLabel(key).find(" _ ") != std::string::npos)
			s += (param2 != 0) ? "WAIT" : "CONTINUE";
		else
			s += (param2 != 0) ? "true" : "false";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithUnitIntPosIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntPosIntParams *>(c);
		// check unit
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check param1
		if (Call::callMaps.contains(key, "action") && c != NULL && param1 != cc->param1)
			ids.push_back("action");
		// check position
		if (Call::callMaps.contains(key, "position") && c != NULL && pos != cc->pos)
			ids.push_back("position");
		// check synchronized
		if (Call::callMaps.contains(key, "synchronized") && c != NULL && param2 != cc->param2)
		{
			if (param2 == 0)
				ids.push_back("synchronous");
			else
				ids.push_back("asynchronous");
		}
		return ids;
	}
};

class CallWithUnitIntFloatIntParams : public Call
{

  public:
	CallWithUnitIntFloatIntParams(std::string key, int unitId, int unitType, int param1, float param2, int param3) : Call(key), param1(param1), param2(param2), param3(param3)
	{
		unit.id = unitId;
		unit.type = unitType;
	}

	CallWithUnitIntFloatIntParams(const CallWithUnitIntFloatIntParams *c) : Call(c)
	{
		unit.id = c->unit.id;
		unit.type = c->unit.type;
		param1 = c->param1;
		param2 = c->param2;
		param3 = c->param3;
	}

	virtual Trace::sp_trace clone() const
	{
		return std::make_shared<CallWithUnitIntFloatIntParams>(this);
	}

  private:
	CallMisc::Unit unit;
	int param1;
	float param2;
	int param3;

	virtual bool compare(const Call *c) const
	{
		const CallWithUnitIntFloatIntParams *cc = dynamic_cast<const CallWithUnitIntFloatIntParams *>(c);
		if ((Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id) ||
			(Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type) ||
			(Call::callMaps.contains(key, "action") && param1 != cc->param1) ||
			(Call::callMaps.contains(key, "param") && param2 != cc->param2) ||
			(Call::callMaps.contains(key, "synchronized") && param3 != cc->param3))
			return false;
		return true;
	}

	virtual void filter(const Call *c)
	{
		const CallWithUnitIntFloatIntParams *cc = dynamic_cast<const CallWithUnitIntFloatIntParams *>(c);
		if (!Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id && unit.id != -1)
			unit.id = -1;
		if (!Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type && unit.type != -1)
			unit.type = -1;
		if (!Call::callMaps.contains(key, "action") && param1 != cc->param1 && param1 != -1)
			param1 = -1;
		if (!Call::callMaps.contains(key, "param") && param2 != cc->param2 && param2 != -1)
			param2 = -1;
		if (!Call::callMaps.contains(key, "synchronized") && param3 != cc->param3 && param3 != -1)
			param3 = -1;
	}

	virtual std::pair<int, int> distance(const Call *c) const
	{
		const CallWithUnitIntFloatIntParams *cc = dynamic_cast<const CallWithUnitIntFloatIntParams *>(c);
		int sc = 0;
		if (Call::callMaps.contains(key, "unitId") && unit.id != cc->unit.id)
			sc++;
		if (Call::callMaps.contains(key, "unitType") && unit.type != cc->unit.type)
			sc++;
		if (Call::callMaps.contains(key, "action") && param1 != cc->param1)
			sc++;
		if (Call::callMaps.contains(key, "param") && param2 != cc->param2)
			sc++;
		if (Call::callMaps.contains(key, "synchronized") && param3 != cc->param3)
			sc++;
		return std::pair<int, int>(sc, 5);
	}

	virtual std::string getParams() const
	{
		std::string s = "";
		s += (unit.id == -1) ? "?" : std::to_string(unit.id);
		s += "_";
		s += (unit.type == -1) ? "?" : std::to_string(unit.type);
		s += " ";
		s += (param1 == -1) ? "?" : std::to_string(param1);
		s += " ";
		s += (param2 == -1) ? "?" : std::to_string(param2);
		s += " ";
		s += (param3 == -1) ? "?" : std::to_string(param3);
		return s;
	}

	virtual std::string getReadableParams() const
	{
		std::string s = "(";
		// Ugly!!! check if the label contains "Unit::" this means it is an object oriented langage
		// and then the first parameter is the object used to call this function. So display
		// unit attributes only for non object oriented langages.
		if (Call::callMaps.getLabel(key).find("Unit::") == std::string::npos)
		{
			s += (Call::units_id_map.find(unit.type) != Call::units_id_map.end()) ? Call::units_id_map.at(unit.type) + " unit" : "_";
			s += ", ";
		}
		s += (Call::orders_map.find(param1) != Call::orders_map.end()) ? Call::orders_map.at(param1) : "_";
		s += ", ";
		s += (param2 != -1) ? std::to_string(param2) : "_";
		// Ugly!!! only Scratch labels include " _ " token to describe parameters position. We use this
		// trick to define how to display last parameter.
		s += ", ";
		if (Call::callMaps.getLabel(key).find(" _ ") != std::string::npos)
			s += (param3 != 0) ? "WAIT" : "CONTINUE";
		else
			s += (param3 != 0) ? "true" : "false";
		s += ")";
		return s;
	}

	virtual std::vector<std::string> id_wrong_params(Call *c) const
	{
		std::vector<std::string> ids;
		CallWithUnitIntFloatIntParams *cc;
		if (c != NULL)
			cc = dynamic_cast<CallWithUnitIntFloatIntParams *>(c);
		// check unit
		if (Call::callMaps.contains(key, "unitId") && c != NULL && unit.id != cc->unit.id)
			ids.push_back("unitId");
		if (Call::callMaps.contains(key, "unitType") && c != NULL && unit.type != cc->unit.type)
			ids.push_back("unitType");
		// check action
		if (Call::callMaps.contains(key, "action") && c != NULL && param1 != cc->param1)
			ids.push_back("action");
		// check param
		if (Call::callMaps.contains(key, "param") && c != NULL && param2 != cc->param2)
			ids.push_back("param3");
		// check synchronized
		if (Call::callMaps.contains(key, "synchronized") && c != NULL && param3 != cc->param3)
		{
			if (param3 == 0)
				ids.push_back("synchronous");
			else
				ids.push_back("asynchronous");
		}
		return ids;
	}
};

#endif
