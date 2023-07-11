#include "Episode.h"

// weight of the support in relation to proximity parameter. Must be included between [0, 1]. 0 means the support is ignored (only proximity). 1 means the proximity is ignored (only support)
float Episode::WEIGHT_SUPPORT = 0.5;
// control balancing between inside and outside proximity of episodes. Must be included between [0, 1]. 0 means take only inside proximity in consideration (no outside proximity). 1 means take only outside proximity in consideration (no inside proximity). If WEIGH_SUPPORT is set to 1, PROXIMITY_BALANCING is useless.
float Episode::PROXIMITY_BALANCING = 0.5;

int Episode::MIN_SUPPORT = 1;
int Episode::MAX_SUPPORT = 1;
int Episode::MAX_LENGTH = 1;	

Episode::Episode(){
}

Episode::Episode(std::vector<Trace::sp_trace> traces, std::vector<std::pair<int, int>> input_BoundList) {
	events = traces;
	boundlist = input_BoundList;
	explored = false;
}

Episode::~Episode(){}

std::string Episode::toString()
{
	std::string ret = "";
	int episodeLength = events.size();
	for (int i=0; i<episodeLength; ++i){
		ret += events[i]->getString();
	}
	ret += "#SCORE=" + std::to_string(getScore()) + " SUPPORT=" + std::to_string(getSupport()) + " PROXIMITEOUTSIDE=" + std::to_string(getOutsideProximity()) + " PROXIMITEINSIDE=" + std::to_string(getInsideProximity()) + "\n";
	return ret;
}

void Episode::setBounds(std::vector<std::pair<int, int>> newBoundList){
	this->boundlist = newBoundList;
}

int Episode::getSupport(){
	return this->boundlist.size();
}

float Episode::getScore(){
	/*float part1 = 0;
	if (Episode::WEIGHT_SUPPORT >= 0){
		// la partie 1 du score concerne le support et la longueur de son pattern. Si WEIGHT_SUPPORT == 1 prise en compte uniquement du support, si == 0 prise en compte uniquement de la longueur du pattern du support
		part1 = Episode::WEIGHT_SUPPORT * getSupport()/Episode::MAX_SUPPORT + (1-Episode::WEIGHT_SUPPORT) * events.size()/Episode::MAX_LENGTH;
	}
	float part2 = 0;
	if (Episode::PROXIMITY_BALANCING >= 0){
		// la partie du score concerne la proximité. On cherche à réduire au maximum les proximités interne et externe. Le calcul des proximités internes et externes retourne une valeur comprite dans l'intervalle [0,1] avec 0 très positif, donc on prend l'opposé et on balance les deux proximités en fonction du PROXIMITY_BALANCING
		part2 = (1-Episode::PROXIMITY_BALANCING) * (1-getInsideProximity()) + Episode::PROXIMITY_BALANCING * (1-getOutsideProximity());
	}
	return (part1 + part2) / 2; // on renvoie la moyenne des deux parties*/

	if (getSupport() <= 1)
		return 0; // Indépendamenet de WEIGHT_SUPPORT (même s'il est défini à 0 <=> ignorer le support) on discalifie les épisodes qui n'ont pas un support au moins égal à 2, en effet on cherche les épisodes qui se répètent au moins une fois (support >= 2)
	float part1 = (float)getSupport()/Episode::MAX_SUPPORT;
	// la partie 2 du score concerne la proximité. On cherche à réduire au maximum les proximités interne et externe. Le calcul des proximités internes et externes retourne une valeur comprite dans l'intervalle [0,1] avec 0 très positif, donc on prend l'opposé et on balance les deux proximités en fonction du PROXIMITY_BALANCING
	float part2 = (1-Episode::PROXIMITY_BALANCING) * (1-getInsideProximity()) + Episode::PROXIMITY_BALANCING * (1-getOutsideProximity());
	// Si WEIGHT_SUPPORT == 1 prise en compte uniquement du support, si == 0 prise en compte uniquement de la longueur du pattern du support
	return WEIGHT_SUPPORT * part1 + (1-WEIGHT_SUPPORT)*part2;
}

// calcul de la proximité externe de cet épisode comprise entre [0, 1] : proportion d'évènements intercallés entre chaque bounds
float Episode::getOutsideProximity(){
	if(boundlist.size() > 1){
		int nbEventBetweenBounds=0;
		for(int i=1; i<(int)boundlist.size(); ++i){
			nbEventBetweenBounds += boundlist[i].first - boundlist[i-1].second - 1; // comptabilisé le nombre d'évènement entre deux bounds
		}
		return float(nbEventBetweenBounds)/float(std::prev(boundlist.end())->second - boundlist.begin()->first); // calcul de la proportion de proximité externe
	} 
	else if(boundlist.size() == 1)
		return 0;
	else
		return 1;
}

// calcul de la proximité interne de cet épisode comprise entre [0, 1] : proportion d'évènements intercallés à l'intérieur de chaque bound
float Episode::getInsideProximity(){
	if(events.size() > 0 && boundlist.size() > 0){
		int nbAdditionalEventInsideBounds = boundlist[0].second - boundlist[0].first + 1 - events.size();
		for(int i=1; i<(int)boundlist.size(); ++i){
			nbAdditionalEventInsideBounds += boundlist[i].second - boundlist[i].first + 1 - events.size(); // comptabiliser le nombre d'évènement intercalés à l'intérieur du bound
		}
		return float(nbAdditionalEventInsideBounds)/float(events.size()*boundlist.size()); // calcul de la proportion de proximité interne
	} else
		return 1;
}