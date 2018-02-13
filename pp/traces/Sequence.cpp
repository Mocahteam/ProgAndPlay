#include "Sequence.h"
#include "Event.h"
#include "TraceConstantList.h"
#include "TracesParser.h"

Sequence::Sequence(std::string info, bool num_fixed) : Trace(SEQUENCE,info), num_fixed(num_fixed), num(0), pt(0), endReached(false), shared(false), root(false) {}

Sequence::Sequence(unsigned int num, bool root) : Trace(SEQUENCE), num_fixed(false), num(num), pt(0), endReached(false), shared(false), root(root) {
	addIteration(num);
}

Sequence::Sequence(const_sp_sequence sps) : Trace(sps.get()), pt(0), endReached(false), shared(false) {
	num = sps->getNum();
	num_fixed = sps->hasNumberIterationFixed();
	root = sps->isRoot();
	mergeIterationDescription(sps->getIterationDescription());
}

Sequence::Sequence(const_sp_sequence sps_up, const_sp_sequence sps_down) : Trace(SEQUENCE), pt(0), endReached(false), shared(true) {
	num = std::max(sps_up->getNum(),sps_down->getNum());
	num_fixed = sps_up->hasNumberIterationFixed() && sps_down->hasNumberIterationFixed();
	root = false;
	mergeIterationDescription(sps_up->getIterationDescription());
	mergeIterationDescription(sps_down->getIterationDescription());
}

bool Sequence::operator==(Trace *t) const {
	bool res = false;
	if (t->isSequence()) {
		Sequence *s = dynamic_cast<Sequence*>(t);
		if (traces.size() == s->size()) {
			for (unsigned int i = 0; i < traces.size(); i++) {
				if (!at(i)->operator==(s->at(i).get()))
					return false;
			}
			res = true;
		}
	}
	return res;
}

void Sequence::resetAligned() {
	Trace::resetAligned();
	for (unsigned int i = 0; i < traces.size(); i++)
		traces.at(i)->resetAligned();
}

bool Sequence::compare(Trace* t) {
	bool res = false;
	if (t->isSequence()) {
		Sequence *s = dynamic_cast<Sequence*>(t);
		Call::call_vector calls = getCalls();
		Call::call_vector s_calls = s->getCalls();
		if (calls.size() == s_calls.size()) {
			res = true;
			for (unsigned int i = 0; res && i < calls.size(); i++) {
				if (!calls.at(i)->operator==(s_calls.at(i).get()))
					res = false;
			}
		}
	}
	return res;
}

Trace::sp_trace Sequence::clone() const {
	sp_sequence sps_clone = boost::make_shared<Sequence>(shared_from_this());
	for (unsigned int i = 0; i < traces.size(); i++)
		sps_clone->addTrace(traces.at(i)->clone());
	return sps_clone;
}

void Sequence::exportAsString(std::ostream &os) const {
	numTab++;
	for (int i = 0; i < numTab; i++)
		os << "\t";
	if (delayed)
		os << "delayed ";
	if (!root)
		os << "Sequence < " << getIterartionDescriptionString(getPercentageIterationDescription()) << " >" << std::endl;
	else
		os << "Root" << std::endl;
	for (unsigned int i = 0; i < traces.size(); i++)
		traces.at(i)->exportAsString(os);
	for (int i = 0; i < numTab; i++)
		os << "\t";
	if (!root)
		os << "EndSequence" << std::endl;
	else
		os << "EndRoot" << std::endl;
	numTab--;
}

unsigned int Sequence::length() const {
	unsigned int len = 0;
	for (unsigned int i = 0; i < traces.size(); i++)
		len += traces.at(i)->length();
	return len;
}

std::vector<Trace::sp_trace>& Sequence::getTraces() {
	return traces;
}

int Sequence::getIndex(const Trace::sp_trace& spt) const {
	for (unsigned int i = 0; i < traces.size(); i++) {
		if (traces.at(i) == spt)
			return i;
	}
	return -1;
}

bool Sequence::isRoot() const {
	return root;
}

unsigned int Sequence::getNum() const {
	return num;
}

bool Sequence::hasNumberIterationFixed() const {
	return num_fixed;
}

void Sequence::addOne() {
	// We remove one appearance of the current number of iteration
	addIteration(num,-1);
	// And we increase the number of iteration and we add one more appearance
	addIteration(++num);
}

unsigned int Sequence::size() const {
	return traces.size();
}

const Trace::sp_trace& Sequence::at(unsigned int i) const {
	if (i >= traces.size())
		throw std::runtime_error("cannot access to the trace at position 'i' in 'traces' vector");
	return traces.at(i);
}

bool Sequence::addTrace(Trace::sp_trace spt, int ind) {
	if (ind >= -1 && ind <= (int)traces.size()) {
		if (ind == -1)
			traces.push_back(spt);
		else
			traces.insert(traces.begin()+ind, spt);
		spt->setParent(shared_from_this());
		return true;
	}
	return false;
}

const Trace::sp_trace& Sequence::next() {
	if (pt >= traces.size())
		throw std::runtime_error("cannot access to the trace at position 'pt' in 'traces' vector");
	const Trace::sp_trace& spt = traces.at(pt++);
	if (pt == traces.size()) {
		pt = 0;
		endReached = true;
	}
	else if (pt == 1 && endReached)
		endReached = false;
	return spt;
}

unsigned int Sequence::getPt() const {
	return pt;
}

void Sequence::reset() {
	pt = 0;
	endReached = false;
	Sequence *s;
	for (unsigned int i = 0; i < traces.size(); i++) {
		if (traces.at(i)->isSequence()) {
			s = dynamic_cast<Sequence*>(traces.at(i).get());
			s->reset();
		}
	}
}

void Sequence::clear() {
	traces.clear();
	reset();
}

bool Sequence::isEndReached() const {
	return endReached;
}

bool Sequence::isShared() const {
	return shared;
}

bool Sequence::isImplicit() const {
	return iterDesc.size() == 1 && iterDesc.find(1) != iterDesc.end() && iterDesc.at(1) == 1;
}

bool Sequence::checkDelayed() {
	if (!delayed) {
		for (unsigned i = 0; i < traces.size(); i++) {
			if (traces.at(i)->isSequence()) {
				Sequence *s = dynamic_cast<Sequence*>(traces.at(i).get());
				if (!s->checkDelayed())
					return false;
			}
			else if (!traces.at(i)->isDelayed())
				return false;
		}
		delayed = true;
	}
	return delayed;
}

const std::map<unsigned int,unsigned int>& Sequence::getIterationDescription() const {
	return iterDesc;
}

std::map<unsigned int,double> Sequence::getPercentageIterationDescription() const {
	std::map<unsigned int,double> pIterDesc;
	std::map<unsigned int,unsigned int>::const_iterator it = iterDesc.begin();
	double sum = 0;
	while(it != iterDesc.end())
		sum += (it++)->second;
	it = iterDesc.begin();
	while(it != iterDesc.end()) {
		pIterDesc.insert(std::make_pair<unsigned int,double>(it->first,it->second/sum));
		it++;
	}
	return pIterDesc;
}

void Sequence::addIteration(unsigned int nbIter, int appearance) {
	// If no key value exists, we create it and set value to "appearance"
	if (iterDesc.find(nbIter) == iterDesc.end() && appearance > 0){
		iterDesc.insert(std::make_pair<unsigned int, unsigned int>(nbIter,appearance));
	}
	// Else we add appearance to the current value
	else {
		iterDesc.at(nbIter) += appearance;
		if (iterDesc.at(nbIter) <= 0)
			iterDesc.erase(nbIter);
	}
}

void Sequence::mergeIterationDescription(const std::map<unsigned int,unsigned int>& newIterationsDesc) {
	std::map<unsigned int, unsigned int>::const_iterator it = newIterationsDesc.begin();
	while (it != newIterationsDesc.end()) {
		addIteration(it->first,it->second);
		it++;
	}
	// If we find an entry as <1:X> with X > 0 then we transform it in <X:1>. Indeed we prefer to know that X iterations occur one time than 1 iteration occurs X times because in th e last case when we transform it in percentage we will obtain <1:Y%> and we loose the number of iterations.
	if (iterDesc.find(1) != iterDesc.end() && iterDesc[1] > 1){
		addIteration(iterDesc[1]);
		iterDesc.erase(1);
	}
}

void Sequence::completeIterationDescription() {
	if (getParent()) {
		Sequence::sp_sequence sps = boost::dynamic_pointer_cast<Sequence>(getParent());
		unsigned int num = 0;
		std::map<unsigned int,unsigned int>::const_iterator it = sps->getIterationDescription().begin();
		while(it != sps->getIterationDescription().end()) {
			num += it->first * it->second;
			it++;
		}
		if (iterDesc.find(1) != iterDesc.end()) {
			it = iterDesc.begin();
			while (it != iterDesc.end()) {
				num -= it->second;
				it++;
			}
			addIteration(1,num);
		}
	}
}

double Sequence::getIterationDescriptionMeanDistance(const Sequence::sp_sequence& sps) const {
	std::map<unsigned int,double> fmap = getPercentageIterationDescription();
	std::map<unsigned int,double> smap = sps->getPercentageIterationDescription();
	double fm = 0, sm = 0;
	std::map<unsigned int,double>::const_iterator it = fmap.begin();
	while (it != fmap.end()) {
		fm += it->first * it->second;
		it++;
	}
	it = smap.begin();
	while (it != smap.end()) {
		sm += it->first * it->second;
		it++;
	}
	return std::abs(fm - sm) / (fm + sm);
}

/**
 * \brief Extraction de l'ensemble des sequences contenus dans le vecteur de traces de la séquence. La séquence appelante est inclus dans le résultat.
 *
 * \return un vecteur de sequences
 */
Sequence::sequence_vector Sequence::getSequences() {
	Sequence::sequence_vector v;
	Trace::sp_trace spt;
	Sequence::sp_sequence sps = shared_from_this();
	sps->reset();
	std::stack<Sequence::sp_sequence> stack;
	stack.push(sps);
	v.push_back(sps);
	while(!stack.empty()) {
		while (!stack.empty() && sps->isEndReached()) {
			stack.pop();
			if (!stack.empty())
				sps = stack.top();
		}
		if (!sps->isEndReached()) {
			spt = sps->next();
			if (spt->isSequence()) {
				sps = boost::dynamic_pointer_cast<Sequence>(spt);
				sps->reset();
				stack.push(sps);
				v.push_back(sps);
			}
		}
	}
	return v;
}

/**
 * \brief Extraction de l'ensemble des calls contenus dans le vecteur de traces de la séquence
 *
 * \param setMod : un booléen qui est à faux si on autorise les doublons, et à vrai sinon
 *
 * \return un vecteur de calls
 */
Call::call_vector Sequence::getCalls(bool setMod) {
	Call::call_vector v;
	Sequence::sp_sequence sps = shared_from_this();
	if (sps->length() > 0) {
		Trace::sp_trace spt;
		sps->reset();
		std::stack<Sequence::sp_sequence> stack;
		stack.push(sps);
		while(!stack.empty()) {
			while (!stack.empty() && sps->isEndReached()) {
				stack.pop();
				if (!stack.empty())
					sps = stack.top();
			}
			if (!sps->isEndReached()) {
				spt = sps->next();
				if (spt->isSequence()) {
					sps = boost::dynamic_pointer_cast<Sequence>(spt);
					sps->reset();
					stack.push(sps);
				}
				else if (spt->isCall()) {
					Call::sp_call spc = boost::dynamic_pointer_cast<Call>(spt);
					if (!setMod)
						v.push_back(spc);
					else {
						bool found = false;
						for (unsigned int j = 0; !found && j < v.size(); j++) {
							if (v.at(j)->getKey().compare(spc->getKey()) == 0)
								found = true;
						}
						if (!found)
							v.push_back(spc);
					}
				}
			}
		}
	}
	return v;
}

void Sequence::compressSequences(Sequence::sp_sequence& workingSequence, int startingPoint, bool processOnlyCall) {
	// Taille courrante des fenêtres utilisées pour les fusions. Cette taille augmente progressivement au cours de l'algorithme pour commencer par fusionner des petites séquences puis de plus en plus grandes.
	unsigned int max_length = 2;

  bool climb = false;

	// Initialisation des indices de recherche
	for (int i = startingPoint ; i < (int)workingSequence->size() ; i++)
		workingSequence->at(i)->indSearch = -1;

	// Lorsque la taille des fenêtres dépassent la longueur des traces / 2 l'algorithme peut s'arrêter. En effet les deux fenêtres couvrent la totalité de la trace et n'ont pas pu être fusionnée, il ne sert donc à rien de tenter de continuer à augmenter la taille des fenêtres
	while(max_length <= Trace::getLength(workingSequence->getTraces(),startingPoint) / 2) {
		// Indice de travail initialisé au point de départ
		unsigned int startId = startingPoint;
		// On parcours toutes les traces
		while (startId < workingSequence->size()) {
			// Si on est sur le premier élément de la trace ou si la trace courante est un évènement, on passe au suivant
			if (startId == 0 || workingSequence->at(startId)->isEvent()) {
				startId++;
				continue;
			}
			// Vérifier si les indices de la trace courante doivent être initialisés
			if (workingSequence->at(startId)->indSearch == -1) {
				// On pointe la trace en amont de la trace courrante
				workingSequence->at(startId)->indSearch = startId-1;
				// On calcule la longueur séparant cette trace (excluse) de celle pointée en amont (incluse). Cette longueur peut être supérieure à 1 si la trace en amont n'est pas un simple Call mais une séquence par exemple.
				workingSequence->at(startId)->lenSearch = workingSequence->at(startId-1)->length();
			}
			// Stoper si on a atteint la fin des traces OU si la taille de la fenêtre "up" dépasse la taille maximale fixée actuellement OU si la séquence "up" est remontée au delà du point du point de départ
			while (startId < workingSequence->size() && workingSequence->at(startId)->lenSearch <= max_length && workingSequence->at(startId)->indSearch >= startingPoint) {
				// Vérifier que la longueur de la fenêtre "up" est au moins égale à 2 car les répétitions succéssives ont déjà étaient aggrégées lors de la construction de la trace (voir TracesParser::inlineCompression)
				if (workingSequence->at(startId)->lenSearch >= 2) {
					// Création de la séquence "up" contenant les traces de la fenêtre
					Sequence::sp_sequence sps_up = boost::make_shared<Sequence>(1);
					// enregistrement de l'indice de départ de la séquence "up"
					unsigned int up_start = workingSequence->at(startId)->indSearch;
					// ajout à la séquence "up" de toutes les traces comprises dans la fenêtre
					unsigned int j = up_start;
					// si la fenêtre a une taille de 1 et que la trace précédente est une séquence, on l'utilise directement
					if (startId-j == 1 && workingSequence->at(j)->isSequence()){
					 	sps_up = boost::dynamic_pointer_cast<Sequence>(workingSequence->at(j));
						j++;
					}
					else{
						// on alimente la séquence de travail avec les éléments de la trace
						while(j < startId){
							if (processOnlyCall && workingSequence->at(j)->isSequence())
								break;
							sps_up->addTrace(workingSequence->at(j));
							j++;
						}
					}
					// Vérifier qu'il existe suffisament de traces à partir de la trace courante. En effet pour pouvoir compresser il faut qu'à partir de la trace courrante il y ait au moins autant de traces restantes que la taille de la séquence "up"
					if (sps_up->size() > 0 && Sequence::checkFeasibility(workingSequence, sps_up->length(), startId)) {
						bool found = true;
						bool mergeOccurs = false;
						// Indice max atteint en cas de fusion
						unsigned int max_fusion = 0;
						// tantque l'on trouve des séquences à fusionner et qu'on n'a pas atteint la fin de la séquence
						while (found && j < workingSequence->size()) {
							// Création de la séquence "down"
							Sequence::sp_sequence sps_down = boost::make_shared<Sequence>(1);
							// Vérifier si la prochaine trace ne serait pas une séquence déjà suffisamment grande
							if (workingSequence->at(j)->isSequence() && workingSequence->at(j)->length() >= sps_up->length()){
								// Alors on peut l'utiliser directement sans l'encapsuler dans une sequence artificielle
								sps_down = boost::dynamic_pointer_cast<Sequence>(workingSequence->at(j));
								j++;
							} else {
								// ajout à la séquence "down" d'autant de traces nécessaire pour obtenir une longueur équivalente à la séquence "up"
								while(j < workingSequence->size() && sps_down->length() < sps_up->length()){
									sps_down->addTrace(workingSequence->at(j++));
								}
							}
							// tentative de fusion des séquences "up" et "down"
							Sequence::sp_sequence sps_res = mergeSequences(sps_up, sps_down);
							// Vérifier si la fusion a réussie
							if (sps_res) {
								// dans ce cas la séquence "up" devient la fusion
								sps_up = sps_res;
								// et l'indice max atteint dans la fusion est mis à jour
								max_fusion = j;
								mergeOccurs = true;
							}
							else
								found = false;
						}
						// Si une fusion a été réalisée
						if (mergeOccurs) {
							// Activer l'attribut "delayed" si toutes les traces de la séquence sont "delayed"
							sps_up->checkDelayed();
							std::vector<Trace::sp_trace>& traces = workingSequence->getTraces();
							// Suppression des traces de toutes les traces comprises entre l'indice de début de la fenêtre "up" et l'indice max atteint dans les fusions
							traces.erase(traces.begin() + up_start, traces.begin() + max_fusion);
							// Ajout des traces de la séquence "up" (contenant la fusion des anciennes fenêtre "up" et "down")
							traces.insert(traces.begin() + up_start, sps_up);
							// Réinitialisation des indices de travail de chaque trace à partir du dernier de la dernière fusion
							std::vector<Trace::sp_trace>::iterator it = traces.begin() + up_start;
							while(it != traces.end())
								(*it++)->indSearch = -1;
							// Le nouveau point de départ est défini comme le début de la fenêtre "up" + sa longueur
							startId = up_start + max_length;
							// i = up_start + max_length + 1
							#ifdef DEBUG_PARSER
								TracesParser::osParser << "current trace compression: " << startId << std::endl;
								it = traces.begin();
								while (it != traces.end())
									(*it++)->exportAsString(TracesParser::osParser);
								TracesParser::osParser << std::endl;
							#endif
						}
						else
							climb = true;
					}
					else
						break;
				}
				else
					climb = true;
				// Augmentation de la fenêtre de recherche
				if (climb) {
					// On regarde dans la trace un cran plus haut
					workingSequence->at(startId)->indSearch--;
					// Si on est toujours dans la trace on met à jour la longueur
					if (workingSequence->at(startId)->indSearch > 0)
						workingSequence->at(startId)->lenSearch += workingSequence->at(workingSequence->at(startId)->indSearch)->length();
					climb = false;
				}
			}
			startId++;
		}
		max_length++;
	}
}

bool Sequence::checkFeasibility(Sequence::sp_sequence workingSequence, unsigned int min_length, unsigned int ind_start) {
	unsigned int len = 0;
	for (unsigned int i = ind_start; len < min_length && i < workingSequence->size(); i++) {
		if (workingSequence->at(i)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(workingSequence->at(i))->getLabel().c_str(), Event::noConcatEventsArr) > -1)
			break;
		len += workingSequence->at(i)->length();
	}
	return len >= min_length;
}

/*
 * Constructs the most generalised sequence from seq_up and seq_down. seq_up and seq_down must have the same length and must be equals.
 *
 * Returns a shared_ptr object which contains a pointer to the new sequence if the construction has succeeded, or a NULL pointer otherwise.
 *
 */
Sequence::sp_sequence Sequence::mergeSequences(Sequence::sp_sequence sps_up, Sequence::sp_sequence sps_down) {
	Sequence::sp_sequence sps_merging;
	// Check if the two sequences are equal
	if (sps_up->compare(sps_down.get())) {
		#ifdef DEBUG_PARSER
			TracesParser::osParser << "starting merge:" << std::endl;
			sps_up->exportAsString(TracesParser::osParser);
			sps_down->exportAsString(TracesParser::osParser);
			TracesParser::osParser << std::endl;
		#endif
		// Reset sequence iterators
		sps_up->reset();
		sps_down->reset();
		unsigned int len_up, len_down, pop = 0;
		bool next_up = false, next_down = false;
		// create an empty new sequence based on up sequences and as many iterations defined in down sequence. This sequence will store merging of up and down sequences
		sps_merging = boost::make_shared<Sequence>(sps_up);
		for (int i = 0 ; i < (int)sps_down->getNum() ; i++)
			sps_merging->addOne();
		// Build a new stack to proceed merged sequences
		std::stack<Sequence::sp_sequence> mergedStack;
		mergedStack.push(sps_merging);
		// Build a new stack to proceed the up parts and add the main up sequence
		std::stack<Sequence::sp_sequence> upStack;
		upStack.push(sps_up);
		// Build a new stack to proceed the down parts and add the main down sequence
		std::stack<Sequence::sp_sequence> downStack;
		downStack.push(sps_down);
		// Look for the first trace that is not an event into the up sequence and store inserted events
		std::vector<Trace::sp_trace> events;
		Trace::sp_trace spt_up = sps_up->next();
		while (!sps_up->isEndReached() && spt_up->isEvent()) {
			events.push_back(spt_up);
			spt_up = sps_up->next();
		}
		// Look for the first trace that is not an event into the down sequence and store inserted events
		Trace::sp_trace spt_down = sps_down->next();
		while (!sps_down->isEndReached() && spt_down->isEvent()) {
			events.push_back(spt_down);
			spt_down = sps_down->next();
		}
		// while the up and down stack are not empty (means it is reamaining traces to merge)
		while (!upStack.empty() && !downStack.empty()) {
			// If both current up and down traces are not sequences (this means they are Calls because Events has been cashed previously) => we merge the two Calls
			if (!spt_up->isSequence() && !spt_down->isSequence()) {
				// if some events have been found, add them to the merged sequence
				if (!events.empty()) {
					for (unsigned int i = 0; i < events.size(); i++)
						sps_merging->addTrace(events.at(i));
					// clear events list
					events.clear();
				}
				// add the up Call (down Call is equal) to the merged sequence
				sps_merging->addTrace(spt_up);
				// and filter it with the down Call
				dynamic_cast<Call*>(spt_up.get())->filterCall(dynamic_cast<const Call*>(spt_down.get()));
				// ask to get next traces into up and down sequence
				next_up = true;
				next_down = true;
			}
			// Here, at least one of the two traces is a sequence => we have to enter into sequences to find next calls to merge
			else {
				// Check if the current up trace is a sequence
				if (spt_up->isSequence()){
					// We update the working up sequence (the previous one has bee store in up stack)
					sps_up = boost::dynamic_pointer_cast<Sequence>(spt_up);
				}
				// Check if the current down trace is a sequence
				if (spt_down->isSequence()){
					// We update the working down sequence (the previous one has bee store in down stack)
					sps_down = boost::dynamic_pointer_cast<Sequence>(spt_down);
				}
				// compute length of up and down traces
				len_up = spt_up->length();
				len_down = spt_down->length();
				// Check if up and down traces are sequences with the same length => We have to stack these two sequences and build a new one to receive merging
				if (spt_up->isSequence() && spt_down->isSequence() && len_up == len_down) {
					// So we build a new empty sequence based on up and down working sequences (the previous merged sequence has bee store in mergedStack)
					sps_merging = boost::make_shared<Sequence>(sps_up, sps_down);
					// Add on stacks up and down working sequences in order to enter possible sub-sequences
					upStack.push(sps_up);
					downStack.push(sps_down);
					// ask to get next traces into up and down sequences
					next_up = true;
					next_down = true;
				}

				// Check if up is a sequence but not down OR up sequence is greater than down sequence => Because we have at least one sequence we have to build a new one to receive merging
				else if ((spt_up->isSequence() && !spt_down->isSequence()) || len_up > len_down) {
					// We build a new empty sequence based on up working sequence (the previous merged sequence has bee store in mergedStack)
					sps_merging = boost::make_shared<Sequence>(sps_up);
					// if up trace is a sequence and not down trace (means it's a Call)
					if (spt_up->isSequence() && !spt_down->isSequence())
						// this sequence has been done one time only once (indead a Call means it's possible to run this sequence only once)
						sps_merging->addIteration(1,1);
					// Add on up stack the up working sequence in order to enter possible sub-sequences
					upStack.push(sps_up);
					// ask to get the next trace into up sequence
					next_up = true;
				}
				// Check if down is a sequence but not up OR down sequence is greater than up sequence => Because we have at least one sequence we have to build a new one to receive merging
				else if ((!spt_up->isSequence() && spt_down->isSequence()) || len_up < len_down) {
					// We build a new empty sequence based on down working sequence (the previous merged sequence has bee store in mergedStack)
					sps_merging = boost::make_shared<Sequence>(sps_down);
					// if down is a sequence and not up trace (means it's a Call)
					if (!spt_up->isSequence() && spt_down->isSequence())
						// this sequence has been done one time only once (indead a Call means it's possible to run this sequence only once)
						sps_merging->addIteration(1,1);
					// Add on down stack the down working sequence in order to enter possible sub-sequences
					downStack.push(sps_down);
					// ask to get the next trace into down sequence
					next_down = true;
				}
				// Stack the new merging sequence
				mergedStack.push(sps_merging);
			}
			// If we have to get the next one into the up sequence and the end of up sequence is reached => then we have to pop this sequence and continue into parent
			while (next_up && !upStack.empty() && sps_up->isEndReached()) {
				// We pop the up stack
				upStack.pop();
				// If up stack is not empty we get the top sequence (i.e. the parent of the poped sequence)
				if (!upStack.empty()) {
					sps_up = upStack.top();
					// Count that a new pop occurs
					pop++;
				}
			}
			// If we have to get the next one into down sequence and the end of down sequence is reached => then we have to pop this sequence and continue into parent
			while (next_down && !downStack.empty() && sps_down->isEndReached()) {
				// We pop the down stack
				downStack.pop();
				// If down stack is not empty we get the top sequence (i.e. the parent of the poped sequence)
				if (!downStack.empty()) {
					sps_down = downStack.top();
					// Count that a new pop occurs
					pop++;
				}
			}
			// For each pop
			while (pop > 0) {
				// if merging sequence is shared (means it was built from two sequences) => we consume two pop
				if (sps_merging->isShared()) {
					pop -= 2;
				}
				// else we concume only one pop
				else {
					pop--;
				}
				// We remove the top of the stack
				mergedStack.pop();
				// and we add the working merged sequence to the sequence at the top of the stack
				mergedStack.top()->addTrace(sps_merging);
				sps_merging->completeIterationDescription();
				// then we get the new working sequence
				sps_merging = mergedStack.top();
			}
			// If we have processed the up sequence and the up stack is not empty
			if (next_up && !upStack.empty()) {
				// Look for the next trace that is not an event into the up sequence and store inserted events
				spt_up = sps_up->next();
				while (!sps_up->isEndReached() && spt_up->isEvent()) {
					events.push_back(spt_up);
					spt_up = sps_up->next();
				}
				// reset up flag
				next_up = false;
			}
			// If we have processed the down sequence and the down stack is not empty
			if (next_down && !downStack.empty()) {
				// Look for the next trace that is not an event into the down sequence and store inserted events
				spt_down = sps_down->next();
				while (!sps_down->isEndReached() && spt_down->isEvent()) {
					events.push_back(spt_down);
					spt_down = sps_down->next();
				}
				// reset up flag
				next_down = false;
			}
		}
		#ifdef DEBUG_PARSER
			TracesParser::osParser << "merge result" << std::endl;
			sps_merging->exportAsString(TracesParser::osParser);
			TracesParser::osParser << std::endl;
		#endif
	}
	return sps_merging;
}
