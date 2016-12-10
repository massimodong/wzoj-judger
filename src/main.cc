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
const char *OJ_USERNAME = NULL;
const char *OJ_PASSWORD = NULL;
int OJ_MAXRUNNING = 3;
int OJ_SLEEPTIME = 10;

/**
 * parameters
 */
int OJ_DEBUG = false;
char *OJ_PROGRAMNAME=NULL;
char *OJ_HOME=NULL;
bool OJ_ONCE = false;
bool OJ_ALLOWADMIN = false;

static const option longopts[] =
{
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { "debug", no_argument, NULL, 'd' },
  { "cd", required_argument, NULL, 'c'},
  { "once", no_argument, NULL, 'a'},
  { "allow-admin", no_argument, NULL, 'm'},
  { NULL, 0, NULL, 0 } 
};

bool already_running();
void call_for_exit(int);
void daemon_init();
void init_config();

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
			case 'm':
				OJ_ALLOWADMIN = true;
				std::cout<<"WARNING: allowing admin users"<<std::endl;
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

	if(!OJ_DEBUG){
		daemon_init();
		system("/sbin/iptables -A OUTPUT -m owner --uid-owner judger -j DROP");
	}

	signal(SIGQUIT, call_for_exit);
    signal(SIGKILL, call_for_exit);
    signal(SIGTERM, call_for_exit);
	
	init_config();
	init_http();

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

end:
	return 0;
}

void config_read_str(const char *&cfg,const char *idx){
	char *buffer;
	cfg = jsonConfigValue[idx].asString().c_str();
	buffer = (char *)malloc(strlen(cfg)+1);
	strcpy(buffer, cfg);
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
	config_read_str(OJ_USERNAME, "username");
	config_read_str(OJ_PASSWORD, "password");
	OJ_SLEEPTIME = jsonConfigValue["sleep_time"].asInt();
	OJ_MAXRUNNING = jsonConfigValue["max_running"].asInt();

	OJ_MAXRUNNING = std::min(OJ_MAXRUNNING , 100);
	
	if(OJ_DEBUG){
		std::cout<<"OJ_URL:"<<OJ_URL<<std::endl
			<<"OJ_USERNAME:"<<OJ_USERNAME<<std::endl
			<<"OJ_PASSWORD:"<<OJ_PASSWORD<<std::endl
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

void call_for_exit(int s){
}