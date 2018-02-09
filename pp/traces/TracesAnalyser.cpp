#include "TracesAnalyser.h"
#include "TracesParser.h"


#define DEBUG
#define LOG_IN_FILE

#ifdef DEBUG
	#ifdef LOG_IN_FILE
		std::ofstream debugOfsAnalyser("debugAnalyser.log", std::ofstream::out);
		std::ostream& osAnalyser = debugOfsAnalyser;
	#else
		std::ostream& osAnalyser = std::cout;
	#endif
#endif

const char* TracesAnalyser::feedbackTypesArr[] = {"useful_call", "useless_call", "seq_extra", "seq_lack", "ind_seq_num", "dist_seq_num", "call_extra", "call_lack", "call_params", NULL};
std::map<std::string,std::string> TracesAnalyser::messages_map;

TracesAnalyser::TracesAnalyser(std::string lang): endless_loop(false), loaded(false), lang(lang) {}

void TracesAnalyser::loadXmlInfos(const std::string& feedbacks_xml, const std::string& mission_feedbacks_xml) {
	if (!loaded) {
		//Parse default feedbacks.xml
		try {
			std::vector<char> xml_content(feedbacks_xml.begin(), feedbacks_xml.end());
			xml_content.push_back('\0');
			rapidxml::xml_document<> feedbackDoc;
			feedbackDoc.parse<0>(&xml_content[0]);
			// Initialise messages_map
			importMessagesFromXml(&feedbackDoc);
			// Fill ref_feedbacks with default feedbacks
			importFeedbacksFromXml(&feedbackDoc);
			loaded = true;
		}
		catch (const std::runtime_error& e) {
			osAnalyser << "import from feedbacks xml file : " << e.what() << std::endl;
		}
		//Parse mission feedbacks.xml
		osAnalyser << "Parse mission feedbacks.xml" << std::endl;
		if (!mission_feedbacks_xml.empty()) {
			try {
				std::vector<char> xml_content(mission_feedbacks_xml.begin(), mission_feedbacks_xml.end());
				xml_content.push_back('\0');
				rapidxml::xml_document<> mFeedbackDoc;
				mFeedbackDoc.parse<0>(&xml_content[0]);
				// Fill ref_feedbacks with mission feedbacks
				importFeedbacksFromXml(&mFeedbackDoc);
			}
			catch (const std::runtime_error& e) {
				osAnalyser << "import from mission feedbacks xml file : " << e.what() << std::endl;
			}
		}
		osAnalyser << "num ref feedbacks : " << ref_feedbacks.size() << std::endl;
	}
}

// import feedbacks from xml file to ref_feedbacks vector
void TracesAnalyser::importFeedbacksFromXml(rapidxml::xml_document<> *doc) {
	osAnalyser << "import feedbacks from xml" << std::endl;
	rapidxml::xml_node<> *root_node = doc->first_node("root");
	if (root_node)
		root_node = root_node->first_node("feedbacks");
	else
		root_node = doc->first_node("feedbacks");
	if (root_node != 0) {
		for (rapidxml::xml_node<> *feedback_node = root_node->first_node(); feedback_node; feedback_node = feedback_node->next_sibling()) {
			if (feedback_node->first_attribute("type") != 0 && feedback_node->first_attribute("priority") != 0) {
				Feedback f;
				f.type = Call::getEnumType<FeedbackType>(feedback_node->first_attribute("type")->value(),feedbackTypesArr);
				f.priority = std::atoi(feedback_node->first_attribute("priority")->value());
				f.level = (feedback_node->first_attribute("level") != 0) ? std::atoi(feedback_node->first_attribute("level")->value()) : -1;
				rapidxml::xml_node<> *node = feedback_node->first_node("infos");
				while(node && node->first_attribute("lang") != 0 && std::string(node->first_attribute("lang")->value()).compare(lang) != 0)
					node = node->next_sibling();
				if (node) {
					int r = getRandomIntInRange(TracesParser::getNodeChildCount(node));
					for (rapidxml::xml_node<> *info_node = node->first_node("info"); info_node; info_node = info_node->next_sibling(), r--) {
						if (r == 0)
							f.info = info_node->value();
					}
					if (!f.info.empty()) {
						node = feedback_node->first_node("learner");
						if (node != 0) {
							if (TracesParser::getNodeChildCount(node) == 1) {
								std::vector<Trace::sp_trace> traces;
								TracesParser::importTraceFromNode(node->first_node(),traces);
								f.learner_spt = traces.at(0);
							}
							else
								throw std::runtime_error("learner node can have only one trace");
						}
						node = feedback_node->first_node("expert");
						if (node != 0) {
							if (TracesParser::getNodeChildCount(node) == 1) {
								std::vector<Trace::sp_trace> traces;
								TracesParser::importTraceFromNode(node->first_node(),traces);
								f.expert_spt = traces.at(0);
							}
							else
								throw std::runtime_error("expert node can have only one trace");
						}
						ref_feedbacks.push_back(f);
					}
				}
			}
		}
	}
}

void TracesAnalyser::importMessagesFromXml(rapidxml::xml_document<> *doc) {
	osAnalyser << "import messages from xml" << std::endl;
	rapidxml::xml_node<> *root_node = doc->first_node("root")->first_node("messages");
	if (root_node != 0) {
		for (rapidxml::xml_node<> *message_node = root_node->first_node(); message_node; message_node = message_node->next_sibling()) {
			if (message_node->first_attribute("id") != 0) {
				rapidxml::xml_node<> *node = message_node->first_node("infos");
				while(node && node->first_attribute("lang") != 0 && std::string(node->first_attribute("lang")->value()).compare(lang) != 0)
					node = node->next_sibling();
				if (node) {
					std::string value;
					int r = getRandomIntInRange(TracesParser::getNodeChildCount(node));
					for (rapidxml::xml_node<> *info_node = node->first_node("info"); info_node; info_node = info_node->next_sibling(), r--) {
						if (r == 0)
							value = info_node->value();
					}
					if (!value.empty())
						messages_map.insert(std::make_pair<std::string,std::string>(message_node->first_attribute("id")->value(),value));
				}
			}
		}
	}
}

int TracesAnalyser::getRandomIntInRange(int max) {
	srand(time(NULL));
	return rand() % max;
}

std::string TracesAnalyser::constructFeedback(const std::string& learner_xml, const std::vector<std::string>& experts_xml, int ind_mission, int ind_execution) {
	rapidjson::Document doc;
	rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
	doc.SetObject();
	double best_score = 0;
	std::vector<Trace::sp_trace> learner_traces = TracesParser::importTraceFromXml(learner_xml, osAnalyser);
	if (getInfosOnMission(learner_traces, learner_gi, ind_mission)) {
		int ind_best = -1;
		bool reimport = false;
		if (getInfosOnExecution(learner_gi, ind_execution)) {
			for(unsigned int i = 0; i < experts_xml.size(); i++) {
				if (reimport) {
					learner_traces = TracesParser::importTraceFromXml(learner_xml, osAnalyser);
					getInfosOnMission(learner_traces, learner_gi, ind_mission);
					getInfosOnExecution(learner_gi, ind_execution);
				}
				std::vector<Trace::sp_trace> expert_traces = TracesParser::importTraceFromXml(experts_xml.at(i), osAnalyser);
				// filtrage des solutions expertes non compatibles avec le langage de programmation utilisé par le joueur
				if (getInfosOnMission(expert_traces, expert_gi) && getInfosOnExecution(expert_gi) && expert_gi.nee->getProgrammingLangageUsed().compare(learner_gi.nee->getProgrammingLangageUsed()) == 0) {
					osAnalyser << "\nexpert solution analysed: "+expert_gi.nee->getProgrammingLangageUsed() << std::endl;
					Call::call_vector expert_calls = expert_gi.root_sps->getCalls(true);
					for (unsigned int j = 0; j < expert_calls.size(); j++) {
						if (experts_calls_freq.find(expert_calls.at(j)->getKey()) != experts_calls_freq.end())
							experts_calls_freq.at(expert_calls.at(j)->getKey())++;
						else
							experts_calls_freq.insert(std::make_pair<std::string,double>(expert_calls.at(j)->getKey(),1));
					}
					reimport = addImplicitSequences(learner_gi.root_sps, expert_gi.root_sps);
					if (reimport) {
						osAnalyser << "learner traces have been modified\n" << std::endl;
						learner_gi.root_sps->exportAsString(osAnalyser);
					}
					else
						osAnalyser << "learner traces have not been modified\n" << std::endl;
					if (addImplicitSequences(expert_gi.root_sps, learner_gi.root_sps)) {
						osAnalyser << "expert traces have been modified\n" << std::endl;
						expert_gi.root_sps->exportAsString(osAnalyser);
					}
					else
						osAnalyser << "expert traces have not been modified\n" << std::endl;

					// Align roots
					learner_gi.root_sps->setAligned(expert_gi.root_sps);
					expert_gi.root_sps->setAligned(learner_gi.root_sps);

					std::pair<double,double> res = findBestAlignment(learner_gi.root_sps->getTraces(), expert_gi.root_sps->getTraces(), false);
					double score = res.first / res.second;
					osAnalyser << "gross score : " << res.first << std::endl;
					osAnalyser << "norm value : " << res.second << std::endl;
					osAnalyser << "similarity score : " << score << "\n" << std::endl;
					if (score >= best_score) {
						best_score = score;
						ind_best = i;
					}
				}
			}
		}

		// Si une solution experte a été trouvée
		if (ind_best > -1) {
			std::map<std::string,double>::iterator it = experts_calls_freq.begin();
			while (it != experts_calls_freq.end())
				(it++)->second /= experts_xml.size();
			osAnalyser << "\nexpert program " << ind_best << " has been chosen for alignment with learner traces" << std::endl;
			osAnalyser << "similarity score: " << best_score << std::endl;
			std::vector<Trace::sp_trace> expert_traces = TracesParser::importTraceFromXml(experts_xml.at(ind_best), osAnalyser);
			if (getInfosOnMission(expert_traces, expert_gi) && getInfosOnExecution(expert_gi)) {
				if (reimport) {
					learner_traces = TracesParser::importTraceFromXml(learner_xml, osAnalyser);
					getInfosOnMission(learner_traces, learner_gi, ind_mission);
					getInfosOnExecution(learner_gi, ind_execution);
				}
				addImplicitSequences(learner_gi.root_sps, expert_gi.root_sps);
				addImplicitSequences(expert_gi.root_sps, learner_gi.root_sps);

				// Align roots
				learner_gi.root_sps->setAligned(expert_gi.root_sps);
				expert_gi.root_sps->setAligned(learner_gi.root_sps);

				findBestAlignment(learner_gi.root_sps->getTraces(), expert_gi.root_sps->getTraces());
				displayAlignment(learner_gi.root_sps->getTraces(), expert_gi.root_sps->getTraces());
			}

			int num_attempts = learner_gi.getNumExecutions();
			doc.AddMember("num_attempts", num_attempts, allocator); // nombre de tentatives
			double time = learner_gi.getExecutionTime();
			if (time != -1)
				doc.AddMember("execution_time", time, allocator); // temps d'execution de la derniere tentative
			time = expert_gi.getExecutionTime();
			if (time != -1)
				doc.AddMember("ref_execution_time", time, allocator); // temps d'execution reference
			if (learner_gi.eme != NULL) {
				if (learner_gi.eme->getStatus().compare("won") == 0) {
					time = learner_gi.getResolutionTime();
					if (time != -1)
						doc.AddMember("resolution_time", time, allocator); // temps de resolution de la mission
					time = expert_gi.getResolutionTime();
					if (time != -1)
						doc.AddMember("ref_resolution_time", time, allocator); // temps de resolution reference
				}
				time = learner_gi.getAverageWaitTime();
				if (time != -1)
					doc.AddMember("exec_mean_wait_time", time, allocator); // temps d'attente moyen entre deux tentatives
				doc.AddMember("won", learner_gi.eme->getStatus().compare("won") == 0, allocator); // victoire / defaite
			}
			doc.AddMember("score", std::floor(best_score * 100), allocator); // score

			if (loaded) {
				if (num_attempts == 0) {
					std::string msg;
					if (messages_map.find("no_execution_detected") != messages_map.end())
						msg = messages_map.at("no_execution_detected");
					else
						msg = "No execution has been detected";
					rapidjson::Value arrWarnings(rapidjson::kArrayType);
					rapidjson::Value f(msg.c_str(), msg.size(), allocator);
					arrWarnings.PushBack(f, allocator);
					doc.AddMember("warnings", arrWarnings, allocator);
				}
				else {
					if (endless_loop) {
						std::string msg;
						if (learner_gi.eme != NULL && messages_map.find("endless_loop") != messages_map.end())
							msg = messages_map.at("endless_loop");
						else if (learner_gi.eme == NULL && messages_map.find("probable_endless_loop") != messages_map.end())
							msg = messages_map.at("probable_endless_loop");
						else
							msg = "Endless loop detected";
						rapidjson::Value arrWarnings(rapidjson::kArrayType);
						rapidjson::Value f(msg.c_str(), msg.size(), allocator);
						arrWarnings.PushBack(f, allocator);
						doc.AddMember("warnings", arrWarnings, allocator);
					}
					if (!ref_feedbacks.empty()) {
						if (!feedbacks.empty())
							feedbacks.clear();
						listAlignmentFeedbacks(learner_gi.root_sps->getTraces(), expert_gi.root_sps->getTraces());
						listGlobalFeedbacks();
						bindFeedbacks();
						std::sort(feedbacks.begin(), feedbacks.end());
						filterFeedbacks();

						unsigned int num_max_downgrads = std::max(1, num_attempts / NUM_DOWNGRADS), cpt_downgrads = 0, cpt_feedbacks = 0;
						osAnalyser << "num_max_downgrads : " << num_max_downgrads << std::endl;
						rapidjson::Value arrInfos(rapidjson::kArrayType);
						osAnalyser << "complete list of feedbacks" << std::endl;
						osAnalyser << "_________" << std::endl;
						for(unsigned int i = 0; i < feedbacks.size(); i++) {
							osAnalyser << "feedback num " << i << std::endl;
							if (cpt_feedbacks < NUM_MAX_FEEDBACKS) {
								osAnalyser << "num_max_feedbacks not reached" << std::endl;
								if (i > 1 && feedbacks.at(i).priority > feedbacks.at(i-1).priority)
									cpt_downgrads++;
								if (cpt_downgrads <= num_max_downgrads) {
									rapidjson::Value f(feedbacks.at(i).info.c_str(), feedbacks.at(i).info.size(), allocator);
									arrInfos.PushBack(f, allocator);
									cpt_feedbacks++;
									osAnalyser << "feedback added" << std::endl;
								}
							}
							feedbacks.at(i).exportAsString(osAnalyser);
						}
						osAnalyser << "_________" << std::endl;

						doc.AddMember("feedbacks", arrInfos, allocator);
					}
				}
			}
		} else{
			std::string lang;
			if (learner_gi.nee != NULL)
				lang = learner_gi.nee->getProgrammingLangageUsed();
			else
				lang ="undefined";
			osAnalyser << "No expert trace match to analyse " << lang << " learner solution." << std::endl;
		}
	}
	rapidjson::StringBuffer s;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
	doc.Accept(writer);

	#ifdef DEBUG
		#ifdef LOG_IN_FILE
			if (debugOfsAnalyser.is_open())
				debugOfsAnalyser.close();
		#endif
	#endif

	return s.GetString();
}

void TracesAnalyser::setEndlessLoop(bool endless_loop) {
	this->endless_loop = endless_loop;
}

void TracesAnalyser::setLang(std::string lang) {
	this->lang = lang;
}

bool TracesAnalyser::getInfosOnMission(const std::vector<Trace::sp_trace>& traces, GameInfos& gi, int ind_mission) {
	if (!traces.empty()) {
		int ind_start = 0, ind_end = -1, cpt_mission = 0;
		for (int i = 0; i < (int)traces.size(); i++) {
			if (traces.at(i)->isEvent() && dynamic_cast<Event*>(traces.at(i).get())->getLabel().compare(START_MISSION) == 0) {
				ind_start = i;
				if ((++cpt_mission)-1 == ind_mission)
					break;
			}
		}
		if (cpt_mission-1 == ind_mission || ind_mission == -1) {
			gi.clearMission();
			for (int i = ind_start + 1; i < (int)traces.size(); i++) {
				if (traces.at(i)->isEvent()) {
					Event *e = dynamic_cast<Event*>(traces.at(i).get());
					if (e->getLabel().compare(START_MISSION) == 0 || e->getLabel().compare(END_MISSION) == 0) {
						ind_end = i;
						if (e->getLabel().compare(END_MISSION) == 0)
							gi.eme = dynamic_cast<EndMissionEvent*>(traces.at(i).get());
						break;
					}
				}
			}
			gi.sme = dynamic_cast<StartMissionEvent*>(traces.at(ind_start).get());
			if (ind_end == -1)
				ind_end = traces.size();
			if (++ind_start < ind_end)
				gi.mission_traces.assign(traces.begin() + ind_start, traces.begin() + ind_end);
			return true;
		}
	}
	return false;
}

bool TracesAnalyser::getInfosOnExecution(GameInfos& gi, int ind_execution) {
	std::vector<Trace::sp_trace>& m_traces = gi.mission_traces;
	if (!m_traces.empty()) {
		int ind_start = 0, ind_end = -1, cpt_execution = 0;
		for (int i = 0; i < (int)m_traces.size(); i++) {
			if (m_traces.at(i)->isEvent()) {
				Event *e = dynamic_cast<Event*>(m_traces.at(i).get());
				if (e->getLabel().compare(NEW_EXECUTION) == 0) {
					ind_start = i;
					if ((++cpt_execution)-1 == ind_execution)
						break;
				}
				else if (e->getLabel().compare(START_MISSION) == 0)
					break;
			}
		}
		if (cpt_execution-1 == ind_execution || ind_execution == -1) {
			gi.clearExecution();
			for (int i = ind_start + 1; i < (int)m_traces.size(); i++) {
				if (m_traces.at(i)->isEvent()) {
					Event *e = dynamic_cast<Event*>(m_traces.at(i).get());
					if (e->getLabel().compare(START_MISSION) == 0 || e->getLabel().compare(NEW_EXECUTION) == 0 || e->getLabel().compare(END_EXECUTION) == 0) {
						ind_end = i;
						if (e->getLabel().compare(END_EXECUTION) == 0)
							gi.eee = dynamic_cast<EndExecutionEvent*>(m_traces.at(i).get());
						break;
					}
				}
			}
			gi.nee = dynamic_cast<NewExecutionEvent*>(m_traces.at(ind_start).get());
			if (ind_end == -1)
				ind_end = m_traces.size();
			if (++ind_start < ind_end) {
				// creation of the root sequence
				gi.root_sps = boost::make_shared<Sequence>(1,true);
				for (int j = ind_start; j < ind_end; j++) {
					gi.root_sps->addTrace(m_traces.at(j));
				}
				return true;
			}
		}
	}
	return false;
}

bool TracesAnalyser::addImplicitSequences(Sequence::sp_sequence& mod_sps, Sequence::sp_sequence& ref_sps) const {
	osAnalyser << "mod : " << std::endl;
	mod_sps->exportAsString(osAnalyser);
	osAnalyser << "ref : " << std::endl;
	ref_sps->exportAsString(osAnalyser);
	Trace::sp_trace spt;
	Sequence::sp_sequence sps = ref_sps;
	sps->reset();
	std::stack<Sequence::sp_sequence> stack;
	stack.push(ref_sps);
	bool change = false;
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
				stack.push(sps);
				osAnalyser << "sps" << std::endl;
				sps->exportAsString(osAnalyser);
				Call::call_vector pattern = sps->getCalls();
				// looking for the pattern in mod_sps calls
				std::vector<Call::call_vector> patterns = getPatterns(mod_sps,pattern);
				osAnalyser << "patterns size : " << patterns.size() << std::endl;
				for (unsigned int i = 0; i < patterns.size(); i++) {
					// for each pattern found in mod_sps calls we search a common parent with the maximum level in the tree
					const Sequence::sp_sequence common_parent = getClosestCommonParent(patterns.at(i));
					common_parent->exportAsString(osAnalyser);
					if ((common_parent->isRoot() || common_parent->length() > sps->length()) && common_parent->getLevel() == sps->getLevel()-1) {
						change = true;
						Sequence::sp_sequence new_sps = boost::make_shared<Sequence>(1);
						// adds will contain the traces we have to add in new_sps
						std::vector<Trace::sp_trace> adds;
						for (unsigned int j = 0; j < patterns.at(i).size(); j++) {
							Trace::sp_trace add = patterns.at(i).at(j);
							Sequence::sp_sequence parent = boost::dynamic_pointer_cast<Sequence>(add->getParent());
							while (parent != common_parent) {
								add = parent;
								parent = boost::dynamic_pointer_cast<Sequence>(parent->getParent());
							}
							if (std::find(adds.begin(), adds.end(), add) == adds.end())
								adds.push_back(add);
						}
						// adding adds elements in new_sps and new_sps in common_parent
						std::vector<Trace::sp_trace>& t = common_parent->getTraces();
						for (unsigned int j = 0, h = 0; j < adds.size() && h < t.size();) {
							if (t.at(h) == adds.at(j)) {
								new_sps->addTrace(t.at(h));
								t.erase(t.begin() + h);
								if (j++ == 0) {
									common_parent->addTrace(new_sps,h++);
								}
							}
							else
								h++;
						}
					}
				}
				sps->reset();
			}
		}
	}
	return change;
}

std::vector<Call::call_vector> TracesAnalyser::getPatterns(const Sequence::sp_sequence& mod_sps, const Call::call_vector& pattern) const {
	std::vector<Call::call_vector> patterns;
	Call::call_vector vc = mod_sps->getCalls();
	for (unsigned int i = 0; i < vc.size(); i++) {
		int ind = (int)i;
		for (unsigned int j = 0; ind > -1 && ind < (int)vc.size() && j < pattern.size(); j++) {
			// Utilisation de la distance entre 2 calls
			// double match_score = 1 - vc.at(ind)->getEditDistance(pattern.at(j).get());
			// match_score = TRANSFORM_SCORE(match_score);
			// ind = (match_score <= 0) ? -1 : ind + 1;
			//---
			// Comparaison du label entre deux calls
			ind = (vc.at(ind)->getKey().compare(pattern.at(j)->getKey()) == 0) ? ind + 1 : -1;
		}
		if (ind > -1) {
			osAnalyser << "new pattern found in mod_sps calls at ind " << i << std::endl;
			Call::call_vector v;
			for (unsigned int j = i; j < i + pattern.size() && j < vc.size() ; j++)
				v.push_back(vc.at(j));
			patterns.push_back(v);
			i += pattern.size()-1;
		}
	}
	return patterns;
}

const Sequence::sp_sequence TracesAnalyser::getClosestCommonParent(const Call::call_vector& pattern) const {
	Sequence::sp_sequence parents[pattern.size()];
	bool same = true;
	for (unsigned int i = 0; i < pattern.size(); i++) {
		parents[i] = boost::dynamic_pointer_cast<Sequence>(pattern.at(i)->getParent());
		if (parents[i]->isRoot())
			return parents[i]; // root is the closest common parent
		if (i > 0 && parents[i] != parents[i-1])
			same = false;
	}
	while (!same) {
		unsigned int ind_max = 0, max_level = parents[0]->getLevel();
		for (unsigned int i = 1; i < pattern.size(); i++) {
			unsigned int level = parents[i]->getLevel();
			if (level > max_level) {
				max_level = level;
				ind_max = i;
			}
		}
		parents[ind_max] = boost::dynamic_pointer_cast<Sequence>(parents[ind_max]->getParent());
		if (parents[ind_max]->isRoot())
			return parents[ind_max]; // root is the closest common parent
		same = true;
		for (unsigned int i = 0; i < pattern.size()-1; i++) {
			if (parents[i] != parents[i+1])
				same = false;
		}
	}
	return parents[0];
}

std::pair<double,double> TracesAnalyser::findBestAlignment(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e, bool align) const {
	int cpt_path = 0;
	unsigned int lsize = l.size()+1, esize = e.size()+1;
	double score = 0, norm = 0;
	std::pair<double,double>** val = new std::pair<double,double>*[lsize];
	std::pair<int,int>** ind = new std::pair<int,int>*[lsize];
	char** help = new char*[lsize];
	osAnalyser << "begin findBestAlignment" << std::endl;
	for (unsigned int i = 0; i < l.size(); i++) {
		l.at(i)->exportAsString(osAnalyser);
		if (align)
			l.at(i)->resetAligned();
	}
	osAnalyser << std::endl;
	for (unsigned int j = 0; j < e.size(); j++) {
		e.at(j)->exportAsString(osAnalyser);
		if (align)
			e.at(j)->resetAligned();
	}
	osAnalyser << std::endl;
	for (unsigned int i = 0; i < lsize; i++) {
		val[i] = new std::pair<double,double>[esize];
		ind[i] = new std::pair<int,int>[esize];
		help[i] = new char[esize];
	}
	for (unsigned int i = 0; i < lsize; i++) {
		val[i][0] = std::make_pair<double,double>(ALIGN_GAP_SCORE * i, 0);
		ind[i][0] = std::make_pair<int,int>(i-1,0);
		help[i][0] = 'h';
	}
	for (unsigned int j = 1; j < esize; j++) {
		val[0][j] = std::make_pair<double,double>(ALIGN_GAP_SCORE * j, 0);
		ind[0][j] = std::make_pair<int,int>(0,j-1);
		help[0][j] = 'g';
	}
	for (unsigned int i = 1; i < lsize; i++) {
		for (unsigned int j = 1; j < esize; j++) {
			if (l.at(i-1)->isEvent()) {
				val[i][j] = std::make_pair<double,double>(val[i-1][j].first + ALIGN_GAP_SCORE, 0);
				help[i][j] = 'h';
				ind[i][j] = std::make_pair<int,int>(i-1,j);
				continue;
			}
			if (e.at(j-1)->isEvent()) {
				val[i][j] = std::make_pair<double,double>(val[i][j-1].first + ALIGN_GAP_SCORE, 0);
				help[i][j] = 'g';
				ind[i][j] = std::make_pair<int,int>(i,j-1);
				continue;
			}
			val[i][j] = std::make_pair<double,double>(0,0);
			ind[i][j] = std::make_pair<int,int>(i-1,j-1);
			double match_score = 0;
			if (l.at(i-1)->isCall() && e.at(j-1)->isCall()) {
				Call::sp_call learner_spc = boost::dynamic_pointer_cast<Call>(l.at(i-1));
				Call::sp_call expert_spc = boost::dynamic_pointer_cast<Call>(e.at(j-1));
				match_score = 1 - learner_spc->getEditDistance(expert_spc.get());
				val[i][j].second = match_score;
				match_score = TRANSFORM_SCORE(match_score);
			}
			else if (l.at(i-1)->isSequence() && e.at(j-1)->isSequence()) {
				Sequence::sp_sequence learner_sps = boost::dynamic_pointer_cast<Sequence>(l.at(i-1));
				Sequence::sp_sequence expert_sps = boost::dynamic_pointer_cast<Sequence>(e.at(j-1));
				std::pair<double,double> res = findBestAlignment(learner_sps->getTraces(),expert_sps->getTraces(),false);
				match_score = res.first;
				val[i][j].second = match_score;
				if (expert_sps->hasNumberIterationFixed() && !learner_sps->isImplicit() && !expert_sps->isImplicit()) {
					double mean_dis = learner_sps->getNumMapMeanDistance(expert_sps);
					val[i][j].second += (1 - mean_dis) * (res.second / IND_SEQ_NUM_CONST);
				}
				match_score /= res.second;
				match_score = TRANSFORM_SCORE(match_score);
			}
			else
				match_score = ALIGN_MISMATCH_SCORE;
			val[i][j].first = val[i-1][j-1].first + match_score;
			help[i][j] = 'd';
			if (val[i-1][j].first + ALIGN_GAP_SCORE > val[i][j].first && val[i-1][j].first + ALIGN_GAP_SCORE > val[i][j-1].first + ALIGN_GAP_SCORE) {
				val[i][j].first = val[i-1][j].first + ALIGN_GAP_SCORE;
				val[i][j].second = 0;
				ind[i][j].second++;
				help[i][j] = 'h';
			}
			else if (val[i][j-1].first + ALIGN_GAP_SCORE > val[i][j].first) {
				val[i][j].first = val[i][j-1].first + ALIGN_GAP_SCORE;
				val[i][j].second = 0;
				ind[i][j].first++;
				help[i][j] = 'g';
			}
			if ((val[i][j].first == val[i-1][j].first + ALIGN_GAP_SCORE && val[i][j].first >= val[i][j-1].first + ALIGN_GAP_SCORE) || (val[i][j].first == val[i][j-1].first + ALIGN_GAP_SCORE && val[i][j].first > val[i-1][j].first + ALIGN_GAP_SCORE) || (val[i-1][j].first + ALIGN_GAP_SCORE == val[i][j-1].first + ALIGN_GAP_SCORE && val[i][j-1].first + ALIGN_GAP_SCORE > val[i][j].first))
				cpt_path++;
		}
	}
	for (unsigned int i = 0; i < lsize; i++) {
		for (unsigned int j = 0; j < esize; j++)
			osAnalyser << "(" << val[i][j].first << "," << val[i][j].second << ")\t";
		osAnalyser << std::endl;
	}
	osAnalyser << std::endl;
	for (unsigned int i = 0; i < lsize; i++) {
		for (unsigned int j = 0; j < esize; j++)
			osAnalyser << "(" << ind[i][j].first << "," << ind[i][j].second << ")\t";
		osAnalyser << std::endl;
	}
	osAnalyser << std::endl;
	for (unsigned int i = 0; i < lsize; i++) {
		for (unsigned int j = 0; j < esize; j++)
			osAnalyser << help[i][j] << "\t";
		osAnalyser << std::endl;
	}
	osAnalyser << std::endl;
	TracesAnalyser::path p;
	std::pair<int,int> pind = ind[lsize-1][esize-1];
	score += val[lsize-1][esize-1].second;
	while(pind.first >= 0 && pind.second >= 0) {
		p.push_back(pind);
		score += val[pind.first][pind.second].second;
		pind = ind[pind.first][pind.second];
	}
	std::reverse(p.begin(),p.end());
	osAnalyser << "path : ";
	for (unsigned int i = 0; i < p.size(); i++)
		osAnalyser << "(" << p.at(i).first << "," << p.at(i).second << ") ";
	osAnalyser << std::endl;

	for (unsigned int i = 0; i < p.size(); i++) {
		double norm_val = 1;
		int indi = p.at(i).first, indj = p.at(i).second;
		if ((i < p.size()-1 && indi == p.at(i+1).first) || indi >= (int)l.size()) {
			if (align)
				e.at(indj)->resetAligned();
		}
		else if ((i < p.size()-1 && indj == p.at(i+1).second) || indj >= (int)e.size())	{
			if (align)
				l.at(indi)->resetAligned();
		}
		else {
			if (l.at(indi)->isSequence() && e.at(indj)->isSequence()) {
				Sequence::sp_sequence learner_sps = boost::dynamic_pointer_cast<Sequence>(l.at(indi));
				Sequence::sp_sequence expert_sps = boost::dynamic_pointer_cast<Sequence>(e.at(indj));
				std::pair<double,double> res = findBestAlignment(learner_sps->getTraces(), expert_sps->getTraces(), align);
				norm_val = res.second;
				if (expert_sps->hasNumberIterationFixed() && !learner_sps->isImplicit() && !expert_sps->isImplicit())
					norm_val += res.second / IND_SEQ_NUM_CONST;
			}
			if (align) {
				l.at(indi)->setAligned(e.at(indj));
				e.at(indj)->setAligned(l.at(indi));
			}
		}
		norm += norm_val;
	}

	if (cpt_path == 0)
		osAnalyser << "only one path" << std::endl;
	else
		osAnalyser << "more than one path" << std::endl;
	for (unsigned int i = 0; i < lsize; i++) {
		delete[] val[i];
		delete[] ind[i];
		delete[] help[i];
	}
	delete[] val;
	delete[] ind;
	delete[] help;
	osAnalyser << "end findBestAlignment" << std::endl;
	return std::make_pair<double,double>(score,norm);
}

void TracesAnalyser::displayAlignment(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e) const {
	unsigned int i = 0, j = 0;
	while (i < l.size() || j < e.size()) {
		if (i < l.size() && j < e.size() && l.at(i)->getAligned() && e.at(j)->getAligned()) {
			l.at(i)->exportAsString(osAnalyser);
			osAnalyser << "with" << std::endl;
			l.at(i)->getAligned()->exportAsString(osAnalyser); //l.at(i)->aligned is equal to e.at(j) in this case
			if (l.at(i)->isSequence() && l.at(i)->getAligned()->isSequence()) {
				osAnalyser << "enter both sequence" << std::endl;
				displayAlignment(dynamic_cast<Sequence*>(l.at(i).get())->getTraces(), dynamic_cast<Sequence*>(l.at(i)->getAligned().get())->getTraces());
				osAnalyser << "exit both sequence" << std::endl;
			}
			i++;
			j++;
		}
		else if (i < l.size() && !l.at(i)->getAligned()) {
			l.at(i++)->exportAsString(osAnalyser);
			osAnalyser << "with" << std::endl;
			osAnalyser << "\t-" << std::endl;
		}
		else if (j < e.size() && !e.at(j)->getAligned()) {
			osAnalyser << "\t-" << std::endl;
			osAnalyser << "with" << std::endl;
			e.at(j++)->exportAsString(osAnalyser);
		}
		osAnalyser << std::endl;
	}
}

void TracesAnalyser::bindFeedbacks() {
	osAnalyser << "bind feedbacks begin" << std::endl;
	for (unsigned int i = 0; i < feedbacks.size(); i++) {
		int ind_max = -1;
		double max_score = 0;
		for (unsigned int j = 0; j < ref_feedbacks.size(); j++) {
			if (feedbacks.at(i).type == ref_feedbacks.at(j).type) {
				double score = getFeedbackScore(feedbacks.at(i),j);
				if (score > max_score) {
					max_score = score;
					ind_max = j;
				}
				else if (score == max_score && (ind_max == -1 || ref_feedbacks.at(ind_max).priority > ref_feedbacks.at(j).priority))
					ind_max = j;
				osAnalyser << "---" << std::endl;
				feedbacks.at(i).exportAsString(osAnalyser);
				osAnalyser << std::endl;
				ref_feedbacks.at(j).exportAsString(osAnalyser);
				osAnalyser << std::endl;
				osAnalyser << "score :" << score << std::endl;
				osAnalyser << "---" << std::endl;
			}
		}
		// Set feedback priority
		feedbacks.at(i).priority = ref_feedbacks.at(ind_max).priority;
		// Set Feedback info
		setFeedbackInfo(feedbacks.at(i),ref_feedbacks.at(ind_max));
		// Set defined to true if the feedback is a personnalised feedback (and not a default one)
		feedbacks.at(i).defined = ref_feedbacks.at(ind_max).learner_spt || ref_feedbacks.at(ind_max).expert_spt;
	}
	osAnalyser << "bind feedbacks end" << std::endl;
}

double TracesAnalyser::getFeedbackScore(const Feedback& f, int j) {
	double score[2] = {0,0};
	Trace::sp_trace t[2] = {f.learner_spt,f.expert_spt};
	Trace::sp_trace ref_t[2] = {ref_feedbacks.at(j).learner_spt,ref_feedbacks.at(j).expert_spt};
	for (unsigned int i = 0; i < 2; i++) {
		if (t[i] && ref_t[i]) {
			if (ref_feedbacks.at(j).level == -1 || (int)t[i]->getLevel() == ref_feedbacks.at(j).level) {
				if (t[i]->isCall() && ref_t[i]->isCall()) {
					Call::sp_call spc = boost::dynamic_pointer_cast<Call>(t[i]);
					Call::sp_call ref_spc = boost::dynamic_pointer_cast<Call>(ref_t[i]);
					score[i] = 1 - spc->getEditDistance(ref_spc.get());
				}
				else if (t[i]->isSequence() && ref_t[i]->isSequence()) {
					Sequence::sp_sequence sps = boost::dynamic_pointer_cast<Sequence>(t[i]);
					Sequence::sp_sequence ref_sps_clone = boost::dynamic_pointer_cast<Sequence>(ref_t[i]->clone());
					// we have to clone ref_sps because it is not advised to modify it
					addImplicitSequences(ref_sps_clone,sps);
					if (feedbackSequencesMatch(sps,ref_sps_clone)) {
						std::pair<double,double> res = findBestAlignment(sps->getTraces(),ref_sps_clone->getTraces(),false);
						score[i] = res.first / res.second;
					}
				}
			}
			if (score[i] == 0)
				score[i] = -1;
		}
	}
	if (score[0] == -1 || score[1] == -1)
		return -1;
	return std::max(score[0],score[1]);
}

bool TracesAnalyser::feedbackSequencesMatch(const Sequence::sp_sequence& sps, const Sequence::sp_sequence& ref_sps) const {
	unsigned int sps_size = 0, ref_sps_size = 0;
	for (unsigned int i = 0; i < sps->size(); i++) {
		if (!sps->at(i)->isEvent())
			sps_size++;
	}
	for (unsigned int i = 0; i < ref_sps->size(); i++) {
		if (!ref_sps->at(i)->isEvent())
			ref_sps_size++;
	}
	if (sps_size == ref_sps_size) {
		for (unsigned int i = 0, j = 0; i < sps->size() && j < ref_sps->size();) {
			if (sps->at(i)->isEvent()) {
				i++;
				continue;
			}
			if (ref_sps->at(j)->isEvent()) {
				j++;
				continue;
			}
			if (sps->at(i)->isSequence() && ref_sps->at(j)->isSequence() && !feedbackSequencesMatch(boost::dynamic_pointer_cast<Sequence>(sps->at(i)),boost::dynamic_pointer_cast<Sequence>(ref_sps->at(j))))
				return false;
			else if (sps->at(i)->isCall() && ref_sps->at(j)->isCall()) {
				Call::sp_call spc = boost::dynamic_pointer_cast<Call>(sps->at(i));
				Call::sp_call ref_spc = boost::dynamic_pointer_cast<Call>(ref_sps->at(j));
				if (spc->getKey().compare(ref_spc->getKey()) != 0)
					return false;
			}
			else
				return false;
			i++;
			j++;
		}
		return true;
	}
	return false;
}

void TracesAnalyser::filterFeedbacks() {
	const std::vector<Trace::sp_trace>& l = learner_gi.root_sps->getTraces();
	const std::vector<Trace::sp_trace>& e = expert_gi.root_sps->getTraces();
	for (unsigned int i = 0; i < feedbacks.size(); i++) {
		osAnalyser << "(before filter)[" << std::endl;
		feedbacks.at(i).exportAsString(osAnalyser);
		osAnalyser << "]" << std::endl;
	}
	std::vector<unsigned int> to_erase;
	osAnalyser << "start filter feedbacks [" << feedbacks.size() << " feedbacks left]" << std::endl;

	// ----------------------------------------
	// Filter : remove specific types of feedbacks
	// osAnalyser << "---\nFilter -1" << std::endl;
	// for (unsigned int i = 0; i < feedbacks.size(); i++) {
		// if (feedbackTypeIn(feedbacks.at(i).type, 4, CALL_LACK, CALL_EXTRA, SEQ_LACK, SEQ_EXTRA)) { // change types here to specify which types have to be removed
			// to_erase.push_back(i);
			// feedbacks.at(i).exportAsString(osAnalyser);
		// }
	// }
	// osAnalyser << "---\n" << std::endl;
	// ----------------------------------------

	// ----------------------------------------
	// Filter : find and ask to remove duplicated feedbacks.
	osAnalyser << "---\nFilter 0 (duplicated feedbacks)" << std::endl;
	for (unsigned int i = 0; i < feedbacks.size(); i++) {
		Feedback& gf = feedbacks.at(i);
		// check if the feedback is not already marked to be removed
		if (std::find(to_erase.begin(),to_erase.end(),i) == to_erase.end()) {
			// loop through the feedback a second time
			for (unsigned int j = i+1; j < feedbacks.size(); j++) {
				// check if the feedback is not already marked to be removed
				if (std::find(to_erase.begin(),to_erase.end(),j) == to_erase.end()) {
					Feedback& af = feedbacks.at(j);
					if (gf.type == af.type && gf.info.compare(af.info) == 0){
						to_erase.push_back(j);
						af.exportAsString(osAnalyser);
					}
				}
			}
		}
	}

	// ----------------------------------------
	// Filter : find redundancies between useful_call\useless_call and call_lack\call_extra. Eliminate the feedback which is given less priority.
	osAnalyser << "---\nFilter 1.1 (Eliminate the feedback which is given less priority between useful_call/useless_call and call_lack/call_extra)" << std::endl;
	for (unsigned int i = 0; i < feedbacks.size(); i++) {
		Feedback& gf = feedbacks.at(i);
		// check if the feedback is not already marked to be removed and is kind of the right types (USEFUL_CALL or USELESS_CALL)
		if (std::find(to_erase.begin(),to_erase.end(),i) == to_erase.end() && feedbackTypeIn(gf.type, 2, USEFUL_CALL, USELESS_CALL)) {
			// if feedback is kind of USEFUL_CALL we init spc with call of expert and learner otherwise
			Call::sp_call spc = (gf.type == USEFUL_CALL) ? boost::dynamic_pointer_cast<Call>(gf.expert_spt) : boost::dynamic_pointer_cast<Call>(gf.learner_spt);
			// loop through the feedback a second time
			for (unsigned int j = 0; j < feedbacks.size(); j++) {
				// check if the feedback is not already marked to be removed
				if (std::find(to_erase.begin(),to_erase.end(),j) == to_erase.end()) {
					Feedback& af = feedbacks.at(j);
					Call::sp_call sec_spc;
					if (gf.type == USEFUL_CALL && af.type == CALL_LACK)
						sec_spc = boost::dynamic_pointer_cast<Call>(af.expert_spt);
					else if (gf.type == USELESS_CALL && af.type == CALL_EXTRA)
						sec_spc = boost::dynamic_pointer_cast<Call>(af.learner_spt);
					if (sec_spc && spc->getKey().compare(sec_spc->getKey()) == 0) {
						if (gf.priority <= af.priority) {
							to_erase.push_back(j);
							af.exportAsString(osAnalyser);
						}
						else {
							to_erase.push_back(i);
							gf.exportAsString(osAnalyser);
						}
						break;
					}
				}
			}
			if (std::find(to_erase.begin(),to_erase.end(),i) != to_erase.end())
				break;
		}
	}
	osAnalyser << "---\n" << std::endl;
	// ----------------------------------------

	// ----------------------------------------
	// Filter : remove redundancies between seq_lack\seq_extra and useful_call,call_lack\useless_call,call_extra.
	// osAnalyser << "---\nFilter 1.2 (remove redundancies between seq_lack/seq_extra and useful_call,call_lack/useless_call,call_extra)" << std::endl;
	// for (unsigned int i = 0; i < feedbacks.size(); i++) {
		// Feedback& gf = feedbacks.at(i);
		// if (feedbackTypeIn(gf.type, 2, SEQ_LACK, SEQ_EXTRA)) {
			// Sequence::sp_sequence sps = (gf.type == SEQ_LACK) ? boost::dynamic_pointer_cast<Sequence>(gf.expert_spt) : boost::dynamic_pointer_cast<Sequence>(gf.learner_spt);
			// Call::call_vector calls = sps->getCalls(true);
			// for (unsigned int j = 0; j < calls.size(); j++) {
				// for (unsigned int h = 0; h < feedbacks.size(); h++) {
					// if (std::find(to_erase.begin(),to_erase.end(),h) == to_erase.end()) {
						// Feedback& af = feedbacks.at(h);
						// Call::sp_call spc;
						// if (gf.type == SEQ_EXTRA && (af.type == CALL_EXTRA || af.type == USELESS_CALL))
							// spc = boost::dynamic_pointer_cast<Call>(af.learner_spt);
						// else if (gf.type == SEQ_LACK && (af.type == CALL_LACK || af.type == USEFUL_CALL))
							// spc = boost::dynamic_pointer_cast<Call>(af.expert_spt);
						// if (spc && calls.at(j)->getLabel().compare(spc->getLabel()) == 0) {
							// bool del = true;
							// if (gf.type == SEQ_LACK) {
								// /* Tous les labels des appels de la séquence ne sont pas forcément affichés pour un feedback de type SEQ_LACK.
								 // * On peut donc supprimer l'autre feedback uniquement si on est certain que le label sera affiché avec ce feedback.
								 // */
								// del = false;
								// unsigned int nb = std::max(1, (int)(calls.size() * SEQ_LACK_INFO_RATIO));
								// if (j < nb)
									// del = true;
							// }
							// if (del) {
								// to_erase.push_back(h);
								// af.exportAsString(osAnalyser);
								// break;
							// }
						// }
					// }
				// }
			// }
		// }
	// }
	// osAnalyser << "---\n" << std::endl;
	// ----------------------------------------

	// ----------------------------------------
	// Filter : remove not defined feedbacks with sequence of length 1
	osAnalyser << "---\nFilter 2 (remove not defined feedbacks with sequence of length 1)" << std::endl;
	for (unsigned int i = 0; i < feedbacks.size(); i++) {
		Feedback& f = feedbacks.at(i);
		if (!f.defined && std::find(to_erase.begin(),to_erase.end(),i) == to_erase.end() && feedbackTypeIn(f.type, 2, SEQ_LACK, SEQ_EXTRA)) {
			Sequence::sp_sequence sps = (f.type == SEQ_LACK) ? boost::dynamic_pointer_cast<Sequence>(f.expert_spt) : boost::dynamic_pointer_cast<Sequence>(f.learner_spt);
			if (sps->length() == 1) {
				to_erase.push_back(i);
				f.exportAsString(osAnalyser);
			}
		}
	}
	osAnalyser << "---\n" << std::endl;
	// ----------------------------------------

	// ----------------------------------------
	/*
		Filter : remove CALL_LACK feedbacks when the call could be aligned with an error call from the player if there was no error
		Patterns such as the one given below have to be detected by this filter :

			learner_call_1						expert_call_1
			learner_call_2 (with error)			- (GAP)
			learner_call_3 (with error)			-
			-									expert_call_2
			-									expert_call_3

		(learner_call_2 and expert_call_2 have the same label, learner_call_3 and expert_call_3 have the same label)

		In this example, CALL_LACK feedbacks about expert_call_2 and expert_call_3 should be deleted.
	*/
	osAnalyser << "---\nFilter 3 (remove CALL_LACK feedbacks when the call could be aligned with an error call from the player if there was no error)" << std::endl;
	Sequence::sequence_vector sequences = learner_gi.root_sps->getSequences();
	for (unsigned int i = 0; i < sequences.size(); i++) {
		if (sequences.at(i)->getAligned()) {
			Sequence::sp_sequence aligned = boost::dynamic_pointer_cast<Sequence>(sequences.at(i)->getAligned());
			path p = constructAlignmentPath(sequences.at(i)->getTraces(),aligned->getTraces());
			unsigned int j = 0, incr = 0;
			Call::sp_call learner_spc, expert_spc;
			while(j < p.size()) {
				if (j-incr >= 0 && j+1+incr < p.size()) {
					if (p.at(j-incr).second == -1 && p.at(j+1+incr).first == -1 && l.at(p.at(j-incr).first)->isCall() && e.at(p.at(j+1+incr).second)->isCall()) {
						learner_spc = boost::dynamic_pointer_cast<Call>(l.at(p.at(j-incr).first));
						expert_spc = boost::dynamic_pointer_cast<Call>(e.at(p.at(j+1+incr).second));
					}
					else if (p.at(j-incr).first == -1 && p.at(j+1+incr).second == -1 && l.at(p.at(j-incr).second)->isCall() && e.at(p.at(j+1+incr).first)->isCall()) {
						learner_spc = boost::dynamic_pointer_cast<Call>(l.at(p.at(j-incr).second));
						expert_spc = boost::dynamic_pointer_cast<Call>(e.at(p.at(j+1+incr).first));
					}
				}
				bool stopSearch = false;
				if (learner_spc && expert_spc && learner_spc->getError() != Call::NONE && learner_spc->getKey().compare(expert_spc->getKey()) == 0) {
					int ind = getFeedbackIndex(expert_spc,CALL_LACK);
					if (ind > -1) {
						if (std::find(to_erase.begin(),to_erase.end(),ind) == to_erase.end()) {
							to_erase.push_back(ind);
							feedbacks.at(ind).exportAsString(osAnalyser);
						}
						incr++;
					}
					else
						stopSearch = true;
				}
				else
					stopSearch = true;
				if (stopSearch) {
					incr = 0;
					j++;
				}
				learner_spc.reset();
				expert_spc.reset();
			}
		}
	}
	osAnalyser << "---\n" << std::endl;
	// ----------------------------------------

	// ----------------------------------------
	/*
		Filter : remove expert alignment feedbacks located after a learner endless sequence

		This filter can work only if traces are analysed during player's game because we need the endless loop information.
		Besides, when we have gaps both in expert and learner traces, learner trace have to appear first (which is normally the case), i.e. :

			learner_call 	-(GAP)
			-				expert_call

		And not :

			-				learner_call
			expert_call		-
	*/
	osAnalyser << "---\nFilter 4 (remove expert alignment feedbacks located after a learner endless sequence)" << std::endl;
	if (endless_loop) {
		Sequence::sp_sequence last_sps = learner_gi.root_sps->getSequences().back();
		if (last_sps->getParent()) {
			Sequence::sp_sequence parent = boost::dynamic_pointer_cast<Sequence>(last_sps->getParent());
			while (!parent->getAligned() && parent->getParent()) {
				last_sps = parent;
				parent = boost::dynamic_pointer_cast<Sequence>(parent->getParent());
			}
			if (parent->getAligned()) {
				Sequence::sp_sequence aligned = boost::dynamic_pointer_cast<Sequence>(parent->getAligned());
				// recuperation de l'indice de last_sps dans le vecteur de traces de son parent
				int ind = parent->getIndex(last_sps), ind_aligned = -1;
				// construction du chemin d'alignement afin de pouvoir observer quelles sont les traces alignées avec un trou
				path p = constructAlignmentPath(parent->getTraces(),aligned->getTraces());
				bool found = false;
				for (unsigned int i = 0; ind_aligned == -1 && i < p.size(); i++) {
					if (!found && p.at(i).first == ind)
						found = true;
					if (found && p.at(i).second != -1)
						ind_aligned = p.at(i).second;
				}
				// suppression de tous les feedbacks experts qui se trouvent apres la derniere sequence identifiée chez le joueur
				do {
					for(unsigned int i = ind_aligned; i < aligned->size(); i++) {
						int ind = getFeedbackIndex(aligned->at(i));
						if (ind > -1 && isExpertRelatedFeedback(feedbacks.at(ind).type) && std::find(to_erase.begin(),to_erase.end(),ind) == to_erase.end()) {
							to_erase.push_back(ind);
							feedbacks.at(ind).exportAsString(osAnalyser);
						}
					}
					if (!aligned->isRoot()) {
						parent = boost::dynamic_pointer_cast<Sequence>(aligned->getParent());
						aligned = parent;
					}
				} while(!aligned->isRoot());
			}
		}
	}
	osAnalyser << "---\n" << std::endl;
	// ----------------------------------------

	// Delete index duplications in case there are any
	for (unsigned int i = 0; i < to_erase.size(); i++) {
		for (unsigned int j = i+1; j < to_erase.size(); j++) {
			if (to_erase.at(i) == to_erase.at(j)) {
				osAnalyser << "Warning : duplication detected in to_erase vector. Duplication removed." << std::endl;
				to_erase.erase(to_erase.begin()+j);
			}
		}
	}
	// Sort indexes contained in to_erase vector
	std::sort(to_erase.begin(),to_erase.end());
	// Do the delete
	osAnalyser << to_erase.size() << " feedback(s) deleted with filter operations" << std::endl;
	for (int i = to_erase.size()-1; i >= 0; i--)
		feedbacks.erase(feedbacks.begin() + to_erase.at(i));
	osAnalyser << "end filter feedbacks [" << feedbacks.size() << " feedbacks left]" << std::endl;
	for (unsigned int i = 0; i < feedbacks.size(); i++) {
		osAnalyser << "(after filter)[" << std::endl;
		feedbacks.at(i).exportAsString(osAnalyser);
		osAnalyser << "]" << std::endl;
	}
}

TracesAnalyser::path TracesAnalyser::constructAlignmentPath(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e) const {
	TracesAnalyser::path p;
	unsigned int i = 0, j = 0;
	while (i < l.size() || j < e.size()) {
		if (i < l.size() && j < e.size() && l.at(i)->getAligned() && e.at(j)->getAligned())
			p.push_back(std::make_pair<int,int>(i++,j++));
		else if (i < l.size() && !l.at(i)->getAligned())
			p.push_back(std::make_pair<int,int>(i++,-1));
		else if (j < e.size() && !e.at(j)->getAligned())
			p.push_back(std::make_pair<int,int>(-1,j++));
	}
	return p;
}

int TracesAnalyser::getFeedbackIndex(const Trace::sp_trace& spt, FeedbackType type) const {
	for (unsigned int i = 0; i < feedbacks.size(); i++) {
		if ((type == NONE || feedbacks.at(i).type == type) && (feedbacks.at(i).learner_spt == spt || feedbacks.at(i).expert_spt == spt))
			return i;
	}
	return -1;
}

bool TracesAnalyser::feedbackTypeIn(FeedbackType type, int n, ...) {
	va_list ap;
    va_start(ap,n);
	bool found = false;
    for(int i = 0; !found && i < n; i++) {
		if (static_cast<int>(type) == va_arg(ap,int))
			found = true;
    }
    va_end(ap);
    return found;
}

bool TracesAnalyser::isExpertRelatedFeedback(FeedbackType type) {
	return feedbackTypeIn(type, 6, USEFUL_CALL, SEQ_LACK, DIST_SEQ_NUM, IND_SEQ_NUM, CALL_LACK, CALL_PARAMS);
}

bool TracesAnalyser::isLearnerRelatedFeedback(FeedbackType type) {
	return feedbackTypeIn(type, 6, USELESS_CALL, SEQ_EXTRA, DIST_SEQ_NUM, IND_SEQ_NUM, CALL_EXTRA, CALL_PARAMS);
}

void TracesAnalyser::setFeedbackInfo(Feedback& f, Feedback& ref_f) const {
	// if f is a CALL_EXTRA feedback and the call has an associated error
	if (f.type == CALL_EXTRA) {
		Call::sp_call spc = boost::dynamic_pointer_cast<Call>(f.learner_spt);
		if (spc->getError() != Call::NONE) {
			ref_f.info = messages_map.at(Call::getEnumLabel<Call::ErrorType>(spc->getError(),Call::errorsArr));
			f.priority = 0;
		}
	}
	// we can replace parts of the info (surrounded by * characters) with the appropriate string
	std::vector<std::string> tokens = TracesParser::splitLine(ref_f.info,'*');
	for (unsigned int i = 0; i < tokens.size(); i++) {
		std::string& s = tokens.at(i);
		if (s.compare("learner_call") == 0 && feedbackTypeIn(f.type, 3, CALL_EXTRA, CALL_PARAMS, USELESS_CALL)) {
			Call::sp_call spc = boost::dynamic_pointer_cast<Call>(f.learner_spt);
			s = "\"" + Call::callMaps.getLabel(spc->getKey()) + spc->getReadableParams() + "\"";
		}
		else if (s.compare("expert_call") == 0 && feedbackTypeIn(f.type, 3, CALL_LACK, CALL_PARAMS, USEFUL_CALL)) {
			Call::sp_call spc = boost::dynamic_pointer_cast<Call>(f.expert_spt);
			s = "\"" + Call::callMaps.getLabel(spc->getKey()) + spc->getReadableParams() + "\"";
		}
		else if (s.compare("label") == 0 && feedbackTypeIn(f.type, 5, CALL_EXTRA, CALL_LACK, CALL_PARAMS, USEFUL_CALL, USELESS_CALL)) {
			Call::sp_call spc = f.learner_spt ? boost::dynamic_pointer_cast<Call>(f.learner_spt) : boost::dynamic_pointer_cast<Call>(f.expert_spt);
			s = "\"" + Call::callMaps.getLabel(spc->getKey()) + "\"";
		}
		else if (s.compare("diff_params") == 0 && f.type == CALL_PARAMS) {
			Call::sp_call learner_spc = boost::dynamic_pointer_cast<Call>(f.learner_spt);
			Call::sp_call expert_spc = boost::dynamic_pointer_cast<Call>(f.expert_spt);
			std::vector<std::string> ids = learner_spc->getListIdWrongParams(expert_spc.get());
			s = "";
			for (unsigned int j = 0; j < ids.size(); j++){
				s += messages_map.at(ids.at(j));
				if (j == ids.size()-1)
				 	s += ".";
				else
					s += " ; ";
			}
		}
		else if (s.compare("list_calls") == 0 && feedbackTypeIn(f.type, 3, SEQ_EXTRA, IND_SEQ_NUM, DIST_SEQ_NUM)) {
			Sequence::sp_sequence sps = boost::dynamic_pointer_cast<Sequence>(f.learner_spt);
			Call::call_vector calls = sps->getCalls(false);
			s = "";
			for (Call::call_vector::const_iterator it = calls.begin(); it != calls.end(); it++) {
				s += "\"" + Call::callMaps.getLabel((*it)->getKey()) + (*it)->getReadableParams() + "\"";
				if (it+1 != calls.end())
					s += ", ";
			}
		}
		else if (s.compare("list_calls_labels") == 0 && f.type == SEQ_LACK) {
			Sequence::sp_sequence sps = boost::dynamic_pointer_cast<Sequence>(f.expert_spt);
			Call::call_vector calls = sps->getCalls(true);
			unsigned int nb = std::max(1, (int)(calls.size() * SEQ_LACK_INFO_RATIO));
			s = "";
			for (unsigned int j = 0; j < nb; j++) {
				s += "\"" + Call::callMaps.getLabel(calls.at(j)->getKey()) + "\"";
				if (j < nb - 1)
					s += ", ";
			}
			if (nb < calls.size())
				s += "...";
		}
		else if (s.compare("learner_ind_seq_num") == 0 && f.type == IND_SEQ_NUM) {
			Sequence::sp_sequence learner_sps = boost::dynamic_pointer_cast<Sequence>(f.learner_spt);
			s = boost::lexical_cast<std::string>(learner_sps->getNumMap().begin()->first);
		}
		else if (s.compare("expert_ind_seq_num") == 0 && f.type == IND_SEQ_NUM) {
			Sequence::sp_sequence expert_sps = boost::dynamic_pointer_cast<Sequence>(f.expert_spt);
			s = boost::lexical_cast<std::string>(expert_sps->getNumMap().begin()->first);
		}
		else if (s.compare("info") == 0) {
			s = "";
			// we can use the info added by the expert to the trace
			Trace::sp_trace spt;
			if (f.expert_spt)
				spt = f.expert_spt; // if we have the expert trace, we can use it directly
			else if (f.learner_spt) {
				Trace::sp_trace learner_spt = f.learner_spt; //if we have the learner trace, we climb up the tree to get an aligned sequence
				while (!learner_spt->getAligned() && learner_spt->getParent())
					learner_spt = learner_spt->getParent();
				if (learner_spt->getAligned() && !boost::dynamic_pointer_cast<Sequence>(learner_spt->getAligned())->isRoot())
					spt = learner_spt->getAligned();
			}
			std::string info;
			bool filled = false;
			while (spt) { // if spt is set, it is an expert trace. we can use the info of the parents (if threre is any info).
				info = spt->getInfo();
				if (!info.empty()) {
					if (!filled) {
						s += " (";
						filled = true;
					}
					else
						s += " inclus dans ";
					s += info;
				}
				if (spt->getParent()) {
					Sequence::sp_sequence parent = boost::dynamic_pointer_cast<Sequence>(spt->getParent());
					if (parent->isRoot())
						spt.reset();
					else
						spt = parent;
				}
			}
			if (filled)
				s += ")";
		}
		else if (s.compare("out_of_range_param") == 0 && f.type == CALL_EXTRA) {
			Call::sp_call learner_spc = boost::dynamic_pointer_cast<Call>(f.learner_spt);
			std::vector<std::string> ids = learner_spc->getListIdWrongParams();
			if (!ids.empty())
				s = "\"" + ids.at(0) + "\"";
		}
		else if (s.compare("n") == 0) {
			s = "\n";
		}
		else if (s.compare("t") == 0) {
			s = "\t";
		}
		else{
			if (Call::callMaps.getLabel(tokens.at(i)).compare("Unknown") != 0){
				s = "\""+Call::callMaps.getLabel(tokens.at(i))+"\"";
			}
		}
		f.info += tokens.at(i);
	}
}

void TracesAnalyser::listGlobalFeedbacks() {
	// USEFUL_CALL and USELESS_CALL
	Call::call_vector learner_calls = learner_gi.root_sps->getCalls(true);
	for (unsigned int i = 0; i < learner_calls.size(); i++) {
		if (experts_calls_freq.find(learner_calls.at(i)->getKey()) == experts_calls_freq.end() || experts_calls_freq.at(learner_calls.at(i)->getKey()) <= USELESS_FREQ) {
			Feedback f;
			f.type = USELESS_CALL;
			f.learner_spt = learner_calls.at(i);
			f.priority = -1;
			feedbacks.push_back(f);
		}
	}
	std::map<std::string,double>::const_iterator it = experts_calls_freq.begin();
	while (it != experts_calls_freq.end()) {
		if (it->second >= USEFUL_FREQ) {
			bool found = false;
			for (unsigned int i = 0; !found && i < learner_calls.size(); i++) {
				if (learner_calls.at(i)->getKey().compare(it->first) == 0)
					found = true;
			}
			if (!found) {
				Feedback f;
				f.type = USEFUL_CALL;
				f.expert_spt = TracesParser::parseLine(it->first);
				f.priority = -1;
				feedbacks.push_back(f);
			}
		}
		it++;
	}
}

void TracesAnalyser::listAlignmentFeedbacks(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e) {
	// feedbacks from alignment with the chosen expert
	for (unsigned int i = 0; i < l.size(); i++) {
		if (!l.at(i)->isEvent()) {
			Feedback f;
			f.type = NONE;
			if (!l.at(i)->getAligned())
				f.type = (l.at(i)->isSequence()) ? SEQ_EXTRA : CALL_EXTRA;
			else if (l.at(i)->isSequence()) {
				Sequence::sp_sequence learner_sps = boost::dynamic_pointer_cast<Sequence>(l.at(i));
				Sequence::sp_sequence expert_sps = boost::dynamic_pointer_cast<Sequence>(l.at(i)->getAligned());
				// learner_sps and expert_sps cannot be both implicit at the same time
				if (learner_sps->isImplicit() && !expert_sps->isImplicit())
					f.type = SEQ_LACK;
				else if (expert_sps->isImplicit() && !learner_sps->isImplicit())
					f.type = SEQ_EXTRA;
				else if (!learner_sps->isImplicit() && !expert_sps->isImplicit()) {
					if (expert_sps->hasNumberIterationFixed()) {
						if (learner_sps->getLevel() == 1 && expert_sps->getLevel() == 1) {
							// sequences are at root
							std::map<unsigned int,unsigned int> learner_numMap = learner_sps->getNumMap();
							std::map<unsigned int, unsigned int> expert_numMap = expert_sps->getNumMap();
							if (learner_numMap.size() == 1 && expert_numMap.size() == 1 && expert_numMap.find(learner_numMap.begin()->first) == expert_numMap.end())
								f.type = IND_SEQ_NUM;
						}
						else if (learner_sps->getNumMapMeanDistance(expert_sps) >= DIST_SEQ_NUM_THRES)
							f.type = DIST_SEQ_NUM;
					}
					listAlignmentFeedbacks(learner_sps->getTraces(), expert_sps->getTraces());
				}
			}
			else {
				Call::sp_call learner_call = boost::dynamic_pointer_cast<Call>(l.at(i));
				Call::sp_call expert_call = boost::dynamic_pointer_cast<Call>(l.at(i)->getAligned());
				double match_score = 1 - learner_call->getEditDistance(expert_call.get());
				if (match_score < 1)
					f.type = CALL_PARAMS;
			}
			if (f.type != NONE) {
				f.learner_spt = l.at(i);
				if (l.at(i)->getAligned())
					f.expert_spt = l.at(i)->getAligned();
				f.priority = -1;
				feedbacks.push_back(f);
			}
		}
	}
	for (unsigned int i = 0; i < e.size(); i++) {
		if (!e.at(i)->getAligned() && !e.at(i)->isEvent()) {
			Feedback f;
			f.expert_spt = e.at(i);
			f.type = (e.at(i)->isSequence()) ? SEQ_LACK : CALL_LACK;
			f.priority = -1;
			feedbacks.push_back(f);
		}
	}
}
