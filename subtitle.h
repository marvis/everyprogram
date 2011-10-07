#ifndef __SUBTITLE_H__
#define __SUBTITLE_H__

#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>
#include <unistd.h>

#include "simple_c.h"

using namespace std;

struct Time
{
	int hour;
	int minute;
	int second;
	int ms; 
	Time()
	{
		hour = minute = second = ms = -1;
	}
	Time(int h, int m, int s, int _ms)
	{
		hour = h;
		minute = m;
		second = s;
		ms = _ms;
	}
};

struct Subtitle
{
	int id;
	Time start;
	Time stop;
	string sub_str;
	Subtitle()
	{
		id = -1; sub_str = "";
	}
	void clear()
	{
		id = -1;
		start = Time(-1,-1,-1,-1);
		stop = Time(-1,-1,-1,-1);
		sub_str = "";
	}
	bool is_ok()
	{
		return (id != -1 && start.hour >= 0 && start.minute >= 0 && start.second >= 0 && start.ms >= 0);
	}
};

vector<Subtitle> readSubtitle_file(string sub_file)
{
	vector<Subtitle> subtitles;
	ifstream ifs(sub_file.c_str());
	if(ifs.fail())
	{
		cerr<<"load subfile error!"<<endl;
		return subtitles;
	}

	char buff[1000];
	Subtitle cur_sub;
	int pos;
	while(ifs.good())
	{
		ifs.getline(buff, 1000, '\n');
		int len = strlen(buff);
		if(buff[len-1] == '\r') buff[len-1] = 0;

		string buff_str(buff);

		if(is_number(buff_str))
		{
			if(cur_sub.is_ok()) 
			{
				subtitles.push_back(cur_sub);
				cout<<cur_sub.id<<endl;
				cout<<cur_sub.start.hour<<":"<<cur_sub.start.minute<<":"<<cur_sub.start.second<<","<<cur_sub.start.ms<<" --> "<<cur_sub.stop.hour<<":"<<cur_sub.stop.minute<<":"<<cur_sub.stop.second<<","<<cur_sub.stop.ms<<endl;
				cout<<cur_sub.sub_str<<endl;
				cout<<endl;
			}
			cur_sub.clear();
			cur_sub.id = atoi(buff);

		}
		else if((pos = buff_str.find_first_of(":")) != string::npos)
		{
			if((pos = buff_str.find_first_of(":",  pos+1)) != string::npos &&
			   (pos = buff_str.find_first_of(",",  pos+1)) != string::npos &&
			   (pos = buff_str.find_first_of("-->",pos+1)) != string::npos &&
			   (pos = buff_str.find_first_of(":",  pos+1)) != string::npos &&
			   (pos = buff_str.find_first_of(":",  pos+1)) != string::npos &&
			   (pos = buff_str.find_first_of(",",  pos+1)) != string::npos)
			{
				int ps = 0; 
				int pe = buff_str.find_first_of(":", ps);
				cur_sub.start.hour = atoi(buff_str.substr(ps, pe-ps).c_str());

				ps = pe + 1;
				pe = buff_str.find_first_of(":", ps);
				cur_sub.start.minute = atoi(buff_str.substr(ps, pe-ps).c_str());

				ps = pe + 1;
				pe = buff_str.find_first_of(",", ps);
				cur_sub.start.second = atoi(buff_str.substr(ps, pe-ps).c_str());

				ps = pe + 1;
				pe = buff_str.find_first_of("-->", ps);
				cur_sub.start.ms = atoi(buff_str.substr(ps, pe-ps).c_str());

				ps = pe + 3;
				pe = buff_str.find_first_of(":", ps);
				cur_sub.stop.hour = atoi(buff_str.substr(ps, pe-ps).c_str());

				ps = pe + 1;
				pe = buff_str.find_first_of(":", ps);
				cur_sub.stop.minute = atoi(buff_str.substr(ps, pe-ps).c_str());

				ps = pe + 1;
				pe = buff_str.find_first_of(",", ps);
				cur_sub.stop.second = atoi(buff_str.substr(ps, pe-ps).c_str());
				
				ps = pe + 1;
				pe = buff_str.size();
				cur_sub.stop.ms = atoi(buff_str.substr(ps, pe-ps).c_str());
			}
		}
		else if(buff_str != "" && buff_str != "\n")
		{
			if(cur_sub.sub_str == "") cur_sub.sub_str = buff_str;
			else cur_sub.sub_str  += "\n" + buff_str;
		}
	}
	return subtitles;
}

void mysleep(int s)
{
	int start = time(NULL);
	while((time(NULL) - start) == s) return;
	//usleep(ms * 500);
}

bool run_subtitle(vector<Subtitle> & allsubtitles, long start = 0, long offset = 0) // start and offset : second
{
	long per_hour = 1 * 3600;
	long per_minute = 1 * 60;
	long pre_time = start;
	system("clear");
	for(int i = 0; i < allsubtitles.size(); i++)
	{
		Subtitle cur = allsubtitles[i];
		long start_sec = offset + cur.start.hour * per_hour + cur.start.minute * per_minute + cur.start.second + cur.start.ms/1000.0 + 0.5;
		if(start_sec < start) continue;
		sleep(start_sec - pre_time);
		system("clear");
		printf("%d  %2d:%2d:%2d %s\n",cur.id, cur.start.hour, cur.start.minute, cur.start.second, cur.sub_str.c_str());
		//cout<<cur.start.hourcur.sub_str<<endl;

		long stop_sec = offset + cur.stop.hour * per_hour + cur.stop.minute * per_minute + cur.stop.second + cur.stop.ms/1000.0 + 0.5;
		if(stop_sec <= start_sec) stop_sec = start_sec + 1;
		sleep((stop_sec - start_sec));

		pre_time = stop_sec;
	}
}

#endif
