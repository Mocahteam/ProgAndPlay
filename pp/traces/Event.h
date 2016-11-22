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
 * \brief Classe abstraite héritant de Trace. Cette classe sert de classe mère pour toutes les classes définis dans le fichier EventDef.h.
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
	  * Tableau contenant les labels des événements autorisées en cours d'éxecution du programme du joueur.
	  *
	  * \see TracesParser::handleLine
	  */
	static const char* concatEventsArr[];
	
	/**
	  * Tableau contenant les labels des événements définissant les débuts/fins de mission/d'éxécution.
	  */
	static const char* noConcatEventsArr[];
	
	/**
	  * \brief Récupération de la place occupée par un événement dans un vecteur de traces.
	  *
	  * Retourne 0.
	  *
	  * \return la taille d'un objet événement.
	  */
	virtual unsigned int length() const;
	
	/**
	  * \brief Comparaison de l'objet Event avec une trace \p t.
	  *
	  * \param t : la trace utilisée pour la comparaison.
	  *
	  * \return vrai si la trace \p t est également un événement et si il a le même label que l'objet Event.
	  */
	virtual bool operator==(Trace *t) const;
	
	
	virtual Trace::sp_trace clone() const;
	virtual void display(std::ostream &os = std::cout) const;
	virtual std::string getParams() const;
	
	std::string getLabel() const;
	
protected:
	
	std::string label;

};

#endif