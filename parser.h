/**********************************************
 * file : parser.h  Oct 01, 2011, by Hang Xiao
 *
 *************************************/
#ifndef __PARSER_H_H_
#define __PARSER_H_H_

#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>

//#include "simple_c.h"

using namespace std;
struct CommandOption
{
	string para_name;
	int option_type;  // 0 : no option
	// 1 : one option followed
	// 2 : no option or one non-image option
	// 3 : at least one option until next image or next parameter or end

};

struct SinglePara
{
	string para_name;          // -resize
	string para_string;
	SinglePara(){para_name=""; para_string=""; }
	SinglePara(string name, string str){para_name=name; para_string=str; }
};

typedef bool (*UsageFunc)(string,ostream&);

struct SimpleParser
{
	int cur_par_id;
	vector<string> filelist;
	vector<SinglePara> paras;
	vector<CommandOption> cmd_opts;
	string s_error;
	UsageFunc usage_func;

	SimpleParser(){cur_par_id = -1; usage_func = 0;}
	SimpleParser(CommandOption * cmds, int n){cur_par_id = -1; usage_func = 0; set_supported_cmd(cmds,n);}
	void set_usage_func(UsageFunc _func)
	{
		usage_func = _func;
	}
	bool set_supported_cmd(CommandOption * cmds, int n)
	{
		cmd_opts.clear();
		if(n <= 0) return false;
		bool is_ok = true;
		set<string> allcmds;
		for(int i = 0; i < n; i++) 
		{
			if(allcmds.find(cmds[i].para_name) != allcmds.end())
			{
				cout<<"warning : duplicated command option "<<cmds[i].para_name<<endl;
				is_ok = false;
			}
			allcmds.insert(cmds[i].para_name);
			cmd_opts.push_back(cmds[i]);
		}
		return false;
	}

	string err()
	{
		return s_error;
	}
	bool good()
	{
		if(s_error == "") return true;
		else return false;
	}
	bool is_support(string para_name)
	{
		if(cmd_opts.empty()) return false;
		int cmd_num = cmd_opts.size();
		for(int i = 0; i < cmd_num; i++)
		{
			string sup_cmd = cmd_opts[i].para_name;
			string cur_cmd = para_name;
			sup_cmd.at(0) = '-';
			cur_cmd.at(0) = '-';
			if(sup_cmd == cur_cmd) return true;
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
			if(cmd_opt == para_name) return cmd_opts[i].option_type;
		}
	}

	int get_order(string para_name)
	{
		if(paras.empty()) return -1;
		for(int i = 0; i < paras.size(); i++)
		{
			if(paras[i].para_name == para_name) return i;
		}
		return -1;
	}
	bool is_empty()
	{
		return paras.empty();
	}
	bool is_exist(string para_name)
	{
		if(get_order(para_name) != -1) return true;
		return false;
	}
	string get_cur_cmd()
	{
		return (cur_par_id >= 0) ? paras[cur_par_id].para_name : "";
	}
	bool get_next_cmd(string & next_cmd)
	{
		if(paras.empty()) return false;
		if(cur_par_id < -1) cur_par_id = -1;
		if(cur_par_id >= (int) paras.size() - 1 ){next_cmd="error"; return false;}
		cur_par_id++; 
		next_cmd = paras[cur_par_id].para_name;
		return true;
	}
	string get_para(string para_name)
	{
		int order = get_order(para_name);
		if(order == -1) return string("");
		return paras[order].para_string;
	}
	int get_delim_num(string para_name, string delim="x")
	{
		int count = 0;
		if(!is_support(para_name) || !is_exist(para_name)) return 0;
		string str = get_para(para_name);
		size_t found = str.find_first_of(delim);
		while(found != string::npos)
		{
			count++;
			found = str.find_first_of(delim, found + 1);
		}
		return count;
	}
	template <class T> bool get_int_para(T &v, string para_name, string &s_error)
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return false;}
		v = (T) (atol(get_para(para_name).c_str()));
		return true;
	}
	int get_int_para(string para_name)
	{
		int v = 0;
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return v;}
		v = atol(get_para(para_name).c_str());
		return v;
	}
	int get_int_para(string para_name, string &s_error)
	{
		int v = 0;
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return v;}
		v = atol(get_para(para_name).c_str());
		return v;
	}
	template <class T> bool get_int_para(T &v, string para_name, int index, string &s_error, string sep = "x")
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return false;}
		if(get_delim_num(para_name, sep) < index){s_error += "exceed the number of delim '" + sep + "'"; return false;}
		string par_str = get_para(para_name);
		size_t sp=0, ep=par_str.find_first_of(sep);
		if(index == 0)
		{
			if(ep == string::npos) {v = (T) (atol(par_str.c_str())); return true;}
			else if(ep == 0){v = 0; return true;}
			else {v = (T)(atol(par_str.substr(sp, ep - sp).c_str())); return true;}
		}
		while(index>0)
		{
			if(ep != string::npos){sp = ep + 1; ep = par_str.find_first_of(sep, sp);}
			else {s_error += "index exceed the number of "; s_error += sep; return false;}
			index--;
		}
		if(ep == string::npos) ep = par_str.size();
		v = (T)(atol(par_str.substr(sp, ep - sp).c_str()));
		return true;
	}
	int get_int_para(string para_name, int index, string sep = "x")
	{
		int v = 0;
		get_int_para(v, para_name, index, s_error, sep);
		return v;
	}
	int get_int_para(string para_name, int index, string &s_error, string sep = "x")
	{
		int v = 0;
		get_int_para(v, para_name, index, s_error, sep);
		return v;
	}
	bool get_double_para(double &v, string para_name, string &s_error)
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return false;}
		v = atof(get_para(para_name).c_str());
		return true;
	}
	double get_double_para(string para_name)
	{
		double v = 0.0;
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return v;}
		v = atof(get_para(para_name).c_str());
		return v;
	}
	double get_double_para(string para_name, string &s_error)
	{
		double v = 0.0;
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para ";s_error += para_name; return v;}
		v = atof(get_para(para_name).c_str());
		return v;
	}
	bool get_double_para(double &v, string para_name, int index, string &s_error, string sep = "x")
	{
		if(!is_support(para_name) || !is_exist(para_name)){s_error += "unsupported or unexist para"; return false;}
		if(get_delim_num(para_name, sep) < index){s_error += "exceed the number of delim " + sep; return false;}
		string par_str = get_para(para_name);
		size_t sp=0, ep=par_str.find_first_of(sep);
		if(index == 0)
		{
			if(ep == string::npos) {v = atof(par_str.c_str()); return true;}
			else if(ep == 0){v = 0; return true;}
			else {v=atof(par_str.substr(sp, ep - sp).c_str()); return true;}
		}
		while(index>0)
		{
			if(ep != string::npos){sp = ep + 1; ep = par_str.find_first_of(sep, sp);}
			else {s_error += "index exceed the number of "; s_error += sep; return false;}
			index--;
		}
		if(ep == string::npos) ep = par_str.size();
		v = atof(par_str.substr(sp, ep - sp).c_str());
		return true;
	}
	double get_double_para(string para_name, int index, string sep = "x")
	{
		//string s_error;
		return get_double_para(para_name, index, s_error, sep);
	}
	double get_double_para(string para_name, int index, string &s_error, string sep = "x")
	{
		double v = 0.0;
		get_double_para(v, para_name, index, s_error, sep);
		return v;
	}
	void add_para(string para_name, string para_string){
		paras.push_back(SinglePara(para_name, para_string));
	}
	void add_para(string para_name){
		paras.push_back(SinglePara(para_name, ""));
	}
	void add_para(SinglePara para){
		paras.push_back(para);
	}

	bool parse(int argc, char* argv[])
	{
		int i = 1;      // switch ind

		while(i < argc)
		{
			SinglePara para;
			if(argv[i][0] != '-') filelist.push_back(string(argv[i]));
			else if(is_support(argv[i]) && !is_exist(argv[i]))
			{
				int opt_type = option_type(argv[i]);
				if(opt_type == 0) add_para(argv[i]);
				else if((opt_type == 1) && i+1 < argc && argv[i+1][0] != '-') {add_para(argv[i], argv[i+1]); i++;}
				else if(opt_type == 2) 
				{
					if(i+1 < argc && argv[i+1][0] != '-'){add_para(argv[i], argv[i+1]); i++;}
					else add_para(argv[i]);
				}
				else if((opt_type == 3) && i+1 < argc && argv[i+1][0] != '-')
				{
					string opt = argv[i+1];
					int j = i+2;
					while(j < argc && argv[j][0] != '-')
					{
						opt = opt + " " + argv[j];
						j++;
					}
					add_para(argv[i], opt);
					i = j - 1;
				}
				else {s_error += "need parameter for "; s_error += argv[i]; print_usage(argv[i], cout);return false;}
			}
			// check other error reason
			else if(!is_support(argv[i]))
			{
				s_error += argv[i];
				s_error += " is not supported";
				return false;
			}
			else if(is_exist(argv[i]))
			{
				s_error += "duplicated para ";
				s_error += argv[i];
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
		if(usage_func) return (*(usage_func))(cmd_name, out); 
	}
};
#endif
