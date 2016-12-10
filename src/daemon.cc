// daemon.cc
//
// Copyright (C) 2016 - Unknown
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
#include <vector>

void get_jobs(std::vector<int> &);

bool daemon_work(){
	std::vector<int> jobs;
	static pid_t ID[100]={0};
	static int work_cnt = 0;
	
	get_jobs(jobs);
	if(OJ_DEBUG){
		for(auto const &p: jobs){
			std::cout<<p<<",";
		}
		std::cout<<std::endl;
	}
	if(jobs.empty()){
		return false;
	}

	int i=0;
	for(auto const &p: jobs){
		if(OJ_DEBUG){
			std::cout<<"Judging solution "<<p<<std::endl;
		}
		if(work_cnt >= OJ_MAXRUNNING){
			pid_t tpid = waitpid(-1,NULL,0);
			for(i=0;i<OJ_MAXRUNNING;++i){
				if(ID[i] == tpid){
					work_cnt--;
					ID[i]=0;
					break;
				}
			}
		}else{
			for(i=0;i<OJ_MAXRUNNING;++i){
				if(ID[i] == 0) break;
			}
		}

		if(i<OJ_MAXRUNNING && work_cnt<OJ_MAXRUNNING){
			work_cnt++;
			ID[i] = fork();
			if(ID[i] == 0){
				judge_solution(p,i);
				exit(EXIT_SUCCESS);
			}
		}
	}
	return false;
}

void get_jobs(std::vector<int> &jobs){
	std::map<std::string, std::string> par;
	Json::Value sols = http_get("/judger/pending-solutions",par);
	//std::cout<<"get val:\n"<<val<<std::endl;
	for(int i=0;i<sols.size();++i){
		if(OJ_DEBUG){
			std::cout<<"solution:"<<std::endl<<sols[i]<<std::endl;
		}
		jobs.push_back(sols[i]["id"].asInt());
	}
}