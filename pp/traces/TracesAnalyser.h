/**
 * \file TracesAnalyser.h
 * \brief Déclaration de la classe TracesAnalyser, de la structure Feedback et de la structure GameInfos.
 * \author meresse
 * \version 0.1
 */

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
#include <rapidxml/rapidxml.hpp>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include "Trace.h"
#include "Call.h"
#include "Sequence.h"
#include "Event.h"
#include "EventDef.h"
#include "TraceConstantList.h"

/**
  * valeur comprise dans l'intervalle [0,1] utilisée pour déterminer si l'on doit créer un feedback de type TracesAnalyser::FeedbackType::USELESS_CALL
  */
#define USELESS_FREQ 0

/**
  * Valeur comprise dans l'intervalle [0,1] utilisée pour déterminer si l'on doit créer un feedback de type TracesAnalyser::FeedbackType::USEFUL_CALL
  */
#define USEFUL_FREQ 1

/**
  * Valeur comprise dans l'intervalle [0,1] utilisée pour déterminer si l'on doit créer un feedback de type TracesAnalyser::FeedbackType::DIST_SEQ_NUM
  */
#define DIST_SEQ_NUM_THRES 0.5

/**
  * Nombre maximal de feedbacks retournés au joueur
  */
#define NUM_MAX_FEEDBACKS 3

/**
  * Valeur utilisée pour calculer le nombre de dégradations de priorité autorisées.
  *
  * Exemple :
  * 	- un feedback avec une priorité à 0
  *	    - un feedback avec une priorité à 1
  *		- le nombre de dégradations autorisé est à 0
  *
  *		-> On ne retourne que le premier feedback.
  */
#define NUM_DOWNGRADS 2

/**
  * Valeur comprise dans l'intervalle [0,1] correspondant au pourcentage de labels à afficher au joueur dans le cas d'un feedback de type TracesAnalyser::FeedbackType::SEQ_LACK.
  *
  * Si la valeur est à 1, on affichera donc l'ensemble des labels des appels utilisés dans la séquence.
  */
#define SEQ_LACK_INFO_RATIO 1

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

/**
  * \class TracesAnalyser
  *
  * \brief La classe TracesAnalyser définit l'ensemble des traitements relatifs à l'analyse des traces.
  */
class TracesAnalyser {

public:

	typedef std::vector< std::pair<int,int> > path;

	/**
	  * Enumération utilisée pour connaître le type de feedback.
	  */
	enum FeedbackType {
		NONE = -1,
		/**
		  * La plupart des experts ont utilisé cet appel mais pas le joueur. Paramétrable avec TracesAnalyser::USEFUL_FREQ.
		  */
		USEFUL_CALL,
		/**
		  * le joueur a utilisé cet appel mais très peu d'experts l'ont utilisé. Paramétrable avec TracesAnalyser::USELESS_FREQ.
		  */
		USELESS_CALL,
		/**
		  * Une séquence non présente dans les traces de l'expert est présente dans les traces du joueur.
		  */
		SEQ_EXTRA,
		/**
		  * Une séquence non présente dans les traces du joueur est présente dans les traces de l'expert.
		  */
		SEQ_LACK,
		/**
		  * L'attribut de la séquence experte Sequence::num_fixed est à true. La séquence experte est alignée avec une séquence présente dans les traces du joueur. Les deux séquences se trouvent sous la racine. Les indices des deux séquences diffèrent.
		  */
		IND_SEQ_NUM,
		/**
		  * L'attribut de la séquence experte Sequence::num_fixed est à true. La séquence experte est alignée avec une séquence présente dans les traces du joueur. La distance entre les deux ensembles d'indices est supérieure au seuil défini par DIST_SEQ_NUM_THRES.
		  */
		DIST_SEQ_NUM,
		/**
		  * Un appel non présent dans les traces de l'expert est présent dans les traces du joueur.
		  */
		CALL_EXTRA,
		/**
		  * Un appel non présent dans les traces du joueur est présent dans les traces de l'expert.
		  */
		CALL_LACK,
		/**
		  * La distance entre deux appels alignés est supérieure à 0 (le score de similarité n'est pas égale à 1)
		  */
		CALL_PARAMS
	};

	/**
	  * Tableau contenant les chaînes de caractères associées aux différents types de feedback.
	  */
	static const char* feedbackTypesArr[];

	/**
	  * Objet permettant d'associer un message (qui pourra être utilisé pour le retour donné au joueur) à un identifiant.
	  */
	static std::map<std::string,std::string> messages_map;

	/**
	  * Structure utilisée pour contenir un ensemble d'informations sur un retour qui pourrait être donné au joueur.
	  */
	struct Feedback {

		/**
		 * Définition du type du feedback. Ce champ est obligatoire dans le fichier XML de définition de feedbacks.
		 *
		 * \see FeedbackType
		 */
		FeedbackType type;

		/**
		  * Chaîne de caractères contenant le texte associé au feedback. Défini par la balise \<info\> dans un fichier XML de définition de feedbacks.
		  *	A noter que la balise \<infos lang="..."\> doit contenir au moins une balise \<info\> pour la langue choisie.
		  */
		std::string info;

		/**
		  * Trace de l'apprenant pour ce feedback. Défini par la balise \<learner\> dans un fichier XML de définition de feedbacks.
		  */
		Trace::sp_trace learner_spt;

		/**
		  * Trace de l'expert défini pour ce feedback. Défini par la balise \<expert\> dans un fichier XML de définition de feedbacks.
		  */
		Trace::sp_trace expert_spt;

		/**
		  * Priorité associée au feedback. Ce champ est obligatoire dans le fichier XML de définition de feedbacks.
		  */
		int priority;

		/**
		 * Variable prenant la valeur de l'attribut 'level' associé à la balise \<feedback\> dans le fichier XML de définition de feedbacks. Cet attribut est optionnel. S'il n'est pas défini, cette variable prend la valeur -1 et ne sera pas utilisée lors de l'analyse. Dans le cas, où il est défini, sa valeur est utilisée pour associer un niveau aux traces \p learner_spt et \p expert_spt.
		 */
		int level;

		/**
		  * Booléen mis à vrai si un pattern a été défini pour le feedback dans le fichier XML de définition de feedbacks, i.e. si learner_spt OU (non exclusif) expert_spt est défini.
		  */
		bool defined;

		/**
		  * \brief Fonction utilisé lors de l'opération de tri des feedbacks listés.
		  *
		  * \param f l'objet Feedback utilisé pour la comparaison.
		  *
		  */
		bool operator<(const Feedback& f) const {
			return priority < f.priority || (learner_spt && f.learner_spt && learner_spt->getLevel() < f.learner_spt->getLevel()) || (expert_spt && f.expert_spt && expert_spt->getLevel() < f.expert_spt->getLevel()) || (defined && !f.defined);
		}

		/**
		  * \brief Affichage du feedback.
	      *
	      * \param os le flux de sortie utilisé pour l'affichage.
	      *
	      */
		void display(std::ostream &os = std::cout) {
			os << "feedback : " << info << std::endl;
			os << "type : " << std::string(Call::getEnumLabel<FeedbackType>(type,feedbackTypesArr)) << std::endl;
			if (learner_spt) {
				os << "learner : " << std::endl;
				learner_spt->display(os);
			}
			if (expert_spt) {
				os << "expert : " << std::endl;
				expert_spt->display(os);
			}
			os << "priority : " << priority << std::endl;
		}

	};

	/**
	  * Structure utilisée pour contenir un ensemble d'informations sur une trace compressée.
	  */
	struct GameInfos {

		/**
		  * Pointeur vers l'objet Event modélisant le lancement de la mission.
		  */
		StartMissionEvent *sme;

		/**
		  * Pointeur vers l'objet Event modélisant la fin de la mission.
		  */
		EndMissionEvent *eme;

		/**
		  * Pointeur vers l'objet Event modélisant le lancement d'une exécution.
		  */
		NewExecutionEvent *nee;

		/**
		  * Pointeur vers l'objet Event modélisant la fin d'une exécution.
		  */
		EndExecutionEvent *eee;

		/**
		  * Vecteur de traces contenant l'ensemble des traces générées durant une mission, i.e. un ensemble d'exécutions.
		  */
		std::vector<Trace::sp_trace> mission_traces;

		/**
		  * Objet Sequence correspondant à une séquence racine (son indice est donc <1:1>) dont le vecteur contient l'ensemble des traces relatives à une exécution. Ce sont ces traces qui seront analysées pour calculer le score du joueur et pour déterminer le retour à lui donner.
		  *
		  */
		Sequence::sp_sequence root_sps;

		/**
		  * \brief Constructeur de GameInfos
		  */
		GameInfos() : sme(NULL), eme(NULL), nee(NULL), eee(NULL) {}

		/**
		  * \brief Réinitialise toutes les informations de l'objet GameInfos.
		  */
		void clearMission() {
			sme = NULL;
			eme = NULL;
			mission_traces.clear();
			clearExecution();
		}

		/**
		  * \brief Réinitialise toutes les informations relatives à une exécution.
		  */
		void clearExecution() {
			nee = NULL;
			eee = NULL;
			root_sps.reset();
		}

		/**
		  * \brief Calcul du temps de résolution de la mission.
		  *
		  * Pour que ce temps puisse être calculé, la mission doit avoir été terminée.
		  *
		  * \return le temps de résolution de la mission (en secondes) si celui-ci peut être calculé, ou -1 sinon.
		  */
		int getResolutionTime() {
			int time = -1;
			if (sme != NULL && eme != NULL)
				time = eme->getEndTime() - sme->getStartTime();
			return time;
		}

		/**
		  * \brief Calcul du temps d'exécution du programme de résolution de la mission.
		  *
		  * Pour que ce temps puisse être calculé, il faut que \p nee et \p eee soit différents de NULL (afin d'avoir accès aux timestamps de début et de fin d'exécution).
		  *
		  * \return le temps d'exécution du programme (en secondes) si celui-ci peut être calculé, ou -1 sinon.
		  */
		int getExecutionTime() {
			int time = -1;
			if (nee != NULL && eee != NULL)
				time = eee->getEndTime() - nee->getStartTime();
			return time;
		}

		/**
		  * \brief Calcul du nombre d'exécutions lancées par le joueur pour résoudre une mission.
		  *
		  * \return le nombre d'exécutions lancées pour résoudre la mission.
		  */
		int getNumExecutions() {
			int num = 0;
			for (unsigned int i = 0; i < mission_traces.size(); i++) {
				if (mission_traces.at(i)->isEvent() && dynamic_cast<Event*>(mission_traces.at(i).get())->getLabel().compare(NEW_EXECUTION) == 0)
					num++;
			}
			return num;
		}

		/**
		  * \brief Calcul du temps moyen d'attente entre deux exécutions.
		  *
		  * Pour que ce temps puisse être calculé, il faut qu'un événement NewExecutionEvent soit toujours précédée par un événement EndExecutionEvent (excepté pour la première exécution).
		  *
		  * \return le temps moyen d'attente (en secondes) entre deux lancements d'exécutions par le joueur si celui-ci peut être calculé, ou -1 sinon.
		  */
		double getAverageWaitTime() {
			EndExecutionEvent *eee = NULL;
			double avg = 0;
			int cpt = 0;
			bool in = false;
			for (unsigned int i = 0; i < mission_traces.size(); i++) {
				if (mission_traces.at(i)->isEvent()) {
					Event *e = dynamic_cast<Event*>(mission_traces.at(i).get());
					if (e->getLabel().compare(NEW_EXECUTION) == 0) {
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
					else if (e->getLabel().compare(END_EXECUTION) == 0) {
						in = false;
						eee = dynamic_cast<EndExecutionEvent*>(e);
					}
				}
			}
			if (cpt <= 1)
				return -1;
			return avg / cpt;
		}

	};

	/**
	  * \brief Constructeur principal de la classe TracesAnalyser.
	  *
	  * \param lang la langue qui sera utilisée pour le retour d'informations (par défaut : le français).
	  */
	TracesAnalyser(std::string lang = "fr");

	/**
	  * \brief Fonction principale d'analyse des traces.
	  *
	  * Cette fonction permet de construire le feedback qui sera donné au joueur. Ce feedback comprend la note du joueur, des statistiques sur sa partie et une liste de conseils pour améliorer son programme.
	  *
	  * \param learner_xml une chaîne de caractères contenant une trace complète et compressée provenant de la partie (ou des parties) du joueur au format XML. Une trace complète peut contenir plusieurs occurrences de missions correspondants à plusieurs lancements de mission. Chaque occurrence de mission peut elle-même contenir plusieurs occurrences d'exécution.
	  * \param experts_xml un vecteur de chaînes de caractères contenant les traces compressées provenant des exécutions des solutions expertes au format XML.
	  * \param ind_mission l'indice de l'occurrence de la mission à considérer parmi toutes celles se trouvant dans la trace compressée du joueur. Par défaut, la dernière est celle choisie.
	  * \param ind_execution l'indice de l'exécution à considérer parmi toutes celles contenues dans l'occurrence de la mission selectionnée. Par défaut, la dernière est celle choisie.
	  *
	  * \return une chaîne de caractères contenant le feedback pour le joueur au format JSON.
	  */
	std::string constructFeedback(const std::string& learner_xml, const std::vector<std::string>& experts_xml, int ind_mission = -1, int ind_execution = -1);

	/**
	  * \brief Chargement des feedbacks et des messages contenus dans les fichiers XML 'feedbacks.xml'.
	  *
	  * \param feedbacks_xml la chaîne de caractères correspondant au contenu du fichier 'feedbacks.xml' de base. Cette chaîne doit permettre de charger les feedbacks par défaut, mais aussi les messages. Elle doit obligatoirement être non vide.
	  * \param mission_feedbacks_xml la chaîne de caractères correspondant au contenu du fichier 'feedbacks.xml' spécifique pour la mission considérée. Cette chaîne doit permettre de charger les feedbacks définis pour la mission. Cette chaîne peut être vide car ces feedbacks ne sont pas obligatoires.
	  */
	void loadXmlInfos(const std::string& feedbacks_xml, const std::string& mission_feedbacks_xml);

	/**
	  * \brief Setter pour la variable \p endless_loop.
	  *
	  * \param endless_loop la nouvelle valeur du champ \p endless_loop.
	  */
	void setEndlessLoop(bool endless_loop);

	/**
	  * \brief Setter pour la variable \p lang.
	  *
	  * \param lang la nouvelle valeur du champ \p lang.
	  */
	void setLang(std::string lang);

	/**
	  * \brief Génération aléatoire d'un entier compris dans l'intervalle [0,max[.
	  */
	static int getRandomIntInRange(int max);

	/**
	  * \brief Test d'appartenance de \p type à un ensemble de valeurs.
	  *
	  * Cette fonction permet de tester si la valeur de \p type appartient à l'ensemble défini en extension par les valeurs donnés en arguments de la fonction.
	  *
	  * \param type la valeur du type de feedback à tester.
	  * \param n la taille de l'ensemble.
	  * \param ... les types de feedback constituants l'ensemble.
	  *
	  * \return vrai si la valeur de \p type appartient à l'ensemble, et faux sinon.
	  */
	static bool feedbackTypeIn(FeedbackType type, int n, ...);

	/**
	  * \brief Test si la valeur de \p type est égale à l'une des valeurs suivantes : USEFUL_CALL, SEQ_LACK, DIST_SEQ_NUM, IND_SEQ_NUM, CALL_LACK, CALL_PARAMS.
	  *
	  * \return vrai si c'est le cas, et faux sinon.
	  */
	static bool isExpertRelatedFeedback(FeedbackType type);

	/**
	  * \brief Test si la valeur de \p type est égale à l'une des valeurs suivantes : USELESS_CALL, SEQ_EXTRA, DIST_SEQ_NUM, IND_SEQ_NUM, CALL_EXTRA, CALL_PARAMS.
	  *
	  * \return vrai si c'est le cas, et faux sinon.
	  */
	static bool isLearnerRelatedFeedback(FeedbackType type);

private:

	/**
	  * Booléen mis à vrai si une boucle infinie est détectée dans le programme du joueur.
	  */
	bool endless_loop;

	/**
	  * Booléen mis à vrai lorsque les feedbacks de référence par défaut (ceux qui sont non spécifiques à une mission) ont bien été chargés.
	  */
	bool loaded;

	/**
	  * La langue qui sera utilisée pour le retour d'informations.
	  */
	std::string lang;

	/**
	  * Objet GameInfos utilisé pour traiter la trace compressée du joueur.
	  */
	GameInfos learner_gi;

	/**
	  * Objet GameInfos utilisé pour traiter la trace compressée de l'expert.
	  */
	GameInfos expert_gi;

	/**
	  * Vecteur d'objets Feedback contenant les feedbacks globaux et les feedbacks issus de l'alignement des traces du joueur avec celle de l'expert sélectionné.
	  *
	  * \see TracesAnalyser::listAlignmentFeedbacks
	  * \see TracesAnalyser::listGlobalFeedbacks
	  */
	std::vector<Feedback> feedbacks;

	/**
	  * Vecteur d'objets Feedback contenant les feedbacks de référence par défaut (ceux qui sont non spécifiques à une mission) et les feedbacks de référence définis pour une mission particulière.
	  */
	std::vector<Feedback> ref_feedbacks;

	/**
	  * Objet liant le label de chaque appel utilisé par au moins un expert avec la proportion d'experts ayant utilisé cet appel.
	  *
	  * \see TracesAnalyser::listGlobalFeedbacks
	  */
	std::map<std::string,double> experts_calls_freq;

	/**
	  * \brief Importation des feedbacks de référence.
	  *
	  * Cette fonction permet d'importer les feedbacks de référence définis dans un fichier 'feedbacks.xml'.
	  *
	  * \param doc objet modélisant le document XML contenant les feedbacks de référence.
	  */
	void importFeedbacksFromXml(rapidxml::xml_document<> *doc);

	/**
	  * \brief Importation des messages.
	  *
	  * Cette fonction permet d'importer les messages définis dans le fichier 'feedbacks.xml' de base.
	  *
	  * \param doc objet modélisant le document XML contenant les messages à importer.
	  */
	void importMessagesFromXml(rapidxml::xml_document<> *doc);

	/**
	  * \brief Récupération des traces spécifiques à une occurrence de mission à partir d'une trace complète.
	  *
	  * Cette fonction permet de modifier \p gi pour y conserver les informations sur l'occurrence de la mission indiquée, i.e. l'événement de lancement de la mission (StartMissionEvent), les traces provenant de l'ensemble des exécutions, et éventuellement l'événement de fin de mission (EndMissionEvent) si la mission s'est terminée.
	  *
	  * \param traces le vecteur contenant la trace complète.
	  * \param gi l'objet GameInfos à modifier.
	  * \param ind_mission l'indice de l'occurrence de la mission à considérer parmi toutes celles se trouvant dans la trace compressée. Sa valeur doit être dans l'intervalle [0,num_missions-1] où num_missions est le nombre d'occurrences de missions présentes dans la trace complète. Par défaut, la dernière disponible est celle choisie.
	  *
	  * \return vrai si des informations sur l'occurrence de la mission indiquée ont été passées à \p gi, et faux sinon.
	  */
	bool getInfosOnMission(const std::vector<Trace::sp_trace>& traces, GameInfos& gi, int ind_mission = -1);

	/**
	  * \brief Récupération des traces spécifiques à une occurrence d'exécution à partir des traces correspondantes à un ensemble d'exécutions.
	  *
	  * Cette fonction permet de modifier \p gi pour y conserver les informations sur l'occurrence d'exécution choisie, i.e. l'événement de lancement d'une nouvelle exécution (NewExecutionEvent), les traces spécifiques à cette exécution (que l'on place dans le vecteur d'une séquence racine), et éventuellement l'événement de fin d'exécution (EndExecutionEvent) si celui-ci est présent.
	  *
	  * \param gi l'objet GameInfos à modifier et à partir duquel sont récupérées les traces spécifiques à la mission courante.
	  * \param ind_execution l'indice de l'exécution à considérer parmi toutes celles contenues dans l'occurrence de la mission selectionnée. Par défaut, la dernière est celle choisie.
	  *
	  * \return vrai si des informations sur l'occurrence de l'exécution indiquée ont été passées à \p gi, et faux sinon.
	  */
	bool getInfosOnExecution(GameInfos& gi, int ind_execution = -1);

	/**
	  * \brief Insertion de séquences implicites dans une séquence.
	  *
	  * Cette fonction permet d'ajouter (si besoin est) des séquences implicites dans la séquence \p mod_sps en se basant sur la séquence \p ref_sps. Ces séquences sont nécessaires lors de la phase d'alignement des traces si l'on souhaite pouvoir aligner toutes les traces qui doivent l'être.
	  *
	  * Exemple :
	  *
	  * mod_sps :
	  * 	root
	  *			A
	  *			B
	  *         Sequence
	  *  			C
	  *
	  * ref_sps :
	  * 	root
	  *			Sequence
	  *				A
	  *				B
	  *			C
	  *
	  * Une séquence implicite est ajoutée dans mod_sps :
	  *
	  *  mod_sps :
	  * 	root
	  *			Sequence<1:1>
	  *				A
	  *				B
	  *         Sequence
	  *  			C
	  *
	  * Attention : \p mod_sps et \p ref_sps doivent être des séquences racine.
	  *
	  * \param mod_sps la séquence dans laquelle on ajoute les séquences implicites.
	  * \param ref_sps la séquence sur laquelle on se base pour ajouter les séquences implicites dans \p mod_sps.
	  */
	bool addImplicitSequences(Sequence::sp_sequence& mod_sps, Sequence::sp_sequence& ref_sps) const;

	/**
	  * \brief Récupération d'un ensemble de patterns d'appels dans une séquence.
	  *
	  * Cette fonction permet de construire un vecteur de patterns d'appels provenants de \p mod_sps similaires au pattern \p pattern, un pattern étant un suite d'appels qui se suivent.
	  *
	  * Attention : \p mod_sps doit être une séquence racine.
	  *
	  * \param mod_sps la séquence dans laquelle est recherché le pattern \p pattern.
	  * \param pattern le pattern d'appels recherché dans \p mod_sps.
	  *
	  * \return le vecteur contenant les patterns d'appels trouvés dans \p mod_sps.
	  */
	std::vector<Call::call_vector> getPatterns(const Sequence::sp_sequence& mod_sps, const Call::call_vector& pattern) const;

	/**
	  * \brief Récupération du premier parent commun.
	  *
	  * Cette fonction permet de récupérer le premier parent de la hiérarchie commun à l'ensemble des objets Call contenus dans le vecteur \p pattern.
	  *
	  * \param pattern un vecteur d'appels.
	  *
	  * \return la séquence contenant l'ensemble des appels contenus dans \p pattern et de plus haut niveau dans la hiérarchie de traces.
	  */
	const Sequence::sp_sequence getClosestCommonParent(const Call::call_vector& pattern) const;

	/**
	  * \brief Calcul du meilleur alignement possible entre deux vecteurs de traces.
	  *
	  * Cette fonction est la fonction permettant de déterminer le meilleur alignement possible entre les traces de \p l et les traces de \p e. Le traitement est effectué de façon récursive, i.e. que le meilleur alignement possible pour le contenu des séquences et de leurs sous-séquences est également cherché. Enfin, cette fonction permet également de calculer le score du joueur.
	  *
	  * \param l un vecteur de traces correspondant à l'apprenant
	  * \param e un vecteur de traces correspondant à l'expert
	  * \param align booléen permettant d'indiquer si le meilleur alignement trouvé doit être conservé, i.e. si les champs Trace::aligned doivent être modifiés. Sa valeur par défaut est vrai.
	  *
	  * \return un couple (gs,nv) où gs est le score de similarité brut (non normalisé) obtenu à partir du meilleur alignement trouvé et nv est la valeur de normalisation qui doit être utilisée pour ramener ce score dans l'intervalle [0,1].
	  */
	std::pair<double,double> findBestAlignment(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e, bool align = true) const;

	/**
	  * \brief Affichage de l'alignement trouvé entre deux vecteurs de traces.
	  *
	  * \param l un vecteur de traces correspondant à l'apprenant.
	  * \param e un vecteur de traces correspondant à l'expert.
	  */
	void displayAlignment(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e) const;

	/**
	  * \brief Détermination de l'ensemble des feedbacks d'alignement.
	  *
	  * Cette fonction permet d'ajouter au vecteur \p feedbacks un ensemble de feedbacks determinés à partir de l'alignement effectué entre les deux vecteurs de traces \p l et \p e.
	  *
	  * \param l le vecteur de traces correspondant à l'apprenant.
	  * \param e le vecteur de traces correspondant à l'expert.
	  */
	void listAlignmentFeedbacks(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e);

	/**
	  * \brief Détermination de l'ensemble des feedbacks globaux.
	  *
	  * Les seuls feedbacks globaux ajoutés actuellement sont les feedbacks de type FeedbackType::USEFUL_CALL et FeedbackType::USELESS_CALL.
	  */
	void listGlobalFeedbacks();

	/**
	  * \brief Association des feedbacks trouvés (ceux contenus dans \p feedbacks) avec les feedbacks de référence (ceux contenus dans \p ref_feedbacks).
	  *
	  * Cette fonction sert à donner une valeur aux champs Feedback::priority, Feedback::info et Feedback::defined des objets contenus dans \p feedbacks.
	  */
	void bindFeedbacks();

	/**
	  * \brief Test de correspondance entre deux séquences.
	  *
	  * Cette fonction est utilisée pour tester la correspondance entre deux séquences.
	  * Les deux séquences correspondent si elles possèdent la même structure : une séquence à une position dans l'une des séquences entraîne une séquence à la même position dans l'autre, un appel à une position dans l'une des séquences entraîne un appel dans l'autre avec la même valeur pour le champ Call::label. Le test est effectué de façon récursive.
	  *
	  * \param sps la première séquence utilisée pour le test de correspondance.
      * \param ref_sps la deuxième séquence du test de correspondance.
	  */
	bool feedbackSequencesMatch(const Sequence::sp_sequence& sps, const Sequence::sp_sequence& ref_sps) const;

	/**
	  * \brief Calcul d'un score de similarité entre les patterns d'un feedback listé et d'un feedback de référence.
	  *
	  * Cette fonction est appelé par la fonction TracesAnalyser::bindFeedbacks pour calculer un score de similarité entre un feedback listé (cf. TracesAnalyser::listAlignmentFeedbacks et TracesAnalyser::listGlobalFeedbacks) et un feedback de référence en se basant sur les patterns définis par les champs Feedback::learner_spt et Feedback::expert_spt. Cette fonction est donc utilisée pour associer un feedback listé avec un feedback de référence.
	  *
	  * \param f le feedback listé (contenu dans TracesAnalyser::feedbacks).
	  * \param j l'indice du feedback de référence dans le vecteur TracesAnalyser::ref_feedbacks.
	  *
	  * \return le score calculé.
	  */
	double getFeedbackScore(const Feedback& f, int j);

	/**
	  * \brief Filtrage des feedbacks listés
	  *
	  * Cette fonction contient plusieurs filtres permettant d'éliminer les feedbacks redondants ou non pertinents pour le retour qui sera donné au joueur.
	  */
	void filterFeedbacks();

	/**
	  * \brief Construction de la chaîne de caractères qui servira pour le retour sur la solution.
	  *
	  * Cette fonction effectue les traitement nécessaires sur la chaîne de caractères Feedback::info de \p ref_f afin d'obtenir un message qui pourra être affiché pour le joueur. Ces traitements consistent principalement à substituer des parties de chaîne en fonction du contexte et du type de feedback. Le message résultant des traitements effectués est stocké dans le champ Feedback::info de \p f.
	  *
	  * \param f le feedback listé
	  * \param ref_f le feedback de référence
	  */
	void setFeedbackInfo(Feedback &f, Feedback &ref_f) const;

	/**
	  * \brief Construction d'un vecteur permettant de faciliter le parcours de l'alignement courant entre deux vecteurs de traces.
	  *
	  * Cette fonction se base sur l'alignement effectué entre \p l et \e pour construire un vecteur de couples d'indices reflétant cet alignement. On donne un exemple ci-dessous :
	  *
	  * Prenons les deux vecteurs de traces suivants :
	  *
	  *		root					root
	  *			Sequence				C
	  *				A
	  *				B
	  *			C
	  *
	  * Après alignement, on obtient le résultat suivant :
	  *
	  *    root
	  *    		Sequence	-		GAP
	  *				A
	  *				B
	  *			C			-		C
	  *
	  * L'appel à cette fonction va alors permettre de construire le chemin suivant :
	  *
	  *		(0,	-1) : la trace d'indice 0 du premier vecteur est aligné avec un GAP
	  * 	(1,	0) : la trace d'indice 1 du premier vecteur est aligné avec la trace d'indice 0 du second vecteur
	  *
	  * Remarque : Ce qui est appelé "chemin" ici n'est pas de la même nature que le "chemin" utilisé par la fonction TracesAnalyser::findBestAlignment pour trouver le meilleur alignement possible entre deux vecteurs de traces.
	  */
	path constructAlignmentPath(const std::vector<Trace::sp_trace>& l, const std::vector<Trace::sp_trace>& e) const;

	/**
	  * \brief Récupération de l'indice d'un feedback listé à partir d'une trace et d'un type de feedback.
	  *
	  * \param t la trace utilisée pour la recherche du feedback. Le champ Feedback::learner_spt ou le champ Feedback::expert_spt du feedback recherché est égal à \p t.
	  * \param type le type du feedback recherché
	  *
	  * \return l'indice du feedback trouvé dans le vecteur TracesAnalyser::feedbacks, ou -1 si aucun feedback n'est trouvé.
	  */
	int getFeedbackIndex(const Trace::sp_trace& t, FeedbackType type = NONE) const;

};

#endif
