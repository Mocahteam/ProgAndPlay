/**
 * \file Call.h
 * \brief Déclaration des classes Call et ParamsMap
 * \author meresse
 * \version 0.1
 */

#ifndef __CALL_H__
#define __CALL_H__

#define MAX_SIZE_PARAMS 2

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <rapidjson/document.h>
#include <boost/lexical_cast.hpp>

#include "Trace.h"

/** 
  *	si DEFAULT_COMPRESSION_MOD == 0, alors aucune valeur de paramètre n'est utilisé durant la compression. Sinon, tous les paramètres sont pris en compte pour la compression.
  */
#define DEFAULT_COMPRESSION_MOD 0

#define EDIT_MATCH_SCORE 0
#define EDIT_MISMATCH_SCORE 1
#define EDIT_GAP_SCORE 1

/** 
 * \class ParamsMap
 * \brief Classe utilisée pour le chargement des paramètres de compression à partir du fichier JSON 'params.json'.
 */
class ParamsMap {
		
public:

	/**
	  * \brief Constructeur de la classe ParamsMap.
	  */
	ParamsMap() : loaded(false) {}
	
	/**
	  * \brief Chargement des paramètres de compression
      *
	  * Le booléen \p loaded est mis à vrai si la chaine à bien été parsée, de sorte que le chargement ne puisse être effectué qu'une seule fois. 
	  * 
      * \param json : une chaîne de caractères contenant les données au format JSON.
	  */
	void initMap(const std::string& json) {
		if (!loaded) {
			try {
				rapidjson::Document doc;
				if (doc.Parse<0>(json.c_str()).HasParseError())
					throw std::runtime_error("parse error");
				if (doc.IsObject()) {
					for (rapidjson::Value::ConstMemberIterator it = doc.MemberBegin(); it != doc.MemberEnd(); it++) {
						string_vector v;
						for (rapidjson::Value::ConstMemberIterator _it = it->value.MemberBegin(); _it != it->value.MemberEnd(); _it++) {
							if (_it->value.IsBool() && _it->value.GetBool())
								v.push_back(_it->name.GetString());
						}
						map.insert(std::make_pair<std::string,string_vector>(it->name.GetString(),v));
					}
					loaded = true;
				}
			}
			catch (const std::runtime_error& e) {
				std::cout << e.what() << std::endl;
			}
		}
	}
	
	/**
	  * \brief Récupération des paramètres de compression
      *
	  * Le booléen \p loaded est mis à vrai si la chaine à bien été parsée, de sorte que le chargement ne puisse être effectué qu'une seule fois.
	  *
	  * Si cette fonction alors que \p loaded est à faux (la fonction ParamsMap::initMap n'a pas été appelée ou le parsing a échoué), un mode de compression par défaut est utilisé.
	  * Suivant la valeur de DEFAULT_COMPRESSION_MOD :
	  * 	- 0 : Aucun paramètre n'est pris en considération pour la compression (compression maximale)
	  *		- 1 : Tous les paramètres sont pris en considération pour la compression (compression minimale)
	  * 
      * \param label : un label de fonction utilisée pouvant être utilisée dans Call.
	  * \param param : un nom de paramètre associé à la fonction \p label.
	  *
	  * \return vrai si la valeur du paramètre \p param du label \p label doit être pris en considération lors de la compression et faux sinon.
	  */
	bool contains(std::string label, std::string param) const {
		if (!loaded) {
			#if DEFAULT_COMPRESSION_MOD == 1
				return true;
			#else
				return false;
			#endif
		}
		return map.find(label) != map.end() && std::find(map.at(label).begin(), map.at(label).end(), param) != map.at(label).end();
	}
	
private:

	typedef std::vector<std::string> string_vector;
	
	/**
	  * Une entrée label:param signifie que le paramètre param de la fonction label doit être pris en considération pour la compression, i.e. que les valeurs de ce paramètre pour deux appels doivent être égaux pour que ces deux appels puissent être considérés comme égaux.
	  */
	std::map<std::string,string_vector> map;
	
	/**
	  * Un booléen mis à vrai lorsque la fonction ParamsMap::initMap a été appelée et que les paramètres de compression ont bien été chargés.
	  */
	bool loaded;
	
};


/** 
 * \class Call
 * \brief Classe abstraite héritant de Trace. Cette classe sert de classe mère pour toutes les classes définis dans le fichier CallDef.h.
 */
class Call : public Trace {
	
public:

	typedef boost::shared_ptr<Call> sp_call;
	typedef std::vector<sp_call> call_vector;

	/**
	  * \brief Enumération utilisée pour connaître le type d'erreur associé à l'objet Call.
	  */
	enum ErrorType {
		NONE = -1,
		OUT_OF_RANGE,
		WRONG_COALITION,
		WRONG_UNIT,
		WRONG_TARGET,
		WRONG_POSITION
	};
	
	/**
	  * \brief Constructeur principal de la classe Call.
	  *
	  * \param label le label de la fonction modélisée par l'objet Call.
	  * \param error l'erreur survenue lors de l'appel de la fonction représentée par l'objet Call.
	  * \param info le label attribué par l'expert.
	  */
	Call(std::string label, ErrorType error, std::string info = "");
	
	/**
	  * Constructeur de la classe Call utilisé notamment lors de la copie de l'objet.
	  */
	Call(const Call *c);
	
	/**
	  * Tableau contenant les chaînes de caractères associées aux différentes valeurs définies dans l'enumération Call::ErrorType.
	  */
	static const char* errorsArr[];
	
	/**
	  * Tableau contenant les chaînes de caractères associées aux différentes valeurs définies dans l'enumération CallMisc::Coalition.
	  */
	static const char* coalitionsArr[];
	
	/**
	  * Tableau contenant les labels autorisés à être utilisé pour l'instanciation d'un objet NoParamCall.
	  */
	static const char* noParamCallLabelsArr[];
	
	/**
	  * Tableau contenant les labels autorisés à être utilisé pour l'instanciation d'un objet UnitCall.
	  */
	static const char* unitCallLabelsArr[];
	
	/**
	  * Variable utilisée pour le chargement et le stockage des paramètres de compression. 
	  *
	  * \see ParamsMap
	  */
	static ParamsMap paramsMap;
	
	/**
	  * \brief Récupération de la valeur d'une énumération à partir d'une chaîne de caractères.
	  *
	  * La chaîne \p ch est recherchée dans le tableau \p arr et l'indice de sa position est utilisée pour récupérér la bonne valeur d'énumération.
	  *
	  * \param ch : la chaîne de caractères.
	  * \param arr : le tableau de chaînes de caractères dans lequel \p ch est recherchée.
	  *
	  * \return la valeur de l'énumération associée à \p ch.
	  *
	  * \see Call::getEnumLabel
	  */
	template<typename E>
	static E getEnumType(const char *ch, const char **arr) {
		return static_cast<E>(Trace::inArray(ch,arr));
	}

	/**
	  * \brief Récupération d'une chaîne de caractères à partir de la valeur d'une énumération.
	  *
	  * On utilise l'entier associé à la valeur de l'énumération pour récupérer la chaîne de caractères contenu dans \p arr.
	  *
	  * \param e : la valeur de l'énumération.
	  * \param arr : le tableau de chaînes de caractères où l'on récupère la chaîne.
	  *
	  * \return la chaîne de caractères associée à la valeur de l'énumération si elle peut être récupérée dans \p arr, ou NULL sinon.
	  *
	  * \see Call::getEnumType
	  */
	template<typename E>
	static const char* getEnumLabel(E e, const char **arr) {
		int ind = static_cast<int>(e);
		if (ind > -1)
			return arr[ind];
		return NULL;
	}
	
	virtual unsigned int length() const;
	virtual bool operator==(Trace *t) const;
	virtual void filterCall(const Call *c);
	virtual void display(std::ostream &os = std::cout) const;
	virtual Trace::sp_trace clone() const = 0;
	virtual std::string getParams() const = 0;
	virtual std::string getReadableParams() const = 0;
	virtual std::vector<std::string> getListIdWrongParams(Call *c = NULL) const;
	
	double getEditDistance(const Call *c) const;
	
	std::string getLabel() const;
	ErrorType getError() const;
	bool addReturnCode(float code);
	std::string getReturn() const;
	bool compareReturn(const Call *c) const;
	void setReturn();
	bool hasReturn() const;
	
protected:

	virtual bool compare(const Call *c) const = 0;
	virtual void filter(const Call *c) = 0;
	virtual std::pair<int,int> distance(const Call *c) const = 0;
	virtual std::vector<std::string> id_wrong_params(Call *c) const = 0;
	
	/**
	  * Le label associé permettant d'identifier la fonction modélisée par l'objet Call.
	  */
	std::string label;
	
	/**
	  * Variable permettant de connaître le type d'erreur retourné lors de l'appel à la fonction Call::label, si une erreur a été retournée. Si aucune erreur n'est présente, la variable prend la valeur Call::ErrorType::NONE.
	  */
	ErrorType error;
	
	/**
	  * Indice utilisée pour remplir le tableau Call::ret.
	  */
	int ind_ret;
	
	/**
	  * Tableau contenant les valeurs de retour de l'appel à la fonction Call::label.
	  */
	float ret[MAX_SIZE_PARAMS];
		
};

#endif