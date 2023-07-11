/**
 * \file TracesParser.h
 * \brief Déclaration de la classe TracesParser.
 * \author Meresse/Muratet
 * \version 0.1
 */

#ifndef __TRACES_PARSER_H__
#define __TRACES_PARSER_H__

#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <conio.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include <rapidxml/rapidxml_utils.hpp>

#include "Trace.h"
#include "Call.h"
#include "CallDef.h"
#include "Sequence.h"
#include "Scenario.h"

/**
 * \class TracesParser
 *
 * \brief La classe TracesParser définit les méthodes de parsing de fichiers de traces brutes (que ce soit dans le contexte d'une partie de jeu ou en ligne de commandes), les différentes fonctions de l'algorithme hors-ligne, et les fonctions d'export et d'import de traces à partir d'un document XML.
 */
class TracesParser
{

public:
	/**
	  * \brief Constructeur de TracesParser.
	  */
	TracesParser();

	/**
	  * \brief Destructeur de TracesParser.
	  *
	  * Tous les fichiers ouverts sont fermés. L'utilisation de std::shared_ptr évite d'avoir à appeler delete sur les objets alloués dynamiquement.
	  */
	~TracesParser();

	/**
	 * Outputstream for debug
	 **/
	static std::ostream &osParser;

	/**
	 * If false don't write log in std::cout
	 **/
	static bool outputLog;

	/**
	  * \brief Lancement de la compression de traces brutes avec l'algorithme de compression hors-ligne.
	  *
	  * \param logs contenu des traces brutes
	  */
	void parseLogs(std::istream &logs);

	/*
	 * Contient le résultat de la dernière compression en texte brut
	 */
	std::string lastCompression;

	/**
	  * Entier correspondant à temp mamimale autorisé pour la processus de offline compress, si le temp d'éxécution dépasse ce temps l'algorithme va arrêter et afficher "Overtime" à sortir.
	  */
	static int TIME_LIMIT;

	/**
	  * Entier correspondant à nombre maximale de candidates pendant la processus de offline compress, l'algorithme va explorer plus de cas possibles si cet entier augmente .
	  */
	static int CANDIDATE_LIMIT;

	/**
	 * ENtier correspondant à nombre de fois autorisé maximale pour un scénario qui baisse son score successivement
	 */ 
	static int DESCEND_LIMIT; 


	/*
	 * Contient le résultat de la dernière compression sous une forme XML
	 */
	rapidxml::xml_document<> lastCompressionXML;

	/**
		* \brief Set human langage
		*/
	static void setLang(std::string lang);

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
	static std::vector<Trace::sp_trace> importTraceFromXml(const std::string &xml, std::ostream &os);

	/**
	  * \brief Importation de traces à partir d'un noeud de document XML.
	  *
	  * Cette fonction permet d'instancier et d'insérer de nouveaux objets Trace dans le vecteur \p traces en parsant un document XML à partir d'un de ses noeuds \p node. La structure établie par le document XML est ainsi respectée.
	  *
	  * \param node le noeud à partir duquel le document XML est parsé.
	  * \param traces le vecteur de traces dans lequel les nouvelles traces créées sont ajoutées.
	  */
	static void importTraceFromNode(rapidxml::xml_node<> *node, std::vector<Trace::sp_trace> &traces);

	/**
	  * \brief Détermination du nombre de noeuds fils d'un noeud dans un document XML.
	  *
	  * \param node le noeud dont l'on souhaite connaître le nombre de noeuds fils.
	  *
	  * \return le nombre de noeuds fils de \p node.
	  */
	static unsigned int getNodeChildCount(rapidxml::xml_node<> *node);

	/**
	  * \brief Analyse la chaine de caractère passée en paramètre et peut : (1) initialiser TracesParser::mission_name si le token GAME_START est detecté, (2) initialiser TracesParser::mission_end_time si le token MISSION_END_TIME est detecté, (3) initialiser TracesParser::execution_start_time si le token EXECUTION_START_TIME est detecté, (4) construire un objet Trace et le retourner (les objets Trace construits peuvent être des objets de type Call, StartMissionEvent, NewExecutionEvent, EndExecutionEvent, EndMissionEvent ou Event). A noter que pour les cas 1 à 3 NULL est retourné.
	  *
	  * \param s une chaîne de caractères formatée selon le modèle de traces défini.
	  *
	  * \return un pointeur intelligent de type Trace::sp_trace pointant vers l'objet Trace créé, ou NULL si aucun objet n'a pu être créé à partir de la chaine de caractère (dans ce cas d'autres variable statiques ont pu être initialisées).
	  */
	static Trace::sp_trace parseLine(const std::string &s);

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
	static std::vector<std::string> splitLine(const std::string &s, char delim = ' ');

	/**
	  * Le nom de la mission correspondante aux traces.
	  */
	static std::string mission_name;

	/**
	  * Paramètres de compressions
		*/
	static std::string params_json;
	

	/**
		* \brief Calcul du meilleur alignement possible entre deux vecteurs de traces.
		*
		* Cette fonction est la fonction permettant de déterminer le meilleur alignement possible entre les traces de \p l et les traces de \p e. Le traitement est effectué de façon récursive, i.e. que le meilleur alignement possible pour le contenu des séquences et de leurs sous-séquences est également cherché. Enfin, cette fonction permet également de calculer le score du joueur.
		*
		* \param l un vecteur de traces correspondant à l'apprenant
		* \param e un vecteur de traces correspondant à l'expert
		* \param align booléen permettant d'indiquer si le meilleur alignement trouvé doit être conservé, i.e. si les champs Trace::aligned doivent être modifiés. Sa valeur par défaut est vrai.
		*
		* \return un couple (gs,nv) où gs est le score de similarité brut (non normalisé) obtenu à partir du meilleur alignement trouvé et nv est la valeur de normalisation qui doit être utilisée pour ramener ce score dans l'intervalle [0,1].
		*/
	static std::pair<double, double> findBestAlignment(const std::vector<Trace::sp_trace> &l, const std::vector<Trace::sp_trace> &e, bool align = true);

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
		* Language utilisé
		*/
	static std::string lang;

	/**
	  * Une séquence contenant les objets Trace créés lors du parsage du fichier de traces brutes. Lors de l'étape de compression, cette séquence est modifiée. A la fin de la compression, elle contient l'ensemble des traces compressées.
	  */
	Sequence::sp_sequence root;

	/**
	  * \brief Cette fonction réinitialise les données et ressources qui doivent l'être.
	  */
	void CloseResources();

	/**
	  * \brief Sauvegarde les résultats de la compression.
	  *
	  * Cette fonction est utilisée pour sauvegarder les résultats de la compression. Les résultats seront sauvegardés sous forme de texte brut (voir TracesParser::lastCompression) et dans sous une forme XML  (voir TracesParser::lastCompressionXML) qui pourra être utilisé pour l'analyse de ces traces.
	  */
	void saveCompression();

	/**
	  * \brief Fonction de pré-compression
	  *
	  * Cette fonction est utilisée pour compacter directement (i.e. avant tout appel à TracesParser::offlineCompression) les appels qui se répètent dans la trace de façon contiguë. Elle permet ainsi de gagner un temps considérable dans certains cas comparé au temps de traitement qui serait requis par la fonction TracesParser::offlineCompression.
	  *
	  * \param spt une nouvelle trace à ajouter dans le vecteur TracesParser::traces.
	  */
	void inlineCompression(Trace::sp_trace &spt);

	/**
	  * \brief Fonction de compression
	  *
	  * Cette fonction est utilisée pour compacter la trace et appliqué plusieurs étapes de compression et de corrections
	  *
	  */
	void offlineCompression();

	/**
	 * \brief Intègre une trace dans un ensemble de patterns. En sortie de cette fonction la liste des patterns peut être mise à jour
	 * 
	 * \param patterns liste des patterns dans lesquels insérer la trace. Suite à cet ajout de nouveaux scénarios peuvent être créés voir supprimés
	 * \param currentTrace trace à insérer à la liste des patterns
	*/
	void insertTraceInsidePatterns(std::vector<Scenario::sp_scenario> & patterns, const Trace::sp_trace & currentTrace);

	/**
	  * \brief Export des traces contenues dans le vecteur TracesParser::traces sous la forme d'une chaîne de caractère.
	  *
	  * \param os le flux de sortie à utiliser pour la sortie.
	  */
	void exportTracesAsString(std::ostream &os = std::cout);

	/**
	  * \brief Conversion d'une chaîne de caractères en entier.
	  *
	  * Si la conversion échoue, une erreur est générée et le programme se termine.
	  * Si \p s = "?", la valeur -1 est renvoyée.
	  *
	  * \return la valeur résultante de la conversion.
	  */
	static int stoi(const std::string &s);

	/**
	  * \brief Conversion d'une chaîne de caractères en flottant.
	  *
	  * Si la conversion échoue, une erreur est générée et le programme se termine.
	  * Si \p s = "?", la valeur -1.0 est renvoyée.
	  *
	  * \return la valeur résultante de la conversion.
	  */
	static float stof(const std::string &s);
};

#endif
