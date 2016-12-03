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
	get_jobs(jobs);
	return false;
}

void get_jobs(std::vector<int> &jobs){
	std::map<std::string, std::string> par;
	std::string fa("fa"),fu("fasd");
	http_post(fa,par);
}