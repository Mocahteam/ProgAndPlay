#include "Call.h"
#include "TraceConstantList.h"
#include <cmath>

std::map<int, std::string> Call::units_id_map;  // this map has to be initialised by game engine depending on mod loaded
std::map<int, std::string> Call::orders_map;	// this map has to be initialised by game engine depending on mod loaded
std::map<int, std::string> Call::resources_map; // this map has to be initialised by game engine depending on mod loaded
const char *Call::errorsArr[] = {UNIT_NOT_FOUND_LABEL, NOT_UNIT_OWNER_LABEL, INVALID_COALITION_LABEL, TARGET_NOT_FOUND_LABEL, OUT_OF_RANGE_LABEL, INVALID_GROUP_LABEL, POSITION_OUT_OF_BOUNDS_LABEL, NULL};
const char *Call::coalitionsArr[] = {"MY_COALITION", "ALLY_COALITION", "ENEMY_COALITION", NULL};
CallMaps Call::callMaps;

Call::Call(std::string key, std::string info) : Trace(CALL, info), key(key), ind_ret(0) {}

Call::Call(const Call *c) : Trace(c)
{
	key = c->key;
	info = c->info;
	ind_ret = c->ind_ret;
	for (int i = 0; i < ind_ret; i++)
		ret[i] = c->ret[i];
}

bool Call::operator==(Trace *t)
{
	bool res = false;
	// chech if t is a call (probably an event)
	if (t->isCall())
	{
		// cast trace into call
		Call *c = dynamic_cast<Call *>(t);
		// check if we have the same key ("PP_Open" for instance)
		if (key.compare(c->key) == 0)
		{
			// If we don't have to take into account return or the return is equal
			if (!Call::callMaps.contains(key, "return") || compareReturn(c))
				// We compare the two calls (parameters)
				res = compare(c);
		}
	}
	return res;
}

double Call::getEditDistance(const Call *c) const
{
	// check if the two call have the same key ("PP_Open" for instance)
	if (key.compare(c->key) == 0)
	{
		double dis = 0;
		unsigned int tot = 1; // because they have the same key
		// check if we have to evaluate return
		if (Call::callMaps.contains(key, "return"))
		{
			tot++;
			if (!compareReturn(c))
				dis++;
		}
		// we compute parameters distance
		std::pair<int, int> sub_dis = distance(c);
		dis += sub_dis.first;
		tot += sub_dis.second;
		return dis / tot;
	}
	return 1; // the two call haven't the same key => they are not the same
}

void Call::filterCall(const Call *c)
{
	// We filter return value if it is not useful for compression and returns are differents
	if (!Call::callMaps.contains(key, "return") && !compareReturn(c))
		ind_ret = -1;
	// We filter parameters
	filter(c);
}

std::vector<std::string> Call::getListIdWrongParams(Call *c) const
{
	std::vector<std::string> ids;
	// We add return as a wrong "parameter" if return is useful in compression and returns are differents
	if (c != NULL && Call::callMaps.contains(key, "return") && !compareReturn(c))
		ids.push_back("return");
	// compute wrong parameters
	std::vector<std::string> _ids = id_wrong_params(c);
	ids.insert(ids.end(), _ids.begin(), _ids.end());
	return ids;
}

void Call::exportAsString(std::ostream &os) const
{
	for (int i = 0; i <= numTab; i++)
		os << "\t";
	if (opt)
		os << "optional ";
	os << key;
	std::string s = getParams();
	if (s.compare("") != 0)
		os << " ";
	os << s;
	s = getReturn();
	if (ind_ret > 0 || ind_ret == -1)
		os << " - " << s;
	os << std::endl;
}

std::string Call::getString() const{
	std::string ret = "";
	for (int i = 0; i <= numTab; i++)
		ret += "\t";
	if (opt)
		ret += "optional ";
	ret += key;
	std::string s = getParams();
	if (s.compare("") != 0)
		ret +=  " ";
	ret += s;
	s = getReturn();
	if (ind_ret > 0 || ind_ret == -1)
		ret +=  (" - " + s);
	ret += "\n";
	return ret;
}

void Call::exportAsCompressedString(std::ostream &os) const
{
	if (opt)
		os << "*";
	os << key << " " << std::flush;
}

unsigned int Call::length(int start, bool processOptions) const
{
	if (!processOptions && isOptional())
		return 0;
	else
		return 1;
}


std::vector<Trace::sp_trace> &Call::getLinearSequence(int start, int end){
	linearizedTraces.clear();
	linearizedTraces.push_back(this->clone());
	return linearizedTraces;
}

std::string Call::getKey() const
{
	return key;
}

std::string Call::getReturn() const
{
	if (ind_ret > -1)
	{
		std::string s = "";
		for (int i = 0; i < ind_ret; i++)
		{
			s += std::to_string(ret[i]);
			if (i < ind_ret - 1)
				s += " ";
		}
		return s;
	}
	return "?";
}

bool Call::addReturnCode(float code)
{
	if (ind_ret < MAX_SIZE_PARAMS)
	{
		ret[ind_ret++] = code;
		return true;
	}
	return false;
}

bool Call::compareReturn(const Call *c) const
{
	if (ind_ret == c->ind_ret)
	{
		for (int i = 0; i < ind_ret; i++)
		{
			if (fabs(ret[i] - c->ret[i]) > FLOAT_EPSILON)
				return false;
		}
		return true;
	}
	return false;
}

void Call::setReturn()
{
	ind_ret = -1;
}

bool Call::hasReturn() const
{
	return ind_ret != 0;
}
