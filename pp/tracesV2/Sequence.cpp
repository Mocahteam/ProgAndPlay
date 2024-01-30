#include "Sequence.h"
#include "TraceConstantList.h"
#include "TracesParser.h"
#include <algorithm>
#include <exception>

// Cette donnée est utilisée lors des différentes recherches de parties optionnelles. Lors de ces étapes on recherche en périphérie de la séquence à traiter des traces alignables avec cette séquence. Si la partie de la séquence analysée a une taille X, on s'autorise à regarder en périphérie (amont ou aval) X+MAX_OPT_WINDOW traces. Et donc potentiellement un nombre de traces optionnelles pour les parties périphériques égale à MAX_OPT_WINDOW.
#define MAX_OPT_WINDOW 10
// Toujours lors de ces étapes d'identification des partie optionnelles on acceptera une fusion uniquement si le score d'alignement est supérieur à MIN_SCORE
#define MIN_SCORE 0.6

bool debug = false;

Sequence::Sequence(std::string info, bool num_fixed) : Trace(SEQUENCE, info), num_fixed(num_fixed), num(1), pt(0), endReached(false), root(false) {
	addIteration(1);
}

Sequence::Sequence(unsigned int num, bool root, bool opt) : Trace(SEQUENCE), num_fixed(false), num(num), pt(0), endReached(false), root(root)
{
	addIteration(num);
	setOptional(opt);
}

Sequence::Sequence(const_sp_sequence sps) : Trace(sps.get()), pt(0), endReached(false)
{
	num = sps->getNum();
	num_fixed = sps->hasNumberIterationFixed();
	root = sps->root;
	mergeIterationDescription(sps->getIterationDescription());
}

Sequence::Sequence(const_sp_sequence sps_up, const_sp_sequence sps_down) : Trace(SEQUENCE), pt(0), endReached(false)
{
	num = std::max(sps_up->getNum(), sps_down->getNum());
	num_fixed = sps_up->hasNumberIterationFixed() && sps_down->hasNumberIterationFixed();
	root = false;
	opt = sps_up->isOptional() || sps_down->isOptional();
	mergeIterationDescription(sps_up->getIterationDescription());
	mergeIterationDescription(sps_down->getIterationDescription());
}

bool Sequence::operator==(Trace *t)
{
	bool res = false;
	if (t->isSequence())
	{
		Sequence *s = dynamic_cast<Sequence *>(t);
		if (traces.size() == s->size())
		{
			for (unsigned int i = 0; i < traces.size(); i++)
			{
				if (!at(i)->operator==(s->at(i).get()))
					return false;
			}
			res = true;
		}
	}
	return res;
}

bool Sequence::compare(Trace *t)
{
	bool res = false;
	if (t->isSequence())
	{
		Sequence *s = dynamic_cast<Sequence *>(t);
		Call::call_vector calls = getCalls();
		Call::call_vector s_calls = s->getCalls();
		if (calls.size() == s_calls.size())
		{
			res = true;
			for (unsigned int i = 0; res && i < calls.size(); i++)
			{
				if (!calls.at(i)->operator==(s_calls.at(i).get()))
					res = false;
			}
		}
	}
	return res;
}

Trace::sp_trace Sequence::clone() const
{
	sp_sequence sps_clone = std::make_shared<Sequence>(shared_from_this());
	for (unsigned int i = 0; i < traces.size(); i++)
		sps_clone->addTrace(traces.at(i)->clone());
	return sps_clone;
}

void Sequence::exportAsString(std::ostream &os) const
{
	numTab++;
	for (int i = 0; i < numTab; i++)
		os << "\t";
	if (opt)
		os << "optional ";
	if (!root)
		os << "Sequence < " << getIterartionDescriptionString(getPercentageIterationDescription()) << " >" << std::endl;
	else
		os << "Root" << std::endl;
	for (unsigned int i = 0; i < traces.size(); i++){
		//os << "number :"<<i <<" info="<< traces.at(i)->getInfo()<<" //";
		traces.at(i)->exportAsString(os);
	}
	for (int i = 0; i < numTab; i++)
		os << "\t";
	if (!root)
		os << "EndSequence" << std::endl;
	else
		os << "EndRoot" << std::endl;
	numTab--;
}


std::string Sequence::getString() const
{
	std::string ret = "";
	numTab++;
	for (int i = 0; i < numTab; i++)
		ret += "\t";
	if (opt)
		ret += "optional ";
	if (!root)
		ret += ("Sequence < " + getIterartionDescriptionString(getPercentageIterationDescription()) + " >\n");
	else
		ret += "Root\n";
	for (unsigned int i = 0; i < traces.size(); i++)
		ret += traces.at(i)->getString();
	for (int i = 0; i < numTab; i++)
		ret += "\t";
	if (!root)
		ret += "EndSequence\n";
	else
		ret += "EndRoot\n";
	numTab--;
	return ret;
}

void Sequence::exportAsCompressedString(std::ostream &os) const
{
	if (numTab == 0)
		os << " ";
	numTab++;
	if (opt)
		os << "*";
	if (!root)
		//os << "Sequence()"
		os << "Sequence(" << getIterartionDescriptionString(getPercentageIterationDescription()) << ")"
		   << " ";
	else
		os << "Root"
		   << " ";
	for (unsigned int i = 0; i < traces.size(); i++)
		traces.at(i)->exportAsCompressedString(os);
	if (!root)
		os << "EndSequence"
		   << " ";
	else
		os << "EndRoot" << std::endl;
	numTab--;
	if (numTab == 0)
		os << std::endl;
}

void Sequence::exportLinearSequenceAsString(std::vector<Trace::sp_trace> & linearSequence, std::ostream &os, bool exportSeqIter)
{
	for (int i = 0 ; i < (int)linearSequence.size() ; i++)
	{
		if (linearSequence[i]->isSequence()){
			if (linearSequence[i]->isOptional())
				os << "*";
			os << "Seq(" << linearSequence[i]->getInfo();
			if (exportSeqIter)
				os << "*" << std::dynamic_pointer_cast<Sequence>(linearSequence[i])->getNum();
			os << ") ";
		}
		else
			linearSequence[i]->exportAsCompressedString(os);
	}
	os << std::endl;
}

unsigned int Sequence::length(int start, bool processOptions) const
{
	start = start < 0 ? 0 : start;
	unsigned int len = 0;
	for (unsigned int i = start; i < size(); i++)
	{
		if (processOptions || !at(i)->isOptional())
			// Ici on traite les séquences optionnelles comme les autres donc on peut passer sur la version de Sequence::length héritée
			len += at(i)->length(0, processOptions);
		// dans le cas du else on ne compte pas la longueur vue que la trace est optionnelle et que processOptions est à faux
	}
	return len;
}

std::vector<Trace::sp_trace> &Sequence::getTraces()
{
	return traces;
}

Sequence::sp_sequence Sequence::getSubSequence(int start, int end)
{
	start = start < 0 ? 0 : start;
	end = end < 0 || end > (int)this->size() ? this->size() : end;
	Sequence::sp_sequence subSeq = std::make_shared<Sequence>(1, false, false);
	for (int i = start; i < end; i++)
	{
		subSeq->addTrace(traces.at(i)->clone());
	}
	return subSeq;
}

unsigned int Sequence::getNum() const
{
	return num;
}

bool Sequence::hasNumberIterationFixed() const
{
	return num_fixed;
}

void Sequence::addOne()
{
	// We remove one appearance of the current number of iteration
	addIteration(num, -1);
	// And we increase the number of iteration and we add one more appearance
	addIteration(++num);
}

void Sequence::removeOne()
{
	// We remove one appearance of the current number of iteration
	addIteration(num, -1);
	// And we decrease the number of iteration and we add one more appearance
	addIteration(--num);
}

unsigned int Sequence::size() const
{
	return traces.size();
}

const Trace::sp_trace &Sequence::at(unsigned int i) const
{
	if (i >= traces.size())
		throw std::runtime_error("cannot access to the trace at position 'i' in 'traces' vector");
	return traces.at(i);
}

bool Sequence::addTrace(Trace::sp_trace spt, int ind)
{
	if (ind >= -1 && ind <= (int)traces.size())
	{
		if (ind == -1)
			traces.push_back(spt);
		else
			traces.insert(traces.begin() + ind, spt);
		spt->setParent(shared_from_this());
		return true;
	}
	return false;
}

const Trace::sp_trace &Sequence::next()
{
	if (pt >= traces.size())
		throw std::runtime_error("cannot access to the trace at position 'pt' in 'traces' vector");
	const Trace::sp_trace &spt = traces.at(pt++);
	if (pt == traces.size())
	{
		pt = 0;
		endReached = true;
	}
	else if (pt == 1 && endReached)
		endReached = false;
	return spt;
}

const Trace::sp_trace &Sequence::nextRec()
{
	if (pt >= traces.size())
		throw std::runtime_error("cannot access to the trace at position 'pt' in 'traces' vector");
		
	const Trace::sp_trace &spt = traces.at(pt);
	if (spt->isSequence())
	{
		Sequence::sp_sequence seq = std::dynamic_pointer_cast<Sequence>(spt);
		if (seq->isEndReached())
		{
			pt++;
			return nextRec();
		}
		else
			return seq->nextRec();
	}

	if (pt == traces.size())
	{
		pt = 0;
		endReached = true;
	}
	else if (pt == 1 && endReached)
		endReached = false;
	return spt;
}

unsigned int Sequence::getPt() const
{
	return pt;
}

void Sequence::setPtAndEndState(unsigned int pos, bool endState)
{
	pt = pos;
	endReached = endState;
	if (pos >= traces.size())
		pt = 0;
}

void Sequence::reset()
{
	pt = 0;
	endReached = false;
	Sequence *s;
	for (unsigned int i = 0; i < traces.size(); i++)
	{
		if (traces.at(i)->isSequence())
		{
			s = dynamic_cast<Sequence *>(traces.at(i).get());
			s->reset();
		}
	}
}

void Sequence::clear()
{
	traces.clear();
	reset();
}

bool Sequence::isEndReached() const
{
	return endReached;
}

const std::map<unsigned int, unsigned int> &Sequence::getIterationDescription() const
{
	return iterDesc;
}

std::map<unsigned int, double> Sequence::getPercentageIterationDescription() const
{
	std::map<unsigned int, double> pIterDesc;
	std::map<unsigned int, unsigned int>::const_iterator it = iterDesc.begin();
	double sum = 0;
	while (it != iterDesc.end())
		sum += (it++)->second;
	it = iterDesc.begin();
	while (it != iterDesc.end())
	{
		pIterDesc.insert(std::pair<unsigned int, double>(it->first, it->second / sum));
		it++;
	}
	return pIterDesc;
}

void Sequence::addIteration(unsigned int nbIter, int appearance)
{
	// If no key value exists, we create it and set value to "appearance"
	if (iterDesc.find(nbIter) == iterDesc.end() && appearance > 0)
		iterDesc.insert(std::pair<unsigned int, unsigned int>(nbIter, appearance));
	// Else we add appearance to the current value
	else
	{
		iterDesc.at(nbIter) += appearance;
		if (iterDesc.at(nbIter) <= 0)
			iterDesc.erase(nbIter);
	}
}

void Sequence::mergeIterationDescription(const std::map<unsigned int, unsigned int> &newIterationsDesc)
{
	std::map<unsigned int, unsigned int>::const_iterator it = newIterationsDesc.begin();
	while (it != newIterationsDesc.end())
	{
		addIteration(it->first, it->second);
		it++;
	}

	// If we find an entry as <1:X> with X > 0 then we transform it in <X:1>. Indeed we prefer to know that X iterations occur one time than 1 iteration occurs X times because in th e last case when we transform it in percentage we will obtain <1:Y%> and we loose the number of iterations.
	if (iterDesc.find(1) != iterDesc.end() && iterDesc[1] > 1)
	{
		addIteration(iterDesc[1]);
		iterDesc.erase(1);
	}
}

double Sequence::getIterationDescriptionMeanDistance(const Sequence::sp_sequence &sps) const
{
	std::map<unsigned int, double> fmap = getPercentageIterationDescription();
	std::map<unsigned int, double> smap = sps->getPercentageIterationDescription();
	double fm = 0, sm = 0;
	std::map<unsigned int, double>::const_iterator it = fmap.begin();
	while (it != fmap.end())
	{
		fm += it->first * it->second;
		it++;
	}
	it = smap.begin();
	while (it != smap.end())
	{
		sm += it->first * it->second;
		it++;
	}
	return std::abs(fm - sm) / (fm + sm);
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
 * Est linéarisée de la manière suivante où Sd est une séquence vide dont le champ info marque un début de séquence et Sf est une séquence vide dont le champ info marque une fin de séquence :
 * C C Sd C C Sd C Sf Sf C
 *
 * \param start indice de départ de linéarisation de la séquence (valeur par défaut : 0)
 * \param end indice de fin de linéarisation de la séquence (valeur par défaut : -1 => fin de la trace)
 *
 * \return un vecteur de trace représentant une version linéarisée des traces
 */
std::vector<Trace::sp_trace> &Sequence::getLinearSequence(int start, int end)
{
	linearizedTraces.clear();
	Sequence::sp_sequence sps = shared_from_this();
	start = start < 0 ? 0 : start;
	end = end < 0 || end > (int)sps->size() ? sps->size() : end;
	if (sps->length() > 0)
	{
		Trace::sp_trace spt;
		unsigned int back_pt = sps->getPt();
		bool back_endReached = sps->isEndReached();
		sps->reset();
		for (int i = 0; i < start; i++)
			sps->next();
		std::stack<Sequence::sp_sequence> stack;
		stack.push(sps);
		Sequence::sp_sequence tmpSeq = std::make_shared<Sequence>("Begin", -1);
		// remove default iteration description
		tmpSeq->iterDesc.clear();
		// and merge with the model's one
		tmpSeq->mergeIterationDescription(sps->getIterationDescription());
		tmpSeq->num = tmpSeq->getIterationDescription().rbegin()->first;
		linearizedTraces.push_back(tmpSeq);
		while (!stack.empty())
		{
			while (!stack.empty() && (sps->isEndReached() || sps->length() == 0))
			{
				stack.pop();
				linearizedTraces.push_back(std::make_shared<Sequence>("End", -1));
				if (!stack.empty())
					sps = stack.top();
			}
			if (!sps->isEndReached() && sps->length() > 0)
			{
				spt = sps->next();
				// Si on est au premier niveau de la pile (<=> root) on vérifie si on n'a pas atteint l'indice d'arrêt. Si oui on simule l'atteinte de la fin de la trace
				if ((int)stack.size() == 1 && (int)this->pt >= end)
					sps->setPtAndEndState(sps->size(), true);
				if (spt->isSequence())
				{
					sps = std::dynamic_pointer_cast<Sequence>(spt);
					sps->reset();
					stack.push(sps);
					tmpSeq = std::make_shared<Sequence>("Begin", -1);
					tmpSeq->mergeIterationDescription(sps->getIterationDescription());
					tmpSeq->num = tmpSeq->getIterationDescription().rbegin()->first;
					linearizedTraces.push_back(tmpSeq);
				}
				else
				{
					linearizedTraces.push_back(spt);
				}
			}
		}
		sps->setPtAndEndState(back_pt, back_endReached);
	}
	return linearizedTraces;
}

void Sequence::insertLinearSequence (std::vector<Trace::sp_trace> & linearSequence, int pos){
	int i = 0;
	// on parcours toute la séquence linéarisée
	while (i < (signed)linearSequence.size()){
		// on ajoute la trace courante
		this->addTrace(linearSequence[i]->clone(), pos);
		// Si la trace que l'on vient d'ajouter est un début de séquence, il faut la remplir avec son contenu
		if (this->traces.at(pos)->isSequence() && this->traces.at(pos)->getInfo().compare("Begin") == 0){
			// On supprime la propriété "info" de la version linéarisée
			this->traces.at(pos)->setInfo("");
			// On récupère les bornes de la sous-séquence
			int start = i;
			int end = Sequence::getEndPosOfLinearSequence(linearSequence, i);
			// On récupère le contenue de la sous-séquence linéarisée
			std::vector<Trace::sp_trace> subLinearSequence(linearSequence.begin()+start+1, linearSequence.begin()+end);
			// appel récursif sur la sous-séquence pour y intégrer la sous-partie linéarisée
			std::dynamic_pointer_cast<Sequence>(this->traces.at(pos))->insertLinearSequence(subLinearSequence, 0);
			// On saute à la fin de la sous-séquence linéarisée puisqu'elle vient d'être traité dans l'appel récursif
			i = end;
		}
		i++;
		pos++;
	}
}

std::vector<Trace::sp_trace> Sequence::cloneLinearSequence(std::vector<Trace::sp_trace> & linearSequence)
{
	std::vector<Trace::sp_trace> clone;
	for (int i = 0 ; i < (signed)linearSequence.size() ; i++)
		clone.push_back(linearSequence.at(i)->clone());
	return clone;
}

int Sequence::getBeginPosOfLinearSequence(std::vector<Trace::sp_trace> & linearSequence, int from)
{
	int seqCounter = 0;

	if (from < 0)
		return -1;

	int i = from >= (signed)linearSequence.size() ? linearSequence.size()-1 : from;
	
	// Au cas où le point de départ est une fin de séquence, on va toujours commencer à chercher à partir de la trace précédente (évite d'incrémenter seqCounter dans la boucle ci-dessous pour le point de départ s'il s'agit d'une fin de séquence) sauf si le point de départ est un début de séquence au quel cas il faut retourner immediatement
	if (linearSequence[i]->isSequence() && linearSequence[i]->getInfo().compare("Begin") == 0)
		return i;
	else
		i--;
	// On remonte la trace jusqu'à trouver le début de la séquence
	while (i >= 0)
	{
		if (linearSequence[i]->isSequence() && linearSequence[i]->getInfo().compare("Begin") == 0)
			if (seqCounter <= 0)
				return i;
			else
				seqCounter--;
		else if (linearSequence[i]->isSequence() && linearSequence[i]->getInfo().compare("End") == 0)
			seqCounter++;
		i--;
	}
	return -1;
}

int Sequence::getEndPosOfLinearSequence(std::vector<Trace::sp_trace> & linearSequence, int from)
{
	int seqCounter = 0;
	int i = from < 0 ? 0 : from;
	
	// Au cas où le point de départ est un début de séquence, on va toujours commencer à chercher à partir de la trace suivante (évite d'incrémenter seqCounter dans la boucle ci-dessous pour le point de départ s'il s'agit d'un début de séquence) sauf si le point de départ est une fin de séquence au quel cas il faut retourner immediatement
	if (linearSequence[i]->isSequence() && linearSequence[i]->getInfo().compare("End") == 0)
		return i;
	else
		i++;
	// On remonte la trace jusqu'à trouver le début de la séquence
	while (i < (signed)linearSequence.size())
	{
		if (linearSequence[i]->isSequence() && linearSequence[i]->getInfo().compare("End") == 0)
			if (seqCounter <= 0)
				return i;
			else
				seqCounter--;
		else if (linearSequence[i]->isSequence() && linearSequence[i]->getInfo().compare("Begin") == 0)
			seqCounter++;
		i++;
	}
	return -1;
}

std::vector<std::vector<int>> Sequence::computeLinearSequenceDistance(std::vector<Trace::sp_trace> & s1, std::vector<Trace::sp_trace> & s2){
	// D est un tableau de s1+1 lignes et s2+1 colonnes
	std::vector<std::vector<int>> D (s1.size()+1, std::vector<int>(s2.size()+1));

	// Important pour la compréhension de la suite : dans notre adaptation de la distance de Levenshtein on considère que le coût vertical et horizontal est nul pour une option et pour une fin de séquence.

	D[0][0] = 0;
	// initialisation de la première colonne
	for (unsigned int l = 1 ; l < s1.size()+1 ; l++)
		D[l][0] = s1[l-1]->isOptional(true) || s1[l-1]->getInfo() == "End" ? D[l-1][0] : D[l-1][0]+1; // on ajoute 1 si la trace n'est pas optionnelle (ou fille d'une trace optionnelle) et que ce n'est pas une fin de séquence
	// initialisation de la première ligne
	for (unsigned int c = 1 ; c < s2.size()+1 ; c++)
		D[0][c] = s2[c-1]->isOptional(true) || s2[c-1]->getInfo() == "End" ? D[0][c-1] : D[0][c-1]+1; // on ajoute 1 si la trace n'est pas optionnelle (ou fille d'une trace optionnelle) et que ce n'est pas une fin de séquence

	// calcul de la distance
	int substitutionCost;
	for (unsigned int l = 1 ; l < s1.size()+1 ; l++)
		for (unsigned int c = 1 ; c < s2.size()+1 ; c++){
			substitutionCost = s1[l-1]->operator==(s2[c-1].get()) ? 0 : 1;
			D[l][c] = std::min(std::min(
				D[l-1][c]+(s1[l-1]->isOptional(true) || s1[l-1]->getInfo() == "End" ? 0 : 1), // Attention l dans la matrice D <=> l-1 dans s1. Donc si s1[l-1] est optionnel ou est une fin de séquence, le coût pour passer de D[l-1][X] à D[l][X] est 0 et 1 sinon
				D[l][c-1]+(s2[c-1]->isOptional(true) || s2[c-1]->getInfo() == "End" ? 0 : 1)), // Attention c dans la matrice D <=> c-1 dans s2. Donc si s2[c-1] est optionnel ou est une fin de séquence, le coût pour passer de D[X][c-1] à D[X][c] est 0 et 1 sinon
				D[l-1][c-1] + substitutionCost);
		}

	return D;
}

void Sequence::unstackSequence(std::vector<std::tuple<std::string, Trace::sp_trace, bool>> & stack, std::vector<Trace::sp_trace> & mergedSequence, std::string action, Trace::sp_trace trace){
	// l'action à dépiler est la même que la tête de la pile, c'est parfait on n'a qu'à la dépiler
	if (std::get<0>(stack.back()) == action)
		stack.pop_back();
	// la tête de la pile est une diagonale et notre action n'est pas une diagonale, on transforme la tête de la pile en l'opposé de l'action à dépiler et on monte toutes les traces intercallées d'un niveau
	// exemple : B[C] vs [BC] => avec "B[C]" sur les lignes de la matrice et "[BC]" sur les colonnes. Sur la remontée on sera sur "C]". Le "]" a été ajouté avec un "d" et on cherche à ajouter le "[" du "B[C]". Donc on transforme le "C]" en "C]]" et on change la tête de la pile à "c" pour noter que l'imbrication des lignes a été traité mais qu'il reste un "c" à gérer.
	else if (std::get<0>(stack.back()) == "d"){
		// rechercher dans le vecteur résultat la trace correspondant à ce dernier empilement en diagonale et ajout d'une nouvelle séquence
		for (int i = mergedSequence.size()-1; i >= 0; i--)
			if (mergedSequence[i] == std::get<1>(stack.back())){
				mergedSequence.insert(mergedSequence.begin()+i+1, mergedSequence[i]->clone());
				break;
			}
		// remplacement dans la pile du "d" par l'opposé de l'action
		std::get<0>(stack.back()) = (action == "c" ? "l" : "c");
	}
	// l'action est une diagonale et la tête de la pile est soit "l" soit "c". On va décomposer le "d" pour traiter la tête de la pile. On ajoute donc une fermeture pour cette première composante du "d", on dépile l'action de la pile et on fait un appel récursif pour traiter le complément du "d" non traité.
	// exemple : [B]C vs [BC] => avec "[B]C" sur les lignes de la matrice et "[BC]" sur les colonnes. Sur la remontée on sera sur "B]C]". Les deux "]" ont été ajoutés une première fois en colonne puis en ligne et on cherche à ajouter "[" en diagonale donc à la fois sur "l" et "c". Donc on ajoute un "[" supplémentaire et on dépile le premier des deux "l" et "c" et on fait un appel récursif pour gérer le complément non traité du "d".
	else if (action == "d"){
		std::string head = std::get<0>(stack.back());
		// on dépile la stack, soit un "l" soit un "c" (on est sûr que ce n'est par un "d" car sinon on serait rentré dans le tout premier cas de cette fonction)
		stack.pop_back();
		// maintenant la tête de la pile peux être un autre "l" ou "c" mais aussi pourquoi pas un "d". On fait donc un appel récursif pour gérer le dépilement de la seconde composante de notre action "d". On passe donc comme action le complément au "d" de l'action en cours de traitement, donc "l" si "c" et "c" si "l"
		unstackSequence(stack, mergedSequence, (head == "c" ? "l" : "c"), trace);
		// on ajoute un clone additionnel pour intégrer cette double fermeture. Le second est ajouté comme pour les autres cas à l'extérieur de l'appel de cette fonction
		mergedSequence.push_back(trace->clone());
	}
	// l'action est soit "l" soit "c" et la tête de la pile est l'opposée (cas de séquences qui se chevochent)
	// exemple : A[BC] vs [AB]C => avec "A[BC]" sur les lignes de la matrice et "[AB]C" sur les colonnes. Sur la remontée on sera sur "B]C]". Les deux "]" ont été ajoutés une première fois en ligne puis en colonne (tête de la pile "c") et on cherche à ajouter "[" en ligne ce qui est pour l'instant pas possible puisque l'action à dépiler n'est pas cohérente avec la tête de la pile. On va donc passer les traces non incluses dans le chevauchement en optionnelle pour obtenir [*A[B]*C] qui est bien un moyen de fusionner les deux traces en exemple.
	// Soit "x" l'action à dépiler (la tête de la pile est le complément de "x") :
	// 1- Chercher dans la pile le premier "x" (ou "d") disponible correspondant à l'action, noté "t" pour target.
	// 2- Mettre toutes les traces comprises entre "t" et la tête de la pile comme optionnelle (si on tombe sur une séquence on la marque comme optionnelle et on saute directement à son End pour éviter de traiter tout ses enfants).
	// 3- Noter la tête de la pile en chevauchement de manière à ce que toute nouvelle trace soit notée comme optionnelle tant que cette tête se trouve en haut de pile.
	// 4- Retirer "t" de la pile, s'il s'agit d'un "d" le remplacer par le complément de "x".
	else if ((action == "l" && std::get<0>(stack.back()) == "c") || (action == "c" && std::get<0>(stack.back()) == "l")){
		// 1- Chercher dans la pile le premier "x" (ou "d") disponible correspondant à l'action, noté "t" pour target.
		int stackTpos = -1;
		for (int i = stack.size()-1; i >= 0; i--)
			if (std::get<0>(stack[i]) == action || std::get<0>(stack[i]) == "d"){
				stackTpos = i;
				break;
			}
		if (stackTpos == -1)
			throw ("No action in the stack corresponding to the one expected");
		
		// 2- Mettre toutes les traces comprises entre "t" et la tête de la pile comme optionnelle (si on tombe sur une séquence on la marque comme optionnelle mais on ne traite pas ses enfants).
		// Rechercher dans la séquence fusionnée la position de la trace correspondant à la tête de la pile
		int headSeqPos = -1;
		for (int i = mergedSequence.size()-1; i >= 0; i--)
			if (mergedSequence[i] == std::get<1>(stack.back())){
				headSeqPos = i;
				break;
			}
		if (headSeqPos == -1)
			throw ("No trace in the merged sequence corresponding to the one expected");
		// Rechercher dans la séquence fusionnée la position de la trace correspondant à "t"
		int targetSeqPos = -1;
		for (int i = headSeqPos; i >= 0; i--)
			if (mergedSequence[i] == std::get<1>(stack[stackTpos])){
				targetSeqPos = i;
				break;
			}
		if (targetSeqPos == -1)
			throw ("No trace in the merged sequence corresponding to the one expected");
		// passer en optionnel toutes les traces comprises entre la target et la tête de la pile. On ne traite que les trace au même niveau que la target.
		int level = 0;
		for (int i = targetSeqPos+1 ; i < headSeqPos ; i++){
			if (level == 0)
				mergedSequence[i]->setOptional(true);
			if (mergedSequence[i]->isSequence() && mergedSequence[i]->getInfo() == "End")
				level++;
			if (mergedSequence[i]->isSequence() && mergedSequence[i]->getInfo() == "Begin")
				level--;
		}

		// 3- Noter la tête de la pile en chevauchement de manière à ce que toute nouvelle trace soit notée comme optionnelle tant que cette tête se trouve en haut de pile.
		std::get<2>(stack.back()) = true;

		// 4- Retirer "t" de la pile, s'il s'agit d'un "d" le remplacer par le complément de "x".
		if (std::get<0>(stack[stackTpos]) == "d")
			std::get<0>(stack[stackTpos]) = (action == "c" ? "l" : "c");
		else
			stack.erase(stack.begin()+stackTpos);
	}
}

void Sequence::manageStack(std::vector<std::tuple<std::string, Trace::sp_trace, bool>> & stack, std::vector<Trace::sp_trace> & mergedSequence, std::string action, Trace::sp_trace selection, int step){
	if (selection->getInfo() == "End"){
		stack.push_back(std::make_tuple(action, selection, false)); // on enregistre qu'on rentre dans une séquence suite à une action "l", "c" ou "d" (pour rappel on remonte la trace donc on entre dans des séquences par des End et on en ressort par des Begin)
		if (step == 0){
			// Sur cette étape (step == 0) on va se servir de l'attribut "optionnel" d'une séquence End pour coder le fait que cette séquence doit être mise en option lors de l'étape suivante (step == 1) si elle ne contient aucune trace alignée. Cette astuce sera aussi utilisée pour déterminer si un Call doit être mis en option sur un changement de ligne ou de colonne (pas la diagonale) en effet si le End de la séquence mère est tagué Optionnel alors il n'est pas nécessaire de noter les Call enfants comme optionnels.
			// Donc à l'étape 0 on tague par défaut toutes les Séquences End comme optionnelle. Si ensuite en construisant la fusion on trouve des traces alignées, on annulera cette mise en option.
			selection->setOptional(true);
		}
	}
	// ici on est sur un Begin, il faut vérifier si on peut dépiler simplement ou s'il faut faire des opérations spécifiques
	else{
		unstackSequence(stack, mergedSequence, action, selection);
	}
}

std::vector<Trace::sp_trace> Sequence::mergeLinearSequences(std::vector<Trace::sp_trace> & s1, std::vector<Trace::sp_trace> & s2){
	std::vector<Trace::sp_trace> mergedSequence; // la séquence contenant le résulta de la fusion
	std::vector<std::vector<int>> transformationMatrix = computeLinearSequenceDistance(s1, s2);

	mergeLinearSequences_local(s1, s2, transformationMatrix, 0, mergedSequence);
	mergeLinearSequences_local(s1, s2, transformationMatrix, 1, mergedSequence);
	
	// On met le vecteur de fusion dans le bon sens
	std::reverse(mergedSequence.begin(), mergedSequence.end());
	return mergedSequence;
}

void Sequence::mergeLinearSequences_local(std::vector<Trace::sp_trace> & s1, std::vector<Trace::sp_trace> & s2, std::vector<std::vector<int>> & transformationMatrix, int step, std::vector<Trace::sp_trace> & mergedSequence){
	//la pile permettant d'enregistrer la provenance des séquences empilées. Chaque élément de la pile est composé de 3 valeurs{orientation, Trace, Chevauchement}. Orientation indique si l'empilement de la séquence provient de la ligne noté "l" (source s1), de la colonne noté "c" (source s2), ou de la diagonale noté "d" (sources s1 et s2 alignées) lors de la remonté de la matrice de transformation fournie par computeLinearSequenceDistance. Trace fournit la référence dans mergedSequence de la Sequence correspondant à cet empilement. Chevauchement est un booleen indiquant si cet empilement est en chevauchement avec une autre séquence (gestion du cas [AB]C vs A[BC]).
	std::vector<std::tuple<std::string, Trace::sp_trace, bool>> stack;
	int mergedPos = 0;
	// partir du coin inférieur droit de la matrice et remonter soit à gauche, soit en haut, soit en diagonale (sémantique des orientations : gauche c-1 <- c => ajouter colonne c (si Call, taguer optionnel) ; haut l-1 <- l => ajouter ligne l (si Call, taguer optionnel) ; diagonale [l-1][c-1] <- [l][c] => ajouter la fusion de la ligne l et la colonne c)
	int l = transformationMatrix.size()-1;
	int c = transformationMatrix[0].size()-1;
	// On s'arrête si Si l == 0 et c == 0
	while (l > 0 || c > 0){
		// si on est sur la première ligne ou la première colonne prendre la trace de la ligne (respectivement colonne)
		if (l == 0 || c == 0){
			if (step == 0){
				Trace::sp_trace merged = (l == 0 ? s2[c-1]->clone() : s1[l-1]->clone()); // transformationMatrix contient une ligne et une colonne de plus que s1 et s2, d'où le -1
				mergedSequence.push_back(merged);
			} else {
				// On définit cette trace comme optionnelle si c'est un Call et que la pile est vide (Cette trace n'est pas incluse dans une séquence) OU si c'est un Call inclus dans une séquence non optionnelle OU s'il s'agit d'un début de séquence dont sa fin est restée taguée optionnelle
				mergedSequence[mergedPos]->setOptional((mergedSequence[mergedPos]->isCall() && (stack.size() == 0 || !std::get<1>(stack.back())->isOptional())) || (mergedSequence[mergedPos]->getInfo() == "Begin" && std::get<1>(stack.back())->isOptional()));
				mergedPos++;
			}
			if (l==0)
				c--;
			else
				l--;
		}
		// les deux traces sont des Call
		else if (s1[l-1]->isCall() && s2[c-1]->isCall()){
			// Si la diagonale est le coût minimal et colonne c == ligne l privilégier la diagonale sinon privilégier le min entre haut et gauche, si égalité réduire en priorité la trace la plus longue, sinon à défaut prendre à gauche.

			// si les deux Call sont égaux et le coût minimal est la diagonale, prendre la diagonale
			if (s1[l-1]->operator==(s2[c-1].get()) && transformationMatrix[l-1][c-1] <= std::min(transformationMatrix[l-1][c], transformationMatrix[l][c-1])){
				if (step == 0){
					// fusionner les deux Call
					Trace::sp_trace merged = s1[l-1]->clone();
					dynamic_cast<Call *>(merged.get())->filterCall(dynamic_cast<const Call *>(s2[c-1].get()));
					mergedSequence.push_back(merged);
					// noter que la séquence mère ne peux plus être optionnelle
					if (stack.size() > 0)
						std::get<1>(stack.back())->setOptional(false);
				} else {
					// On définit la trace comme optionnelle si une des deux sources est optionnelle OU si la tête de la pile nous indique un chevauchement de séquence
					mergedSequence[mergedPos]->setOptional(s1[l-1]->isOptional() || s2[c-1]->isOptional() || (stack.size() > 0 && std::get<2>(stack.back())));
					mergedPos++;
				}
				l--;
				c--;
			}
			// sinon si coût minimum sur la ligne d'en dessus ou coût égal mais le nombre de ligne est plus grand que le nombre de colonne, prendre la ligne du dessus
			else if (transformationMatrix[l-1][c] < transformationMatrix[l][c-1] || (transformationMatrix[l-1][c] == transformationMatrix[l][c-1] && s1.size() > s2.size())){
				if (step == 0){
					Trace::sp_trace selection = s1[l-1]->clone();
					mergedSequence.push_back(selection);
				} else {
					// On met ce Call optionnel si on n'est pas dans une séquence ou si la séquence mère n'est pas optionnelle
					mergedSequence[mergedPos]->setOptional(stack.size() == 0 || !std::get<1>(stack.back())->isOptional());
					mergedPos++;
				}
				l--;
			}
			// sinon on prend la colonne de gauche
			else{
				if (step == 0){
					Trace::sp_trace selection = s2[c-1]->clone();
					mergedSequence.push_back(selection);
				} else {
					// On met ce Call optionnel si on n'est pas dans une séquence ou si la séquence mère n'est pas optionnelle
					mergedSequence[mergedPos]->setOptional(stack.size() == 0 || !std::get<1>(stack.back())->isOptional());
					mergedPos++;
				}
				c--;
			}
		}
		// une des traces est un Call et l'autre est une séquence
		else if ((s1[l-1]->isCall() && s2[c-1]->isSequence()) || (s1[l-1]->isSequence() && s2[c-1]->isCall())){
			// privilégier l'orientation vers le haut ou la gauche avec le poid minimal (interdire la diagonale). Si égalité privilégier l'orientation du Seq.

			Trace::sp_trace selection;
			std::string action;
			// si le coût de la ligne du haut est plus petit que la colonne de gauche ou qu'ils sont égaux et que la séquence se trouve sur la ligne du haut, prendre la ligne
			if (transformationMatrix[l-1][c] < transformationMatrix[l][c-1] || (transformationMatrix[l-1][c] == transformationMatrix[l][c-1] && s1[l-1]->isSequence())){
				action = "l";
				if (step == 0){
					selection = s1[l-1]->clone();
					mergedSequence.push_back(selection);
				} else {
					selection = mergedSequence[mergedPos];
					// On ne touche pas à l'option dans le cas où on est sur un End
					if (mergedSequence[mergedPos]->getInfo() != "End"){
						// On définit cette trace comme optionnelle si c'est un Call et que la pile est vide OU si c'est un Call inclus dans une séquence non optionnelle OU s'il s'agit d'un début de séquence dont sa fin est restée taguée optionnelle OU si la tête de la pile nous indique un chevauchement de séquence
						selection->setOptional((mergedSequence[mergedPos]->isCall() && (stack.size() == 0 || !std::get<1>(stack.back())->isOptional())) || (mergedSequence[mergedPos]->getInfo() == "Begin" && std::get<1>(stack.back())->isOptional()) || (stack.size() > 0 && std::get<2>(stack.back())));
					}
					mergedPos++;
				}
				l--;
			}
			// si le coût de la colonne de gauche est plus petit que la ligne du haut ou qu'ils sont égaux et que la séquence se trouve sur la colonne de gauche, prendre la colonne
			else if (transformationMatrix[l][c-1] < transformationMatrix[l-1][c] || (transformationMatrix[l-1][c] == transformationMatrix[l][c-1] && s2[c-1]->isSequence())){
				action = "c";
				if (step == 0){
					selection = s2[c-1]->clone();
					mergedSequence.push_back(selection);
				} else {
					selection = mergedSequence[mergedPos];
					// On ne touche pas à l'option dans le cas où on est sur un End
					if (mergedSequence[mergedPos]->getInfo() != "End"){
						// On définit cette trace comme optionnelle si c'est un Call et que la pile est vide OU si c'est un Call inclus dans une séquence non optionnelle OU s'il s'agit d'un début de séquence dont sa fin est restée taguée optionnelle OU si la tête de la pile nous indique un chevauchement de séquence
						selection->setOptional((mergedSequence[mergedPos]->isCall() && (stack.size() == 0 || !std::get<1>(stack.back())->isOptional())) || (mergedSequence[mergedPos]->getInfo() == "Begin" && std::get<1>(stack.back())->isOptional()) || (stack.size() > 0 && std::get<2>(stack.back())));
					}
					mergedPos++;
				}
				c--;
			}

			// Gestion de la pile si la sélection est une séquence
			if (selection && selection->isSequence()){
				manageStack(stack, mergedSequence, action, selection, step);
			}
		}
		// Ici les deux traces sont des séquences
		else{
			// Si la diagonale est le coût minimal et colonne c == ligne l privilégier la diagonale, sinon privilégier le min entre haut et gauche, si égalité et colonne c != ligne l privilégier le Begin sinon si égalité et colonne c == ligne l réduire en priorité la trace la plus longue, sinon à défaut prendre à gauche.

			Trace::sp_trace selection;
			std::string action;
			// si les deux Séquences sont du même type et le coût minimal est la diagonale, prendre la diagonale
			if (s1[l-1]->getInfo() == s2[c-1]->getInfo() && transformationMatrix[l-1][c-1] <= std::min(transformationMatrix[l-1][c], transformationMatrix[l][c-1])){
				action = "d";
				if (step == 0){
					// fusionner les deux Sequences
					selection = s1[l-1]->clone();
					std::dynamic_pointer_cast<Sequence>(selection)->mergeIterationDescription(std::dynamic_pointer_cast<Sequence>(s2[c-1])->getIterationDescription());
					mergedSequence.push_back(selection);
				} else {
					selection = mergedSequence[mergedPos];
					// On ne touche pas à l'option dans le cas où on est sur un End
					if (mergedSequence[mergedPos]->getInfo() != "End"){
						// prise en compte des éventuelles traces optionnelles OU si la tête de la pile nous indique un chevauchement de séquence
						selection->setOptional(s1[l-1]->isOptional() || s2[c-1]->isOptional() || (stack.size() > 0 && std::get<2>(stack.back())));
					}
					mergedPos++;
				}
				l--;
				c--;
			}
			// sinon si coût minimum sur la ligne d'en dessus ou coût égal et (les deux séquences sont différentes et celle de la ligne d'au dessus est un Begin OU les deux séquence sont de même nature et le nombre de ligne est plus grand que le nombre de colonne), prendre la ligne du dessus
			else if (transformationMatrix[l-1][c] < transformationMatrix[l][c-1] ||
					(transformationMatrix[l-1][c] == transformationMatrix[l][c-1] && 
						((s1[l-1]->getInfo() != s2[c-1]->getInfo() && s1[l-1]->getInfo() == "Begin") ||
						(s1.size() > s2.size())))){
				if (step == 0){
					Trace::sp_trace selection = s1[l-1]->clone();
					mergedSequence.push_back(selection);
				} else {
					selection = mergedSequence[mergedPos];
					// On ne touche pas à l'option dans le cas où on est sur un End
					if (mergedSequence[mergedPos]->getInfo() != "End"){
						// On définit cette trace comme optionnelle s'il s'agit d'un début de séquence dont sa fin est restée taguée optionnelle OU si la tête de la pile nous indique un chevauchement de séquence
						selection->setOptional((mergedSequence[mergedPos]->getInfo() == "Begin" && std::get<1>(stack.back())->isOptional()) || (stack.size() > 0 && std::get<2>(stack.back())));
					}
					mergedPos++;
				}
				l--;
			}
			// sinon on prend la colonne de gauche
			else{
				if (step == 0){
					Trace::sp_trace selection = s2[c-1]->clone();
					mergedSequence.push_back(selection);
				} else {
					selection = mergedSequence[mergedPos];
					// On ne touche pas à l'option dans le cas où on est sur un End
					if (mergedSequence[mergedPos]->getInfo() != "End"){
						// On définit cette trace comme optionnelle s'il s'agit d'un début de séquence dont sa fin est restée taguée optionnelle OU si la tête de la pile nous indique un chevauchement de séquence
						selection->setOptional((mergedSequence[mergedPos]->getInfo() == "Begin" && std::get<1>(stack.back())->isOptional()) || (stack.size() > 0 && std::get<2>(stack.back())));
					}
					mergedPos++;
				}
				c--;
			}

			// Gestion de la pile si la sélection est une séquence
			manageStack(stack, mergedSequence, action, selection, step);
		}
	}
}

/**
 * \brief Extraction de l'ensemble des calls contenus dans le vecteur de traces de la séquence
 *
 * \param setMod : un booléen qui est à faux si on autorise les doublons, et à vrai sinon
 *
 * \return un vecteur de calls
 */
Call::call_vector Sequence::getCalls(bool setMod)
{
	Call::call_vector v;
	Sequence::sp_sequence sps = shared_from_this();
	if (sps->length() > 0)
	{
		Trace::sp_trace spt;
		sps->reset();
		std::stack<Sequence::sp_sequence> stack;
		stack.push(sps);
		while (!stack.empty())
		{
			while (!stack.empty() && sps->isEndReached())
			{
				stack.pop();
				if (!stack.empty())
					sps = stack.top();
			}
			if (!sps->isEndReached() && sps->length() > 0)
			{
				spt = sps->next();
				if (spt->isSequence())
				{
					sps = std::dynamic_pointer_cast<Sequence>(spt);
					sps->reset();
					stack.push(sps);
				}
				else if (spt->isCall())
				{
					Call::sp_call spc = std::dynamic_pointer_cast<Call>(spt);
					if (!setMod)
						v.push_back(spc);
					else
					{
						bool found = false;
						for (unsigned int j = 0; !found && j < v.size(); j++)
						{
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

void Sequence::filterSequence(const Sequence *s)
{
	auto itt = iterDesc.begin();
	while(itt!=iterDesc.end()){
		std::cout<<itt->first<<" : "<<itt->second<<std::endl;
		itt++;
	}
	for(unsigned int i=0; i<traces.size(); ++i){
		if(traces.at(i)->isCall())
			dynamic_cast<Call *>(traces.at(i).get())->filterCall(dynamic_cast<const Call *>(s->traces[i].get()));
		else if(traces.at(i)->isSequence())
			dynamic_cast<Sequence *>(traces.at(i).get())->filterSequence(dynamic_cast<const Sequence *>(s->traces[i].get()));
	}
}