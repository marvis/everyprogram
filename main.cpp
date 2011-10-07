#include "perfect_parser.h"
#include "parser.h"
#include "subtitle.h"

void printHelp();
bool printUsage(string cmd_name, ostream & out);

CommandOption command_options[] = {
	{"+help",0},{"+search",0},
	{"+subtitle",1},{"-start",1}
};

bool run_parser_main(PerfectParser &paras);

int main(int argc, char* argv[])
{
	PerfectParser paras(command_options, sizeof(command_options)/sizeof(CommandOption));

	paras.set_usage_func(&printUsage);
	if(!paras.parse(argc, argv))
	{
		cout<<"Invalid argument : "<< paras.s_error<<endl; return 0;
	}

	if(! run_parser_main(paras)) 
	{ 
		cout<<"run error"<<endl;
	}
	return 0;
}

bool run_parser_main(PerfectParser &paras)
{
	if(paras.is_empty()) {printHelp(); return true;}
	string cmd_name("");
	while(paras.get_next_cmd(cmd_name))
	{
		if(cmd_name == "-help")
		{
			cout<<"it is help"<<endl;
		}
		else if(cmd_name == "-search")
		{
			cout<<"it is search"<<endl;
		}
		else if(cmd_name == "-subtitle")
		{
			string sub_file = paras.get_para("-subtitle");
			long start = paras.is_exist("-start") ? paras.get_double_para("-start"): 0;
			cout<<"sub_file = "<<sub_file<<endl;
			cout<<"start = "<<start<<endl;
			vector<Subtitle> allsubtitles = readSubtitle_file(sub_file);
			cout<<allsubtitles.size()<<" subtitles is read"<<endl;
			run_subtitle(allsubtitles, start);
		}
	}
		
	return true;
}


void printHelp()
{
	cout<<"print help here"<<endl;
}

bool printUsage(string cmd_name, ostream & out)
{
	return true;
}
