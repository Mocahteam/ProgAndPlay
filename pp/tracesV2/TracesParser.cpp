#include "TracesParser.h"
#include "TraceConstantList.h"
#include "Scenario.h"
#include "VariantTKE.h"
#include <cerrno>
#include <utility>

/**
  * Doit être mis à 1 pour prendre en compte les événements de Event::concatEventsArr rencontrés lors du parsage de fichier de traces brutes.
  */
#define INCLUDE_EVENTS 0

int TracesParser::TIME_LIMIT = 300;
int TracesParser::CANDIDATE_LIMIT = 200;
int TracesParser::DESCEND_LIMIT = 3;
bool TracesParser::outputLog = false;


/**
  * Seuil utilisé pour stopper et éviter toute future recherche de répétitions d'un groupe de traces à partir d'une trace.
  */
#define MAX_END_SEARCH 5

int TracesParser::mission_end_time = 0;
int TracesParser::execution_start_time = 0;
std::string TracesParser::lang = "";
std::string TracesParser::mission_name = "";
std::string TracesParser::params_json = "";

#ifdef DEBUG_PARSER
#ifdef LOG_IN_FILE
std::ofstream debugOfsParser("debugParser.log", std::ofstream::out);
std::ostream &TracesParser::osParser = debugOfsParser;
#else
std::ostream &TracesParser::osParser = std::cout;
#endif
#endif

TracesParser::TracesParser()
{
	root = std::make_shared<Sequence>(1, true); 
}

TracesParser::~TracesParser()
{
	CloseResources();
}

void TracesParser::CloseResources()
{
#ifdef DEBUG_PARSER
#ifdef LOG_IN_FILE
	if (debugOfsParser.is_open())
		debugOfsParser.close();
#endif
#endif
}

void TracesParser::saveCompression()
{
#ifdef DEBUG_PARSER
	exportTracesAsString(osParser);
#endif
	// Backup compression results as raw string
	std::ostringstream oss;
	exportTracesAsString(oss);
	this->lastCompression = oss.str();
	// Backup compression results with XML format
	this->lastCompressionXML.clear();
	rapidxml::xml_node<> *dec = this->lastCompressionXML.allocate_node(rapidxml::node_declaration);
	dec->append_attribute(this->lastCompressionXML.allocate_attribute("version", "1.0"));
	dec->append_attribute(this->lastCompressionXML.allocate_attribute("encoding", "utf-8"));
	this->lastCompressionXML.append_node(dec);
	rapidxml::xml_node<> *root_node = this->lastCompressionXML.allocate_node(rapidxml::node_element, "trace");
	this->lastCompressionXML.append_node(root_node);
	std::stack<rapidxml::xml_node<> *> node_stack;
	node_stack.push(root_node);
	rapidxml::xml_node<> *node;
	std::stack<Sequence::sp_sequence> seq_stack;
	Sequence::sp_sequence sps;
	Trace::sp_trace spt;
	unsigned int i = 0;
	bool pass = false;
	while (i < root->size() || !seq_stack.empty())
	{
		if (!seq_stack.empty())
		{
			while (!seq_stack.empty() && sps->isEndReached())
			{
				seq_stack.pop();
				node_stack.pop();
				if (!seq_stack.empty())
					sps = seq_stack.top();
			}
			if (!sps->isEndReached())
			{
				spt = sps->next();
				pass = true;
			}
		}
		if (seq_stack.empty() && i < root->size())
		{
			spt = root->at(i++);
			pass = true;
		}
		if (pass)
		{
			pass = false;
			if (spt->isCall())
			{
				Call *c = dynamic_cast<Call *>(spt.get());
				node = this->lastCompressionXML.allocate_node(rapidxml::node_element, "call");
				node->append_attribute(this->lastCompressionXML.allocate_attribute("label", this->lastCompressionXML.allocate_string(c->getKey().c_str())));
				std::string s = c->getParams();
				if (s.compare("") != 0)
					node->append_attribute(this->lastCompressionXML.allocate_attribute("params", this->lastCompressionXML.allocate_string(s.c_str())));
				s = c->getReturn();
				if (s.compare("") != 0)
					node->append_attribute(this->lastCompressionXML.allocate_attribute("return", this->lastCompressionXML.allocate_string(s.c_str())));
				node->append_attribute(this->lastCompressionXML.allocate_attribute("info", this->lastCompressionXML.allocate_string(c->getInfo().c_str())));
				node_stack.top()->append_node(node);
			}
			else
			{
				sps = std::dynamic_pointer_cast<Sequence>(spt);
				sps->reset();
				node = this->lastCompressionXML.allocate_node(rapidxml::node_element, "sequence");
				node->append_attribute(this->lastCompressionXML.allocate_attribute("num_map", this->lastCompressionXML.allocate_string(Sequence::getIterartionDescriptionString(sps->getIterationDescription()).c_str())));
				node->append_attribute(this->lastCompressionXML.allocate_attribute("info", this->lastCompressionXML.allocate_string(sps->getInfo().c_str())));
				std::string nb_iteration_fixed = (sps->hasNumberIterationFixed()) ? "true" : "false";
				node->append_attribute(this->lastCompressionXML.allocate_attribute("nb_iteration_fixed", this->lastCompressionXML.allocate_string(nb_iteration_fixed.c_str())));
				node_stack.top()->append_node(node);
				node_stack.push(node);
				seq_stack.push(sps);
			}
		}
	}
}

/**
  * \brief Lancement de la compression de traces brutes avec l'algorithme de compression hors-ligne.
  *
  * \param logs contenu des traces brutes
  */
void TracesParser::parseLogs(std::istream &logs)
{
	std::string line;
	// we pop all new line included into the input file stream
	int cpt = 0;
	while (std::getline(logs, line))
	{
		// we build a trace from the current line
		if (cpt % 100 == 0 && TracesParser::outputLog)
			std::cout << "Parse line number: " << cpt << std::endl;
		cpt++;
		Trace::sp_trace spt = parseLine(line);
		if (spt)
			root->addTrace(spt);
	} // End loop: read next line
	// No more lines to read
	offlineCompression();
	saveCompression();
    CloseResources();
}

Trace::sp_trace TracesParser::parseLine(const std::string &s)
{
	Trace *t = NULL;
	std::vector<std::string> tokens = splitLine(s);
	int numTokens = tokens.size();
	int ind = 0;
	// Recherche du token "-". Dans ce cas les tokens suivant représentent les valeurs retournées par l'appel.
	// On réduit donc le nombre de token pour ne compter que les paramètres et ne pas traiter ici les codes de retour.
	std::vector<std::string>::const_iterator it = std::find(tokens.begin(), tokens.end(), "-");
	while (it++ != tokens.end())
		numTokens--;
	// Gestion des appels dans la cas où le fichier params n'a pas été chargé ou l'appel est inconnu ou c'est un appel de fonction connu sans paramètre
	if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_NO_PARAMS) == 0)
	{
		t = new CallWithNoParam(tokens[ind]);
	}
	// Dans ce cas, nous sommes sur un appel de fonction avec paramètre
	// Gestion des appels ne prenant en paramètre qu'un entier
	else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_PARAM) == 0)
	{
		int p = (ind + 1 <= numTokens - 1) ? stoi(tokens[ind + 1]) : -1;
		t = new CallWithIntParam(tokens[ind], p);
	}
	// Gestion des appels prenant en paramètre deux entiers
	else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_INT_PARAMS) == 0)
	{
		int p1 = (ind + 1 <= numTokens - 1) ? stoi(tokens[ind + 1]) : -1;
		int p2 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
		t = new CallWithIntIntParams(tokens[ind], p1, p2);
	}
	// Gestion des appels prenant en paramètre un entier, une unité et un entier
	else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_UNIT_INT_PARAMS) == 0)
	{
		int p1 = (ind + 1 <= numTokens - 1) ? stoi(tokens[ind + 1]) : -1;
		// Récupération de l'id de l'unité et de son type (séparés par un "_")
		int unitId = -1, unitType = -1;
		if (ind + 2 <= numTokens - 1)
		{
			std::vector<std::string> unitParamTokens = splitLine(tokens[ind + 2], '_');
			unitId = stoi(unitParamTokens[0]);
			if (unitParamTokens.size() == 2)
				unitType = stoi(unitParamTokens[1]);
		}
		int p2 = (ind + 3 <= numTokens - 1) ? stoi(tokens[ind + 3]) : -1;
		t = new CallWithIntUnitIntParams(tokens[ind], p1, unitId, unitType, p2);
	}
	// Gestion des appels prenant en paramètre un entier et une unité
	else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_UNIT_PARAMS) == 0)
	{
		int p1 = (ind + 1 <= numTokens - 1) ? stoi(tokens[ind + 1]) : -1;
		// Récupération de l'id de l'unité et de son type (séparés par un "_")
		int unitId = -1, unitType = -1;
		if (ind + 2 <= numTokens - 1)
		{
			std::vector<std::string> unitParamTokens = splitLine(tokens[ind + 2], '_');
			unitId = stoi(unitParamTokens[0]);
			if (unitParamTokens.size() == 2)
				unitType = stoi(unitParamTokens[1]);
		}
		t = new CallWithIntUnitParams(tokens[ind], p1, unitId, unitType);
	}
	// Gestion des appels prenant en paramètre deux entiers et une unité
	else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_INT_UNIT_PARAMS) == 0)
	{
		int p1 = (ind + 1 <= numTokens - 1) ? stoi(tokens[ind + 1]) : -1;
		int p2 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
		// Récupération de l'id de l'unité et de son type (séparés par un "_")
		int unitId = -1, unitType = -1;
		if (ind + 3 <= numTokens - 1)
		{
			std::vector<std::string> unitParamTokens = splitLine(tokens[ind + 3], '_');
			unitId = stoi(unitParamTokens[0]);
			if (unitParamTokens.size() == 2)
				unitType = stoi(unitParamTokens[1]);
		}
		t = new CallWithIntIntUnitParams(tokens[ind], p1, p2, unitId, unitType);
	}
	else
	{
		// Dans ce cas, nous sommes sur un appel avec une unité en paramètres
		// Récupération de l'id de l'unité et de son type (séparés par un "_")
		int unitId = -1, unitType = -1;
		if (ind + 1 <= numTokens - 1)
		{
			std::vector<std::string> unitParamTokens = splitLine(tokens[ind + 1], '_');
			unitId = stoi(unitParamTokens[0]);
			if (unitParamTokens.size() == 2)
				unitType = stoi(unitParamTokens[1]);
		}
		// Gestion des appel ne prenant qu'une unité en paramètre
		if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_PARAM) == 0)
		{
			t = new CallWithUnitParam(tokens[ind], unitId, unitType);
		}
		// Gestion des appel prenant une unité et un entier en paramètre
		else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_PARAMS) == 0)
		{
			int p = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
			t = new CallWithUnitIntParams(tokens[ind], unitId, unitType, p);
		}
		// Gestion des appel prenant une unité et deux entiers en paramètre
		else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_INT_PARAMS) == 0)
		{
			int p1 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
			int p2 = (ind + 3 <= numTokens - 1) ? stoi(tokens[ind + 3]) : -1;
			t = new CallWithUnitIntIntParams(tokens[ind], unitId, unitType, p1, p2);
		}
		// Gestion des appel prenant une unité, un entier, une seconde unité et un second entier en paramètre
		else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_UNIT_INT_PARAMS) == 0)
		{
			int p1 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1, targetId = -1, targetType = -1;
			std::vector<std::string> targetParamTokens;
			if (ind + 3 <= numTokens - 1)
			{
				targetParamTokens = splitLine(tokens[ind + 3], '_');
				targetId = stoi(targetParamTokens[0]);
				if (targetParamTokens.size() == 2)
					targetType = stoi(targetParamTokens[1]);
			}
			int p2 = (ind + 4 <= numTokens - 1) ? stoi(tokens[ind + 4]) : -1;
			t = new CallWithUnitIntUnitIntParams(tokens[ind], unitId, unitType, p1, targetId, targetType, p2);
		}
		// Gestion des appel prenant une unité, un entier, une position et un second entier en paramètre
		else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_POS_INT_PARAMS) == 0)
		{
			int p1 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
			float x = (ind + 3 <= numTokens - 1) ? stof(tokens[ind + 3]) : -1, y = (ind + 4 <= numTokens - 1) ? stof(tokens[ind + 4]) : -1;
			int p2 = (ind + 5 <= numTokens - 1) ? stoi(tokens[ind + 5]) : -1;
			t = new CallWithUnitIntPosIntParams(tokens[ind], unitId, unitType, p1, x, y, p2);
		}
		// Gestion des appel prenant une unité, deux entiers  et un réel en paramètre
		else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_FLOAT_INT_PARAMS) == 0)
		{
			int p1 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
			float p2 = (ind + 3 <= numTokens - 1) ? stof(tokens[ind + 3]) : -1;
			int p3 = (ind + 4 <= numTokens - 1) ? stoi(tokens[ind + 4]) : -1;
			t = new CallWithUnitIntFloatIntParams(tokens[ind], unitId, unitType, p1, p2, p3);
		}
	}
	// Si la trace est valide on définit éventuellement les codes de retour en cas d'erreur
	if (t != NULL && t->isCall())
	{
		Call *c = dynamic_cast<Call *>(t);
		it = std::find(tokens.begin(), tokens.end(), "-");
		if (it != tokens.end())
		{
			while (++it != tokens.end())
				if ((*it).compare("?") == 0)
					c->setReturn();
				else
					c->addReturnCode(stof(*it));
		}
	}
	Trace::sp_trace spt(t);
	return spt;
}

void TracesParser::inlineCompression(Trace::sp_trace &spt)
{
	bool add = false;
	if (root->size() > 1)
	{
		// Get the last trace into this last sequence and look for the last Call
		Sequence::sp_sequence parent_seq = root;
		// Check if the last trace is a sequence
		if (root->getTraces().back()->isSequence())
		{
			// While the last trace of this sequence is a sequence, we continue to progress inside
			while (parent_seq->getTraces().back()->isSequence())
				parent_seq = std::dynamic_pointer_cast<Sequence>(parent_seq->getTraces().back());
			// Now "parent_seq" don't finish by a sequence
		}
		// Check if the last trace of this sequence is equal to the new trace
		Trace::sp_trace lastTrace = parent_seq->getTraces().back();
		if (spt->isCall() && lastTrace->operator==(spt.get()))
		{
			if (parent_seq->size() == 1)
			{
				// We include new trace in the sequence
				parent_seq->addOne();
			}
			else
			{
				// We filter newCall with the previous one
				Call::sp_call newCall = std::dynamic_pointer_cast<Call>(spt);
				newCall->filterCall(std::dynamic_pointer_cast<Call>(lastTrace).get());
				// We create a new sequence to store merge result
				Sequence::sp_sequence new_seq = std::make_shared<Sequence>(2);
				// Add this new filtered trace into the new sequence
				new_seq->addTrace(newCall);
				// Remove the last trace of the parent sequence and we replace it by the new sequence
				parent_seq->getTraces().pop_back();
				parent_seq->addTrace(new_seq);
			}
			add = true;
		}
	}
	// If no direct compression occurs we add the new trace at the end of traces
	if (!add)
		root->addTrace(spt);
}

class ScoredSequence {
	public:
		Scenario::sp_scenario scenario;
		Sequence::sp_sequence sequence;
		
		ScoredSequence(Sequence::sp_sequence newRoot){
			sequence = newRoot;
			scenario = std::make_shared<Scenario>(newRoot->getLinearSequence(), 0, 0);
		};
		
		ScoredSequence(Scenario::sp_scenario newscenario, Sequence::sp_sequence newsequence){
			scenario = newscenario;
			sequence = newsequence;
		};
};

bool sortFunction2 (Scenario::sp_scenario a, Scenario::sp_scenario b) { return ((a->score > b->score) || (a->score==b->score && a->getLength()<b->getLength())) ; }
bool sortFunction (ScoredSequence a, ScoredSequence b) { return sortFunction2(a.scenario, b.scenario); }

double calculer_kieme(std::vector<Scenario::sp_scenario> patterns, int k){
	if(k>=(int)patterns.size()){
		return 0.0;
	}
	std::vector<double> tab;
	for(unsigned int i=0; i<patterns.size(); i++){
		tab.push_back(patterns[i]->score);
	}
	std::sort(tab.begin(), tab.end());
	return tab[k];
}

bool vectorTracesSame(std::vector<Trace::sp_trace> v1, std::vector<Trace::sp_trace> v2){
	if(v1.size()!=v2.size())
		return false;
	for(unsigned int i=0; i<v1.size(); ++i)
	{
		if(!v1[i]->operator==(v2[i].get()))
			return false;
	}
	return true;
}

void TracesParser::offlineCompression()
{
	clock_t t0, dt;
	t0 = clock();

	//créer un objet pour l'algorithme VariantTKE
	VariantTKE tke = VariantTKE();

	std::vector<Trace::sp_trace> bestPattern;
	bool has_passed_limite = false;

	
	Episode::sp_episode best_episode;
	do{
		// Recherche du meilleur pattern avec notre adaptation de TKE
		best_episode = tke.runAlgorithm(root);
		if(best_episode	== 0 || best_episode->getSupport() <= 1) // cas d'arrêt de la boucle
			break;
		// couper si ça prend trop de temps
		dt = clock() - t0;
		if(TIME_LIMIT>0 && dt >= TIME_LIMIT*CLOCKS_PER_SEC){
			has_passed_limite = true;
			break;
		}
		
		// transformer le meilleur pattern en une séquence linearisée
		Sequence::sp_sequence seq = std::make_shared<Sequence>(0, false, false);
		for(unsigned int i=0; i<best_episode->events.size(); ++i)
			seq->addTrace(best_episode->events.at(i)->clone());
		std::vector<Trace::sp_trace> bestPattern = seq->getLinearSequence();

		std::vector<Trace::sp_trace> merge = Sequence::cloneLinearSequence(bestPattern);
		// Parcourir tous les épisodes (du dernier au premier)
		for (auto it = best_episode->boundlist.rbegin() ; it != best_episode->boundlist.rend() ; it++){
			// conversion de l'épisode en une séquence linéarisée
			std::vector<Trace::sp_trace> episode = root->getSubSequence(it->first, it->second+1)->getLinearSequence();
			// calcule la fusion entre l'épisode et le meilleur pattern
			merge = Sequence::mergeLinearSequences(episode, merge);

			// TODO: si les épisodes sont contigues il faut les fusionner comme ça mais s'il y a des trous entre les épisodes, il faut déterminer si on les merge (insérer les traces intermédiaire dans le pattern pour qu'elles soient mises en option) ou les sauter et repartir du bestPattern. Utiliser le WAR ?
		}
		

		// injection de cette fusion dans le root
		// suppression de la partie du root devant être remplacé par le pattern
		std::vector<Trace::sp_trace> & traces = root->getTraces();
		traces.erase(traces.begin()+it->first, traces.begin()+it->second+1);
		// Incrustation du pattern dans le clone
		root->insertLinearSequence(merge, it->first);

		root->exportAsCompressedString();
	}while(true);








/*	
	std::vector<ScoredSequence> roots;

	// enregistrement d'un premier root avec la trace brute
	roots.push_back(ScoredSequence(std::dynamic_pointer_cast<Sequence>(root->clone())));
	// parcourir tous les roots (ils sont ajoutés en queue de vecteur au fur et à mesure qu'ils sont découverts)
	for (unsigned int r = 0 ; r < roots.size() && !has_passed_limite ; r++)
	{
		// récupération du root courant
		Sequence::sp_sequence rootSequence = roots[r].sequence;
		if (TracesParser::outputLog){
			std::cout << "NEW PASS (" << (r+1) << "/" << roots.size() << ")" << std::endl;
			rootSequence->exportAsString();
		}

		// Réinitialiser le flag qui permet de savoir si on est en train de tourner dans une boucle, par sécurité on reset toutes les séquences du pattern
		for (int i = 0 ; i < (signed)roots[r].scenario->pattern.size() ; i++)
			if (roots[r].scenario->pattern[i]->isSequence() && roots[r].scenario->pattern[i]->getInfo().compare("Begin") == 0)
				std::dynamic_pointer_cast<Sequence>(roots[r].scenario->pattern[i])->newIter = false;

		// Recherche du meilleur pattern avec notre adaptation de TKE
		Episode::sp_episode best_episode = tke.runAlgorithm(rootSequence);
		// si on n'a pas de best pattern ou que son support est de 1 (pas de répétitions) => passer au root suivant
		if(best_episode	== 0 || best_episode->getSupport() <= 1){
			continue;
		}

		std::vector<Scenario::sp_scenario> patterns;

		// transform best episode into a sequence
		Sequence::sp_sequence seq = std::make_shared<Sequence>(0, false, false);
		for(unsigned int i=0; i<best_episode->events.size(); ++i)
			seq->addTrace(best_episode->events.at(i)->clone());
		// add this sequence to the patterns to merge
		patterns.push_back(std::make_shared<Scenario>(seq->getLinearSequence()));

		// parcours du meilleur episode de la position de sa première apparition à la position de sa dernière apparition et fusion de chaque évènement dans chacun des patterns
		for (int currentPos = best_episode->boundlist[0].first; currentPos <= best_episode->boundlist.back().second; currentPos++)
		{	// récupération de la trace courante
			Trace::sp_trace currentTrace = rootSequence->at(currentPos);

			dt = clock() - t0;
			if(TIME_LIMIT>0 && dt >= TIME_LIMIT*CLOCKS_PER_SEC){
				has_passed_limite = true;
				break;
			}

			if (TracesParser::outputLog){
				std::cout<<"Scenario "<< (r+1) << "/" << roots.size() << ": merge trace "<<currentPos<<"/"<<best_episode->boundlist.back().second<<" inside "<<patterns.size()<<" patterns (time elapsed:"<<dt/CLOCKS_PER_SEC<<")"<<std::endl;
			}

			insertTraceInsidePatterns(patterns, currentTrace);

			if(patterns.size()==0){
				// plus de pattern disponible
				break;
			}
		} // fin boucle : descente dans la trace

		// ne pas retenir les patterns qui n'ont pas permis de réduire la trace
		for (int i = (signed)patterns.size()-1 ; i >= 0 ; i--)
		{
			if (patterns[i]->getLength() >= best_episode->boundlist.back().second - best_episode->boundlist[0].first)
				patterns.erase(patterns.begin()+i);
		}
		
		int minLength = INT_MAX;

		// Maintenant qu'on a terminé de parcourir la trace, on créé autant de nouveaux roots que de patterns conservés pour poursuivre leur exploration
		int nbInsertion = 0;
		for (unsigned int i = 0 ; i < patterns.size() ; i++)
		{
			// On ignore tous les patterns qui contiendraient des calls non optionnelles au delà de la position du dernier ajout. En effet si le dernier call du root a été intégré en plein milieu d'un scénario et que ce scénario inclus dans la suite des calls obligatoires (non optionnels), cela signifierait que ce pattern produirait des calls au dela de la dernière trace du root... Ce qui n'est pas possible vue que le root est terminé, donc ce pattern est faux
			if (Sequence::getNonOptCallInLinearSequence(patterns[i]->pattern, patterns[i]->position+1) > 0)
				continue;

			// Vérifier que ce pattern n'a pas déjà été traité dans les scénarios déjà enregistrés
			bool found = false;
			for (unsigned int j = 0 ; j < roots.size() && !found ; j++)
				found = roots[j].scenario->isEqualWith(patterns[i]);
			if (!found){
				ScoredSequence ss = ScoredSequence(patterns[i], std::dynamic_pointer_cast<Sequence>(rootSequence->clone()));
Sequence::exportLinearSequenceAsString(patterns[i]->pattern);
				roots.insert(roots.begin()+r+nbInsertion+1, ss); // insertion pour une analyse en profondeur d'abord
				nbInsertion++;
				// Prise en compte de la qualité du pattern inclus
				ss.scenario->alignCount += roots[r].scenario->alignCount;
				ss.scenario->optCount += roots[r].scenario->optCount;
				// suppression de la partie du root devant être remplacé par le pattern
				std::vector<Trace::sp_trace> & traces = ss.sequence->getTraces();
				traces.erase(traces.begin() +best_episode->boundlist[0].first, traces.begin()+best_episode->boundlist.back().second+1);
				// Incrustation du pattern dans le clone
				ss.sequence->insertLinearSequence(patterns[i]->pattern, best_episode->boundlist[0].first);
				minLength = (signed)ss.sequence->length() < minLength ? ss.sequence->length() : minLength;
			}
		}
		if (minLength != INT_MAX){
			// mis à jour du scrore de tous les scénarios
			for (unsigned int i = 0 ; i < roots.size() ; i++){
				roots[i].scenario->updateScore(minLength);
			}
		}*/

		// Suite de Yufei...

		/*int maxAligned = 0;
		// Récupération de la longeur de la trace la plus courte
		int minLength = INT_MAX;
		for (unsigned int i = 0 ; i < patterns.size() ; i++){
			int len = patterns[i]->getLength();
			minLength = len < minLength ? len : minLength;
		}

		// Récupération des traces du root en cours de compression
		std::vector<Trace::sp_trace> vec_traces = rootSequence->getTraces();
		// Récupération de la portion comprise dans la fenêtre du meilleur des épisodes 
		std::vector<Trace::sp_trace> sub_traces(vec_traces.begin()+best_episode->boundlist[0].first, vec_traces.begin()+best_episode->boundlist.back().second+1);

		patterns.push_back(std::make_shared<Scenario>(sub_traces, 0, 0, (best_episode->boundlist.back().second+1 - best_episode->boundlist[0].first), 0, 0 ));
		patterns.back()->position = patterns.back()->pattern.size()-1;
		for (unsigned int i = 0 ; i < patterns.size() ; i++){
			patterns[i]->updateScore( minLength, maxAligned	);
		}
		std::sort(patterns.begin(), patterns.end(), sortFunction2);
		for (int i = (signed)patterns.size()-1 ; i>=0 ; --i){
			if(patterns[i]->checkEnd() == 0){
				patterns.erase(patterns.begin()+i);
			}
		}

		Scenario::sp_scenario sps = patterns[0];

		//remplacement la trace brute avec le meilleur scénario que nous avons trouvé 
		if(!vectorTracesSame(sps->pattern, sub_traces)){
			roots.push_back(ScoredSequence(sps, std::dynamic_pointer_cast<Sequence>(rootSequence->clone())));
			std::vector <Trace::sp_trace> &traces = roots.back().sequence->getTraces();
	        traces.erase(traces.begin() +best_episode->boundlist[0].first, traces.begin()+best_episode->boundlist.back().second+1);
			roots.back().sequence->insertLinearSequence(patterns[0]->pattern, best_episode->boundlist[0].first);


			//std::vector<Trace::sp_trace> vec_traces = rootSequence->getTraces();
			std::vector<Trace::sp_trace> vec_prev, vec_post;
			for(int prev_pos=0; prev_pos<best_episode->boundlist[0].first; ++prev_pos){
				vec_prev.push_back(vec_traces[prev_pos]);
			}
			for(int post_pos=best_episode->boundlist.back().second+1; post_pos<(int)vec_traces.size(); ++post_pos){
				vec_post.push_back(vec_traces[post_pos]);
			}
			if(vec_prev.size()>0){
				roots.back().scenario->insertTraces(0, vec_prev);
				// roots.back().sequence->insertTraces(0, vec_prev);
			}
			if(vec_post.size()>0){
				roots.back().scenario->insertTraces(1, vec_post);
				// roots.back().sequence->insertTraces(1, vec_post);
			}
		}*/
/*	}

	std::sort(roots.begin(), roots.end(), sortFunction);*/

	if(has_passed_limite)
		osParser << "Over Time!\n";
	else
		osParser << "BEST SOLUTION !!!!" << std::endl;
	
	clock_t t_end = clock();
	osParser << "Time used : " << ((float(t_end-t0)/float(CLOCKS_PER_SEC))*1000) << " (ms)" << std::endl;
	osParser << "[1er] ";
	//osParser << roots.begin()->scenario->score << " " << roots.begin()->scenario->alignCount  << " " << roots.begin()->scenario->optCount << " " << roots.begin()->scenario->position;
	//roots.begin()->sequence->exportAsCompressedString(osParser);
	root->exportAsCompressedString(osParser);

	//root = roots.begin()->sequence;
}

void TracesParser::insertTraceInsidePatterns(std::vector<Scenario::sp_scenario> & patterns, const Trace::sp_trace & currentTrace){
	/*if (TracesParser::outputLog){
		std::cout << "Current trace :";
		currentTrace->exportAsString();
	}
	// calcul du meilleur score parmis les patterns
	float maxScore = 0;
	// Positionner chaque pattern sur son prochain Call et enregistrer les nouveaux patterns générés en conséquence
	std::vector<Scenario::sp_scenario> new_patterns;
	for (unsigned int i = 0 ; i < patterns.size() ; i++){
		if (patterns[i]->score > maxScore)
			maxScore = patterns[i]->score;
		std::vector<Scenario::sp_scenario> results = patterns[i]->simulateMoveToNextCall();
		new_patterns.insert(new_patterns.end(), results.begin(), results.end());
	}
	patterns = std::move(new_patterns);
	if(patterns.size()==0){
		// plus de pattern disponible
		return;
	}

	// Récupération de la longeur de la trace la plus courte
	int minLength = INT_MAX;
	for (unsigned int i = 0 ; i < patterns.size() ; i++){
		int len = patterns[i]->getLength();
		minLength = len < minLength ? len : minLength;
	}

	// Fusion du Call courrant du root dans chaque pattern et enregistrer les nouveaux patterns générés en conséquence et non encore explorés
	std::vector<Scenario::sp_scenario> results;
	for (unsigned int i = 0 ; i < patterns.size() ; i++){
		std::vector<Scenario::sp_scenario> sim = patterns[i]->simulateNewTraceIntegration(std::dynamic_pointer_cast<Call>(currentTrace), maxScore, minLength); reprendre là
		// déterminer si ces nouvelles simulations sont à explorer
		for (unsigned int j = 0 ; j < sim.size() ; j++){
			// Supprimer les anciens résultats qui seraient plus large (inclusion) que la nouvelle simulation donc plus compacte
			for (int k = (signed)results.size()-1 ; k >= 0 ; k--){
				if (sim[j]->isIncludedIn(results[k]))
					results.erase(results.begin()+k); // cet ancien résultat inclut la nouvelle simulation => il est donc plus large => on le supprime
			}
			// Vérifier que la nouvelle simulation n'est pas incluse dans un ancien résultat
			bool included = false;
			for (int k = (signed)results.size()-1 ; k >= 0 && !included; k--){
				if (results[k]->isIncludedIn(sim[j]))
					included = true; // cet ancien résultat est inclus dans la nouvelle simulation => il est donc plus compact => on saute cette nouvelle simulation
			}
			//if (!sim[j]->existsIn(results))
			if (!included)
				results.push_back(sim[j]); // on ajoute cette nouvelle simulation que si elle n'est pas incluse dans un ancien résultat
		}
	}
	patterns = std::move(results);
	if(patterns.size()==0){
		// plus de pattern disponible
		return;
	}
	
	// Mise à jour de la longeur de la trace la plus courte
	for (unsigned int i = 0 ; i < patterns.size() ; i++){
		int len = patterns[i]->getLength();
		minLength = len < minLength ? len : minLength;
	}

	// Mise à jour du score de chaque pattern et calcul du score maximum
	patterns[0]->updateScore(minLength);
	maxScore = patterns[0]->score;
	for (unsigned int i = 1 ; i < patterns.size() ; i++){
		patterns[i]->updateScore( minLength	);
		if (patterns[i]->score > maxScore){
			maxScore = patterns[i]->score;
		}
	}

	// Suppression de tous les patterns qui ont un score qui ne cesse de diminuer ou trop faible
	for (int i = (signed)patterns.size()-1 ; i >= 0 ; i--){
		if (patterns[i]->num_descend >= DESCEND_LIMIT || patterns[i]->score < maxScore - Scenario::SCORE_TOLERENCE * maxScore){
			patterns.erase(patterns.begin()+i);
		}
	}

	std::sort(patterns.begin(), patterns.end(), sortFunction2);
	// On ne garde que les CANDIDATE_LIMIT meilleurs patterns
	if((int)patterns.size()>CANDIDATE_LIMIT){
		patterns.erase(patterns.begin()+CANDIDATE_LIMIT, patterns.end());
	}
	
	else{
		// Ici on doit insérer une séquence dans tous les patterns
		// Positionner chaque pattern sur sa prochaine Trace

		// 2 cas possibles :
		//   1 - mettre cette séquence en optionnel et donc la sauter
		//   2 - entrer dans cette séquence pour tenter de s'aligner avec son contenu

		Sequence::sp_sequence currentSeq = std::dynamic_pointer_cast<Sequence>(currentTrace);
		
		// cas 1 
		// Cloner le pattern
		Sequence::sp_sequence clonedSq = currentSeq->clone();
		// Mettre la séquence en optionnelle
		clonedSq->setOptional(true);
		// On insère cette séquence optionnelle dans le pattern
		tmpSc->pattern.insert(tmpSc->pattern.begin()+endSeqPos, rootCall->clone());


		// On positionne ce clone sur la fin de la séquence
		clonedSc->position = endSeqPos;
		// Et on fait un appel récursif pour atteindre le prochain Call
		std::vector<Scenario::sp_scenario> res = clonedSc->simulateMoveToNextCall();
		results.insert(results.end(), res.begin(), res.end());
		while (!currentSeq->isEndReached()){
			insertTraceInsidePatterns(patterns, currentSeq->next());
			if(patterns.size()==0){
				// plus de pattern disponible
				return;
			}
		}
	}*/
}

void TracesParser::exportTracesAsString(std::ostream &os)
{
	for (unsigned int i = 0; i < root->size(); i++)
		root->at(i)->exportAsString(os);
	if (Trace::numTab > 0)
		Trace::numTab = 0;
}

std::vector<Trace::sp_trace> TracesParser::importTraceFromXml(const std::string &xml, std::ostream &os)
{
	std::vector<Trace::sp_trace> traces;
	try
	{
		std::vector<char> xml_content(xml.begin(), xml.end());
		xml_content.push_back('\0');
		rapidxml::xml_document<> doc;
		doc.parse<0>(&xml_content[0]);
		rapidxml::xml_node<> *root_node = doc.first_node("trace");
		if (root_node != 0)
		{
#ifdef DEBUG_PARSER
			os << "\nbegin import from XML file" << std::endl;
#endif
			importTraceFromNode(root_node->first_node(), traces);
			for (unsigned int i = 0; i < traces.size(); i++)
			{
#ifdef DEBUG_PARSER
				os << i << " " << traces.at(i) << std::endl;
#endif
				traces.at(i)->exportAsString(os);
			}
		}
	}
	catch (const std::runtime_error &e)
	{
#ifdef DEBUG_PARSER
		os << e.what() << std::endl;
#endif
		if (TracesParser::outputLog)
			std::cout << e.what() << std::endl;
	}
	return traces;
}

void TracesParser::importTraceFromNode(rapidxml::xml_node<> *node, std::vector<Trace::sp_trace> &traces)
{
	Sequence::sp_sequence sps;
	std::stack<rapidxml::xml_node<> *> node_stack;
	std::stack<Sequence::sp_sequence> seq_stack;
	std::string node_name;
	while (node || !node_stack.empty())
	{
		while (!node && !node_stack.empty())
		{
			node = node_stack.top();
			node_name = node->name();
			if (node_name.compare("sequence") == 0)
			{
				seq_stack.pop();
				if (!seq_stack.empty())
				{
					seq_stack.top()->addTrace(sps);
					sps = seq_stack.top();
				}
				else
					traces.push_back(sps);
			}
			node = node_stack.top()->next_sibling();
			node_stack.pop();
		}
		if (node)
		{
			node_name = node->name();
			if (node_name.compare("sequence") == 0)
			{
				std::string info;
				if (node->first_attribute("info") != 0)
					info = node->first_attribute("info")->value();
				bool num_fixed = false;
				if (node->first_attribute("nb_iteration_fixed") != 0)
					num_fixed = std::string(node->first_attribute("nb_iteration_fixed")->value()).compare("true") == 0;
				sps = std::make_shared<Sequence>(info, num_fixed);
				if (node->first_attribute("num_map") != 0)
				{
					std::vector<std::string> tokens = splitLine(node->first_attribute("num_map")->value());
					for (unsigned int i = 0; i < tokens.size(); i++)
					{
						int pos = tokens.at(i).find(":", 0);
						sps->addIteration(stoi(std::string(tokens.at(i).begin(), tokens.at(i).begin() + pos)), stoi(std::string(tokens.at(i).begin() + pos + 1, tokens.at(i).end())));
					}
				}
				seq_stack.push(sps);
				node_stack.push(node);
				node = node->first_node();
			}
			else
			{
				std::string s = "";
				if (node->first_attribute("error") != 0)
					s += std::string(node->first_attribute("error")->value()) + " ";
				s += std::string(node->first_attribute("label")->value());
				if (node->first_attribute("params") != 0)
					s += " " + std::string(node->first_attribute("params")->value());
				if (node->first_attribute("return") != 0)
					s += " - " + std::string(node->first_attribute("return")->value());
				Trace::sp_trace spt = parseLine(s);
				if (!seq_stack.empty())
					sps->addTrace(spt);
				else
					traces.push_back(spt);
				if (node->first_attribute("info") != 0)
					spt->setInfo(node->first_attribute("info")->value());
				node = node->next_sibling();
			}
		}
	}
}

unsigned int TracesParser::getNodeChildCount(rapidxml::xml_node<> *node)
{
	unsigned int i = 0;
	for (rapidxml::xml_node<> *child = node->first_node(); child; child = child->next_sibling())
		i++;
	return i;
}

void TracesParser::setLang(std::string lang)
{
	TracesParser::lang = lang;
}

std::vector<std::string> TracesParser::splitLine(const std::string &s, char delim)
{
	std::vector<std::string> buf;
	std::stringstream ss(s);
	std::string subs;
	while (std::getline(ss, subs, delim))
	{
		if (subs[subs.size() - 1] == '\r' || subs[subs.size() - 1] == '\n')
			subs.erase(subs.size() - 1);
		buf.push_back(subs);
	}
	return buf;
}

int TracesParser::stoi(const std::string &s)
{
	int res;
	if (s.compare("?") == 0)
		return -1;
	try
	{
		res = std::stoi(s);
	}
	catch (const std::exception &)
	{
#ifdef DEBUG_PARSER
		osParser << "error std::stoi(" << s << ")" << std::endl;
#endif
		if (TracesParser::outputLog)
			std::cout << "error std::stoi(" << s << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
	return res;
}

float TracesParser::stof(const std::string &s)
{
	float res;
	if (s.compare("?") == 0)
		return -1;
	try
	{
		res = std::stof(s);
	}
	catch (const std::exception &)
	{
#ifdef DEBUG_PARSER
		osParser << "error std::stof" << std::endl;
#endif
		if (TracesParser::outputLog)
			std::cout << "error std::stof" << std::endl;
		exit(EXIT_FAILURE);
	}
	return res;
}


/**
  * Cette valeur comprise dans l'intervalle [1,+inf] est utilisée pour définir l'intervalle de définition du bonus ajouté au score de similarité dans le cas de la tentative d'alignement entre deux séquences.
  */
#define IND_SEQ_NUM_CONST 4
/**
  * Score utilisé pour l'alignement. Correspond au pire score possible pour l'alignement. Les deux traces comparées ne seront jamais alignées.
  */
#define ALIGN_MISMATCH_SCORE -1
/**
  * Score utilisé pour l'alignement. Correpond au score obtenu si on aligne la trace avec rien (introduction d'un trou).
  */
#define ALIGN_GAP_SCORE 0
#define INF -1
#define SUP 1
/**
  * Macro utilisée pour changer l'intervalle de définition du score de [0,1] à [TracesAnalyser::INF,TracesAnalyser::SUP].
  */
#define TRANSFORM_SCORE(val) ((SUP - INF) * val + INF)

std::pair<double, double> TracesParser::findBestAlignment(const std::vector<Trace::sp_trace> &l, const std::vector<Trace::sp_trace> &e, bool align)
{
	int cpt_path = 0;
	unsigned int lsize = l.size() + 1, esize = e.size() + 1;
	double score = 0, norm = 0;
	std::pair<double, double> **val = new std::pair<double, double> *[lsize];
	std::pair<int, int> **ind = new std::pair<int, int> *[lsize];
	char **help = new char *[lsize];
#ifdef DEBUG_PARSER
	osParser << "begin findBestAlignment" << std::endl;
#endif
	for (unsigned int i = 0; i < l.size(); i++)
	{
#ifdef DEBUG_PARSER
		l.at(i)->exportAsString(osParser);
#endif
		if (align)
			l.at(i)->resetAligned();
	}
#ifdef DEBUG_PARSER
	osParser << std::endl;
#endif
	for (unsigned int j = 0; j < e.size(); j++)
	{
#ifdef DEBUG_PARSER
		e.at(j)->exportAsString(osParser);
#endif
		if (align)
			e.at(j)->resetAligned();
	}
#ifdef DEBUG_PARSER
	osParser << std::endl;
#endif
	for (unsigned int i = 0; i < lsize; i++)
	{
		val[i] = new std::pair<double, double>[esize];
		ind[i] = new std::pair<int, int>[esize];
		help[i] = new char[esize];
	}
	for (unsigned int i = 0; i < lsize; i++)
	{
		val[i][0] = std::pair<double, double>(ALIGN_GAP_SCORE * i, 0);
		ind[i][0] = std::pair<int, int>(i - 1, 0);
		help[i][0] = 'h';
	}
	for (unsigned int j = 1; j < esize; j++)
	{
		val[0][j] = std::pair<double, double>(ALIGN_GAP_SCORE * j, 0);
		ind[0][j] = std::pair<int, int>(0, j - 1);
		help[0][j] = 'g';
	}
	for (unsigned int i = 1; i < lsize; i++)
	{
		for (unsigned int j = 1; j < esize; j++)
		{
			val[i][j] = std::pair<double, double>(0, 0);
			ind[i][j] = std::pair<int, int>(i - 1, j - 1);
			double match_score = 0;
			if (l.at(i - 1)->isCall() && e.at(j - 1)->isCall())
			{
				Call::sp_call learner_spc = std::dynamic_pointer_cast<Call>(l.at(i - 1));
				Call::sp_call expert_spc = std::dynamic_pointer_cast<Call>(e.at(j - 1));
				match_score = 1 - learner_spc->getEditDistance(expert_spc.get());
				val[i][j].second = match_score;
				match_score = TRANSFORM_SCORE(match_score);
			}
			else if (l.at(i - 1)->isSequence() && e.at(j - 1)->isSequence())
			{
				Sequence::sp_sequence learner_sps = std::dynamic_pointer_cast<Sequence>(l.at(i - 1));
				Sequence::sp_sequence expert_sps = std::dynamic_pointer_cast<Sequence>(e.at(j - 1));
				std::pair<double, double> res = findBestAlignment(learner_sps->getTraces(), expert_sps->getTraces(), false);
				match_score = res.first;
				val[i][j].second = match_score;
				if (expert_sps->hasNumberIterationFixed() && !learner_sps->isImplicit() && !expert_sps->isImplicit())
				{
					double mean_dis = learner_sps->getIterationDescriptionMeanDistance(expert_sps);
					val[i][j].second += (1 - mean_dis) * (res.second / IND_SEQ_NUM_CONST);
				}
				match_score /= res.second;
				match_score = TRANSFORM_SCORE(match_score);
			}
			else
				match_score = ALIGN_MISMATCH_SCORE;
			val[i][j].first = val[i - 1][j - 1].first + match_score;
			help[i][j] = 'd';
			if (val[i - 1][j].first + ALIGN_GAP_SCORE > val[i][j].first && val[i - 1][j].first + ALIGN_GAP_SCORE > val[i][j - 1].first + ALIGN_GAP_SCORE)
			{
				val[i][j].first = val[i - 1][j].first + ALIGN_GAP_SCORE;
				val[i][j].second = 0;
				ind[i][j].second++;
				help[i][j] = 'h';
			}
			else if (val[i][j - 1].first + ALIGN_GAP_SCORE > val[i][j].first)
			{
				val[i][j].first = val[i][j - 1].first + ALIGN_GAP_SCORE;
				val[i][j].second = 0;
				ind[i][j].first++;
				help[i][j] = 'g';
			}
			if ((val[i][j].first == val[i - 1][j].first + ALIGN_GAP_SCORE && val[i][j].first >= val[i][j - 1].first + ALIGN_GAP_SCORE) || (val[i][j].first == val[i][j - 1].first + ALIGN_GAP_SCORE && val[i][j].first > val[i - 1][j].first + ALIGN_GAP_SCORE) || (val[i - 1][j].first + ALIGN_GAP_SCORE == val[i][j - 1].first + ALIGN_GAP_SCORE && val[i][j - 1].first + ALIGN_GAP_SCORE > val[i][j].first))
				cpt_path++;
		}
	}
#ifdef DEBUG_PARSER
	for (unsigned int i = 0; i < lsize; i++)
	{
		for (unsigned int j = 0; j < esize; j++)
			osParser << "(" << val[i][j].first << "," << val[i][j].second << ")\t";
		osParser << std::endl;
	}
	osParser << std::endl;
	for (unsigned int i = 0; i < lsize; i++)
	{
		for (unsigned int j = 0; j < esize; j++)
			osParser << "(" << ind[i][j].first << "," << ind[i][j].second << ")\t";
		osParser << std::endl;
	}
	osParser << std::endl;
	for (unsigned int i = 0; i < lsize; i++)
	{
		for (unsigned int j = 0; j < esize; j++)
			osParser << help[i][j] << "\t";
		osParser << std::endl;
	}
	osParser << std::endl;
#endif
	std::vector< std::pair<int, int> > p;
	std::pair<int, int> pind = ind[lsize - 1][esize - 1];
	score += val[lsize - 1][esize - 1].second;
	while (pind.first >= 0 && pind.second >= 0)
	{
		p.push_back(pind);
		score += val[pind.first][pind.second].second;
		pind = ind[pind.first][pind.second];
	}
	std::reverse(p.begin(), p.end());

#ifdef DEBUG_PARSER
	osParser << "path : ";
	for (unsigned int i = 0; i < p.size(); i++)
		osParser << "(" << p.at(i).first << "," << p.at(i).second << ") ";
	osParser << std::endl;
#endif

	for (unsigned int i = 0; i < p.size(); i++)
	{
		double norm_val = 1;
		int indi = p.at(i).first, indj = p.at(i).second;
		if ((i < p.size() - 1 && indi == p.at(i + 1).first) || indi >= (int)l.size())
		{
			if (align)
				e.at(indj)->resetAligned();
		}
		else if ((i < p.size() - 1 && indj == p.at(i + 1).second) || indj >= (int)e.size())
		{
			if (align)
				l.at(indi)->resetAligned();
		}
		else
		{
			if (l.at(indi)->isSequence() && e.at(indj)->isSequence())
			{
				Sequence::sp_sequence learner_sps = std::dynamic_pointer_cast<Sequence>(l.at(indi));
				Sequence::sp_sequence expert_sps = std::dynamic_pointer_cast<Sequence>(e.at(indj));
				std::pair<double, double> res = findBestAlignment(learner_sps->getTraces(), expert_sps->getTraces(), align);
				norm_val = res.second;
				if (expert_sps->hasNumberIterationFixed() && !learner_sps->isImplicit() && !expert_sps->isImplicit())
					norm_val += res.second / IND_SEQ_NUM_CONST;
			}
			if (align)
			{
				l.at(indi)->setAligned(e.at(indj));
				e.at(indj)->setAligned(l.at(indi));
			}
		}
		norm += norm_val;
	}

	for (unsigned int i = 0; i < lsize; i++)
	{
		delete[] val[i];
		delete[] ind[i];
		delete[] help[i];
	}
	delete[] val;
	delete[] ind;
	delete[] help;
#ifdef DEBUG_PARSER
	if (cpt_path == 0)
		osParser << "only one path" << std::endl;
	else
		osParser << "more than one path" << std::endl;
	osParser << "end findBestAlignment " << score << "/" << norm << std::endl;
#endif
	return std::pair<double, double>(score, norm);
}