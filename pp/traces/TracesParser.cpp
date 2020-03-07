#include "TracesParser.h"
#include "TraceConstantList.h"
#include "Scenario.h"
#include <cerrno>
#include <utility>

/**
  * Doit être mis à 1 pour prendre en compte les événements de Event::concatEventsArr rencontrés lors du parsage de fichier de traces brutes.
  */
#define INCLUDE_EVENTS 0

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

class ScoredSequence {
	public:
		Scenario::sp_scenario scenario;
		Sequence::sp_sequence sequence;
		
		ScoredSequence(Sequence::sp_sequence newRoot){
			sequence = newRoot;
			scenario = boost::make_shared<Scenario>(newRoot->getLinearSequence(), 0, 0);
		};
		
		ScoredSequence(Scenario::sp_scenario newscenario, Sequence::sp_sequence newsequence){
			scenario = newscenario;
			sequence = newsequence;
		};
};

bool sortFunction (ScoredSequence a, ScoredSequence b) { return (a.scenario->score > b.scenario->score); }
bool sortFunction2 (Scenario::sp_scenario a, Scenario::sp_scenario b) { return (a->score > b->score); }

void TracesParser::offlineCompression()
{
	std::vector<ScoredSequence> roots;

	roots.push_back(ScoredSequence(boost::dynamic_pointer_cast<Sequence>(root->clone())));

	std::vector <Trace::sp_trace> bestPattern;

	for (int r = 0 ; r < (signed)roots.size() ; r++)
	{
		Sequence::sp_sequence rootSequence = roots[r].sequence;
		float maxRatio = roots[r].scenario->score;

		// Vecteur de patterns (ces différents patterns vont nous permettre d'envisager différentes options pour intégrer chaque trace)
		std::vector<Scenario::sp_scenario> patterns;

/*		std::cout << "--- NEW PASS ---" << std::endl;
		scenario->exportAsCompressedString(std::cout);
		std::cout << std::endl;*/

		// Recherche du point de départ effectif => le premier Call du root non intégré dans une séquence
		int startingPos = start;
		while (startingPos < (int)rootSequence->size())
			if (!rootSequence->at(startingPos)->isCall())
				startingPos++;
			else
				break;

int statsMove = 0;
int statsAdd = 0;

		// On parcours toute la trace à partir du premier Call trouvé
		for (int currentPos = startingPos; currentPos < (int)rootSequence->size(); currentPos++)
		{
/*if (currentPos == (int)rootSequence->size()-2){
	std::cout << "Debut traitement : ";
	rootSequence->at(currentPos)->exportAsCompressedString(std::cout);
	std::cout << std::endl;
	for (int i = 0 ; i < (signed)patterns.size() ; i++){
		std::cout << patterns[i]->position << " ";
		Sequence::exportLinearSequenceAsString(patterns[i]->pattern, std::cout);
	}
	std::cout << std::endl;
}*/
			// récupération de la trace courante
			Trace::sp_trace currentTrace = rootSequence->at(currentPos);
			// Si la trace courante est un event, on stoppe l'annalyse (ici c'est forcement un event non alignable vue que les autres ont été filtrés lors du parsage des logs)
			if (currentTrace->isEvent())
				break;

			// Rechercher des accroches en amont (une trace égale au Call courrant). Cette accroche peut être soit directement un Call du root soit le premier Call d'une séquence du root
			// Donc on remonte la trace pour essayer de trouver ces accroches
			for (int upPos = currentPos-1 ; upPos >= startingPos ; upPos--)
			{
				Sequence::sp_sequence up_seq;
				// Cas où on trouve une accroche directement dans root
				if (rootSequence->at(upPos)->isCall() && rootSequence->at(upPos)->operator==(currentTrace.get()))
				{
					// on souhaite une distance minimale de taille 2 (pour éviter de fusionner des Calls successifs s'il n'y en pas au moins 3)
					if (currentPos-upPos >= 2)
					{
						// On construit une séquence contenant les traces comprises dans [upPos, currentPos[
						up_seq = rootSequence->getSubSequence(upPos, currentPos);
					}
				}
				// Cas où on trouve une accroche pour le premier Call d'une séquence
				else if (rootSequence->at(upPos)->isSequence() && boost::dynamic_pointer_cast<Sequence>(rootSequence->at(upPos))->getFirstCall()->operator==(currentTrace.get()))
				{
					// si la séquence en question précède immediatement currentPos, on peut directement utiliser cette séquence
					if (currentPos-upPos == 1)
						up_seq = boost::dynamic_pointer_cast<Sequence>(rootSequence->at(upPos)->clone());
					else
						// sinon on construit une séquence contenant les traces comprises dans [upPos, currentPos[
						up_seq = rootSequence->getSubSequence(upPos, currentPos);
				}
				if (up_seq)
				{
					// Ici dans up_seq on a toutes les traces comprises entre l'accroche et la position courante
					// ajout du pattern correspondant au up
					patterns.push_back(boost::make_shared<Scenario>(up_seq->getLinearSequence(), rootSequence->getSubSequence(startingPos, upPos)->length(), upPos));
				}
			}

			// Positionner chaque pattern sur son prochain Call
			std::vector<Scenario::sp_scenario> new_patterns;
			for (int i = 0 ; i < (signed)patterns.size() ; i++){
				std::vector<Scenario::sp_scenario> results = patterns[i]->simulateMoveToNextCall();
				new_patterns.insert(new_patterns.end(), results.begin(), results.end());
statsMove += results.size();
			}
			patterns = std::move(new_patterns);

/*if (currentPos == (int)rootSequence->size()-2){
	std::cout << "Après le MOVE : ";
	rootSequence->at(currentPos)->exportAsCompressedString(std::cout);
	std::cout << std::endl;
	for (int i = 0 ; i < (signed)patterns.size() ; i++){
		std::cout << patterns[i]->position << " ";
		Sequence::exportLinearSequenceAsString(patterns[i]->pattern, std::cout);
	}
	std::cout << std::endl;
}*/

			// Calcul de nombre maximum d'alignement
			int maxAligned;
			if (patterns.size() > 0){
				maxAligned = patterns[0]->alignCount;
				for (int i = 1 ; i < (signed)patterns.size() ; i++){
					if (patterns[i]->alignCount > maxAligned){
						maxAligned = patterns[i]->alignCount;
					}
				}
			} else
				maxAligned = 0;

			// calcul de la trace la plus compressée
			int minLength;
			if (patterns.size() > 0){
				bool considerUpCount = true;
				minLength = patterns[0]->pattern.size()+(considerUpCount ? patterns[0]->upCount : 0);
				for (int i = 1 ; i < (signed)patterns.size() ; i++){
					if ((signed)patterns[i]->pattern.size()+(considerUpCount ? patterns[i]->upCount : 0) < minLength)
						minLength = patterns[i]->pattern.size()+(considerUpCount ? patterns[i]->upCount : 0);
				}
			}

			// Intégration du Call courrant du root dans chaque pattern
			std::vector<Scenario::sp_scenario> results;
			// Parcourir tous les patterns
			for (int i = 0 ; i < (signed)patterns.size() ; i++){
				std::vector<Scenario::sp_scenario> res = patterns[i]->simulateNewCallIntegration(currentTrace, maxRatio, minLength, &maxAligned, patterns[i]->position);
				results.insert(results.end(), res.begin(), res.end());
statsAdd += res.size();
			}
			patterns = std::move(results);

/*if (currentPos == (int)rootSequence->size()-2){
	std::cout << "Après le ADD : ";
	rootSequence->at(currentPos)->exportAsCompressedString(std::cout);
	std::cout << std::endl;
	for (int i = 0 ; i < (signed)patterns.size() ; i++){
		std::cout << patterns[i]->position << " ";
		Sequence::exportLinearSequenceAsString(patterns[i]->pattern, std::cout);
	}
	std::cout << std::endl;
}*/

			// Calcul du score de chaque pattern et mise à jour du maxRatio
			maxRatio = 0;
			for (int i = 0 ; i < (signed)patterns.size() ; i++){
//std::cout << "B " << computeScore (rootMainPos, patterns[i]->pattern, patterns[i]->alignCount, patterns[i]->optCount) << " " << patterns[i]->alignCount << " " << patterns[i]->optCount << std::endl;
				patterns[i]->updateScore(minLength, maxAligned); // mise à jour du score
				if (patterns[i]->score > maxRatio){
					maxRatio = patterns[i]->score;
//osParser << "s" << patterns[i]->score << "/? a" << patterns[i]->alignCount << " o" << patterns[i]->optCount << " ma" << maxAligned << " ["<< patterns[i]->position << "] ";
//Sequence::exportLinearSequenceAsString(patterns[i]->pattern, osParser);
				}
			}

			// Suppression de tous les patterns qui ne pourront pas rattraper le meilleur score
			for (int i = (signed)patterns.size()-1 ; i >= 0 ; i--){
				if (patterns[i]->score < maxRatio - SCORE_TOLERENCE){ // TODO : pouvoir paramétrer ce seuil
//osParser << "Suppression du pattern s" << patterns[i]->score << "/" << maxRatio << " a" << patterns[i]->alignCount << " o" << patterns[i]->optCount << " ma" << maxAligned << " [" << patterns[i]->position << "] ";
//Sequence::exportLinearSequenceAsString(patterns[i]->pattern, osParser);
					patterns.erase(patterns.begin()+i);
				}
			}
/*if (currentPos == (int)rootSequence->size()-2){
	std::cout << "Fin traitement : ";
	rootSequence->at(currentPos)->exportAsCompressedString(std::cout);
	std::cout << std::endl;
	for (int i = 0 ; i < (signed)patterns.size() ; i++){
		std::cout << patterns[i]->position << " ";
		Sequence::exportLinearSequenceAsString(patterns[i]->pattern, std::cout);
	}
	std::cout << std::endl;
}*/
		}

std::sort(patterns.begin(), patterns.end(), sortFunction2);
		float averageScore = 0;
		float minScore = 1;
		float maxScore = 0;
		for (int i = 0 ; i < (signed)patterns.size() ; i++){
			averageScore += patterns[i]->score;
			if (patterns[i]->score < minScore)
				minScore = patterns[i]->score;
			if (patterns[i]->score > maxScore)
				maxScore = patterns[i]->score;
		}
		averageScore = averageScore/patterns.size();
std::cout << r << " " << roots.size() << " " << patterns.size() << " " << statsMove << " " << statsAdd << " " << averageScore << " " << minScore << " " << maxScore << std::endl;

		// Maintenant qu'on a terminé de parcourir la trace pour cette accroche, on créé autant de nouveaux roots que de patterns concervés pour poursuivre leur exploration
		for (int i = 0 ; i < (signed)patterns.size() ; i++)
		{
			// On ignore tous les patterns qui contiendraient des calls non optionnelles au delà de la position du dernier ajout. En effet si le dernier call du root a été intégré en plein milieu d'un scénario et que ce scénario inclus dans la suite des calls obligatoires (non optionnels), celà signifierait que ce pattern produirait des calls au dela de la dernière trace du root... Ce qui n'est pas possible vue que le root est terminé, donc ce pattern est faux
			if (Sequence::getNonOptCallInLinearSequence(patterns[i]->pattern, patterns[i]->position) > 0)
				continue;
				
osParser << patterns[i]->score << " " << patterns[i]->upCount << " " << patterns[i]->rootStartingPos << " " << patterns[i]->alignCount << " " << patterns[i]->optCount << " " << patterns[i]->pattern.size() << " ";
Sequence::exportLinearSequenceAsString(patterns[i]->pattern, osParser);

			roots.push_back(ScoredSequence(patterns[i], boost::dynamic_pointer_cast<Sequence>(rootSequence->clone())));
			std::vector<Trace::sp_trace> & traces = roots.back().sequence->getTraces();
			// Incrustation du pattern dans le clone
std::cout << patterns[i]->rootStartingPos << " " << traces.size() << std::endl;
			traces.erase(traces.begin()+patterns[i]->rootStartingPos, traces.end());
std::cout << traces.size() << std::endl;
			roots.back().sequence->insertLinearSequence(patterns[i]->pattern, patterns[i]->rootStartingPos);
roots.back().sequence->exportAsCompressedString(std::cout);
		}

std::cout << "Apres sortie de boucle" << std::endl;
for (int r2 = 0 ; r2 < roots.size() ; r2++){
	roots[r2].sequence->exportAsCompressedString(osParser);
}

osParser << r << "," << roots.size() << "," << maxRatio << "," << roots[r].scenario->score << "," << roots[r].scenario->alignCount << "," << roots[r].scenario->optCount << ",";
rootSequence->exportAsCompressedString(osParser);

	}

	std::sort(roots.begin(), roots.end(), sortFunction);
	osParser << "BEST SOLUTION !!!!" << std::endl;
	for (int i = 0 ; i < (signed)roots.size() ; i++){
		osParser << roots[i].scenario->score << " ";
		roots[i].sequence->exportAsCompressedString(osParser);
	}
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
