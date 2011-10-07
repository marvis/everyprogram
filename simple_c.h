#ifndef __SIMPLE_C_H__
#define __SIMPLE_C_H__

#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

template<class T> string num2str(T value)
{
	std::ostringstream o;
	if (!(o << value))
		return "";
	return o.str();
}
bool is_lowercase(char c)
{
	return (c >= 'a' && c <= 'z');
}
bool is_number(string str)
{
	bool isnum = true;
	for(int i = 0; i < str.size(); i++)
	{
		if(!isdigit(str[i])){isnum = 0; break;}
	}
	return isnum;
}

// file_type("test.tif") == ".tif"
string file_type(string para)
{
    int pos = para.find_last_of(".");
    if(pos == string::npos) return string("unknown");
    else return para.substr(pos, para.size() - pos);
}

// basename("test.tif") == "test"
string basename(string para)
{
	int pos = para.find_last_of(".");
	if(pos == string::npos) return para;
	else return para.substr(0, pos);
}

bool is_marker_file(string para)
{
	string right7 = (para.size() >= 7) ? para.substr(para.size() -7, 7) : "";
	if(right7 == ".marker") return true;
	else return false;
}

bool is_img_file(string para)
{
	string right4 = (para.size() >= 4) ? para.substr(para.size() -4, 4) : "";
	string right5 = (para.size() >= 5) ? para.substr(para.size() -5, 5) : "";

	// when para == local , it doesn't work
	//if((para.find_last_of(".raw") == (para.size() - 1)) || 
	// (para.find_last_of(".tif") == (para.size() - 1)) || 
	// (para.find_last_of(".tiff") == (para.size() - 1)) || 
	// (para.find_last_of(".lsm") == (para.size() - 1)) ){return true;}
	if(right4 == ".raw" || right4 == ".tif" || right4 == ".lsm" || right5 == ".tiff") return true;
	else return false;
}

bool save_to_history(int argc, char* argv[], string hist_file)
{
	if(0)
	{
		ofstream ofs(hist_file.c_str(), ios_base::out | ios_base::app);
		if(ofs.fail()){cout<<"unable to open history file : "<<hist_file<<endl; return false;}
		for(int i = 0; i < argc; i++)
		{
			ofs<<argv[i]<<" ";
		}
		ofs<<endl;
		ofs.close();
		return true;
	}
	else
	{
		ostringstream oss;
		oss<<"echo \"";
		for(int i = 0; i < argc; i++)
		{
			if(string("-hist") == argv[i] || string("-history") == argv[i]) return true;
			oss<<argv[i]<<" ";
		}
		oss<<"\" >> "<<hist_file;
		if(system(oss.str().c_str()) == -1) {cout<<"error : "<<oss.str()<<endl; return false;}
		return true;
	}
}

bool display_history(string hist_file)
{
	ifstream ifs(hist_file.c_str());
	if(ifs.fail()){cout<<"unable to open history file : "<<hist_file<<endl; return false;}
	int line_num = 1;
	while(ifs.good())
	{
		cout<<line_num<<": ";
		char line_str[1000];
		ifs.getline(line_str, 1000,'\n');
		cout<<line_str<<endl;
		line_num ++;
	}
	ifs.close();
	return true;
}

#endif
