#include "VariantTKE.h"
#include "TracesParser.h"
#include "Episode.h"
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
      
// issu de TKE => nombre d'épisodes conservés, c'est le K des top-K épisodes
int VariantTKE::K = 10;
// issu de QCSP => permet d'intégrer des traces intercalées et donc les options => coefficient de fenêtre maximale
int VariantTKE::ALPHA = 2;

VariantTKE::VariantTKE(){}


Episode::sp_episode VariantTKE::runAlgorithm(Sequence::sp_sequence data){
	Episode::MIN_SUPPORT = 1;
	kEpisodes = std::set<Episode::sp_episode, ComparatorEpisode>();
	std::vector<Episode::sp_episode> evtWithSuffisantSupport;
	minScore = 1;
	int halfSize = data->size()/2;
	//scanner la trace pour obtenir les événements plus fréquents
	std::unordered_map<Trace::sp_trace, std::vector<std::pair<int, int>>, HashFunc, Comparator> mapItemLocationList;
	std::vector<Trace::sp_trace> dataBase = data->getTraces();
	// parcourir toutes les traces de la séquence et enregistrer les positions d'apparition de chaque item
	for (int i = 0 ; i < (int)dataBase.size() ; i++)
	{
		// Noter l'indice de son apparition
		auto it_loc = mapItemLocationList.find(dataBase[i]);
		if(it_loc == mapItemLocationList.end()){ // Nouvelle trace détectée => on crée une nouvelle entrée pour enregistrer sa possition
			std::vector<std::pair<int, int>> locationList;
			locationList.push_back({i , i});
			mapItemLocationList.insert(std::make_pair(dataBase[i], locationList));
		}
		else{ // trace déjà connue => ajout de la nouvelle position
			it_loc->second.push_back({i , i});
		}
	}

	// on ne conserve que les items ayant un score supérieur ou égal au score minimal
	for(auto it=mapItemLocationList.begin(); it!=mapItemLocationList.end(); ++it){
		// Pour chaque item on crée un nouvel épisode
		// Création d'un vecteur de trace qui ne contient pour l'instant que la trace courante la plus fréquente
		std::vector<Trace::sp_trace> events;
		events.push_back(it->first);
		// Création d'un épisode contenant ce vecteur de trace ainsi que ses positions d'apparition
		Episode::sp_episode sp_epi(new Episode(events, mapItemLocationList.find(it->first)->second));
		
		/*std::cout << " MINSUP:" << Episode::MIN_SUPPORT << " MAXSUP:" << Episode::MAX_SUPPORT << " MAXLENGTH:" << Episode::MAX_LENGTH << std::endl;
		std::cout << "sc:" << sp_epi->getScore() << " sup:" << sp_epi->getSupport() << " pi:" << sp_epi->getInsideProximity() << " po:" << sp_epi->getOutsideProximity() << " ";
		if (it->first->isCall())
			Sequence::exportLinearSequenceAsString(events);
		else
			Sequence::exportLinearSequenceAsString(dynamic_cast<Sequence*>(events[0].get())->getLinearSequence());*/

		// A cette étape comme on n'a que des épisodes d'un seul évènement, on ne considère pas le score mais simplement le support
		if(sp_epi->getSupport() >= Episode::MIN_SUPPORT){
			// Ajout de ce nouvel episode dans la liste des top-K
			saveEpisode(sp_epi);

			// sauvegarder cet épisode contenant un seul évènement pour le retrouver indépendament de son appartenance aux top-k
			evtWithSuffisantSupport.push_back(sp_epi);
		}
	}

	/*for (auto it = kEpisodes.begin() ; it != kEpisodes.end() ; it++){
		std::cout << "starting top-K : sc:" << (*it)->getScore() << " sup:" << (*it)->getSupport() << " pi:" << (*it)->getInsideProximity() << " po:" << (*it)->getOutsideProximity() << " : ";
		Sequence::exportLinearSequenceAsString((*it)->events);
	}*/
	
	while (needEpisodeExploration()){
		std::vector<Episode::sp_episode> newEpisodes;
		for (auto it_episode = kEpisodes.begin() ; it_episode != kEpisodes.end() ; it_episode++){
			if (!(*it_episode)->explored){
				for(int i = evtWithSuffisantSupport.size()-1 ; i >= 0 ; i--){
					if (evtWithSuffisantSupport[i]->getSupport() >= Episode::MIN_SUPPORT){
						Episode::sp_episode newEpisode = tryToExtendEpisodeWithEvent(*it_episode, evtWithSuffisantSupport[i], halfSize);
						if (newEpisode->getSupport() > 0 && newEpisode->getScore() >= minScore){
							newEpisodes.push_back(newEpisode);
						}
					}
					else{
						evtWithSuffisantSupport.erase(evtWithSuffisantSupport.begin()+i); // le support de cet event n'est plus assez fort, on le supprime
					}
				}
				(*it_episode)->explored = true;
			}
		}
		// ajouter les nouveaux épisodes aux top-k
		for (auto it = newEpisodes.begin() ; it != newEpisodes.end() ; it++)
			saveEpisode(*it);
	}

	/*for (auto it = kEpisodes.begin() ; it != kEpisodes.end() ; it++){
		std::cout << "new top-K : sc:" << (*it)->getScore() << " sup:" << (*it)->getSupport() << " pi:" << (*it)->getInsideProximity() << " po:" << (*it)->getOutsideProximity() << " : ";
		Sequence::exportLinearSequenceAsString((*it)->events);
	}*/

	// Le meilleur pattern est le premier des kEpisodes
	Episode::sp_episode bestPattern = *kEpisodes.begin();
	if(bestPattern!=0 && TracesParser::outputLog)
		std::cout<<"\n\nBestPattern ["<< bestPattern->boundlist[0].first << "," << bestPattern->boundlist.back().second <<"] => \n"<<bestPattern->toString()<<std::endl;
		
	return bestPattern;
}

/**
 * \brief recherche dans le kEpisode s'il en reste à explorer
*/
bool VariantTKE::needEpisodeExploration(){
	for (auto it = kEpisodes.begin() ; it != kEpisodes.end() ; it++)
		if (!(*it)->explored)
			return true;
	return false;
}

void VariantTKE::saveEpisode(Episode::sp_episode pattern) {
	// We add the pattern to the set of top-k patterns
	kEpisodes.insert(pattern);
	// if the size becomes larger than k => reduce it
	if ((int)kEpisodes.size() > K)
		kEpisodes.erase(std::prev(kEpisodes.end()));
	// we update MIN_SUPPORT, MAX_SUPPORT and MAX_LENGTH
	Episode::MIN_SUPPORT = INT_MAX;
	Episode::MAX_SUPPORT = 1;
	Episode::MAX_LENGTH = 1;
	for (auto it = kEpisodes.begin() ; it != kEpisodes.end() ; it++){
		if ((*it)->getSupport() > Episode::MAX_SUPPORT)
			Episode::MAX_SUPPORT = (*it)->getSupport();
		if ((*it)->getSupport() < Episode::MIN_SUPPORT)
			Episode::MIN_SUPPORT = (*it)->getSupport();
		if ((int)(*it)->events.size() > Episode::MAX_LENGTH)
			Episode::MAX_LENGTH = (*it)->events.size();
	}
	// we raise the minSupport to the support of the last frequent pattern
	minScore = (*std::prev(kEpisodes.end()))->getScore();
}


/**
 * \brief Calcule un nouvel episode en étendant un episode donné avec un event
 * 
 * @param episode: episode à étendre
 * @param event: évènement à tenter d'intégrer à l'épisode. En évènement est un épisode contenant une seule trace, la position d'un évènement est sous forme d'un bound avec la même valeur pour le start et le end, un évènement à la position 9 a un bound égale à [9,9]
 * 
 * \return l'episode étendu
 */
Episode::sp_episode VariantTKE::tryToExtendEpisodeWithEvent(Episode::sp_episode episode, Episode::sp_episode event, int maxWindowSize){
	// vérifier que l'évènement est bien composer d'une et une seule trace
	if (event->events.size() != 1){
		std::cout << "Erreur, un évènement devrait être composé d'une et une seule trace";
		return nullptr;
	}

	// Calcul de la fenêtre maximale autorisée => ALPHA * la longueur cumulée du kième épisodes + 1 (le nouvel evènement) => pour accepter d'éventuelles traces intercalées
	int max_window_size = std::min(int((episode->events.size() + 1) * ALPHA), maxWindowSize);

	// Calcul des bounds contenant la fusion des bounds de l'épisode avec les positions d'un évènement
	std::vector<std::pair<int, int>> newBoundlist = temporalJoin(episode->boundlist, event->boundlist, max_window_size);

	// génération du nouvel épisode
	// clonage du contenu de l'épisode
	std::vector<Trace::sp_trace> eventsClone;
	for (auto it = episode->events.begin() ; it != episode->events.end() ; it++)
		eventsClone.push_back((*it)->clone());
	eventsClone.push_back(event->events[0]); // ajout de l'évènement
	Episode::sp_episode newEpisode(new Episode(eventsClone, newBoundlist));
	return newEpisode;
}

/**
 * \brief Calcule les bounds en fusionnant un épisode avec un event en respectant la fenêtre maximale acceptée
 * 
 * @param episodeBoundlist: liste des bounds pour un episode
 * @param eventBoundList: liste des positions de l'évènement (la position d'un évènement est sous forme d'un bound avec la même valeur pour le start et le end, un évènement à la position 9 a un bound égale à <9,9>)
 * 
 * \return l'ensemble de bound possibles en joignant l'event à l'épisode
 */
std::vector<std::pair<int, int>> VariantTKE::temporalJoin(std::vector<std::pair<int, int>> episodeBoundlist, std::vector<std::pair<int, int>> eventBoundlist, int max_window_size)
{
	std::vector<std::pair<int, int>> newBoundlist;
	int i=0, j=0;

	while (i<(int)episodeBoundlist.size() && j<(int)eventBoundlist.size())
	{
		// vérifier que le bound de l'event est bien une position, sinon on passe au suivant
		if (eventBoundlist[j].first != eventBoundlist[j].second){
			std::cout << "Erreur, un event contient une position mal configurée!!!";
			j++;
		}
		// éviter la superposition de bounds => sinon le support augmente mais ne reflète pas bien la future compression possible car un event ne peut être inclus que dans un et un seul bound
		// episode : [... <?,4> ...] => onfait avancer l'épisode pour tenter de trouver un bound après le dernier bound de la nouvelle bound list
		// new     : [... <6,?>]
		else if(newBoundlist.size()>0 && episodeBoundlist[i].first <= newBoundlist.back().second){
			i++;
		}
		// avancer sur le bound suivant de l'épisode tant que son bound actuel commence avant la position courante de l'évènement => ça ne sert à rien d'explorer une position de l'event qui commence avant la fin du dernier bound de l'épisode
		// episode : [... <?,6> ...]
		// event   : [... <3,3> ...] => on fait avancer l'event pour tenter de trouver un bound qui commence après le 6
		else if(episodeBoundlist[i].second >= eventBoundlist[j].first){
			j++;
		}
		// avancer sur le bound suivant de l'épisode tant que la distance entre le début de ce bound et la position de l'event est supérieur ou égal à la taille de la fenêtre maximale autorisée
		// episode : [... <4,?> ...] => on fait avancer l'épisode pour tenter de trouver un bound plus proche de la position de l'évènement
		// event   : [... <9,9> ...]
		// max   : 3
		else if(eventBoundlist[j].first - episodeBoundlist[i].first >= max_window_size){
			i++;
		}
		// Ici les contraintes suivantes sont respectées :
		//  - le bound de l'évènement est bien une position (start du bound égale fin du bound)
		//  - le bound de l'épisode se situe avant la position de l'évènement
		//  - la disance entre le bound de l'épisode et la position de l'évènement entre dans le fenêtre (<max_window_size)
		// on ajoute un boundlist et on avance sur alpha
		// episode : [... <6,7> ...] => on passe au bound suivant
		// event   : [... <9,9> ...]
		// new     : [... <6,9>]     => on ajoute à new un nouveau bound qui commence au début de celui de l'épisode jusqu'à la position de l'évènement
		else{
			newBoundlist.push_back({episodeBoundlist[i].first, eventBoundlist[j].second});
			i++;
		}
	}
	return newBoundlist;
}
