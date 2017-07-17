// sim.cc
//
// Copyright (C) 2017 - Unknown
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
#include <wzoj-judger.h>
#include <dirent.h>
const char *Extention[] = {"c", "cc", "pas", "java", "py"};
const char *SimExtention[] = {"c", "c++", "pasc", "java", "text"};

bool isNum(char c){
	return '0' <= c && c <= '9';
}

pid_t daemon_pid;

void sim_wake(){
}

void sim_kill(){
	kill(daemon_pid, SIGKILL);
}

void sim_get_solutions(std::vector<int> &sid){
	std::map<std::string, std::string> par;
	Json::Value sols = http_get("/judger/get-sim-solutions",par);
	for(int i=0;i<sols.size();++i){
		if(OJ_DEBUG){
			std::cout<<"sim_solution:"<<std::endl<<sols[i]<<std::endl;
		}
		sid.push_back(sols[i]["id"].asInt());
	}
}

void sim_read_report(int sid, int &s2id, int &rate){
	FILE *fsim = fopen("sim.out", "r");
	static char buf1[1000], buf2[1000];
	int tr, ts1, ts2;

	s2id = rate = 0;

	fgets(buf1, 1000, fsim);

	while(~fscanf(fsim, "%s consists for %d %% of %s material\n", buf1, &tr, buf2)){
		if(tr <= rate) continue;
		int p1 = strlen(buf1), p2 = strlen(buf2);
		while(buf1[--p1] != '.');
		while(isNum(buf1[--p1]));
		sscanf(buf1 + p1 + 1, "%d", &ts1);
		
		while(buf2[--p2] != '.');
		while(isNum(buf2[--p2]));
		sscanf(buf2 + p2 + 1, "%d", &ts2);

		if(ts1 == ts2) continue;
		
		if(ts2 == sid) std::swap(ts1, ts2);
		sid = ts1;
		s2id = ts2;
		rate = tr;
	}
}

void update_sim(int sid,int s2id, int rate){
	std::map<std::string, std::string> par;
	par["solution_id"] = std::to_string(sid);
	par["solution2_id"] = std::to_string(s2id);
	par["rate"] = std::to_string(rate);
	http_post("/judger/update-sim", par);
}

void sim_work(int sid){
	if(OJ_DEBUG){
		std::cout<<"sim working on solution "<<sid<<std::endl;
	}
	Json::Value solution = get_solution(sid);
	std::string data_dir(OJ_HOME);
	int language = solution["language"].asInt();
	if(language == 0){
		language = 1;//C is C++
	}

	//data_dir
	data_dir += "/data/" + std::to_string(solution["problem_id"].asInt()) +
		"/ac/" + std::to_string(language) + "/";
	if(OJ_DEBUG){
		std::cout<<"data_dir:"<<data_dir<<std::endl;
	}
	execute_cmd (("mkdir -p " + data_dir).c_str());

	//files to be compared
	FILE *files = fopen("files.txt" ,"w");

	fprintf(files, "%s/%d.%s\n|\n", data_dir.c_str(),sid, Extention[language]);
	
	DIR *dp;
	dirent *dirp;
	dp = opendir(data_dir.c_str());
	while((dirp = readdir(dp)) != NULL){
		if(dirp->d_name[0] != '.'){
			fprintf(files, "%s/%s\n", data_dir.c_str(), dirp->d_name);
		}
	}
	closedir(dp);

	fclose(files);

	//new source file
	FILE *new_src = fopen((data_dir +
	            std::to_string(sid) + "." + Extention[language]).c_str(), "w");
	fputs(solution["code"].asString().c_str(), new_src);
	fclose(new_src);


	execute_cmd("sim_%s -i -pP -ae -osim.out -T -t 10 < files.txt", SimExtention[language]);

	int s2id=0, rate = 0;
	sim_read_report(sid, s2id, rate);

	update_sim(sid, s2id, rate);

	if(rate > 80){//delete similiar codes
		execute_cmd ("rm %s/%d.%s", data_dir.c_str(),sid, Extention[language]);
	}
}

void sim_set_workdir(){
	std::string workdir = std::string(OJ_HOME) + "/sim";
	chown(workdir.c_str(), JUDGER_UID, JUDGER_UID);
	chdir(workdir.c_str());
}

void sim_daemon(){
	std::vector<int> solutions_id;
	daemon_pid = fork();
	if(daemon_pid != 0){
		if(OJ_DEBUG){
			std::cout<<"sim daemon pid:"<<daemon_pid<<std::endl;
		}
		return;
	}
	sim_set_workdir();
	do{
		sim_get_solutions(solutions_id);
		for(auto const &p: solutions_id){
			sim_work(p);
		}
		if(solutions_id.empty()){
			sleep(10);
		}
		solutions_id.clear();
	}while(true);
}