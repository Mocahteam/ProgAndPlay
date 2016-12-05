#include "TracesParser.h"
#include "TraceConstantList.h"

#define DEBUG
#define LOG_IN_FILE

int TracesParser::lineNum = 0;
int TracesParser::mission_end_time = 0;
int TracesParser::execution_start_time = 0;
std::string TracesParser::mission_name = "";
Trace::sp_trace TracesParser::spe_eme;

#ifdef DEBUG
	#ifdef LOG_IN_FILE
		std::ofstream debugOfsParser("debugParser.log", std::ofstream::out);
		std::ostream& osParser = debugOfsParser;
	#else
		std::ostream& osParser = std::cout;
	#endif
#endif

TracesParser::TracesParser(bool in_game): in_game(in_game), used(false), compressed(false), end(false), proceed(false), start(0) {}

TracesParser::~TracesParser() {
	endParse();
}

const std::string TracesParser::loadFile(std::string path) {
	std::string res;
	std::ifstream in(path.c_str());
	if (in.good()) {
		std::string line;
		while(std::getline(in,line))
			res += line;
	}
	return res;
}

bool TracesParser::beginParse(const std::string& dir_path, const std::string& filename) {
	if (used) {
		#ifdef DEBUG
			osParser << "parsing of traces file already launched" << std::endl;
		#endif
		return false;
	}
	if (filename.find(".log") == std::string::npos) {
		#ifdef DEBUG
		 	osParser << "not a log file" << std::endl;
		#endif
		return false;
	}
	std::string s = dir_path + "\\" + filename;
	if (ifs.is_open())
		ifs.close();
	ifs.open(s.c_str(), std::ios::in | std::ios::binary);
	if (ifs.good()) {
		if (!traces.empty())
			traces.clear();
		this->dir_path = dir_path;
		this->filename = filename;
		used = true;
	}
	else{
		#ifdef DEBUG
			osParser << "error opening file : " << strerror(errno) << std::endl;
		#endif
	}
	return used;
}

void TracesParser::endParse() {
	lineNum = 0;
	start = 0;
	used = false;
	end = false;
	compressed = false;
	proceed = false;
	if (ifs.is_open())
		ifs.close();
	#ifdef DEBUG
		#ifdef LOG_IN_FILE
			if (debugOfsParser.is_open())
				debugOfsParser.close();
		#endif
	#endif
}

void TracesParser::writeFiles() {
	display(osParser);
	std::string s = "\\" + filename;
	s.replace(s.find(".log"), 4, "_compressed.txt");
	s.insert(0, dir_path);
	std::ofstream ofs(s.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (ofs.good()) {
		display(ofs);
		ofs.close();
	}
	exportTraceToXml();
	compressed = true;
}

/*
 * Starts the parsing of the traces file 'filename' which is located in 'dir_path' with offline algorithm (the file has to be already filled with the traces).
 *
 */
void TracesParser::parseTraceFileOffline(const std::string& dir_path, const std::string& filename) {
	if (beginParse(dir_path,filename)) {
		if (in_game && reachLastStart())
			readTracesOfflineInGame();
		else {
			readTracesOffline();
			writeFiles();
		}
		endParse();
	}
}

void TracesParser::parseTraceFile(const std::string& dir_path, const std::string& filename) {
	if (beginParse(dir_path,filename)) {
		std::string line;
		Event *e = NULL;
		while (std::getline(ifs, line)) {
			lineNum++;
			#ifdef DEBUG
				osParser << "line : " << lineNum << std::endl;
			#endif
			Trace::sp_trace spt = handleLine(line);
			if (spt) {
				e = (spt->isEvent()) ? dynamic_cast<Event*>(spt.get()) : NULL;
				if (e != NULL && Trace::inArray(e->getLabel().c_str(), Event::noConcatEventsArr) > -1) {
					if (e->getLabel().compare(END_EXECUTION) == 0)
						traces.push_back(spt);
					if (spe_eme && e->getLabel().compare(NEW_EXECUTION) != 0) {
						traces.push_back(spe_eme);
						spe_eme.reset();
					}
					if (e->getLabel().compare(END_EXECUTION) != 0)
						traces.push_back(spt);
				}
				else
					traces.push_back(spt);
			}
		}
	}
	exportTraceToXml();
	endParse();
}

/*
 * Handles all traces contained in the file with the offline algorithm.
 *
 */
void TracesParser::readTracesOffline() {
	std::string line;
	while (std::getline(ifs, line)) {
		lineNum++;
		Trace::sp_trace spt = handleLine(line);
		if (spt) {
			Event::sp_event spe;
			if (spt->isEvent())
				spe = boost::dynamic_pointer_cast<Event>(spt);
			if (spe && Trace::inArray(spe->getLabel().c_str(), Event::noConcatEventsArr) > -1) {
				detectSequences();
				if (spe->getLabel().compare(END_EXECUTION) == 0)
					traces.push_back(spe);
				if (spe_eme && spe->getLabel().compare(START_MISSION) == 0) {
					traces.push_back(spe_eme);
					spe_eme.reset();
				}
				if (spe->getLabel().compare(END_EXECUTION) != 0)
					traces.push_back(spe);
				start = traces.size();
			}
			else
				handleTraceOffline(spt);
		}
	}
	detectSequences();
	if (spe_eme) {
		traces.push_back(spe_eme);
		spe_eme.reset();
	}
}

void TracesParser::readTracesOfflineInGame() {
	std::string line;
	bool change = false;
	while (!end) {
		while (std::getline(ifs, line)) {
			lineNum++;
			Trace::sp_trace spt = handleLine(line);
			if (spt) {
				Event::sp_event spe;
				if (spt->isEvent())
					spe = boost::dynamic_pointer_cast<Event>(spt);
				if (spe && Trace::inArray(spe->getLabel().c_str(), Event::noConcatEventsArr) > -1) {
					detectSequences();
					traces.push_back(spe);
					start = traces.size();
					if (spe->getLabel().compare(NEW_EXECUTION) == 0 || spe->getLabel().compare(START_MISSION) == 0)
						change = true;
				}
				else
					handleTraceOffline(spt);
			}
		}
		if (change && proceed) {
			if (spe_eme) {
				detectSequences();
				traces.push_back(spe_eme);
				spe_eme.reset();
				start = traces.size();
			}
			Event::sp_event spe;
			if (traces.back()->isEvent())
				spe = boost::dynamic_pointer_cast<Event>(traces.back());
			if (!spe || (spe->getLabel().compare(START_MISSION) != 0 && spe->getLabel().compare(NEW_EXECUTION) != 0)) {
				detectSequences();
				writeFiles(); //inform ProgAndPlay.cpp the compression is done
				change = false;
			}
		}
		if (!ifs.eof())
			end = true; // Ensure end of read was EOF.
		else
			//Sleep(500);
		ifs.clear();
	}
}

Trace::sp_trace TracesParser::handleLine(const std::string& s) {
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
		spe_eme = boost::make_shared<EndMissionEvent>(tokens[1], TracesParser::mission_end_time);
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
			// Chargement du fichier params en fonction du langage utilisé
			#ifdef DEBUG
			 	osParser << "Try to open params.json file from mods directory ([MODS]/traces/)" << std::endl;
			#endif
			std::string params_json = TracesParser::loadFile("./traces/params.json");
			if (params_json.compare("") != 0){
				#ifdef DEBUG
				 	osParser << "File found and used for compression and analysis." << std::endl;
				#endif
				Call::callMaps.initMaps(params_json, tokens[ind+1]);
			} else {
				#ifdef DEBUG
				 	osParser << "File not found\nTry to open params.json file from Spring directory ([SPRING]/traces/data/)" << std::endl;
				#endif
				params_json = TracesParser::loadFile("./traces/data/params.json");
				if (params_json.compare("") != 0){
					#ifdef DEBUG
					 	osParser << "File found and used for compression and analysis." << std::endl;
					#endif
					Call::callMaps.initMaps(params_json, tokens[ind+1]);
				} else {
					#ifdef DEBUG
					 	osParser << "File not found\nTry to open params.json file from example directory (./example/)" << std::endl;
					#endif
					params_json = TracesParser::loadFile("./example/params.json");
					if (params_json.compare("") != 0){
						#ifdef DEBUG
						 	osParser << "File found and used for compression and analysis." << std::endl;
						#endif
						Call::callMaps.initMaps(params_json, tokens[ind+1]);
					} else {
						#ifdef DEBUG
							osParser << "File not found\nUsing default compression options." << std::endl;
						#endif
					}
				}
			}
		}
		else if (tokens[ind].compare(EXECUTION_END_TIME) == 0) {
			t = new EndExecutionEvent(stoi(tokens[ind+1]));
		}
		else if (INCLUDE_EVENTS == 1 && Trace::inArray(tokens[ind].c_str(), Event::concatEventsArr) > -1) {
			t = new Event(tokens[ind]);
		}
		// Gestion des appels dans la cas où le fichier params n'a pas été chargé ou l'appel est inconnu ou c'est un appel de fonction connu sans paramètre
		else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_NO_PARAMS) == 0) {
			t = new CallWithNoParam(tokens[ind]);
		}
		else {
			// Dans ce cas, nous sommes sur un appel de fonction avec paramètre
			// On vérifie si une erreur a été générée
			Call::ErrorType err = Call::getEnumType<Call::ErrorType>(tokens[ind].c_str(),Call::errorsArr);
			if (err != Call::NONE) {
				ind++;
			}
			// Gestion des appels ne prenant en paramètre un entier
			if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_INT_PARAM) == 0) {
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
				// Gestion des appel prenant une unité, un entier et une seconde unité en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_UNIT_PARAMS) == 0) {
					int p = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1, targetId = -1, targetType = -1;
					std::vector<std::string> targetParamTokens;
					if (ind+3 <= numTokens-1) {
						targetParamTokens = splitLine(tokens[ind+3],'_');
						targetId = stoi(targetParamTokens[0]);
						if (targetParamTokens.size() == 2)
							targetType = stoi(targetParamTokens[1]);
					}
					t = new CallWithUnitIntUnitParams(err,tokens[ind],unitId,unitType,p,targetId,targetType);
				}
				// Gestion des appel prenant une unité, un entier et une position en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_POS_PARAMS) == 0) {
					int p = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1;
					float x = (ind+3 <= numTokens-1) ? stof(tokens[ind+3]) : -1, y = (ind+4 <= numTokens-1) ? stof(tokens[ind+4]) : -1;
					t = new CallWithUnitIntPosParams(err,tokens[ind],unitId,unitType,p,x,y);
				}
				// Gestion des appel prenant une unité et deux entiers en paramètre
				else if (Call::callMaps.getCallType(tokens[ind]).compare(CALL_WITH_UNIT_INT_FLOAT_PARAMS) == 0) {
					int p1 = (ind+2 <= numTokens-1) ? stoi(tokens[ind+2]) : -1;
					float p2 = (ind+3 <= numTokens-1) ? stof(tokens[ind+3]) : -1;
					t = new CallWithUnitIntFloatParams(err,tokens[ind],unitId,unitType,p1,p2);
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

void TracesParser::detectSequences() {
	unsigned int i, j, seq_start, seq_end, max_length = 2;
    bool found, climb = false;
	Sequence::sp_sequence sps_up, sps_down, sps_res;
	while(max_length <= Trace::getLength(traces,start) / 2) {
		#ifdef DEBUG
			osParser << "max size : " << max_length << std::endl;
		#endif
		i = start;
		seq_end = start;
		while (i < traces.size()) {
			if (traces.at(i)->isEvent()) {
				i++;
				continue;
			}
			if (traces.at(i)->indSearch == -1) {
				traces.at(i)->indSearch = i-1;
				traces.at(i)->lenSearch = traces.at(i-1)->length();
			}
			#ifdef DEBUG
				osParser << "search from : " << std::endl;
				traces.at(i)->display(osParser);
				osParser << std::endl;
			#endif
			while (i < traces.size() && traces.at(i)->lenSearch <= max_length && traces.at(i)->indSearch >= (int)seq_end && traces.at(i)->endSearch < MAX_END_SEARCH) {
				if (traces.at(i)->lenSearch >= 2) {
					sps_up = boost::make_shared<Sequence>(1);
					seq_start = traces.at(i)->indSearch;
					j = seq_start;
					while(j < i)
						sps_up->addTrace(traces.at(j++));
					if (checkFeasibility(sps_up->length(),j)) {
						#ifdef DEBUG
							osParser << "sps_up : " << std::endl;
							sps_up->display(osParser);
							osParser << std::endl;
						#endif
						found = true;
						while (found) {
							sps_down = boost::make_shared<Sequence>(1);
							while(j < traces.size() && sps_down->length() < sps_up->length())
								sps_down->addTrace(traces.at(j++));
							#ifdef DEBUG
								osParser << "sps_down : " << std::endl;
								sps_down->display(osParser);
								osParser << std::endl;
							#endif
							sps_res = mergeSequences(sps_up, sps_down);
							if (sps_res) {
								sps_up = sps_res;
								seq_end = j;
							}
							else
								found = false;
						}
						if (sps_up->getNum() >= 2) {
							#ifdef DEBUG
								osParser << "seq_start : " << seq_start << std::endl;
								osParser << "seq_end : " << seq_end << std::endl;
							#endif
							sps_up->checkDelayed();
							traces.erase(traces.begin() + seq_start, traces.begin() + seq_end);
							traces.insert(traces.begin() + seq_start, sps_up);
							seq_end = seq_start + 1;
							std::vector<Trace::sp_trace>::iterator it = traces.begin() + seq_end;
							while(it != traces.end())
								(*it++)->indSearch = -1;
							i = seq_start + max_length;
							// i = seq_start + max_length + 1
							#ifdef DEBUG
								osParser << "seq_end : " << seq_end << std::endl;
								osParser << "i : " << i << std::endl;
								it = traces.begin();
								while (it != traces.end())
									(*it++)->display(osParser);
								osParser << std::endl;
							#endif
						}
						else {
							#ifdef DEBUG
								osParser << "false sequence" << std::endl;
							#endif
							climb = true;
						}
					}
					else {
						#ifdef DEBUG
							osParser << "not enough traces for sequence" << std::endl;
						#endif
						traces.at(i)->endSearch = MAX_END_SEARCH;
						break;
					}
				}
				else {
					climb = true;
				}
				if (climb && traces.at(i)->indSearch > 0) {
					traces.at(i)->indSearch--;
					traces.at(i)->lenSearch += traces.at(traces.at(i)->indSearch)->length();
					climb = false;
				}
			}
			i++;
		}
		max_length++;
	}
}

bool TracesParser::checkFeasibility(unsigned int min_length, unsigned int ind_start) {
	unsigned int len = 0;
	for (unsigned int i = ind_start; len < min_length && i < traces.size(); i++) {
		if (traces.at(i)->isEvent() && Trace::inArray(boost::dynamic_pointer_cast<Event>(traces.at(i))->getLabel().c_str(), Event::noConcatEventsArr) > -1)
			break;
		len += traces.at(i)->length();
	}
	return len >= min_length;
}

/*
 * Constructs the most generalised sequence from seq_up and seq_down. seq_up and seq_down must have the same length and must be equals.
 *
 * Returns a shared_ptr object which contains a pointer to the new sequence if the construction has succeeded, or a NULL pointer otherwise.
 *
 */
Sequence::sp_sequence TracesParser::mergeSequences(Sequence::sp_sequence sps_up, Sequence::sp_sequence sps_down) {
	Sequence::sp_sequence sps;
	if (sps_up->compare(sps_down.get())) {
		#ifdef DEBUG
			osParser << "starting merge" << std::endl;
			sps_up->display(osParser);
			sps_down->display(osParser);
			osParser << std::endl;
		#endif
		// the two sequences are equals
		sps_up->reset();
		sps_down->reset();
		unsigned int len_up, len_down, pop = 0;
		bool next_up = false, next_down = false;
		sps = boost::make_shared<Sequence>(sps_up);
		sps->addOne();
		std::stack<Sequence::sp_sequence> newStack;
		newStack.push(sps);
		std::stack<Sequence::sp_sequence> upStack;
		upStack.push(sps_up);
		std::stack<Sequence::sp_sequence> downStack;
		downStack.push(sps_down);
		std::vector<Trace::sp_trace> events;
		Trace::sp_trace spt_up = sps_up->next();
		while (!sps_up->isEndReached() && spt_up->isEvent()) {
			events.push_back(spt_up);
			spt_up = sps_up->next();
		}
		Trace::sp_trace spt_down = sps_down->next();
		while (!sps_down->isEndReached() && spt_down->isEvent()) {
			events.push_back(spt_down);
			spt_down = sps_down->next();
		}
		while (!upStack.empty() && !downStack.empty()) {
			if (!spt_up->isSequence() && !spt_down->isSequence()) {
				#ifdef DEBUG
					osParser << "both not sequence" << std::endl;
				#endif
				next_up = true;
				next_down = true;
				if (!events.empty()) {
					for (unsigned int i = 0; i < events.size(); i++)
						sps->addTrace(events.at(i));
					events.clear();
				}
				sps->addTrace(spt_up);
				dynamic_cast<Call*>(spt_up.get())->filterCall(dynamic_cast<const Call*>(spt_down.get()));
			}
			else {
				if (spt_up->isSequence())
					sps_up = boost::dynamic_pointer_cast<Sequence>(spt_up);
				if (spt_down->isSequence())
					sps_down = boost::dynamic_pointer_cast<Sequence>(spt_down);
				len_up = spt_up->length();
				len_down = spt_down->length();
				if (spt_up->isSequence() && spt_down->isSequence() && len_up == len_down) {
					#ifdef DEBUG
						osParser << "both sequence and same length" << std::endl;
					#endif
					sps = boost::make_shared<Sequence>(sps_up,sps_down);
					upStack.push(sps_up);
					downStack.push(sps_down);
					next_up = true;
					next_down = true;
				}
				else if ((spt_up->isSequence() && !spt_down->isSequence()) || len_up > len_down) {
					#ifdef DEBUG
						osParser << "push up" << std::endl;
					#endif
					sps = boost::make_shared<Sequence>(sps_up);
					if (spt_up->isSequence() && !spt_down->isSequence())
						sps->updateNumMap(1,1);
					upStack.push(sps_up);
					next_up = true;
				}
				else if ((!spt_up->isSequence() && spt_down->isSequence()) || len_up < len_down) {
					#ifdef DEBUG
						osParser << "push down" << std::endl;
					#endif
					sps = boost::make_shared<Sequence>(sps_down);
					if (!spt_up->isSequence() && spt_down->isSequence())
						sps->updateNumMap(1,1);
					downStack.push(sps_down);
					next_down = true;
				}
				newStack.push(sps);
			}
			while (next_up && !upStack.empty() && sps_up->isEndReached()) {
				#ifdef DEBUG
					osParser << "pop up" << std::endl;
				#endif
				upStack.pop();
				if (!upStack.empty()) {
					sps_up = upStack.top();
					pop++;
				}
			}
			while (next_down && !downStack.empty() && sps_down->isEndReached()) {
				#ifdef DEBUG
					osParser << "pop down" << std::endl;
				#endif
				downStack.pop();
				if (!downStack.empty()) {
					sps_down = downStack.top();
					pop++;
				}
			}
			#ifdef DEBUG
				osParser << "pop value : " << pop << std::endl;
			#endif
			while (pop > 0) {
				if (sps->isShared()) {
					pop -= 2;
					#ifdef DEBUG
						osParser << "pop_new (shared)" << std::endl;
					#endif
				}
				else {
					pop--;
					#ifdef DEBUG
						osParser << "pop_new" << std::endl;
					#endif
				}
				newStack.pop();
				newStack.top()->addTrace(sps);
				sps->completeNumMap();
				sps = newStack.top();
			}
			if (next_up && !upStack.empty()) {
				spt_up = sps_up->next();
				while (!sps_up->isEndReached() && spt_up->isEvent()) {
					events.push_back(spt_up);
					spt_up = sps_up->next();
				}
				next_up = false;
			}
			if (next_down && !downStack.empty()) {
				spt_down = sps_down->next();
				while (!sps_down->isEndReached() && spt_down->isEvent()) {
					events.push_back(spt_down);
					spt_down = sps_down->next();
				}
				next_down = false;
			}
			#ifdef DEBUG
				osParser << "end" << std::endl;
			#endif
		}
		#ifdef DEBUG
			osParser << "merge result" << std::endl;
			sps->display(osParser);
			osParser << std::endl;
		#endif
	}
	return sps;
}

void TracesParser::handleTraceOffline(Trace::sp_trace& spt) {
	bool add = false;
	if (traces.size() > 0) {
		Sequence::sp_sequence sps;
		if (traces.back()->isSequence()) {
			sps = boost::dynamic_pointer_cast<Sequence>(traces.back());
			if (sps->at(0)->operator==(spt.get())) {
				sps->addOne();
				add = true;
			}
			else if (spt->isEvent()) {
				sps->addTrace(spt);
				add = true;
			}
		}
		else if (traces.back()->isCall() && traces.back()->operator==(spt.get())) {
			sps = boost::make_shared<Sequence>(2);
			sps->addTrace(traces.back());
			dynamic_cast<Call*>(traces.back().get())->filterCall(dynamic_cast<const Call*>(spt.get()));
			traces.pop_back();
			traces.push_back(sps);
			add = true;
		}
	}
	if (!add)
		traces.push_back(spt);
}

bool TracesParser::reachLastStart() {
	std::string line;
	int pos = 0, start_pos = -1, cpt = 1;
	while (getline(ifs, line)) {
		std::vector<std::string> tokens = splitLine(line);
		if (tokens[0].compare(GAME_START) == 0) {
			start_pos = pos;
			lineNum = cpt;
		}
		pos = ifs.tellg();
		cpt++;
	}
	ifs.clear();
	if (start_pos != -1) {
		ifs.seekg(start_pos);
		//just at the begin of the last 'start' of the file (if there is at least one)
		return true;
	}
	return false;
}

void TracesParser::display(std::ostream &os) {
	int num_start = 0;
	Event *e = NULL;
	for (unsigned int i = 0; i < traces.size(); i++) {
		e = traces.at(i)->isEvent() ? dynamic_cast<Event*>(traces.at(i).get()) : NULL;
		if (e != NULL && Trace::inArray(e->getLabel().c_str(), Event::noConcatEventsArr) > -1) {
			if (e->getLabel().compare(START_MISSION) == 0) {
				if (num_start++ > 0)
					os << std::endl;
				StartMissionEvent *sme = dynamic_cast<StartMissionEvent*>(e);
				os << "mission name : " << sme->getMissionName() << std::endl << "mission start time : " << sme->getStartTime() << std::endl;
			}
			else if (e->getLabel().compare(END_MISSION) == 0) {
				EndMissionEvent *eme = dynamic_cast<EndMissionEvent*>(e);
				os << "status : " << eme->getStatus() << std::endl << "mission end time : " << eme->getEndTime() << std::endl;
			}
			else if (e->getLabel().compare(NEW_EXECUTION) == 0) {
				NewExecutionEvent *nee = dynamic_cast<NewExecutionEvent*>(e);
				os << "\texecution start time : " << nee->getStartTime() << std::endl << "\tprogramming_language_used " << nee->getProgrammingLangageUsed() << std::endl;
				e->numTab = 1;
			}
			else if (e->getLabel().compare(END_EXECUTION) == 0) {
				os << "\texecution end time : " << dynamic_cast<EndExecutionEvent*>(e)->getEndTime() << std::endl;
				e->numTab = 0;
			}
		}
		else
			traces.at(i)->display(os);
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
			#ifdef DEBUG
				os << "begin import from XML file" << std::endl;
			#endif
			importTraceFromNode(root_node->first_node(),traces);
			for (unsigned int i = 0; i < traces.size(); i++){
				#ifdef DEBUG
				 	os << i << " " << traces.at(i) << std::endl;
				#endif
				traces.at(i)->display(os);
			}
		}
	}
	catch (const std::runtime_error& e) {
		#ifdef DEBUG
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
				// if (!seq_stack.empty()) {
					// sps->setParent(seq_stack.top());
				// }
				if (node->first_attribute("num_map") != 0) {
					std::vector<std::string> tokens = splitLine(node->first_attribute("num_map")->value());
					for (unsigned int i = 0; i < tokens.size(); i++) {
						int pos = tokens.at(i).find(":",0);
						sps->updateNumMap(stoi(std::string(tokens.at(i).begin(),tokens.at(i).begin()+pos)),stoi(std::string(tokens.at(i).begin()+pos+1,tokens.at(i).end())));
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
				Trace::sp_trace spt = handleLine(s);
				if (!seq_stack.empty()) {
					sps->addTrace(spt);
					//spt->setParent(sps);
				}
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

void TracesParser::exportTraceToXml() {
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
	std::string s;
	unsigned int i = 0;
	bool pass = false;
	while (i < traces.size() || !seq_stack.empty()) {
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
		if (seq_stack.empty() && i < traces.size()) {
			spt = traces.at(i++);
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
				node->append_attribute(doc.allocate_attribute("num_map", doc.allocate_string(Sequence::getNumMapString(sps->getNumMap()).c_str())));
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
		#ifdef DEBUG
		 	osParser << "error boost::lexical_cast<int>" << std::endl;
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
		#ifdef DEBUG
		 	osParser << "error boost::lexical_cast<float>" << std::endl;
		#endif
		exit(EXIT_FAILURE);
	}
	return res;
}
