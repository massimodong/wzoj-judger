// version.cc
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

void print_version(){
	std::cout<<"wzoj-judger (WZOJ) 0.1"<<std::endl;
}

void print_help(){
	printf("Usage: %s [OPTION]...\n",OJ_PROGRAMNAME);
	printf("Judger for WZMS ONLINE JUDGE\n");
	puts("");
	fputs("\
	-h, --help        display this help and exit\n\
	-v, --version     display version information and exit\n",
	      stdout);
	puts("");
	fputs("\
	--debug           run in debug mode\n\
	--cd=PATH         change judger's home directory\n",
	      stdout);
}