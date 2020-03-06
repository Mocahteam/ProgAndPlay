/**
 * \file Scenario.h
 * \brief Déclaration de la classe Scenario
 * \author muratet
 * \version 0.1
 */

#include <boost/shared_ptr.hpp>
#include "Trace.h"

#ifndef __SCENARIO_H__
#define __SCENARIO_H__


/**
 * Seuil utilisé pour déterminer quels patterns concervés. Un Pattern dont le score est inférieur de SCORE_TOLERENCE par rapport au meilleur score calculé sera supprimé
 */
#define SCORE_TOLERENCE 0.1

class Scenario {

public:
	/**
	  * Définition du type pointeur intelligent vers un objet Trace.
	  */
	typedef boost::shared_ptr<Scenario> sp_scenario;

	// pattern stocké sous une forme linéarisée
	std::vector<Trace::sp_trace> pattern;
	// position nous permettant de connaître la position d'avancement dans le pattern
	int position;
	// compteur d'alignement nous permettant de connaître dans le pattern combien de traces ont pu être alignées
	int alignCount;
	// compteur d'option nous permettant de connaître dans le pattern combien de traces optionnelles ont été définies
	int optCount;
	// score du pattern
	float score;
    // nombre de trace dans le root au dessus du modèle qui a permis de lancer l'aggrégation
    int upCount;
	// position dans le root à laquelle insérer ce pattern (la position du modèle qui a permis l'aggrégation)
	int rootStartingPos;

    /**
     * \brief construction d'un nouveau scénario à partir d'un vecteur de trace
     */
    Scenario(std::vector<Trace::sp_trace> traces, int upCount, int rootStartingPos);

    /**
	  * \brief Copie du scénario.
	  *
	  * \return un pointeur intelligent vers le nouveau scénario créée suite au clonage.
	  */
	sp_scenario clone();

    /**
      * \brief Simule l'avancement d'une position dans le scénario et retourne tous les nouveaux scénario créés.
      * 
      * \return Ensemble des scénario créés suite à la simulation de l'avancée dans le scénario courant
      */
    std::vector<Scenario::sp_scenario> simulateMoveToNextCall();

    /**
     * \brief Simule l'intégration d'un nouveau Call dans le scénario et retourne les différents scénarios résultant de l'intégration.
     * 
     * \param rootCall Call à insérer dans chaque patten
     * \param maxRatio Meilleur score obtenu parmis les patterns à traiter
     * \param minLength Taille du plus petit pattern (le plus compressé)
     * \param maxAligned Nombre maximal d'alignement parmis les patterns à traiter
     * \param recIdKill Cette fonction étant récursive il y a un risque d'entrer dans une récursion infinie (notemment si une séquence ne contient que des traces optionnelles). Si la position du scénario retombe donc sur recIdKill, la récursion est stoppée
     * 
     * \return la liste des scénario découlant de l'ajout du Call dans le scénario courant
     */
    std::vector<Scenario::sp_scenario> simulateNewCallIntegration(const Trace::sp_trace & rootCall, float maxRatio, int minLength, int * maxAligned, int recIdKill);

    /**
     * \brief Recalcule le score du scénario. Ce calcul dépend des autres scénarios à considérer.
     * 
     * \param minLength taille minimale des autres scénario considérés
     * \param maxAligned nombre maximal d'alignement dans les autres scénario considérés
     */
    void updateScore(int minLength, int maxAligned);

    /**
     * \brief simule le score du scénario en considérant une variation sur le nombre de traces alignées et optionnelles. Ce calcul dépend aussi des autres scénarios à considérer.
     * 
     * \param alignIncr incrément à appliquer sur le nombre de traces alignées pour la simulation du score
     * \param optIncr incrément à appliquer sur le nombre de traces optionnelles pour la simulation du score
     * \param minLength taille minimale des autres scénario considérés
     * \param maxAligned nombre maximal d'alignement dans les autres scénario considérés
     */
    float simulateScore(int alignIncr, int optIncr, int minLength, int maxAligned);
};

#endif