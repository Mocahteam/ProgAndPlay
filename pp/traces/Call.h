/**
 * \file Call.h
 * \brief Déclaration des classes Call et CallMaps
 * \author meresse
 * \version 0.1
 */

#ifndef __CALL_H__
#define __CALL_H__

#define MAX_SIZE_PARAMS 2
#define FLOAT_EPSILON 1

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
#include "TraceConstantList.h"

/**
 * \class CallMaps
 * \brief Classe utilisée pour le chargement des paramètres de compression à partir du fichier JSON 'params.json'.
 */
class CallMaps
{

public:
	/**
	  * \brief Constructeur de la classe CallMaps.
	  */
	CallMaps() : loaded(false) {}

	/**
	  * \brief Chargement des paramètres de compression
    *
	  * Le booléen \p loaded est mis à vrai si la chaine à bien été parsée.
	  *
    * \param json : une chaîne de caractères contenant les données au format JSON.
		* \param p_lang : une chaîne de caractères décrivant le langage de programmation utilisé.
		* \param h_lang : une chaîne de caractères décrivant la langue utilisée.
	  */
	void initMaps(const std::string &json, const std::string &p_lang, const std::string &h_lang)
	{
		loaded = false;
		keyToUsefullParams_map.clear();
		keyToCallType_map.clear();
		keyToPublicLabel_map.clear();
		try
		{
			rapidjson::Document doc;
			if (doc.Parse<0>(json.c_str()).HasParseError())
				throw std::runtime_error("parse error");
			if (doc.IsObject())
			{
				for (rapidjson::Value::ConstMemberIterator it = doc.MemberBegin(); it != doc.MemberEnd(); it++)
				{
					string_vector v;
					std::string label = it->name.GetString();
					if (label.compare("_comment") != 0)
					{ // exlude json comments
						for (rapidjson::Value::ConstMemberIterator _it = it->value.MemberBegin(); _it != it->value.MemberEnd(); _it++)
						{
							std::string key = _it->name.GetString();
							if (_it->value.IsBool() && _it->value.GetBool())
								v.push_back(key);
							else if (key.compare("call_type") == 0)
							{
								std::string call_type = _it->value.GetString();
								keyToCallType_map.insert(std::pair<std::string, std::string>(label, call_type));
							}
							else if (key.compare(p_lang + "_public_label") == 0)
							{
								std::string public_label = _it->value.GetString();
								if (public_label.find("[" + h_lang + "]") != std::string::npos)
								{ // try to find human language use in game
									std::string locale_label = public_label.substr(public_label.find("[" + h_lang + "]") + 4);
									// try to find second end token
									if (locale_label.find("[" + h_lang + "]") != std::string::npos)
									{
										public_label = locale_label.substr(0, locale_label.find("[" + h_lang + "]"));
									}
								}
								else if (public_label.find("[en]") != std::string::npos)
								{ // try to find default one "english"
									std::string locale_label = public_label.substr(public_label.find("[en]") + 4);
									// try to find second end token
									if (locale_label.find("[en]") != std::string::npos)
									{
										public_label = locale_label.substr(0, locale_label.find("[en]"));
									}
								}
								keyToPublicLabel_map.insert(std::pair<std::string, std::string>(label, public_label));
							}
						}
						keyToUsefullParams_map.insert(std::pair<std::string, string_vector>(label, v));
					}
				}
				loaded = true;
			}
		}
		catch (const std::runtime_error &e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	/**
	  * \brief Récupération des paramètres de compression
    *
	  * Si cette fonction est appelée alors que \p loaded est à faux (la fonction CallMaps::initMaps n'a pas été appelée ou le parsing a échoué), le mode de compression par défaut est utilisé
		* sans prise en compte des paramètres (compression maximale).
	  *
    * \param key : une clé pouvant être utilisée dans un Call.
	  * \param param : un nom de paramètre associé à la clé \p key.
	  *
	  * \return vrai si la valeur du paramètre \p param de la clé \p key doit être pris en considération lors de la compression et faux sinon.
	  */
	bool contains(std::string key, std::string param) const
	{
		if (!loaded)
			return false;
		// retourne vrai si on trouve une clé dans la map correspondante à "key" et dont sa valeur contient "param"
		return keyToUsefullParams_map.find(key) != keyToUsefullParams_map.end() && std::find(keyToUsefullParams_map.at(key).begin(), keyToUsefullParams_map.at(key).end(), param) != keyToUsefullParams_map.at(key).end();
	}

	/**
	  * \brief Récupération du type d'appel associé à une clé
    *
	  * Si cette fonction est appelée alors que \p loaded est à faux (la fonction CallMaps::initMaps n'a pas été appelée ou le parsing a échoué), le type d'appel par défaut est utilisé (CALL_WITH_NO_PARAMS).
	  *
    * \param key : une clé pouvant être utilisée dans un Call.
	  *
	  * \return le type d'appel associé à la clé \p key.
	  */
	std::string getCallType(std::string key) const
	{
		if (!loaded || keyToCallType_map.find(key) == keyToCallType_map.end())
			return CALL_WITH_NO_PARAMS;
		else
			return keyToCallType_map.at(key);
	}

	/**
	  * \brief Récupération du label "public" (dépendant du langage de programmation utilisé) associé à une clé
    *
	  * Si cette fonction est appelée alors que \p loaded est à faux (la fonction CallMaps::initMaps n'a pas été appelée ou le parsing a échoué), le label par défaut est utilisé ("Unknown").
	  *
    * \param key : une clé pouvant être utilisée dans un Call.
	  *
	  * \return le label (dépendant du langage de programmation utilisé) associé à la clé \p key.
	  */
	std::string getLabel(std::string key) const
	{
		if (!loaded || keyToPublicLabel_map.find(key) == keyToPublicLabel_map.end())
			return "Unknown";
		else
			return keyToPublicLabel_map.at(key);
	}

private:
	typedef std::vector<std::string> string_vector;

	/**
	  * Une entrée key:param signifie que le paramètre param de la fonction key doit être pris en considération pour la compression, i.e. que les valeurs de ce paramètre pour deux appels doivent être égaux pour que ces deux appels puissent être considérés comme égaux.
	  */
	std::map<std::string, string_vector> keyToUsefullParams_map;

	/**
	  * Donne l'association entre une clé et un type d'appel (voir fichiers "params.json").
	  */
	std::map<std::string, std::string> keyToCallType_map;

	/**
	  * Donne l'association entre une clé et un label dépendant du langage de programmation utilisé (voir fichiers "params.json").
	  */
	std::map<std::string, std::string> keyToPublicLabel_map;

	/**
	  * Un booléen mis à vrai lorsque la fonction CallMaps::initMaps a été appelée et que les paramètres de compression ont bien été chargés.
	  */
	bool loaded;
};

/**
 * \class Call
 * \brief Classe abstraite héritant de Trace. Cette classe sert de classe mère pour toutes les classes définies dans le fichier CallDef.h.
 */
class Call : public Trace
{

public:
	/**
	  * Définition du type pointeur intelligent vers un objet Call.
	  */
	typedef boost::shared_ptr<Call> sp_call;

	typedef std::vector<sp_call> call_vector;

	/**
	  * \brief Enumération utilisée pour connaître le type d'erreur associé à l'objet Call.
	  */
	enum ErrorType
	{
		NONE = -1,
		UNIT_NOT_FOUND,
		NOT_UNIT_OWNER,
		INVALID_COALITION,
		TARGET_NOT_FOUND,
		OUT_OF_RANGE,
		INVALID_GROUP,
		POSITION_OUT_OF_BOUNDS
	};

	/**
	  * \brief Constructeur principal de la classe Call.
	  *
	  * \param key la clé de la fonction modélisée par l'objet Call.
	  * \param error l'erreur survenue lors de l'appel de la fonction représentée par l'objet Call.
	  * \param info le label attribué par l'expert.
	  */
	Call(std::string key, ErrorType error, std::string info = "");

	/**
	  * Constructeur de la classe Call utilisé notamment lors de la copie de l'objet.
	  */
	Call(const Call *c);

	/**
	  * Objet permettant de récupérer la chaîne de caractères associée au code du type d'unité. Ces codes sont propres à chaque jeu, cette map est donc normalement initialisée par le moteur en fonction du mod chargé.
	  */
	static std::map<int, std::string> units_id_map;

	/**
	  * Objet permettant de récupérer la chaîne de caractères associée au code du type d'ordre. Ces codes sont propres à chaque jeu, cette map est donc normalement initialisée par le moteur en fonction du mod chargé.
	  */
	static std::map<int, std::string> orders_map;

	/**
	  * Objet permettant de récupérer la chaîne de caractères associée au code du type de ressource. Ces codes sont propres à chaque jeu, cette map est donc normalement initialisée par le moteur en fonction du mod chargé.
	  */
	static std::map<int, std::string> resources_map;

	/**
	  * Tableau contenant les chaînes de caractères associées aux différentes valeurs définies dans l'enumération Call::ErrorType.
	  */
	static const char *errorsArr[];

	/**
	  * Tableau contenant les chaînes de caractères associées aux différentes valeurs définies dans l'enumération CallMisc::Coalition.
	  */
	static const char *coalitionsArr[];

	/**
	  * Variable utilisée pour le chargement et le stockage des paramètres de compression.
	  *
	  * \see CallMaps
	  */
	static CallMaps callMaps;

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
	template <typename E>
	static E getEnumType(const char *ch, const char **arr)
	{
		return static_cast<E>(Trace::inArray(ch, arr));
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
	template <typename E>
	static const char *getEnumLabel(E e, const char **arr)
	{
		int ind = static_cast<int>(e);
		if (ind > -1)
			return arr[ind];
		return NULL;
	}

	/**
	  * \brief Récupération de la longueur (l'espace occupé dans un vecteur de traces) d'un appel.
	  *
	  * \return 1
	  */
	virtual unsigned int length(int start = 0, bool processOptions = true) const;

	/**
	  * \brief Comparaison de l'objet Call avec une trace \p t.
	  *
	  * \param t : la trace utilisée pour la comparaison.
	  *
	  * \return vrai si la trace \p t est également un appel et qu'elle a les mêmes paramètres (clé, type d'erreur, valeur de retour, et paramètres pris en considération lors de la compression) que cet appel.
	  */
	virtual bool operator==(Trace *t) const;

	/**
	  * \brief Détection des paramètres non robustes.
	  *
	  * Cette fonction est utilisée pour affecter aux paramètres non robustes de l'appel, i.e. ceux non pris en compte lors de la compression et dont la valeur diffère entre deux itération d'une même séquence, la valeur -1.
	  * Attention : cette fonction doit être appelée uniquement si operator==(c) renvoie vrai.
	  *
	  * \param c : un pointeur vers l'appel utilisé pour la comparaison.
	  */
	virtual void filterCall(const Call *c);

	/**
	  * \brief Export des informations de l'objet Call sous la forme d'une chaine de caractère.
	  *
	  * \param os le flux de sortie utilisé pour la sortie.
	  */
	virtual void exportAsString(std::ostream &os = std::cout) const;

	/**
	  * \brief Export de la trace sous la forme d'une chaine de caractère compressée (sur une seule ligne).
	  *
	  * \param os : le flux de sortie utilisé pour l'export.
	  */
	virtual void exportAsCompressedString(std::ostream &os = std::cout) const;

	/**
	  * \brief Clonage d'un appel.
	  *
	  * \return une copie de l'objet Call.
	  */
	virtual Trace::sp_trace clone() const = 0;

	/**
	  * \brief Récupération des paramètres de l'appel sous forme de chaîne de caractères.
	  *
	  * Cette fonction est notamment utilisée lors de l'export des traces vers un document XML.
	  *
	  * \return une chaîne de caractères formatée contenant les valeurs des différents paramètres de l'appel séparées par des espaces.
	  */
	virtual std::string getParams() const = 0;

	/**
	  * \brief Récupération des paramètres de l'appel sous forme de chaîne de caractères.
	  *
	  * Cette fonction est notamment utilisée lors de la construction du feedback qui sera fait au joueur.
	  *
	  * \return une chaîne de caratères formatée contenant les valeurs des différents paramètres de l'appel.
	  */
	virtual std::string getReadableParams() const = 0;

	/**
	  * \brief Récupération des identifiants des paramètres non conformes.
	  *
	  * Si \p c pointe vers un objet Call, les identifiants des paramètres dont les valeurs diffèrent de ceux de \p c sont ajoutés à la liste. Sinon, si \p c est à NULL et si la valeur de \p error est différente de Call::ErrorType::None, alors l'identifiant du paramètre ayant causé cette erreur est ajouté à la liste.
	  * Cette fonction est utilisée lors de la construction des messages qui seront affichés au joueur.
	  *
	  * \param c : un pointeur vers l'objet Call utilisé pour la comparaison.
	  *
	  * \return la liste d'identifiants des paramètres non conformes.
	  */
	virtual std::vector<std::string> getListIdWrongParams(Call *c = NULL) const;

	/**
	  * \brief Calcul de la distance entre deux appels.
	  *
	  * \param c : un pointeur vers l'objet Call dont l'on souhaite mesurer la distance avec cet appel.
	  *
	  * \return la distance dans l'intervalle [0,1] entre les deux appels. Si la distance est égale à 0, les deux appels sont complètement identiques. Si elle est égale à 1, les deux appels ont des clés et/ou des types d'erreur différents.
	  */
	double getEditDistance(const Call *c) const;

	/**
	  * \brief Getter pour la variable \p key.
	  *
	  * \return la chaîne de caractères \p key de l'appel.
	  */
	std::string getKey() const;

	/**
	  * \brief Getter pour la variable \p error.
	  *
	  * \return le type d'erreur associé à l'appel.
	  */
	ErrorType getError() const;

	/**
	  * \brief Ajout d'une valeur dans le tableau \p ret contenant les codes de retour de l'appel.
	  *
	  * \param code : le code retour de l'appel à sauvegarder dans le tableau \p ret.
	  *
	  * \return vrai si la valeur a bien été sauvegardée dans \p ret, et faux sinon (plus de place disponible).
	  */
	bool addReturnCode(float code);

	/**
	  * \brief Récupération des codes de retour sous forme de chaîne de caractères.
	  *
	  * \return une chaîne de caractères formatée contenant les valeurs du retour de l'appel séparées par des espaces.
	  */
	std::string getReturn() const;

	/**
	  * \brief Comparaison des codes de retour entre deux appels.
	  *
	  * \param c : un pointeur vers l'appel utilisé pour la comparaison.
	  *
	  * \return vrai si le tableau \p ret de l'objet pointé par \p c est identique à celui de cet appel avec une précision de FLOAT_EPSILON.
	  */
	bool compareReturn(const Call *c) const;

	/**
	  * \brief Mise à -1 de \p ind_ret.
	  *
	  * Cette fonction doit être appelée pour enregistrer le retour de l'appel comme étant un paramètre non robuste.
	  */
	void setReturn();

protected:
	/**
		* \brief Indique si l'appel a un retour.
		*
		* \return vrai si l'appel a un retour (robuste ou non), et faux sinon.
		*/
	bool hasReturn() const;

	/**
	  * \brief Comparaison entre deux appels.
	  *
	  * \param c : un pointeur vers l'objet Call utilisé pour la comparaison.
	  *
	  * \return vrai si les deux appels sont égaux, et faux sinon.
	  *
	  * \see Call::operator==
	  */
	virtual bool compare(const Call *c) const = 0;

	/**
	  * \brief Détection des paramètres non robustes.
	  *
	  * \param c : un pointeur vers l'objet Call utilisé pour la comparaison.
	  *
	  * \see Call::filterCall
	  */
	virtual void filter(const Call *c) = 0;

	/**
	  * \brief Détermination de la distance entre deux appels.
	  *
	  * \param c : un pointeur vers l'objet Call utilisé pour la comparaison.
	  *
	  * \return un couple d'entiers où le premier élément correspond au nombre de paramètres qui diffèrent entre les deux appels et où le second élément est le nombre de comparaisons effectués.
	  *
	  * \see Call::getEditDistance
	  */
	virtual std::pair<int, int> distance(const Call *c) const = 0;

	/**
	  * \see Call::getListIdWrongParams
	  */
	virtual std::vector<std::string> id_wrong_params(Call *c) const = 0;

	/**
	  * La clé associée permettant d'identifier la fonction modélisée par l'objet Call.
	  */
	std::string key;

	/**
	  * Variable permettant de connaître le type d'erreur retourné lors de l'appel à la fonction Call::key, si une erreur a été retournée. Si aucune erreur n'est présente, la variable prend la valeur Call::ErrorType::NONE.
	  */
	ErrorType error;

	/**
	  * Indice utilisée pour remplir le tableau Call::ret.
	  */
	int ind_ret;

	/**
	  * Tableau contenant les valeurs de retour de l'appel à la fonction Call::key.
	  */
	float ret[MAX_SIZE_PARAMS];
};

#endif
