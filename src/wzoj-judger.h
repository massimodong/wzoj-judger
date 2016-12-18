/*
 * wzoj-judger.h
 *
 * Copyright (C) 2016 - Unknown
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <sys/wait.h>
#include <locale.h>
#include <json/json.h>
#include <vector>
#include <map>

const uid_t JUDGER_UID = 1536;
const int MAX_TIME_LIMIT = 60000; // 1min
const double MAX_MEM_LIMIT = 2048.00; // 2GB

const int SL_PENDING = 0;
const int SL_PENDING_REJUDGING = 1;
const int SL_COMPILING = 2;
const int SL_RUNNING = 3;
const int SL_JUDGED = 4;
const int SL_CANCELED = 5;

extern int OJ_DEBUG;
extern char *OJ_PROGRAMNAME;
extern char *OJ_HOME;
extern bool OJ_ONCE;
extern bool OJ_ALLOWADMIN;
extern int OJ_SOLUTION_NO;

extern const char *OJ_URL;
extern const char *OJ_USERNAME;
extern const char *OJ_PASSWORD;
extern int OJ_MAXRUNNING;
extern int OJ_SLEEPTIME;

/**
 * version
 */
void print_version();
void print_help();

/**
 * daemon
 */
bool daemon_work();

/**
 * http
 */
void init_http();
Json::Value http_get(std::string, std::map<std::string, std::string>);
Json::Value http_post(std::string, std::map<std::string, std::string>);

/**
 * judger
 **/
void judge_solution(int,int);