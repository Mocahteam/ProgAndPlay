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

class Scenario {

public:
	/**
	  * Définition du type pointeur intelligent vers un objet Scenario.
	  */
	typedef boost::shared_ptr<Scenario> sp_scenario;
  
  /**
   * Seuil utilisé pour déterminer quels patterns concerver. Un Pattern dont le score est inférieur de SCORE_TOLERENCE par rapport au meilleur score calculé sera supprimé
   */
  static float SCORE_TOLERENCE;

  // Constantes utiliser pour le calcul du score d'un pattern
  static float WEIGHT_ALIGN_RATIO;
  static float OPTION_PENALTY;
  static float WEIGHT_MAXIMIZE_ALIGN;
  static float WEIGHT_MINIMIZE_LENGTH;
  static float MIN_WINDOW;

	// pattern stocké sous une forme linéarisée
	std::vector<Trace::sp_trace> pattern;
	// position nous permettant de connaître la position d'avancement dans le pattern
	int position = 0;
	// compteur d'alignement nous permettant de connaître dans le pattern combien de traces ont pu être alignées
	int alignCount = 0;
	// compteur d'option nous permettant de connaître dans le pattern combien de traces optionnelles ont été définies
	int optCount = 0;
	// score du pattern
	float score = 0;
  // nombre de trace dans le root au dessus du modèle qui a permis de lancer l'aggrégation
  int upCount = 0;
	// position dans le root (en version non linéarisée) à laquelle insérer ce pattern (la position du modèle qui a permis l'aggrégation).
	int rootStartingPos = 0;

  /**
   * \brief construction d'un nouveau scénario à partir d'un vecteur de trace
   */
  Scenario(std::vector<Trace::sp_trace> traces, int upCount, int rootStartingPos, int initAlignCount = 0, int initoptCount = 0, int initScore = -1);

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
   * 
   * \return la liste des scénario découlant de l'ajout du Call dans le scénario courant
   */
  std::vector<Scenario::sp_scenario> simulateNewCallIntegration(const Trace::sp_trace & rootCall, float maxRatio, int minLength, int * maxAligned, int nbRec = 0, std::vector<Scenario::sp_scenario> * viewed = NULL);

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

  /**
   * \brief Vérifie si this est inclus dans l'ensemble de scénario passé en paramètre.
   * 
   * \param set ensemble de scénarios
   * \param checkPosition inclus la position dans l'évaluation de l'inclusion
   * 
   * \return true si le scénario est inclus dans l'ensemble et false sinon
   */
  bool existsIn(std::vector<Scenario::sp_scenario> set, bool checkPosition = false);


  /**
   * \brief Vérifie si this est égal au scénario passé en paramètre.
   * 
   * \param scenario Scénario à tester
   * \param checkPosition inclus la position dans l'évaluation de l'égalité
   * 
   * \return true si le scénario est égal au scénario passé en paramètre et false sinon
   */
  bool isEqualWith(Scenario::sp_scenario scenario, bool checkPosition = false);
};

#endif