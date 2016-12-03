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
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <locale.h>
#include <json/json.h>
#include <vector>
#include <map>


extern int OJ_DEBUG;
extern char *OJ_PROGRAMNAME;
extern char *OJ_HOME;

extern char *OJ_URL;
extern char *OJ_USERNAME;
extern char *OJ_PASSWORD;
extern bool OJ_ONCE;
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
Json::Value http_post(std::string, std::map<std::string, std::string>);