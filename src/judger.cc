// judger.cc
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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <dirent.h>
#include <stdarg.h>

#define STD_MB 1048576
#define STD_F_LIM (STD_MB<<5)
#define BUFFER_SIZE 512

int execute_cmd(const char * fmt, ...) {
	char cmd[BUFFER_SIZE];

	int ret = 0;
	va_list ap;

	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	ret = system(cmd);
	va_end(ap);
	return ret;
}

std::string readFile(const std::string &fileName)
{
    std::ifstream ifs(fileName.c_str(),
                      std::ios::in | std::ios::binary | std::ios::ate);

    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::vector<char> bytes(fileSize);
    ifs.read(&bytes[0], fileSize);

    return std::string(&bytes[0], fileSize);
}

int isInFile(const char fname[]) {
	int l = strlen(fname);
	if (l <= 3 || strcmp(fname + l - 3, ".in") != 0)
		return 0;
	else
		return l - 3;
}

void set_rlimits(){
	struct rlimit LIM;
	LIM.rlim_max = 800;
	LIM.rlim_cur = 800;
	setrlimit(RLIMIT_CPU, &LIM);

	LIM.rlim_max = 80 * STD_MB;
	LIM.rlim_cur = 80 * STD_MB;
	setrlimit(RLIMIT_FSIZE, &LIM);

	LIM.rlim_max = STD_MB << 11;
	LIM.rlim_cur = STD_MB << 11;
	setrlimit(RLIMIT_AS, &LIM);

	LIM.rlim_cur = LIM.rlim_max = 200;
	setrlimit(RLIMIT_NPROC, &LIM);
}

bool checkout(int sid){
	std::map<std::string, std::string> par;
	par["solution_id"]=std::to_string(sid);
	if(sid == OJ_SOLUTION_NO){
		par["force"] = "true";
	}
	Json::Value val = http_post("/judger/checkout", par);
	std::cout<<val["ok"].asString()<<std::endl;
	return val["ok"].asBool();
}

void set_workdir(int rid){
	std::string workdir = std::string(OJ_HOME) + "/run" + std::to_string(rid);
	struct stat st = {0};
	if(stat(workdir.c_str(), &st) == -1) {
		mkdir(workdir.c_str(), 0700);
		mkdir((workdir + "/run").c_str(), 0700);
		chown(workdir.c_str(), JUDGER_UID, JUDGER_UID);
		chown((workdir + "/run").c_str(), JUDGER_UID, JUDGER_UID);
	}
	chdir(workdir.c_str());
}

void clean_run_dir(){
	execute_cmd("/bin/umount ./run/*");
	execute_cmd("sudo --user=judger /bin/rm -R -f ./run/*");
}

Json::Value get_solution(int sid){
	std::map<std::string, std::string> par;
	par["solution_id"] = std::to_string(sid);
	return http_get("/judger/solution", par);
}

Json::Value get_problem(int pid){
	std::map<std::string, std::string> par;
	par["problem_id"] = std::to_string(pid);
	return http_get("/judger/problem", par);
}

bool compile(Json::Value solution, Json::Value problem, std::string &ce){
	const char * CP_C[] = { "gcc", "Main.c", "-o", "Main", "-fno-asm", "-Wall",
			"-lm", "--static", "-std=c99", "-DONLINE_JUDGE", NULL };
	const char * CP_X[] = { "g++", "Main.cc", "-o", "Main", "-fno-asm", "-Wall",
			"-lm", "--static", "-std=c++0x", "-DONLINE_JUDGE", NULL };
	const char * CP_P[] =
			{ "fpc", "Main.pas","-Cs32000000","-Sh", "-O2", "-Co",
				"-Ct", "-Ci", NULL };
	
	pid_t pid;
	pid = fork();
	if(pid == 0){
		chdir("./run");
		struct rlimit LIM;
		LIM.rlim_max = 60;
		LIM.rlim_cur = 60;
		setrlimit(RLIMIT_CPU, &LIM);
		alarm(60);
		LIM.rlim_max = 10 * STD_MB;
		LIM.rlim_cur = 10 * STD_MB;
		setrlimit(RLIMIT_FSIZE, &LIM);
		LIM.rlim_max = STD_MB *256 ;
		LIM.rlim_cur = STD_MB *256 ;
		setrlimit(RLIMIT_AS, &LIM);

		//execute_cmd("chown judger *");
		execute_cmd("mkdir -p bin usr lib lib64 proc");
		execute_cmd("mount -o bind /bin bin");
		execute_cmd("mount -o bind /usr usr");
		execute_cmd("mount -o bind /lib lib");
#ifndef __i386
		execute_cmd("mount -o bind /lib64 lib64");
#endif
		//execute_cmd("mount -o bind /etc/alternatives etc/alternatives");
		execute_cmd("mount -o bind /proc proc");

		chroot("./");

		while(setgid(1536)!=0) sleep(1);
		while(setuid(1536)!=0) sleep(1);
		while(setresuid(1536, 1536, 1536)!=0) sleep(1);

		if (solution["language"].asInt() != 2) {
			freopen("ce.txt", "w", stderr);
		}else{
			freopen("ce.txt", "w", stdout);
		}

		FILE *fsrc;
		switch(solution["language"].asInt()){
			case 0:
				fsrc = fopen("Main.c", "w");
				break;
			case 1:
				fsrc = fopen("Main.cc", "w");
				break;
			case 2:
				fsrc = fopen("Main.pas", "w");
				break;
			default:
				exit(EXIT_FAILURE);
		}
		fputs(solution["code"].asString().c_str(), fsrc);
		fclose(fsrc);
		
		switch(solution["language"].asInt()){
			case 0:
				execvp(CP_C[0], (char * const *) CP_C);
				break;
			case 1:
				execvp(CP_X[0], (char * const *) CP_X);
				break;
			case 2:
				execvp(CP_P[0], (char * const *) CP_P);
				break;
			default:
				exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}else{
		int status = 0;
		waitpid(pid, &status, 0);

		if(status){
			ce = readFile("./run/ce.txt");
		}else{
			execute_cmd("/bin/cp ./run/Main ./Main");
		}
		
		clean_run_dir();
		if(OJ_DEBUG){
			std::cout<<"COMPILE STATUS:"<<status<<std::endl;
		}
		
		return status;
	}
}

void update_ce(int sid, std::string ce){
	std::map<std::string, std::string> par;
	par["solution_id"] = std::to_string(sid);
	par["ce"] = ce;
	http_post("/judger/update-ce", par);
}
void update_solution(Json::Value solution){
	std::map<std::string, std::string> par;
	par["solution_id"] = std::to_string(solution["id"].asInt());
	par["time_used"] = std::to_string(solution["time_used"].asInt());
	par["memory_used"] = std::to_string(solution["memory_used"].asDouble());
	par["status"] = std::to_string(solution["status"].asInt());
	par["score"] = std::to_string(solution["score"].asInt());
	http_post("/judger/update-solution", par);
}

void run_main(int time_limit, double memory_limit){
	nice(19);
	chdir("./run");
	freopen("./data.in", "r", stdin);
	freopen("./user.out", "w", stdout);
	freopen("./error.out", "w", stderr);
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	chroot("./");
	//change user
	while (setgid(1536) != 0)
		sleep(1);
	while (setuid(1536) != 0)
		sleep(1);
	while (setresuid(1536, 1536, 1536) != 0)
		sleep(1);
	//rlimits:

	//time limit
	struct rlimit LIM;
	LIM.rlim_max = LIM.rlim_cur = time_limit / 1000 + 1;
	setrlimit(RLIMIT_CPU, &LIM);
	alarm(0);
	alarm((time_limit/1000) * 10);

	//file limit
	LIM.rlim_max = STD_F_LIM + STD_MB;
	LIM.rlim_cur = STD_F_LIM;
	setrlimit(RLIMIT_FSIZE, &LIM);

	//proc limit
	LIM.rlim_cur = LIM.rlim_max = 1;
	setrlimit(RLIMIT_NPROC, &LIM);

	// set the stack
	LIM.rlim_cur = STD_MB << 6;
	LIM.rlim_max = STD_MB << 6;
	setrlimit(RLIMIT_STACK, &LIM);

	// set the memory
	LIM.rlim_cur = STD_MB * memory_limit / 2 * 3;
	LIM.rlim_max = STD_MB * memory_limit * 2;
	setrlimit(RLIMIT_AS, &LIM);

	//execute
	execl("./Main", "./Main", (char *) NULL);
	fflush(stderr);
	exit(0);
}

void watch_main(pid_t pidApp, Json::Value problem){
}

void run_testcase(Json::Value solution, Json::Value problem,
                  int time_limit, double memory_limit,
                  std::string data_dir, std::string testcase){
	execute_cmd("/bin/cp ./Main ./run/Main");
	execute_cmd("/bin/cp %s/%s.in ./run/data.in",
	            data_dir.c_str(), testcase.c_str());
	pid_t pidApp = fork();
	if(pidApp == 0){
		run_main(time_limit, memory_limit);
	}else{
		watch_main(pidApp, problem);
	}
}

void run_solution(Json::Value solution, Json::Value problem,
                  int time_limit, double memory_limit){
	if(OJ_DEBUG){
		std::cout<<"running solution"<<std::endl;
	}
	DIR *dp;
	dirent *dirp;
	std::string data_dir(OJ_HOME);
	data_dir += "/data/" + std::to_string(problem["id"].asInt());
	dp = opendir(data_dir.c_str());

	while((dirp = readdir(dp)) != NULL){
		// check if the file is *.in or not
		int namelen = isInFile(dirp->d_name);
		if(namelen == 0) continue;
		
		std::string testcase_name(dirp->d_name);
		testcase_name = testcase_name.substr(0,namelen);
		if(OJ_DEBUG){
			std::cout<<"found testcase:"<<testcase_name<<std::endl;
		}
		run_testcase(solution, problem,
		             time_limit, memory_limit,
		             data_dir, testcase_name);
	}
}

void judge_solution(int sid, int rid){
	if(OJ_DEBUG){
		std::cout<<"judging solution "<<sid<<" with run"<<rid<<std::endl;
	}
	set_rlimits();
	if(!checkout(sid)){
		return;
	}
	set_workdir(rid);

	/* Get solution and problem*/
	Json::Value solution, problem;
	solution = get_solution(sid);
	if(sid) problem = get_problem(solution["problem_id"].asInt());

	/* Time limit and mem limit*/
	int time_limit = 1000;
	double mem_limit = 256.00;
	if(sid){
		time_limit = problem["timelimit"].asInt();
		mem_limit = problem["memorylimit"].asDouble();
	}
	if(time_limit > MAX_TIME_LIMIT)
		time_limit = MAX_TIME_LIMIT;
	if(mem_limit > MAX_MEM_LIMIT)
		mem_limit = MAX_MEM_LIMIT;

	/*compile*/
	std::string ce;
	if(compile(solution, problem, ce)){
		if(OJ_DEBUG){
			std::cout<<"compile error!"<<std::endl;
		}
		update_ce(sid ,ce);
		solution["status"] = SL_JUDGED;
		update_solution(solution);
		return;
	}

	/*run solution*/
	run_solution(solution, problem, time_limit, mem_limit);
	
	std::cerr<<"done "<<sid<<std::endl;
}