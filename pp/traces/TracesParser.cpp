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

int TracesParser::lineNum = 0;
int TracesParser::mission_end_time = 0;
int TracesParser::execution_start_time = 0;
std::string TracesParser::lang = "";
std::string TracesParser::mission_name = "";
std::string TracesParser::params_json = "";

#ifdef DEBUG_PARSER
	#ifdef LOG_IN_FILE
		std::ofstream debugOfsParser("debugParser.log", std::ofstream::out);
		std::ostream& TracesParser::osParser = debugOfsParser;
	#else
		std::ostream& TracesParser::osParser = std::cout;
	#endif
#endif

TracesParser::TracesParser(): used(false), compressed(false), end(false), proceed(false), start(0) {
	root = boost::make_shared<Sequence>(1, true);
}

TracesParser::~TracesParser() {
	CloseResources();
}

bool TracesParser::InitResources(const std::string& dir_path, const std::string& filename) {
	if (used) {
		#ifdef DEBUG_PARSER
			osParser << "parsing of traces file already launched" << std::endl;
		#endif
		return false;
	}
	if (filename.find(".log") == std::string::npos) {
		#ifdef DEBUG_PARSER
		 	osParser << "not a log file" << std::endl;
		#endif
		return false;
	}
	std::string s = dir_path + "\\" + filename;
	if (ifs.is_open())
		ifs.close();
	ifs.open(s.c_str(), std::ios::in | std::ios::binary);
	if (ifs.good()) {
		if (root->size() != 0)
			root->clear();
		this->dir_path = dir_path;
		this->filename = filename;
		used = true;
	}
	else{
		#ifdef DEBUG_PARSER
			osParser << "error opening file: " << strerror(errno) << std::endl;
		#endif
	}
	return used;
}

void TracesParser::CloseResources() {
	lineNum = 0;
	start = 0;
	used = false;
	end = false;
	compressed = false;
	proceed = false;
	if (ifs.is_open())
		ifs.close();
	#ifdef DEBUG_PARSER
		#ifdef LOG_IN_FILE
			if (debugOfsParser.is_open())
				debugOfsParser.close();
		#endif
	#endif
}

void TracesParser::saveCompression() {
	#ifdef DEBUG_PARSER
		exportTracesAsString(osParser);
	#endif
	// Export compression results as txt file
	std::string s = "\\" + filename;
	s.replace(s.find(".log"), 4, "_compressed.txt");
	s.insert(0, dir_path);
	std::ofstream ofs(s.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (ofs.good()) {
		exportTracesAsString(ofs);
		ofs.close();
	}
	// Export compression results as xml file
	rapidxml::xml_document<> doc;
	rapidxml::xml_node<>* dec = doc.allocate_node(rapidxml::node_declaration);
	dec->append_attribute(doc.allocate_attribute("version", "1.0"));
	dec->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(dec);
	rapidxml::xml_node<>* root_node = doc.allocate_node(rapidxml::node_element, "trace");
	doc.append_node(root_node);
	std::stack<rapidxml::xml_node<> *> node_stack;
	node_stack.push(root_node);
	rapidxml::xml_node<> *node;
	std::stack<Sequence::sp_sequence> seq_stack;
	Sequence::sp_sequence sps;
	Trace::sp_trace spt;
	unsigned int i = 0;
	bool pass = false;
	while (i < root->size() || !seq_stack.empty()) {
		if (!seq_stack.empty()) {
			while (!seq_stack.empty() && sps->isEndReached()) {
				seq_stack.pop();
				node_stack.pop();
				if (!seq_stack.empty())
					sps = seq_stack.top();
			}
			if (!sps->isEndReached()) {
				spt = sps->next();
				pass = true;
			}
		}
		if (seq_stack.empty() && i < root->size()) {
			spt = root->at(i++);
			pass = true;
		}
		if (pass) {
			pass = false;
			if (spt->isEvent()) {
				Event *e = dynamic_cast<Event*>(spt.get());
				if (e->getLabel().compare(START_MISSION) == 0) {
					while (node_stack.size() > 1)
						node_stack.pop();
					StartMissionEvent *sme = dynamic_cast<StartMissionEvent*>(e);
					node = doc.allocate_node(rapidxml::node_element, "mission");
					node->append_attribute(doc.allocate_attribute("name", doc.allocate_string(sme->getMissionName().c_str())));
					node->append_attribute(doc.allocate_attribute("start_time", doc.allocate_string(boost::lexical_cast<std::string>(sme->getStartTime()).c_str())));
					node_stack.top()->append_node(node);
					node_stack.push(node);
				}
				else if (e->getLabel().compare(END_MISSION) == 0) {
					EndMissionEvent *eme = dynamic_cast<EndMissionEvent*>(e);
					if (node_stack.size() > 2)
						node_stack.pop();
					node_stack.top()->append_attribute(doc.allocate_attribute("end_time", doc.allocate_string(boost::lexical_cast<std::string>(eme->getEndTime()).c_str())));
					node_stack.top()->append_attribute(doc.allocate_attribute("status", doc.allocate_string(eme->getStatus().c_str())));
					node_stack.pop();
				}
				else if (e->getLabel().compare(NEW_EXECUTION) == 0) {
					NewExecutionEvent *nee = dynamic_cast<NewExecutionEvent*>(e);
					if (node_stack.size() > 2)
						node_stack.pop();
					node = doc.allocate_node(rapidxml::node_element, "execution");
					node->append_attribute(doc.allocate_attribute("start_time", doc.allocate_string(boost::lexical_cast<std::string>(nee->getStartTime()).c_str())));
					node->append_attribute(doc.allocate_attribute("programming_language_used", doc.allocate_string(nee->getProgrammingLangageUsed().c_str())));
					node_stack.top()->append_node(node);
					node_stack.push(node);
				}
				else if (e->getLabel().compare(END_EXECUTION) == 0) {
					node_stack.top()->append_attribute(doc.allocate_attribute("end_time", doc.allocate_string(boost::lexical_cast<std::string>(dynamic_cast<EndExecutionEvent*>(e)->getEndTime()).c_str())));
					node_stack.pop();
				}
				else {
					node = doc.allocate_node(rapidxml::node_element, "event");
					node->append_attribute(doc.allocate_attribute("label", doc.allocate_string(e->getLabel().c_str())));
					if (!e->getInfo().empty())
						node->append_attribute(doc.allocate_attribute("info", doc.allocate_string(e->getInfo().c_str())));
					node_stack.top()->append_node(node);
				}
			}
			else if (spt->isCall()) {
				Call *c = dynamic_cast<Call*>(spt.get());
				node = doc.allocate_node(rapidxml::node_element, "call");
				node->append_attribute(doc.allocate_attribute("label", doc.allocate_string(c->getKey().c_str())));
				if (c->getError() != Call::NONE)
					node->append_attribute(doc.allocate_attribute("error", doc.allocate_string(Call::getEnumLabel<Call::ErrorType>(c->getError(),Call::errorsArr))));
				s = c->getParams();
				if (s.compare("") != 0)
					node->append_attribute(doc.allocate_attribute("params", doc.allocate_string(s.c_str())));
				s = c->getReturn();
				if (s.compare("") != 0)
					node->append_attribute(doc.allocate_attribute("return", doc.allocate_string(s.c_str())));
				node->append_attribute(doc.allocate_attribute("info", doc.allocate_string(c->getInfo().c_str())));
				node_stack.top()->append_node(node);
			}
			else {
				sps = boost::dynamic_pointer_cast<Sequence>(spt);
				sps->reset();
				node = doc.allocate_node(rapidxml::node_element, "sequence");
				node->append_attribute(doc.allocate_attribute("num_map", doc.allocate_string(Sequence::getIterartionDescriptionString(sps->getIterationDescription()).c_str())));
				node->append_attribute(doc.allocate_attribute("info", doc.allocate_string(sps->getInfo().c_str())));
				std::string nb_iteration_fixed = (sps->hasNumberIterationFixed()) ? "true" : "false";
				node->append_attribute(doc.allocate_attribute("nb_iteration_fixed", doc.allocate_string(nb_iteration_fixed.c_str())));
				node_stack.top()->append_node(node);
				node_stack.push(node);
				seq_stack.push(sps);
			}
		}
	}
	s = "\\" + filename;
	s.replace(s.find(".log"), 4, "_compressed.xml");
	s.insert(0, dir_path);
	std::ofstream ofsXml(s.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (ofsXml.good()) {
		ofsXml << doc;
		ofsXml.close();
	}
	doc.clear();
}

/**
	* \brief Lancement de la compression d'un fichier de traces brutes avec l'algorithme de compression hors-ligne.
	*
	* \param dir_path le chemin d'accès au fichier.
	* \param filename le nom du fichier.
	*/
void TracesParser::parseLogFile(const std::string& dir_path, const std::string& filename, bool waitEndFlag) {
	// check if we can start parsing
	if (InitResources(dir_path,filename)) {
		#ifdef DEBUG_PARSER
			osParser << "Start parsing traces" << std::endl;
		#endif
		std::string line;
		bool executionDetected = false;
		#ifdef DEBUG_PARSER
			osParser << "Loop until end is set" << std::endl;
		#endif
		end = false;
		proceed = false;
		while (!end) {
			// If we don't have to wait the "end" flag we set it to true and the "proceed" also in order to compress and exit after reading all the file (no waiting new input)
			if (!waitEndFlag){
				end = true;
				proceed = true;
			}
			// we pop all new line included into the input file stream
			while (std::getline(ifs, line)) {
				lineNum++;
				// we build a trace from the current line
				#ifdef DEBUG_PARSER
					osParser << std::endl << "Parse line: " << line;
				#endif
				Trace::sp_trace spt = parseLine(line);
				if (spt) {
					#ifdef DEBUG_PARSER
						osParser << "Check if this line is an event" << std::endl;
					#endif
					Event::sp_event spe;
					if (spt->isEvent()){
						#ifdef DEBUG_PARSER
							osParser << "\tThis line is an event" << std::endl;
						#endif
						spe = boost::dynamic_pointer_cast<Event>(spt);
					}
					// We check if this trace is an event that we can't aggregate
					if (spe && Trace::inArray(spe->getLabel().c_str(), Event::noConcatEventsArr) > -1) {
						#ifdef DEBUG_PARSER
							osParser << "\tevent nature: "<< spe->getLabel().c_str() << std::endl;
						#endif
						// We add this event at the end of the trace
						#ifdef DEBUG_PARSER
							osParser << "\tadd event at the end of the trace" << std::endl;
						#endif
						root->addTrace(spe);
						// If we detect a new execution
						if (spe->getLabel().compare(NEW_EXECUTION) == 0){
							// Check if a previous execution has been detected
							if (executionDetected){
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
						} else if (spe->getLabel().compare(END_EXECUTION) == 0){ // If we detect an end execution
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
					}
					else{
						// The trace is agregable, so we try to do it
						#ifdef DEBUG_PARSER
							osParser << "Minimal inline compression" << std::endl;
						#endif
						inlineCompression(spt);
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
			if (!ifs.eof()){
				#ifdef DEBUG_PARSER
					osParser << "error occurs => we stop to read traes" << std::endl;
				#endif
				end = true; // Stop to read input file
			}
			ifs.clear();
			// check if we have to compress agregate traces. proceed means game engine asks to proceed compression
			if (proceed) {
				// check if new traces are becoming since previous start mission event
				if (start < (int)root->size()){
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
}

Trace::sp_trace TracesParser::parseLine(const std::string& s) {
	Trace *t = NULL;
	std::vector<std::string> tokens = splitLine(s);
	// Gestion des évènements produits par le jeu
	if (tokens[0].compare(GAME_START) == 0) {
		TracesParser::mission_name = tokens[1];
	}
	else if (tokens[0].compare(MISSION_START_TIME) == 0) {
		t = new StartMissionEvent(TracesParser::mission_name, stoi(tokens[1]));
	}
	else if (tokens[0].compare(MISSION_END_TIME) == 0) {
		TracesParser::mission_end_time = stoi(tokens[1]);
	}
	else if (tokens[0].compare(GAME_END) == 0) {
		t = new EndMissionEvent(tokens[1], TracesParser::mission_end_time);
	}
	else {
		int numTokens = tokens.size();
		int ind = 0;
		// Recherche du token "-". Dans ce cas les tokens suivant représentent les valeurs retournées par l'appel.
		// On réduit donc le nombre de token pour ne compter que les paramètres et ne pas traiter ici les codes de retour.
		std::vector<std::string>::const_iterator it = std::find(tokens.begin(), tokens.end(), "-");
		while (it++ != tokens.end())
			numTokens--;
		bool delayed = false;
		// Gestion des évènements produit par l'éxecution du code du joueur
		if (tokens[ind].compare(DELAYED) == 0) {
			delayed = true;
			ind++;
		}
		if (tokens[ind].compare(EXECUTION_START_TIME) == 0) {
			TracesParser::execution_start_time = stoi(tokens[ind+1]);
		}
		else if (tokens[ind].compare(PROGRAMMING_LANGUAGE_USED) == 0) {
			// Création de l'évènement
			t = new NewExecutionEvent(TracesParser::execution_start_time, tokens[ind+1]);
			// Chargement des params en fonction du langage utilisé
			if (TracesParser::params_json.compare("") != 0){
				#ifdef DEBUG_PARSER
				 	osParser << "Params defined, use it to compress and analyse." << std::endl;
				#endif
				Call::callMaps.initMaps(TracesParser::params_json, tokens[ind+1], TracesParser::lang);
			} else {
				#ifdef DEBUG_PARSER
					osParser << "No params defined\nUsing default compression options." << std::endl;
				#endif
			}
		}
		else if (tokens[ind].compare(EXECUTION_END_TIME) == 0) {
			t = new EndExecutionEvent(stoi(tokens[ind+1]));
		}
		else if (INCLUDE_EVENTS == 1 && Trace::inArray(tokens[ind].c_str(), Event::concatEventsArr) > -1) {
			t = new Event(tokens[ind]);
		}
		else {
			// On vérifie si une erreur a été générée
			Call::ErrorType err = Call::getEnumType<Call::ErrorType>(tokens[ind].c_str(),Call::errorsArr);
			if (err != Call::NONE) {
				ind++;
			}
			// Gestion des appels dans la cas où le fichier params n'a pas été chargé ou l'appel est inconnu ou c'est un appel de fonction connu sans paramètre
			if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_NO_PARAMS) == 0) {
				t = new CallWithNoParam(tokens[ind]);
			}
			// Dans ce cas, nous sommes sur un appel de fonction avec paramètre
			// Gestion des appels ne prenant en paramètre qu'un entier
			else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_PARAM) == 0) {
				int p = (ind+1 <= numTokens-1) ? stoi(tokens[ind+1]) : -1;
				t = new CallWithIntParam(err,tokens[ind],p);
			}
			// Gestion des appels prenant en paramètre deux entiers
			else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_INT_PARAMS) == 0) {
				int p1 = (ind+1 <= numTokens-1) ? stoi(tokens[ind+1]) : -1;
				int p2 = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1;
				t = new CallWithIntIntParams(err,tokens[ind],p1,p2);
			}
			// Gestion des appels prenant en paramètre un entier, une unité et un entier
			else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_UNIT_INT_PARAMS) == 0) {
				int p1 = (ind+1 <= numTokens-1) ? stoi(tokens[ind+1]) : -1;
				// Récupération de l'id de l'unité et de son type (séparés par un "_")
				int unitId = -1, unitType = -1;
				if (ind+2 <= numTokens-1) {
					std::vector<std::string> unitParamTokens = splitLine(tokens[ind+2],'_');
					unitId = stoi(unitParamTokens[0]);
					if (unitParamTokens.size() == 2)
						unitType = stoi(unitParamTokens[1]);
				}
				int p2 = (ind+3 <= numTokens-1) ? stoi(tokens[ind+3]) : -1;
				t = new CallWithIntUnitIntParams(err,tokens[ind],p1,unitId,unitType,p2);
			}
			// Gestion des appels prenant en paramètre un entier et une unité
			else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_UNIT_PARAMS) == 0) {
				int p1 = (ind+1 <= numTokens-1) ? stoi(tokens[ind+1]) : -1;
				// Récupération de l'id de l'unité et de son type (séparés par un "_")
				int unitId = -1, unitType = -1;
				if (ind+2 <= numTokens-1) {
					std::vector<std::string> unitParamTokens = splitLine(tokens[ind+2],'_');
					unitId = stoi(unitParamTokens[0]);
					if (unitParamTokens.size() == 2)
						unitType = stoi(unitParamTokens[1]);
				}
				t = new CallWithIntUnitParams(err,tokens[ind],p1,unitId,unitType);
			}
			// Gestion des appels prenant en paramètre deux entiers et une unité
			else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_INT_UNIT_PARAMS) == 0) {
				int p1 = (ind+1 <= numTokens-1) ? stoi(tokens[ind+1]) : -1;
				int p2 = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1;
				// Récupération de l'id de l'unité et de son type (séparés par un "_")
				int unitId = -1, unitType = -1;
				if (ind+3 <= numTokens-1) {
					std::vector<std::string> unitParamTokens = splitLine(tokens[ind+3],'_');
					unitId = stoi(unitParamTokens[0]);
					if (unitParamTokens.size() == 2)
						unitType = stoi(unitParamTokens[1]);
				}
				t = new CallWithIntIntUnitParams(err,tokens[ind],p1,p2,unitId,unitType);
			}
			else {
				// Dans ce cas, nous sommes sur un appel avec une unité en paramètres
				// Récupération de l'id de l'unité et de son type (séparés par un "_")
				int unitId = -1, unitType = -1;
				if (ind+1 <= numTokens-1) {
					std::vector<std::string> unitParamTokens = splitLine(tokens[ind+1],'_');
					unitId = stoi(unitParamTokens[0]);
					if (unitParamTokens.size() == 2)
						unitType = stoi(unitParamTokens[1]);
				}
				// Gestion des appel ne prenant qu'une unité en paramètre
				if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_PARAM) == 0) {
					t = new CallWithUnitParam(err,tokens[ind],unitId,unitType);
				}
				// Gestion des appel prenant une unité et un entier en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_PARAMS) == 0) {
					int p = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1;
					t = new CallWithUnitIntParams(err,tokens[ind],unitId,unitType,p);
				}
				// Gestion des appel prenant une unité et deux entiers en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_INT_PARAMS) == 0) {
					int p1 = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1;
					int p2 = (ind+3 <= numTokens-1) ? stoi(tokens[ind+3]) : -1;
					t = new CallWithUnitIntIntParams(err,tokens[ind],unitId,unitType,p1,p2);
				}
				// Gestion des appel prenant une unité, un entier, une seconde unité et un second entier en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_UNIT_INT_PARAMS) == 0) {
					int p1 = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1, targetId = -1, targetType = -1;
					std::vector<std::string> targetParamTokens;
					if (ind+3 <= numTokens-1) {
						targetParamTokens = splitLine(tokens[ind+3],'_');
						targetId = stoi(targetParamTokens[0]);
						if (targetParamTokens.size() == 2)
							targetType = stoi(targetParamTokens[1]);
					}
					int p2 = (ind+4 <= numTokens-1) ? stoi(tokens[ind+4]) : -1;
					t = new CallWithUnitIntUnitIntParams(err,tokens[ind],unitId,unitType,p1,targetId,targetType,p2);
				}
				// Gestion des appel prenant une unité, un entier, une position et un second entier en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_POS_INT_PARAMS) == 0) {
					int p1 = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1;
					float x = (ind+3 <= numTokens-1) ? stof(tokens[ind+3]) : -1, y = (ind+4 <= numTokens-1) ? stof(tokens[ind+4]) : -1;
					int p2 = (ind+5 <= numTokens-1) ? stoi(tokens[ind+5]) : -1;
					t = new CallWithUnitIntPosIntParams(err,tokens[ind],unitId,unitType,p1,x,y,p2);
				}
				// Gestion des appel prenant une unité, deux entiers  et un réel en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_FLOAT_INT_PARAMS) == 0) {
					int p1 = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1;
					float p2 = (ind+3 <= numTokens-1) ? stof(tokens[ind+3]) : -1;
					int p3 = (ind+4 <= numTokens-1) ? stoi(tokens[ind+4]) : -1;
					t = new CallWithUnitIntFloatIntParams(err,tokens[ind],unitId,unitType,p1,p2,p3);
				}
			}
		}
		// Si la trace est valide on définit si c'est un appel en retard et éventuellement les codes de retour en cas d'erreur
		if (t != NULL) {
			if (delayed)
				t->setDelayed();
			if (t->isCall()) {
				Call *c = dynamic_cast<Call*>(t);
				it = std::find(tokens.begin(), tokens.end(), "-");
				if (it != tokens.end()) {
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

void TracesParser::inlineCompression(Trace::sp_trace& spt) {
	bool add = false;
	if (root->size() > 1) {
		// Check if the last trace is an event
		if (spt->isEvent()) {
			// add this event (new trace) at the end of the traces
			root->addTrace(spt);
			add = true;
		}
		else{
			// Get the last trace into this last sequence and look for the last Call
			Sequence::sp_sequence parent_seq = root;
			// Check if the last trace is a sequence
			if (root->getTraces().back()->isSequence()) {
				// While the last trace of this sequence is a sequence, we continue to progress inside
				while (parent_seq->getTraces().back()->isSequence())
					parent_seq = boost::dynamic_pointer_cast<Sequence>(parent_seq->getTraces().back());
				// Now "parent_seq" don't finish by a sequence
			}
			// Check if the last trace of this sequence is equal to the new trace
			Trace::sp_trace lastTrace = parent_seq->getTraces().back();
			if (spt->isCall() && lastTrace->operator==(spt.get())) {
				if (parent_seq->size() == 1){
					// We include new trace in the sequence
					parent_seq->addOne();
				} else {
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
	// If no direct compression occurs we add the new trace at the end of traces and we try a classic compression with a short window
	if (!add){
		root->addTrace(spt);
		// on remonte jusqu'à un maximum de 3 fois la fenêtre maximale...
		int limit = root->size() - MAX_END_SEARCH*3;
		// ... ou si on atteindrait le début de la trace
		if (limit < start) limit = start;
		// try to compress succesive Calls
		Sequence::findAndAggregateSuccessiveSequences(root, limit, true);
	}
}

void TracesParser::offlineCompression() {
	#ifdef DEBUG_PARSER
		osParser << "\tWe try to aggregate successive sequences" << std::endl;
	#endif
	Sequence::findAndAggregateSuccessiveSequences(root, start);
	#ifdef DEBUG_PARSER
		root->exportAsString(osParser);
	#endif
/*	#ifdef DEBUG_PARSER
		osParser << "\tWe try to rotate sequences due to if statements" << std::endl;
	#endif
	Sequence::findAndProcessRotatingSequences(root, start);
	#ifdef DEBUG_PARSER
		root->exportAsString(osParser);
	#endif
	#ifdef DEBUG_PARSER
		osParser << "\tWe try merge inclusive sequences" << std::endl;
	#endif
	Sequence::findAndProcessInclusiveSequences(root, start);
	#ifdef DEBUG_PARSER
		root->exportAsString(osParser);
	#endif*/
}

void TracesParser::exportTracesAsString(std::ostream &os) {
	int num_start = 0;
	Event *e = NULL;
	for (unsigned int i = 0; i < root->size(); i++) {
		e = root->at(i)->isEvent() ? dynamic_cast<Event*>(root->at(i).get()) : NULL;
		if (e != NULL && Trace::inArray(e->getLabel().c_str(), Event::noConcatEventsArr) > -1) {
			if (e->getLabel().compare(START_MISSION) == 0) {
				if (num_start++ > 0)
					os << std::endl;
				StartMissionEvent *sme = dynamic_cast<StartMissionEvent*>(e);
				os << GAME_START << " " << sme->getMissionName() << std::endl << MISSION_START_TIME << " " << sme->getStartTime() << std::endl;
			}
			else if (e->getLabel().compare(END_MISSION) == 0) {
				EndMissionEvent *eme = dynamic_cast<EndMissionEvent*>(e);
				os << "status " << eme->getStatus() << std::endl << MISSION_END_TIME << " " << eme->getEndTime() << std::endl;
			}
			else if (e->getLabel().compare(NEW_EXECUTION) == 0) {
				NewExecutionEvent *nee = dynamic_cast<NewExecutionEvent*>(e);
				os << "\t" << EXECUTION_START_TIME << " " << nee->getStartTime() << std::endl << "\t" << PROGRAMMING_LANGUAGE_USED << " " << nee->getProgrammingLangageUsed() << std::endl;
				e->numTab = 1;
			}
			else if (e->getLabel().compare(END_EXECUTION) == 0) {
				os << "\t" << EXECUTION_END_TIME << " " << dynamic_cast<EndExecutionEvent*>(e)->getEndTime() << std::endl;
				e->numTab = 0;
			}
		}
		else
			root->at(i)->exportAsString(os);
	}
	if (Trace::numTab > 0)
		Trace::numTab = 0;
}

std::vector<Trace::sp_trace> TracesParser::importTraceFromXml(const std::string& xml, std::ostream& os) {
	std::vector<Trace::sp_trace> traces;
	try {
		std::vector<char> xml_content(xml.begin(), xml.end());
		xml_content.push_back('\0');
		rapidxml::xml_document<> doc;
		doc.parse<0>(&xml_content[0]);
		rapidxml::xml_node<> *root_node = doc.first_node("trace");
		if (root_node != 0) {
			#ifdef DEBUG_PARSER
				os << "\nbegin import from XML file" << std::endl;
			#endif
			importTraceFromNode(root_node->first_node(),traces);
			for (unsigned int i = 0; i < traces.size(); i++){
				#ifdef DEBUG_PARSER
				 	os << i << " " << traces.at(i) << std::endl;
				#endif
				traces.at(i)->exportAsString(os);
			}
		}
	}
	catch (const std::runtime_error& e) {
		#ifdef DEBUG_PARSER
			os << e.what() << std::endl;
		#endif
	}
	return traces;
}

void TracesParser::importTraceFromNode(rapidxml::xml_node<> *node, std::vector<Trace::sp_trace>& traces) {
	Sequence::sp_sequence sps;
	std::stack<rapidxml::xml_node<> *> node_stack;
	std::stack<Sequence::sp_sequence> seq_stack;
	std::string node_name;
	while(node || !node_stack.empty()) {
		while (!node && !node_stack.empty()) {
			node = node_stack.top();
			node_name = node->name();
			if (node_name.compare("mission") == 0 && node->first_attribute("end_time") != 0) {
				traces.push_back(boost::make_shared<EndMissionEvent>(node->first_attribute("status")->value(),atoi(node->first_attribute("end_time")->value())));
			}
			else if (node_name.compare("execution") == 0 && node->first_attribute("end_time") != 0) {
				traces.push_back(boost::make_shared<EndExecutionEvent>(atoi(node->first_attribute("end_time")->value())));
			}
			else if (node_name.compare("sequence") == 0) {
				seq_stack.pop();
				if (!seq_stack.empty()) {
					seq_stack.top()->addTrace(sps);
					sps = seq_stack.top();
				}
				else
					traces.push_back(sps);
			}
			node = node_stack.top()->next_sibling();
			node_stack.pop();
		}
		if (node) {
			node_name = node->name();
			if (node_name.compare("mission") == 0) {
				traces.push_back(boost::make_shared<StartMissionEvent>(node->first_attribute("name")->value(),atoi(node->first_attribute("start_time")->value())));
				node_stack.push(node);
				node = node->first_node();
			}
			else if (node_name.compare("execution") == 0) {
				traces.push_back(boost::make_shared<NewExecutionEvent>(atoi(node->first_attribute("start_time")->value()), node->first_attribute("programming_language_used")->value()));
				node_stack.push(node);
				node = node->first_node();
			}
			else if (node_name.compare("sequence") == 0) {
				std::string info;
				if (node->first_attribute("info") != 0)
					info = node->first_attribute("info")->value();
				bool num_fixed = false;
				if (node->first_attribute("nb_iteration_fixed") != 0)
					num_fixed = std::string(node->first_attribute("nb_iteration_fixed")->value()).compare("true") == 0;
				sps = boost::make_shared<Sequence>(info,num_fixed);
				if (node->first_attribute("num_map") != 0) {
					std::vector<std::string> tokens = splitLine(node->first_attribute("num_map")->value());
					for (unsigned int i = 0; i < tokens.size(); i++) {
						int pos = tokens.at(i).find(":",0);
						sps->addIteration(stoi(std::string(tokens.at(i).begin(),tokens.at(i).begin()+pos)),stoi(std::string(tokens.at(i).begin()+pos+1,tokens.at(i).end())));
					}
				}
				seq_stack.push(sps);
				node_stack.push(node);
				node = node->first_node();
			}
			else {
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

unsigned int TracesParser::getNodeChildCount(rapidxml::xml_node<> *node) {
	unsigned int i = 0;
	for (rapidxml::xml_node<> *child = node->first_node(); child; child = child->next_sibling())
		i++;
	return i;
}

void TracesParser::setEnd() {
	end = true;
}

bool TracesParser::getEnd() const {
	return end;
}

bool TracesParser::compressionDone() {
	if (compressed) {
		compressed = false;
		return true;
	}
	return false;
}

void TracesParser::setProceed(bool proceed) {
	this->proceed = proceed;
}

bool TracesParser::getProceed() {
	return proceed;
}

void TracesParser::setLang(std::string lang){
	TracesParser::lang = lang;
}

std::vector<std::string> TracesParser::splitLine(const std::string& s, char delim) {
	std::vector<std::string> buf;
	std::stringstream ss(s);
	std::string subs;
	while(std::getline(ss, subs, delim)) {
		if (subs[subs.size()-1] == '\r' || subs[subs.size()-1] == '\n')
			subs.erase(subs.size()-1);
		buf.push_back(subs);
	}
	return buf;
}

int TracesParser::stoi(const std::string& s) {
	int res;
	if (s.compare("?") == 0)
		return -1;
	try {
		res = boost::lexical_cast<int>(s);
	}
	catch(const boost::bad_lexical_cast &) {
		#ifdef DEBUG_PARSER
		 	osParser << "error boost::lexical_cast<int>(" << s << ")" << std::endl;
		#endif
		exit(EXIT_FAILURE);
	}
	return res;
}

float TracesParser::stof(const std::string& s) {
	float res;
	if (s.compare("?") == 0)
		return -1;
	try {
		res = boost::lexical_cast<float>(s);
	}
	catch(const boost::bad_lexical_cast &) {
		#ifdef DEBUG_PARSER
		 	osParser << "error boost::lexical_cast<float>" << std::endl;
		#endif
		exit(EXIT_FAILURE);
	}
	return res;
}
