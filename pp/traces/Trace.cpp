#include "Trace.h"

int Trace::numTab = 0;

Trace::Trace(TraceType type, std::string info): indSearch(-1), lenSearch(1), endSearch(0), type(type), info(info), delayed(false) {}

// Constructeur utilisé pour cloner un objet Trace
Trace::Trace(const Trace *t) {
	type = t->type;
	info = t->info;
	delayed = t->delayed;
	// aligned et parent ne peuvent pas être modifiés par le clone car getAligned() et getParent() retournent des references constantes
	aligned = t->getAligned();
	parent = t->getParent();
}

bool Trace::isSequence() const {
	return type == SEQUENCE;
}

bool Trace::isEvent() const {
	return type == EVENT;
}

bool Trace::isCall() const {
	return type == CALL;
}

bool Trace::isDelayed() const {
	return delayed;
}

void Trace::setDelayed() {
	delayed = true;
}

std::string Trace::getInfo() const {
	return info;
}

void Trace::setInfo(std::string info) {
	this->info = info;
}

const Trace::sp_trace& Trace::getParent() const {
	return parent;
}

const Trace::sp_trace& Trace::getAligned() const {
	return aligned;
}

void Trace::setParent(const sp_trace& spt) {
	parent = spt;
}

void Trace::setAligned(const sp_trace& spt) {
	aligned = spt;
}

void Trace::resetAligned() {
	aligned.reset();
}

unsigned int Trace::getLevel() const {
	unsigned int level = 0;
	sp_trace spt = parent;
	while (spt) {
		spt = spt->getParent();
		level++;
	}
	return level;
}

int Trace::inArray(const char *ch, const char *arr[]) {
	unsigned int i = 0;
	while (arr[i] != NULL) {
		if (strcmp(ch, arr[i]) == 0)
			return i;
		i++;
	}
	return -1;
}

Trace::sp_trace Trace::getNeighbour(const std::vector<Trace::sp_trace>& traces, const Trace::sp_trace& spt, int add_to_ind) {
	Trace::sp_trace nbh;
	int ind = -1;
	for (unsigned int i = 0; i < traces.size(); i++) {
		if (traces.at(i) == spt) {
			ind = i;
			break;
		}
	}
	if (ind > -1 && ind + add_to_ind >= 0 && ind + add_to_ind < (int)traces.size())
		nbh = traces.at(ind + add_to_ind);
	return nbh;
}