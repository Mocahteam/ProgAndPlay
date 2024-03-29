/**
 * \file Sequence.h
 * \brief Déclaration de la classe Sequence
 * \author meresse
 * \version 0.1
 */

#ifndef __SEQUENCE_H__
#define __SEQUENCE_H__

#include <iostream>
#include <sstream>
#include <stack>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "Trace.h"
#include "Call.h"

/**
 * \class Sequence
 * \brief La classe Sequence hérite de la classe @see Trace. Une séquence possède un vecteur d'objets Trace.
 */
class Sequence : public Trace, public boost::enable_shared_from_this<Sequence>
{

public:
	/**
	  * Définition du type pointeur intelligent vers un objet Sequence.
	  */
	typedef boost::shared_ptr<Sequence> sp_sequence;
	typedef boost::shared_ptr<const Sequence> const_sp_sequence;
	typedef std::vector<sp_sequence> sequence_vector;

	
	/**
	 * Utilisé dans les séquences linéarisées ("Begin") pour stocker le contecte de l'analyse dans le cas où le premier call d'une séquence n'est pas alignable avec le call du root traité : true => arrivé à la fin de séquence et reprise au début donc ajout du call du root en option au début de la séquence et en fin de séquence ; false => avancé dans le pattern et arrivé au début d'une séquence donc ajout du call du root en option au début de la séquence et avant la séquence.
	 */
	bool newIter;

	/**
	  * \brief Constructeur utilisé dans la fonction TracesParser::importTraceFromNode.
	  *
	  * \param info label de la séquence ajouté par l'expert.
	  * \param num_fixed valeur de l'attribut num_fixed indiquée par l'expert.
	  *
	  * \see TracesParser::importTraceFromNode
	  */
	Sequence(std::string info, bool num_fixed);

	/**
	  * \brief Constructeur principal de la classe Sequence.
	  *
	  * \param num le nombre de répétitions contiguës de la séquence.
	  * \param root booléen mis à vrai si la séquence créée est une racine.
	  * \param opt booléen mis à vrai si la séquence créée est optionnelle.
	  */
	Sequence(unsigned int num, bool root = false, bool opt = false);

	/**
	  * \brief Constructeur se basant sur une séquence.
	  *
	  * \param sps la séquence utilisée pour la construction de la nouvelle séquence.
	  */
	Sequence(const_sp_sequence sps);

	/**
	  * \brief Constructeur se basant sur deux séquences.
	  *
	  * \param sps_up la première séquence.
	  * \param sps_down la deuxième séquence.
	  *
	  * \see TracesParser::mergeEquivalentSequences
	  */
	Sequence(const_sp_sequence sps_up, const_sp_sequence sps_down);

	/**
	  * \brief Fonction principale de l'algorithme de compression hors-ligne.
	  *
	  * Cette fonction est la fonction qui va compresser les traces en effectuant des parcours successifs et en prenant comme point de départ "startingPoint" dans la sequence.
	  *
	  * Exemple :
	  *  - Transformer : ABCBCABCBCD en [A[BC]]D 
	  */
	void findAndAggregateSuccessiveSequences(int startingPoint);

	/**
	  * \brief Fonction d'amélioration de l'algorithme de compression hors-ligne.
	  *
	  * Cette fonction tente d'améliorer la compression en identifiant et intégrant des séquences optionnelles à pratir de "startingPoint".
	  *
	  * Exemple :
	  *  - Transformer : [AB]C[AB][CAB]C en [AB(C)] 
	  */
	bool findAndProcessOptionalTokens(int startingPoint);

	/**
	 * \brief Réduit les sous-séquence et exploite les options pour obtenir un résultat toujours englobant mais sans sous-séquences. Ce traitement est possible s'il n'y a qu'une seule séquence non optionnelle dans this->traces.
	 * 
	 * Exemple :
	 *  - Transformer : [[AB]C] en [AB(C)] => dans les deux cas on peut bien construire la séquence ABABCABC par exemple
	 */
	void reduceSubsequences();

	/**
	  * \brief Comparaison de la séquence avec une trace.
	  *
	  * Cette fonction permet de tester l'égalité entre l'objet Sequence et une trace \p t. On considère dans ce cas qu'il y a égalité si \p t est une séquence, si les tailles des vecteurs de traces des séquences sont égales à une certaine valeur 'size' et si pour tout i dans [0,size[, on a égalité entre la trace qui se trouve à la position i dans le vecteur de traces de la séquence et la trace qui se trouve à la position i dans le vecteur de traces de la séquence \p t (égalité testée en utilisant la fonction Trace::operator==).
	  *
	  * Par exemple, les deux séquences ci-dessous ne sont pas égales :
	  *
	  * 	Sequence
	  *			A
	  *			B
	  * 		Sequence
	  *  			C
	  *
	  * 	Sequence
	  *			Sequence
	  *				A
	  *				B
	  *			C
	  *
	  * \return vrai si la séquence est considérée comme étant égale à \p t, et faux sinon.
	  */
	virtual bool operator==(Trace *t) const;

	/**
	  * \brief Comparaison de la séquence avec une trace.
	  *
	  * Cette fonction permet de tester l'égalité entre l'objet Sequence et une trace \p t. On considère dans ce cas qu'il y a égalité si \p t est une séquence et si la liste 'fl' des appels de l'objet Sequence est égale à la liste d'appels 'sl' de la séquence \p t (même taille et pour tout i, fl[i] est considéré égal à sl[i]).
	  *
	  * On peut noter que ce test d'égalité entre deux séquences est bien moins exigeant que celui de la fonction Sequence::operator==. Si l'on reprend l'exemple donné pour Sequence::operator==, les deux séquences sont cette fois-ci égales :
	  *
	  * 	Sequence
	  *			A
	  *			B
	  *     Sequence
	  *  			C
	  *
	  * 	Sequence
	  *			Sequence
	  *				A
	  *				B
	  *			C
	  *
	  * \return vrai si la séquence est considérée comme étant égale à \p t, et faux sinon.
	  *
	  * \see Sequence::getCalls
	  */
	bool compare(Trace *t);

	/**
	  * \brief Clonage d'une séquence.
	  *
	  * \return une copie de l'objet Sequence.
	  */
	virtual Trace::sp_trace clone() const;

	/**
	  * \brief Export des informations de l'objet Sequence sous la forme d'une chaine de caractères.
	  *
	  * \param os le flux de sortie utilisé pour l'export.
	  */
	virtual void exportAsString(std::ostream &os = std::cout) const;

	/**
	  * \brief Export de la trace sous la forme d'une chaine de caractère compressée (sur une seule ligne).
	  *
	  * \param os : le flux de sortie utilisé pour l'export.
	  */
	virtual void exportAsCompressedString(std::ostream &os = std::cout) const;

	/**
	 * \brief Retourne le premier call de cette séquence, si la première trace de la séquence est une séquence, appelle récursivement getFirstCall sur cette séquence
	 */
	Call::sp_call getFirstCall();

	/**
	  * \brief Réinitialisation récursive de l'alignement de la séquence.
	  *
	  * Cette fonction permet de supprimer le lien vers la trace avec laquelle la séquence était éventuellement alignée, i.e. de réinitialiser le pointeur intelligent Trace::aligned de la séquence. Ce traitement est effectué de façon récursive pour toutes les traces inclus dans la séquence.
	  */
	virtual void resetAligned();

	/**
	  * \brief Construction d'une chaîne de caractères contenant les entrées d'un objet Sequence::iterDesc.
	  *
	  * \param iterDesc l'objet iterDesc dont les informations doivent être récupérées sous forme de chaîne de caractères.
	  *
	  * \return la chaîne de caractères construite.
	  */
	template <typename T>
	static std::string getIterartionDescriptionString(const std::map<unsigned int, T> &iterDesc)
	{
		std::stringstream ss;
		typename std::map<unsigned int, T>::const_iterator it = iterDesc.begin();
		while (it != iterDesc.end())
		{
			if (it != iterDesc.begin())
				ss << " ";
			ss << it->first << ":" << it->second;
			it++;
		}
		return ss.str();
	}

	/**
	  * \brief Getter pour le vecteur de traces de la séquence Sequence::traces.
	  *
	  * \return le vecteur de traces de la séquence.
	  */
	std::vector<Trace::sp_trace> &getTraces();

	/**
	  * \brief Getter pour récupérer une sous partie du vecteur de traces de la séquence Sequence::traces à partir de l'indice "start".
	  *
	  * \return la sous partie du vecteur de traces de la séquence.
	  */
	std::vector<Trace::sp_trace> getSubTraces(int start);

	/**
	  * \brief Getter pour récupérer une sous partie de la séquence comprise entre l'indice "start" (inclus) et l'indice "end" (exclus).
	  *
	  * \return la sous partie clonée de la séquence.
	  */
	Sequence::sp_sequence getSubSequence(int start, int end = -1);

	/**
	  * \brief Getter pour la variable Sequence::pt.
	  *
	  * \see Sequence::pt
	  */
	unsigned int getPt() const;

	/**
	  * \brief Getter pour la variable Sequence::num.
	  *
	  * \see Sequence::num
	  */
	unsigned int getNum() const;

	/**
	  * \brief Getter pour la variable Sequence::iterDesc.
	  *
	  * \see Sequence::iterDesc
	  */
	const std::map<unsigned int, unsigned int> &getIterationDescription() const;

	/**
	  * \brief Getter pour la variable Sequence::num_fixed.
	  *
	  * \see Sequence::num_fixed
	  */
	bool hasNumberIterationFixed() const;

	/**
	  * \brief Getter pour la variable Sequence::endReached.
	  *
	  * \see Sequence::endReached
	  */
	bool isEndReached() const;

	/**
	  * \brief Getter pour la variable Sequence::shared.
	  *
	  * \see Sequence::shared
	  */
	bool isShared() const;

	/**
	  * \brief Getter pour la variable Sequence::root.
	  *
	  * \see Sequence::root
	  */
	bool isRoot() const;

	/**
	  * \brief Test si la séquence est une séquence implicite.
	  *
	  * Une séquence est implicite si la seule entrée contenue dans Sequence::iterDesc est <1:1>.
	  *
	  * \return vrai si la séquence est implicite, et faux sinon.
	  *
	  * \see TracesAnalyser::addImplicitSequences
	  */
	bool isImplicit() const;

	/**
	  * \brief Taille du vecteur de traces de la séquence.
	  *
	  * \return la taille du vecteur de traces de la séquence.
	  */
	unsigned int size() const;

	/**
	  * \brief Calcul partiel de la longueur (l'espace occupé dans un vecteur de traces) d'une séquence.
	  *
	  * La longueur d'une séquence est calculée de façon récursive en parcourant ses séquences et sous-séquences et en appelant Trace::length sur les traces parcourues.
	  *
	  * \param start position dans la sequence à partir de laquelle le calcul de la longueur est calculé
	  * \param processOptions si false la longueur des séquences optionnelles n'est pas pris en compte (true par défaut)
	  *
	  * \return la longueur de la séquence
	  */
	unsigned int length(int start = 0, bool processOptions = true) const;

	/**
	  * \brief Accès à une trace du vecteur de traces de la séquence.
	  *
	  * \param i l'indice de la trace à récupérer dans le vecteur.
	  *
	  * \return Une référence constante vers la trace qui se trouve à l'indice \p i dans le vecteur de traces de la séquence.
	  */
	const Trace::sp_trace &at(unsigned int i) const;

	/**
	  * \brief Récupération de l'indice de la position d'une trace dans le vecteur de traces de la séquence.
	  *
	  * \param spt une référence constante vers la trace à rechercher dans le vecteur.
	  *
	  * \return l'indice de la position de la trace dans le vecteur si elle s'y trouve, ou -1 sinon.
	  */
	int getIndex(const Trace::sp_trace &spt) const;

	/**
	 * \brief Linéarisation de l'ensemble des traces. Le vecteur retourné peut contenir des objets de type Sequence qui marquent le début ou la fin d'une séquence. Toutes les traces comprise entre un début et un fin de séquence font parties de la séquence.
	 * Exemple, la séquence suivante (C modélisent un Call et S une séquence) :
	 *       C
	 *      \ /
	 *   C C S
	 *   \_ _/
	 * C C S C
	 *
	 * Est linéarisée de la manière suivante où Sd est une séquence vide dont le champ info maqrue un début de séquence et Sf est une séquence vide dont le champ info maqrue une fin de séquence :
	 * Sd C C Sd C C Sd C Sf Sf C Sf
	 *
	 * \param start point de départ de linéarisation de la séquence (valeur par défaut : 0)
   * \param end indice de fin de linéarisation de la séquence (valeur par défaut : -1 => fin de la trace)
	 *
	 * \return un vecteur de trace représentant une version linéarisée des traces
	 */
	std::vector<Trace::sp_trace> &getLinearSequence(int start = 0, int end = -1);

	/**
	 * \brief Insérer à la position indiquée le contenu d'une séquence linéarisée
	 * 
	 * \param linearSequence séquence linéarisée à insérer dans la séquence
	 * \param pos position à laquelle insérer la séquence linéarisée
	 */
	void insertLinearSequence (std::vector<Trace::sp_trace> & linearSequence, int pos);

	/**
	 * \brief exporte une séquence linéarisée sous la forme d'une chaine de caractère et écrit le résultat dans le flux "os". Si exportSeqIter == true, une séquence est exportée sous la forme "Seq(Begin*22)", sinon une séquence est exportée sous la forme "Seq(Begin)"
	 */
	static void exportLinearSequenceAsString(std::vector<Trace::sp_trace> & linearSequence, std::ostream &os = std::cout, bool exportSeqIter = true);

	/**
	 * \brief clone une trace linéarisée
	 * 
	 * \param linearSequence trace linéarisée servant de modèle
	 * 
	 * \return le clone de linearSequence
	 */
	static std::vector<Trace::sp_trace> cloneLinearSequence(std::vector<Trace::sp_trace> & linearSequence);

	/**
	 * \brief recherche le point de départ d'une séquence dans une trace linéarisée à partir d'une position de départ. Si la position de départ est un début de séquence alors le point de départ est immédiatement retrouné. Si le point de départ n'est pas un début de séquence alors l'algorithme cherchera en amont le début de séquence dans laquelle la position courrante est incluse.
	 * Exemple:
	 *         +---+---------+
	 *         |   |         |
 	 *         |   |  +---+  |
	 *       \ | / |\ | / |  | 
	 *        \_/  | \_/  |  | 
	 * Sd C C Sd C C Sd C Sf Sf C Sf
	 * 
	 * \param linearSquence une trace linéarisée
	 * \param indice de départ dans la trace linéarisée passée en paramètre
	 * 
	 * \return l'indice de départ dans "linearSequence" de la séquence dans laquelle l'indice "from" est inclus. -1 est retourné si l'algorithme est remonté jusqu'au bout de la trace sans identifié le début de séquence associé à l'indice "from".
	 */
	static int getBeginPosOfLinearSequence(std::vector<Trace::sp_trace> & linearSequence, int from);

  /**
	 * \brief recherche le point terminal d'une séquence dans une trace linéarisée à partir d'une position de départ. Si la position de départ est une fin de séquence alors le point de départ est immédiatement retourné. Si le point de départ n'est pas une fin de séquence alors l'algorithme cherchera en aval la fin de la séquence dans laquelle la position courrante est incluse.
	 * Exemple:
	 *  +---+---------------------+
	 *  |   |                     |
 	 *  |   |         +---+       |
	 *  |   |         | \ | /   \ | / 
	 *  |   |         |  \_/     \_/ 
	 * Sd C C Sd C C Sd C Sf Sf C Sf
	 * 
	 * \param linearSquence une trace linéarisée
	 * \param indice de départ dans la trace linéarisée passée en paramètre
	 * 
	 * \return l'indice de fin dans "linearSequence" de la séquence dans laquelle l'indice "from" est inclus. -1 est retourné si l'algorithme a atteind le bout de la trace sans identifié la fin de la séquence associé à l'indice "from".
	 */
	static int getEndPosOfLinearSequence(std::vector<Trace::sp_trace> & linearSequence, int from);

	/**
	 * \brief retourne l'indice dans la trace linéarisée contenant le nième Call. Si num <= 0 retourne l'indice du premier Call de la trace linéarisée. Si num >= au nombre de call dans la trace linéarisée -1 alors l'indice du dernier Call dans la trace linéarisée est retourné. Peut retourner -1 si la strace linéarisée ne contient aucun Call.
	 */
	static int getCallPosInLinearSequence (std::vector<Trace::sp_trace> & linearSequence, int num);

	/**
	 * \brief retourne le nombre de Call non optionnel dans la séquence linéarisée linearSequence dans l'intervale [start, end[
	 * 
   * \param linearSequence séquence linéarisée dans laquelle faire la recherche
	 * \param start point de départ dans la trace linéarisée (valeur par défaut : 0)
   * \param end indice de fin dans la trace linéarisée (valeur par défaut : -1 => fin de la trace)
	 */
	static int getNonOptCallInLinearSequence (std::vector<Trace::sp_trace> & linearSequence, int start = 0, int end = -1);

	/**
	  * \brief Récupération de l'ensemble des appels contenus dans la séquence et dans ses sous-séquences.
	  *
	  * Cette fonction permet de construire le vecteur des appels contenus dans la séquence et dans ses sous-séquences en parcourant de façon récursive la séquence.
	  *
	  * \param setMod booléen mis à vrai si le vecteur ne doit pas contenir deux appels partageant un même label (pas de doublon).
	  *
	  * \return le vecteur des appels contenus dans la séquence.
	  */
	Call::call_vector getCalls(bool setMod = false);

	/**
	  * \brief Récupération de l'ensemble des séquences contenues dans la séquence et dans ses sous-séquences.
	  *
	  * Cette fonction permet de construire le vecteur des séquences contenues dans la séquence et dans ses sous-séquences en parcourant récursivement la séquence.
	  *
	  * \return le vecteur des séquences contenues dans la séquence.
	  */
	sequence_vector getSequences();

	/**
	  * \brief Ajout d'une répétition pour la séquence.
	  *
	  * Cette fonction met à jour l'objet Sequence::iterDesc de la séquence.
	  */
	void addOne();

	/**
	  * \brief Retire une répétition pour la séquence.
	  *
	  * Cette fonction met à jour l'objet Sequence::iterDesc de la séquence.
	  */
	void removeOne();

	/**
	  * \brief Ajout d'une trace dans le vecteur de la séquence.
	  *
	  * Cette fonction permet d'ajouter une trace dans le vecteur Sequence::traces. La séquence devient le nouveau parent de \p spt durant l'opération.
	  *
	  * \param spt la trace à ajouter dans le vecteur.
	  * \param ind l'indice de la position dans le vecteur Sequence::traces que \p spt doit occupée après l'insertion. Sa valeur par défaut est -1, i.e. que la trace est ajoutée à la fin du vecteur.
	  *
	  * \return vrai si la trace a bien été insérée dans le vecteur Sequence::traces, ou faux sinon.
	  */
	bool addTrace(Trace::sp_trace spt, int ind = -1);

	/**
	  * \brief Récupération de la prochaine trace dans le vecteur de la séquence.
	  *
	  * Cette fonction sert à parcourir les traces contenues dans Sequence::traces de façon non récursive, i.e. qu'on ne rentre pas dans les séquence (s'il y en a). A chaque appel à Sequence::next, la trace se trouvant à l'indice Sequence::pt dans le vecteur Sequence::traces est retournée, la variable Sequence::pt est incrémentée (pour pointer vers la trace suivante qui sera retournée lors du prochain appel à Sequence::next). Le booleén Sequence::endReached est mis à vrai lorsque la dernière trace du vecteur est retournée.
	  *
	  * \return la prochaine trace du vecteur Sequence::traces.
	  */
	const Trace::sp_trace &next();

	/**
	  * \brief Récupération de la prochaine trace dans le vecteur de la séquence.
	  *
	  * Cette fonction sert à parcourir les traces contenues dans Sequence::traces de façon récursive (contrairement à Sequence::next()), i.e. on rentre dans les séquences (s'il y en a). A chaque appel à Sequence::nextRec, la trace se trouvant à l'indice Sequence::pt dans le vecteur Sequence::traces est retournée, la variable Sequence::pt est incrémentée (pour pointer vers la trace suivante qui sera retournée lors du prochain appel à Sequence::next). Le booléen Sequence::endReached est mis à vrai lorsque la dernière trace du vecteur est retournée.
	  *
	  * \return la prochaine trace de la séquence.
	  */
	const Trace::sp_trace &nextRec();

	/**
	  * \brief Réinitialisation du parcours de la séquence
	  *
	  * Cette fonction doit être appelée pour réinitialiser le parcours de la séquence. Lors de la réinitialisation, la variable Sequence::pt est mise à 0 afin de pointer sur la première trace du vecteur, le booléen Sequence::endReached est mis à faux. Cette réinitialisation est effectuée de façon récursive, i.e. pour toutes les séquences et sous-séquences de l'objet Sequence.
	  */
	void reset();

	/**
	  * \brief Supprime le contenu de la séquence
	  *
	  */
	void clear();

	/**
	  * \brief Test si le champ Trace::delayed de la séquence doit être mis à vrai.
	  *
	  * Cette fonction parcours l'ensemble des traces contenues dans la séquence de façon récursive et vérifie les valeurs de leur attribut Trace::delayed. L'attribut Trace::delayed de la séquence est mis à vrai si tous les booléens Trace::delayed des traces qu'elle contient sont à vrai.
	  */
	bool checkDelayed();

	/**
	  * \brief Ajoute une descrition d'itération à Sequence::iterDesc.
	  *
	  * Cette fonction va permettre d'ajouter l'entrée <\p nbIter:\p appearance> dans l'objet Map \p iterDesc si aucune entrée avec la clé \p nbIter ne s'y trouve déjà. Dans le cas contraire, l'entrée existante est modifiée et \p appearance est ajouté à la valeur correspondante à cette entrée.
	  *
	  * \param nbIter la clé de l'entrée à ajouter ou à modifier.
	  * \param appearance la valeur indiquant le nombre de fois que la séquence a été exécutée nbIter fois.
	  */
	void addIteration(unsigned int nbIter, int appearance = 1);

	/**
	  * \brief Fusionne la descritpion des iterations Sequence::iterDesc avec la description passée en paramètre.
	  *
	  * \param newIterationsDesc l'objet décrivant la description des itérations à intégrer.
	  */
	void mergeIterationDescription(const std::map<unsigned int, unsigned int> &newIterationsDesc);

	/**
	  * \brief Complétion de Sequence::iterDesc.
	  *
	  * Il est possible qu'une séquence ne se répète qu'une fois à un moment donné dans les traces brutes. Lors de la compression, cette seule itération n'est alors pas modélisée par un objet Sequence mais par un ensemble de traces dans TracesParser::traces. Cette fonction se base donc sur la séquence parente (Trace::parent) de l'objet Sequence afin de compléter sa variable \p iterDesc en y ajoutant ces itérations manquantes.
	  */
	void completeIterationDescription();

	/**
	  * \brief Récupération des fréquences de répétitions de la séquence.
	  *
	  * Cette fonction construit et retourne un objet Map contenant des entrées <x:z> en se basant sur Sequence::iterDesc. Une entrée <x:z> signifie que la séquence se répète x fois de façon contiguë avec une fréquence égale à z.
	  *
	  * Par exemple, si Sequence::iterDesc = <1:4> <3:5> <10:1>, on obtient le résultat suivant en appelant cette fonction : <1,0.4> <3,0.5> <10,0.1>.
	  *
	  * \see Sequence::iterDesc
	  */
	std::map<unsigned int, double> getPercentageIterationDescription() const;

	/**
	  * \brief Calcul de la distance entre deux objets Sequence::iterDesc.
	  *
	  * La distance entre deux objets Sequence::iterDesc est calculée avec la formule suivante : abs(sl - se) / (sl + se) où 'sl' est la somme des produits x*y pour toute entrée <x:y> de l'objet Sequence::iterDesc de la séquence et 'se' est la somme des produits x*y pour toute entrée <x:y> de l'objet Sequence::iterDesc de \p sps.
	  *
	  * \return la distance calculée.
	  */
	double getIterationDescriptionMeanDistance(const sp_sequence &sps) const;

	/**
	 * \brief Calcule et retourne le meilleur score d'alignement entre deux traces linéarisées. Le meilleur score est celui qui compte le plus d'alignements pour le moins d'options
	 *
	 * \return un couple (nbAlign, nbOpt) où nbAlign indique le nombre de traces alignées et nbOpt indique le nombre de traces optionnelles
	 *
	 * \see Sequence::getLinearSequence
	 */
	static std::pair<int, int> getMaxAlignAndMinOpt(std::vector<Trace::sp_trace> &linearTraces1, std::vector<Trace::sp_trace> &linearTraces2);

	/**
	 * \brief Compte le nombre de Call alignés et optionnels entre deux séquences linéarisées
	 *
	 * \return un couple (nbAlign, nbOpt) où nbAlign indique le nombre de traces alignées et nbOpt indique le nombre de traces optionnelles
	 *
	 * \see Sequence::getLinearSequence
	 */
	static std::pair<int, int> getNbAlignAndOpt(std::vector<Trace::sp_trace> &linearTraces1, std::vector<Trace::sp_trace> &linearTraces2);

protected:
	/**
	 * Un booléen permettant d'indiquer si le nombre d'occurrences de la sequence doit être utilisé lors de l'analyse pour le calcul du score du joueur et pour la détermination des feedbacks. Cette variable est mise à vraie uniquement si l'expert a fixé la valeur de l'attribut 'nb_iteration_fixed' à 'true' pour la séquence dans le fichier XML utilisé pour l'import.
	 */
	bool num_fixed;

	/**
	 * Le vecteur des traces de la séquence.
	 */
	std::vector<Trace::sp_trace> traces;

	/**
	 * Un vecteur contenant les traces linearisées.
	 *
	 * \see Sequence::getLinearSequence
	 */
	std::vector<Trace::sp_trace> linearizedTraces;

	/**
	 * Un entier indiquant le nombre d'occurrences actuel de la séquence.
	 */
	unsigned int num;

	/**
	 * Objet servant à stocker des entrées <x:y>. Une entrée <x:y> signifie que la séquence de traces se répète y fois avec un nombre de répétitions contiguës égal à x.
	 *
	 * Par exemple, si Sequence::iterDesc = <1:4> <3:5> <10:1>, cela signifique que la séquence se répète en tout 1*4 + 3*5 + 10*1 = 29 fois : la sequence c'est exécuté 1 fois à 4 reprises, 3 fois à 5 reprise et 10 fois une seule fois.
	 */
	std::map<unsigned int, unsigned int> iterDesc;

	/**
	 * Un indice sur le vecteur des traces de la séquence. A chaque appel à Sequence::next, cette valeur est incrémentée et le prochain élément du vecteur est renvoyé.
	 */
	unsigned int pt;

	/**
	 * Un booléen mis à vrai lorsque la dernière trace du vecteur de la séquence a été renvoyée suite à l'appel de Sequence::next.
	 */
	bool endReached;

	/**
	 * Un booléen utilisé uniquement lors de la fusion de deux séquences.
	 *
	 * \see TracesParser::mergeEquivalentSequences
	 */
	bool shared;

	/**
	  * Un booléen indiquant si la séquence est une séquence racine. Une séquence racine n'a pas de parent et l'objet Map \p iterDesc ne contient qu'une entrée : <1:1>.
	  *
	  * \see TracesAnalyser::getInfosOnExecution
	  */
	bool root;

	/**
	 * \brief Initialise la position de la variable Sequence::pt et du booléen Sequence::endReached
	 */
	void setPtAndEndState(unsigned int pos, bool endState);

	/**
	 * \brief Modifie une trace linéarisée en y supprimant le premier Call (ou Event)
	 *
	 * \see Sequence::getLinearSequence
	 */
	static void removeFirstCall(std::vector<Trace::sp_trace> &linearTraces);

	/**
	  * \brief Fusionne deux séquences
	  *
	  * Cette fonction permet de construire une nouvelle séquence la plus générale possible à partir de deux séquences \p sps_up et \p sps_down.
	  *
	  * Exemple de cas singuliers :
	  *  1 - [AB[C]] et [[A]BC] => [[A]B[C]]
	  *  2 - [[A[B]]C] et [ABC] => [[A[B]]C]
	  *  
	  * \param sps_up la première séquence passée en entrée de la fusion.
	  * \param sps_down la seconde séquence passée en entrée de la fusion.
	  * \param avoidOptionalSequence si true ne réalise la fusion que si aucune séquence optionnelle n'est requise. Si false, la fusion pourra contenir des séquences optionnelles (true par défaut).
		* \param nbAlign si non NULL ce paramètre contiendra le nombre de trace aligné à la fin de la fusion (paramètre de sortie)
		* \param nbOpt si non NULL ce paramètre contiendra le nombre de trace optionnelle définie à la fin de la fusion (paramètre de sortie)
	  *
	  * \return la nouvelle séquence créée résultante de la fusion de \p sps_up et \p sps_down.
	  *
	  * \see Sequence::findAndAggregateSuccessiveSequences
	  */
	static Sequence::sp_sequence mergeEquivalentSequences(Sequence::sp_sequence sps_up, Sequence::sp_sequence sps_down, bool avoidOptionalSequence = true, int* nbAlign = NULL, int* nbOpt = NULL);

	/**
	 * \brief Fusion de deux séquences alignées
	 *
	 * Cette fonction permet de construire une nouvelle séquence à partir de deux séquences \p seq1 et \p seq2 préalablement alignées (voir TracesAnalyser::findBestAlignment).
	 *
	 * Exemple de cas singuliers : [ABC] aligné avec [A-C] => [A(B)C]
	 *
	 * \param seq1 la première séquence alignée
	 * \param seq2 la seconde séquence alignée
	 *
	 * \return la fusion des deux séquences alignées
	 *
	 * \see TracesAnalyser::findBestAlignment
	 * \see Sequence::mergeEquivalentSequences
	 */
	static Sequence::sp_sequence mergeAlignedSequences(Sequence::sp_sequence seq1, Sequence::sp_sequence seq2);

	/**
	  * \brief Test de la possibilité de répétitions d'un groupe de traces.
	  *
	  * Cette fonction est utilisée par la fonction Sequence::findAndAggregateSuccessiveSequences lors de la recherche de répétitions d'un groupe de traces. Le test consiste à vérifier si la trace contient assez de traces à partir d'un certain indice. Cette fonction est une optimisation permettant d'éviter le lancement d'opérations de comparaison qui retourneront forcément des résultats négatifs.
	  */
	bool checkFeasibility(int min_length, int ind_start);

	/**
	  * \brief Teste si une séquence donnée contient au moins un evènement non concaténable
	  *
	  * \return true si la séquence donnée contient au moins un évènement non concaténable (voir Event::noConcatEventsArr) et false sinon
	  */
	bool includeNoConcatEvent();

	/**
	 * \brief Construit itérativement des fenêtres à partir de workingId d'une taille allant de 1 à maxLength et tente de les fusionner avec sps_up. Si un de ces alignements fournit un meilleur score que celui passé en paramètre alors bestScore et endDown sont mis à jour (bestMerge contient le résultat de la fusion uniquement si un bestScore est obtenu pour une taille de fenêtre up originale ; endDown contient la position dans this->trace de la fin de cette fusion).
	 * Si bestScore >= 0 => la meilleure fusion a pu être faite pour la taille de fenêtre up originale 
	 * Si bestScore < 0 => la meilleure fusion a pu être faite avec une taille de fenêtre up plus grande. Dans ce cas le paramètre bestScore contient l'opposé de la valeur du score calculé et bestMerge n'est pas modifié. Ce dernier cas ne peut avoir lieu que si originalUpSize == false.
	 */
	void evaluateMerging(int workingId, int maxLength, Sequence::sp_sequence sps_up, bool originalUpSize, float & bestScore, Sequence::sp_sequence & bestMerge, int & endDown);

	/**
	 * \brief Supprime dans "traces" toute trace comprise entre "upStart" (inclus) et "endDown" (exclus), insère ensuite "merge" à la position "upStart" dans "traces"
	 */
	static void cleanTracesAndAddMerge(std::vector<Trace::sp_trace> &traces, Sequence::sp_sequence merge, int upStart, int endDown);

	/**
	 * \brief Calcule le meilleur alignement entre deux séquences de traces et retourne le score corrigé sans prise en compte des traces optionnelles initiales
	 */
	static std::pair<double, double> computeBestCorrectedScore(std::vector<Trace::sp_trace> &traces1, std::vector<Trace::sp_trace> &traces2);

	/**
	 * \brief Extrait de la trace l'élément référencé à l'indice currentPos ainsi que son contexte (c'est à dire l'ensemble des traces en amont et en aval pouvant intervenir dans le fenêtre de recherche pour le calcul des partie optionnelles). Renseigne startPos et endPos comme étant les indices de départ et de fin du contexte dans la trace.
	 *
	 * \return une paire dont first indique dans second la position de la trace qui se trouvait à currentPos dans this. Null est retrouné si la trace à l'indice currentPos n'est pas une séquence.
	 */
	std::pair<int, Sequence::sp_sequence> extractContext(int currentPos, int *startPos, int *endPos);

	/**
	  * \brief Tente d'opérer une rotation de la séquence située à l'indice seqPos dans this->traces à l'aide des traces en amont.
	  *
	  * \return la nouvelle position de la séquence initialement positionnée à seqPos.
	  *
	  * Cas typique :
	  *  - Soit la trace : D[ABCD]
	  *  - Correction visée : [DABC]D
	  */
	int tryToProcessUpRotation(int seqPos);

	/**
	  * \brief Tente d'opérer une rotation de la séquence située à l'indice seqPos dans this->traces à l'aide des traces en aval.
	  *
	  * \return la nouvelle position de la séquence initialement positionnée à seqPos.
	  *
	  * Cas typique :
	  *  - Soit la trace : [ABCD]A
	  *  - Correction visée : A[BCDA]
	  */
	int tryToProcessDownRotation(int seqPos);

	/**
	  * \brief Appele respectivement Sequence::processUnaggregateTracesDueToInsertedTokens et Sequence::processInclusiveSequences
	  */
	bool processOptionalTokens(int seqPos);

	/**
	 * \brief Tente de trouver les parties optionnelles et de les fusionner avec les séquences actuelles.
	 *
	 * \return la nouvelle position de la séquence initialement positionnée à seqPos.
	 *
	 * Exemple :
	 *  - ABC[AB]   => [AB(C)]
	 *  - [AB]CAB   => [(C)AB]
	 *  - [ABC]AB   => [AB(C)]
	 */
	int processUnaggregateTracesDueToInsertedTokens(int seqPos);

	/**
	  * \brief Tente de trouver des séquences successives de longueurs différentes (éventuellement séparées par une suite de tokens) qui peuvent être fusionnées.
	 *
	 * \return la nouvelle position de la séquence initialement positionnée à seqPos.
	  *
	  * Cas singuliers :
	  *  1 - [ABC][ABDC] => [AB(D)C]
	  *  2 - [ABC]DE[ABC] => [ABC(DE)]
	  */
	int processInclusiveSequences(int seqPos);
};

#endif
