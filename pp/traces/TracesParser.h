/**
 * \file TracesParser.h
 * \brief Déclaration de la classe TracesParser.
 * \author meresse
 * \version 0.1
 */

#ifndef __TRACES_PARSER_H__
#define __TRACES_PARSER_H__

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif
#include <boost/lexical_cast.hpp>
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include <rapidxml/rapidxml_utils.hpp>

#include "Trace.h"
#include "Call.h"
#include "CallDef.h"
#include "Event.h"
#include "EventDef.h"
#include "Sequence.h"

/**
  * Doit être mis à 1 pour prendre en compte les événements de Event::concatEventsArr rencontrés lors du parsage de fichier de traces brutes.
  */
#define INCLUDE_EVENTS 0


/**
  * Seuil utilisé pour stopper et éviter toute future recherche de répétitions d'un groupe de traces à partir d'une trace.
  *
  * \see TracesParser::detectSequences
  */
#define MAX_END_SEARCH 10

/**
 * \class TracesParser
 *
 * \brief La classe TracesParser définit les méthodes de parsing de fichiers de traces brutes (que ce soit dans le contexte d'une partie de jeu ou en ligne de commandes), les différentes fonctions de l'algorithme hors-ligne, et les fonctions d'export et d'import de traces à partir d'un document XML.
 */
class TracesParser {

public:

	/**
	  * \brief Constructeur de TracesParser.
	  *
	  * \param in_game indique si la compression doit se faire en cours de jeu.
	  */
	TracesParser(bool in_game);

	/**
	  * \brief Destructeur de TracesParser.
	  *
	  * Tous les fichiers ouverts sont fermés. L'utilisation de boost::shared_ptr évite d'avoir à appeler delete sur les objets alloués dynamiquement.
	  */
	~TracesParser();

	/**
	  * Variable utilisée pour stocker le numero de la ligne courante lorsqu'un fichier de traces brutes est parsé.
	  */
	static int lineNum;

	/**
	  * \brief Lancement de la compression d'un fichier de traces brutes avec l'algorithme de compression hors-ligne.
	  *
	  * \param dir_path le chemin d'accès au fichier.
	  * \param filename le nom du fichier.
	  */
	void parseTraceFileOffline(const std::string& dir_path, const std::string& filename);

//  A SUPPRIMER ???
// 	/**
// 	  * \brief Lancement du parsage d'un fichier de traces brutes.
// 	  *
// 	  * Cette fonction permet de parser un fichier de traces brutes. Les traces sont simplement ajoutées dans le vecteur TracesParser::traces sans aucune compression.
// 	  *
// 	  * \param dir_path le chemin d'accès au fichier.
// 	  * \param filename le nom du fichier.
// 	  */
// 	void parseTraceFile(const std::string& dir_path, const std::string& filename);

	/**
	  * \brief Affichage des traces contenues dans le vecteur TracesParser::traces.
	  *
	  * \param os le flux de sortie à utiliser pour l'affichage.
	  */
	void display(std::ostream &os = std::cout);

	/**
	  * \brief Setter pour la variable TracesParser::end.
	  *
	  * La variable TracesParser::end est mise à vraie.
	  *
	  * \see TracesParser::end
	  */
	void setEnd();

	/**
	  * \brief Getter pour la variable TracesParser::end.
	  *
	  * \return la valeur du booléen TracesParser::end.
	  *
	  * \see TracesParser::end
	  */
	bool getEnd() const;

	/**
	  * \brief Getter pour la variable TracesParser::compressed.
	  *
	  * Si le booléen TracesParser::compressed est à vrai lorsque la fonction est appelée, il est mis à faux.
	  *
	  * Cette fonction est appelée par la classe CProgAndPlay définie au niveau du moteur de jeu.
	  *
	  * \return la valeur du booléen TracesParser::compressed.
	  *
	  * \see TracesParser::compressed
	  */
	bool compressionDone();

	/**
	  * \brief Setter pour la variable TracesParser::proceed.
	  *
	  * Cette fonction est appelée par la classe CProgAndPlay définie au niveau du moteur de jeu.
	  *
	  * \see TracesParser::proceed
	  */
	void setProceed(bool proceed);

	/**
	  * \brief Getter pour la variable TracesParser::proceed.
	  *
	  * \return la valeur du booléen TracesParser::proceed.
	  *
	  * \see TracesParser::proceed
	  */
	bool getProceed();

	/**
		* \brief Set human langage
		*/
	static void setLang(std::string lang);

	/**
	  * \brief Instantiation d'un objet Trace à partir d'une chaîne de caractères.
	  *
	  * Cette fonction est utilisée pour l'instanciation d'objets de type Call et Event.
	  *
	  * \param s une chaîne de caractères formatée selon le modèle de traces défini.
	  *
	  * \return un pointeur intelligent de type Trace::sp_trace pointant vers l'objet Trace créé, ou vers NULL si aucun objet n'a été créé lors de l'appel.
	  */
	static Trace::sp_trace handleLine(const std::string& s);

	/**
	  * \brief Fusion de deux séquences
	  *
	  * Cette fonction permet de construire une nouvelle séquence la plus générale possible à partir de deux séquences \p sps_up et \p sps_down qui doivent être égales pour que la fusion ait lieue.
	  *
	  * \param sps_up la première séquence passée en entrée de la fusion.
	  * \param sps_down la seconde séquence passée en entrée de la fusion.
	  *
	  * \return la nouvelle séquence créée résultante de la fusion de \p sps_up et \p sps_down.
	  *
	  * \see Sequence::compare
	  * \see TracesParser::detectSequences
	  */
	static Sequence::sp_sequence mergeSequences(Sequence::sp_sequence sps_up, Sequence::sp_sequence sps_down);

	/**
	  * \brief Importation de traces à partir d'un document XML.
	  *
	  * Cette fonction permet de construire un vecteur de traces à partir d'une chaîne de caractères au format XML. Le document XML correspondant constitue une sauvegarde des informations et des traces relatifs à un ou plusieurs lancements de mission.
	  *
	  * \param xml la chaîne de caractères correspondant au contenu du document XML.
	  * \param xml le flux sur dans lequel afficher les logs.
	  *
	  * \return le vecteur de traces construit à la fin de l'import.
	  */
	static std::vector<Trace::sp_trace> importTraceFromXml(const std::string& xml, std::ostream& os);

	/**
	  * \brief Importation de traces à partir d'un noeud de document XML.
	  *
	  * Cette fonction permet d'instancier et d'insérer de nouveaux objets Trace dans le vecteur \p traces en parsant un document XML à partir d'un de ses noeuds \p node. La structure établie par le document XML est ainsi respectée.
	  *
	  * \param node le noeud à partir duquel le document XML est parsé.
	  * \param traces le vecteur de traces dans lequel les nouvelles traces créées sont ajoutées.
	  */
	static void importTraceFromNode(rapidxml::xml_node<> *node, std::vector<Trace::sp_trace>& traces);

	/**
	  * \brief Détermination du nombre de noeuds fils d'un noeud dans un document XML.
	  *
	  * \param node le noeud dont l'on souhaite connaître le nombre de noeuds fils.
	  *
	  * \return le nombre de noeuds fils de \p node.
	  */
	static unsigned int getNodeChildCount(rapidxml::xml_node<> *node);

	/**
	  * \brief Fonction permettant de découper une chaîne de caractères en se basant sur un délimiteur.
	  *
	  * Cette fonction prend en entrée une chaîne de caractère \p s et construit un vecteur de chaînes de caractères contenant les différentes parties de \p s qui sont séparées par le délimiteur \p delim dans la chaîne. Si la chaîne de caractères ne contient aucune occurrence du délimiteur, alors le vecteur renvoyé ne contiendra qu'un seul élément : la chaîne elle-même.
	  *
	  * \param s la chaîne de caractères à découper.
	  * \param delim le délimiteur utilisé pour le découpage de \p s.
	  *
	  * \return le vecteur construit lors du découpage de la chaîne.
	  */
	static std::vector<std::string> splitLine(const std::string& s, char delim = ' ');

	/**
	  * \brief Conversion d'une chaîne de caractères en entier.
	  *
	  * Si la conversion échoue, une erreur est générée et le programme se termine.
	  * Si \p s = "?", la valeur -1 est renvoyée.
	  *
	  * \return la valeur résultante de la conversion.
	  */
	static int stoi(const std::string& s);

	/**
	  * \brief Conversion d'une chaîne de caractères en flottant.
	  *
	  * Si la conversion échoue, une erreur est générée et le programme se termine.
	  * Si \p s = "?", la valeur -1.0 est renvoyée.
	  *
	  * \return la valeur résultante de la conversion.
	  */
	static float stof(const std::string& s);

		/**
		  * Le nom de la mission correspondante aux traces.
		  */
		static std::string mission_name;

		/**
		  * Paramètres de compressions
			*/
		static std::string params_json;

private:

	/**
	  * La valeur du timestamp de fin de mission.
	  */
	static int mission_end_time;

	/**
	  * La valeur du timestamp de début d'exécution d'une tentative de résolution.
	  */
	static int execution_start_time;

	/**
	  * L'événement de fin de mission. Cet objet doit être conservé car l'événement de fin de mission n'est pas forcément la dernière trace dans un fichier de traces brutes.
	  */
	static Trace::sp_trace spe_eme;

	/**
		* Language utilisé
		*/
	static std::string lang;

	/**
	  * Booléen utilisé pour indiquer si l'objet TracesParser est utilisé dans le moteur de jeu ou dans le programme utilisable en ligne de commandes.
	  */
	bool in_game;

	/**
	  * Booléen utilisé comme pour bloquer l'utilisation de l'objet TracesParser une fois une tâche de parsage de fichier de traces brutes lancée.
	  */
	bool used;

	/**
	  * Booléen utilisé pour informer le moteur de jeu que la compression des traces est terminée. Le moteur de jeu peut alors lire le résultat de la compression dans le document XML créé.
	  */
	bool compressed;

	/**
	  * Booléen mis à vrai par le moteur de jeu pour mettre fin à l'attente active effectuée par la fonction TracesParser::readTracesOfflineInGame. Une boucle présent dans cette fonction va tester l'ajout d'une nouvelle entrée ajoutée à la fin du fichier de traces brutes (afin de la traiter) tant que ce booléen est à faux.
	  */
	bool end;

	/**
	  * Booléen dont la valeur est mise à jour par le moteur de jeu. Lorsque les conditions sont réunies, sa valeur est mise à vraie. Les traces sont alors compressées (si elles ne l'était pas déjà) et le résultat de cette compression est sauvegardé dans un document XML qui servira pour leur analyse. Une fois ces traitement effectués, sa valeur est mise à faux.
	  */
	bool proceed;

	/**
	  * Entier correspondant à un indice de position dans le vecteur TracesParser::traces utilisé pour séparer les traces déjà compressées (celles dont l'indice est inférieur à TracesParser::start) de celles qui sont encore à traitées (celles dont l'indice est supérieur ou égal à TracesParser::start). On évite ainsi de reparcourir les traces déja traitées lors de l'étape de compression.
	  */
	int start;

	/**
	  * Objet utilisé pour la lecture du contenu du fichier de traces brutes à parser.
	  */
	std::ifstream ifs;

	/**
	  * Le chemin d'accès au fichier de traces brutes à parser.
	  */
	std::string dir_path;

	/**
	  * Le nom du fichier de traces brutes à parser.
	  */
	std::string filename;

	/**
	  * Un vecteur contenant les objets Trace créés lors du parsage du fichier de traces brutes. Lors de l'étape de compression, ce vecteur est modifié. A la fin de la compression, il contient l'ensemble des traces compressées.
	  */
	std::vector<Trace::sp_trace> traces;

	/**
	  * \brief Initialisation du parsage d'un fichier de traces brutes.
	  *
	  * Pour lancer le parsage d'un fichier de traces brutes, il faut que le booléen TracesParser::used soit à faux et que le fichier d'extension .log contenant les traces brutes ait été correctement ouvert.
	  *
	  * \param dir_path le chemin d'accès au fichier de traces brutes à parser.
	  * \param filename le nom du fichier de traces brutes à parser. Ce nom doit contenir l'extension ".log".
	  *
	  * \return vrai si le parsage du fichier de traces brutes peut débuter, et faux sinon.
	  */
	bool beginParse(const std::string& dir_path, const std::string& filename);

	/**
	  * \brief Fin du parsage d'un fichier de traces brutes.
	  *
	  * Cette fonction réinitialise les valeurs des variables qui doivent l'être.
	  */
	void endParse();

	/**
	  * \brief Export des résultats de la compression.
	  *
	  * Cette fonction est utilisée pour sauvegarder les résultats de la compression. Les résultats seront sauvegardés dans un fichier d'extension .txt et dans un document XML qui pourra être utilisé pour l'analyse de ces traces. Enfin, le booléen TracesParser::compressed est mis à vrai pour informer le moteur de jeu de la fin de la compression et de la disponibilité des résultats.
	  */
	void writeFiles();

	/**
	  * \brief Parsage des traces brutes en ligne de commandes.
	  *
	  * Cette fonction sert à parser les traces brutes dans le contexte d'une utilisation du programme en ligne de commandes.
	  */
	void readTracesOffline();

	/**
	  * \brief Parsage des traces brutes en cours de partie.
	  *
	  * Cette fonction est utilisée pour parser les traces brutes dans le contexte d'une utilisation de l'objet TracesParser au sein du moteur de jeu (cf. la classe CProgAndPlay).
	  */
	void readTracesOfflineInGame();

	/**
	  * \brief Exportation de traces vers un document XML.
	  *
	  * Cette fonction permet d'exporter les traces contenues dans le vecteur TracesParser::traces, i.e. l'ensemble des informations et des traces relatifs à un ou plusieurs lancements de mission, vers un nouveau document XML. Le fichier XML créé est placé dans le même répertoire que celui du fichier de traces brutes. Il est nommé 'filename_compressed.xml' où filename est égal à TracesParser::filename (sans l'extension ".log").
	  */
	void exportTraceToXml();

	/**
	  * \brief Parcours du fichier de traces brutes pour atteindre la dernière occurrence de 'start mission_name' présente dans le fichier.
	  *
	  * Cette fonction est utilisée lorsque la compression est lancée en cours de partie, i.e. si TracesParser::in_game est à vrai. Elle permet de parcourir le fichier de traces brutes à l'aide de l'objet TracesParser::ifs et de définir ainsi la position du curseur de lecture de sorte à ce que la prochaine ligne qui sera extraite avec la fonction getline() sera la dernière occurrence de 'start mission_name' présente dans le fichier. Le fichier doit donc contenir au moins une occurrence de ce type.
	  *
	  * \return vrai si la dernière occurrence a été trouvée et si le curseur a bien été positionné, et faux sinon.
	  */
	bool reachLastStart();

	/**
	  * \brief Fonction de pré-compression
	  *
	  * Cette fonction est utilisée pour compacter directement (i.e. avant tout appel à TracesParser::detectSequences) les appels qui se répètent dans la trace de façon contiguë. Elle permet ainsi de gagner un temps considérable dans certains cas comparé au temps de traitement qui serait requis par la fonction TracesParser::detectSequences.
	  *
	  * \param spt une nouvelle trace à ajouter dans le vecteur TracesParser::traces.
	  */
	void handleTraceOffline(Trace::sp_trace& spt);

	/**
	  * \brief Test de la possibilité de répétitions d'un groupe de traces.
	  *
	  * Cette fonction est utilisée par la fonction TracesParser::detectSequences lors de la recherche de répétitions d'un groupe de traces dans TracesParser::traces. Le test consiste à vérifier si TracesParser::traces contient assez de traces à partir d'un certain indice. Cette fonction est une optimisation permettant d'éviter le lancement d'opérations de comparaison qui retourneront forcément des résultats négatifs.
	  */
	bool checkFeasibility(unsigned int min_length, unsigned int ind_start);

	/**
	  * \brief Fonction principale de l'algorithme de compression hors-ligne.
	  *
	  * Cette fonction est la fonction qui va compresser les traces en effectuant des parcours du vecteur TracesParser::traces. La compression étant effectuée en mode hors-ligne, elle doit être appelée uniquement lorsque TracesParser::traces est complet, i.e. qu'aucune nouvelle trace ne doit être ajoutée en cours de traitement.
	  */
	void detectSequences();

};

#endif
