#include "TracesParser.h"
#include "TraceConstantList.h"
#include <cerrno>

/**
  * Doit être mis à 1 pour prendre en compte les événements de Event::concatEventsArr rencontrés lors du parsage de fichier de traces brutes.
  */
#define INCLUDE_EVENTS 0

/**
  * Seuil utilisé pour stopper et éviter toute future recherche de répétitions d'un groupe de traces à partir d'une trace.
  */
#define MAX_END_SEARCH 5

/**
 * Seuil utilisé pour déterminer quels patterns concervés. Un Pattern dont le score est inférieur de SCORE_TOLERENCE par rapport au meilleur score calculé sera supprimé
 */
#define SCORE_TOLERENCE 0.1

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

TracesParser::TracesParser() : used(false), compressed(false), end(false), proceed(false), start(0)
{
	root = boost::make_shared<Sequence>(1, true); 
}

TracesParser::~TracesParser()
{
	CloseResources();
}

void TracesParser::CloseResources()
{
	start = 0;
	used = false;
	end = false;
	compressed = false;
	proceed = false;
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
			if (spt->isEvent())
			{
				Event *e = dynamic_cast<Event *>(spt.get());
				if (e->getLabel().compare(MISSION_START_TIME) == 0)
				{
					while (node_stack.size() > 1)
						node_stack.pop();
					StartMissionEvent *sme = dynamic_cast<StartMissionEvent *>(e);
					node = this->lastCompressionXML.allocate_node(rapidxml::node_element, "mission");
					node->append_attribute(this->lastCompressionXML.allocate_attribute("name", this->lastCompressionXML.allocate_string(sme->getMissionName().c_str())));
					node->append_attribute(this->lastCompressionXML.allocate_attribute("start_time", this->lastCompressionXML.allocate_string(boost::lexical_cast<std::string>(sme->getStartTime()).c_str())));
					node_stack.top()->append_node(node);
					node_stack.push(node);
				}
				else if (e->getLabel().compare(MISSION_END_TIME) == 0)
				{
					EndMissionEvent *eme = dynamic_cast<EndMissionEvent *>(e);
					if (node_stack.size() > 2)
						node_stack.pop();
					node_stack.top()->append_attribute(this->lastCompressionXML.allocate_attribute("end_time", this->lastCompressionXML.allocate_string(boost::lexical_cast<std::string>(eme->getEndTime()).c_str())));
					node_stack.top()->append_attribute(this->lastCompressionXML.allocate_attribute("status", this->lastCompressionXML.allocate_string(eme->getStatus().c_str())));
					node_stack.pop();
				}
				else if (e->getLabel().compare(EXECUTION_START_TIME) == 0)
				{
					NewExecutionEvent *nee = dynamic_cast<NewExecutionEvent *>(e);
					if (node_stack.size() > 2)
						node_stack.pop();
					node = this->lastCompressionXML.allocate_node(rapidxml::node_element, "execution");
					node->append_attribute(this->lastCompressionXML.allocate_attribute("start_time", this->lastCompressionXML.allocate_string(boost::lexical_cast<std::string>(nee->getStartTime()).c_str())));
					node->append_attribute(this->lastCompressionXML.allocate_attribute("programming_language_used", this->lastCompressionXML.allocate_string(nee->getProgrammingLangageUsed().c_str())));
					node_stack.top()->append_node(node);
					node_stack.push(node);
				}
				else if (e->getLabel().compare(EXECUTION_END_TIME) == 0)
				{
					node_stack.top()->append_attribute(this->lastCompressionXML.allocate_attribute("end_time", this->lastCompressionXML.allocate_string(boost::lexical_cast<std::string>(dynamic_cast<EndExecutionEvent *>(e)->getEndTime()).c_str())));
					node_stack.pop();
				}
				else
				{
					node = this->lastCompressionXML.allocate_node(rapidxml::node_element, "event");
					node->append_attribute(this->lastCompressionXML.allocate_attribute("label", this->lastCompressionXML.allocate_string(e->getLabel().c_str())));
					if (!e->getInfo().empty())
						node->append_attribute(this->lastCompressionXML.allocate_attribute("info", this->lastCompressionXML.allocate_string(e->getInfo().c_str())));
					node_stack.top()->append_node(node);
				}
			}
			else if (spt->isCall())
			{
				Call *c = dynamic_cast<Call *>(spt.get());
				node = this->lastCompressionXML.allocate_node(rapidxml::node_element, "call");
				node->append_attribute(this->lastCompressionXML.allocate_attribute("label", this->lastCompressionXML.allocate_string(c->getKey().c_str())));
				if (c->getError() != Call::NONE)
					node->append_attribute(this->lastCompressionXML.allocate_attribute("error", this->lastCompressionXML.allocate_string(Call::getEnumLabel<Call::ErrorType>(c->getError(), Call::errorsArr))));
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
				sps = boost::dynamic_pointer_cast<Sequence>(spt);
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
  * \param waitEndFlag si true (défaut) l'appel est bloquant jusqu'à ce que le flag "end" soit activé (voir TraceParser::setEnd) - utile si le flux d'entrée (logs) est alimenté en même temps qu'il est lu (cas de Prog&Play). Si false le flux d'entrée est lu et analysé en une seule fois.
  */
void TracesParser::parseLogs(std::istream &logs, bool waitEndFlag)
{
// check if we can start parsing
#ifdef DEBUG_PARSER
	osParser << "Start parsing traces" << std::endl;
#endif
	if (!used)
	{
		used = true;
		std::string line;
		bool executionDetected = false;
#ifdef DEBUG_PARSER
		osParser << "Loop until end is set" << std::endl;
#endif
		end = false;
		proceed = false;
		while (!end)
		{
			// If we don't have to wait the "end" flag we set it to true and the "proceed" also in order to compress and exit after reading all the file (no waiting new input)
			if (!waitEndFlag)
			{
				end = true;
				proceed = true;
			}
			// we pop all new line included into the input file stream
			int cpt = 0;
			while (std::getline(logs, line))
			{
// we build a trace from the current line
#ifdef DEBUG_PARSER
				osParser << std::endl
						 << "Parse line: " << line;
#endif
				if (cpt % 100 == 0)
					std::cout << "Parse line number: " << cpt << std::endl;
				cpt++;
				Trace::sp_trace spt = parseLine(line);
				if (spt)
				{
#ifdef DEBUG_PARSER
					osParser << "Check if this line is an event" << std::endl;
#endif
					Event::sp_event spe;
					if (spt->isEvent())
					{
#ifdef DEBUG_PARSER
						osParser << "\tThis line is an event" << std::endl;
#endif
						spe = boost::dynamic_pointer_cast<Event>(spt);
					}
					else
					{
#ifdef DEBUG_PARSER
						osParser << "\tThis line is NOT an event" << std::endl;
#endif
					}

					// We check if this trace is an event that we can't aggregate
					if (spe)
					{
						if (Trace::inArray(spe->getLabel().c_str(), Event::noConcatEventsArr) > -1)
						{
#ifdef DEBUG_PARSER
							osParser << "\tevent nature: " << spe->getLabel().c_str() << std::endl;
#endif
							// If we detect a new execution
							if (spe->getLabel().compare(EXECUTION_START_TIME) == 0)
							{
								// Check if a previous execution has been detected
								if (executionDetected)
								{
// We try to detect and compress sequences. This case appears when two executions are launched in a same mission without an end execution event
#ifdef DEBUG_PARSER
									osParser << "\tNew Execution: We try to detect and compress sequences" << std::endl;
#endif
									offlineCompression();
// And we define the top of the trace as the new starting point
#ifdef DEBUG_PARSER
									osParser << "\tNew Execution: The top of the trace is now the new starting point" << std::endl;
#endif
									start = root->size();
								}
								// we set execution flag
								executionDetected = true;
								// We end by adding this event at the end of the trace
								root->addTrace(spe);
							}
							else if (spe->getLabel().compare(EXECUTION_END_TIME) == 0)
							{ // If we detect an end execution
								// We start by adding this event at the end of the trace
								root->addTrace(spe);
// We try to detect and compress sequences. Default case, if we receive an end execution event we have to compress last trace
#ifdef DEBUG_PARSER
								osParser << "\tEnd Execution: We try to detect and compress sequences" << std::endl;
#endif
								offlineCompression();
// And we define the top of the trace as the new starting point
#ifdef DEBUG_PARSER
								osParser << "\tEnd Execution: The top of the trace is now the new starting point" << std::endl;
#endif
								start = root->size();
								// we reset execution flag
								executionDetected = false;
							}
							else
							{
								// We simply add this event at the end of the trace
								root->addTrace(spe);
							}
						}
					}
					else
					{
// The trace is agregable, so we try to do it
#ifdef DEBUG_PARSER
						osParser << "Minimal inline compression" << std::endl;
#endif
						// Desable in-line compression
						//inlineCompression(spt);
						root->addTrace(spt);
#ifdef DEBUG_PARSER
						osParser << "After inline compression" << std::endl;
						root->exportAsString(osParser);
#endif
					}
				}
			}
			// No more lines to read, check if it's due to end of file
			// If yes, we can continue
			// If no (means another error), then we stop to read input file
			if (!logs.eof())
			{
#ifdef DEBUG_PARSER
				osParser << "error occurs => we stop to read traces" << std::endl;
#endif
				end = true; // Stop to read input file
			}
			logs.clear();
			// check if we have to compress agregate traces. proceed means game engine asks to proceed compression
			if (proceed)
			{
				// check if at least two traces are becoming since previous start mission event
				if (start < (int)root->size() - 1)
				{
// We try to detect and compress remaining traces
#ifdef DEBUG_PARSER
					osParser << "\tWe try to detect and compress sequences" << std::endl;
#endif
					offlineCompression();
				}
#ifdef DEBUG_PARSER
				osParser << "\tWe save compression results in files" << std::endl;
#endif
				saveCompression();
#ifdef DEBUG_PARSER
				osParser << "\tWarn compression is done and can be used" << std::endl;
#endif
				compressed = true; // inform ProgAndPlay.cpp the compression is done
#ifdef DEBUG_PARSER
				osParser << "\treset flags" << std::endl;
#endif
				// we reset flags
				executionDetected = false;
				proceed = false;
			}
		}
		CloseResources();
	}
	else
	{
#ifdef DEBUG_PARSER
		osParser << "parsing of traces already launched" << std::endl;
#endif
	}
#ifdef DEBUG_PARSER
	osParser << "End parsing traces" << std::endl;
#endif
}

Trace::sp_trace TracesParser::parseLine(const std::string &s)
{
	Trace *t = NULL;
	std::vector<std::string> tokens = splitLine(s);
	// Gestion des évènements produits par le jeu
	if (tokens[0].compare(GAME_START) == 0)
	{
		TracesParser::mission_name = tokens[1];
	}
	else if (tokens[0].compare(MISSION_START_TIME) == 0)
	{
		t = new StartMissionEvent(TracesParser::mission_name, stoi(tokens[1]));
	}
	else if (tokens[0].compare(MISSION_END_TIME) == 0)
	{
		TracesParser::mission_end_time = stoi(tokens[1]);
	}
	else if (tokens[0].compare(GAME_END) == 0)
	{
		t = new EndMissionEvent(tokens[1], TracesParser::mission_end_time);
	}
	else
	{
		int numTokens = tokens.size();
		int ind = 0;
		// Recherche du token "-". Dans ce cas les tokens suivant représentent les valeurs retournées par l'appel.
		// On réduit donc le nombre de token pour ne compter que les paramètres et ne pas traiter ici les codes de retour.
		std::vector<std::string>::const_iterator it = std::find(tokens.begin(), tokens.end(), "-");
		while (it++ != tokens.end())
			numTokens--;
		bool delayed = false;
		// Gestion des évènements produit par l'éxecution du code du joueur
		if (tokens[ind].compare(DELAYED) == 0)
		{
			delayed = true;
			ind++;
		}
		if (tokens[ind].compare(EXECUTION_START_TIME) == 0)
		{
			TracesParser::execution_start_time = stoi(tokens[ind + 1]);
		}
		else if (tokens[ind].compare(PROGRAMMING_LANGUAGE_USED) == 0)
		{
			// Création de l'évènement
			t = new NewExecutionEvent(TracesParser::execution_start_time, tokens[ind + 1]);
			// Chargement des params en fonction du langage utilisé
			if (TracesParser::params_json.compare("") != 0)
			{
#ifdef DEBUG_PARSER
				osParser << "Params defined, use it to compress and analyse." << std::endl;
#endif
				Call::callMaps.initMaps(TracesParser::params_json, tokens[ind + 1], TracesParser::lang);
			}
			else
			{
#ifdef DEBUG_PARSER
				osParser << "No params defined\nUsing default compression options." << std::endl;
#endif
			}
		}
		else if (tokens[ind].compare(EXECUTION_END_TIME) == 0)
		{
			t = new EndExecutionEvent(stoi(tokens[ind + 1]));
		}
		else if (INCLUDE_EVENTS == 1 && Trace::inArray(tokens[ind].c_str(), Event::concatEventsArr) > -1)
		{
			t = new Event(tokens[ind]);
		}
		else
		{
			// On vérifie si une erreur a été générée
			Call::ErrorType err = Call::getEnumType<Call::ErrorType>(tokens[ind].c_str(), Call::errorsArr);
			if (err != Call::NONE)
			{
				ind++;
			}
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
				t = new CallWithIntParam(err, tokens[ind], p);
			}
			// Gestion des appels prenant en paramètre deux entiers
			else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_INT_PARAMS) == 0)
			{
				int p1 = (ind + 1 <= numTokens - 1) ? stoi(tokens[ind + 1]) : -1;
				int p2 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
				t = new CallWithIntIntParams(err, tokens[ind], p1, p2);
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
				t = new CallWithIntUnitIntParams(err, tokens[ind], p1, unitId, unitType, p2);
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
				t = new CallWithIntUnitParams(err, tokens[ind], p1, unitId, unitType);
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
				t = new CallWithIntIntUnitParams(err, tokens[ind], p1, p2, unitId, unitType);
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
					t = new CallWithUnitParam(err, tokens[ind], unitId, unitType);
				}
				// Gestion des appel prenant une unité et un entier en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_PARAMS) == 0)
				{
					int p = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
					t = new CallWithUnitIntParams(err, tokens[ind], unitId, unitType, p);
				}
				// Gestion des appel prenant une unité et deux entiers en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_INT_PARAMS) == 0)
				{
					int p1 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
					int p2 = (ind + 3 <= numTokens - 1) ? stoi(tokens[ind + 3]) : -1;
					t = new CallWithUnitIntIntParams(err, tokens[ind], unitId, unitType, p1, p2);
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
					t = new CallWithUnitIntUnitIntParams(err, tokens[ind], unitId, unitType, p1, targetId, targetType, p2);
				}
				// Gestion des appel prenant une unité, un entier, une position et un second entier en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_POS_INT_PARAMS) == 0)
				{
					int p1 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
					float x = (ind + 3 <= numTokens - 1) ? stof(tokens[ind + 3]) : -1, y = (ind + 4 <= numTokens - 1) ? stof(tokens[ind + 4]) : -1;
					int p2 = (ind + 5 <= numTokens - 1) ? stoi(tokens[ind + 5]) : -1;
					t = new CallWithUnitIntPosIntParams(err, tokens[ind], unitId, unitType, p1, x, y, p2);
				}
				// Gestion des appel prenant une unité, deux entiers  et un réel en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_FLOAT_INT_PARAMS) == 0)
				{
					int p1 = (ind + 2 <= numTokens - 1) ? stoi(tokens[ind + 2]) : -1;
					float p2 = (ind + 3 <= numTokens - 1) ? stof(tokens[ind + 3]) : -1;
					int p3 = (ind + 4 <= numTokens - 1) ? stoi(tokens[ind + 4]) : -1;
					t = new CallWithUnitIntFloatIntParams(err, tokens[ind], unitId, unitType, p1, p2, p3);
				}
			}
		}
		// Si la trace est valide on définit si c'est un appel en retard et éventuellement les codes de retour en cas d'erreur
		if (t != NULL)
		{
			if (delayed)
				t->setDelayed();
			if (t->isCall())
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
		// Check if the last trace is an event
		if (spt->isEvent())
		{
			// add this event (new trace) at the end of the traces
			root->addTrace(spt);
			add = true;
		}
		else
		{
			// Get the last trace into this last sequence and look for the last Call
			Sequence::sp_sequence parent_seq = root;
			// Check if the last trace is a sequence
			if (root->getTraces().back()->isSequence())
			{
				// While the last trace of this sequence is a sequence, we continue to progress inside
				while (parent_seq->getTraces().back()->isSequence())
					parent_seq = boost::dynamic_pointer_cast<Sequence>(parent_seq->getTraces().back());
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
					Call::sp_call newCall = boost::dynamic_pointer_cast<Call>(spt);
					newCall->filterCall(boost::dynamic_pointer_cast<Call>(lastTrace).get());
					// We create a new sequence to store merge result
					Sequence::sp_sequence new_seq = boost::make_shared<Sequence>(2);
					// Add this new filtered trace into the new sequence
					new_seq->addTrace(newCall);
					// Remove the last trace of the parent sequence and we replace it by the new sequence
					parent_seq->getTraces().pop_back();
					parent_seq->addTrace(new_seq);
				}
				add = true;
			}
		}
	}
	// If no direct compression occurs we add the new trace at the end of traces
	if (!add)
		root->addTrace(spt);
}

typedef std::pair<float, Sequence::sp_sequence> ScoredRoot;

bool sortFunction (ScoredRoot a, ScoredRoot b) { return (a.first>b.first); }

float computeScore(std::vector<Trace::sp_trace> & pattern, int nbAlign, int nbOpt){
	//return ((float)(nbAlign*nbAlign) / (nbAlign+nbOpt));
	return ((float)(nbAlign) / (nbAlign+nbOpt));
	//return ((float)(nbAlign) / (rootMainPos+nbAlign+nbOpt+pattern.size()-Sequence::getEndPosOfLinearSequence(pattern, 0)));
}

typedef struct {
	// pattern stocké sous une forme linéarisée
	std::vector<Trace::sp_trace> pattern;
	// position nous permettant de connaître la position d'avancement dans le pattern
	int position;
	// compteur d'alignement nous permettant de connaître dans le pattern combien de traces ont pu être alignées
	int alignCount;
	// compteur d'option nous permettant de connaître dans le pattern combien de traces optionnelles ont été définies
	int optCount;
	// score du pattern
	float score;
} Scenario;

// clone patterns[source] et l'insère à la positione patterns[target]
// source doit être inclus dans [0, patterns.size()[
// si target < 0 || target > patterns.size() => ajout du clone en fin de vecteur
void duplicatePattern (std::vector<Scenario> & patterns, int source, int target){
	// s'assurer que les indice source et target sont bien dans les limites du vecteur
	assert (source >= 0 && source < (signed)patterns.size());
	target = (target < 0 || target > (signed)patterns.size()) ? patterns.size() : target;
	Scenario model = patterns[source];
	Scenario clone;
	clone.pattern = Sequence::cloneLinearSequence(model.pattern);
	clone.position = model.position;
	clone.alignCount = model.alignCount;
	clone.optCount = model.optCount;
	clone.score = -1;
	patterns.insert(patterns.begin()+target, clone);
}

void addNewPattern (std::vector<Scenario> & patterns, std::vector<Trace::sp_trace> & pattern){
	Scenario s;
	s.pattern = pattern;
	s.position = 0;
	s.alignCount = 0;
	s.optCount = 0;
	s.score = -1;
	patterns.push_back(s);
}

void moveToNextCall(std::vector<Scenario> & patterns){
	// Note :
	//  - lorsque'on atteint une fin de séquence il faut proposer deux scénarios (=> duplication du pattern en question) à savoir recommencer la séquence ou continuer sur la prochaine trace du pattern
	//  - lorsqu'on atteint un début de séquence optionnelle il faut aussi proposer deux scénarios à savoir accéder à la première trace de la séquence ou sauter la séquence pour se positionner directement sur la prochaine trace suivant la séquence
	// Parcourir tous les patterns pour se positionner sur le prochain Call
	for (int i = 0 ; i < (signed)patterns.size() ; i++)
	{
		// Incrémenter de 1 l'indice de parcours du pattern courrant
		patterns[i].position++;
		// Si on a dépassé la fin du pattern courrant
		if ((signed)patterns[i].pattern.size() <= patterns[i].position)
		{
			// rien à faire, on est à la fin du pattern, on reste à la fin du pattern
			// On recalle quand mmême la position à la taille du pattern
			patterns[i].position = patterns[i].pattern.size();
		}
		else
		{
			// Si on est sur une fin de séquence il faut dupliquer le pattern pour dans un cas se repositionner au début de la séquence et dans l'autre cas passer à la trace suivante
			if (patterns[i].pattern[patterns[i].position]->isSequence() && patterns[i].pattern[patterns[i].position]->getInfo().compare("End") == 0)
			{
				// Dans tous les cas on note qu'on a fait une itération supplémentaire. Comme ici on est en fin de seéquence il faut retrouver la position du début de la séquence
				int startSeq = Sequence::getBeginPosOfLinearSequence(patterns[i].pattern, patterns[i].position);
				boost::dynamic_pointer_cast<Sequence>(patterns[i].pattern[startSeq])->addOne();

				// Cas 1 : Cloner le pattern en fin de vecteur et positionner l'indice de ce clone sur la position de la fin de la séquence, ainsi l'indice sera augmenté de 1 lorsque ce pattern sera évalué et sera donc positionné sur la trace suivant immédiatement cette séquence
				duplicatePattern(patterns, i, -1);

				// Cas 2 : repositionner le pattern courant au début de la séquence
				// Récupérer l'indice du début de la séquence
				int beginSeqPos = Sequence::getBeginPosOfLinearSequence(patterns[i].pattern, patterns[i].position);
				if (beginSeqPos != -1)
				{
					// Se repositionner au début de la séquence (le cas où la première trace est une séquence est géré dans la boucle juste après)
					patterns[i].position = beginSeqPos;
					boost::dynamic_pointer_cast<Sequence>(patterns[i].pattern[beginSeqPos])->newIter = true; // noter que cette séquence est en reprise d'itération
				}
			}
			// Tantqu'on est sur un début de séquence, on progresse dans cette séquence jusqu'à tomber sur un Call
			while (patterns[i].pattern[patterns[i].position]->isSequence() && patterns[i].pattern[patterns[i].position]->getInfo().compare("Begin") == 0)
			{
				// Si on tombe sur une séquence optionnelle il faut dupliquer le pattern pour gérer le cas où on la saute
				if (patterns[i].pattern[patterns[i].position]->isOptional())
				{
					// Récupération de l'indice de fin de la séquence
					int endSeqPos = Sequence::getEndPosOfLinearSequence(patterns[i].pattern, patterns[i].position);
					if (endSeqPos > -1)
					{
						// Cloner le pattern en fin de vecteur
						duplicatePattern(patterns, i, -1);
						// On corrige l'avancement de ce clone sur la position de la fin de cette séquence clonée, ainsi l'indice sera augmenté de 1 lorsque ce pattern sera évalué et sera donc positionné sur la trace suivant immédiatement cette séquence clonée (elle sera donc bien ignorée)
						patterns[patterns.size()-1].position = endSeqPos;
					}
				}
				// Pour la trace non clonnée, se positionner sur la première trace de cette séquence (si cette trace est elle même une séquence, elle sera traitée par le prochain tour de boucle)
				patterns[i].position++;
			}
		}
	}
}

void addNewCall(const Trace::sp_trace & rootCall, float maxRatio, std::vector<Scenario> & patterns, int startPatternsId = 0){
	// Parcourir tous les patterns à partir de l'indice startPatternsId.
	for (int i = startPatternsId ; i < (signed)patterns.size() ; i++)
	{
		// Vérifier si on n'a pas dépassé la fin du pattern
		if (patterns[i].position >= (signed)patterns[i].pattern.size())
		{
			// Si on a dépassé la fin du pattern courrant il faut vérifier plusieurs choses :
			//  1 - si le Call courrant est identique au premier Call du pattern et que des traces ont commencé à être accumulées à la fin du pattern on va se repositionner au début du pattern et insérer les traces accumulées comme optionnelles (on prend soin de simuler le nouveau score pour le pas faire ça pour rien)
			//      Exemple on veut passer de [Sd C_i C_d Sf C_a C_e C_i] à [Sd C_i C_d opt(C_A) opt(C_e) Sf]
			//		Cas particulier, on ne gère pas ici le cas [Sd C_i C_d Sf C_i] où il n'y a pas encore de traces qui ont été accumulées à la fin du pattern car ce cas est déjà géré lorsqu'on atteind la fin de la séquence (Sf)
			//  2 - ajouter ce Call à la fin du pattern

			// Cas 1 : Si le premier Call du pattern == au Call en cours d'analyse du root et traces accumulées en fin de pattern et score simulé toujours intéressant
			int firstCallPos = Sequence::getCallPosInLinearSequence(patterns[i].pattern, 0);
			int endSeqPos = Sequence::getEndPosOfLinearSequence(patterns[i].pattern, 0);
			if (firstCallPos != -1 &&
				patterns[i].pattern[firstCallPos]->operator==(rootCall.get()) && // premier call du pattern == call du root
				endSeqPos < (signed)patterns[i].pattern.size()-1 && computeScore(patterns[i].pattern, patterns[i].alignCount+1, patterns[i].optCount+patterns[i].pattern.size()-1-endSeqPos) > maxRatio - SCORE_TOLERENCE // simulation du score en comptant un alignement supplémentaire et autant d'options que de traces accumulées à la fin du pattern qui seront intégrées dans ce dernier en option
			)
			{
				// Intégrer toutes les traces accumulées après la séquence mère du pattern à l'intérieur de la séquence mère et les mettre en optionnelle
				// Passer de [Sd C C Sf C C] à [Sd C C opt(C) opt(C) Sf]
				int endSeqPos = Sequence::getEndPosOfLinearSequence(patterns[i].pattern, 0);
				if (endSeqPos > -1)
				{
					// Insérer un clone dans le vecteur de patterns AVANT le modèle
					duplicatePattern(patterns, i, i);

					// IMPORTANT : Maintenant en i se trouve le clone et en i+1 le modèle

					// enregistrer la trace de fin de séquence
					Trace::sp_trace lastEndSeq = patterns[i].pattern[endSeqPos];
					// décaller d'une position vers la gauche toutes les traces et les passer en optionnelle (si elles ne le sont pas déjà)
					for (int j = endSeqPos ; j < (signed)patterns[i].pattern.size()-1 ; j++)
					{
						patterns[i].pattern[j] = patterns[i].pattern[j+1];
						if (!patterns[i].pattern[j]->isOptional())
						{
							patterns[i].pattern[j]->setOptional(true);
							patterns[i].optCount++;
						}
					}
					// remplacer la dernière trace (qui a été décallé juste avant) par la fin de séquence
					patterns[i].pattern[patterns[i].pattern.size()-1] = lastEndSeq;
					// Se repositionner sur le premier Call du pattern
					patterns[i].position = firstCallPos;
					// Augmenter de 1 le nombre de trace alignée dans le vecteur de compteur des alignements
					patterns[i].alignCount++;
					
					// incrémenter i pour se repositionner sur le modèle
					i++;
				}
			}

			// Cas 2 : ajouter le Call courant (root) à la fin du pattern
			// Ici on stoque la trace à l'extérieur du pattern, pas la peine de noter ces traces là comme optionnelles. Si elles seront réintégrées dans le pattern, c'est à ce moment là qu'elles passeront optionnelles
			patterns[i].pattern.push_back(rootCall->clone());
			patterns[i].position++;
		}
		else
		{
			// Ici on est toujours à l'intérieur du pattern, il faut maintenant voir si le Call du pattern est alignable avec celui du root
			// Si le Call courrant du pattern == au Call du root
			if (patterns[i].pattern[patterns[i].position]->operator==(rootCall.get()))
			{
				// Cas idéal => augmenter de 1 le nombre d'alignement de ce pattern dans le vecteur de compteur d'alignement
				patterns[i].alignCount++;
				// Si on est sur un début de séquence, réinitialiser le flag qui permet de savoir si on est en train de tourner dans une boucle
				if (patterns[i].pattern[patterns[i].position-1]->isSequence() && patterns[i].pattern[patterns[i].position-1]->getInfo().compare("Begin") == 0){
					boost::dynamic_pointer_cast<Sequence>(patterns[i].pattern[patterns[i].position-1])->newIter = false;
					// Propager ça sur toutes les séquences dans lesquelles on est imbriqué
					int parentSequenceId = Sequence::getBeginPosOfLinearSequence(patterns[i].pattern, patterns[i].position-1);
					while (parentSequenceId != -1){
						boost::dynamic_pointer_cast<Sequence>(patterns[i].pattern[parentSequenceId])->newIter = false;
						parentSequenceId = Sequence::getBeginPosOfLinearSequence(patterns[i].pattern, parentSequenceId-1);
					}
				}
				// Fusionner les deux appels
				dynamic_cast<Call *>(patterns[i].pattern[patterns[i].position].get())->filterCall(dynamic_cast<const Call *>(rootCall.get()));
			}
			else
			{
				// Ici les deux call ne sont pas alignable, l'un des deux doit être mis en option

				// Vérifier que le score resterait intéressant malgrès l'ajout d'une nouvelle option
				if (computeScore(patterns[i].pattern, patterns[i].alignCount, patterns[i].optCount+1) > maxRatio - SCORE_TOLERENCE){

					// CAS 1 : Possibilité de mettre le Call du root en option
					
					// On commence par vérifier si la trace précédente est un début de séquence
					if (patterns[i].pattern[patterns[i].position-1]->isSequence() && patterns[i].pattern[patterns[i].position-1]->getInfo().compare("Begin") == 0){
						// Ici on est sur un début de séquence et les deux calls sont différents. On doit donc générer plusieurs scénarios qui dépendent du contexte.
						//  (a) Si on avait atteint une fin de séquence dans le pattern et qu'on est revenu au début de la séquence (cf ETAPE 1 lors de la progression dans chaque pattern). Dans ce cas il faut générer deux scénarios : un pour insérer l'option en début de séquence et un autre pour l'insérer en fin de séquence.
						//  (b) Si on était en train de progresser dans le pattern et qu'on était arrivé sur un début de séquence, alors on a plongé dans cette séquence (et éventuelement ses sous-séquences) jusqu'à tomber sur un Call. Danc ce cas là, il faut générer plusieurs scénarios : un pour insérer l'option en début de séquence avant le Call mais aussi autant que necessaire pour insérer l'option avant chaque début de séquence imbriquées. Attention, l'un de ces scénarios pourrait remonter sur une séquence correspondant au cas (a), dans ce cas, c'est un cas d'arrêt. 

						// => Dans ces deux possibilités un scénario consiste à insérer l'option en début de séquence (juste avant le call du pattern) ce qui correspond aussi au cas où on n'aurait pas été en début de séquence. Ce cas est traité juste à la sortie de ce "if"

						int savePos = patterns[i].position; // on sauvegarde notre position dans le pattern pour pouvoir le rétablir après le traitement de la boucle
						bool newLoop = false;
						do{
							// Insérer un clone dans le vecteur de patterns AVANT le modèle
							duplicatePattern(patterns, i, i);
							
							// IMPORTANT : Maintenant en i se trouve le clone et en i+1 le modèle

							// On teste pour vérifier dans quel contexte on est
							if (boost::dynamic_pointer_cast<Sequence>(patterns[i].pattern[patterns[i].position-1])->newIter){
								// cas (a)
								
								// Se positionner à la fin de la séquence du clone
								patterns[i].position = Sequence::getEndPosOfLinearSequence(patterns[i].pattern, patterns[i].position);
								// Insérer dans le clone un clone du Call du root (donc à la fin de la séquence)
								patterns[i].pattern.insert (patterns[i].pattern.begin()+patterns[i].position, rootCall->clone());
								// Indiquer ce clone du Call du root comme optionnel
								patterns[i].pattern[patterns[i].position]->setOptional(true);
								// Se repositionner sur la fin de la séquence
								patterns[i].position++;
								// Augmenter de 1 le nombre d'option de ce pattern dans le vecteur de compteur des options
								patterns[i].optCount++;
								// stopper la boucle
								newLoop = false;
							} else {
								// cas (b)

								// Insérer dans le clone un clone du Call du root AVANT le début de la séquence (donc à l'extérieur de la séquence)
								patterns[i].pattern.insert (patterns[i].pattern.begin()+patterns[i].position-1, rootCall->clone());
								// Indiquer ce clone du Call du root comme optionnel
								patterns[i].pattern[patterns[i].position-1]->setOptional(true);
								// Augmenter de 1 le nombre d'option de ce pattern dans le vecteur de compteur des options
								patterns[i].optCount++;

								// Dans le modèle (donc en i+1) ou souhaite vérifier si la trace en amont est un début de séquence (sauf si la première car on ne souhaite pas insérer de Call en amont du scénario). Si oui, autoriser une nouvelle itération pour insérer le clone de Call du root en amont de ce début de séquence.
								// Donc il faut se positionner sur la trace en amont dans le modèle
								// Rq : on est en train de remonter le pattern et tant qu'on tombe sur des début de séquence on refait le processus (cloner et insérer la trace du root en option)
								patterns[i+1].position--;

								// Vérifer s'il faut refaire un tour de boucle
								if (patterns[i+1].position-1 > 0 &&	patterns[i+1].pattern[patterns[i+1].position-1]->isSequence() && patterns[i+1].pattern[patterns[i+1].position-1]->getInfo().compare("Begin") == 0)
									newLoop = true;
							}

							// incrémenter i pour se repositionner sur le modèle
							i++;
						} while (newLoop);
						
						// En sortant de cette boucle cela signifie que la trace actuelle du modèle n'est pas un début de séquence. Il faut donc revenir sur la position courrante
						patterns[i].position = savePos;
					}
					
					// Dans tous les cas on ajoute le call du root en option juste avant le call du pattern
					// on céée un nouveau clone pour pouvoir gérer le Cas 2 (call du pattern en option)
					duplicatePattern(patterns, i, i);
							
					// IMPORTANT : Maintenant en i se trouve le clone et en i+1 le modèle

					// Ajout du call du root en option juste avant le call du pattern
					patterns[i].pattern.insert (patterns[i].pattern.begin()+patterns[i].position, rootCall->clone());
					// Indiquer ce clone du Call du root comme optionnel
					patterns[i].pattern[patterns[i].position]->setOptional(true);
					// Augmenter de 1 le nombre d'option de ce pattern dans le vecteur de compteur des options
					patterns[i].optCount++;

					// On se repostionne sur le modèle
					i++;
					
					// CAS 2 : mettre le Call du pattern en option.
					// Si la trace courrante du pattern n'est pas une option
					if (!patterns[i].pattern[patterns[i].position]->isOptional())
					{
						// Indiquer la trace courante du pattern comme optionnel
						patterns[i].pattern[patterns[i].position]->setOptional(true);
						// Augmenter de 1 le nombre d'option de ce pattern dans le vecteur de compteur des options
						patterns[i].optCount++;
					}
					// Avancer dans ce pattern pour prendre en compte la mise en option du Call du pattern
					patterns[i].position++;
					// Problème ici, il ne faut pas avancer dans les traces tant qu'on n'a pas réussi à positionner la trace courante. On fait donc un appel récursif pour tenter de positionner la trace courrante
					addNewCall(rootCall, maxRatio, patterns, i);
					// Au retour de cet appel, tous les patterns ont été traités. on peut donc sortir direct de la boucle
					break;
				} else {
					// Supprimer ce modèle qui ne mènera à rien
					patterns.erase(patterns.begin()+i);
					// comme on vient de supprimer un pattern alors qu'on parcours en même temps les vecteurs il faut revenir en arrière d'une position
					i--;
				}
			}
		}
	}
}

float buildPatterns(Sequence::sp_sequence & root, int rootMainPos, float maxRatio, std::vector<Scenario> & patterns){
	// Parcourir toute la fin du root
	for (int rootDownPos = rootMainPos ; rootDownPos < (int)root->size() ; rootDownPos++)
	{
std::cout << rootDownPos << "/" << root->size() << std::endl;
		///////////////////////////////////////////////////////////
		// ETAPE 1 : Faire avancer chaque pattern d'une position //
		///////////////////////////////////////////////////////////

		moveToNextCall(patterns);

		// Ici pour chaque pattern on a soit atteint la fin, soit on est positionné sur un Call

		//////////////////////////////////////////////////////////////
		// ETAPE 2 : Intégrer la trace courante dans chaque pattern //
		//////////////////////////////////////////////////////////////

		addNewCall(root->at(rootDownPos), maxRatio, patterns);
		
		// Calcul du score de chaque pattern
		for (int i = 0 ; i < (signed)patterns.size() ; i++){
//std::cout << "B " << computeScore (rootMainPos, patterns[i].pattern, patterns[i].alignCount, patterns[i].optCount) << " " << patterns[i].alignCount << " " << patterns[i].optCount << std::endl;
			patterns[i].score = computeScore (patterns[i].pattern, patterns[i].alignCount, patterns[i].optCount); // mise à jour du score
			if (patterns[i].score > maxRatio){
				maxRatio = patterns[i].score;
			}
		}
		// Suppression de tous les patterns qui ne pourront pas rattraper le meilleur score
		for (int i = (signed)patterns.size()-1 ; i >= 0 ; i--){
			if (patterns[i].score < maxRatio - SCORE_TOLERENCE){ // TODO : pouvoir paramétrer ce seuil
				patterns.erase(patterns.begin()+i);
			}
		}
	}
	return maxRatio;
}

void TracesParser::offlineCompression()
{
	std::vector<ScoredRoot> roots;
	roots.push_back(ScoredRoot(0, boost::dynamic_pointer_cast<Sequence>(root->clone())));

	std::vector <Trace::sp_trace> bestPattern;

	for (int r = 0 ; r < (signed)roots.size() ; r++)
	{
		Sequence::sp_sequence root = roots[r].second;
		float maxRatio = roots[r].first;

//			std::cout << "--- NEW PASS ---" << std::endl;
//			root->exportAsCompressedString(std::cout);
//			std::cout << std::endl;

		// Recherche du point de départ effectif => le premier Call du root
		int startingPos = start;
		while (startingPos < (int)root->size())
			if (!root->at(startingPos)->isCall())
				startingPos++;
			else
				break;

		// On parcours toute la trace
		for (int currentPos = startingPos; currentPos < (int)root->size(); currentPos++)
		{
			// récupération de la trace courante
			Trace::sp_trace currentTrace = root->at(currentPos);
			// Si la trace courante est un event, on stoppe l'annalyse (ici c'est forcement un event non alignable vue que les autres ont été filtrés lors du parsage des logs)
			if (currentTrace->isEvent())
				break;
			// Rechercher des accroches en amont (une trace égale au Call courrant). Cette accroche peut être soit directement un Call du root soit le premier Call d'une séquence du root
			// Donc on remonte la trace pour essayer de trouver cette accroche
			for (int prevPos = currentPos-1 ; prevPos >= startingPos ; prevPos--)
			{
				Sequence::sp_sequence up_seq;
				// Cas où on trouve une accroche directement dans root
				if (root->at(prevPos)->isCall() && root->at(prevPos)->operator==(currentTrace.get()))
				{
					// on souhaite une distance minimale de taille 2 (pour éviter de fusionner des Calls successifs s'il n'y en pas au moins 3)
					if (currentPos-prevPos >= 2)
					{
						// On construit une séquence contenant les traces comprises dans [prevPos, currentPos[
						up_seq = root->getSubSequence(prevPos, currentPos);
					}
				}
				// Cas où on trouve une accroche pour le premier Call d'une séquence
				else if (root->at(prevPos)->isSequence() && boost::dynamic_pointer_cast<Sequence>(root->at(prevPos))->getFirstCall()->operator==(currentTrace.get()))
				{
					// si la séquence en question précède immediatement currentPos, on peut directement utiliser cette séquence
					if (currentPos-prevPos == 1)
						up_seq = boost::dynamic_pointer_cast<Sequence>(root->at(prevPos)->clone());
					else
						// sinon on construit une séquence contenant les traces comprises dans [prevPos, currentPos[
						up_seq = root->getSubSequence(prevPos, currentPos);
				}
				if (up_seq)
				{
					// Ici dans up_seq on a toutes les traces comprises entre l'accroche et la position courante
					// Vecteur de patterns (ces différents patterns vont nous permettre d'envisager différentes options pour intégrer chaque trace dans les différents scénarios)
					std::vector<Scenario> patterns;
					// ajout d'un premier pattern correspondant au up
					addNewPattern(patterns, up_seq->getLinearSequence());
					
					// On va maintenant descendre dans les traces du root courrant en partant de currentPos pour essayer d'intégrer chaque traces aux différents patterns (et produire de nouveaux patterns si besoin)
					maxRatio = buildPatterns(root, currentPos, maxRatio, patterns);

					// Maintenant qu'on a terminé de parcourir la trace pour cette acrroche, on créé autant de nouveaux roots que de patterns concervés pour poursuivre leur exploration
					for (int i = 0 ; i < (signed)patterns.size() ; i++)
					{
						// On ignore tous les patterns qui contiendraient des calls non optionnelles au delà de la position du dernier ajout. En effet si le dernier call du root a été intégré en plein milieu d'un scénario et que ce scénario inclus dans la suite des calls obligatoires (non optionnels), celà signifierait que ce pattern produirait des calls au dela de la dernière trace du root... Ce qui n'est pas possible vue que le root est terminé, donc ce pattern est faux
						if (Sequence::getNonOptCallInLinearSequence(patterns[i].pattern, patterns[i].position) > 0)
							continue;
						roots.push_back(ScoredRoot(patterns[i].score, boost::dynamic_pointer_cast<Sequence>(root->clone())));
						// Incrustation du pattern dans le clone
						roots[roots.size()-1].second->getTraces().erase(roots[roots.size()-1].second->getTraces().begin()+prevPos, roots[roots.size()-1].second->getTraces().end());
						roots[roots.size()-1].second->insertLinearSequence(patterns[i].pattern, prevPos);
					}
				}
			}
		}

osParser << r << " " << roots.size() << " " << maxRatio << std::endl;
	}

	std::sort(roots.begin(), roots.end(), sortFunction);
	osParser << "BEST SOLUTION !!!!" << std::endl;
	for (int i = 0 ; i < (signed)roots.size() ; i++){
		osParser << roots[i].first << " ";
		roots[i].second->exportAsCompressedString(osParser);
	}

/*	// Recherche du point de départ effectif => le premier Call du root
	while (start < (int)root->size())
		if (root->at(start)->isEvent())
			start++;
		else
			break;

#ifdef DEBUG_PARSER
	osParser << "\tWe try to aggregate successive sequences from " << start << std::endl;
#endif
	root->findAndAggregateSuccessiveSequences(start);
#ifdef DEBUG_PARSER
	root->exportAsString(osParser);
#endif

root->exportAsString(std::cout);

	bool improvement;
	do
	{
#ifdef DEBUG_PARSER
		osParser << "\tWe try to find and process optional tokens" << std::endl;
#endif
		improvement = root->findAndProcessOptionalTokens(start);
#ifdef DEBUG_PARSER
		root->exportAsString(osParser);
#endif
	} while (improvement);

std::cout << "AFTER OPTIMISATION" << std::endl;
root->exportAsString(std::cout);

#ifdef DEBUG_PARSER
	osParser << "\tFinal result" << std::endl;
	root->exportAsString(osParser);
#endif*/
}

void TracesParser::exportTracesAsString(std::ostream &os)
{
	int num_start = 0;
	Event *e = NULL;
	for (unsigned int i = 0; i < root->size(); i++)
	{
		e = root->at(i)->isEvent() ? dynamic_cast<Event *>(root->at(i).get()) : NULL;
		if (e != NULL && Trace::inArray(e->getLabel().c_str(), Event::noConcatEventsArr) > -1)
		{
			if (e->getLabel().compare(MISSION_START_TIME) == 0)
			{
				if (num_start++ > 0)
					os << std::endl;
				StartMissionEvent *sme = dynamic_cast<StartMissionEvent *>(e);
				os << GAME_START << " " << sme->getMissionName() << std::endl;
				os << MISSION_START_TIME << " " << sme->getStartTime() << std::endl;
			}
			else if (e->getLabel().compare(MISSION_END_TIME) == 0)
			{
				EndMissionEvent *eme = dynamic_cast<EndMissionEvent *>(e);
				os << "status " << eme->getStatus() << std::endl;
				os << MISSION_END_TIME << " " << eme->getEndTime() << std::endl;
			}
			else if (e->getLabel().compare(EXECUTION_START_TIME) == 0)
			{
				NewExecutionEvent *nee = dynamic_cast<NewExecutionEvent *>(e);
				os << "\t" << EXECUTION_START_TIME << " " << nee->getStartTime() << std::endl;
				os << "\t" << PROGRAMMING_LANGUAGE_USED << " " << nee->getProgrammingLangageUsed() << std::endl;
				e->numTab = 1;
			}
			else if (e->getLabel().compare(EXECUTION_END_TIME) == 0)
			{
				os << "\t" << EXECUTION_END_TIME << " " << dynamic_cast<EndExecutionEvent *>(e)->getEndTime() << std::endl;
				e->numTab = 0;
			}
		}
		else
			root->at(i)->exportAsString(os);
	}
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
			if (node_name.compare("mission") == 0 && node->first_attribute("end_time") != 0)
			{
				traces.push_back(boost::make_shared<EndMissionEvent>(node->first_attribute("status")->value(), atoi(node->first_attribute("end_time")->value())));
			}
			else if (node_name.compare("execution") == 0 && node->first_attribute("end_time") != 0)
			{
				traces.push_back(boost::make_shared<EndExecutionEvent>(atoi(node->first_attribute("end_time")->value())));
			}
			else if (node_name.compare("sequence") == 0)
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
			if (node_name.compare("mission") == 0)
			{
				traces.push_back(boost::make_shared<StartMissionEvent>(node->first_attribute("name")->value(), atoi(node->first_attribute("start_time")->value())));
				node_stack.push(node);
				node = node->first_node();
			}
			else if (node_name.compare("execution") == 0)
			{
				traces.push_back(boost::make_shared<NewExecutionEvent>(atoi(node->first_attribute("start_time")->value()), node->first_attribute("programming_language_used")->value()));
				node_stack.push(node);
				node = node->first_node();
			}
			else if (node_name.compare("sequence") == 0)
			{
				std::string info;
				if (node->first_attribute("info") != 0)
					info = node->first_attribute("info")->value();
				bool num_fixed = false;
				if (node->first_attribute("nb_iteration_fixed") != 0)
					num_fixed = std::string(node->first_attribute("nb_iteration_fixed")->value()).compare("true") == 0;
				sps = boost::make_shared<Sequence>(info, num_fixed);
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

void TracesParser::setEnd()
{
	end = true;
}

bool TracesParser::compressionDone()
{
	if (compressed)
	{
		compressed = false;
		return true;
	}
	return false;
}

void TracesParser::setProceed(bool proceed)
{
	this->proceed = proceed;
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
		res = boost::lexical_cast<int>(s);
	}
	catch (const boost::bad_lexical_cast &)
	{
#ifdef DEBUG_PARSER
		osParser << "error boost::lexical_cast<int>(" << s << ")" << std::endl;
#endif
		std::cout << "error boost::lexical_cast<int>(" << s << ")" << std::endl;
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
		res = boost::lexical_cast<float>(s);
	}
	catch (const boost::bad_lexical_cast &)
	{
#ifdef DEBUG_PARSER
		osParser << "error boost::lexical_cast<float>" << std::endl;
#endif
		std::cout << "error boost::lexical_cast<float>" << std::endl;
		exit(EXIT_FAILURE);
	}
	return res;
}
