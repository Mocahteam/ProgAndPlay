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
class Sequence : public Trace, public boost::enable_shared_from_this<Sequence> {
	
public:

	/**
	  * Définition du type pointeur intelligent vers un objet Sequence.
	  */
	typedef boost::shared_ptr<Sequence> sp_sequence;
	typedef boost::shared_ptr<const Sequence> const_sp_sequence;
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
	  */
	Sequence(unsigned int num, bool root = false);
	
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
	  * \see TracesParser::mergeSequences
	  */
	Sequence(const_sp_sequence sps_up, const_sp_sequence sps_down);
	
	/**
	  * \brief Récupération de la longueur (l'espace occupé dans un vecteur de traces) d'une séquence.
	  *
	  * La longueur d'une séquence est calculée de façon récursive en parcourant ses séquences et sous-séquences et en appelant Trace::length sur les traces parcourues.
	  *
	  * \return la longueur de la séquence
	  */
	virtual unsigned int length() const;
	
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
	  *         Sequence
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
	  * \brief Clonage d'une séquence.
	  *
	  * \return une copie de l'objet Sequence.
	  */
	virtual Trace::sp_trace clone() const;
	
	/**
	  * \brief Affichage des informations de l'objet Sequence.
	  *
	  * \param os le flux de sortie utilisé pour l'affichage.
	  */
	virtual void display(std::ostream &os = std::cout) const;
	
	/**
	  * \brief Réinitialisation récursive de l'alignement de la séquence.
	  *
	  * Cette fonction permet de supprimer le lien vers la trace avec laquelle la séquence était éventuellement alignée, i.e. de réinitialiser le pointeur intelligent Trace::aligned de la séquence. Ce traitement est effectué de façon récursive pour toutes les traces inclus dans la séquence.
	  */
	virtual void resetAligned();
	
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
	  *         Sequence
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
	  * \brief Construction d'une chaîne de caractères contenant les entrées d'un objet Sequence::numMap.
	  *
	  * \param numMap l'objet numMap dont les informations doivent être récupérées sous forme de chaîne de caractères.
	  *
	  * \return la chaîne de caractères construite.	  
	  */
	template<typename T>
	static std::string getNumMapString(const std::map<unsigned int,T>& numMap) {
		std::stringstream ss;
		typename std::map<unsigned int,T>::const_iterator it = numMap.begin();
		while (it != numMap.end()) {
			if (it != numMap.begin())
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
	std::vector<Trace::sp_trace>& getTraces();
	
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
	  * \brief Getter pour la variable Sequence::numMap.
	  *
	  * \see Sequence::numMap
	  */
	const std::map<unsigned int,unsigned int>& getNumMap() const;
	
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
	  * Une séquence est implicite si la seule entrée contenue dans Sequence::numMap est <1:1>.
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
	  * \brief Accès à une trace du vecteur de traces de la séquence.
	  *
	  * \param i l'indice de la trace à récupérer dans le vecteur.
	  *
	  * \return Une référence constante vers la trace qui se trouve à l'indice \p i dans le vecteur de traces de la séquence.
	  */
	const Trace::sp_trace& at(unsigned int i) const;
	
	/**
	  * \brief Récupération de l'indice de la position d'une trace dans le vecteur de traces de la séquence.
	  *
	  * \param spt une référence constante vers la trace à rechercher dans le vecteur.
	  *
	  * \return l'indice de la position de la trace dans le vecteur si elle s'y trouve, ou -1 sinon.
	  */
	int getIndex(const Trace::sp_trace& spt) const;
	
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
	  * Cette fonction met à jour l'objet Sequence::numMap de la séquence.
	  */
	void addOne();
	
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
	const Trace::sp_trace& next();
	
	/**
	  * \brief Réinitialisation du parcours de la séquence
	  *
	  * Cette fonction doit être appelée pour réinitialiser le parcours de la séquence. Lors de la réinitialisation, la variable Sequence::pt est mise à 0 afin de pointer sur la première trace du vecteur, le booléen Sequence::endReached est mis à faux. Cette réinitialisation est effectuée de façon récursive, i.e. pour toutes les séquences et sous-séquences de l'objet Sequence.
	  */
	void reset();
	
	/**
	  * \brief Test si le champ Trace::delayed de la séquence doit être mis à vrai.
	  *
	  * Cette fonction parcours l'ensemble des traces contenues dans la séquence de façon récursive et vérifie les valeurs de leur attribut Trace::delayed. L'attribut Trace::delayed de la séquence est mis à vrai si tous les booléens Trace::delayed des traces qu'elle contient sont à vrai.
	  */
	bool checkDelayed();
	
	/**
	  * \brief Mise à jour de Sequence::numMap.
	  *
	  * Cette fonction va permettre d'ajouter l'entrée <\p num:\p update> dans l'objet Map \p numMap si aucune entrée avec la clé \p num ne s'y trouve déjà. Dans le cas contraire, l'entrée existante est modifiée et \p update est ajoutée à la valeur correspondante à cette entrée.
	  *
	  * \param num la clé de l'entrée à ajouter ou à modifier.
	  * \param update la valeur de la mise à jour.
	  */
	void updateNumMap(unsigned int num, int update = 1);
	
	/**
	  * \brief Mise à jour de Sequence::numMap à partir d'une autre Sequence::numMap.
	  *
	  * Cette fonction est utilisée pour mettre à jour la variable Sequence::numMap de la séquence avec chacune des entrées de l'objet \p numMap passé en paramètres.
	  *
	  * \param numMap l'objet Map Sequence::numMap utilisé pour la mise à jour.
	  */
	void updateNumMap(const std::map<unsigned int,unsigned int>& numMap);
	
	/**
	  * \brief Complétion de Sequence::numMap.
	  *
	  * Il est possible qu'une séquence ne se répète qu'une fois à un moment donné dans les traces brutes. Lors de la compression, cette seule itération n'est alors pas modélisée par un objet Sequence mais par un ensemble de traces dans TracesParser::traces. Cette fonction se base donc sur la séquence parente (Trace::parent) de l'objet Sequence afin de compléter sa variable \p numMap en y ajoutant ces itérations manquantes.
	  */
	void completeNumMap();
	
	/**
	  * \brief Récupération des fréquences de répétitions de la séquence.
	  *
	  * Cette fonction construit et retourne un objet Map contenant des entrées <x:z> en se basant sur Sequence::numMap. Une entrée <x:z> signifie que la séquence se répète x fois de façon contiguë avec une fréquence égale à z.
	  *
	  * Par exemple, si Sequence::numMap = <1:4> <3:5> <10:1>, on obtient le résultat suivant en appelant cette fonction : <1,0.4> <3,0.5> <10,0.1>.
	  *
	  * \see Sequence::numMap
	  */
	std::map<unsigned int,double> getPercentageNumMap() const;
	
	/**
	  * \brief Calcul de la distance entre deux objets Sequence::numMap.
	  *
	  * La distance entre deux objets Sequence::numMap est calculée avec la formule suivante : abs(sl - se) / (sl + se) où 'sl' est la somme des produits x*y pour toute entrée <x:y> de l'objet Sequence::numMap de la séquence et 'se' est la somme des produits x*y pour toute entrée <x:y> de l'objet Sequence::numMap de \p sps.
	  *
	  * \return la distance calculée.
	  */
	double getNumMapMeanDistance(const sp_sequence& sps) const;
	
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
	 * Par exemple, si Sequence::numMap = <1:4> <3:5> <10:1>, cela signifique que la séquence se répète en tout 1*4 + 3*5 + 10*1 = 29 fois : 4 fois avec une seule répétition, 5 fois avec 3 répétitions contiguës et enfin 1 fois avec 10 répétitions contiguës.
	 */
	std::map<unsigned int,unsigned int> numMap;
	
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
	 * \see TracesParser::mergeSequences
	 */
	bool shared;
	
	/**
	  * Un booléen indiquant si la séquence est une séquence racine. Une séquence racine n'a pas de parent et l'objet Map \p numMap ne contient qu'une entrée : <1:1>.
	  *
	  * \see TracesAnalyser::getInfosOnExecution
	  */
	bool root;
	
};

#endif