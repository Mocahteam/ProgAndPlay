#include "Trace.h"

int Trace::numTab = 0;

Trace::Trace(TraceType type, std::string info) : type(type), info(info), opt(false) {}

// Constructeur utilisé pour cloner un objet Trace
Trace::Trace(const Trace *t)
{
	type = t->type;
	info = t->info;
	// aligned et parent ne peuvent pas être modifiés par le clone car getAligned() et getParent() retournent des references constantes
	aligned = t->getAligned();
	parent = t->getParent();
	opt = t->opt;
}

bool Trace::isSequence() const
{
	return type == SEQUENCE;
}

bool Trace::isCall() const
{
	return type == CALL;
}

std::string Trace::getInfo() const
{
	return info;
}

void Trace::setInfo(std::string info)
{
	this->info = info;
}

const Trace::wp_trace &Trace::getParent() const
{
	return parent;
}

const Trace::wp_trace &Trace::getAligned() const
{
	return aligned;
}

void Trace::setParent(const sp_trace &spt)
{
	parent = spt;
}

void Trace::setAligned(const sp_trace &spt)
{
	aligned = spt;
}

void Trace::resetAligned()
{
	aligned.reset();
}

unsigned int Trace::getLevel() const
{
	unsigned int level = 0;
	sp_trace spt = parent.lock();
	while (spt)
	{
		spt = spt->getParent().lock();
		level++;
	}
	return level;
}

int Trace::inArray(const char *ch, const char *arr[])
{
	unsigned int i = 0;
	while (arr[i] != NULL)
	{
		if (strcmp(ch, arr[i]) == 0)
			return i;
		i++;
	}
	return -1;
}

unsigned int Trace::getLength(const std::vector<Trace::sp_trace> &traces, int ind_start, int ind_end, bool processOptions)
{
	if (ind_end == -1)
		ind_end = traces.size();
	if (ind_start < ind_end && (ind_start < 0 || ind_start >= (int)traces.size()))
		throw std::runtime_error("invalid index used in Trace::getLength() function");
	unsigned int len = 0;
	for (int i = ind_start; i < ind_end; i++)
		len += traces.at(i)->length(0, processOptions);
	return len;
}

bool Trace::isOptional(bool checkParent) const
{
	if (!checkParent || !parent.lock())
		return opt;
	else
		return opt || parent.lock()->isOptional(true);
}

void Trace::setOptional(bool state)
{
	opt = state;
}

void Trace::exportAsString(std::ostream &os, const std::vector<sp_trace> &traces, int ind_start, int ind_end)
{
	if (ind_end == -1)
		ind_end = traces.size();
	if (ind_start < ind_end && (ind_start < 0 || ind_start >= (int)traces.size()))
		throw std::runtime_error("invalid index used in Trace::exportAsString() function");
	if (ind_start > 0)
		os << "..." << std::endl;
	for (int i = ind_start; i < ind_end; i++)
		traces.at(i)->exportAsString(os);
	if (ind_end < (int)traces.size())
		os << "..." << std::endl;
}


