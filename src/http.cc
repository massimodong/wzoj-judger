// http.cc
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

std::string HTTP_TOKEN;

Json::Value raw_post(std::string url,
                     std::map<std::string,std::string> par){
}

bool http_check_auth(){
}

void http_login(){
	if(http_check_auth()) return;
	std::map<std::string,std::string> par;
	par["user"] = std::string(OJ_USERNAME);
	par["password"] = std::string(OJ_PASSWORD);
	Json::Value ret = raw_post(std::string("login"),par);
}

Json::Value http_post(std::string url,
                      std::map<std::string,std::string> par){
	http_login();
	return raw_post(url,par);
}