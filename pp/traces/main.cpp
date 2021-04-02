#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "TracesParser.h"
#include "TracesAnalyser.h"
#include "Scenario.h"
#include "../test/constantList_KP4.1.h"

static std::string dir_path = "./example";
static std::string output = "";
static TracesParser tp;

void saveCompressedTraces(const std::string &filename)
{
	// Save compression results as txt file
	std::string s = "\\" + output + ".txt";
	if (!output.compare(""))
	{
		s = "\\" + filename;
		s.replace(s.find(".log"), 4, "_compressed.txt");
	}
	s.insert(0, dir_path);
	std::cout << "Write compressed traces into " << s << std::endl;
	std::ofstream ofs(s.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (ofs.good())
	{
		ofs << tp.lastCompression;
		ofs.close();
	}
	// Save compression results as xml file
	s = "\\" + output + ".xml";
	if (!output.compare(""))
	{
		s = "\\" + filename;
		s.replace(s.find(".log"), 4, "_compressed.xml");
	}
	s.insert(0, dir_path);
	std::cout << "Write compressed traces into " << s << std::endl;
	std::ofstream ofsXml(s.c_str(), std::ofstream::out | std::ofstream::trunc);
	if (ofsXml.good())
	{
		ofsXml << tp.lastCompressionXML;
		ofsXml.close();
	}
}

int compressAllTraces(std::string dir_path)
{
	DIR *pdir;
	struct dirent *pent;
	pdir = opendir(dir_path.c_str());
	if (!pdir)
	{
		perror("");
		return -1;
	}
	while ((pent = readdir(pdir)))
	{
		if (strchr(pent->d_name, '.') == NULL)
		{
			std::cout << "move to directory " << pent->d_name << std::endl;
			compressAllTraces(dir_path + "\\" + pent->d_name);
		}
		else if (strcmp(pent->d_name, ".") != 0 && strcmp(pent->d_name, "..") != 0)
		{
			std::string filename(pent->d_name);
			if (filename.find("_compressed") == std::string::npos && filename.find(".log") != std::string::npos && filename.compare("meta.log") != 0)
			{
				std::cout << "parse " << dir_path << "\\" << filename << std::endl;
				std::string s = dir_path + "\\" + filename;
				std::ifstream ifs(s.c_str(), std::ios::in | std::ios::binary);
				tp.parseLogs(ifs, false);
				saveCompressedTraces(filename);
			}
		}
	}
	closedir(pdir);
	return 0;
}

const std::string loadFile(std::string full_path)
{
	std::string res;
	std::ifstream in(full_path.c_str());
	if (in.good())
	{
		std::string line;
		while (std::getline(in, line))
			res += line;
	}
	return res;
}

void initExternalConstants()
{
	// Reset all maps
	Call::units_id_map.clear();
	Call::orders_map.clear();
	Call::resources_map.clear();

	//Initialise units_id_map
	Call::units_id_map.insert(std::make_pair<int, std::string>(ASSEMBLER, "ASSEMBLER"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(BADBLOCK, "BADBLOCK"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(BIT, "BIT"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(BYTE, "BYTE"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(KERNEL, "KERNEL"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(LOGIC_BOMB, "LOGIC BOMB"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(POINTER, "POINTER"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(SIGNAL, "SIGNAL"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(SOCKET, "SOCKET"));
	Call::units_id_map.insert(std::make_pair<int, std::string>(TERMINAL, "TERMINAL"));

	//Initialise orders_map
	Call::orders_map.insert(std::make_pair<int, std::string>(STOP, "STOP"));
	Call::orders_map.insert(std::make_pair<int, std::string>(WAIT, "WAIT"));
	Call::orders_map.insert(std::make_pair<int, std::string>(FIRE_STATE, "FIRE STATE"));
	Call::orders_map.insert(std::make_pair<int, std::string>(SELF_DESTRUCTION, "SELF DESTRUCTION"));
	Call::orders_map.insert(std::make_pair<int, std::string>(REPEAT, "REPEAT"));
	Call::orders_map.insert(std::make_pair<int, std::string>(MOVE, "MOVE"));
	Call::orders_map.insert(std::make_pair<int, std::string>(PATROL, "PATROL"));
	Call::orders_map.insert(std::make_pair<int, std::string>(FIGHT, "FIGHT"));
	Call::orders_map.insert(std::make_pair<int, std::string>(GUARD, "GUARD"));
	Call::orders_map.insert(std::make_pair<int, std::string>(MOVE_STATE, "MOVE_STATE"));
	Call::orders_map.insert(std::make_pair<int, std::string>(ATTACK, "ATTACK"));
	Call::orders_map.insert(std::make_pair<int, std::string>(REPAIR, "REPAIR"));
	Call::orders_map.insert(std::make_pair<int, std::string>(RECLAIM, "RECLAIM"));
	Call::orders_map.insert(std::make_pair<int, std::string>(RESTORE, "RESTORE"));
	Call::orders_map.insert(std::make_pair<int, std::string>(BUILD_BADBLOCK, "BUILD BADBLOCK"));
	Call::orders_map.insert(std::make_pair<int, std::string>(BUILD_LOGIC_BOMB, "BUILD LOGIC BOMB"));
	Call::orders_map.insert(std::make_pair<int, std::string>(BUILD_SOCKET, "BUILD SOCKET"));
	Call::orders_map.insert(std::make_pair<int, std::string>(BUILD_TERMINAL, "BUILD TERMINAL"));
	Call::orders_map.insert(std::make_pair<int, std::string>(DEBUG, "DEBUG"));
	Call::orders_map.insert(std::make_pair<int, std::string>(BUILD_ASSEMBLER, "BUILD ASSEMBLER"));
	Call::orders_map.insert(std::make_pair<int, std::string>(BUILD_BYTE, "BUILD BYTE"));
	Call::orders_map.insert(std::make_pair<int, std::string>(BUILD_POINTER, "BUILD POINTER"));
	Call::orders_map.insert(std::make_pair<int, std::string>(BUILD_BIT, "BUILD BIT"));
	Call::orders_map.insert(std::make_pair<int, std::string>(STOP_BUILDING, "STOP BUILD"));
	Call::orders_map.insert(std::make_pair<int, std::string>(LAUNCH_MINE, "LAUNCH MINE"));
	Call::orders_map.insert(std::make_pair<int, std::string>(NX_FLAG, "NX_FLAG"));
	Call::orders_map.insert(std::make_pair<int, std::string>(SIGTERM, "SIGTERM"));

	//Initialise resources_map
	Call::resources_map.insert(std::make_pair<int, std::string>(METAL, "METAL"));
	Call::resources_map.insert(std::make_pair<int, std::string>(ENERGY, "ENERGY"));
}

std::vector<std::string> loadExpertsXml()
{
	std::vector<std::string> experts_xml;
	std::string path = dir_path + "/expert/" + TracesParser::mission_name;
	DIR *pdir;
	struct dirent *pent;
	pdir = opendir(path.c_str());
	if (pdir)
	{
		while ((pent = readdir(pdir)))
		{
			std::string name = pent->d_name;
			if (name.find(".xml") != std::string::npos && name.compare("feedbacks.xml") != 0)
				experts_xml.push_back(loadFile(path + "\\" + name));
		}
	}
	closedir(pdir);
	return experts_xml;
}

int getParamPos(int argc, char *argv[], const char *param){
	for (int i = 0 ; i < argc ; i++)
		if (strcmp(argv[i], param) == 0)
			return i;
	return -1;
}

int main(int argc, char *argv[])
{
	// reset static fields
	Scenario::SCORE_TOLERENCE = 0.1;
	Scenario::WEIGHT_ALIGN_RATIO = 0.6;
	Scenario::OPTION_PENALTY = 1.6;
	Scenario::WEIGHT_MAXIMIZE_ALIGN = 0.2;
	Scenario::WEIGHT_MINIMIZE_LENGTH = 0.2;

	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
	{
		std::cout << "Usage: parser all [-d dir_path]\n";
		std::cout << "\tall: all complete traces files present in dir_path directory and its subdirectories (recursively) will be compressed\n";
		std::cout << "\tdir_path: path to the directory containing complete traces files (default ./example/)\n\n";
		std::cout << "Usage: parser filename [-d dir_path] [-la] [-sc n] [-war n] [-op n] [-wma n] [-wml] [-o output]\n";
		std::cout << "\tfilename: the complete traces file named filename will be compressed. This file must have the extension .log\n";
		std::cout << "\t-d dir_path: path to the directory containing complete traces files (default ./example/)\n";
		std::cout << "\t-la: launch analysis after compression. If this option is used :\n";
		std::cout << "\t\t- \"params.json\" has to be present in the dir_path directory. If no params file exist, the default compression mod will be used.\n";
		std::cout << "\t\t- \"feedbacks.xml\" has to be present in the dir_path directory\n";
		std::cout << "\t\t- \"feedbacks.xml\" in the directory \"[dir_path]/expert/[mission_name]\" can be present to add extra feedbacks for the mission\n";
		std::cout << "\t\t- there must be at least one solution (an XML file) for the mission in the directory \"[dir_path]/expert/[mission_name]\"\n";
		std::cout << "\t-sc n: SCORE_TOLERENCE  (default 0.1) define minimal score required to keep a pattern (a pattern with a score lesser than n will be ignored)\n";
		std::cout << "\t-war n: WEIGHT_ALIGN_RATIO (default 0.6) define weight of aligned tokens over optional tokens in score computing. -war + -wma + -wml sould be equal to 1.\n";
		std::cout << "\t-op n: OPTION_PENALTY (default 1.6) define penalty value of optional tokens. 1 means optional tokens have the same weight of aligned values, lesser than 1 means optional tokens are minimize against aligned tokens, greater than 1 means optional tokens are maximized against aligned tokens\n";
		std::cout << "\t-wma n: WEIGHT_MAXIMIZE_ALIGN (default 0.2) define weight of aligned tokens over maximal alignment. -war + -wma + -wml sould be equal to 1.\n";
		std::cout << "\t-wml n: WEIGHT_MINIMIZE_LENGTH (default 0.2) define weight of patterns length. -war + -wma + -wml sould be equal to 1.\n";
		std::cout << "\t-mw n: MIN_WINDOW (default 1) minimal window require to compress.\n";
		std::cout << "\t-o output: the output file name (without extension) to write results (default ./exemple/[filename]_compressed.txt)\n\n";
		return -1;
	}
	int paramPos = getParamPos(argc, argv, "-d");
	if (paramPos != -1 && paramPos+1 < argc)
		dir_path = argv[paramPos+1];
	// load compression params
	std::cout << "Try to open params.json file from example directory (" + dir_path + "/)" << std::endl;
	TracesParser::params_json = loadFile(dir_path + "/params.json");
	if (TracesParser::params_json.compare("") != 0)
		std::cout << "\tFile found and used for compression and analysis." << std::endl;
	else
		std::cout << "\tFile not found." << std::endl;
	if (strcmp(argv[1], "all") == 0)
		return compressAllTraces(dir_path);
	else
	{
		bool analysis = getParamPos(argc, argv, "-la") != -1;

		output = "";
		paramPos = getParamPos(argc, argv, "-o");
		if (paramPos != -1 && paramPos+1 < argc)
			output = argv[paramPos+1];
		
		paramPos = getParamPos(argc, argv, "-sc");
		if (paramPos != -1 && paramPos+1 < argc)
			Scenario::SCORE_TOLERENCE = strtof(argv[paramPos+1], NULL);
		else
			Scenario::SCORE_TOLERENCE = 0.1;
		
		paramPos = getParamPos(argc, argv, "-war");
		if (paramPos != -1 && paramPos+1 < argc)
			Scenario::WEIGHT_ALIGN_RATIO = strtof(argv[paramPos+1], NULL);
		else
			Scenario::WEIGHT_ALIGN_RATIO = 0.6;
		
		paramPos = getParamPos(argc, argv, "-op");
		if (paramPos != -1 && paramPos+1 < argc)
			Scenario::OPTION_PENALTY = strtof(argv[paramPos+1], NULL);
		else
			Scenario::OPTION_PENALTY = 1.6;
		
		paramPos = getParamPos(argc, argv, "-wma");
		if (paramPos != -1 && paramPos+1 < argc)
			Scenario::WEIGHT_MAXIMIZE_ALIGN = strtof(argv[paramPos+1], NULL);
		else
			Scenario::WEIGHT_MAXIMIZE_ALIGN = 0.2;
		
		paramPos = getParamPos(argc, argv, "-wml");
		if (paramPos != -1 && paramPos+1 < argc)
			Scenario::WEIGHT_MINIMIZE_LENGTH = strtof(argv[paramPos+1], NULL);
		else
			Scenario::WEIGHT_MINIMIZE_LENGTH = 0.2;
		
		paramPos = getParamPos(argc, argv, "-mw");
		if (paramPos != -1 && paramPos+1 < argc)
			Scenario::MIN_WINDOW = strtof(argv[paramPos+1], NULL);
		else
			Scenario::MIN_WINDOW = 1;

		// Compression
		std::string filename = argv[1];
		if (filename.find(".log") == std::string::npos)
		{
			std::cout << "not a log file" << std::endl;
			return -1;
		}
		std::string s = dir_path + "\\" + filename;
		std::ifstream ifs(s.c_str(), std::ios::in | std::ios::binary);
		if (!ifs.good())
		{
			std::cout << "error opening file: " << strerror(errno) << std::endl;
			return -1;
		}
		tp.parseLogs(ifs, false);
		std::cout << "traces compressed" << std::endl;
		saveCompressedTraces(filename);

		std::cout << Scenario::SCORE_TOLERENCE << " " << Scenario::WEIGHT_ALIGN_RATIO << " " << Scenario::OPTION_PENALTY << " " << Scenario::WEIGHT_MAXIMIZE_ALIGN << " " << Scenario::WEIGHT_MINIMIZE_LENGTH << " " << Scenario::MIN_WINDOW << std::endl;

		if (analysis)
		{
			std::cout << "launch analysis" << std::endl;
			initExternalConstants();
			// Analysis
			TracesAnalyser ta;
			const std::string feedbacks_xml = loadFile(dir_path + "/feedbacks.xml");
			const std::string mission_feedbacks_xml = loadFile(dir_path + "/expert/" + TracesParser::mission_name + "/feedbacks.xml");
			ta.loadXmlInfos(feedbacks_xml, mission_feedbacks_xml);
			std::vector<std::string> experts_xml = loadExpertsXml();
			if (experts_xml.empty())
			{
				std::cout << "no expert solutions for mission " + TracesParser::mission_name << std::endl;
				return -1;
			}
			filename.replace(filename.find(".log"), 4, "");
			std::cout << "construct feedbacks" << std::endl;
			std::ostringstream oss;
			oss << tp.lastCompressionXML;
			std::string json = ta.constructFeedback(oss.str(), experts_xml);
			std::cout << json << std::endl;
			std::ofstream jsonFile;
			jsonFile.open("feedback_result.json");
			if (jsonFile.good())
			{
				jsonFile << json;
				jsonFile.close();
			}
		}
	}
	return 0;
}
