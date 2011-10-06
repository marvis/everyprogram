#include "perfect_parser.h"
#include "parser.h"

void printHelp();
bool printUsage(string cmd_name, ostream & out);

CommandOption command_options[] = {
	{"-help",0},{"-search",0}
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
