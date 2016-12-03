/**
 * \file Event.h
 * \brief Déclaration de la classe Event
 * \author meresse
 * \version 0.1
 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include <iostream>

#include "Trace.h"

/** 
 * \class Event
 * \brief Classe héritant de Trace. Cette classe sert de classe mère pour toutes les classes définies dans le fichier EventDef.h.
 */
class Event : public Trace {
	
public:

	/**
	  * Définition du type pointeur intelligent vers un objet Event.
	  */
	typedef boost::shared_ptr<Event> sp_event;
	
	/**
	  * Constructeur principal de la classe Event.
	  *
	  * \param label le label de l'événement modélisé par l'objet Event.
	  * \param info le label attribué par l'expert.
	  */
	Event(std::string label, std::string info = "");
	
	/**
	  * Constructeur de la classe Event utilisé notamment lors de la copie de l'objet.
	  */
	Event(const Event *e);
	
	/**
	  * Tableau contenant les labels des événements pris en compte lors du parsing du fichier de traces brutes.
	  *
	  * \see TracesParser::handleLine
	  */
	static const char* concatEventsArr[];
	
	/**
	  * Tableau contenant les labels des événements définissant les débuts/fins de mission/d'éxécution.
	  */
	static const char* noConcatEventsArr[];
	
	/**
	  * \brief Récupération de la longueur (l'espace occupé dans un vecteur de traces) d'un événement.
	  *
	  * \return 0
	  */
	virtual unsigned int length() const;
	
	/**
	  * \brief Comparaison de l'objet Event avec une trace \p t.
	  *
	  * \param t : la trace utilisée pour la comparaison.
	  *
	  * \return vrai si la trace \p t est également un événement et si elle a le même label que cet événement.
	  */
	virtual bool operator==(Trace *t) const;
	
	/**
	  * \brief Clonage d'un événement.
	  *
	  * \return une copie de l'objet Event.
	  */
	virtual Trace::sp_trace clone() const;
	
	/**
	  * \brief Affichage des informations de l'objet Event.
	  *
	  * \param os le flux de sortie utilisé pour l'affichage.
	  */
	virtual void display(std::ostream &os = std::cout) const;
	
	/**
	  * \brief Retourne les différents paramètres relatifs à l'événement sous forme de chaîne de caractères.
	  *
	  * Cette fonction doit être redéfinie par les classes héritant de Event.
	  *
	  * \return la chaîne de caractères formatée contenant les valeurs des différents paramètres de l'événement séparées par des espaces.
	  */
	virtual std::string getParams() const;
	
	/**
	  * \brief Getter pour la variable \p label.
	  *
	  * \return la chaîne de caractères \p label associée à l'événement.
	  */
	std::string getLabel() const;
	
protected:
	
	/**
	  * Le label associé permettant d'identifier l'événement modélisé par l'objet Event.
	  */
	std::string label;

};

#endif