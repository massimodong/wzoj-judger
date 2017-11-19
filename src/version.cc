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

const char version[] = "0.1";

void print_version(){
	std::cout<<"wzoj-judger (WZOJ) "<<version<<std::endl;
	std::cout<<"Copyright (C) 2017 Free Software Foundation, Inc.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";
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
	-d, --debug           debug mode\n\
	-c, --cd=PATH         change judger's home directory\n\
	-a, --once            exit after first solution\n\
	-s, --solution=sid    judge sid and exit\n",
	      stdout);

	puts("");
	fputs("Report bugs to: dongmassimo@gmail.com\n\
pkg home page: <https://github.com/massimodong/wzoj-judger>\n\
General help using GNU software: <http://www.gnu.org/gethelp/>\n"
	      ,stdout);
}