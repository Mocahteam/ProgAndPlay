#ifndef __TRACES_ANALYSER_H__
#define __TRACES_ANALYSER_H__

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <dirent.h>
#include <vector>
#include <map>
#include <stack>
#include <limits>
#include <algorithm>
#include <cstdarg>
#include <rapidxml-1.13/rapidxml.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include "Trace.h"
#include "Call.h"
#include "Sequence.h"
#include "Event.h"
#include "EventDef.h"

#define USELESS_FREQ 0					// threshold value in [0,1] used to determine if we have to make a USELESS_CALL feedback
#define USEFUL_FREQ 1					// threshold value in [0,1] used to determine if we have to make a USEFUL_CALL feedback
#define DIST_SEQ_NUM_THRES 0.5 			// threshold value in [0,1] used to determine if we have to make a SEQ_NUM feedback
#define NUM_MAX_FEEDBACKS 3				// maximal number of feedbacks return to the player
#define NUM_DOWNGRADS 2		 			// this value is used to determine the number of priority downgrads accepted
#define SEQ_LACK_INFO_RATIO 1			// the proportion in [0,1] of labels given to the player for SEQ_LACK feedbacks
#define NUM_CALL_APPEARS_THRES 2		// if the number of appearances of a call is less than this value in the trace, we can keep the associated CALL_EXTRA/CALL_LACK feedbacks
#define IND_SEQ_NUM_CONST 4				// this value in [1,inf] is used to set the range of the bonus added to the similarity score in the case of the alignment of two sequences

//#define ALIGN_MATCH_SCORE 1
/**
  * Score utilisé pour l'alignement. Correspond au pire score possible pour l'alignement. Les deux traces comparées ne seront jamais alignées.
  */
#define ALIGN_MISMATCH_SCORE -1
/**
  * Score utilisé pour l'alignement. Correpond au score obtenu si on aligne la trace avec rien (introduction d'un trou).
  */
#define ALIGN_GAP_SCORE 0

// macro for change score range from [0,1] to [INF,SUP]
#define INF -1
#define SUP 1
#define TRANSFORM_SCORE(val) ((SUP - INF) * val + INF)

class TracesAnalyser {
	
public:

	typedef std::vector< std::pair<int,int> > path;

	enum FeedbackType {
		NONE = -1,
		USEFUL_CALL, 			// most experts have used this call but the player hasn't
		USELESS_CALL, 			// the player has used this call but only a few expert has used it
		SEQ_EXTRA, 				// the player has a sequence than the chosen expert hasn't
		SEQ_LACK, 				// the chosen expert has a sequence than the player hasn't
		IND_SEQ_NUM,			// the level of the aligned sequences is 0 and their indexes are not the same
		DIST_SEQ_NUM,   		// notable difference between the indexes of the aligned sequences
		CALL_EXTRA,				// the player has a call than the chosen expert hasn't
		CALL_LACK,				// the chosen expert has a call than the player hasn't
		CALL_PARAMS,			// notable difference between the parameters of the aligned calls
		INCLUDE_CALL_IN_SEQ,	// the player should include a call in a sequence
		EXCLUDE_CALL_FROM_SEQ 	// the player should take a call out from a sequence
	};
		
	static const char* feedbackTypesArr[];
	static std::map<std::string,std::string> messages_map;
	
	struct Feedback {
		/**
		 * Définition du type du feedback. Ce champ est obligatoire dans le fichier XML de définition de feedbacks.
		 *
		 * \see FeedbackType
		 */
		FeedbackType type;
		
		/**
		  * Chaîne de caractères contenant le texte associé au feedback. Défini par la balise <info> dans un fichier XML de définition de feedbacks.
		  *	A noter que la balise <infos lang="..."> doit contenir au moins une balise <info> pour la langue choisie. 
		  */
		std::string info;
		
		/**
		  * Trace de l'apprenant pour ce feedback. Défini par la balise <learner> dans un fichier XML de définition de feedbacks.
		  */
		Trace::sp_trace learner_spt;
		
		/**
		  * Trace de l'expert défini pour ce feedback. Défini par la balise <expert> dans un fichier XML de définition de feedbacks.
		  */
		Trace::sp_trace expert_spt;
		/**
		  * Priorité associée au feedback. Ce champ est obligatoire dans le fichier XML de définition de feedbacks.
		  */
		int priority;
		/**
		 * Variable prenant la valeur de l'attribut 'level' associé à la balise <feedback> dans le fichier XML de définition de feedbacks. Cet attribut est optionnel. S'il n'est pas défini,
		 * cette variable prend la valeur -1 et ne sera pas utilisée lors de l'analyse. Dans le cas, où il est défini, sa valeur est utilisée pour associer un niveau aux traces \p learner_spt et \p expert_spt.
		 */
		int level;
		/**
		  * Booléen mis à vrai si un pattern a été défini pour le feedback dans le fichier XML de définition de feedbacks, i.e. si learner_spt OU (non exclusif) expert_spt est défini.
		  */
		bool defined;
		
		bool operator<(const Feedback& f) const {
			return priority < f.priority || (learner_spt && f.learner_spt && learner_spt->getLevel() < f.learner_spt->getLevel()) || (expert_spt && f.expert_spt && expert_spt->getLevel() < f.expert_spt->getLevel()) || (defined && !f.defined);
		}
		
		void display() {
			std::cout << "feedback : " << info << std::endl;
			std::cout << "type : " << std::string(Call::getEnumLabel<FeedbackType>(type,feedbackTypesArr)) << std::endl;
			if (learner_spt) {
				std::cout << "learner : " << std::endl;
				learner_spt->display();
			}
			if (expert_spt) {
				std::cout << "expert : " << std::endl;
				expert_spt->display();
			}
			std::cout << "priority : " << priority << std::endl;
		}

	};

	struct GameInfos {
		StartMissionEvent *sme;
		EndMissionEvent *eme;
		NewExecutionEvent *nee;
		EndExecutionEvent *eee;
		std::vector<Trace::sp_trace> mission_traces;
		/**
		  * The root Sequence :
		  * 	Sequence<1:1>
		  *			...
		  */
		Sequence::sp_sequence root_sps;
		
		GameInfos() : sme(NULL), eme(NULL), nee(NULL), eee(NULL) {}
		
		void clearMission() { 
			sme = NULL;
			eme = NULL;
			mission_traces.clear();
			clearExecution();
		}
		
		void clearExecution() {
			nee = NULL;
			eee = NULL;
			root_sps.reset();
		}
		
		int getResolutionTime() {
			int time = std::numeric_limits<int>::max();
			if (sme != NULL && eme != NULL && eme->getStatus().compare("won") == 0)
				time = eme->getEndTime() - sme->getStartTime();
			return time;
		}
		
		int getExecutionTime() {
			int time = std::numeric_limits<int>::max();
			if (nee != NULL && eee != NULL)
				time = eee->getEndTime() - nee->getStartTime();
			return time;
		}
		
		int getNumExecutions() {
			int num = 0;
			for (unsigned int i = 0; i < mission_traces.size(); i++) {
				if (mission_traces.at(i)->isEvent() && dynamic_cast<Event*>(mission_traces.at(i).get())->getLabel().compare("new_execution") == 0)
					num++;
			}
			return num;
		}
		
		double getAverageWaitTime() {
			EndExecutionEvent *eee = NULL;
			double avg = 0;
			int cpt = 0;
			bool in = false;
			for (unsigned int i = 0; i < mission_traces.size(); i++) {
				if (mission_traces.at(i)->isEvent()) {
					Event *e = dynamic_cast<Event*>(mission_traces.at(i).get());
					if (e->getLabel().compare("new_execution") == 0) {
						if (!in) {
							in = true;
							cpt++;
							if (eee != NULL) {
								avg += dynamic_cast<NewExecutionEvent*>(e)->getStartTime() - eee->getEndTime();
								eee = NULL;
							}
						}
						else
							return -1;
					}
					else if (e->getLabel().compare("end_execution") == 0) {
						in = false;
						eee = dynamic_cast<EndExecutionEvent*>(e);
					}
				}
			}
			if (--cpt == 0)
				return -1;
			else
				return avg / cpt;
		}
		
	};

	TracesAnalyser(std::string lang = "fr");

	std::string constructFeedback(const std::string& learner_xml, const std::vector<std::string>& experts_xml, int ind_mission = -1, int ind_execution = -1, std::ostream &os = std::cout);
	void loadXmlInfos(const std::string& feedbacks_xml, const std::string& mission_feedbacks_xml);
	void setEndlessLoop(bool endless_loop);
	void setLang(std::string lang);
	
	static int getRandomIntInRange(int min, int max);
	static bool feedbackTypeIn(FeedbackType type, int n, ...);
	
private:

	bool endless_loop;
	bool loaded;
	std::string lang;
	GameInfos learner_gi;
	GameInfos expert_gi;
	std::vector<Feedback> feedbacks;
	std::vector<Feedback> ref_feedbacks;
	std::map<std::string,double> experts_calls_freq;
	
	void importFeedbacksFromXml(rapidxml::xml_document<> *doc);
	void importMessagesFromXml(rapidxml::xml_document<> *doc);
	
	bool getInfosOnMission(const std::vector<Trace::sp_trace>& traces, GameInfos& gi, int ind_mission = -1);
	bool getInfosOnExecution(GameInfos& gi, int ind_execution = -1);
	
	bool addImplicitSequences(Sequence::sp_sequence& mod_sps, Sequence::sp_sequence& ref_sps) const;
	std::vector<Call::call_vector> getPatterns(const Sequence::sp_sequence& mod_sps, const Call::call_vector& pattern) const;
	const Sequence::sp_sequence getClosestCommonParent(const Call::call_vector& pattern) const;
	
	std::pair<double,double> findBestAlignment(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e, bool align = true) const;
	void displayAlignment(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e, std::ostream &os = std::cout) const;
	
	void listAlignmentFeedbacks(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e);
	void listGlobalFeedbacks();
	void bindFeedbacks();
	bool feedbackSequencesMatch(const Sequence::sp_sequence& sps, const Sequence::sp_sequence& ref_sps) const;
	double getFeedbackScore(const Feedback& f, int j);
	void filterFeedbacks();
	void setFeedbackInfo(Feedback &f, Feedback &ref_f) const;
	path constructAlignmentPath(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e) const;
	int getFeedbackIndex(const Trace::sp_trace& t, FeedbackType type = NONE) const;
	
};

#endif