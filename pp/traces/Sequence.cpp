#include "Sequence.h"
#include "Event.h"
#include "TraceConstantList.h"
#include "TracesParser.h"

// Cette donnée est utilisée lors des différentes recherches de parties optionnelles. Lors de ces étapes on recherche en périphérie de la séquence à traiter des traces alignables avec cette séquence. Si la partie de la séquence analysée a une taille X, on s'autorise à regarder en périphérie (amont ou aval) X+MAX_OPT_WINDOW traces. Et donc potentiellement un nombre de traces optionnelles pour les parties périphériques égale à MAX_OPT_WINDOW.
#define MAX_OPT_WINDOW 10
// Toujours lors de ces étapes d'identification des partie optionnelles on acceptera une fusion uniquement si le score d'alignement est supérieur à MIN_SCORE
#define MIN_SCORE 0.6

bool debug = false;

Sequence::Sequence(std::string info, bool num_fixed) : Trace(SEQUENCE, info), newIter(false), num_fixed(num_fixed), num(1), pt(0), endReached(false), shared(false), root(false) {
	addIteration(1);
}

Sequence::Sequence(unsigned int num, bool root, bool opt) : Trace(SEQUENCE), newIter(false), num_fixed(false), num(num), pt(0), endReached(false), shared(false), root(root)
{
	addIteration(num);
	setOptional(opt);
}

Sequence::Sequence(const_sp_sequence sps) : Trace(sps.get()), pt(0), endReached(false), shared(false)
{
	num = sps->getNum();
	num_fixed = sps->hasNumberIterationFixed();
	root = sps->isRoot();
	newIter = sps->newIter;
	mergeIterationDescription(sps->getIterationDescription());
}

Sequence::Sequence(const_sp_sequence sps_up, const_sp_sequence sps_down) : Trace(SEQUENCE), pt(0), endReached(false), shared(true)
{
	num = std::max(sps_up->getNum(), sps_down->getNum());
	num_fixed = sps_up->hasNumberIterationFixed() && sps_down->hasNumberIterationFixed();
	root = false;
	opt = sps_up->isOptional() || sps_down->isOptional();
	newIter = sps_up->newIter || sps_down->newIter;
	mergeIterationDescription(sps_up->getIterationDescription());
	mergeIterationDescription(sps_down->getIterationDescription());
}

bool Sequence::operator==(Trace *t) const
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

void Sequence::resetAligned()
{
	Trace::resetAligned();
	for (unsigned int i = 0; i < traces.size(); i++)
		traces.at(i)->resetAligned();
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
	sp_sequence sps_clone = boost::make_shared<Sequence>(shared_from_this());
	for (unsigned int i = 0; i < traces.size(); i++)
		sps_clone->addTrace(traces.at(i)->clone());
	return sps_clone;
}

void Sequence::exportAsString(std::ostream &os) const
{
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

void Sequence::exportAsCompressedString(std::ostream &os) const
{
	if (numTab == 0)
		os << " ";
	numTab++;
	if (opt)
		os << "*";
	if (!root)
		os << "Sequence"
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

void Sequence::exportLinearSequenceAsString(std::vector<Trace::sp_trace> & linearSequence, std::ostream &os)
{
	for (int i = 0 ; i < (signed)linearSequence.size() ; i++)
	{
		if (linearSequence[i]->isSequence())
			os << "Seq(" << linearSequence[i]->getInfo() << "*" << boost::dynamic_pointer_cast<Sequence>(linearSequence[i])->getNum() << ")\t";
		else
			linearSequence[i]->exportAsCompressedString(os);
	}
	os << std::endl;
}

Call::sp_call Sequence::getFirstCall(){
	Trace::sp_trace tr = at(0);
	if (tr->isSequence())
		return boost::dynamic_pointer_cast<Sequence>(tr)->getFirstCall();
	return boost::dynamic_pointer_cast<Call>(tr);
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

std::vector<Trace::sp_trace> Sequence::getSubTraces(int start)
{
	std::vector<Trace::sp_trace> remainingTraces;
	start = start < 0 ? 0 : start;
	for (int i = start; i < (int)traces.size(); i++)
	{
		remainingTraces.push_back(traces.at(i));
	}
	return remainingTraces;
}

Sequence::sp_sequence Sequence::getSubSequence(int start, int end)
{
	start = start < 0 ? 0 : start;
	end = end < 0 || end > (int)this->size() ? this->size() : end;
	Sequence::sp_sequence subSeq = boost::make_shared<Sequence>(1, false, false);
	for (int i = start; i < end; i++)
	{
		subSeq->addTrace(traces.at(i)->clone());
	}
	return subSeq;
}

int Sequence::getIndex(const Trace::sp_trace &spt) const
{
	for (unsigned int i = 0; i < traces.size(); i++)
	{
		if (traces.at(i) == spt)
			return i;
	}
	return -1;
}

bool Sequence::isRoot() const
{
	return root;
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
		Sequence::sp_sequence seq = boost::dynamic_pointer_cast<Sequence>(spt);
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

bool Sequence::isShared() const
{
	return shared;
}

bool Sequence::isImplicit() const
{
	return iterDesc.size() == 1 && iterDesc.find(1) != iterDesc.end() && iterDesc.at(1) == 1;
}

bool Sequence::checkDelayed()
{
	if (!delayed)
	{
		for (unsigned i = 0; i < traces.size(); i++)
		{
			if (traces.at(i)->isSequence())
			{
				Sequence *s = dynamic_cast<Sequence *>(traces.at(i).get());
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

void Sequence::completeIterationDescription()
{
	if (!getParent().expired())
	{
		Sequence::sp_sequence sps = boost::dynamic_pointer_cast<Sequence>(getParent().lock());
		unsigned int num = 0;
		std::map<unsigned int, unsigned int>::const_iterator it = sps->getIterationDescription().begin();
		while (it != sps->getIterationDescription().end())
		{
			num += it->first * it->second;
			it++;
		}
		if (iterDesc.find(1) != iterDesc.end())
		{
			it = iterDesc.begin();
			while (it != iterDesc.end())
			{
				num -= it->second;
				it++;
			}
			addIteration(1, num);
		}
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

// Parcours une trace linéarisée et supprime le premier Call
void Sequence::removeFirstCall(std::vector<Trace::sp_trace> &linearTraces)
{
	unsigned int i = 0;
	// Avancer dans la trace linéarisée jusqu'à atteindre le premier Call ou Event
	while (i < linearTraces.size() && linearTraces.at(i)->isSequence())
		i++;
	// Si on a trouvé un Call
	if (i < linearTraces.size())
	{
		// On le supprime
		linearTraces.erase(linearTraces.begin() + i);
		// La suppression du Call peut générer une séquence vide qui peuvent éventuellement être imbriquées => Suppression des séquences vides
		while (i > 1 && linearTraces.at(i - 1)->isSequence() && linearTraces.at(i)->isSequence())
		{
			Sequence *s1 = dynamic_cast<Sequence *>(linearTraces.at(i - 1).get());
			Sequence *s2 = dynamic_cast<Sequence *>(linearTraces.at(i).get());
			if (s1->getInfo().compare("Begin") == 0 && s2->getInfo().compare("End") == 0)
			{
				// Suppression de ces deux traces qui marquent une séquence vide
				linearTraces.erase(linearTraces.begin() + (i - 1), linearTraces.begin() + (i + 1));
			}
			i--;
		}
	}
}

// Compte le nombre de Call alignés et optionnels entre deux séquences
// Retourne un couple (nbAlign, nbOpt) où nbAlign indique le nombre de traces alignées et nbOpt indique le nombre de traces optionnelles
std::pair<int, int> Sequence::getNbAlignAndOpt(std::vector<Trace::sp_trace> &linearTraces1, std::vector<Trace::sp_trace> &linearTraces2)
{
	unsigned int i = 0, j = 0;
	int nbOpt = 0, nbAlign = 0;
	// pile permettant de stocker l'ordre d'entrée dans les séquences pour ne sortir d'une séquence que si elle n'est pas bloquée
	std::stack<int> stack;
	// Parcours de la trace linéarisée 1
	while (i < linearTraces1.size())
	{
		if (linearTraces1.at(i)->isSequence())
		{
			Sequence *s1 = dynamic_cast<Sequence *>(linearTraces1.at(i).get());
			// Si c'est une entrée de séquence on insère dans la pile la valeur 1 pour indiquer qu'on entre dans une séquence des traces 1
			if (s1->getInfo().compare("Begin") == 0)
			{
#ifdef DEBUG_PARSER
				TracesParser::osParser << "[A ";
#endif
				stack.push(1);
				// Si la trace 2 est aussi sur une entrée de séquence, on monte aussi pour garder les deux synchronisées
				if (j < linearTraces2.size() && linearTraces2.at(j)->isSequence())
				{
					Sequence *s2 = dynamic_cast<Sequence *>(linearTraces2.at(j).get());
					if (s2->getInfo().compare("Begin") == 0)
					{
#ifdef DEBUG_PARSER
						TracesParser::osParser << "[B ";
#endif
						stack.push(2);
						j++;
					}
				}
			}
			else if (s1->getInfo().compare("End") == 0)
			{
				// On ne peut sortir de cette séquence que si la tête de la pile correspond à une séquence 1
				// Dépiler jusqu'à ce que la tête de la pile soit du type 1
				while (j < linearTraces2.size() && !stack.empty() && stack.top() != 1)
				{
					// Avancer dans la trace 2 et compter toutes ces traces comme optionnelles
					if (linearTraces2.at(j)->isSequence())
					{
						Sequence *s2 = dynamic_cast<Sequence *>(linearTraces2.at(j).get());
						if (s2->getInfo().compare("Begin") == 0)
						{
#ifdef DEBUG_PARSER
							TracesParser::osParser << "[B ";
#endif
							stack.push(2);
						}
						else if (s2->getInfo().compare("End") == 0)
						{
#ifdef DEBUG_PARSER
							TracesParser::osParser << "B] ";
#endif
							stack.pop();
						}
						else
							throw std::runtime_error("Sequence::getNbAlignAndOpt => linear traces malformed");
					}
					else
					{
#ifdef DEBUG_PARSER
						TracesParser::osParser << "(B" << j << ") ";
#endif
						nbOpt++;
					}
					j++;
				}
				if (!stack.empty())
				{
#ifdef DEBUG_PARSER
					TracesParser::osParser << "A] ";
#endif
					stack.pop(); // Libérer cette séquence
				}
			}
			else
			{
				// On ne devrait pas trouver dans une trace linéariser des séquences non tagué "Begin" ou "End"
				throw std::runtime_error("Sequence::getNbAlignAndOpt => linear traces malformed");
			}
		}
		else
		{
			// On est sur un Call de la trace 1, on cherche donc son équivalent dans la trace 2
			// sauvegarder la position des compteurs i et j pour pouvoir les restaurer si besoin
			int k = i;
			int l = j;
			int nbOpt2 = 0;
			std::stack<int> stack2(stack);
			std::ostringstream oss;
			// Progresser dans la séquence 2 jusqu'à trouver un alignement correct
			while (i < linearTraces1.size() && j < linearTraces2.size() && !linearTraces1.at(i)->operator==(linearTraces2.at(j).get()))
			{
				// Avancer dans la trace 2
				if (linearTraces2.at(j)->isSequence())
				{
					Sequence *s2 = dynamic_cast<Sequence *>(linearTraces2.at(j).get());
					if (s2->getInfo().compare("Begin") == 0)
					{
						oss << "[B ";
						stack2.push(2);
					}
					else if (s2->getInfo().compare("End") == 0)
					{
						// On ne peut sortir de cette séquence que si la tête de la pile correspond à une séquence 2
						// Dépiler jusqu'à ce que la tête de la pile soit du type 2
						while (i < linearTraces1.size() && !stack2.empty() && stack2.top() != 2)
						{
							// Avancer dans la trace 1 et compter toutes ces traces comme optionnelles
							if (linearTraces1.at(i)->isSequence())
							{
								Sequence *s1 = dynamic_cast<Sequence *>(linearTraces1.at(i).get());
								if (s1->getInfo().compare("Begin") == 0)
								{
									oss << "[A ";
									stack2.push(1);
								}
								else if (s1->getInfo().compare("End") == 0)
								{
									oss << "A] ";
									stack2.pop();
								}
								else
									throw std::runtime_error("Sequence::getNbAlignAndOpt => linear traces malformed");
							}
							else
							{
								oss << "(A" << i << ") ";
								nbOpt2++;
							}
							i++;
						}
						if (!stack2.empty())
						{
							oss << "B] ";
							stack2.pop(); // Libérer cette séquence
						}
					}
					else
						throw std::runtime_error("Sequence::getNbAlignAndOpt => linear traces malformed");
				}
				else
				{
					oss << "(B" << j << ") ";
					nbOpt2++;
				}
				j++;
			}
			// Si la fin des traces 2 a été atteint
			if (j >= linearTraces2.size())
			{
				// restaurer les compteurs pour repartir après le dernier alignement trouvé
				i = k;
				j = l;
#ifdef DEBUG_PARSER
				TracesParser::osParser << "(A" << i << ") ";
#endif
				nbOpt++;
			}
			else
			{
				// On a trouvé un alignement
				nbAlign++;
#ifdef DEBUG_PARSER
				TracesParser::osParser << oss.str() << "{AB" << i << "/" << j << "} ";
#endif
				j++;
				nbOpt += nbOpt2; // prise en compte des sauts dans les traces 2
				stack = stack2;  // prise en compte de l'état de la pile
				// Si on est en bout de séquences, on les libèrent dans la mesure du possible
				bool stop = false;
				while (i + 1 < linearTraces1.size() && j < linearTraces2.size() && !stack.empty() && !stop)
				{
					stop = true;
					if (linearTraces1.at(i + 1)->isSequence() && stack.top() == 1)
					{
						Sequence *s = dynamic_cast<Sequence *>(linearTraces1.at(i + 1).get());
						if (s->getInfo().compare("End") == 0)
						{
#ifdef DEBUG_PARSER
							TracesParser::osParser << "A] ";
#endif
							stack.pop();
							i++;
							stop = false;
						}
					}
					if (linearTraces2.at(j)->isSequence() && !stack.empty() && stack.top() == 2)
					{
						Sequence *s = dynamic_cast<Sequence *>(linearTraces2.at(j).get());
						if (s->getInfo().compare("End") == 0)
						{
#ifdef DEBUG_PARSER
							TracesParser::osParser << "B] ";
#endif
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
	for (unsigned int i = j; i < linearTraces2.size(); i++)
		if (!linearTraces2.at(i)->isSequence())
		{
#ifdef DEBUG_PARSER
			TracesParser::osParser << "(B" << i << ") ";
#endif
			nbOpt++;
		}
#ifdef DEBUG_PARSER
	TracesParser::osParser << std::endl;
#endif
	return std::pair<int, int>(nbAlign, nbOpt);
}

// Retourne le meilleur score d'alignement entre deux traces linéarisées
std::pair<int, int> Sequence::getMaxAlignAndMinOpt(std::vector<Trace::sp_trace> &linearTraces1, std::vector<Trace::sp_trace> &linearTraces2)
{
// Affichage du contenu des traces liéarisées
#ifdef DEBUG_PARSER
	TracesParser::osParser << "Up:" << std::endl;
	for (unsigned int i = 0; i < linearTraces1.size(); i++)
		linearTraces1.at(i)->exportAsString(TracesParser::osParser);
	TracesParser::osParser << "Down:" << std::endl;
	for (unsigned int i = 0; i < linearTraces2.size(); i++)
		linearTraces2.at(i)->exportAsString(TracesParser::osParser);
#endif
	// compter le nombre de Call (et d'Event) dans la première trace linéarisée
	unsigned int nbCalls1 = 0;
	for (unsigned int i = 0; i < linearTraces1.size(); i++)
		if (!linearTraces1.at(i)->isSequence())
			nbCalls1++;
	// compter le nombre de Call (et d'Event) dans la seconde trace linéarisée
	unsigned int nbCalls2 = 0;
	for (unsigned int i = 0; i < linearTraces2.size(); i++)
		if (!linearTraces2.at(i)->isSequence())
			nbCalls2++;
	// Initialisation des valeurs par défaut
	std::pair<int, int> ret(0, nbCalls1 + nbCalls2);
	// Pour chaque Call de la séquence 1
	for (unsigned int i = 0; i < nbCalls1; i++)
	{
		// Calculer les scores entre les deux traces linéarisées
		std::pair<int, int> tmp = Sequence::getNbAlignAndOpt(linearTraces1, linearTraces2);
		// Mettre à jour les meilleurs score si besoin
		if (tmp.first > ret.first || (tmp.first == ret.first && tmp.second + (int)i < ret.second))
		{
			ret.first = tmp.first;
			ret.second = tmp.second + i;
		}
		// réduction de la première trace
		Sequence::removeFirstCall(linearTraces1);

		// Si le meilleur nombre d'alignement dépasse la longueur restante de la première trace, on peut couper la boucle car on ne trouvera pas mieux
		if ((int)linearTraces1.size() < ret.first)
			break;
	}
#ifdef DEBUG_PARSER
	TracesParser::osParser << "Result => " << ret.first << "/" << ret.second << std::endl;
#endif
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
		Sequence::sp_sequence tmpSeq = boost::make_shared<Sequence>("Begin", -1);
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
				linearizedTraces.push_back(boost::make_shared<Sequence>("End", -1));
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
					sps = boost::dynamic_pointer_cast<Sequence>(spt);
					sps->reset();
					stack.push(sps);
					tmpSeq = boost::make_shared<Sequence>("Begin", -1);
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
			boost::dynamic_pointer_cast<Sequence>(this->traces.at(pos))->insertLinearSequence(subLinearSequence, 0);
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

int Sequence::getCallPosInLinearSequence (std::vector<Trace::sp_trace> & linearSequence, int num)
{
	num = num < 0 ? 0 : num;
	int lastCallFound = -1;
	for (int i = 0 ; i < (signed)linearSequence.size() ; i++){
		if (linearSequence[i]->isCall())
		{
			if (num <= 0)
				return i;
			else
			{
				lastCallFound = i;
				num--;
			}
		}
	}
	return lastCallFound;
}


int Sequence::getNonOptCallInLinearSequence (std::vector<Trace::sp_trace> & linearSequence, int start, int end){
	start = start < 0 ? 0 : start;
	end = (end < 0 || end > (signed)linearSequence.size()) ? linearSequence.size() : end;
	int cpt = 0;
	for (int i = start ; i < end ; i++){
		if (linearSequence[i]->isCall() && !linearSequence[i]->isOptional())
			cpt++;
	}
	return cpt;
}

/**
 * \brief Extraction de l'ensemble des sequences contenus dans le vecteur de traces de la séquence. La séquence appelante est inclus dans le résultat.
 *
 * \return un vecteur de sequences
 */
Sequence::sequence_vector Sequence::getSequences()
{
	Sequence::sequence_vector v;
	Trace::sp_trace spt;
	Sequence::sp_sequence sps = shared_from_this();
	sps->reset();
	std::stack<Sequence::sp_sequence> stack;
	stack.push(sps);
	v.push_back(sps);
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
					sps = boost::dynamic_pointer_cast<Sequence>(spt);
					sps->reset();
					stack.push(sps);
				}
				else if (spt->isCall())
				{
					Call::sp_call spc = boost::dynamic_pointer_cast<Call>(spt);
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

void Sequence::findAndAggregateSuccessiveSequences(int startingPoint)
{
	// Taille courrante des fenêtres utilisées pour les fusions. Cette taille augmente progressivement au cours de l'algorithme pour commencer par fusionner des petites séquences puis de plus en plus grandes.
	unsigned int windowSize = 1;
	
	unsigned int workingId;
	unsigned int upStart;
	unsigned int endDown;
	unsigned int lastEndFusion;
	
	bool mergeOccurs;
	
	// Point de départ dans la trace en fonction de la taille de la fenêtre
	unsigned int currentStartingPoint = startingPoint;
	
	Sequence::sp_sequence sps_up;

	#ifdef DEBUG_PARSER
		TracesParser::osParser << "WorkingSequence (start at " << startingPoint << "):" << std::endl;
		Trace::exportAsString(TracesParser::osParser, traces, startingPoint);
	#endif

	// Lorsque la taille des fenêtres * 2 dépassent la longueur des traces l'algorithme peut s'arrêter. En effet les deux fenêtres couvrent la totalité de la trace et n'ont pas pu être fusionnées, il ne sert donc plus à rien de tenter de continuer à augmenter la taille des fenêtres
	while(checkFeasibility(windowSize*2, startingPoint)) {
		// On augmente la taille de la fenêtre
		windowSize++;
		#ifdef DEBUG_PARSER
			TracesParser::osParser << "New window size: " << windowSize << std::endl;
		#endif
		std::cout << "New window size: " << windowSize << "; current size: " << size() << "; current length: " << length(startingPoint) << std::endl;
		// On recherche le point de départ permettant de construire une fenêtre "up" d'une taille supérieure ou égale à celle de la fenêtre
		while (Trace::getLength(traces, startingPoint, currentStartingPoint) < windowSize)
			currentStartingPoint++;
		// On positionne l'indice de travail à ce nouveau point de départ
		workingId = currentStartingPoint;
		mergeOccurs = false;
		
		// On parcours toutes les traces restantes jusqu'à ce qu'il n'y en ait plus assez pour construire une séquence "down" d'une taille correspondante à celle de la fenêtre
		while (checkFeasibility(windowSize, workingId)) {
			if ((size() - workingId) % 100 == 0)
				std::cout << "Window size: " << windowSize << "; remaining traces: " << size() - workingId << " " << workingId << std::endl;
			#ifdef DEBUG_PARSER
				TracesParser::osParser << "WorkingId " << workingId << std::endl;
				Trace::exportAsString(TracesParser::osParser, traces, startingPoint);
			#endif
			// Si la trace courante est un évènement, on passe au suivant
			if (at(workingId)->isEvent()) {
				workingId++;
				#ifdef DEBUG_PARSER
					TracesParser::osParser << "Move to next trace due to event detection" << std::endl;
				#endif
				continue;
			}
			// Tentative de création d'une séquence "up" de la taille de la fenêtre (mergeOccurs peut être à true si une fusion vient d'être faite à l'itération précédente, dans ce cas pas besoin de reconstruire la séquence "up" => Autrement dit, il faut construire une séquence "up" s'il n'y a pas eu de fusion à l'itération précédente)
			if (!mergeOccurs){
				//  1 - Création d'une séquence "up" pour acceuillir les traces de la fenêtre 
				sps_up = boost::make_shared<Sequence>(1);
				//  2 - Intégration des traces précédentes jusqu'à ce que la taille de la fenêtre soit atteinte
				if (at(workingId-1)->length() == windowSize){
					// 2.1 - Cas particulier où la trace précédente correspond à la taille de la fenêtre. Dans ce cas là on peut l'utiliser directement sans l'encapsuler dans une sequence artificielle
					sps_up = boost::dynamic_pointer_cast<Sequence>(at(workingId-1));
					upStart = workingId-1;
				} else {
					// 2.2 - Cas général, on remonte la trace pour construire la séquence "up"
					// Initialisation du compteur sur la trace précédant la position de travail (rappel la trace correspondant à la position de travail fait partie de la séquence "down")
					unsigned int _cpt = workingId-1;
					// On construit la séquence "up" en remontant la trace
					while (sps_up->length() < windowSize){
						sps_up->addTrace(at(_cpt));
						_cpt--;
					}
					// We reverse "up" vector because we built it from end to start
					std::reverse(sps_up->getTraces().begin(),sps_up->getTraces().end()); 
					upStart = _cpt+1;
				}
				//  3 - Si la fenêtre "up" n'a pas la taille souhaitée, on avance donc dans l'analyse des traces
				if (sps_up->length() != windowSize){
					workingId++;
					#ifdef DEBUG_PARSER
						TracesParser::osParser << "Move to next trace due to wrong up sequence size" << std::endl;
					#endif
					continue;
				}
			}
			
			// Ici nous avons donc une séquence "up" d'une taille correspondant à la fenêtre de recherche
			// Nous allons maintenant recherche une séquence "down" de la même taille et tenter une fusion
			
			// Même algo que pour la construction de la séquence "up"
			// Tentative de création d'une séquence "down" de la taille de la fenêtre
			//  1 - Création d'une séquence "down"
			Sequence::sp_sequence sps_down = boost::make_shared<Sequence>(1);
			//  2 - Intégration des traces faisant partie de la séquence "down" jusqu'à ce que la taille de la fenêtre soit atteinte
			if (at(workingId)->length() == windowSize){
				// 2.1 - Cas particulier où la trace courante correspond à la taille de la fenêtre. Dans ce cas là on peut l'utiliser directement sans l'encapsuler dans une sequence artificielle
				sps_down = boost::dynamic_pointer_cast<Sequence>(at(workingId));
				endDown = workingId;
			} else {
				// 2.2 - Cas général, on progresse dans la trace pour construire la séquence "down"
				// Initialisation du compteur pour parcourir la trace en aval (ici pas de -1 car la trace courrante doit être intégrée à la séquence "down")
				unsigned int _cpt = workingId;
				// On construit la séquence "down" en avancant dans la trace
				while (sps_down->length() < windowSize){
					sps_down->addTrace(at(_cpt));
					_cpt++;
				}
				endDown = _cpt;
			}
			//  3 - Si la fenêtre "down" n'a pas la taille souhaitée, on avance donc dans l'analyse des traces
			if (sps_down->length() != windowSize){
				workingId++;
				#ifdef DEBUG_PARSER
					TracesParser::osParser << "Move to next trace due to wrong down sequence size" << std::endl;
				#endif
				continue;
			}
			
			// Ici nous avons donc une séquence "down" d'une taille correspondant à la fenêtre de recherche
			// Nous allons maintenant tenter de fusionner les séquences "up" et "down"
			Sequence::sp_sequence sps_res = mergeEquivalentSequences(sps_up, sps_down);
			// Vérifier si la fusion a réussie
			if (sps_res) {
				// dans ce cas la séquence "up" devient la fusion
				sps_up = sps_res;
				// Optimisation: dans le cas où d'autres séquences "down" serait fusionnable, on continue à descendre dans la trace jusqu'à ce qu'on ne puisse plus fusionner. On ne nettoiera la trace qu'à ce moment là (suppression des séquences et ajout de la fusion).
				mergeOccurs = true;
				lastEndFusion = endDown;
				// on repositionne l'indice de travail juste après la séquence "down"
				workingId = endDown;
			}
			else{
				// La fusion a échoué, si des fusions précédentes ont été détectées dans les itérations présédentes => on nettoie la trace
				if (mergeOccurs){
					cleanTracesAndAddMerge(traces, sps_up, upStart, lastEndFusion);
					// On repositionne l'indice de travail sur l'élément fusionné
					workingId = upStart;
					// Repositionner le dernier point de départ si la fusion a créé un décallage
					// Exemple : soit la trace [AB*ABC] avec une fenêtre de 2 où * indique le dernier point de départ. Dans ce cas là "up" <=> [AB] et "down" <=> [AB]. la fusion va donc donner [[AB]C]. Si le dernier point de départ n'est pas recallé on sera dans la situation [[AB]C*] et la trace C sera "sautée". Il faut que le dernier point de départ soit repositionné au départ de la dernière fusion si celle-ci se trouve en amont : [*[AB]C]
					if (upStart < currentStartingPoint)
						currentStartingPoint = upStart;
					
				}

				// on avance maintenant dans l'analyse des traces
				workingId++;
				#ifdef DEBUG_PARSER
					TracesParser::osParser << "Move to next trace due to fusion failed" << std::endl;
				#endif
				mergeOccurs = false;
			}
		}
		// Gérer une éventuelle fusion en attente
		if (mergeOccurs){
			cleanTracesAndAddMerge(traces, sps_up, upStart, lastEndFusion);
			// Voir commentaire du même "if" précédant 
			if (upStart < currentStartingPoint)
				currentStartingPoint = upStart;
		}
	}


/*	// Taille courrante des fenêtres utilisées pour les fusions. Cette taille augmente progressivement au cours de l'algorithme pour commencer par fusionner des petites séquences puis de plus en plus grandes.
	int minWindowSize = 1;

	int workingId;
	int upStart;
	int endDown;
	int lastEndFusion;

	bool mergeOccurs;

	// Point de départ dans la trace en fonction de la taille de la fenêtre
	int currentStartingPoint = startingPoint;

	Sequence::sp_sequence sps_up;

#ifdef DEBUG_PARSER
	TracesParser::osParser << "WorkingSequence (start at " << startingPoint << "):" << std::endl;
	Trace::exportAsString(TracesParser::osParser, traces, startingPoint);
#endif

	// Lorsque la taille des fenêtres * 2 dépassent la longueur des traces l'algorithme peut s'arrêter. En effet les deux fenêtres couvrent la totalité de la trace et n'ont pas pu être fusionnées, il ne sert donc plus à rien de tenter de continuer à augmenter la taille des fenêtres
	while (checkFeasibility(minWindowSize * 2, startingPoint))
	{
		// On augmente la taille de la fenêtre
		minWindowSize++;
		int maxLength = minWindowSize + MAX_OPT_WINDOW;
#ifdef DEBUG_PARSER
		TracesParser::osParser << "New window size: " << minWindowSize << std::endl;
#endif
		std::cout << "New window size: " << minWindowSize << "; current size: " << size() << "; current length: " << length(startingPoint) << std::endl;
		// On recherche le point de départ permettant de construire une fenêtre "up" d'une taille supérieure ou égale à celle de la fenêtre
		while ((signed)Trace::getLength(traces, startingPoint, currentStartingPoint) < minWindowSize)
			currentStartingPoint++;
		// On positionne l'indice de travail à ce nouveau point de départ
		workingId = currentStartingPoint;
		mergeOccurs = false;
		// On parcours toutes les traces restantes jusqu'à ce qu'il n'y en ait plus assez pour construire une séquence "down" d'une taille correspondante à celle de la fenêtre
		while (checkFeasibility(minWindowSize, workingId))
		{
			// Si la trace courante est un évènement, on passe au suivant
			if (at(workingId)->isEvent())
			{
				workingId++;
				continue;
			}

			if ((size() - workingId) % 100 == 0)
				std::cout << "Windows size: " << minWindowSize << "; remaining traces: " << size() - workingId << " " << workingId << std::endl;

			float bestScore = 0;
			Sequence::sp_sequence bestMerge;
			// Tentative de création d'une séquence "up" de la taille de la fenêtre (mergeOccurs peut être à true si une fusion vient d'être faite à l'itération précédente, dans ce cas pas besoin de reconstruire la séquence "up" => Autrement dit, il faut construire une séquence "up" s'il n'y a pas eu de fusion à l'itération précédente)
			if (!mergeOccurs)
			{
				//  1 - Création d'une séquence "up" pour acceuillir les traces de la fenêtre
				sps_up = boost::make_shared<Sequence>(1);
				//  2 - Intégration des traces précédentes jusqu'à ce que la taille minimale de la fenêtre soit atteinte
				if ((signed)at(workingId - 1)->length() == minWindowSize)
				{
					// 2.1 - Cas particulier où la trace précédente correspond à la taille de la fenêtre. Dans ce cas là on peut l'utiliser directement sans l'encapsuler dans une sequence artificielle
					// Ici nous sommes forcément sur une séquence car minWindowSize a une taille initiale de 2 (incrémenté de 1 dès la première itération de la boucle principale)
					sps_up = boost::dynamic_pointer_cast<Sequence>(at(workingId - 1));
					upStart = workingId - 1;
				}
				else
				{
					// 2.2 - Cas général, on remonte la trace pour construire la séquence "up"
					// Initialisation du compteur sur la trace précédant la position de travail (rappel la trace correspondant à la position de travail fait partie de la séquence "down")
					int _cpt = workingId - 1;
					// On construit la séquence "up" en remontant la trace (les traces optionnelles sont ici comptées dans la longueur de la séquence "up")
					while ((signed)sps_up->length() < minWindowSize && _cpt >= startingPoint)
					{
						sps_up->addTrace(at(_cpt));
						_cpt--;
					}
					upStart = _cpt + 1;
					// On inverse la séquence "up" car on l'a construite de la fin vers le début
					std::reverse(sps_up->getTraces().begin(), sps_up->getTraces().end());
				}
				//  3 - Si la fenêtre "up" n'a pas la taille minimale recherchée, on passe à la trace suivante
				if ((signed)sps_up->length() != minWindowSize)
				{
					workingId++;
					continue;
				}
				// 4 - Si on a pu construire une fenêtre "up" de la taille minimale requise on va essayer de trouver un alignement avec les traces "down". On fera aussi attention si on trouve un alignement avec option qu'il n'y aurait une trace "up" plus grande qui permettrait d'obtenir un alignement parfait
				else
				{
					// On construit plusieurs tailles de fenêtre "up"
					int tmpWorkingId = workingId;
					int currentWindowSize = minWindowSize;
					// Création d'un clone de la séquence "up" pour ne pas changer la trace principale lorsqu'on fait grossir le "up"
					Sequence::sp_sequence sps_upExtended = boost::dynamic_pointer_cast<Sequence>(sps_up->clone());
					while (currentWindowSize < maxLength && bestScore >= 0 && bestScore < 1)
					{
						// Ici nous avons donc une séquence "up" consistante d'une taille correspondant à la fenêtre de recherche en prenant en compte ces traces optionnelles
						// Nous allons maintenant tenter des fusions avec plusieurs tailles de traces "down" pour ne retenir que la plus efficace
						evaluateMerging(tmpWorkingId, maxLength, sps_upExtended, (currentWindowSize == minWindowSize), bestScore, bestMerge, endDown);
						
						// Si bestScore est positif cela signifie qu'un alignement a été possible, on va donc chercher s'il n'y en a pas un de meilleur avec une fenêtre "up" plus grande
						if (bestScore >= 0)
						{
							// Si on a atteint la fin de la trace ou que la trace analysée est un event non fusionable, on stoppe l'analyse
							if (bestScore == 1 || tmpWorkingId >= (signed)size() || (at(tmpWorkingId)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(at(tmpWorkingId))->getLabel().c_str(), Event::noConcatEventsArr) > -1))
								currentWindowSize = maxLength;
							else
							{
								sps_upExtended->addTrace(at(tmpWorkingId));
								tmpWorkingId++;
								currentWindowSize++;
							}
						} else
							// Dans ce cas, c'est qu'on a détecté qu'il était plus interessant que le "up" grossisse
							// Si on a trouvé un alignement parfait, on peut positionner la tête de lecture juste après la fin de cette fenêtre "up"
							if (bestScore == -1)
								workingId = endDown-1; // -1 car on va l'incrémenter de 1 à la fin de la boucle vue que bestScore < 0
					}
				}
			}
			else // Cas où mergeOccurs == true => cela signifie que l'on peut poursuivre la descente dans la trace pour tenter à continuer à fusionner des sequences down sans avoir à reconstruire la séquence up. On est donc forcement dans le cas où on exploite la fenêtre "up" de taille minimale
				evaluateMerging(workingId, maxLength, sps_up, true, bestScore, bestMerge, endDown);

			// Si on n'est pas sensé passer à la prochaine trace cela signifie que la meilleure fusion à faire est avec le "up" de taille minimale. Il faut alors vérifier si le score obtenu est tout de même supérieur au seuil minimal requis
			if (bestScore > 0)
			{
				// Analyse des scores de fusion pour déterminer si la meilleure fusion est suffisante
				if (bestScore >= MIN_SCORE)
				{
					// dans ce cas la séquence "up" devient la fusion
					sps_up = bestMerge;
					// Optimisation: dans le cas où d'autres séquences "down" seraient fusionnables, on continue à descendre dans la trace jusqu'à ce qu'on ne puisse plus fusionner. On ne nettoiera la trace qu'à ce moment là (suppression des séquences et ajout de la fusion).
					mergeOccurs = true;
					lastEndFusion = endDown;
					// on repositionne l'indice de travail juste après la séquence "down"
					workingId = endDown;
				}
				else
				{
					// La fusion n'a pas été retenu, si des fusions précédentes ont été détectées dans les itérations présédentes => on nettoie la trace
					if (mergeOccurs)
					{
						cleanTracesAndAddMerge(traces, sps_up, upStart, lastEndFusion);
						// On repositionne l'indice de travail sur l'élément fusionné
						workingId = upStart;

						// Repositionner le dernier point de départ si la fusion a créé un décallage
						// Exemple : soit la trace [AB*ABC] avec une fenêtre de 2 où * indique le dernier point de départ. Dans ce cas là "up" <=> [AB] et "down" <=> [AB]. la fusion va donc donner [[AB]C]. Si le dernier point de départ n'est pas recallé on sera dans la situation [[AB]C*] et la trace C sera "sautée". Il faut que le dernier point de départ soit repositionné au départ de la dernière fusion si celle-ci se trouve en amont : [*[AB]C]
						if (upStart < currentStartingPoint)
							currentStartingPoint = upStart;
					}
					workingId++;
					mergeOccurs = false;
				}
			}
			else
				// on avance maintenant dans l'analyse des traces
				workingId++;
		}
		// Gérer une éventuelle fusion en attente
		if (mergeOccurs)
		{
			cleanTracesAndAddMerge(traces, sps_up, upStart, lastEndFusion);
			// Voir commentaire du même "if" précédant
			if (upStart < currentStartingPoint)
				currentStartingPoint = upStart;
		}
	}*/
}

// Construit itérativement des fenêtres à partir de workingId d'une taille allant de 1 à maxLength et tente de les fusionner avec sps_up. Si un de ces alignements fournit un meilleur score que celui passé en paramètre alors bestScore et endDown sont mis à jour (bestMerge contient le résultat de la fusion uniquement si un bestScore est obtenu pour une taille de fenêtre up originale ; endDown contient la position dans this->trace de la fin de cette fusion).
// Si bestScore >= 0 => la meilleure fusion a pu être faite pour la taille de fenêtre up originale 
// Si bestScore < 0 => la meilleure fusion a pu être faite avec une taille de fenêtre up plus grande. Dans ce cas le paramètre bestScore contient l'opposé de la valeur du score calculé et bestMerge n'est pas modifié. Ce dernier cas ne peut avoir lieu que si originalUpSize == false.
void Sequence::evaluateMerging(int workingId, int maxLength, Sequence::sp_sequence sps_up, bool originalUpSize, float & bestScore, Sequence::sp_sequence & bestMerge, int & endDown){
	// Calcul de la position jusqu'où on s'autorise à avancer
	int length = 0;
	int endPos = workingId + 1;
	while (endPos < (signed)size() && length < maxLength)
	{
		// Si la trace analysée est un event non fusionable, on stoppe l'analyse
		if (at(endPos)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(at(endPos))->getLabel().c_str(), Event::noConcatEventsArr) > -1)
			break;
		length += at(endPos)->length();
		endPos++;
	}
	// On parcours toutes les traces "down" entre workingId et endPos et on calcule le score d'alignement
	//  1 - Création d'une séquence "down"
	Sequence::sp_sequence sps_down = boost::make_shared<Sequence>(1);
	//  2 - Intégration de la trace suivante jusqu'à ce que la borne de fin soit atteinte
	for (int i = workingId; i < endPos; i++)
	{
		sps_down->addTrace(at(i));
		// tentative de fusion et comparaison du score obtenu
		// cas particulier pour la première trace s'il s'agit d'une séquence, il faut calculer son score de fusion sans l'encapsuler elle même dans une séquence
		int nbAlign;
		int nbOpt;
		Sequence::sp_sequence sps_res;
		if (i == workingId && sps_down->at(0)->isSequence())
			sps_res = mergeEquivalentSequences(sps_up, boost::dynamic_pointer_cast<Sequence>(sps_down->at(0)), false, &nbAlign, &nbOpt);
		else{
			sps_res = mergeEquivalentSequences(sps_up, sps_down, false, &nbAlign, &nbOpt);
		}
		// Vérifier si le résultat de la fusion est meilleur que le précédent
		if ((float)nbAlign/(nbAlign+nbOpt) > bestScore)
		{
			// Si ce meilleur score est obtenu avec une fenêtre "up" d'une taille >= minWindowSize+1 alors cela signifie qu'on a intérêt à ce que le fenêtre "up" gorssisse. Donc on peut arrêter l'analyse à partir de cette trace
			if (!originalUpSize){
				bestScore = -(float)nbAlign/(nbAlign+nbOpt);
				endDown = i + 1;
				return;
			}
			else
			{
				bestScore = (float)nbAlign/(nbAlign+nbOpt);
				bestMerge = sps_res;
				endDown = i + 1;

			}
		}
		// Si on a trouvé un alignement parfait (pas d'option et au moins un alignement), on ne trouveras pas mieux donc on peut stopper la boucle
		if (nbAlign > 0 && nbOpt == 0){
			return;
		}
	}
}

void Sequence::cleanTracesAndAddMerge(std::vector<Trace::sp_trace> &traces, Sequence::sp_sequence merge, int upStart, int endDown)
{
	// Activer l'attribut "delayed" si toutes les traces de la séquence sont "delayed"
	merge->checkDelayed();
	// Suppression de toutes les traces comprises entre l'indice de début et l'indice de fin
	traces.erase(traces.begin() + upStart, traces.begin() + endDown);
	// Ajout de la fusion
	traces.insert(traces.begin() + upStart, merge);
}

bool Sequence::checkFeasibility(int min_length, int ind_start)
{
	int len = 0;
	for (int i = ind_start; len < min_length && i < (signed)size(); i++)
	{
		if (at(i)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(at(i))->getLabel().c_str(), Event::noConcatEventsArr) > -1)
			break;
		len += at(i)->length();
	}
	return len >= min_length;
}

bool Sequence::includeNoConcatEvent()
{
	for (unsigned int i = 0; i < this->size(); i++)
		if (this->at(i)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(this->at(i))->getLabel().c_str(), Event::noConcatEventsArr) > -1)
			return true;
	return false;
}

Sequence::sp_sequence Sequence::mergeAlignedSequences(Sequence::sp_sequence seq1, Sequence::sp_sequence seq2)
{
	// create an empty new sequence based on up sequences and as many iterations defined in down sequence. This sequence will store merging of up and down sequences
	Sequence::sp_sequence sps_merging = boost::make_shared<Sequence>(seq1);
	for (int i = 0; i < (int)seq2->getNum(); i++)
		sps_merging->addOne();
	// Process the two sequences in parallel
	unsigned int i = 0, j = 0;
	while (i < seq1->size() || j < seq2->size())
	{
		// check if the both traces are aligned
		if (i < seq1->size() && j < seq2->size() && !seq1->at(i)->getAligned().expired() && !seq2->at(j)->getAligned().expired())
		{
			// Check if the both traces are Calls
			if (!seq1->at(i)->isSequence() && !seq2->at(j)->isSequence())
			{
				// filter first call with the second
				dynamic_cast<Call *>(seq1->at(i).get())->filterCall(dynamic_cast<const Call *>(seq2->at(j).get()));
				// and add the up Call to the merged sequence
				sps_merging->addTrace(seq1->at(i));
			}
			else
			{
				// merge the both sequences
				sps_merging->addTrace(mergeAlignedSequences(boost::dynamic_pointer_cast<Sequence>(seq1->at(i)), boost::dynamic_pointer_cast<Sequence>(seq2->at(j))));
			}
			i++;
			j++;
		}
		// Check if the first trace is aligned with a hole => move it as optional
		else if (i < seq1->size() && seq1->at(i)->getAligned().expired())
		{
			seq1->at(i)->setOptional(true);
			sps_merging->addTrace(seq1->at(i));
			i++;
		}
		else if (j < seq2->size() && seq2->at(j)->getAligned().expired())
		{
			seq2->at(j)->setOptional(true);
			sps_merging->addTrace(seq2->at(j));
			j++;
		}
	}
	return sps_merging;
}

Sequence::sp_sequence Sequence::mergeEquivalentSequences(Sequence::sp_sequence sps_up, Sequence::sp_sequence sps_down, bool strict, int* nbAlignRes, int* nbOptRes)
{
#ifdef DEBUG_PARSER
	TracesParser::osParser << "###############################################" << std::endl;
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
	for (int i = 0; i < (int)sps_down->getNum(); i++)
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
	while (!sps_up->isEndReached() && spt_up->isEvent())
	{
		events.push_back(spt_up);
		spt_up = sps_up->next();
	}
	// Look for the first trace that is not an event into the down sequence and store inserted events
	Trace::sp_trace spt_down = sps_down->next();
	while (!sps_down->isEndReached() && spt_down->isEvent())
	{
		events.push_back(spt_down);
		spt_down = sps_down->next();
	}
	// while the up and down stack are not empty (means it is reamaining traces to merge)
	while (!upStack.empty() || !downStack.empty())
	{
#ifdef DEBUG_PARSER
		TracesParser::osParser << "===============================================" << std::endl;
#endif
		// Check if current up and down trace are defined
		if (!spt_up && !spt_down)
		{
			// There is a problem with the stacks, we can't pop and we can't get next traces
#ifdef DEBUG_PARSER
			TracesParser::osParser << "Sequence::mergeEquivalentSequences => WARNING no traces to process." << std::endl;
#endif
			// so we cancel merging and return an empty sequence
			Sequence::sp_sequence sps_empty;
			if(nbAlignRes != NULL) *nbAlignRes = 0;
			if(nbOptRes != NULL) *nbOptRes = 0;
			return sps_empty;
		}
		else if ((!spt_up || !spt_down) && strict)
		{
			// Si une des deux traces n'est pas définie, celà signifie que l'on ne peut dépiler la séquence pour passer à la trace suivante car elle est bloqué par le dépilement de l'autre séquence.
			// Exemple : sps_up = [A[BC*]D]; sps_down = [AB[C*D]] => soit la position de l'étoile marquant la position de la tête de lecture dans le processus de fusion. Dans cet exemple l'empilement des séquence est "root|up|down" il faudrait dépiler up pour passer au symbole suivant, hors on ne peut pas car le dernier empilement a eut lieu sur la sequence down. Il faut donc attendre d'atteindre la fin du down pour pouvoir d'épiler le down et enfin le up. Cette situation doit donc introduire des séquences optionnelles pour régler ce problème. Comme nous sommes ici en mode strict, on retourne une séquence vide.
			Sequence::sp_sequence sps_empty;
			if(nbAlignRes != NULL) *nbAlignRes = 0;
			if(nbOptRes != NULL) *nbOptRes = 0;
			return sps_empty;
			// Note : Si cette précaution n'est pas prise cela entraine la construction d'une fusion fausse. En effet si on n'attend pas la fin de la séquence down le résultat de la fusion serait : [A[B[C]D]]. Cette fusion ne permet pas de représenter la séquence ABCBCD pourtant valide avec la séquence up. La solution passe par l'ajout de partie optionnelle : [A[B[C(D)]](D)]
		}
		else
		{
			if (!spt_up)
			{ // nous sommes ici en non strict (cf "if ((!spt_up || !spt_down) && strict)")
				// Si le pointeur de la séquence up n'est pas défini c'est qu'il a été impossible de dépiler une séquence up car l'algo est bloqué par l'empilement d'une séquence down. Il faut terminer cette séquence down pour pouvoir continuer sur la séquence up. Donc on met toutes les traces de la séquence down comme optionnelles
				// construction de la sequence optionnelle
				Sequence::sp_sequence sps_opt = boost::make_shared<Sequence>(1, false, true);
				// On ajoute dans la séquence optionnelle tous ce qui reste dans la séquence down
				sps_opt->addTrace(spt_down);
				nbOpt++;
				while (!sps_down->isEndReached())
				{
					sps_opt->addTrace(sps_down->next());
					nbOpt++;
				}
				// et nous ajoutons cette séquence optionnelle dans la fusion
				// si la séquence optionnelle ne contient qu'un seul élément il n'est pas utile de l'encapsuler dans une séquence. Dans ce cas on l'ajoute directement
				if (sps_opt->size() == 1)
				{
					Trace::sp_trace spt_opt = sps_opt->at(0)->clone();
					spt_opt->setOptional(true);
					sps_merging->addTrace(spt_opt);
				}
				else
					sps_merging->addTrace(sps_opt);
				next_down = true;
			}
			else if (!spt_down)
			{ // nous sommes ici en non strict (cf "if ((!spt_up || !spt_down) && strict)")
				// Si le pointeur de la séquence down n'est pas défini c'est qu'il a été impossible de dépiler une séquence down car l'algo est bloqué par l'empilement d'une séquence up. Il faut terminer cette séquence up pour pouvoir continuer sur la séquence down. Donc on met toutes les traces de la séquence up comme optionnelles
				// construction de la sequence optionnelle
				Sequence::sp_sequence sps_opt = boost::make_shared<Sequence>(1, false, true);
				// On ajoute dans la séquence optionnelle tous ce qui reste dans la séquence up
				sps_opt->addTrace(spt_up);
				nbOpt++;
				while (!sps_up->isEndReached())
				{
					sps_opt->addTrace(sps_up->next());
					nbOpt++;
				}
				// et nous ajoutons cette séquence optionnelle dans la fusion
				// si la séquence optionnelle ne contient qu'un seul élément il n'est pas utile de l'encapsuler dans une séquence. Dans ce cas on l'ajoute directement
				if (sps_opt->size() == 1)
				{
					Trace::sp_trace spt_opt = sps_opt->at(0)->clone();
					spt_opt->setOptional(true);
					sps_merging->addTrace(spt_opt);
				}
				else
					sps_merging->addTrace(sps_opt);
				next_up = true;
			}
			else
			{
#ifdef DEBUG_PARSER
				TracesParser::osParser << "Do we have to align:" << std::endl;
				spt_up->exportAsString(TracesParser::osParser);
				TracesParser::osParser << "WITH" << std::endl;
				spt_down->exportAsString(TracesParser::osParser);
#endif
				// Ici nous sommes sûr que stp_up et stp_down sont définis
				// Création d'une trace pour recevoir les options
				Trace::sp_trace spt_opt;
				// Initialisation de l'indice de départ pour la linéarisation de la séquence Up (Sequence::getPt() pointe sur le prochain élément à lire, d'où le -1 pour récupérer la position de l'élément courant)
				unsigned int startLinearisationUp = sps_up->getPt() - 1;
				// Si la fin de la séquence a été atteinte, Sequence::getPt() pointe sur 0 hors nous souhaitons dans notre cas le dernier élément de la séquence, on corrige donc l'indice de départ
				if (sps_up->isEndReached())
					startLinearisationUp = sps_up->size() - 1;
				// Initialisation de l'indice de départ pour la linéarisation de la séquence Down (Sequence::getPt() pointe sur le prochain élément à lire, d'où le -1 pour récupérer la position de l'élément courant)
				unsigned int startLinearisationDown = sps_down->getPt() - 1;
				// Si la fin de la séquence a été atteinte, Sequence::getPt() pointe sur 0 hors nous souhaitons dans notre cas le dernier élément de la séquence, on corrige donc l'indice de départ
				if (sps_down->isEndReached())
					startLinearisationDown = sps_down->size() - 1;
				// Obtenir le meilleur score d'alignement entre la trace up à partir de la trace courante et la trace down à partir de la trace courante
				std::pair<int, int> pair_scoreUpDown = Sequence::getMaxAlignAndMinOpt(sps_up->getLinearSequence(startLinearisationUp), sps_down->getLinearSequence(startLinearisationDown));
#ifdef DEBUG_PARSER
				TracesParser::osParser << "Result for h1::q1 and h2::q2 => " << pair_scoreUpDown.first << "/" << pair_scoreUpDown.second << "\n"
									   << std::endl;
#endif
				// Obtenir le meilleur score d'alignement entre la trace up à partir de la trace SUIVANT la trace courante et la trace down à partir de la trace courante
				std::pair<int, int> pair_scoreNextUpDown = Sequence::getMaxAlignAndMinOpt(sps_up->getLinearSequence(startLinearisationUp + 1), sps_down->getLinearSequence(startLinearisationDown));
#ifdef DEBUG_PARSER
				TracesParser::osParser << "Result for q1 and h2::q2 => " << pair_scoreNextUpDown.first << "/" << pair_scoreNextUpDown.second << "\n"
									   << std::endl;
#endif
				// Obtenir le meilleur score d'alignement entre la trace up à partir de la trace courante et la trace down à partir de la trace SUIVANT la trace courante
				std::pair<int, int> pair_scoreUpNextDown = Sequence::getMaxAlignAndMinOpt(sps_up->getLinearSequence(startLinearisationUp), sps_down->getLinearSequence(startLinearisationDown + 1));
#ifdef DEBUG_PARSER
				TracesParser::osParser << "Result for h1::q1 and q2 => " << pair_scoreUpNextDown.first << "/" << pair_scoreUpNextDown.second << "\n"
									   << std::endl;
#endif
				// Interprétation des scores
				int scoreUpDown = 0;
				int scoreNextUpDown = 0;
				int scoreUpNextDown = 0;
				// scoreUpDown meilleur que scoreNextUpDown
				if (pair_scoreUpDown.first > pair_scoreNextUpDown.first || (pair_scoreUpDown.first == pair_scoreNextUpDown.first && pair_scoreUpDown.second < pair_scoreNextUpDown.second))
					scoreUpDown += 2;
				// scoreUpDown meilleur que scoreUpNextDown
				if (pair_scoreUpDown.first > pair_scoreUpNextDown.first || (pair_scoreUpDown.first == pair_scoreUpNextDown.first && pair_scoreUpDown.second < pair_scoreUpNextDown.second))
					scoreUpDown += 2;
				// scoreNextUpDown meilleur que scoreUpDown
				if (pair_scoreNextUpDown.first > pair_scoreUpDown.first || (pair_scoreNextUpDown.first == pair_scoreUpDown.first && pair_scoreNextUpDown.second < pair_scoreUpDown.second))
					scoreNextUpDown += 2;
				// scoreNextUpDown meilleur que scoreUpNextDown
				if (pair_scoreNextUpDown.first > pair_scoreUpNextDown.first || (pair_scoreNextUpDown.first == pair_scoreUpNextDown.first && pair_scoreNextUpDown.second < pair_scoreUpNextDown.second))
					scoreNextUpDown += 2;
				// scoreUpNextDown meilleur que scoreUpDown
				if (pair_scoreUpNextDown.first > pair_scoreUpDown.first || (pair_scoreUpNextDown.first == pair_scoreUpDown.first && pair_scoreUpNextDown.second < pair_scoreUpDown.second))
					scoreUpNextDown += 2;
				// scoreUpNextDown meilleur que scoreNextUpDown
				if (pair_scoreUpNextDown.first > pair_scoreNextUpDown.first || (pair_scoreUpNextDown.first == pair_scoreNextUpDown.first && pair_scoreUpNextDown.second < pair_scoreNextUpDown.second))
					scoreUpNextDown += 2;
				// Egalité entre scoreUpDown et scoreNextUpDown
				if (pair_scoreUpDown.first == pair_scoreNextUpDown.first && pair_scoreUpDown.second == pair_scoreNextUpDown.second)
				{
					scoreUpDown++;
					scoreNextUpDown++;
				}
				// Egalité entre scoreUpDown et scoreUpNextDown
				if (pair_scoreUpDown.first == pair_scoreUpNextDown.first && pair_scoreUpDown.second == pair_scoreUpNextDown.second)
				{
					scoreUpDown++;
					scoreUpNextDown++;
				}
				// Egalité entre scoreNextUpDown et scoreUpNextDown
				if (pair_scoreUpNextDown.first == pair_scoreNextUpDown.first && pair_scoreUpNextDown.second == pair_scoreNextUpDown.second)
				{
					scoreUpNextDown++;
					scoreNextUpDown++;
				}

				// Check if the current up trace is a sequence
				if (spt_up->isSequence())
				{
					// We update the working up sequence (the previous one has been stored in up stack)
					sps_up = boost::dynamic_pointer_cast<Sequence>(spt_up);
					sps_up->reset();
				}
				// Check if the current down trace is a sequence
				if (spt_down->isSequence())
				{
					// We update the working down sequence (the previous one has been stored in down stack)
					sps_down = boost::dynamic_pointer_cast<Sequence>(spt_down);
					sps_down->reset();
				}
				// compute length of up and down traces
				len_up = spt_up->length();
				len_down = spt_down->length();

#ifdef DEBUG_PARSER
				TracesParser::osParser << "------------------------------------------" << std::endl;
#endif
				// Si le score obtenu avec les traces courantes est meilleur que les scores obtenu sans une des deux traces courante => On doit procéder à la fusion
				if (scoreUpDown > scoreNextUpDown && scoreUpDown > scoreUpNextDown)
				{
					// If both current up and down traces are not sequences (this means they are Calls because Events has been cashed previously) => we merge the two Calls
					if (!spt_up->isSequence() && !spt_down->isSequence())
					{
						// if some events have been found, add them to the merged sequence
						if (!events.empty())
						{
							for (unsigned int i = 0; i < events.size(); i++)
								sps_merging->addTrace(events.at(i));
							// clear events list
							events.clear();
						}
						// Check if Calls are equals
						if (spt_up->operator==(spt_down.get()))
						{
#ifdef DEBUG_PARSER
							TracesParser::osParser << "We choose to merge the two Calls" << std::endl;
#endif
							nbAlign++; // On compte un alignement
							Trace::sp_trace spt_opt = spt_up->clone();
							// filter up call with the down Call
							dynamic_cast<Call *>(spt_opt.get())->filterCall(dynamic_cast<const Call *>(spt_down.get()));
							// prise en compte des éventuelles traces optionnelles
							spt_opt->setOptional(spt_up->isOptional() || spt_down->isOptional());
							// and add the up Call to the merged sequence
							sps_merging->addTrace(spt_opt);
						}
#ifdef DEBUG_PARSER
						else
						{
							TracesParser::osParser << "ERROR (Sequence::mergeEquivalentSequences): Ask to merge two different Calls!!!" << std::endl;
							spt_up->exportAsString(TracesParser::osParser);
							TracesParser::osParser << "WITH" << std::endl;
							spt_down->exportAsString(TracesParser::osParser);
						}
#endif
						// ask to get next traces into up and down sequence
						next_up = true;
						next_down = true;
					}
					else
					{
						// On doit fusionner deux éléments qui ne sont pas des Call => empilement d'une séquence de fusion (voir fin de ce cas)
						// Si on doit aligner un Call du up avec une séquence du down OU une sequence up est plus courte qu'une séquence down
						if ((!spt_up->isSequence() && spt_down->isSequence()) || len_up < len_down)
						{
#ifdef DEBUG_PARSER
							TracesParser::osParser << "We choose to inter into down Sequence" << std::endl;
#endif
							// On construit une nouvelle séquence vide basé sur la séquence "down" de travail (on peut écraser sps_merging car elle a déjà été sauvegardée dans la pile mergedStack)
							sps_merging = boost::make_shared<Sequence>(sps_down);
							// if down is a sequence and not up trace (means it's a Call)
							if (!spt_up->isSequence() && spt_down->isSequence())
								// this sequence has been done one time only once (indead a Call means it's possible to run this sequence only once)
								sps_merging->addIteration(1, 1);
							// Add on down stack the down working sequence in order to enter possible sub-sequences
							downStack.push(sps_down);
							orderStack.push('d'); // means we stack down sequence
							// ask to get the next trace into down sequence
							next_down = true;
						}
						// Si on doit aligner un Call du down avec une séquence du up OU une sequence down est plus courte qu'une séquence up
						else if ((spt_up->isSequence() && !spt_down->isSequence()) || len_up > len_down)
						{
#ifdef DEBUG_PARSER
							TracesParser::osParser << "We choose to inter into up Sequence" << std::endl;
#endif
							// On construit une nouvelle séquence vide basé sur la séquence "up" de travail (on peut écraser sps_merging car elle a déjà été sauvegardée dans la pile mergedStack)
							sps_merging = boost::make_shared<Sequence>(sps_up);
							// if up trace is a sequence and not down trace (means it's a Call)
							if (spt_up->isSequence() && !spt_down->isSequence())
								// this sequence has been done one time only once (indead a Call means it's possible to run this sequence only once)
								sps_merging->addIteration(1, 1);
							// Add on up stack the up working sequence in order to enter possible sub-sequences
							upStack.push(sps_up);
							orderStack.push('u'); // means we stack up sequence
							// ask to get the next trace into up sequence
							next_up = true;
						}
						// Si on doit aligner deux séquences de même taille
						else
						{
#ifdef DEBUG_PARSER
							TracesParser::osParser << "We choose to merge both sequences" << std::endl;
#endif
							// On construit une nouvelle séquence vide basé sur les séquences "up" et "down" de travail (on peut écraser sps_merging car elle a déjà été sauvegardée dans la pile mergedStack)
							sps_merging = boost::make_shared<Sequence>(sps_up, sps_down);
							// Add on stacks up and down working sequences in order to enter possible sub-sequences
							upStack.push(sps_up);
							downStack.push(sps_down);
							orderStack.push('x'); // means we stack the both sequences
							// ask to get next traces into up and down sequences
							next_up = true;
							next_down = true;
						}
						// Stack the new merging sequence
						mergedStack.push(sps_merging);
					}
				}
				else
				{
					// Ici on doit insérer une option
					// si on est en mode strict, on coupe la fusion
					if (strict)
					{
#ifdef DEBUG_PARSER
						TracesParser::osParser << "We abort merging due to strict mode and requirement to add optional trace" << std::endl;
#endif
						Sequence::sp_sequence sps_empty;
						if(nbAlignRes != NULL) *nbAlignRes = 0;
						if(nbOptRes != NULL) *nbOptRes = 0;
						return sps_empty;
					}
					// Si le score obtenu sans la trace courante up est supérieur au score sans la trace courante down => mettre la trace up en option
					else if (scoreNextUpDown > scoreUpNextDown || (scoreNextUpDown == scoreUpNextDown && len_up >= len_down))
					{
#ifdef DEBUG_PARSER
						TracesParser::osParser << "We choose to set optional the up trace" << std::endl;
#endif
						// On clone la trace "up"
						spt_opt = spt_up->clone();
						// on définit la trace comme optionnelle
						spt_opt->setOptional(true);
						nbOpt += spt_opt->length(); // On compte autant d'options que la longueur de la trace
						// and we add this optionnal trace into the merged sequence
						sps_merging->addTrace(spt_opt);
						// Si la trace "up" est une séquence, il faut se repositionner sur son parent
						if (spt_up->isSequence())
						{
							// pas besoin de recaster spt_up dans sps_up, ça a déjà été fait plus haut
							if (!sps_up->getParent().expired())
								sps_up = boost::dynamic_pointer_cast<Sequence>(sps_up->getParent().lock());
						}
						next_up = true;
					}
					// Si le score obtenu sans la trace courante down est supérieur au score sans la trace courante up => mettre la trace down en option
					else if (scoreUpNextDown > scoreNextUpDown || (scoreUpNextDown == scoreNextUpDown && len_down > len_up))
					{
#ifdef DEBUG_PARSER
						TracesParser::osParser << "We choose to set optional the down trace" << std::endl;
#endif
						// On clone la trace "down"
						spt_opt = spt_down->clone();
						// on définit la trace comme optionnelle
						spt_opt->setOptional(true);
						nbOpt += spt_opt->length(); // On compte autant d'options que la longueur de la trace
						// and we add this optionnal trace into the merged sequence
						sps_merging->addTrace(spt_opt);
						// Si la trace "down" est une séquence, il faut se repositionner sur son parent
						if (spt_down->isSequence())
						{
							// pas besoin de recaster spt_up dans sps_up, ça a déjà été fait plus haut
							if (!sps_down->getParent().expired())
								sps_down = boost::dynamic_pointer_cast<Sequence>(sps_down->getParent().lock());
						}
						next_down = true;
					}
				}
#ifdef DEBUG_PARSER
				TracesParser::osParser << "------------------------------------------" << std::endl;
#endif
			}
		}
		bool endPop = false;
		while (!endPop)
		{
			// Vérifier si on doit dépiler le up
			if (next_up && !upStack.empty() && sps_up->isEndReached())
			{
				// Vérifier si on peut dépiler cette pile => On pourra le faire si le dernier empilement a eu lieu sur cette pile
				char lastPush = orderStack.top();
				// 'u' pour "up" et 'x' pour "les deux en même temps"
				if (lastPush == 'u' || lastPush == 'x')
				{
					orderStack.pop(); // on dépile
					// si c'était un 'x' on rempile un 'd' car il faudra attendre la fin du down avant de pourvoir dépiler à nouveau un up
					if (lastPush == 'x')
						orderStack.push('d');
					// We pop the up stack
					upStack.pop();
					// If up stack is not empty we get the top sequence (i.e. the parent of the poped sequence)
					if (!upStack.empty())
					{
						sps_up = upStack.top();
						// Count that a new pop occurs
						if (lastPush != 'x')
							pop++;
					}
					continue; // On coupe cette itération pour sauter le "endPop = true" de fin de boucle
				}
			}
			// Vérifier si l'on doit dépiler le down
			if (next_down && !downStack.empty() && sps_down->isEndReached())
			{
				// Vérifier si on peut dépiler cette pile => On pourra le faire si le dernier empilement a eu lieu sur cette pile
				char lastPush = orderStack.top();
				// 'd' pour "down" et 'x' pour "les deux en même temps"
				if (lastPush == 'd' || lastPush == 'x')
				{
					orderStack.pop(); // on dépile
					// si c'était un 'x' on rempile un 'u' car il faudra attendre la fin du up avant de pourvoir dépiler à nouveau un down
					if (lastPush == 'x')
						orderStack.push('u');
					// We pop the down stack
					downStack.pop();
					// If down stack is not empty we get the top sequence (i.e. the parent of the poped sequence)
					if (!downStack.empty())
					{
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
		while (pop > 0)
		{
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
		if (next_up && sps_up->isEndReached())
			spt_up.reset();
		else
		{
			// If we have processed the up sequence and the up stack is not empty
			if (next_up && !upStack.empty())
			{
				// Look for the next trace that is not an event into the up sequence and store inserted events
				spt_up = sps_up->next();
				while (!sps_up->isEndReached() && spt_up->isEvent())
				{
					events.push_back(spt_up);
					spt_up = sps_up->next();
				}
				// reset up flag
				next_up = false;
			}
		}
		// si on doit récupérer la trace suivante dans la séquence down mais qu'on est bloqué sur la fin d'une séquence c'est qu'on n'a pas pu la dépiler. On reset donc le pointeur de trace.
		if (next_down && sps_down->isEndReached())
			spt_down.reset();
		else
		{
			// If we have processed the down sequence and the down stack is not empty
			if (next_down && !downStack.empty())
			{
				// Look for the next trace that is not an event into the down sequence and store inserted events
				spt_down = sps_down->next();
				while (!sps_down->isEndReached() && spt_down->isEvent())
				{
					events.push_back(spt_down);
					spt_down = sps_down->next();
				}
				// reset up flag
				next_down = false;
			}
		}

		/*#ifdef DEBUG_PARSER
			TracesParser::osParser << "\nAffichage de UP STACK" << std::endl;
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
			}
		#endif*/
	}
	/*float ratio = nbOpt != 0 ? (float)nbAlign / nbOpt : nbAlign;
	#ifdef DEBUG_PARSER
		TracesParser::osParser << "Merging proposition:" << std::endl;
		sps_merging->exportAsString(TracesParser::osParser);
		TracesParser::osParser << "Alignment ratio: " << nbAlign << "/" << nbOpt << " = " << ratio << std::endl;
	#endif*/
	if(nbAlignRes != NULL) *nbAlignRes = nbAlign;
	if(nbOptRes != NULL) *nbOptRes = nbOpt;
	return sps_merging;
}

bool Sequence::findAndProcessOptionalTokens(int startingPoint)
{
	bool improvement = false;
	// On parcours toute la trace
	for (int currentPos = startingPoint; currentPos < (int)size(); currentPos++)
	{
		// Si la trace courante est un event non fusionable, on stoppe l'annalyse
		if (at(currentPos)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(at(currentPos))->getLabel().c_str(), Event::noConcatEventsArr) > -1)
			break;

		// Si la trace courante est une séquence
		if (at(currentPos)->isSequence())
		{
			Sequence::sp_sequence subseq = boost::dynamic_pointer_cast<Sequence>(at(currentPos));
			// On traite les parties optionnelles dans cette sous-sequence (appel récursif)
			subseq->findAndProcessOptionalTokens(0);
			// On réduit au maximum le nombre de sous-séquences
			subseq->reduceSubsequences();

			// On récupère le contexte autour de cette séquence
			int ctxStartPos;
			int ctxEndPos;
			std::pair<int, Sequence::sp_sequence> context = extractContext(currentPos, &ctxStartPos, &ctxEndPos);
			// On tente d'oppérer une rotation amont en prenant en compte le contexte
			std::pair<int, Sequence::sp_sequence> contexteRotatedUp(-1, boost::dynamic_pointer_cast<Sequence>(context.second->clone()));
			contexteRotatedUp.first = contexteRotatedUp.second->tryToProcessUpRotation(context.first);
			// On tente d'oppérer une rotation aval en prenant en compte le contexte
			std::pair<int, Sequence::sp_sequence> contexteRotatedDown(-1, boost::dynamic_pointer_cast<Sequence>(context.second->clone()));
			contexteRotatedDown.first = contexteRotatedDown.second->tryToProcessDownRotation(context.first);
			// Application des traitements pour révéler les parties optionnelles
			bool localImprovement = context.second->processOptionalTokens(context.first);
			if (contexteRotatedUp.first != context.first)
				localImprovement = contexteRotatedUp.second->processOptionalTokens(contexteRotatedUp.first) || localImprovement;
			if (contexteRotatedDown.first != context.first)
				localImprovement = contexteRotatedDown.second->processOptionalTokens(contexteRotatedDown.first) || localImprovement;

			// Vérifier si une amélioration a été calculée
			if (localImprovement)
			{
				// Identification de la meilleure opérations
				std::pair<int, Sequence::sp_sequence> bestContext = context;
				if (contexteRotatedUp.second->length() < bestContext.second->length())
					bestContext = contexteRotatedUp;
				if (contexteRotatedDown.second->length() < bestContext.second->length())
					bestContext = contexteRotatedDown;
				// Intégration du meilleur contexte dans la trace
				// Récupération des tailles du nouveau contexte
				int newContextSize = bestContext.second->size();
				// S'il y a eu des améliorations c'est que le nouveau contexte est plus court que l'ancien, on évite donc de supprimer puis de réinsérer des éléments mais on les écrasent plutot
				// Ecrasement de l'ancien contexte par le nouveau
				for (int i = 0; i < newContextSize; i++)
					traces[ctxStartPos + i] = bestContext.second->at(i);
				// On termine en supprimant les traces en trop de l'ancien contexte
				traces.erase(traces.begin() + ctxStartPos + newContextSize, traces.begin() + ctxEndPos);
				// recaler la position courante au cas les changements auraient créés des décallages dans la trace
				currentPos += bestContext.first - context.first;
			}
			improvement = improvement || localImprovement;
		}
	}
	return improvement;
}

void Sequence::reduceSubsequences()
{
	// Récupération de la première séquence et vérification qu'il n'y en a pas plus d'une
	Sequence::sp_sequence firstSeq;
	int seqPos = -1;
	for (int i = 0 ; i < (signed)size() ; i++)
	{
		if (at(i)->isSequence() && !at(i)->isOptional())
		{
			if (!firstSeq)
			{
				firstSeq = boost::dynamic_pointer_cast<Sequence>(at(i));
				seqPos = i;
			}
			else
				return;
		}
	}
	// S'il n'y a pas de sous-séquence, on a rien à faire
	if (seqPos == -1)
		return;
	// Si on est là c'est que la séquence contient exactement une seule sous-séquence qui n'est pas optionnelle. Donc on va passer toutes les autres traces de la séquence mère en optionel
	// La sous-séquence non optionnelle peut donc être encadrée par un ensemble de trace en amont et un ensemble de trace en aval
	Sequence::sp_sequence prevSeq = boost::make_shared<Sequence>(1, false, true);
	Sequence::sp_sequence nextSeq = boost::make_shared<Sequence>(1, false, true);
	for (int j = 0 ; j < (signed)size() ; j++)
		if (j < seqPos)
			prevSeq->addTrace(at(j));
		else if (j > seqPos)
			nextSeq->addTrace(at(j));
	// Intégration des sous séquences optionnelles en amont
	if ((signed)prevSeq->size() > 0)
	{
		traces.erase(traces.begin(), traces.begin()+seqPos);
		// gestion des cas où la partie en amont ne contient qu'une seule trace. Dans ce cas là pas utile de l'encapsuler dans une séquence
		if ((signed)prevSeq->size() == 1)
			addTrace (prevSeq->at(0), 0);
		else
			addTrace (prevSeq, 0);
		// Décallage de la position de la séquence référence, elle se trouve maintenant à l'indice 1 (juste après la trace optionnelle en amont qui elle se trouve en 0)
		seqPos = 1;
	}
	// Intégration des sous séquences optionnelles en amont
	if ((signed)nextSeq->size() > 0)
	{
		traces.erase(traces.begin() + seqPos + 1, traces.end());
		// gestion des cas où la partie en aval ne contient qu'une seule trace. Dans ce cas là pas utile de l'encapsuler dans une séquence
		if ((signed)nextSeq->size() == 1)
			addTrace (nextSeq->at(0), seqPos + 1);
		else
			addTrace (nextSeq, seqPos + 1);
	}

	// Maintenant on peut transférer tout le contenu de la sous-séquence dans la séquence mère
	for (int j = firstSeq->size()-1 ; j >= 0 ; j--)
		this->addTrace(firstSeq->at(j), seqPos+1);
	// On termine en supprimant la sous-séquence
	traces.erase(traces.begin() + seqPos);
}

std::pair<int, Sequence::sp_sequence> Sequence::extractContext(int currentPos, int *startPos, int *endPos)
{
	// Vérifier que la trace à l'indice currentPos est bien une séquence
	if (at(currentPos)->isSequence())
	{
		// On récupère cette séquence
		Sequence::sp_sequence seq = boost::dynamic_pointer_cast<Sequence>(at(currentPos));
		// Calcul de la position jusqu'où on s'autorise à remonter
		int maxLength = seq->length() + MAX_OPT_WINDOW;
		int length = 0;
		if (currentPos > 0)
			*startPos = currentPos - 1;
		else
			*startPos = 0;
		while (*startPos > 0 && length < maxLength)
		{
			// Si la trace analysée est un event non fusionable, on stoppe l'analyse
			if (at(*startPos)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(at(*startPos))->getLabel().c_str(), Event::noConcatEventsArr) > -1)
			{
				(*startPos)++; // pour exclure l'évènement sur lequel on vient de tomber
				break;
			}
			length += at(*startPos)->length();
			(*startPos)--;
		}

		// Calcul de la position jusqu'où on s'autorise à avancer
		length = 0;
		*endPos = currentPos + 1;
		while ((unsigned)*endPos < size() && length < maxLength)
		{
			// Si la trace analysée est un event non fusionable, on stoppe l'analyse
			if (at(*endPos)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(at(*endPos))->getLabel().c_str(), Event::noConcatEventsArr) > -1)
				break;
			length += at(*endPos)->length();
			(*endPos)++;
		}

		return std::pair<int, Sequence::sp_sequence>(currentPos - *startPos, getSubSequence(*startPos, *endPos));
	}
	return std::pair<int, Sequence::sp_sequence>(currentPos, shared_from_this());
}

int Sequence::tryToProcessUpRotation(int seqPos)
{
	// Si la trace courante est une séquence
	if (at(seqPos)->isSequence())
	{
		// On garde une ref de cette sequence
		Sequence::sp_sequence seq = boost::dynamic_pointer_cast<Sequence>(at(seqPos));
		// On va maintenant chercher si la séquence est précédée par la fin de la séquence.
		// on va compter le nombre de traces précédant la séquence identiques à celles de la queue de la séquence
		int nbTraceFoundBefore = 0;
		int tailLength = 0;
		while (nbTraceFoundBefore >= 0 && seqPos - nbTraceFoundBefore - 1 >= 0 && seq->at(seq->size() - nbTraceFoundBefore - 1)->operator==(at(seqPos - nbTraceFoundBefore - 1).get()))
		{
			// Tant qu'on n'a pas atteint le début de la séquence, le début des traces et que les deux traces sont égales => on continue à compter
			tailLength += seq->at(seq->size() - nbTraceFoundBefore - 1)->length();
			nbTraceFoundBefore++;
		}
		// Vérifier qu'au moins la première trace ait été trouvée
		if (nbTraceFoundBefore != 0)
		{
			// On peut donc opérer à la rotation
			// Switch des traces identifiées en amont de la séquence courante
			// Ex : D[ABCD] => [ABCD]D
			for (int i = 0; i < nbTraceFoundBefore; i++)
			{
				traces[seqPos - i] = at(seqPos - i - 1);
				if (at(seqPos - i)->isCall())
					dynamic_cast<Call *>(at(seqPos - i).get())->filterCall(dynamic_cast<const Call *>(seq->at(seq->size() - i - 1).get()));
			}
			traces[seqPos - nbTraceFoundBefore] = seq;
			// Switch des traces à l'intérieur de la séquence courante
			// Ex : [ABCD]D => [DABC]D
			for (int i = 0; i < nbTraceFoundBefore; i++)
				seq->getTraces().insert(seq->getTraces().begin(), seq->at(seq->size() - i - 1));
			seq->getTraces().erase(seq->getTraces().end() - nbTraceFoundBefore, seq->getTraces().end());
			// on est donc passé de D[ABCD] à [DABC]D
			// Terminer en repositionnant seqPos à l'emplacement de la séquence courante vue qu'elle a été déplacée
			seqPos -= nbTraceFoundBefore;
		}
	}
	return seqPos;
}

int Sequence::tryToProcessDownRotation(int seqPos)
{
	// Si la trace courante est une séquence
	if (at(seqPos)->isSequence())
	{
		// On garde une ref de cette sequence
		Sequence::sp_sequence seq = boost::dynamic_pointer_cast<Sequence>(at(seqPos));
		// On va maintenant chercher si la suite de la trace contient le début de cette séquence.
		// on va compter le nombre de traces suivant la séquence identiques à celles de la tête de la séquence
		int nbTraceFoundAfterwards = 0;
		int headLength = 0;
		while (nbTraceFoundAfterwards < (int)seq->size() && seqPos + nbTraceFoundAfterwards + 1 < (int)size() && seq->at(nbTraceFoundAfterwards)->operator==(at(seqPos + nbTraceFoundAfterwards + 1).get()))
		{
			// Tant qu'on n'a pas atteint la fin de la séquence, la fin des traces et que les deux traces sont égales => on continue à compter
			headLength += seq->at(nbTraceFoundAfterwards)->length();
			nbTraceFoundAfterwards++;
		}
		// Vérifier qu'au moins la première trace ait été trouvée
		if (nbTraceFoundAfterwards != 0)
		{
			// On peut donc opérer à la rotation
			// Switch des traces identifiées en aval avec la séquence courante
			// Ex : [ABCDEF]AB => AB[ABCDEF]
			for (int i = 0; i < nbTraceFoundAfterwards; i++)
			{
				traces[seqPos + i] = at(seqPos + i + 1);
				if (at(seqPos + i)->isCall())
					dynamic_cast<Call *>(at(seqPos + i).get())->filterCall(dynamic_cast<const Call *>(seq->at(i).get()));
			}
			traces[seqPos + nbTraceFoundAfterwards] = seq;
			// Switch des traces à l'intérieur de la séquence courante
			// Ex : AB[ABCDEF] => AB[CDEFAB]
			for (int i = 0; i < nbTraceFoundAfterwards; i++)
				seq->addTrace(seq->at(i));
			seq->getTraces().erase(seq->getTraces().begin(), seq->getTraces().begin() + nbTraceFoundAfterwards);
			// on est donc passé de [ABCDEF]AB à AB[CDEFAB]
			// Terminer en repositionnant seqPos à l'emplacement de la séquence courante vue qu'elle a été déplacée
			seqPos += nbTraceFoundAfterwards;
		}
	}
	return seqPos;
}

bool Sequence::processOptionalTokens(int seqPos)
{
	bool improvement = false;
	bool unaggregateFlag = true;
	bool inclusiveFlag = true;
	// On traite les traces optionnelles tant que les traitements améliorent la longueur de la trace
	do
	{
		int prevLength = 0;
		// on tente d'aggréger les traces intercallées si l'étape sur les sequences inclusives a modifié la trace
		if (inclusiveFlag)
		{
			do
			{
				prevLength = length();
				seqPos = processUnaggregateTracesDueToInsertedTokens(seqPos); // on écrase seqPos pour qu'il soit mis à jour pour le prochain tour de boucle
				if ((unsigned)prevLength > length())
				{
					unaggregateFlag = true;
					improvement = true;
				}
			} while ((unsigned)prevLength > length());
		}
		inclusiveFlag = false;
		// on tente de traiter les sequences inclusives si l'étape sur les les traces intercallées a modifié la trace
		if (unaggregateFlag)
		{
			do
			{
				prevLength = length();
				seqPos = processInclusiveSequences(seqPos); // on écrase seqPos pour qu'il soit mis à jour pour le prochain tour de boucle
				if ((unsigned)prevLength > length())
				{
					inclusiveFlag = true;
					improvement = true;
				}
			} while ((unsigned)prevLength > length());
		}
		unaggregateFlag = false;
	} while (inclusiveFlag);
	return improvement;
}

int Sequence::processUnaggregateTracesDueToInsertedTokens(int seqPos)
{
	// Si la trace référence est une séquence
	if (at(seqPos)->isSequence())
	{
		// On garde une ref de cette sequence
		Sequence::sp_sequence seq = boost::dynamic_pointer_cast<Sequence>(at(seqPos));

		// =================================================
		// Etape 1 on regarde si les traces en amont peuvent être intégrées moyennant l'ajout d'options
		// =================================================
		// Calcul du meilleur score d'alignement entre la séquence référence et les traces précédant la séquence référence. On cherche le meilleur alignement à savoir celui qui permet d'aligner le plus de traces de la séquence avec le moins de trous possibles (maximiser le nombre d'alignement et minimiser la lonqueur des traces)
		// On va donc chercher le meilleur alignement en augmentant progressivement la fenêtre d'alignement en amont de la séquence courrante
		// On commence en positionnant le curseur d'analyse en amont de la séquence courrante
		int reverseCounterPos = seqPos - 1;
		// et on ne compte pas les traces optionnelles
		for (std::vector<Trace::sp_trace>::iterator it = seq->getTraces().begin(); it != seq->getTraces().end(); it++)
			if ((*it)->isOptional())
				reverseCounterPos++;
		if (reverseCounterPos < 0)
			reverseCounterPos = 0;
		float bestScore = 0;
		int bestPos = reverseCounterPos;
		// On va augmenter la fenêtre jusqu'à atteindre la position min
		while (reverseCounterPos >= 0)
		{
			// On construit les traces en amont de la séquence courrante
			Sequence::sp_sequence upPart = getSubSequence(reverseCounterPos, seqPos);
			// et on détermine son score
			std::pair<double, double> localScore = computeBestCorrectedScore(seq->getTraces(), upPart->getTraces());
			// Evaluation du score
			if (localScore.first / localScore.second > bestScore)
			{
				bestScore = localScore.first / localScore.second;
				bestPos = reverseCounterPos;
			}
			reverseCounterPos--;
		}

		// Si le meilleur score calculé est supérieur au seuil => procéder à la fusion et ajouter les traces optionnelles
		if (bestScore >= MIN_SCORE)
		{
			// Recalculer le meilleur alignement
			Sequence::sp_sequence upPart = getSubSequence(bestPos, seqPos);
			TracesAnalyser::findBestAlignment(seq->getTraces(), upPart->getTraces(), true);
			// Et procéder à la fusion
			Sequence::sp_sequence sps_merged = mergeAlignedSequences(seq, upPart);
			// Intégrer la fusion dans la trace
			traces[bestPos] = sps_merged;
			// Supprimer les éléments fusionnés
			traces.erase(traces.begin() + bestPos + 1, traces.begin() + seqPos + 1);
			// Terminer en repositionnant seqPos à l'emplacement de la séquence référence vue qu'elle a été remontée
			seqPos = bestPos;
			// Enregistrement de la séquence fusionnée comme la nouvelle séquence de travail en prévision de l'étape 2 (voir ci-dessous)
			seq = sps_merged;
		}

		// =================================================
		// Etape 2 on regarde maintenant en aval si les traces peuvent être intégrées moyennant l'ajout d'options
		// =================================================
		// Calcul du meilleur score d'alignement entre la séquence référence et les traces suivant la séquence référence. On cherche le meilleur alignement à savoir celui qui permet d'aligner le plus de traces de la séquence avec le moins de trous possibles (maximiser le nombre d'alignement et minimiser la lonqueur des traces)
		// On va donc chercher le meilleur alignement en augmentant progressivement la fenêtre d'alignement en aval de la séquence courrante
		// On commence en positionnant le curseur juste après la séquence courrante
		int counterPos = seqPos + 1;
		bestScore = 0;
		bestPos = counterPos;
		// On va augmenter la fenêtre jusqu'à atteindre la position max
		while (counterPos < (int)size())
		{
			// On construit les traces en aval de la séquence courrante
			Sequence::sp_sequence downPart = getSubSequence(seqPos + 1, counterPos);
			// et on détermine son score
			std::pair<double, double> localScore = computeBestCorrectedScore(seq->getTraces(), downPart->getTraces());
			// Evaluation du score
			if (localScore.first / localScore.second > bestScore)
			{
				bestScore = localScore.first / localScore.second;
				bestPos = counterPos;
			}
			counterPos++;
		}
		// Si le meilleur score calculé est supérieur au seuil => procéder à la fusion et ajouter les traces optionnelles
		if (bestScore >= MIN_SCORE)
		{
			// Recalculer le meilleur alignement
			Sequence::sp_sequence downPart = getSubSequence(seqPos + 1, bestPos);
			TracesAnalyser::findBestAlignment(seq->getTraces(), downPart->getTraces(), true);
			// Et procéder à la fusion
			Sequence::sp_sequence sps_merged = mergeAlignedSequences(seq, downPart);
			// Intégrer la fusion dans la trace
			traces[seqPos] = sps_merged;
			// Supprimer les éléments fusionnés
			traces.erase(traces.begin() + seqPos + 1, traces.begin() + bestPos);
			// Ici on n'a pas à repositionner seqPos comme dans l'étape 1 car la séquence référence n'a pas bougé de place
		}
	}
	return seqPos;
}

int Sequence::processInclusiveSequences(int seqPos)
{
	// Si la trace courante est une séquence
	if (at(seqPos)->isSequence())
	{
		// On garde une ref de cette sequence
		Sequence::sp_sequence seq = boost::dynamic_pointer_cast<Sequence>(at(seqPos));

		// compteur permettant d'aller chercher en aval les séquences à analyser
		int afterwardsPos = 1;
		// On calcule tous les scores d'alignement entre la séquences courante et les séquences suivantes dans la limite de la fenêtre autorisée
		std::pair<double, double> bestScore(0, 1);
		int bestPos = seqPos + afterwardsPos;
		do
		{
			int i = seqPos + afterwardsPos;
			if (i < (signed)size() && at(i)->isSequence())
			{
				// On garde une ref de cette sequence
				Sequence::sp_sequence seq_candidate = boost::dynamic_pointer_cast<Sequence>(at(i));
				// et on détermine son score
				std::pair<double, double> localScore = computeBestCorrectedScore(seq->getTraces(), seq_candidate->getTraces());
				// Evaluation du score
				if (localScore.first / localScore.second > bestScore.first / bestScore.second)
				{
					bestScore = localScore;
					bestPos = i;
				}
			}
			afterwardsPos++;
		} while (seqPos + afterwardsPos < (int)size());

		// Si le meilleur score calculé est supérieur au seuil => intégrer au calcul du score les traces intercallées
		if (bestScore.first / bestScore.second >= MIN_SCORE)
		{
			// intégrer dans le calcul du score la longueur des traces intercallées
			Sequence::sp_sequence seq_inserted = getSubSequence(seqPos + 1, bestPos);
			bestScore.second += seq_inserted->length();
			// Si le score est toujours meilleur que le seuil minimal => fusionner les deux séquences et les traces intercallées
			if (bestScore.first / bestScore.second >= MIN_SCORE)
			{
				// Recalculer le meilleur alignement
				TracesAnalyser::findBestAlignment(seq->getTraces(), boost::dynamic_pointer_cast<Sequence>(at(bestPos))->getTraces(), true);
				// Effectuer la fusion entre la séquence courrante et la meilleur séquence suivante trouvée
				Sequence::sp_sequence sps_merged = mergeAlignedSequences(seq, boost::dynamic_pointer_cast<Sequence>(at(bestPos)));
				// Intégrer la fusion dans la trace
				traces[seqPos] = sps_merged;
				// Ajout des traces intercallées s'il y en avait
				if (bestPos - seqPos > 1)
				{
					Trace::sp_trace tr_inserted = seq_inserted;
					// gestion du cas où il n'y a qu'une seule trace intercallée
					if (seq_inserted->isSequence() && seq_inserted->length() == 1)
						tr_inserted = seq_inserted->at(0);
					// Définir ces traces intercallées comme des options et les ajouter à la fusion
					tr_inserted->setOptional(true);
					sps_merged->addTrace(tr_inserted);
				}
				// Supprimer les éléments fusionnés
				traces.erase(traces.begin() + seqPos + 1, traces.begin() + bestPos + 1);
			}
		}
	}
	return seqPos;
}

std::pair<double, double> Sequence::computeBestCorrectedScore(std::vector<Trace::sp_trace> &traces1, std::vector<Trace::sp_trace> &traces2)
{
	// Et on calcule l'alignement entre ces deux ensembles de traces
	std::pair<double, double> res = TracesAnalyser::findBestAlignment(traces1, traces2, true);
	// Les valeurs retournées ne prennent pas en compte deux critères important pour déterminer l'intérêt de procéder à une fusion ou pas :
	//  1 - les parties optionnelles déjà présentes avant la fusion, en effet ici nous souhaitons uniquement considérer les trous directement liées à cet alignement. On corrige donc le score en considérant qu'une trace optionnelle non alignée est non pénalisante
	//  2 - les séquences non alignées (et non optionnelles) impactent le score avec un poids de 1 (identique à celui d'un simple Call), on le corrige ici pour intégrer leur longueur
	// On parcours donc chacune des deux traces et on retire du score toute trace optionnelle non alignée
	for (std::vector<Trace::sp_trace>::iterator it = traces1.begin(); it != traces1.end(); it++)
	{
		// Vérifier si la trace courrante est une séquence
		if ((*it)->isSequence())
		{
			Sequence *seqTmp = dynamic_cast<Sequence *>(it->get());
			// Si aucun alignement a été possible pour cette séquence MAIS que c'est une séquence optionnelle => on remonte le score
			if ((*it)->getAligned().expired() && seqTmp->isOptional())
				res.second--;
			// Si aucun alignement a été possible pour cette séquence ET que ce n'est pas une séquence optionnelle => on baisse le score
			if ((*it)->getAligned().expired() && !seqTmp->isOptional())
				res.second += seqTmp->length() - 1;
		}
	}
	// On fait la même chose pour la seconde séquence de traces
	for (std::vector<Trace::sp_trace>::iterator it = traces2.begin(); it != traces2.end(); it++)
	{
		if ((*it)->isSequence())
		{
			Sequence *seqTmp = dynamic_cast<Sequence *>(it->get());
			if ((*it)->getAligned().expired() && seqTmp->isOptional())
				res.second--;
			if ((*it)->getAligned().expired() && !seqTmp->isOptional())
				res.second += seqTmp->length() - 1;
		}
	}
	return res;
}