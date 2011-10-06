/*****************************************************
 * file : perfect_parser.h  Oct 01, 2011, by HangXiao
 *
 * function : derived from parser.h, it will support much powerful parameter parser work. And it is specially designed for v3d_convert
 * new features :
 * 1. support main command, the command with preffix '+' will be marked as main command, otherwise option. The option command belong to the nearest main command. If no main command before itself, it will be global option command.
 * 2. support abbreviation, each command will seach in the following order
 *    a. if has_main_command , search all its option command and then the global option command
 *    b. if no main_command, search all main command and all global option command
 * 3. the match between abbreviation and support command will be
 	  a. is exactly same, reture true
 *    b. split the support command into many parts by '-' or '+'
 *    b. if abbr lenth is less or equal to parts number ,
 *          extract the first word of each part to form a new string 
 *          if abbr is the substring of the new string, return true
 *       else 
 *          start from the first part
 *          if no common prefix return false
 *          if common prefix between abbr and curent part is no empty, cutoff the common prefix of abbr, go to next part 
 * 4. abbreviation is used only for users to invoke your program, when you write your program, your are not allowd to use abbreviation
 * 5. so for the program you only have to rewrite parser(). The only thing to do is convert the abbr to the correct full command
 * 6. child option and global should not appear before main command
 *    if no main command, global could exist
 *****************************************************/

#ifndef __Perfect_PARSER_H__
#define __Perfect_PARSER_H__

#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <iostream>

#include "parser.h"
using namespace std;

// return common prefix number
int common_prefix(string str1, string str2)
{
	string short_str = str1;
	string long_str = str2;
	if(str1.size() > str2.size())
	{
		short_str = str2;
		long_str = str1;
	}
	int i = 0;
	while(i  < short_str.size() && short_str.at(i) == long_str.at(i)) i++;
	return i;
}

// make sure abbr is abbreviation


struct PerfectParser : public SimpleParser
{
	map<string, int> allglobal_opts;
	map<string, int> allmain_cmds;  // store "-cmd-name" instead of "+cmd-name"

	PerfectParser() : SimpleParser(){}
	PerfectParser(CommandOption * cmds, int n){
		cur_par_id = -1; 
		usage_func = 0; 
		set_supported_cmd(cmds,n);
	}
	
	// abbr and full should better prefixed by '-'
	bool is_abbr_match(string abbr, string full, int level)
	{
		if(abbr[0] != '-') abbr = "-" + abbr;
		if(full[0] != '-') full = "-" + full;

		// level 0
		if(level == 0) return (abbr == full);

		//get splits from abbr and full
		vector<string> abbr_splits, full_splits;
		int ps = 0, pe = 0;
		while((pe = abbr.find_first_of("-", ps+1)) != string::npos)
		{
			abbr_splits.push_back(abbr.substr(ps+1, pe - ps -1));
			ps = pe;
		}
		abbr_splits.push_back(abbr.substr(ps+1, abbr.size() - ps -1));

		ps = 0; pe = 0;
		while((pe = full.find_first_of("-", ps+1)) != string::npos)
		{
			full_splits.push_back(full.substr(ps+1, pe - ps -1));
			ps = pe;
		}
		full_splits.push_back(full.substr(ps+1, full.size() - ps - 1));


		if(abbr_splits.size() == 1)
		{
			abbr = abbr_splits[0];

			int common_num = 0;
			for(int i = 0; i < full_splits.size(); i++)
			{
				if(abbr == "") return true;
				common_num = common_prefix(abbr, full_splits[i]);
				if(common_num <= 0) return false;
				abbr = abbr.substr(common_num, abbr.size() - common_num);
			}
			// level 1
			if(level >= 1 && abbr == "") return true;

			// level 2 for abbr != ""

			string last_split = full_splits[full_splits.size() -1];
			last_split = last_split.substr(common_num, last_split.size() - common_num);
			int pos = -1;
			bool is_find = true;
			for(int i = 0; i < abbr.size(); i++)
			{
				pos = last_split.find_first_of(abbr[i],pos+1);
				if(pos == string::npos){is_find = false;break;}
			}
			if(level >= 2 && is_find) return true;
		}
		else if(abbr_splits.size() > full_splits.size()) return false;
		else // abbr_splits.size() <= full_splits.size() && abbr_splits.size() > 1
		{
			bool is_find = true;
			for(int i = 0; i < abbr_splits.size(); i++)
			{
				string abbr_str = abbr_splits[i];
				string full_str = full_splits[i];

				if(abbr_str.size() > full_str.size()) {is_find = false; break;}
				if(abbr_str[0] != full_str[0]){is_find = false; break;}
				int pos = 0;
				for(int j = 1; j < abbr_str.size(); j++)
				{
					pos = full_str.find_first_of(abbr_str[j],pos+1);
					if(pos == string::npos){is_find = false;break;}
				}
				if(!is_find) break;
			}
			if(level >= 1 && is_find) return true;
		}
		return false;
	}

	int option_type(string para_name)
	{
		if(!is_support(para_name)) return false;
		int cmd_num = cmd_opts.size();
		for(int i = 0; i < cmd_num; i++)
		{
			string cmd_opt = cmd_opts[i].para_name;
			cmd_opt[0] = '-';
			if(cmd_opt == para_name) return cmd_opts[i].option_type;
		}
	}

	bool find_abbr_in_range(string abbr, map<string, int> & range, vector<string> & candidates)
	{
		if(abbr[0] != '-') abbr = "-" + abbr;
		int level = 0;
		int is_find = 0;
		while(level <= 2)
		{
			for(map<string,int>::iterator it = range.begin(); it != range.end(); it++)
			{
				string cur_cmd = (*it).first;
				if(is_abbr_match(abbr, cur_cmd, level))
				{
					is_find = true;
					candidates.push_back(cur_cmd);
				}
			}
			if(is_find) break;
			level++;
		}
		if(candidates.empty()) return 0;
		return (level >= 1);
	}
	bool check_valid()
	{
		int i = 0;
		bool is_ok = true;
		set<string> allchild_cmds;
		while(i < cmd_opts.size() && cmd_opts[i].para_name[0] == '-') 
		{
			string cur_opt = cmd_opts[i].para_name;
			if(allglobal_opts.find(cur_opt) != allglobal_opts.end())
			{
				cout<<"duplicated global option "<<cur_opt<<endl;
				is_ok = false;
			}
			allglobal_opts[cur_opt] = i;
			i++;
		}
		set<string> localchild_opts;
		string cur_main = "";
		while(i < cmd_opts.size())
		{
			string cur_opt = cmd_opts[i].para_name;
			if(cur_opt[0] == '+')
			{
				cur_main = cur_opt;
				localchild_opts.clear();
				string cpy_opt = cur_opt; cpy_opt[0] = '-';
				if(allmain_cmds.find(cpy_opt) != allmain_cmds.end())
				{
					cout<<"duplicated main command "<<cur_opt<<endl;
					is_ok = false;
				}
				allmain_cmds[cpy_opt] = i;

				if(allglobal_opts.find(cpy_opt) != allglobal_opts.end())
				{
					cout<<"main command "<<cur_opt<<" duplicat with global option "<<cpy_opt<<endl;
					is_ok = false;
				}
			}
			else
			{
				if(localchild_opts.find(cur_opt) != localchild_opts.end())
				{
					cout<<"duplicated child option "<<cur_opt<<"  in main cmd "<<cur_main<<endl;
					return false;
				}
				localchild_opts.insert(cur_opt);
				allchild_cmds.insert(cur_opt);
			}
			i++;
		}
		map<string, int>::iterator it = allmain_cmds.begin();
		while(it != allmain_cmds.end())
		{
			string main_cmd = (*it).first;
			if(allchild_cmds.find(main_cmd) != allchild_cmds.end())
			{
				string child_opt = main_cmd;
				main_cmd[0] = '+';
				cout<<"main command "<<main_cmd<<" duplicate with child option "<<child_opt<<endl;
				is_ok = false;
			}
			it++;
		}
		return is_ok;
	}

	bool set_supported_cmd(CommandOption * cmds, int n)
	{
		cmd_opts.clear();
		if(n <= 0) return false;
		for(int i = 0; i < n; i++) cmd_opts.push_back(cmds[i]);
		return check_valid();
	}
	// if contains abbr return true
	bool abbr2full(int argc, char* argv[], vector<string> &args)
	{
		bool is_abbr_exist = false;
		args.clear();
		args.push_back(argv[0]);
		string abbr;
		bool has_main_cmd;
		int main_cmd_id;
		int i = 1;
		for(i = 1; i < argc; i++) if(argv[i][0] == '+') argv[i][0] = '-';
		i = 1;
		while(i < argc && argv[i][0] != '-')
		{
			args.push_back(argv[i]);
			i++;
		}
		if(i == argc) return false;
		abbr = argv[i];
		// first command should be analysed as main command
		int find_num = 0;
		vector<string> candidates;
		int id = 1;
		is_abbr_exist |= find_abbr_in_range(abbr, allmain_cmds, candidates);
		if(!candidates.empty())
		{
			find_num = candidates.size();
			id = 1;
			if(find_num >1)
			{
				cout<<"Do you mean "<<abbr<<endl;
				cout<<endl;
				for(int j = 0; j < find_num; j++)
				{
					cout<<" "<<j+1<<": "<<candidates[j]<<endl;
				}
				cout<<endl;
				cout<<"choose : [1] ";
				cin>>id;
			}
			if(id < 1 || id > find_num) id = 1;
			main_cmd_id = allmain_cmds[candidates[id-1]];
			string main_cmd = cmd_opts[main_cmd_id].para_name;
			main_cmd[0] = '-';
			args.push_back(main_cmd);
			has_main_cmd = true;
			i++;
		}
		else
		{
			has_main_cmd = false;
		}
		while(i < argc)
		{
			if(argv[i][0] != '-')
			{
				args.push_back(argv[i]);
				i++;
				continue;
			}
			else
			{
				abbr = argv[i];
				if(has_main_cmd)
				{
					map<string, int> range;
					int j = main_cmd_id + 1; 
					while(j < cmd_opts.size() && cmd_opts[j].para_name[0] != '+')
					{
						string local_opt = cmd_opts[j].para_name;
						range[local_opt] = j;
						j++;
					}
					candidates.clear();
					is_abbr_exist |= find_abbr_in_range(abbr, range, candidates);
					if(!candidates.empty())
					{
						find_num = candidates.size();
						id = 1;
						if(find_num >1)
						{
							cout<<"Do you mean "<<abbr<<endl;
							cout<<endl;
							for(int j = 0; j < find_num; j++)
							{
								cout<<" "<<j+1<<": "<<candidates[j]<<endl;
							}
							cout<<endl;
							cout<<"choose : [1] ";
							cin>>id;
						}
						if(id < 1 || id > find_num) id = 1;

						string local_opt = candidates[id-1];
						args.push_back(local_opt);
						i++;
						continue;
					}
				}
				candidates.clear();
				is_abbr_exist |= find_abbr_in_range(abbr, allglobal_opts, candidates);
				if(!candidates.empty())
				{
					find_num = candidates.size();
					id = 1;
					if(find_num >1)
					{
						cout<<"Do you mean "<<abbr<<endl;
						cout<<endl;
						for(int j = 0; j < find_num; j++)
						{
							cout<<" "<<j+1<<": "<<candidates[j]<<endl;
						}
						cout<<endl;
						cout<<"choose : [1] ";
						cin>>id;
					}
					if(id < 1 || id > find_num) id = 1;
					string global_opt = candidates[id-1];
					args.push_back(global_opt);
					i++;
					continue;
				}
				else
				{
					s_error += abbr + " unknow command";
					args.clear();
					return false;
				}
			}
		}
		return is_abbr_exist;
	}
	bool parse(int argc, char* argv[])
	{
		int i = 1;      // switch ind
		vector<string> args;

		bool is_abbr_exist = abbr2full(argc, argv, args);
		if(args.empty()) return false;
		if(is_abbr_exist) 
		{
			for(int j = 0; j < argc; j++) 
				cout<<args[j]<<" ";
			cout<<endl;
		}

		while(i < argc)
		{
			SinglePara para;
			if(args[i][0] != '-') filelist.push_back(string(args[i]));
			else if(is_support(args[i]) && !is_exist(args[i]))
			{
				int opt_type = option_type(args[i]);
				if(opt_type == 0) add_para(args[i]);
				else if((opt_type == 1) && i+1 < argc && args[i+1][0] != '-') {add_para(args[i], args[i+1]); i++;}
				else if((opt_type == 2)) 
				{
					if(i+1 < argc && args[i+1][0] != '-'){add_para(args[i], args[i+1]); i++;}
					else add_para(args[i]);
				}
				else if((opt_type == 3) && i+1 < argc && args[i+1][0] != '-')
				{
					string opt = args[i+1];
					int j = i+2;
					while(j < argc && args[j][0] != '-')
					{
						opt = opt + " " + args[j];
						j++;
					}
					add_para(args[i], opt);
					i = j - 1;
				}
				else {s_error += "need parameter for "; s_error += args[i]; if(usage_func) (*(usage_func))(args[i], cout); return false;}
			}
			// check other error reason
			else if(!is_support(args[i]))
			{
				s_error += args[i];
				s_error += " is not supported";
				return false;
			}
			else if(is_exist(args[i]))
			{
				s_error += "duplicated para ";
				s_error += args[i];
				return false;
			}
			else
			{
				s_error += "what is this paramter";
				return false;
			}
			i++;
		}
		return true;
	}

	bool print_usage(string cmd_name, ostream & out)
	{
		if(cmd_name == "") return false;
		bool is_usage_found = 0;
		out<<"Usage :"<<endl<<endl;
		if(usage_func && (*(usage_func))(cmd_name, out)) return true;

		vector<string> candidates;
		bool is_abbr = find_abbr_in_range(cmd_name, allmain_cmds, candidates);
		int find_num = candidates.size();
		if(candidates.empty())
		{
			out<<"Invalid command : "<<cmd_name<<endl;
			return false;
		}
		else if(find_num == 1)
		{
			cmd_name = candidates[0];
		}
		else
		{
			string abbr = cmd_name.substr(1, cmd_name.size() -1);
			int id = 1;
			cout<<"Do you mean "<<abbr<<endl;
			cout<<endl;
			for(int j = 0; j < find_num; j++)
			{
				cout<<" "<<j+1<<": "<<candidates[j]<<endl;
			}
			cout<<endl;
			cout<<"choose : [1] ";
			cin>>id;
			if(id < 1 || id > find_num) id = 1;
			cmd_name = candidates[id -1];
		}
		if((*(usage_func))(cmd_name, out)) return true;

		int main_cmd_id = allmain_cmds[cmd_name];
		int i = main_cmd_id + 1;
		out<<"v3d_convert [<img_file>] "<<cmd_name<<" ";
		while(cmd_opts[i].para_name[0] != '+' && i < cmd_opts.size())
		{
			string para_name = cmd_opts[i].para_name;
			int opt_type = cmd_opts[i].option_type;
			out<<para_name<<" ";
			if(opt_type == 0)
			{
			}
			else if(opt_type == 1)
			{
				out<<"<para> ";
			}
			else if(opt_type == 2)
			{
				out<<"[<para>] ";
			}
			else if(opt_type > 2)
			{
				out<<"<para> ... ";
			}
			i++;
		}
		out<<endl<<endl;
	}
	vector<string> search(string query, int level)
	{
		set<string> result;
		vector<string> output;
		// 1. search abbr in main commands, return matched main commands
		vector<string> candidates;
		find_abbr_in_range(query, allmain_cmds, candidates);
		if(!candidates.empty())
		{
			for(int i = 0; i < candidates.size(); i++)
			{
				result.insert(candidates[i]);
				output.push_back(candidates[i]);
			}
		}
		if(level <= 1) return output;
		// 2. search abbr in options of each main command and return matched main command and the option
		map<string, int>::iterator it = allmain_cmds.begin();
		while(it != allmain_cmds.end())
		{
			string main_cmd = (*it).first;
			int next_id = (*it).second;
			map<string, int> range;
			while(next_id+1 < cmd_opts.size() && cmd_opts[next_id+1].para_name[0] != '+')
			{
				string cur_opt = cmd_opts[next_id+1].para_name;
				range[cur_opt] = next_id+1;
				next_id++;
			}
			candidates.clear();
			find_abbr_in_range(query, range, candidates);
			if(!candidates.empty())
			{
				for(int i = 0; i < candidates.size(); i++)
				{
					string out_str = main_cmd + " " + candidates[i];
					if(result.find(out_str) == result.end())
					{
						result.insert(out_str);
						output.push_back(out_str);
					}
				}
			}
			it++;
		}
		if(level <= 2) return output;
		// 3. search in usage return the main command whose usage contain query string
		it = allmain_cmds.begin();
		while(it != allmain_cmds.end())
		{
			string main_cmd = (*it).first;
			ostringstream oss;
			if((*(usage_func))(main_cmd, oss)) 
			{
				string help_str = oss.str();
				if(help_str.find(query) != string::npos)
				{
					string out_str = "-help " + main_cmd.substr(1, main_cmd.size());
					if(result.find(out_str) == result.end())
					{
						result.insert(out_str);
						output.push_back(out_str);
					}
				}
			}
			it++;
		}
		if(level <= 3) return output;
		return output;
	}
};

#endif
