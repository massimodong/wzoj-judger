/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.cc
 * Copyright (C) 2016 Massimo Dong <dongmassimo@gmail.com>
 * 
 * wzoj-judger is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wzoj-judger is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

char OJ_HOME_DEFAULT[] = "/home/judger";

#include <wzoj-judger.h>

/**
 * json Config
 **/
Json::Value jsonConfigValue;

/**
 * read from configure file
 */
const char *OJ_URL = NULL;
const char *OJ_TOKEN = NULL;
int OJ_MAXRUNNING = 3;
int OJ_SLEEPTIME = 10;
bool OJ_SIMCHECK = false;

/**
 * parameters
 */
int OJ_DEBUG = false;
char *OJ_PROGRAMNAME=NULL;
char *OJ_HOME=NULL;
bool OJ_ONCE = false;
int OJ_SOLUTION_NO = -1;

static const option longopts[] =
{
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { "debug", no_argument, NULL, 'd' },
  { "cd", required_argument, NULL, 'c'},
  { "once", no_argument, NULL, 'a'},
  { "solution", required_argument, NULL, 's'},
  { NULL, 0, NULL, 0 } 
};

bool already_running();
void call_for_exit(int);
void daemon_init();
void init_config();
void clean_run_dirs();

int main(int argc,char *argv[])
{
	//atexit(close_stdout);

	OJ_PROGRAMNAME = argv[0];
	
	int optc;
	bool lose = false;
	while((optc = getopt_long(argc,argv,"g:hntv",longopts,NULL)) != -1){
		switch(optc){
			case 'v':
				print_version();
				exit(EXIT_SUCCESS);
			case 'h':
				print_help();
				exit(EXIT_SUCCESS);
			case 'd':
				OJ_DEBUG = true;
				break;
			case 'c':
				OJ_HOME = optarg;
				break;
			case 'a':
				OJ_ONCE = true;
				break;
			case 's':
				sscanf(optarg, "%d", &OJ_SOLUTION_NO);
				break;
			default:
				lose = true;
				break;
		}
	}
	
	if(lose || optind < argc){
		if(optind < argc){
			fprintf(stderr,"%s: extra operand: %s\n",
			        OJ_PROGRAMNAME,argv[optind]);
		}else{
			fprintf(stderr,"Try `%s --help` for more information.\n",
			        OJ_PROGRAMNAME);
		}
		exit(EXIT_FAILURE);
	}

	if(OJ_HOME == NULL) OJ_HOME = OJ_HOME_DEFAULT;

	if(OJ_DEBUG){
		printf("running in debug mode!\n");
	}

	if(OJ_DEBUG){
		printf("home:%s\n",OJ_HOME);
	}

	if(already_running()){
		printf("A judger on %s is already running!\n", OJ_HOME);
		exit(EXIT_FAILURE);
	}

	signal(SIGQUIT, call_for_exit);
    signal(SIGKILL, call_for_exit);
    signal(SIGTERM, call_for_exit);
	signal(SIGINT, call_for_exit);
	
	init_config();
	init_http();

	clean_run_dirs();
	if(OJ_SIMCHECK){
		sim_daemon();
	}
	if(OJ_SOLUTION_NO > 0){
		judge_solution(OJ_SOLUTION_NO, 0);
		call_for_exit (0);
	}

	if(!OJ_DEBUG){
		daemon_init();
	}
	system("/sbin/iptables -A OUTPUT -m owner --uid-owner judger -j DROP");

	if(OJ_ONCE || OJ_SLEEPTIME){
		bool flag=true;
		while(true){
			while(flag){
				flag = daemon_work();
				if(flag && OJ_ONCE){
					goto end;
				}
			}
			if(OJ_DEBUG){
				printf("all tasks finished\n");
			}
			sleep(OJ_SLEEPTIME);
			flag = true;
		}
	}else{
		listen_udp();
	}

end:
	call_for_exit (0);
}

void config_read_str(const char *&cfg,const char *idx){
	char *buffer;
	buffer = (char *)malloc(sizeof(char) *
			(strlen(jsonConfigValue[idx].asString().c_str())+1));
	strcpy(buffer, jsonConfigValue[idx].asString().c_str());
	cfg = buffer;
}

void init_config(){
	if(OJ_DEBUG){
		std::cout<<"init configure"<<std::endl;
	}

	//path of the config file
	std::string config_d("/etc/config.json");
	std::string config_path = OJ_HOME + config_d;

	//open config file
	std::ifstream config(config_path.c_str(),
	                         std::ifstream::binary);

	//read config file
	config>>jsonConfigValue;

	config_read_str(OJ_URL, "url");
	config_read_str(OJ_TOKEN, "token");
	OJ_SLEEPTIME = jsonConfigValue["sleep_time"].asInt();
	OJ_MAXRUNNING = jsonConfigValue["max_running"].asInt();
	OJ_SIMCHECK = jsonConfigValue["sim_check"].asBool();

	OJ_MAXRUNNING = std::min(OJ_MAXRUNNING , 100);
	
	if(OJ_DEBUG){
		std::cout<<"OJ_URL:"<<OJ_URL<<std::endl
			<<"OJ_TOKEN:"<<OJ_TOKEN<<std::endl
			<<"OJ_SLEEPTIME:"<<OJ_SLEEPTIME<<std::endl
			<<"OJ_MAXRUNNING:"<<OJ_MAXRUNNING<<std::endl;
		std::cout<<std::endl;
	}
}

void daemon_init(){
}

bool already_running(){
	return false;
}

void clean_run_dirs(){
	for(int i=0;i<OJ_MAXRUNNING;++i){
		std::string workdir = std::string(OJ_HOME) + "/run" + std::to_string(i);
		struct stat st = {0};
		if(stat(workdir.c_str(), &st) != -1) {
			chdir(workdir.c_str());
			execute_cmd ("umount -l ./compile/bin ./compile/etc/alternatives\
			./compile/lib ./compile/lib64 ./compile/proc ./compile/usr\
			./python/etc/alternatives ./python/lib ./python/lib64 ./python/usr");
			execute_cmd ("rm -Rf ./python/dev");
			chdir("..");
		}
	}
	execute_cmd ("rm -Rf ./run*");
}

void call_for_exit(int s){
	std::cout<<"EXIT JUDGER...";
	clean_run_dirs();
	exit(EXIT_SUCCESS);
}
