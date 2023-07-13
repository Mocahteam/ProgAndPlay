#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

#include "TracesParser.h"
#include "Scenario.h"
#include "constantList_KP4.1.h"
#include "VariantTKE.h"

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
	if (TracesParser::outputLog)
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
	if (TracesParser::outputLog)
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
			if (TracesParser::outputLog)
				std::cout << "move to directory " << pent->d_name << std::endl;
			compressAllTraces(dir_path + "\\" + pent->d_name);
		}
		else if (strcmp(pent->d_name, ".") != 0 && strcmp(pent->d_name, "..") != 0)
		{
			std::string filename(pent->d_name);
			if (filename.find("_compressed") == std::string::npos && filename.find(".log") != std::string::npos && filename.compare("meta.log") != 0)
			{
				if (TracesParser::outputLog)
					std::cout << "parse " << dir_path << "\\" << filename << std::endl;
				std::string s = dir_path + "\\" + filename;
				std::ifstream ifs(s.c_str(), std::ios::in | std::ios::binary);
				tp.parseLogs(ifs);
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
	Scenario::WEIGHT_MINIMIZE_LENGTH = 0.4;

	if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
	{
		std::cout << "Usage: parser all [-d dir_path] [-disableLogs]\n";
		std::cout << "\tall: all complete traces files present in dir_path directory and its subdirectories (recursively) will be compressed\n";
		std::cout << "\tdir_path: path to the directory containing complete traces files (default ./example/)\n\n";
		std::cout << "\tdisableLogs: if set no log will be written in std::cout\n\n";
		std::cout << "Usage: parser filename [-d dir_path] [-k n] [-alpha n] [-sc n] [-war n] [-cl n] [-dl n] [-tl n] [-o output]\n";
		std::cout << "\tfilename: the complete traces file named filename will be compressed. This file must have the extension .log\n";
		std::cout << "\t-d dir_path: path to the directory containing complete traces files (default ./example/)\n";
		std::cout << "\tOptions for episode identification:";
		std::cout << "\t\t-topk n: K (default 10) number of best episode to find by TKE.\n";
		std::cout << "\t\t-alpha n: ALPHA (default 2) window multiplier used by TKE to jump events proportionaly to episode size (will produce optional events).\n";
		std::cout << "\t\t-ws n: WEIGHT_SUPPORT (default 0.5) weight of the support in relation to proximity parameter. Must be included between [0, 1]. 0 means the support is ignored (only proximity). 1 means the proximity is ignored (only support)";
		std::cout << "\t\t-pb n: PROXIMITY_BALANCING (default 0.5) control balancing between inside and outside proximity of episodes. Must be included between [0, 1]. 0 means take only inside proximity in consideration (no outside proximity). 1 means take only outside proximity in consideration (no inside proximity). If WEIGH_SUPPORT is set to 1, PROXIMITY_BALANCING is useless.";
		std::cout << "\tOptions for episode integration:";
		std::cout << "\t\t-sc n: SCORE_TOLERENCE  (default 0.2) define minimal score required to keep a pattern (a pattern with a score lesser than n will be ignored)\n";
		std::cout << "\t\t-war n: WEIGHT_ALIGN_RATIO (default 0.6) define weight of aligned tokens over scenario length in scenario score computing. Must be included between [0, 1]. 0 means alignement ratio is ignored (only scenario length is considered). 1 means only alignement ratio is considered (scenario length is ignored)\n";
		std::cout << "\t\t-gr n: GAP_RATIO  (default 0.5) controls the size of gaps between episodes in relation to the length of the trace. Must be included between [0, 1]. 0 means episodes will be merge if no gap exists between them. 1 means episodes will be merge even if they are separated by te entire trace.\n";
		std::cout << "\t\t-cl n: CANDIDATE_LIMIT (default 50) number maximun of candidates for process compression.\n";
		std::cout << "\t\t-dl n: DESCEND_LIMIT (default 3) number of consecutive drops that can allow a scenario's score. If scenario's score drops more than n, the scenario is deleted.\n";
		std::cout << "\t\t-tl n: TIME_LIMIT (default 10 seconds) max time to process compression (if give a negative number algorithm will not consider time).\n";
		std::cout << "\tOther options:";
		std::cout << "\t\t-o output: the output file name (without extension) to write results (default ./exemple/[filename]_compressed.txt)\n\n";
		std::cout << "\t\t-disableLogs: if set no log will be written in std::cout\n\n";
		return -1;
	}
	
	TracesParser::outputLog = getParamPos(argc, argv, "-disableLogs") == -1;

	int paramPos = getParamPos(argc, argv, "-d");
	if (paramPos != -1 && paramPos+1 < argc)
		dir_path = argv[paramPos+1];
	// load compression params
	if (TracesParser::outputLog)
		std::cout << "Try to open params.json file from example directory (" + dir_path + "/)" << std::endl;
	TracesParser::params_json = loadFile(dir_path + "/params.json");
	if (TracesParser::params_json.compare("") != 0){
		if (TracesParser::outputLog)
			std::cout << "\tFile found and used for compression." << std::endl;
	}
	else
		if (TracesParser::outputLog)
			std::cout << "\tFile not found." << std::endl;
	if (strcmp(argv[1], "all") == 0)
		return compressAllTraces(dir_path);
	else
	{
		output = "";
		paramPos = getParamPos(argc, argv, "-o");
		if (paramPos != -1 && paramPos+1 < argc)
			output = argv[paramPos+1];

		paramPos = getParamPos(argc, argv, "-topk");
		if (paramPos != -1 && paramPos+1 < argc)
			VariantTKE::K = strtof(argv[paramPos+1], NULL);
		else
			VariantTKE::K = 10;

		paramPos = getParamPos(argc, argv, "-alpha");
		if (paramPos != -1 && paramPos+1 < argc)
			VariantTKE::ALPHA = strtof(argv[paramPos+1], NULL);
		else
			VariantTKE::ALPHA = 2;

		paramPos = getParamPos(argc, argv, "-ws");
		if (paramPos != -1 && paramPos+1 < argc)
			Episode::WEIGHT_SUPPORT = strtof(argv[paramPos+1], NULL);
		else
			Episode::WEIGHT_SUPPORT = 0.5;

		paramPos = getParamPos(argc, argv, "-pb");
		if (paramPos != -1 && paramPos+1 < argc)
			Episode::PROXIMITY_BALANCING = strtof(argv[paramPos+1], NULL);
		else
			Episode::PROXIMITY_BALANCING = 0.5;
		
		paramPos = getParamPos(argc, argv, "-sc");
		if (paramPos != -1 && paramPos+1 < argc)
			Scenario::SCORE_TOLERENCE = strtof(argv[paramPos+1], NULL);
		else
			Scenario::SCORE_TOLERENCE = 0.2;
		
		paramPos = getParamPos(argc, argv, "-war");
		if (paramPos != -1 && paramPos+1 < argc){
			Scenario::WEIGHT_ALIGN_RATIO = strtof(argv[paramPos+1], NULL);
			Scenario::WEIGHT_MINIMIZE_LENGTH = float(1 - Scenario::WEIGHT_ALIGN_RATIO);
		}
		else{
			Scenario::WEIGHT_ALIGN_RATIO = 0.6;
			Scenario::WEIGHT_MINIMIZE_LENGTH = 0.4;
		}
		
		paramPos = getParamPos(argc, argv, "-gr");
		if (paramPos != -1 && paramPos+1 < argc)
			TracesParser::GAP_RATIO = strtof(argv[paramPos+1], NULL);
		else
			TracesParser::GAP_RATIO = 0.5;

		paramPos = getParamPos(argc, argv, "-cl");
		if (paramPos != -1 && paramPos+1 < argc)
			TracesParser::CANDIDATE_LIMIT = strtof(argv[paramPos+1], NULL);
		else
			TracesParser::CANDIDATE_LIMIT = 50;

		paramPos = getParamPos(argc, argv, "-dl");
		if (paramPos != -1 && paramPos+1 < argc)
			TracesParser::DESCEND_LIMIT = strtof(argv[paramPos+1], NULL);
		else
			TracesParser::DESCEND_LIMIT = 3;

		paramPos = getParamPos(argc, argv, "-tl");
		if (paramPos != -1 && paramPos+1 < argc)
			TracesParser::TIME_LIMIT = strtof(argv[paramPos+1], NULL);
		else
			TracesParser::TIME_LIMIT = 10;

		// Compression
		std::string filename = argv[1];
		if (filename.find(".log") == std::string::npos)
		{
			if (TracesParser::outputLog)
				std::cout << "not a log file" << std::endl;
			return -1;
		}
		std::string s = dir_path + "\\" + filename;
		std::ifstream ifs(s.c_str(), std::ios::in | std::ios::binary);
		if (!ifs.good())
		{
			if (TracesParser::outputLog)
				std::cout << "error opening file: " << strerror(errno) << std::endl;
			return -1;
		}
		tp.parseLogs(ifs);
		if (TracesParser::outputLog)
			std::cout << "traces compressed" << std::endl;
		saveCompressedTraces(filename);

		if (TracesParser::outputLog)
			std::cout << Scenario::SCORE_TOLERENCE << " " << Scenario::WEIGHT_ALIGN_RATIO << " " << Scenario::WEIGHT_MINIMIZE_LENGTH << std::endl;
	}
	return 0;
}
