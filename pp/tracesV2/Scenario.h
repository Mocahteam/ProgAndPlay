/**
 * \file Scenario.h
 * \brief Déclaration de la classe Scenario
 * \author muratet
 * \version 0.1
 */

#include "Trace.h"
#include "Call.h"

#ifndef __SCENARIO_H__
#define __SCENARIO_H__

class Scenario {

public:
	/**
	  * Définition du type pointeur intelligent vers un objet Scenario.
	  */
	typedef std::shared_ptr<Scenario> sp_scenario;
  
  /**
   * Seuil utilisé pour déterminer quels patterns concerver. Un Pattern dont le score est inférieur de SCORE_TOLERENCE par rapport au meilleur score calculé sera supprimé
   */
  static float SCORE_TOLERENCE;

  // Constantes utiliser pour le calcul du score d'un pattern
  static float WEIGHT_ALIGN_RATIO;
  static float WEIGHT_MINIMIZE_LENGTH;

	// pattern stocké sous une forme linéarisée
	std::vector<Trace::sp_trace> pattern;

  // pattern original donnée par TKE et on peut le modifier si on modifie le résultat de TKE
  std::vector<Trace::sp_trace> pattern_tke;
    
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

  int num_descend = 0;

  /**
   * \brief construction d'un nouveau scénario à partir d'un vecteur de trace
   */
  Scenario(std::vector<Trace::sp_trace> traces, int upCount = 0, int rootStartingPos = 0, int initAlignCount = 0, int initoptCount = 0, int initScore = -1);

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
   * \brief Simule l'intégration d'une nouvelle Trace dans le scénario et retourne les différents scénarios résultant de l'intégration.
   * 
   * \param rootTrace Trace à insérer dans chaque patten
   * \param maxRatio Meilleur score obtenu parmis les patterns à traiter
   * \param minLength Taille du plus petit pattern (le plus compressé)
   * 
   * \return la liste des scénario découlant de l'ajout de la Trace dans le scénario courant
   */
  std::vector<Scenario::sp_scenario> simulateNewTraceIntegration(const Trace::sp_trace & rootTrace, float maxRatio, int minLength, int nbRec = 0, std::vector<Scenario::sp_scenario> * viewed = NULL);

  /**
   * \brief Simule l'intégration d'un nouveau Call dans le scénario et retourne les différents scénarios résultant de l'intégration.
   * 
   * \param rootCall Call à insérer dans chaque patten
   * \param maxRatio Meilleur score obtenu parmis les patterns à traiter
   * \param minLength Taille du plus petit pattern (le plus compressé)
   * 
   * \return la liste des scénario découlant de l'ajout du Call dans le scénario courant
   */
  std::vector<Scenario::sp_scenario> simulateNewCallIntegration(const Call::sp_call & rootCall, float maxRatio, int minLength, int nbRec = 0, std::vector<Scenario::sp_scenario> * viewed = NULL);

  /**
   * \brief Simule l'intégration d'une nouvelle Sequence dans le scénario et retourne les différents scénarios résultant de l'intégration.
   * 
   * \param rootSeq Sequence à insérer dans chaque patten
   * \param maxRatio Meilleur score obtenu parmis les patterns à traiter
   * \param minLength Taille du plus petit pattern (le plus compressé)
   * 
   * \return la liste des scénario découlant de l'ajout du Call dans le scénario courant
   */
  //std::vector<Scenario::sp_scenario> simulateNewSeqIntegration(const Sequence::sp_sequence & rootSeq, float maxRatio, int minLength, int nbRec = 0, std::vector<Scenario::sp_scenario> * viewed = NULL);

  /**
   * \brief Recalcule le score du scénario. Ce calcul dépend des autres scénarios à considérer.
   * 
   * \param minLength taille minimale des autres scénario considérés
   */
  void updateScore(int minLength);

  /**
   * \brief simule le score du scénario en considérant une variation sur le nombre de traces alignées et optionnelles. Ce calcul dépend aussi des autres scénarios à considérer.
   * 
   * \param alignIncr incrément à appliquer sur le nombre de traces alignées pour la simulation du score
   * \param optIncr incrément à appliquer sur le nombre de traces optionnelles pour la simulation du score
   * \param minLength taille minimale des autres scénario considérés
   */
  float simulateScore(int alignIncr, int optIncr, int minLength);

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

  /**
   * \brief Vérifie si this est inclus dans un scénario passé en paramètre.
   * 
   * \param scenario Scénario à tester
   * 
   * \return true si this est inclus dans le scénario passé en paramètre et false sinon
   */
  bool isIncludedIn(Scenario::sp_scenario scenario);

  /**
   * \brief Get the nombre d'éléments dans ce scénario.
   * 
   * 
   * \return la length
   */
  int getLength();


  /**
   * \brief Calcul le nombre de "End" de pos à la fin de trace.
   * 
   * \param pos la position départ de calcul
   * 
   * \return le nombre de "End"
   */
  int getNumEnd(int pos);


  /**
   * \brief Inserer les traces dans ce scénario, soit on les insert au début avec prev_post=0, soit à la fin avec prev_post=1.
   * 
   * \param prev_post 0->prev , 1->post
   * 
   * 
   */
  void insertTraces(int prev_post, std::vector<Trace::sp_trace> traces);


  /**
   * \brief Vérifie si le pointeur de ce scénario est à la dernière position
   * 
   * 
   * \return 1 si on est à la dernière position du scénario, sinon retouner 0
   */
  int checkEnd();
};

#endif