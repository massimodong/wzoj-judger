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
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <stdarg.h>

#define STD_MB 1048576
#define STD_F_LIM (STD_MB<<5)
#define BUFFER_SIZE 5120

#ifdef __i386
#include <okcalls32.h>
#define REG_SYSCALL orig_eax
#define REG_RET eax
#define REG_ARG0 ebx
#define REG_ARG1 ecx
#else
#include <okcalls64.h>
#define REG_SYSCALL orig_rax
#define REG_RET rax
#define REG_ARG0 rdi
#define REG_ARG1 rsi
#endif

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

long get_file_size(const char * filename) {
        struct stat f_stat;

        if (stat(filename, &f_stat) == -1) {
                return 0;
        }

        return (long) f_stat.st_size;
}

void print_runtimeerror(char * err) {
        FILE *ferr = fopen("./error.out", "a+");
        fprintf(ferr, "Runtime Error:%s\n", err);
        fclose(ferr);
}

int compare_files(const char *file1, const char *file2,
                   std::string &verdict){
	int ret = OJ_AC , pe_space = 0 , pe_n = 0;
	FILE * f1, *f2;
	char c1,c2;
	f1 = fopen(file1, "re");
	f2 = fopen(file2, "re");
	if (!f1 || !f2){
		ret = OJ_RE;
		goto wzoj_end;
	}
	c1 = fgetc(f1);
	c2 = fgetc(f2);

	while(true){
		if(c1 == EOF){
			while(c2 != EOF){
				if(!isspace(c2)){
					ret = OJ_WA;
					goto wzoj_end;
				}
				c2 = fgetc(f2);
			}
			goto wzoj_end;
		}
		if(c2 == EOF){
			while(c1 != EOF){
				if(!isspace(c1)){
					ret = OJ_WA;
					goto wzoj_end;
				}
				c1 = fgetc(f1);
			}
			goto wzoj_end;
		}

		if(c1 == '\r' && c2 == '\r'){
		}else if(c1 == '\r' && c2 == '\n'){
			c1 = fgetc(f1);
			if(c1 != '\n'){
				pe_space = 1;
			}
			continue;
		}else if(c1 == '\r' && isspace(c2)){
			pe_space = 1;
		}else if(c1 == '\r'){
			pe_space = 1;
			c1 = fgetc(f1);
			continue;
		}else if(c1 == '\n' && c2 == '\r'){
			c2 = fgetc(f2);
			if(c2 != '\n'){
				pe_space = 1;
			}
			continue;
		}else if(c1 == '\n' && c2 == '\n'){
			pe_space = 0;
		}else if(c1 == '\n' && isspace(c2)){
			pe_space = 1;
			c2 = fgetc(f2);
			continue;
		}else if(c1 == '\n'){
			c1 = fgetc(f1);
			pe_n = 1;
		}else if(isspace(c1) && c2 == '\r'){
			pe_space=1;
		}else if(isspace(c1) && c2 == '\n'){
			pe_space = 1;
			c1 = fgetc(f1);
			continue;
		}else if(isspace(c1) && isspace(c2)){
			if(c1 != c2) pe_space=1;
		}else if(isspace(c1)){
			pe_space = 1;
			c1 = fgetc(f1);
			continue;
		}else if(c2 == '\r'){
			pe_space = 1;
			c2 = fgetc(f2);
			continue;
		}else if(c2 == '\n'){
			pe_n = 1;
			c2 = fgetc(f2);
			continue;
		}else if(isspace(c2)){
			pe_space = 1;
			c2 = fgetc(f2);
			continue;
		}else{
			if(pe_space || pe_n){
				ret = OJ_PE;
			}
			if(c1 != c2){
				ret = OJ_WA;
				goto wzoj_end;
			}
		}
		c1 = fgetc(f1);
		c2 = fgetc(f2);
	}

wzoj_end:
	if(f1) fclose(f1);
	if(f2) fclose(f2);
	/*  <todo>
	if (ret == OJ_WA || ret==OJ_PE){
		if(full_diff)
			make_diff_out_full(f1, f2, c1, c2, file1);
		else
			make_diff_out_simple(f1, f2, c1, c2, file1);
	}*/
	switch(ret){
		case OJ_AC:
			verdict = "AC";
			break;
		case OJ_WA:
			verdict = "WA";
			break;
		case OJ_PE:
			verdict = "PE";
			break;
	}
	return ret;
}

int get_proc_status(int pid, const char * mark) {
        FILE * pf;
        char fn[BUFFER_SIZE], buf[BUFFER_SIZE];
        int ret = 0; 
        sprintf(fn, "/proc/%d/status", pid);
        pf = fopen(fn, "re");
        int m = strlen(mark);
        while (pf && fgets(buf, BUFFER_SIZE - 1, pf)) {

                buf[strlen(buf) - 1] = 0; 
                if (strncmp(buf, mark, m) == 0) { 
                        sscanf(buf + m + 1, "%d", &ret);
                }
        }
        if (pf) 
                fclose(pf);
        return ret; 
}


const int  CALL_ARRAY_SIZE = 512;
bool ALLOWED_CALLS[CALL_ARRAY_SIZE];
void init_syscalls_limits(int lang){
	if(OJ_DEBUG){
		std::cout<<"init syscalls for "<<lang<<std::endl;
	}
	memset(ALLOWED_CALLS, 0, sizeof(ALLOWED_CALLS));
	if(lang <= 1){ //C & C++
		for(int i=0;i==0 || LANG_CV[i];++i){
			ALLOWED_CALLS[LANG_CV[i]] = true;
		}
	}else if(lang == 2){ //Pascal
		for(int i=0;i==0 || LANG_PV[i];++i){
			ALLOWED_CALLS[LANG_PV[i]] = true;
		}
	}else if(lang == 4){//Python
		for(int i=0;i==0 || LANG_YV[i];++i){
			ALLOWED_CALLS[LANG_YV[i]] = true;
		}
	}
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
	//std::cout<<val["ok"].asString()<<std::endl;
	return val["ok"].asBool();
}

void set_compile_workdir(std::string workdir){	
	mkdir((workdir + "/compile").c_str(), 0700);
	chown((workdir + "/compile").c_str(), JUDGER_UID, JUDGER_UID);
	chdir((workdir + "/compile").c_str());
	execute_cmd("mkdir -p bin usr lib lib64 proc etc/alternatives");
	execute_cmd("chown judger etc");
	execute_cmd("mount -o bind /bin bin");
	execute_cmd("mount -o bind /usr usr");
	execute_cmd("mount -o bind /lib lib");
#ifndef __i386
	execute_cmd("mount -o bind /lib64 lib64");
#endif
	execute_cmd("mount -o bind /etc/alternatives etc/alternatives");
	execute_cmd("mount -o bind /proc proc");
}

void set_standalone_workdir(std::string workdir){
	mkdir((workdir + "/standalone").c_str(), 0700);
	chown((workdir + "/standalone").c_str(), JUDGER_UID, JUDGER_UID);
}

void set_python_workdir(std::string workdir){
	mkdir((workdir + "/python").c_str(), 0700);
	chown((workdir + "/python").c_str(), JUDGER_UID, JUDGER_UID);
	chdir((workdir + "/python").c_str());
	execute_cmd("mkdir -p lib lib64 usr etc/alternatives dev");
	execute_cmd("chown judger etc &>/dev/null");
	execute_cmd("mount --bind /lib lib &>/dev/null");
	execute_cmd("mount --bind /lib64 lib64 &>/dev/null");
	execute_cmd("mount --bind /usr usr &>/dev/null");
	execute_cmd("mount --bind /etc/alternatives etc/alternatives &>/dev/null");
	execute_cmd("mknod -m 0444 dev/urandom c 1 9 &>/dev/null");
}

void set_workdir(int rid){
	std::string workdir = std::string(OJ_HOME) + "/run" + std::to_string(rid);
	struct stat st = {0};
	if(stat(workdir.c_str(), &st) == -1) {
		mkdir(workdir.c_str(), 0700);
		chown(workdir.c_str(), JUDGER_UID, JUDGER_UID);
	
		set_compile_workdir(workdir);
		
		set_standalone_workdir(workdir);
		
		set_python_workdir(workdir);
	}
	chdir(workdir.c_str());
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
	const char * CP_PY[] = {"/bin/cp", "Main.py", "Main", NULL};
	
	pid_t pid;
	pid = fork();
	if(pid == 0){
		chdir("./compile");
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
		
		if(problem["type"].asInt() == 2){//copy interact files
			std::string data_dir(OJ_HOME);
			data_dir += "/data/" + std::to_string(problem["id"].asInt());
			execute_cmd (("/bin/cp " + data_dir + "/interact.h ./").c_str());
			execute_cmd (("/bin/cp " + data_dir + "/interact.pas ./").c_str());
			execute_cmd (("/bin/cp " + data_dir + 
			              "/interact_main.pas ./Main.pas").c_str());
		}

		chroot("./");

		while(setgid(JUDGER_UID)!=0) sleep(1);
		while(setuid(JUDGER_UID)!=0) sleep(1);
		while(setresuid(JUDGER_UID, JUDGER_UID, JUDGER_UID)!=0) sleep(1);

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
				if(problem["type"].asInt() == 2){
					fsrc = fopen("solution.pas", "w");
				}else{
					fsrc = fopen("Main.pas", "w");
				}
				break;
			case 4:
				fsrc = fopen("Main.py", "w");
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
			case 4:
				execvp(CP_PY[0], (char * const *) CP_PY);
				break;
			default:
				exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}else{
		int status = 0;
		waitpid(pid, &status, 0);

		if(status){
			ce = readFile("./compile/ce.txt");
		}else{
			execute_cmd("/bin/mv ./compile/Main ./Main");
		}
		//remove files
		if(problem["type"].asInt() == 2){//rm interact files
			execute_cmd ("/bin/rm ./compile/interact.h");
			execute_cmd ("/bin/rm ./compile/interact.pas");
			execute_cmd ("/bin/rm ./compile/Main.pas");
		}
		execute_cmd ("/bin/rm ./compile/ce.txt");
		switch(solution["language"].asInt()){
			case 0:
				execute_cmd ("/bin/rm ./compile/Main.c");
				break;
			case 1:
				execute_cmd ("/bin/rm ./compile/Main.cc");
				break;
			case 2:
				if(problem["type"].asInt() == 2){
					execute_cmd ("/bin/rm ./compile/solution.* ./compile/interact.* ./compile/Main.*");
				}else{
					execute_cmd ("/bin/rm ./compile/Main.*");
				}
				break;
			case 4:
				execute_cmd ("/bin/rm ./compile/Main.py");
				break;
			default:
				exit(EXIT_FAILURE);
		}
		if(OJ_DEBUG){
			std::cout<<"COMPILE STATUS:"<<status<<std::endl;
		}
		
		return status;
	}
}

bool compile_spj(Json::Value problem){
	const char * CP_X[] = { "g++", "spj.cc", "-o", "spj", "-fno-asm", "-Wall",
			"-lm", "--static", "-std=c++0x", NULL };
	
	pid_t pid;
	pid = fork();
	if(pid == 0){
		chdir("./compile");
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

		//copy spj.cc
		std::string data_dir(OJ_HOME);
		data_dir += "/data/" + std::to_string(problem["id"].asInt());
		execute_cmd("/bin/cp %s/spj.cc ./spj.cc", data_dir.c_str());

		chroot("./");

		while(setgid(JUDGER_UID)!=0) sleep(1);
		while(setuid(JUDGER_UID)!=0) sleep(1);
		while(setresuid(JUDGER_UID, JUDGER_UID, JUDGER_UID)!=0) sleep(1);

		freopen("ce.txt", "w", stderr);
		
		execvp(CP_X[0], (char * const *) CP_X);
	}else{
		int status = 0;
		waitpid(pid, &status, 0);

		if(status){
			std::cerr<<"compile error!"<<std::endl;
		}else{
			execute_cmd("/bin/mv ./compile/spj ./spj");
		}
		execute_cmd ("/bin/rm ./compile/spj.cc");
		execute_cmd ("/bin/rm ./compile/ce.txt");
		
		if(OJ_DEBUG){
			std::cout<<"SPJ COMPILE STATUS:"<<status<<std::endl;
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
	par["cnt_testcases"] =std::to_string(solution["cnt_testcases"].asInt());
	http_post("/judger/update-solution", par);
}

void run_main(int time_limit, double memory_limit,int language){
	nice(-20);
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);

	chroot("./");
	//change user
	while (setgid(JUDGER_UID) != 0)
		sleep(1);
	while (setuid(JUDGER_UID) != 0)
		sleep(1);
	while (setresuid(JUDGER_UID, JUDGER_UID, JUDGER_UID) != 0)
		sleep(1);
	
	//rlimits:

	//time limit
	struct rlimit LIM;
	LIM.rlim_max = LIM.rlim_cur = time_limit / 1000 + 1;
	setrlimit(RLIMIT_CPU, &LIM);
	alarm(0);
	alarm((5 * time_limit/1000) + 1);

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
	for(int i=10;i;--i){
		switch(language){
			case 0: //C
			case 1: //C++
			case 2: //Pascal
				execl("./Main", "./Main", (char *) NULL);
				break;
			case 4: //python
				execl("/usr/bin/python", "/usr/bin/python", "Main", (char *) NULL);
				break;
		}
		sleep(1);
	}
	fflush(stderr);
	exit(0);
}
void run_spj(){
		nice(-20);
		execute_cmd("/bin/cp ../spj ./spj");

		chroot("./");
		//change user
		while (setgid(JUDGER_UID) != 0)
			sleep(1);
		while (setuid(JUDGER_UID) != 0)
			sleep(1);
		while (setresuid(JUDGER_UID, JUDGER_UID, JUDGER_UID) != 0)
			sleep(1);
	
		//rlimits:

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
	
		//execute
		execl("./spj", "./spj", "./data.in", "user.out", "data.ans",
		   "verdict.out", "score.out", "checklog.out", (char *) NULL);
		exit(0);
}

bool watch_main(pid_t pidApp, Json::Value problem,
                int time_limit, double memory_limit,
                int &time_used, double &memory_used, std::string &verdict
                ){
	if(OJ_DEBUG){
		std::cout<<"watch solution "<<pidApp<<std::endl;
	}
	bool success = true;
	int tempmemory;
	int status, sig, exitcode;
	bool *allowed_calls = ALLOWED_CALLS;
	user_regs_struct reg;
	rusage ruse;
	memory_used = get_proc_status(pidApp, "VmRSS:") << 10;
	while(true){
		wait4(pidApp, &status, 0, &ruse);

		//check memory
		tempmemory = get_proc_status(pidApp, "VmPeak:") << 10;
		if(tempmemory > memory_used){
			memory_used = tempmemory;
			if(memory_used > memory_limit*STD_MB){
				verdict = "MLE";
				ptrace(PTRACE_KILL, pidApp, NULL, NULL);
				success = false;
				break;
			}
		}

		//check runtime error
		if(WIFEXITED(status)){
			break;
		}
		if(get_file_size("./error.out")){
			verdict = "RE";
			ptrace(PTRACE_KILL, pidApp, NULL, NULL);
			success = false;
			break;
		}
		exitcode = WEXITSTATUS(status);
		if(exitcode == 0x05 || exitcode == 0){
			//go on;
			;
		}else{
			switch(exitcode){
				case SIGCHLD:
				case SIGALRM:
					alarm(0);
				case SIGKILL:
				case SIGXCPU:
					verdict = "TLE";
					break;
				case SIGXFSZ:
					verdict = "OLE";
					break;
				default:
					verdict = "RE";
			}
			print_runtimeerror(strsignal(exitcode));
			success = false;
			ptrace(PTRACE_KILL, pidApp, NULL, NULL);
			break;
		}
		if(WIFSIGNALED(status)){
			sig = WTERMSIG(status);
			switch(sig){
				case SIGCHLD:
				case SIGALRM:
					alarm(0);
				case SIGKILL:
				case SIGXCPU:
					verdict = "TLE";
					break;
				case SIGXFSZ:
					verdict = "OLE";
					break;
				default:
					verdict = "RE";
			}
			print_runtimeerror(strsignal(exitcode));
			success = false;
			break;
		}

		//check the system calls
		ptrace(PTRACE_GETREGS, pidApp, NULL, &reg);
		if(allowed_calls[reg.REG_SYSCALL]){
		}else{
			verdict = "RE";
			success = false;
			char error[BUFFER_SIZE];
			sprintf(error,
			        "[ERROR] A Not allowed system call:%ld\n"
					" TO FIX THIS , ask admin to add the CALLID into"
					"corresponding LANG_XXV[] located at okcalls32/64.h ,"
					"and recompile judger. \n",
			        (long)reg.REG_SYSCALL);
			print_runtimeerror(error);
			ptrace(PTRACE_KILL, pidApp, NULL, NULL);
		}
		ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);
	}
	time_used = (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000);
	time_used += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);
	if(time_used > time_limit){
		verdict = "TLE";
		success = false;
	}
	if(OJ_DEBUG){
		std::cout<<"time_used:"<<time_used<<std::endl
			<<"memory_used:"<<memory_used / STD_MB<<"MB"<<std::endl
			<<"verdict:"<<verdict<<std::endl;
	}
	waitpid(pidApp, NULL, 0);
	return success;
}

void post_testcase(Json::Value testcase){
	std::map<std::string, std::string> par;
	par["solution_id"] = std::to_string(testcase["solution_id"].asInt());
	par["filename"] = testcase["filename"].asString();
	par["time_used"] = std::to_string(testcase["time_used"].asInt());
	par["memory_used"] = std::to_string(testcase["memory_used"].asDouble());
	par["verdict"] = testcase["verdict"].asString();
	par["score"] = std::to_string(testcase["score"].asInt());
	par["checklog"] = testcase["checklog"].asString();
	http_post("judger/post-testcase", par);
}

std::string get_answer(Json::Value solution, std::string filename){
	std::map<std::string, std::string> par;
	par["solution_id"] = std::to_string(solution["id"].asInt());
	par["filename"] = filename;
	Json::Value res = http_get("judger/get-answer", par);
	return res["answer"].asString();
}

void run_testcase(Json::Value &solution, Json::Value problem,
                  int time_limit, double memory_limit,
                  std::string data_dir, std::string testcase_name){
	Json::Value testcase;
	int time_used;
	double memory_used;
	std::string verdict;
	pid_t spj_pid;
	int spj_main[2],main_spj[2];

	int language = solution["language"].asInt();
	if(language == 4){//python
		chdir("./python");
	}else{
		chdir("./standalone");
	}
	
	execute_cmd("/bin/cp %s/%s.in ./data.in",
	            data_dir.c_str(), testcase_name.c_str());
	testcase["solution_id"] = solution["id"].asInt();
	testcase["filename"] = testcase_name;

	int problemType = problem["type"].asInt();

	if(problemType != 3){//execute solution if needed
		execute_cmd("/bin/cp ../Main ./Main");

		if(problemType == 2){//create pipe & run spj first
			while(pipe(spj_main) == -1) sleep(3);
			while(pipe(main_spj) == -1) sleep(3);
			
			spj_pid = fork();
			if(spj_pid == 0){
				dup2(main_spj[0],STDIN_FILENO);
				dup2(spj_main[1],STDOUT_FILENO);
				run_spj();
				exit(EXIT_SUCCESS);
			}
		}
		
		pid_t pidApp = fork();
		if(pidApp == 0){
			if(problemType == 1){
				freopen("./data.in", "r", stdin);
				freopen("./user.out", "w", stdout);
				freopen("./error.out", "w", stderr);
			}else{
				dup2(spj_main[0],STDIN_FILENO);
				dup2(main_spj[1],STDOUT_FILENO);
				freopen("./error.out", "w", stderr);
			}
			run_main(time_limit, memory_limit, solution["language"].asInt());
			exit(EXIT_SUCCESS);
		}
		bool success = watch_main(pidApp, problem, time_limit, memory_limit,
		             time_used, memory_used, verdict);
		testcase["time_used"] = time_used;
		testcase["memory_used"] = memory_used;
		if(time_used > solution["time_used"].asInt()){
			solution["time_used"] = time_used;
		}
		if(memory_used > solution["memory_used"].asDouble()){
			solution["memory_used"] = memory_used;
		}
		if(!success){
			testcase["verdict"] = verdict;
			testcase["score"] = 0;
			post_testcase(testcase);
			if(problemType == 2){
				kill(spj_pid, SIGKILL);
			}
			return;
		}
	}else if(problemType == 3){//generate .out file
		std::string answer = get_answer(solution, testcase_name);
		if(OJ_DEBUG){
			std::cout<<"answer:<"<<answer<<">"<<std::endl;
		}
		
		FILE *answerfile = fopen("./user.out", "w");
		fputs(answer.c_str(), answerfile);
		fclose(answerfile);
	}

	//compare & complete testcase
	execute_cmd("/bin/cp %s/%s.ans ./data.ans",
        data_dir.c_str(), testcase_name.c_str());
	execute_cmd("/bin/cp %s/%s.out ./data.ans",
        data_dir.c_str(), testcase_name.c_str());
	if(problem["spj"].asBool() || problemType == 2){
		int score=0;

		if(problemType != 2){
			spj_pid = fork();
			if(spj_pid == 0){
				run_spj();
				exit(EXIT_SUCCESS);
			}
		}
		int status = 0;
		waitpid(spj_pid, &status, 0);
		
		sscanf(readFile("./score.out").c_str(), "%d", &score);
			
		testcase["verdict"] = readFile("./verdict.out");
		testcase["score"] = score;
		testcase["checklog"] = readFile("./checklog.out");

		execute_cmd ("/bin/rm Main spj data.in user.out error.out data.ans verdict.out score.out checklog.out");
		
		if(OJ_DEBUG){
			std::cout<<"verdict:"<<testcase["verdict"].asString()<<std::endl
				<<"score:"<<testcase["score"].asInt()<<std::endl
				<<"checklog:"<<testcase["checklog"].asString()<<std::endl;
		}
	}else{
		int flag = compare_files("./data.ans",
		                         "./user.out", verdict);
		testcase["verdict"] = verdict;
		if(flag == OJ_AC){
			testcase["score"] = 100;
		}else{
			testcase["score"] = 0;
		}
		execute_cmd ("/bin/rm Main data.in user.out data.ans error.out");
	}

	chdir("../");
		
	post_testcase(testcase);
	solution["score"] = solution["score"].asInt() + testcase["score"].asInt();
}

class comp_test_name{
	public:
	bool operator()(const std::string &a, const std::string &b){
		if(a.length() != b.length()){
			return a.length() < b.length();
		}else{
			return a < b;
		}
	}
};
/*
bool comp_test_name(std::string a,std::string b){
}*/

void run_solution(Json::Value &solution, Json::Value problem,
                  int time_limit, double memory_limit){
	if(OJ_DEBUG){
		std::cout<<"running solution "<<solution["id"].asInt()<<std::endl;
	}
	DIR *dp;
	dirent *dirp;
	std::string data_dir(OJ_HOME);
	data_dir += "/data/" + std::to_string(problem["id"].asInt());
	dp = opendir(data_dir.c_str());

	init_syscalls_limits(solution["language"].asInt());

	std::set<std::string, comp_test_name> test_names;
	while((dirp = readdir(dp)) != NULL){
		// check if the file is *.in or not
		// if yes, return name length
		// otherwise, return 0
		int namelen = isInFile(dirp->d_name);
		if(namelen == 0) continue;
		
		std::string testcase_name(dirp->d_name);
		testcase_name = testcase_name.substr(0,namelen);

		test_names.insert(testcase_name);
	}

	solution["cnt_testcases"] = test_names.size();
	solution["status"] = SL_RUNNING;
	update_solution(solution);

	for(auto const &t: test_names){
		run_testcase(solution, problem,
		             time_limit, memory_limit,
		             data_dir, t);
	}

	/*
	while((dirp = readdir(dp)) != NULL){
		if(OJ_DEBUG){
			std::cout<<"found testcase:"<<testcase_name<<std::endl;
		}
		run_testcase(solution, problem,
		             time_limit, memory_limit,
		             data_dir, testcase_name);
	}*/
}

void finishJudging(Json::Value &solution){
	int cnt_testcases = solution["cnt_testcases"].asInt();
	if(cnt_testcases > 0){
		solution["score"] = solution["score"].asInt() / cnt_testcases;
	}else{
		solution["score"] = 0;
	}
	update_solution(solution);
	std::map<std::string, std::string> par;
	par["solution_id"] = std::to_string(solution["id"].asInt());
	http_post("/judger/finish-judging", par);
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
	if(problem["type"].asInt() != 3){//compile if is not "submit-answer prolem"
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
	}

	/*compile spj*/
	if(problem["spj"].asBool() || problem["type"].asInt() == 2){
		compile_spj(problem);
	}

	/*run solution  (even if it is "submit-answer problem")*/
	run_solution(solution, problem, time_limit, mem_limit);

	//clean up
	if(problem["type"].asInt() != 3){
		execute_cmd("/bin/rm ./Main");
	}
	if(problem["spj"].asBool() || problem["type"].asInt() == 2){
		execute_cmd("/bin/rm ./spj");
	}

	finishJudging(solution);
}