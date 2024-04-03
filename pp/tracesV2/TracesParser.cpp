#include "TracesParser.h"
#include "Sequence.h"
#include "TraceConstantList.h"
#include "VariantTKE.h"
#include "CallDef.h"
#include <cerrno>
#include <utility>

int TracesParser::TIME_LIMIT = 300;
float TracesParser::GAP_RATIO = 0.5;
bool TracesParser::outputLog = false;

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
	// we pop all new line included into the input file stream and we build the root sequence of traces
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

	// No more lines to read, now it's time to compress the root sequence
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
		Sequence::sp_sequence rootCopy = std::dynamic_pointer_cast<Sequence>(root->clone());
		// Recherche du meilleur pattern avec notre adaptation de TKE
		best_episode = tke.runAlgorithm(rootCopy);
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

		std::pair<int, int> mergedEpisode = best_episode->boundlist.back();
		std::vector<Trace::sp_trace> merge = Sequence::mergeLinearSequences(rootCopy->getSubSequence(mergedEpisode.first, mergedEpisode.second+1)->getLinearSequence(), bestPattern);
		unsigned int mergeCount = 1;
		int rootSize = rootCopy->getLinearSequence().size();
		// Parcourir tous les épisodes (de l'avant dernier au premier)
		for (int i = best_episode->boundlist.size()-2 ; i >= 0 ; i--){
			std::pair<int, int> currentEpisode = best_episode->boundlist[i];
			// vérifier si l'écart entre cet épisode et le précédent est inférieur au seuil
			if ((float)(mergedEpisode.first - currentEpisode.second - 1)/rootSize <= GAP_RATIO){
				// conversion de l'épisode courant en une séquence linéarisée (on inclus toutes les traces intercallées entre la fin de l'épisode courrant et le début des épisodes précédement fusionnés)
				std::vector<Trace::sp_trace> episode = rootCopy->getSubSequence(currentEpisode.first, mergedEpisode.first)->getLinearSequence();
				// calcule la fusion entre le dernier état de fusion et ce nouvel épisode
				merge = Sequence::mergeLinearSequences(episode, merge);
				mergedEpisode.first = currentEpisode.first; // on étend la plage de la fusion pour englober ce nouvel épisode
			} else {
				// injection de cette fusion dans le rootCopy
				// suppression de la partie du rootCopy devant être remplacé par le pattern
				std::vector<Trace::sp_trace> & traces = rootCopy->getTraces();
				traces.erase(traces.begin()+mergedEpisode.first, traces.begin()+mergedEpisode.second+1);
				// Incrustation du pattern dans le clone
				rootCopy->insertLinearSequence(merge, mergedEpisode.first);

				// on réinitialise la fusion au pattern fournit par TKE
				merge = Sequence::cloneLinearSequence(bestPattern);
				mergedEpisode = currentEpisode;
				// on comptabilise une integration supplémentaire
				mergeCount++;
			}
		}
		// injection de la dernière fusion dans le rootCopy
		// suppression de la partie du rootCopy devant être remplacé par le pattern
		std::vector<Trace::sp_trace> & traces = rootCopy->getTraces();
		traces.erase(traces.begin()+mergedEpisode.first, traces.begin()+mergedEpisode.second+1);
		// Incrustation du pattern dans le clone
		rootCopy->insertLinearSequence(merge, mergedEpisode.first);

		// si le nombre d'intégration au rootCopy est égal au nombre d'épisode c'est qu'on n'a pas réussi à faire de fusion entre épisodes. Chaque épisode a été réinjecté dans le rootCopy. Donc on s'arrête.
		if (mergeCount == best_episode->boundlist.size())
			break;
		
		// validation du rootCopy
		root = rootCopy;
		
	}while(true);

	if(has_passed_limite)
		osParser << "Over Time!\n";
	else
		osParser << "BEST SOLUTION !!!!" << std::endl;
	
	clock_t t_end = clock();
	osParser << "Time used : " << ((float(t_end-t0)/float(CLOCKS_PER_SEC))*1000) << " (ms)" << std::endl;
	osParser << "[1er]";
	//osParser << roots.begin()->scenario->score << " " << roots.begin()->scenario->alignCount  << " " << roots.begin()->scenario->optCount << " " << roots.begin()->scenario->position;
	//roots.begin()->sequence->exportAsCompressedString(osParser);
	root->exportAsCompressedString(osParser);

	//root = roots.begin()->sequence;
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