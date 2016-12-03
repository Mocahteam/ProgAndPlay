/**
 * \file Trace.h
 * \brief Déclaration de la classe Trace
 * \author meresse
 * \version 0.1
 */

#ifndef __TRACE_H__
#define __TRACE_H__

#include <stdexcept>
#include <vector>
#include <iostream>
#include <string.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

/** 
 * \class Trace
 * \brief La classe Trace est une classe abstraite servant de classe mère aux classes Sequence, Call et Event.
 */
class Trace {

public:

	/**
	  * Définition du type pointeur intelligent vers un objet Trace.
	  */
	typedef boost::shared_ptr<Trace> sp_trace;

	/**
	  * \brief Enumeration utilisée pour connaître le type de la trace. Une trace peut être de type Trace::TraceType::SEQUENCE, Trace::TraceType::CALL ou Trace::TraceType::EVENT.
	  */
	enum TraceType {
		SEQUENCE,
		CALL,
		EVENT
	};

	/**
	  * \brief Destructeur de Trace.
	  */
	virtual ~Trace() {}
	
	/**
	  * \brief Retourne la longueur d'une trace.
	  *
	  * Cette fonction est implémentée dans les classes héritant de Trace.
	  *
	  * \return la longueur de la trace.
	  *
	  * \see Sequence::length
	  * \see Call::length
	  * \see Event::length
	  */
	virtual unsigned int length() const = 0;
	
	/**
	  * \brief Comparaison de la trace avec une trace \p t.
	  *
	  * Cette fonction est implémentée dans les classes héritant de Trace.
	  *
	  * \param t : la trace utilisée pour la comparaison.
	  *
	  * \return vrai si la trace est considérée comme étant égale à \p t, ou faux sinon.
	  */
	virtual bool operator==(Trace *t) const = 0;
	
	/**
	  * \brief Copie de la trace.
	  *
	  * \return un pointeur intelligent vers la nouvelle trace créée suite au clonage.
	  */
	virtual sp_trace clone() const = 0;
	
	/**
	  * \brief Affichage de la trace.
	  *
	  * \param os : le flux de sortie utilisé pour l'affichage.
	  */
	virtual void display(std::ostream &os = std::cout) const = 0;
	
	/**
	  * \brief Remise à zéro du pointeur \p aligned.
	  *
	  * Cette fonction permet de supprimer le lien vers une trace avec laquelle cette trace était alignée.
	  *
	  * \see Sequence::resetAligned
	  */
	virtual void resetAligned();
	
	/**
	  * \brief Recherche d'une chaîne de caractères dans un tableau de chaînes de caractères.
	  *
	  * \param ch : la chaîne de caractères recherchée dans \p arr.
	  * \param arr : le tableau dans lequel est recherchée \p ch. \p arr doit obligatoirement se terminer par NULL.
	  *
	  * \return -1 si \p ch n'est pas présent dans \p arr, et l'indice de sa position dans \p arr sinon.
	  */
	static int inArray(const char *ch, const char *arr[]);
	
	/**
	  * \brief Récupération de la longueur du sous-vecteur [ind_start,ind_end[ du vecteur \p traces.
	  *
	  * \param traces : le vecteur contenant les traces.
	  * \param ind_start : l'indice du début du sous-vecteur dans \p traces. Sa valeur par défaut est 0.
	  * \param ind_end : l'indice de fin du sous-vecteur dans \p traces. Sa valeur par défaut est traces.size().
	  *
	  * \return la longueur du sous-vecteur calculée.	  
	  */
	static unsigned int getLength(const std::vector<sp_trace>& traces, int ind_start = 0, int ind_end = -1);
	
	/**
	  * Variable utilisée pour obtenir une indentation valide des traces lors de leur affichage.
	  */
	static int numTab;
	
	/**
	  * Indice de la trace dans TracesParser::traces sur laquelle est placé le curseur lors de la recherche de répétitions d'un groupe de traces à partir de la trace possédant cette variable.
	  *
	  * \see TracesParser::detectSequences
	  */
	int indSearch;
	
	/**
	  * La somme des longueurs des traces contenues dans le vecteur TracesParser::traces et comprise entre la trace qui se trouve à la position Trace::indSearch (incluse) et celle possédant cette variable (excluse).
	  *
	  * \see TracesParser::detectSequences
	  */
	unsigned int lenSearch;
	
	/**
	  * Compteur utilisé pour stopper et éviter toute future recherche de répétitions d'un groupe de traces à partir de la trace possédant cette variable lorsque sa valeur a atteint un certain seuil MAX_END_SEARCH défini dans le fichier TracesParser.h.
	  *
	  * \see TracesParser::detectSequences
	  */
	unsigned int endSearch;
	
	/**
	  * \brief Teste si la trace est une séquence.
	  *
	  * \return vrai si la trace est une séquence, faux sinon.
	  */
	bool isSequence() const;
	
	/**
	  * \brief Teste si la trace est un event.
	  *
	  * \return vrai si la trace est un event, faux sinon.
	  */
	bool isEvent() const;
	
	/**
	  * \brief Teste si la trace est un call.
	  *
	  * \return vrai si la trace est un call, faux sinon.
	  */
	bool isCall() const;
	
	/**
	  * \brief Getter pour la variable Trace::delayed.
	  *
	  * \see Trace::delayed
	  */
	bool isDelayed() const;
	
	/**
	  * \brief Setter pour la variable Trace::delayed.
	  *
	  * La variable Trace::delayed est mise à vraie. 
	  *
	  * \see Trace::delayed
	  */
	void setDelayed();
	
	/**
	  * \brief Getter pour la variable \p info.
	  *
	  * \return la chaîne de caractères \p info de la trace.
	  */
	std::string getInfo() const;
	
	/**
	  * \brief Setter pour la variable \p info.
	  *
	  * \param info la nouvelle valeur du champ \p info pour la trace.
	  */
	void setInfo(std::string info);
	
	/**
	  * \brief Getter pour la variable \p parent.
	  *
	  * \return une référence constante à la variable \p parent de la trace.
	  */
	const sp_trace& getParent() const;
	
	/**
	  * \brief Getter pour la variable \p aligned.
	  *
	  * \return une référence constante à la variable \p aligned de la trace.
	  */
	const sp_trace& getAligned() const;
	
	/**
	  * \brief Setter pour la variable \p parent.
	  *
	  * Cette méthode est utilisée dans Sequence::addTrace pour définir comme parent de la trace la séquence dans laquelle on l'ajoute.
	  *
	  * \param spt le nouveau \p parent de la trace.
	  */
	void setParent(const sp_trace& spt);
	
	/**
	  * \brief Setter pour la variable \p aligned.
	  *
	  * \param spt la nouvelle valeur pour la variable \p aligned de la trace.
	  */
	void setAligned(const sp_trace& spt);
	
	/**
	  * \brief Retoune le niveau de la trace dans la hierarchie globale.
	  *
	  * Cette information n'est pas stockée et est donc calculée lors de chaque appel à cette fonction (afin d'éviter les oublis de mise à jour).
	  * Il correspond au nombre de séquences parentes comptées en remontant la hiérarchie à partir de la trace.
	  *
	  * \return le niveau de la trace
	  */
	unsigned int getLevel() const;
		
protected:

	/**
	  * \brief Constructeur principal de Trace.
	  */
	Trace(TraceType type, std::string info = "");
	
	/**
	  * \brief Constructeur de Trace utilisé pour le clonage d'une trace.
	  */
	Trace(const Trace *t);
	
	/**
	  * Le type de la trace. Une trace peut-être de type CALL, EVENT ou SEQUENCE.
	  */
	TraceType type;
	
	/**
	  * Un label ajouté par l'expert dans le fichier XML utilisé pour l'import.
	  */
	std::string info;
	
	/**
	  * Un booléen mis à vrai lorsque la trace a été générée par le moteur alors que la mission était déjà terminée.
	  */
	bool delayed;
	
	/**
	  * Contient un pointeur vers la trace alignée avec cette trace durant la phase d'alignement.
	  *
	  * \see TracesAnalyser::findBestAlignment
	  */
	sp_trace aligned;
	
	/**
	  * Contient un pointeur vers la séquence contenant cette trace. Si le pointeur est à 0, la trace n'a pas de parent. Si le pointeur pointe vers un objet, cet objet est forcément
	  * une séquence.
	  */
	sp_trace parent;
	
};

#endif