/**
 * \file Episode.h
 * \brief Déclaration de la classe Episode
 * \author yufei
 * \version 0.1
 */

#ifndef __EPISODE_H__
#define __EPISODE_H__

#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>
#include <memory>

#include "Trace.h"
#include "Call.h"
#include "Sequence.h"


/**
 * \class Epiosde
 * \brief La classe Episode est une classe pour représenter un épisode, des informations pour un ensemble de traces
 */
class Episode
{

public:
	// weight of the support in relation to proximity parameter. Must be included between [0, 1]. 0 means the support is ignored (only proximity). 1 means the proximity is ignored (only support)
	static float WEIGHT_SUPPORT;
	// control balancing between inside and outside proximity of episodes. Must be included between [0, 1]. 0 means take only inside proximity in consideration (no outside proximity). 1 means take only outside proximity in consideration (no inside proximity). If WEIGH_SUPPORT is set to 1, PROXIMITY_BALANCING is useless.
	static float PROXIMITY_BALANCING;

	static int MIN_SUPPORT; // enregistre parmis les top-k le support minimal
	static int MAX_SUPPORT; // enregistre parmis les top-k le support maximal
	static int MAX_LENGTH; // enregistre parmis les top-k la longueur du pattern maximal

	std::vector<Trace::sp_trace> events;
	bool explored; // dans l'exploration des top-k permet de savoir si cet épisode a déjà été exploré
	// Ce vector représente les plages de temps où on peut trouver cette épisode, exemple : <1,4> signifie que cette épisode commence à t1 et fini à t4
	std::vector<std::pair<int, int>> boundlist;



	typedef std::unique_ptr<Episode> up_episode;

	/**
	  * Définition du type pointeur intelligent vers un objet Episode.
	  */
	typedef std::shared_ptr<Episode> sp_episode;
	/**
	  * Définition du type pointeur faible intelligent vers un objet Episode.
	  */
	typedef std::weak_ptr<Episode> wp_episode;

	/**
	  * \brief Constructeur principal de Episode.
	  */
	Episode(std::vector<Trace::sp_trace> traces, std::vector<std::pair<int, int>> input_BoundList);

	Episode();


	/**
	  * \brief Destructeur de Episode.
	  */
	~Episode();


	/**
	  * \brief Augmenter le supprot de l'épisode.
	  *
	  * 
	  */
	void increaseSupport();


	/**
	  * \brief Getter pour la variable \p info.
	  *
	  * \return la chaîne de caractères \p info de l'épisode.
	  */
	std::string toString(); 


	/**
	  * \brief Fonction static pour fixer la valeur de paramètre PoidsSupport(PSP). 
	  *
	  *
	  */
	static void setPoidSupport(float input_ps);


	/**
	 * \brief Affecte un nouvelle fenêtre à cet épisode
	*/
	void setBounds(std::vector<std::pair<int, int>> newBoundList);

	/**
	 * \brief Retourne le support de cet épisode
	 */
	int getSupport();

	/**
	 * \brief Retourne le score de cet épisode
	 */
	float getScore();

	/**
	 * \brief calcul de la proximité externe moyenne de cet épisode : nombre moyen d'évènement entre chaque bounds
	*/
	float getOutsideProximity();

	/**
	 * \brief calcul de la proximité interne moyenne de cet épisode comprise entre [0, 1] : proportion d'évènements intercallés à l'intérieur de chaque bound
	*/
	float getInsideProximity();
};

struct ComparatorEpisode : public std::binary_function<Episode::sp_episode, Episode::sp_episode, bool>
{
    bool operator() (const Episode::sp_episode a,const Episode::sp_episode b)
    {
		std::stringstream aSS;
		Sequence::exportLinearSequenceAsString(a->events, aSS, false);
		std::stringstream bSS;
		Sequence::exportLinearSequenceAsString(b->events, bSS, false);
        /*return a->getSupport() > b->getSupport() || // privilégier le support
				(a->getSupport() == b->getSupport() && (a->events.size() > b->events.size() || // en cas d'égalité privilégier la longueur de l'épisode
					(a->events.size() == b->events.size() && (a->getInsideProximity() > b->getInsideProximity() || // en cas d'égalité, privilégier la proximité interne
						(a->getInsideProximity() == b->getInsideProximity() && (a->getOutsideProximity() > b->getOutsideProximity() || // en cas d'égalité, privilégier la proximité interne
							(a->getOutsideProximity() == b->getOutsideProximity() && aSS.str().compare(bSS.str()) < 0))))))) ; // en cas d'égalité, trier par ordre alphabétique*/
		float aScore = a->getScore();
		float bScore = b->getScore();
		return aScore > bScore || // privilégier le score
				 (aScore == bScore && aSS.str().compare(bSS.str()) < 0) ; // en cas d'égalité, trier par ordre alphabétique
    }
};
#endif
