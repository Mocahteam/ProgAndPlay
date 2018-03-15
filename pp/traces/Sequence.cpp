#include "Sequence.h"
#include "Event.h"
#include "TraceConstantList.h"
#include "TracesParser.h"

// Taille de la fenêtre maximale pour mettre en cache les traces intercallées lors de la tentative de rotation des séquences
#define MAX_OPT_WINDOW 10

Sequence::Sequence(std::string info, bool num_fixed) : Trace(SEQUENCE,info), num_fixed(num_fixed), num(0), pt(0), endReached(false), shared(false), root(false) {}

Sequence::Sequence(unsigned int num, bool root, bool opt) : Trace(SEQUENCE), num_fixed(false), num(num), pt(0), endReached(false), shared(false), root(root), opt(opt) {
	addIteration(num);
}

Sequence::Sequence(const_sp_sequence sps) : Trace(sps.get()), pt(0), endReached(false), shared(false) {
	num = sps->getNum();
	num_fixed = sps->hasNumberIterationFixed();
	root = sps->isRoot();
	opt = sps->isOptional();
	mergeIterationDescription(sps->getIterationDescription());
}

Sequence::Sequence(const_sp_sequence sps_up, const_sp_sequence sps_down) : Trace(SEQUENCE), pt(0), endReached(false), shared(true) {
	num = std::max(sps_up->getNum(),sps_down->getNum());
	num_fixed = sps_up->hasNumberIterationFixed() && sps_down->hasNumberIterationFixed();
	root = false;
	opt = sps_up->isOptional() || sps_down->isOptional();
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
	if (opt)
		os << "optional ";
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

void Sequence::exportAsCompressedString(std::ostream &os) const {
	if (numTab == 0)
		os << " ";
	numTab++;
	if (!root)
		os << "Sequence" << " ";
	else
		os << "Root" << " ";
	for (unsigned int i = 0; i < traces.size(); i++)
		traces.at(i)->exportAsCompressedString(os);
	if (!root)
		os << "EndSequence" << " ";
	else
		os << "EndRoot" << std::endl;
	numTab--;
	if (numTab == 0)
		os << std::endl;
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

std::vector<Trace::sp_trace> Sequence::getSubTraces(int start) {
	std::vector<Trace::sp_trace> remainingTraces;
	start = start < 0 ? 0 : start;
	for (int i = start ; i < (int)traces.size() ; i++){
		remainingTraces.push_back(traces.at(i));
	}
	return remainingTraces;
}

int Sequence::getIndex(const Trace::sp_trace& spt) const {
	for (unsigned int i = 0; i < traces.size(); i++) {
		if (traces.at(i) == spt)
			return i;
	}
	return -1;
}

bool Sequence::findInTraces(Sequence::sp_sequence& toFind, int start, int end) {
	int i = 0; // indice de parcours dans "toFind"
	int j = start < 0 ? 0 : start; // indice de parcours dans this
	while (i < (int)toFind->size() && j < end && j < (int)this->size()){
		if (toFind->at(i)->operator==(this->at(j).get()))
			i++; // si l'élément de toFind est égal à l'élément de this => on avance dans toFind
		j++; // Dans tous les cas on progresse dans this
	}
	// si on est arrivé au bout de la sequence "toFind" c'est qu'on a trouvé des équivalence dans this. On peut donc retourner "true"
	return i >= (int)toFind->size();
}

bool Sequence::isRoot() const {
	return root;
}

bool Sequence::isOptional() const {
	return opt;
}

void Sequence::setOptional(bool state) {
	opt = state;
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

void Sequence::setPtAndEndState(unsigned int pos, bool endState){
	pt = pos;
	endReached = endState;
	if (pos >= traces.size())
		pt = 0;
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

// Parcours une trace linéarisée et supprime le premier Call
void Sequence::removeFirstCall (std::vector<Trace::sp_trace>& linearTraces){
	unsigned int i = 0;
	// Avancer dans la trace linéarisée jusqu'à atteindre le premier Call ou Event
	while (i < linearTraces.size() && linearTraces.at(i)->isSequence())
		i++;
	// Si on a trouvé un Call
	if (i < linearTraces.size()){
		// On le supprime
		linearTraces.erase(linearTraces.begin()+i);
		// La suppression du Call peut générer une séquence vide qui peuvent éventuellement être imbriquées => Suppression des séquences vides
		while (i > 1 && linearTraces.at(i-1)->isSequence() && linearTraces.at(i)->isSequence()){
			Sequence * s1 = dynamic_cast<Sequence*>(linearTraces.at(i-1).get());
			Sequence * s2 = dynamic_cast<Sequence*>(linearTraces.at(i).get());
			if (s1->getInfo().compare("Begin") == 0 && s2->getInfo().compare("End") == 0){
				// Suppression de ces deux traces qui marquent une séquence vide
				linearTraces.erase(linearTraces.begin()+(i-1), linearTraces.begin()+(i+1));
			}
			i--;
		}
	}
}

// Compte le nombre de Call alignés et optionnels entre deux séquences
// Retourne un couple (nbAlign, nbOpt) où nbAlign indique le nombre de traces alignées et nbOpt indique le nombre de traces optionnelles
std::pair<int, int> Sequence::getNbAlignAndOpt(std::vector<Trace::sp_trace>& linearTraces1, std::vector<Trace::sp_trace>& linearTraces2){
	unsigned int i = 0, j = 0;
	int nbOpt = 0, nbAlign = 0;
	// pile permettant de stocker l'ordre d'entrée dans les séquences pour ne sortir d'une séquence que si elle n'est pas bloquée
	std::stack<int> stack;
	// Parcours de la trace linéarisée 1
	while (i < linearTraces1.size()){
		if (linearTraces1.at(i)->isSequence()){
			Sequence * s1 = dynamic_cast<Sequence*>(linearTraces1.at(i).get());
			// Si c'est une entrée de séquence on insère dans la pile la valeur 1 pour indiquer qu'on entre dans une séquence des traces 1
			if (s1->getInfo().compare("Begin") == 0){
				TracesParser::osParser << "[A ";
				stack.push(1);
				// Si la trace 2 est aussi sur une entrée de séquence, on monte aussi pour garder les deux synchronisées
				if (j < linearTraces2.size() && linearTraces2.at(j)->isSequence()){
					Sequence * s2 = dynamic_cast<Sequence*>(linearTraces2.at(j).get());
					if (s2->getInfo().compare("Begin") == 0){
						TracesParser::osParser << "[B ";
						stack.push(2);
						j++;
					}
				}
			}
			else if (s1->getInfo().compare("End") == 0){
				// On ne peut sortir de cette séquence que si la tête de la pile correspond à une séquence 1
				// Dépiler jusqu'à ce que la tête de la pile soit du type 1
				while (j < linearTraces2.size() && !stack.empty() && stack.top() != 1){
					// Avancer dans la trace 2 et compter toutes ces traces comme optionnelles
					if (linearTraces2.at(j)->isSequence()){
						Sequence * s2 = dynamic_cast<Sequence*>(linearTraces2.at(j).get());
						if (s2->getInfo().compare("Begin") == 0){
							TracesParser::osParser << "[B ";
							stack.push(2);
						}
						else if (s2->getInfo().compare("End") == 0){
							TracesParser::osParser << "B] ";
							stack.pop();
						}
						else std::runtime_error("Sequence::getNbAlignAndOpt => linear traces malformed");
					} else{
						TracesParser::osParser << "(B" << j << ") ";
						nbOpt++;
					}
					j++;
				}
				if (!stack.empty()){
					TracesParser::osParser << "A] ";
					stack.pop(); // Libérer cette séquence
				}
			} else {
				// On ne devrait pas trouver dans une trace linéariser des séquences non tagué "Begin" ou "End"
				throw std::runtime_error("Sequence::getNbAlignAndOpt => linear traces malformed");
			}
		} else {
			// On est sur un Call de la trace 1, on cherche donc son équivalent dans la trace 2
			// sauvegarder la position des compteurs i et j pour pouvoir les restaurer si besoin
			int k = i;
			int l = j;
			int nbOpt2 = 0;
			std::stack<int> stack2(stack);
			std::ostringstream oss;
			// Progresser dans la séquence 2 jusqu'à trouver un alignement correct
			while (i < linearTraces1.size() && j < linearTraces2.size() && !linearTraces1.at(i)->operator==(linearTraces2.at(j).get())){
				// Avancer dans la trace 2
				if (linearTraces2.at(j)->isSequence()){
					Sequence * s2 = dynamic_cast<Sequence*>(linearTraces2.at(j).get());
					if (s2->getInfo().compare("Begin") == 0){
						oss << "[B ";
						stack2.push(2);
					}
					else if (s2->getInfo().compare("End") == 0){
						// On ne peut sortir de cette séquence que si la tête de la pile correspond à une séquence 2
						// Dépiler jusqu'à ce que la tête de la pile soit du type 2
						while (!stack2.empty() && stack2.top() != 2){
							// Avancer dans la trace 1 et compter toutes ces traces comme optionnelles
							if (linearTraces1.at(i)->isSequence()){
								Sequence * s1 = dynamic_cast<Sequence*>(linearTraces1.at(i).get());
								if (s1->getInfo().compare("Begin") == 0){
									oss << "[A ";
									stack2.push(1);
								}
								else if (s1->getInfo().compare("End") == 0){
									oss << "A] ";
									stack2.pop();
								}
								else std::runtime_error("Sequence::getNbAlignAndOpt => linear traces malformed");
							} else{
								oss << "(A" << i << ") ";
								nbOpt2++;
							}
							i++;
						}
						if (!stack2.empty()){
							oss << "B] ";
							stack2.pop(); // Libérer cette séquence
						}
					}
					else std::runtime_error("Sequence::getNbAlignAndOpt => linear traces malformed");
				} else {
					oss << "(B" << j << ") ";
					nbOpt2++;
				}
				j++;
			}
			// Si la fin des traces 2 a été atteint
			if (j >= linearTraces2.size()){
				// restaurer les compteurs pour repartir après le dernier alignement trouvé
				i = k;
				j = l;
				TracesParser::osParser << "(A" << i << ") ";
				nbOpt++;
			} else{
				// On a trouvé un alignement
				nbAlign++;
				TracesParser::osParser << oss.str() << "{AB" << i << "/" << j << "} ";
				j++;
				nbOpt += nbOpt2; // prise en compte des sauts dans les traces 2
				stack = stack2; // prise en compte de l'état de la pile
				// Si on est en bout de séquences, on les libèrent dans la mesure du possible
				bool stop = false;
				while (i+1 < linearTraces1.size() && j < linearTraces2.size() && !stack.empty() && !stop){
					stop = true;
					if (linearTraces1.at(i+1)->isSequence() && stack.top() == 1){
						Sequence * s = dynamic_cast<Sequence*>(linearTraces1.at(i+1).get());
						if (s->getInfo().compare("End") == 0){
							TracesParser::osParser << "A] ";
							stack.pop();
							i++;
							stop = false;
						}
					}
					if (linearTraces2.at(j)->isSequence() && stack.top() == 2){
						Sequence * s = dynamic_cast<Sequence*>(linearTraces2.at(j).get());
						if (s->getInfo().compare("End") == 0){
							TracesParser::osParser << "B] ";
							stack.pop();
							j++;
							stop = false;
						}
					}
				}
			}
		}
		i++;
	}
	// prise en compte de ce qui reste dans les traces 2
	for (unsigned int i = j ; i < linearTraces2.size() ; i++)
		if (!linearTraces2.at(i)->isSequence()){
			TracesParser::osParser << "(B" << i << ") ";
			nbOpt++;
		}
	TracesParser::osParser << std::endl;
	return std::make_pair<int, int>(nbAlign, nbOpt);
}

// Retourne le meilleur score d'alignement entre deux traces linéarisées
std::pair<int, int> Sequence::getMaxAlignAndMinOpt (std::vector<Trace::sp_trace>& linearTraces1, std::vector<Trace::sp_trace>& linearTraces2){
	// Affichage du contenu des traces liéarisées
	/*TracesParser::osParser << "Up:" << std::endl;
	for (unsigned int i = 0 ; i < linearTraces1.size() ; i++)
		linearTraces1.at(i)->exportAsString(TracesParser::osParser);
	TracesParser::osParser << "Down:" << std::endl;
	for (unsigned int i = 0 ; i < linearTraces2.size() ; i++)
		linearTraces2.at(i)->exportAsString(TracesParser::osParser);*/
	// compter le nombre de Call (et d'Event) dans la première trace linéarisée
	unsigned int nbCalls1 = 0;
	for (unsigned int i = 0 ; i < linearTraces1.size() ; i++)
		if (!linearTraces1.at(i)->isSequence()) nbCalls1++;
	// compter le nombre de Call (et d'Event) dans la seconde trace linéarisée
	unsigned int nbCalls2 = 0;
	for (unsigned int i = 0 ; i < linearTraces2.size() ; i++)
		if (!linearTraces2.at(i)->isSequence()) nbCalls2++;
	// Initialisation des valeurs par défaut
	std::pair <int, int> ret(0, nbCalls1+nbCalls2);
	// Pour chaque Call de la séquence 1
	for (unsigned int i = 0 ; i < nbCalls1 ; i++){
		// Calculer les scores entre les deux traces linéarisées
		std::pair<int, int> tmp = Sequence::getNbAlignAndOpt(linearTraces1, linearTraces2);
		// Mettre à jour les meilleurs score si besoin
		if (tmp.first > ret.first || (tmp.first == ret.first && tmp.second+(int)i < ret.second)){
			ret.first = tmp.first;
			ret.second = tmp.second+i;
		}
		// réduction de la première trace
		Sequence::removeFirstCall(linearTraces1);
		// Si le meilleur nombre d'alignement dépasse la longueur restante de la première trace, on peut couper la boucle car on ne trouvera pas mieux
		if ((int)linearTraces1.size() < ret.first)
			break;
	}
	//TracesParser::osParser << "Result => " << ret.first << "/" << ret.second << std::endl;
	return ret;
}

/**
 * \brief Linéarisation de l'ensemble des traces. Le vecteur retourné peut contenir des objets de type Sequence qui marquent le début ou la fin d'une séquence. Toutes les traces comprise entre un début et un fin de séquence font parties de la séquence.
 * Exemple, la séquence suivante (C modélisent un Call et S une séquence) :
 *       C
 *      \ /
 *   C C S
 *   \_ _/
 * C C S C
 *
 * Est linéarisée de la manière suivante où Sd est une séquence vide dont le champ info maqrue un début de séquence et Sf est une séquence vide dont le champ info maqrue une fin de séquence :
 * C C Sd C C Sd C Sf Sf C
 *
 * \param start point de départ de linéarisation de la séquence
 *
 * \return un vecteur de trace représentant une version linéarisée des traces
 */
std::vector<Trace::sp_trace>& Sequence::getLinearSequence(int start){
	linearizedTraces.clear();
	Sequence::sp_sequence sps = shared_from_this();
	if (sps->length() > 0) {
		Trace::sp_trace spt;
		unsigned int back_pt = sps->getPt();
		bool back_endReached = sps->isEndReached();
		sps->reset();
		for (int i = 0 ; i < start ; i++)
			sps->next();
		std::stack<Sequence::sp_sequence> stack;
		stack.push(sps);
		linearizedTraces.push_back(boost::make_shared<Sequence>("Begin", -1));
		while(!stack.empty()) {
			while (!stack.empty() && (sps->isEndReached() || sps->length() == 0)) {
				stack.pop();
				linearizedTraces.push_back(boost::make_shared<Sequence>("End", -1));
				if (!stack.empty())
					sps = stack.top();
			}
			if (!sps->isEndReached() && sps->length() > 0) {
				spt = sps->next();
				if (spt->isSequence()) {
					sps = boost::dynamic_pointer_cast<Sequence>(spt);
					sps->reset();
					stack.push(sps);
					linearizedTraces.push_back(boost::make_shared<Sequence>("Begin", -1));
				}
				else {
					linearizedTraces.push_back(spt);
				}
			}
		}
		sps->setPtAndEndState(back_pt, back_endReached);
	}
	return linearizedTraces;
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
		if (!sps->isEndReached() && sps->length() > 0) {
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
			if (!sps->isEndReached() && sps->length() > 0) {
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

void Sequence::findAndAggregateSuccessiveSequences(Sequence::sp_sequence& workingSequence, int startingPoint, bool processOnlyCall) {
	// Taille courrante des fenêtres utilisées pour les fusions. Cette taille augmente progressivement au cours de l'algorithme pour commencer par fusionner des petites séquences puis de plus en plus grandes.
	unsigned int max_length = 2;

  bool climb = false;

	TracesParser::osParser << "WorkingSequence (start at " << startingPoint << "):" << std::endl;
	workingSequence->exportAsString(TracesParser::osParser);

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
							if (processOnlyCall && !workingSequence->at(j)->isCall())
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
							// tentative de fusion des séquences "up" et "down". Si processOnlyCall == true alors on fait une fusion stricte, sinon on accepte les options
							Sequence::sp_sequence sps_res = mergeEquivalentSequences(sps_up, sps_down);
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

int computeLength (std::vector<Trace::sp_trace> _traces){
	unsigned int len = 0;
	for (unsigned int i = 0; i < _traces.size(); i++)
		len += _traces.at(i)->length();
	return len;
}

Sequence::sp_sequence Sequence::mergeEquivalentSequences(Sequence::sp_sequence sps_up, Sequence::sp_sequence sps_down, bool strict) {
	TracesParser::osParser << "###############################################" << std::endl;
	TracesParser::osParser << "###############################################" << std::endl;
	#ifdef DEBUG_PARSER
		TracesParser::osParser << std::endl;
		TracesParser::osParser << "start merging:" << std::endl;
		sps_up->exportAsString(TracesParser::osParser);
		TracesParser::osParser << "WITH" << std::endl;
		sps_down->exportAsString(TracesParser::osParser);
	#endif
	// Reset sequence iterators
	sps_up->reset();
	sps_down->reset();
	unsigned int len_up, len_down, pop = 0;
	int nbAlign = 0, nbOpt = 0;
	bool next_up = false, next_down = false;
	// create an empty new sequence based on up sequences and as many iterations defined in down sequence. This sequence will store merging of up and down sequences
	Sequence::sp_sequence sps_merging = boost::make_shared<Sequence>(sps_up);
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
	// Build a new stack to store the order of last stack used
	std::stack<char> orderStack;
	orderStack.push('x'); // 'x' means both, 'u' up sequence and 'd' down sequence
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
	while (!upStack.empty() || !downStack.empty()) {
		TracesParser::osParser << "===============================================" << std::endl;
		// Check if current up and down trace are defined
		if (!spt_up && !spt_down){
			// There is a problem with the stacks, we can't pop and we can't get next traces
			#ifdef DEBUG_PARSER
				TracesParser::osParser << "Sequence::mergeEquivalentSequences => WARNING no traces to process." << std::endl;
			#endif
			// so we cancel merging and return an empty sequence
			Sequence::sp_sequence sps_empty;
			return sps_empty;
		} else if ((!spt_up || !spt_down) && strict){
			// Si une des deux traces n'est pas définie, celà signifie que l'on ne peut dépiler la séquence pour passer à la trace suivante car elle est bloqué par le dépilement avant de l'autre séquence. Exemple :
			// Exemple : sps_up = [A[BC*]D]; sps_down = [AB[C*D]] => soit la position de l'étoile marquant la position de la tête de lecture dans le processus de fusion. Dans cet exemple l'empilement des séquence est "root|up|down" il faudrait dépiler up pour passer au symbole suivant, hors on ne peut pas car le dernier empilement a eut lieu sur la sequence down. Il faut donc attendre d'atteindre la fin du down pour pouvoir d'épiler le down et enfin le up. Cette situation doit donc introduire des séquences optionnelles pour régler ce problème. Comme nous sommes ici en mode strict, on retourne une séquence vide.
			Sequence::sp_sequence sps_empty;
			return sps_empty;
			// Note : Si cette précaution n'est pas prise celà entraine la construction d'une fusion fausse. En effet si on n'attend pas la fin de la séquence down le résultat de la fusion serait : [A[B[C]D]]. Cette fusion ne permet pas de représenter la séquence ABCBCD pourtant valide avec la séquence up. La solution passe par l'ajout de partie optionnelle : [A[B[C(D)]](D)]
		} else{
			if (!spt_up){ // nous sommes ici en non strict (cf "if ((!spt_up || !spt_down) && strict)")
				// Si le pointeur de la séquence up n'est pas défini c'est qu'il a été impossible de dépiler une séquence up car l'algo est bloqué par l'empilement d'une séquence down. Il faut terminer cette séquence down pour pouvoir continuer sur la séquence up. Donc on met toutes les traces de la séquence down comme optionnelles
				// construction de la sequence optionnelle
				Sequence::sp_sequence sps_opt = boost::make_shared<Sequence>(1, false, true);
				// On ajoute dans la séquence optionnelle tous ce qui reste dans la séquence down
				sps_opt->addTrace(spt_down);
				while (!sps_down->isEndReached()) {
					sps_opt->addTrace(sps_down->next());
				}
				// and we add this optionnal sequence into the merged sequence
				sps_merging->addTrace(sps_opt);
				next_down = true;
			} else if (!spt_down){ // nous sommes ici en non strict (cf "if ((!spt_up || !spt_down) && strict)")
				// Si le pointeur de la séquence down n'est pas défini c'est qu'il a été impossible de dépiler une séquence down car l'algo est bloqué par l'empilement d'une séquence up. Il faut terminer cette séquence up pour pouvoir continuer sur la séquence down. Donc on met toutes les traces de la séquence up comme optionnelles
				// construction de la sequence optionnelle
				Sequence::sp_sequence sps_opt = boost::make_shared<Sequence>(1, false, true);
				// On ajoute dans la séquence optionnelle tous ce qui reste dans la séquence up
				sps_opt->addTrace(spt_up);
				while (!sps_up->isEndReached()) {
					sps_opt->addTrace(sps_up->next());
				}
				// and we add this optionnal sequence into the merged sequence
				sps_merging->addTrace(sps_opt);
				next_up = true;
			} else {
				TracesParser::osParser << "Do we have to align:" << std::endl;
				spt_up->exportAsString(TracesParser::osParser);
				TracesParser::osParser << "WITH" << std::endl;
				spt_down->exportAsString(TracesParser::osParser);
				// Here we are sure that stp_up and stp_down are defined
				// Création d'une trace pour recevoir les options
				Trace::sp_trace spt_opt;
				// Initialisation de l'indice de départ pour la linéarisation de la séquence Up (Sequence::getPt() pointe sur le prochain élément à lire, d'où le -1 pour récupérer la position de l'élément courant)
				unsigned int startLinearisationUp = sps_up->getPt()-1;
				// Si la fin de la séquence a été atteinte, Sequence::getPt() pointe sur 0 hors nous souhaitons dans notre cas le dernier élément de la séquence, on corrige donc l'indice de départ
				if (sps_up->isEndReached()) startLinearisationUp = sps_up->size()-1;
				// Initialisation de l'indice de départ pour la linéarisation de la séquence Down (Sequence::getPt() pointe sur le prochain élément à lire, d'où le -1 pour récupérer la position de l'élément courant)
				unsigned int startLinearisationDown = sps_down->getPt()-1;
				// Si la fin de la séquence a été atteinte, Sequence::getPt() pointe sur 0 hors nous souhaitons dans notre cas le dernier élément de la séquence, on corrige donc l'indice de départ
				if (sps_down->isEndReached()) startLinearisationDown = sps_down->size()-1;
				// Obtenir le meilleur score d'alignement entre la trace up à partir de la trace courante et la trace down à partir de la trace courante
				std::pair<int, int> pair_scoreUpDown = Sequence::getMaxAlignAndMinOpt(sps_up->getLinearSequence(startLinearisationUp), sps_down->getLinearSequence(startLinearisationDown));
				TracesParser::osParser << "Result for h1::q1 and h2::q2 => " << pair_scoreUpDown.first << "/" << pair_scoreUpDown.second << "\n" << std::endl;
				// Obtenir le meilleur score d'alignement entre la trace up à partir de la trace SUIVANTE la trace courante et la trace down à partir de la trace courante
				std::pair<int, int> pair_scoreNextUpDown = Sequence::getMaxAlignAndMinOpt(sps_up->getLinearSequence(startLinearisationUp+1), sps_down->getLinearSequence(startLinearisationDown));
				TracesParser::osParser << "Result for q1 and h2::q2 => " << pair_scoreNextUpDown.first << "/" << pair_scoreNextUpDown.second << "\n" << std::endl;
				// Obtenir le meilleur score d'alignement entre la trace up à partir de la trace courante et la trace down à partir de la trace SUIVANTE la trace courante
				std::pair<int, int> pair_scoreUpNextDown = Sequence::getMaxAlignAndMinOpt(sps_up->getLinearSequence(startLinearisationUp), sps_down->getLinearSequence(startLinearisationDown+1));
				TracesParser::osParser << "Result for h1::q1 and q2 => " << pair_scoreUpNextDown.first << "/" << pair_scoreUpNextDown.second << "\n" << std::endl;
				// Interprétation des scores
				int scoreUpDown = 0;
				int scoreNextUpDown = 0;
				int scoreUpNextDown = 0;
				// scoreUpDown meilleur que scoreNextUpDown
				if (pair_scoreUpDown.first > pair_scoreNextUpDown.first || (pair_scoreUpDown.first == pair_scoreNextUpDown.first && pair_scoreUpDown.second < pair_scoreNextUpDown.second))
					scoreUpDown+=2;
				// scoreUpDown meilleur que scoreUpNextDown
				if (pair_scoreUpDown.first > pair_scoreUpNextDown.first || (pair_scoreUpDown.first == pair_scoreUpNextDown.first && pair_scoreUpDown.second < pair_scoreUpNextDown.second))
					scoreUpDown+=2;
				// scoreNextUpDown meilleur que scoreUpDown
				if (pair_scoreNextUpDown.first > pair_scoreUpDown.first || (pair_scoreNextUpDown.first == pair_scoreUpDown.first && pair_scoreNextUpDown.second < pair_scoreUpDown.second))
					scoreNextUpDown+=2;
				// scoreNextUpDown meilleur que scoreUpNextDown
				if (pair_scoreNextUpDown.first > pair_scoreUpNextDown.first || (pair_scoreNextUpDown.first == pair_scoreUpNextDown.first && pair_scoreNextUpDown.second < pair_scoreUpNextDown.second))
					scoreNextUpDown+=2;
				// scoreUpNextDown meilleur que scoreUpDown
				if (pair_scoreUpNextDown.first > pair_scoreUpDown.first || (pair_scoreUpNextDown.first == pair_scoreUpDown.first && pair_scoreUpNextDown.second < pair_scoreUpDown.second))
					scoreUpNextDown+=2;
				// scoreUpNextDown meilleur que scoreNextUpDown
				if (pair_scoreUpNextDown.first > pair_scoreNextUpDown.first || (pair_scoreUpNextDown.first == pair_scoreNextUpDown.first && pair_scoreUpNextDown.second < pair_scoreNextUpDown.second))
					scoreUpNextDown+=2;
				// Egalité entre scoreUpDown et scoreNextUpDown
				if (pair_scoreUpDown.first == pair_scoreNextUpDown.first && pair_scoreUpDown.second == pair_scoreNextUpDown.second){
					scoreUpDown++;
					scoreNextUpDown++;
				}
				// Egalité entre scoreUpDown et scoreUpNextDown
				if (pair_scoreUpDown.first == pair_scoreUpNextDown.first && pair_scoreUpDown.second == pair_scoreUpNextDown.second){
					scoreUpDown++;
					scoreUpNextDown++;
				}
				// Egalité entre scoreNextUpDown et scoreUpNextDown
				if (pair_scoreUpNextDown.first == pair_scoreNextUpDown.first && pair_scoreUpNextDown.second == pair_scoreNextUpDown.second){
					scoreUpNextDown++;
					scoreNextUpDown++;
				}

				// Check if the current up trace is a sequence
				if (spt_up->isSequence()){
					// We update the working up sequence (the previous one has been stored in up stack)
					sps_up = boost::dynamic_pointer_cast<Sequence>(spt_up);
					sps_up->reset();
				}
				// Check if the current down trace is a sequence
				if (spt_down->isSequence()){
					// We update the working down sequence (the previous one has been stored in down stack)
					sps_down = boost::dynamic_pointer_cast<Sequence>(spt_down);
					sps_down->reset();
				}
				// compute length of up and down traces
				len_up = spt_up->length();
				len_down = spt_down->length();

				TracesParser::osParser << "------------------------------------------" << std::endl;
				// Si le score obtenu avec les traces courantes est meilleur que les scores obtenu sans une des deux traces courante => On doit procéder à la fusion
				if (scoreUpDown > scoreNextUpDown && scoreUpDown > scoreUpNextDown){
					// If both current up and down traces are not sequences (this means they are Calls because Events has been cashed previously) => we merge the two Calls
					if (!spt_up->isSequence() && !spt_down->isSequence()) {
						// if some events have been found, add them to the merged sequence
						if (!events.empty()) {
							for (unsigned int i = 0; i < events.size(); i++)
								sps_merging->addTrace(events.at(i));
							// clear events list
							events.clear();
						}
						// Check if Calls are equals
						if (spt_up->operator==(spt_down.get())){
							TracesParser::osParser << "We choose to merge the two Calls" << std::endl;
							nbAlign++; // On compte un alignement
							// filter up call with the down Call
							dynamic_cast<Call*>(spt_up.get())->filterCall(dynamic_cast<const Call*>(spt_down.get()));
							// and add the up Call to the merged sequence
							sps_merging->addTrace(spt_up);
						} else{
							TracesParser::osParser << "ERROR (Sequence::mergeEquivalentSequences): Ask to merge two different Calls!!!" << std::endl;
							spt_up->exportAsString(TracesParser::osParser);
							TracesParser::osParser << "WITH" << std::endl;
							spt_down->exportAsString(TracesParser::osParser);
						}
						// ask to get next traces into up and down sequence
						next_up = true;
						next_down = true;
					} else {
						// On fusionner deux éléments qui ne sont pas des Call => empilement d'une séquence de fusion (voir fin de ce cas)
						// Si on doit aligner un Call du up avec une séquence du down OU une sequence up est plus courte qu'une séquence down
						if ((!spt_up->isSequence() && spt_down->isSequence()) || len_up < len_down) {
							TracesParser::osParser << "We choose to inter into down Sequence" << std::endl;
							// We build a new empty sequence based on down working sequence (the previous merged sequence has bee store in mergedStack)
							sps_merging = boost::make_shared<Sequence>(sps_down);
							// if down is a sequence and not up trace (means it's a Call)
							if (!spt_up->isSequence() && spt_down->isSequence())
								// this sequence has been done one time only once (indead a Call means it's possible to run this sequence only once)
								sps_merging->addIteration(1,1);
							// Add on down stack the down working sequence in order to enter possible sub-sequences
							downStack.push(sps_down);
							orderStack.push('d');// means we stack down sequence
							// ask to get the next trace into down sequence
							next_down = true;
						}
						// Si on doit aligner un Call du down avec une séquence du up OU une sequence down est plus courte qu'une séquence up
						else if ((spt_up->isSequence() && !spt_down->isSequence()) || len_up > len_down) {
							TracesParser::osParser << "We choose to inter into up Sequence" << std::endl;
							// We build a new empty sequence based on up working sequence (the previous merged sequence has been stored in mergedStack)
							sps_merging = boost::make_shared<Sequence>(sps_up);
							// if up trace is a sequence and not down trace (means it's a Call)
							if (spt_up->isSequence() && !spt_down->isSequence())
								// this sequence has been done one time only once (indead a Call means it's possible to run this sequence only once)
								sps_merging->addIteration(1,1);
							// Add on up stack the up working sequence in order to enter possible sub-sequences
							upStack.push(sps_up);
							orderStack.push('u');// means we stack up sequence
							// ask to get the next trace into up sequence
							next_up = true;
						}
						// Si on doit aligner deux séquences de même taille
						else {
							TracesParser::osParser << "We choose to merge both sequences" << std::endl;
							// So we build a new empty sequence based on up and down working sequences (the previous merged sequence has bee store in mergedStack)
							sps_merging = boost::make_shared<Sequence>(sps_up, sps_down);
							// Add on stacks up and down working sequences in order to enter possible sub-sequences
							upStack.push(sps_up);
							downStack.push(sps_down);
							orderStack.push('x');// means we stack the both sequences
							// ask to get next traces into up and down sequences
							next_up = true;
							next_down = true;
						}
						// Stack the new merging sequence
						mergedStack.push(sps_merging);
					}
				}
				else{
					// Ici on doit insérer une option
					// si on est en mode strict, on coupe la fusion
					if (strict){
						#ifdef DEBUG_PARSER
							TracesParser::osParser << "We abort merging due to strict mode and requirement to add optional trace" << std::endl;
						#endif
						Sequence::sp_sequence sps_empty;
						return sps_empty;
					}
					// Si le score obtenu sans la trace courante up est supérieur au score sans la trace courante down => mettre la trace up en option
					else if (scoreNextUpDown > scoreUpNextDown || (scoreNextUpDown == scoreUpNextDown && len_up >= len_down)){
						TracesParser::osParser << "We choose to set optional the up trace" << std::endl;
						// construction de la sequence optionnelle
						Sequence::sp_sequence sps_opt;
						// si la trace up est une sequence, on la passe en optionnelle sinon on en crée une nouvelle pour acceuillir la trace
						if (spt_up->isSequence()){
							// pas besoin de recaster spt_up dans sps_up, ça a déjà été fait plus haut
							sps_up->setOptional(true);
							sps_opt = sps_up;
							// On repositionne sps_up sur son parent
							if (sps_up->getParent())
								sps_up = boost::dynamic_pointer_cast<Sequence>(sps_up->getParent());
						} else{
							// construction de la sequence optionnelle
							sps_opt = boost::make_shared<Sequence>(1, false, true);
							// On ajoute dans la séquence optionnelle le contenu de la trace courante
							sps_opt->addTrace(spt_up);
						}
						nbOpt += sps_opt->length(); // On compte autant d'options que la longueur de la trace
						// and we add this optionnal sequence into the merged sequence
						sps_merging->addTrace(sps_opt);
						next_up = true;
					}
					// Si le score obtenu sans la trace courante down est supérieur au score sans la trace courante up => mettre la trace down en option
					else if (scoreUpNextDown > scoreNextUpDown || (scoreUpNextDown == scoreNextUpDown && len_down > len_up)){
						TracesParser::osParser << "We choose to set optional the down trace" << std::endl;
						// construction de la sequence optionnelle
						Sequence::sp_sequence sps_opt;
						// si la trace down est une sequence, on la passe en optionnelle sinon on en crée une nouvelle pour acceuillir la trace
						if (spt_down->isSequence()){
							// pas besoin de recaster spt_down dans sps_down, ça a déjà été fait plus haut
							sps_down->setOptional(true);
							sps_opt = sps_down;
							// On repositionne sps_down sur son parent
							if (sps_down->getParent())
								sps_down = boost::dynamic_pointer_cast<Sequence>(sps_down->getParent());
						} else{
							// construction de la sequence optionnelle
							sps_opt = boost::make_shared<Sequence>(1, false, true);
							// On ajoute dans la séquence optionnelle le contenu de la trace courante
							sps_opt->addTrace(spt_down);
						}
						nbOpt += sps_opt->length(); // On compte autant d'options que la longueur de la trace
						// and we add this optionnal sequence into the merged sequence
						sps_merging->addTrace(sps_opt);
						next_down = true;
					}
				}
				TracesParser::osParser << "------------------------------------------" << std::endl;
			}
		}
		bool endPop = false;
		while(!endPop){
			// Vérifier si on doit dépiler le up
			if (next_up && !upStack.empty() && sps_up->isEndReached()) {
				// Vérifier si on peut dépiler cette pile => On pourra le faire si le dernier empilement a eu lieu sur cette pile
				char lastPush = orderStack.top();
				if (lastPush == 'u' || lastPush == 'x'){ // 'u' pour "up" et 'x' pour "les deux en même temps"
					orderStack.pop(); // on dépile
					// si c'était un 'x' on rempile un 'd' car il faudra attendre la fin du down avant de pourvoir dépiler à nouveau un up
					if (lastPush == 'x') orderStack.push('d');
					// We pop the up stack
					upStack.pop();
					// If up stack is not empty we get the top sequence (i.e. the parent of the poped sequence)
					if (!upStack.empty()) {
						sps_up = upStack.top();
						// Count that a new pop occurs
						if (lastPush != 'x')
							pop++;
					}
					continue; // On coupe cette itération pour sauter le "endPop = true" de fin de boucle
				}
			}
			// Vérifier si l'on doit dépiler le down
			if (next_down && !downStack.empty() && sps_down->isEndReached()) {
				// Vérifier si on peut dépiler cette pile => On pourra le faire si le dernier empilement a eu lieu sur cette pile
				char lastPush = orderStack.top();
				if (lastPush == 'd' || lastPush == 'x'){ // 'd' pour "down" et 'x' pour "les deux en même temps"
					orderStack.pop(); // on dépile
					// si c'était un 'x' on rempile un 'u' car il faudra attendre la fin du up avant de pourvoir dépiler à nouveau un down
					if (lastPush == 'x') orderStack.push('u');
					// We pop the down stack
					downStack.pop();
					// If down stack is not empty we get the top sequence (i.e. the parent of the poped sequence)
					if (!downStack.empty()) {
						sps_down = downStack.top();
						// Count that a new pop occurs
						if (lastPush != 'x')
							pop++;
					}
					continue; // On coupe cette itération pour sauter le "endPop = true" de fin de boucle
				}
			}
			// Si on est ici c'est qu'on ne peut plus dépiler => on stoppe donc
			endPop = true;
		}
		// For each pop
		while (pop > 0) {
			// We consume one pop
			pop--;
			// We remove the top of the stack
			mergedStack.pop();
			// and we add the working merged sequence to the sequence at the top of the stack
			mergedStack.top()->addTrace(sps_merging);
			sps_merging->completeIterationDescription();
			// then we get the new working sequence
			sps_merging = mergedStack.top();
		}
		// si on doit récupérer la trace suivante dans la séquence up mais qu'on est bloqué sur la fin d'une séquence c'est qu'on n'a pas pu la dépiler. On reset donc le pointeur de trace.
		if (next_up && sps_up->isEndReached()){
			spt_up.reset();
		}
		else{
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
		}
		// si on doit récupérer la trace suivante dans la séquence down mais qu'on est bloqué sur la fin d'une séquence c'est qu'on n'a pas pu la dépiler. On reset donc le pointeur de trace.
		if (next_down && sps_down->isEndReached()){
			spt_down.reset();
		}
		else{
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

		/*TracesParser::osParser << "\nAffichage de UP STACK" << std::endl;
		std::stack<Sequence::sp_sequence> tmp;
		while (!upStack.empty()){
			tmp.push(upStack.top());
			upStack.pop();
		}
		int i = 0;
		while (!tmp.empty()){
			TracesParser::osParser << " " << i << " - ";
			tmp.top()->exportAsCompressedString(TracesParser::osParser);
			upStack.push(tmp.top());
			tmp.pop();
			i++;
		}
		TracesParser::osParser << "\nAffichage de DOWN STACK" << std::endl;
		std::stack<Sequence::sp_sequence> tmp2;
		while (!downStack.empty()){
			tmp2.push(downStack.top());
			downStack.pop();
		}
		i = 0;
		while (!tmp2.empty()){
			TracesParser::osParser << " " << i << " - ";
			tmp2.top()->exportAsCompressedString(TracesParser::osParser);
			downStack.push(tmp2.top());
			tmp2.pop();
			i++;
		}
		TracesParser::osParser << "\nAffichage de MERGE STACK" << std::endl;
		std::stack<Sequence::sp_sequence> tmp3;
		while (!mergedStack.empty()){
			tmp3.push(mergedStack.top());
			mergedStack.pop();
		}
		i = 0;
		while (!tmp3.empty()){
			TracesParser::osParser << " " << i << " - ";
			tmp3.top()->exportAsCompressedString(TracesParser::osParser);
			mergedStack.push(tmp3.top());
			tmp3.pop();
			i++;
		}*/
	}
	float ratio = nbOpt != 0 ? (float)nbAlign/nbOpt : nbAlign;
/*	#ifdef DEBUG_PARSER
		TracesParser::osParser << "Merging proposition:" << std::endl;
		sps_merging->exportAsString(TracesParser::osParser);
		TracesParser::osParser << "Alignment ratio: " << nbAlign << "/" << nbOpt << " = " << ratio << std::endl;
	#endif*/
	if (ratio >= 1){
		#ifdef DEBUG_PARSER
			TracesParser::osParser << "Merging accepted and returned (ratio: " << ratio << ")" << std::endl;
		#endif
		return sps_merging;
	}
	else{
		#ifdef DEBUG_PARSER
			TracesParser::osParser << "Merging not accepted (ratio: " << ratio << ")" << std::endl;
		#endif
		Sequence::sp_sequence sps_empty;
		return sps_empty;
	}
}

void Sequence::findAndProcessRotatingSequences(Sequence::sp_sequence& workingSequence, int startingPoint) {
	// On parcours toute la trace
	for (int i = startingPoint ; i < (int)workingSequence->size() ; i++){
		// Si la trace courante est une séquence
		if (workingSequence->at(i)->isSequence()){
			// On garde une ref de cette sequence
			Sequence::sp_sequence seq = boost::dynamic_pointer_cast<Sequence>(workingSequence->at(i));
			// On applique la rotation sur cette sous-sequence (appel récursif)
			Sequence::findAndProcessRotatingSequences(seq, 0);
			// On créé une séquence optionnelle pour stocker les traces intercallées
			Sequence::sp_sequence opt = boost::make_shared<Sequence>(1, false, true);
			// on va compter le nombre de traces suivant la séquence identiques à celles de la tête de la séquence
			int cpt = 0;
			while (cpt < (int)seq->size() && cpt + i + 1 < (int)workingSequence->size() && seq->at(cpt)->operator==(workingSequence->at(i+cpt+1).get()))
				// Tant qu'on n'a pas atteint la fin de la séquence, la fin des traces et que les deux traces sont égales => on continue à compter
				cpt++;
			if (cpt != 0){
				// On a trouvé en aval des symboles identiques au début de la séquence, on cherche donc en amont des symboles identique de la fin de la sequence
				int j = (int)seq->size()-1; // On part de la fin de la sequence et on la remonte jusqu'à atteindre la position du compteur
				int k = i-1;
				// position dans seq où insérer la partie optionnelle
				// On stoppe si on est remonté jusqu'à la position du compteur OU si on a dépassé le seuil d'arrêt dans les traces OU si on a remonté toute les traces
				while (j >= cpt && k > i-(int)seq->size()-1-MAX_OPT_WINDOW && k > startingPoint){
					// Si la trace de la sequence est égale à la trace en amont
					if (seq->at(j)->operator==(workingSequence->at(k).get()))
						j--; // on remonte dans la sequence
					else{
						opt->addTrace(workingSequence->at(k), 0); // on enregistre cette trace dans la sequence optionnelle (en tête vue qu'on remonte)
					}
					k--; // dans tous les cas on remonte en aval de la sequence dans les traces
				}
				// On vérifie si on a atteint le compteur de la séquence <=> on a trouvé en aval de la séquence la tête de la sequence et en amont de la séquence, la queue de la séquence
				if (j < cpt){
					// S'assurer qu'une partie optionnelle a bien été détectée
					if (opt->size() > 0){
						// Enregistrement des tailles de la tête et la queue
						int headSize = cpt;
						int tailSize = seq->size()-cpt;
						// Injection de la partie optionnelle dans la sequence
						// Ex : ABD => ABopt(C)D
						seq->addTrace(opt, headSize);
						// Inverser la tête et la queue de la séquence
						// on commence par recopier la tête à la fin
						// Ex : ABopt(C)D => ABopt(C)DAB
						for (int l = 0 ; l < headSize ; l++)
							seq->addTrace(seq->at(l));
						// puis on supprime la tête
						// Ex : ABopt(C)DAB => opt(C)DAB
						seq->getTraces().erase(seq->getTraces().begin(), seq->getTraces().begin()+headSize);
						// on recopie la partie correspondant à l'ancienne fin
						// Ex : opt(C)DAB => Dopt(C)DAB
						for (int l = 0 ; l < tailSize ; l++)
							seq->addTrace(seq->at(tailSize), 0); // Comme on ajoute un élément en tête on reutilise toujours l'élément position à l'indice tailSize
						// puis on supprime l'ancienne fin
						// Ex : Dopt(C)DAB => Dopt(C)AB
						seq->getTraces().erase(seq->getTraces().begin()+tailSize+1, seq->getTraces().begin()+(tailSize*2)+1);
						// On est donc passé de "ABopt(C)D" à "Dopt(C)AB"

						// Supprimer la partie en aval des traces
						workingSequence->getTraces().erase(workingSequence->getTraces().begin()+i+1, workingSequence->getTraces().begin()+i+cpt+1);
						// Supprimer la partie amont des traces
						workingSequence->getTraces().erase(workingSequence->getTraces().begin()+k+1, workingSequence->getTraces().begin()+i);
						// Ajout d'une itération à la sequence_vector
						seq->addOne();
						i = i-k; // mise à jour du compteur en raison de la suppression des traces en amont
					}
				}
			}
		}
	}
}

void Sequence::findAndProcessInclusiveSequences(Sequence::sp_sequence& workingSequence, int startingPoint){
	// On enregistre le dernier indice auquel l'appel récursif a été fait
	int lastRecursiveId = -1;
	// On parcours toute la trace jusqu'à l'avant dernier
	for (int i = startingPoint ; i < (int)workingSequence->size()-1 ; i++){
		// Si la trace courante est une séquence
		if (workingSequence->at(i)->isSequence()){
			// On garde une ref de cette sequence
			Sequence::sp_sequence seq = boost::dynamic_pointer_cast<Sequence>(workingSequence->at(i));
			// On applique le traitement sur cette sous-sequence (appel récursif) uniquement si l'appel récursif n'a pas déjà été réalisé à cet indice
			if (lastRecursiveId < i){
				Sequence::findAndProcessInclusiveSequences(seq, 0);
				lastRecursiveId = i;
			}
			// On créé une séquence optionnelle pour stocker les traces intercallées
			Sequence::sp_sequence opt = boost::make_shared<Sequence>(1, false, true);
			// On prépare le résultat
			Sequence::sp_sequence seq_res;
			// compteur permettant d'aller chercher en aval les séquences à analyser
			int j = 1;
			do{
				if (workingSequence->at(i+j)->isSequence()){
					// On garde une ref de cette sequence
					Sequence::sp_sequence seq_candidate = boost::dynamic_pointer_cast<Sequence>(workingSequence->at(i+j));
					// On applique le traitement sur cette sous-sequence (appel récursif) uniquement si l'appel récursif n'a pas déjà été réalisé à cet indice
					if (lastRecursiveId < i+j){
						Sequence::findAndProcessInclusiveSequences(seq_candidate, 0);
						lastRecursiveId = i+j;
					}
					// On tente de fusionner la sequence et son candidat
					seq_res = Sequence::mergeEquivalentSequences(seq, seq_candidate, false);
					// Si NULL est retourné cela signifie qu'aucune des deux séquences ne peut être incluse dans l'autre
					if (seq_res){
						if (opt->size() == 0){
							// S'il n'y a pas de traces intercallées entre les deux séquences fusionnées, on supprime ces deux séquences, l'ajout du résultat de la fusion se fait ci-dessous
							seq->getTraces().erase(seq->getTraces().begin()+i, seq->getTraces().begin()+i+j);
						} else {
							// Recherche de la sequence optionnelle en amont de seq
							bool before = workingSequence->findInTraces(opt, i-(j-1), i);
							// Recherche de la sequence optionnelle en aval de seq_candidate
							bool after = workingSequence->findInTraces(opt, i+j+1, i+2*j+1);
							// mise en cache des traces de "workingSequence"
							std::vector<Trace::sp_trace> workingTraces = workingSequence->getTraces();
							if (before){
								// si la séquence optionnelle a été détectée en amont de seq, on la rajoute à la tête du résultat
								seq_res->addTrace(opt, 0);
								workingTraces.erase(workingTraces.begin()+i-(j-1), workingTraces.begin()+i);
								i -= j-1;
								lastRecursiveId -= j-1;
							}
							if (after || !before){
								// on rajoute la sequence en queue si on l'a explicitement détectée après seq_candidate ou si elle n'a pas été détectée avant seq (dans ce dernier cas ça signifie que la séquence optionnelle est intercallée entre seq et seq_candidate et on choisi de l'insérer en fin)
								seq_res->addTrace(opt);
								if (after){
									// Si on a trouvé la partie optionnelle après seq_candidate, on supprime ces éléments des traces
									workingTraces.erase(workingTraces.begin()+i+j+1, workingTraces.begin()+i+2*j+1);
								}
							}
							// Dans tous les cas on supprime la partie optionnelle intercallée entre seq et seq_candidate
							workingTraces.erase(workingTraces.begin()+i+1, workingTraces.begin()+i+j);
						}
						workingSequence->addTrace(seq_res, i);
					} else {
						// Ajout de cette sequence qui est une séquence mais qui n'est pas compatible avec "seq"
						opt->addTrace(workingSequence->at(i+j));
					}
				} else {
					// Ajout de cette trace qui n'est pas une séquence
					opt->addTrace(workingSequence->at(i+j));
				}
				j++;
			// On stoppe lorsqu'on a atteint la taille max de la fenêtre de recherche ou si l'on a atteint la fin de la trace ou si on a obtenu un résultat.
		} while (j < MAX_OPT_WINDOW && i+j < (int)workingSequence->size() && !seq_res);
		}
	}
}

/*
Sequence::sp_sequence Sequence::mergeOptionalTracesFromSequences(Sequence::sp_sequence seqA, Sequence::sp_sequence seqB) {
	// Défintion de la séquence résultat
	std::cout << "mergeOptionalTracesFromSequences" << std::endl;
	seqA->exportAsString(std::cout);
	seqB->exportAsString(std::cout);
	Sequence::sp_sequence seqResult = boost::make_shared<Sequence>(seqA, seqB);
	// Détermination de la sequence la plus courte entre les deux paramètres
	Sequence::sp_sequence seqShort = seqA;
	Sequence::sp_sequence seqLong = seqB;
	if (seqShort->size() > seqLong->size()){
		seqShort = seqLong;
		seqLong = seqA;
	}
	unsigned int j = 0; // indice de parcours de la séquence courte
	unsigned int k = 0; // indice de parcours de la séquence longue
	// on parcours la plus longue séquence
	while (k < seqLong->size()){
		if (j < seqShort->size() && seqShort->at(j)->operator==(seqLong->at(k).get())){
			// L'élément courant de la séquence courte est égale à l'élément courant de la séquence longue => on l'ajoute au résultat
			seqResult->addTrace(seqShort->at(j));
			j++;
		} else {
			// Ici nous devons ajouter la trace de la séquence longue comme une option
			if (j >= seqResult->size()){
				// si l'indice de parcours de la séquence courte est égal à la taille du résultat il faut créer une séquence optionnelle pour stocker cette nouvelle trace
				Sequence::sp_sequence opt = boost::make_shared<Sequence>(1, false, true);
				opt->addTrace(seqLong->at(k));
				seqResult->addTrace(opt);
			} else {
				// si l'indice de parcours de la séquence courte est strictement inférieur à la taille du résultat il faut insérer cette nouvelle trace dans le résultat
				Sequence::sp_sequence opt = boost::dynamic_pointer_cast<Sequence>(seqResult->at(j));
				opt->addTrace(seqLong->at(k));
			}
		}
		k++; // on progresse toujours dans la longue séquence
	}
	if (j >= seqShort->size()){
		std::cout << "Résultat :" << std::endl;
		seqResult->exportAsString(std::cout);
		return seqResult;
	}
	else{
		std::cout << "Résultat : VIDE" << std::endl;
		// return an empty shared pointer
		Sequence::sp_sequence empty;
		return empty;
	}
}*/
