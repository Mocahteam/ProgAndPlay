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

#include "Trace.h"
#include "Call.h"

/**
 * \class Sequence
 * \brief La classe Sequence hérite de la classe @see Trace. Une séquence possède un vecteur d'objets Trace.
 */
class Sequence : public Trace, public std::enable_shared_from_this<Sequence>
{

public:
	/**
	  * Définition du type pointeur intelligent vers un objet Sequence.
	  */
	typedef std::shared_ptr<Sequence> sp_sequence;
	typedef std::shared_ptr<const Sequence> const_sp_sequence;
	typedef std::vector<sp_sequence> sequence_vector;

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
	virtual bool operator==(Trace *t);

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
	virtual bool compare(Trace *t);

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
	  * \brief Get la trace sous la forme d'une chaine de caractère.
	  *
	  */
	virtual std::string getString() const;

	/**
	  * \brief Export de la trace sous la forme d'une chaine de caractère compressée (sur une seule ligne).
	  *
	  * \param os : le flux de sortie utilisé pour l'export.
	  */
	virtual void exportAsCompressedString(std::ostream &os = std::cout) const;

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
				ss << ",";
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
	 * \brief calcule la distance entre deux séquences linéarisée. Calcule une distance de Levenshtein entre deux séquences en considérant les coûts suivants :
	 * Coût vertical, passage d'une ligne à l'autre l[i-1]c[j] -> l[i]c[j]
	 *  - Si key(l[i]) est optionnelle ou une fin de séquence => 0
	 *  - Sinon => 1
	 * Coût horizontal, Passage d'une colonne à l'autre l[i]c[j-1] -> l[i]c[j]
	 *  - Si key(c[j]) est optionnelle ou une fin de séquence => 0
	 *  - Sinon => 1
	 * Coût diagonale, Passage de l[i-1]c[j-1] -> l[i]c[j]
	 *  - Si key(l[i]) != key(c[j]) => 0
	 *  - Sinon => 1
	 * 
	 * Si l1 = ABC et l2 = AC, la matrice résultat sera
	 *     - A C
	 * - [[0 1 2],
	 * A  [1 0 1],
	 * B  [2 1 1],
	 * C  [3 2 1]]
	 * 
	 * \param l1 la première liste de traces
	 * \param l2 la seconde liste de traces
	 * 
	 * \return la matrice d'alignement indiquant comment les deux listes de traces ont été alignées. La valeur située en bas à droite de la matrice représente la distance entre l1 et l2
	*/
	static std::vector<std::vector<int>> computeLinearSequenceDistance(std::vector<Trace::sp_trace> & l1, std::vector<Trace::sp_trace> & l2);

	/**
	  * \brief Détection des paramètres non robustes et nombre d'itération différent.
	  *
	  * Cette fonction est utilisée pour affecter aux paramètres non robustes de l'appel, i.e. ceux non pris en compte lors de la compression et dont la valeur diffère entre deux itération d'une même séquence, la valeur -1.
	  * Attention : cette fonction doit être appelée uniquement si operator==(s) renvoie vrai.
	  *
	  * \param s : un pointeur vers la sequence utilisée pour la comparaison.
	  */
	void filterSequence(const Sequence *s);

	/**
	 * \brief Fusionne deux séquences linéarisées
	 *
	 * Cette fonction permet de construire une nouvelle séquence linéarisée la plus générale possible à partir de deux séquences \p s1 et \p s2.
	 *
	 * Exemple de cas singuliers :
	 *  1 - [AB[C]] et [[A]BC] => [[A]B[C]]
	 *  2 - A[C] et AB => A[*C]B
	 *  3 - A[BC] et [AB]C => [*A[B]*C]
	 *  4 - A[B] et [AB] => [A[B]]
	 *  5 - [A]B et [AB] => [[A]B]
	 *  
	 * \param s1 la première séquence linéarisée passée en entrée de la fusion.
	 * \param s2 la seconde séquence linéarisée passée en entrée de la fusion.
	 *
	 * \return la nouvelle séquence créée résultante de la fusion de \p s1 et \p s2.
	 */
	static std::vector<Trace::sp_trace> mergeLinearSequences(std::vector<Trace::sp_trace> & s1, std::vector<Trace::sp_trace> & s2);

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
	 * \brief Gère le dépilement de la pile en fonction du choix des Begin (haut, gauche ou diagonale)
	*/
	static void unstackBegin(std::vector<std::tuple<std::string, Trace::sp_trace, bool>> & stack, std::vector<Trace::sp_trace> & mergedSequence, std::string action, Sequence::sp_sequence begin);
	
	/**
	 * \brief Gère la pile d'imbrication des séquences Begin et End. Les End sont empilés et les Begin dépilent les End. A l'étape 0 les End sont tous positionné comme optionnels. Pour les dépilement voir unstackBegin
	 * 
	 * \param border ne doit pas être autre chose qu'une séquence taguée Begin ou End
	*/
	static void manageStack(std::vector<std::tuple<std::string, Trace::sp_trace, bool>> & stack, std::vector<Trace::sp_trace> & mergedSequence, std::string action, Sequence::sp_sequence border, int step);
	
	/**
	 * \brief Procède à la fusion de \p s1 et \p s2 à l'aide de la matrice de transformation \p transformationMatrix en deux étapes \p step.
	 * 
	 * \param s1 la première séquence linéarisée passée en entrée de la fusion.
	 * \param s2 la seconde séquence linéarisée passée en entrée de la fusion.
	 * \param transformationMatrix la matrice de transformation permettant de savoir comment aligner les traces de \p s1 et \p s2 (voir computeLinearSequenceDistance pour la génération de cette matrice de transformation)
	 * \param step définit l'étape à appliquer pour traiter mergedSequence. A l'étape 0 mergedSequence est construite (sans définir les options), à l'étape 1 mergedSequence est modifiée pour y définir les traces optionnelles.
	 * \param mergedSequence résultat de la fusion entre \p s1 et \p s2. Lors de l'étape 1 mergedSequence est construite, lors de l'étape 2 mergedSequence est mise à jour pour y définir les traces optionnelles.
	*/
	static void mergeLinearSequences_local(std::vector<Trace::sp_trace> & s1, std::vector<Trace::sp_trace> & s2, std::vector<std::vector<int>> & transformationMatrix, int step, std::vector<Trace::sp_trace> & mergedSequence);
};

#endif
