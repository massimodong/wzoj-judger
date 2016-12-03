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

char OJ_HOME_DEFAULT[] = "/home/judge";
char TEMP_USERNAME[]="judger";
char TEMP_PASSWORD[]="123456";
char TEMP_URL[]="localhost/";

#include <wzoj-judger.h>

/**
 * read from configure file
 */
char *OJ_URL = TEMP_URL;
char *OJ_USERNAME = TEMP_USERNAME;
char *OJ_PASSWORD = TEMP_PASSWORD;
bool OJ_ONCE = false;
int OJ_SLEEPTIME = 10;

/**
 * parameters
 */
int OJ_DEBUG = false;
char *OJ_PROGRAMNAME=NULL;
char *OJ_HOME=NULL;

static const option longopts[] =
{
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { "debug", no_argument, NULL, 'd' },
  { "cd", required_argument, NULL, 'c'},
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
		system("/sbin/iptables -A OUTPUT -m owner --uid-owner judge -j DROP");
	}

	init_config();

	signal(SIGQUIT, call_for_exit);
    signal(SIGKILL, call_for_exit);
    signal(SIGTERM, call_for_exit);

	bool flag=true;
	while(true){
		while(flag){
			flag = daemon_work();
			if(OJ_ONCE){
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

void init_config(){
}

void daemon_init(){
}

bool already_running(){
	return false;
}

void call_for_exit(int s){
}