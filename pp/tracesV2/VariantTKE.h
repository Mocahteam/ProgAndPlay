/**
 * \file Trace.h
 * \brief Déclaration de la classe Trace
 * \author meresse
 * \version 0.1
 */

#ifndef __VARIANTTKE_H__
#define __VARIANTTKE_H__

#include <stdexcept>
#include <vector>
#include <iostream>
#include <sstream>
#include <string.h>
#include <queue>
#include <unordered_set>
#include <map>
#include <set>
#include <algorithm>
#include "Sequence.h"
#include "Scenario.h"
#include "Episode.h"


struct Comparator : public std::binary_function<Trace::sp_trace, Trace::sp_trace, bool>{
  	bool operator()(Trace::sp_trace const & left, Trace::sp_trace const& right) const{
   		return left->operator==(right.get());
  	}
};

class HashFunc {
public:
    size_t operator() (const Trace::sp_trace &keyInit) const {     // the parameter type should be the same as the type of key of unordered_map
        if(keyInit->isCall()){
			// cast trace into call
			const Call *c = dynamic_cast<const Call *>(keyInit.get());
			return std::hash<std::string>{}(keyInit->getLevel()+" "+c->getKey());
        	//hash += 2;
        }
        else if(keyInit->isSequence()){
			Sequence *s = dynamic_cast<Sequence *>(keyInit.get());
			std::stringstream ss;
			Sequence::exportLinearSequenceAsString(s->getLinearSequence(), ss, false); // on passe part l'export de trace linéarisée pour bénéficier du dernier paramètre
			return std::hash<std::string>{}(keyInit->getLevel()+" "+ss.str());
        	//hash += 14;
        }
        return 0;
    }
};


/**
 * \class Trace
 * \brief La classe Trace est une classe abstraite servant de classe mère aux classes Sequence, Call et Event.
 */
class VariantTKE
{

public:
	float minScore;
	
	// issu de TKE => nombre d'épisodes conservés, c'est le K des top-K épisodes
	static int K;
	// issu de QCSP => permet d'intégrer des traces intercalées et donc les options => coefficient de fenêtre maximale
	static int ALPHA;

	VariantTKE();

	/**
   * \brief Exécuter le processus d'analyser la trace pour obtenir l'épisode plus probable
   * 
   * \param data la trace brute
   * 
   * \return la meilleur épisode, retourne NULL si il n'y a aucune épisode trouvée
   */
	Episode::sp_episode runAlgorithm(Sequence::sp_sequence data);


private:
	std::set<Episode::sp_episode, ComparatorEpisode> kEpisodes;
	
	/**
	 * Enregistre un épisode dans l'ensemble de top-k pattern.
	 * 
	 * @param pattern the pattern to be saved
	 */
	void saveEpisode(Episode::sp_episode pattern);



	/**
	 * \brief Calcule un nouvel episode en étendant un episode donné avec un event
	 * 
	 * @param episode: episode à étendre
	 * @param event: évènement à tenter d'intégrer à l'épisode (la position d'un évènement est sous forme d'un bound avec la même valeur pour le start et le end, un évènement à la position 9 a un bound égale à [9,9])
	 * 
	 * \return l'episode étendu
	 */
	Episode::sp_episode tryToExtendEpisodeWithEvent(Episode::sp_episode episode, Episode::sp_episode event, int maxWindowSize);


	/**
	 * \brief Calcule les bounds en fusionnant un épisode avec un event en respectant la fenêtre maximale acceptée
	 * 
	 * @param episodeBoundlist: liste des bounds pour un episode
	 * @param eventBoundList: liste des positions de l'évènement (la position d'un évènement est sous forme d'un bound avec la même valeur pour le start et le end, un évènement à la position 9 a un bound égale à [9,9])
	 * 
	 * \return l'ensemble de bound possibles en joignant l'event à l'épisode
	 */
	std::vector<std::pair<int, int>> temporalJoin(std::vector<std::pair<int, int>> episodeBoundlist, std::vector<std::pair<int, int>> eventBoundlist, int max_window_size);


	/**
	 * \brief recherche dans le kEpisode s'il en reste à explorer
	*/
	bool needEpisodeExploration();
};

#endif