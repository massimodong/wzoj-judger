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
#include <curl/curl.h>

std::string HTTP_TOKEN;

Json::Value raw_post(std::string,std::string,bool isPost);
Json::Value http_get(std::string, std::map<std::string, std::string>);
Json::Value http_post(std::string, std::map<std::string, std::string>);

void http_get_token(){
	std::map<std::string, std::string> par;
	Json::Value val = http_get("/ajax/token", par);
	HTTP_TOKEN = val["_token"].asString();
	if(OJ_DEBUG){
		std::cout<<"GET TOKEN::"<<HTTP_TOKEN<<std::endl<<std::endl;;
	}
}

void http_login(){
	std::map<std::string, std::string> par;
	par["name"] = OJ_USERNAME;
	par["password"] = OJ_PASSWORD;
	Json::Value val = http_post("/auth/login", par);
	//std::cout<<val<<std::endl;
}

void init_http(){
	/*remove old cookie file*/
	std::string cookie_path = OJ_HOME + std::string("/cookie");
	std::remove(cookie_path.c_str());

	/*initiate*/
	curl_global_init(CURL_GLOBAL_ALL);
	http_get_token();
	http_login();

	/*check roles*/
	bool isJudger=false,isAdmin=false;
	std::map<std::string, std::string> par;
	Json::Value roles = http_get("/ajax/roles", par);
	for(int i=0;i<roles.size();++i){
		if(OJ_DEBUG){
			std::cout<<"Current user has role "
				<<roles[i]["name"].asString()<<std::endl;
		}
		if(roles[i]["name"].asString() == "judger") isJudger = true;
		if(roles[i]["name"].asString() == "admin") isAdmin = true;
	}
	if(isAdmin) isJudger = true;
	if(isAdmin && !OJ_ALLOWADMIN){
		std::cerr<<"A user with role \"admin\" is not allowed"<<std::endl
			<<"Use --allow-admin.(Not recommended)"<<std::endl;
		exit(EXIT_FAILURE);
	}

	if(!isJudger){
		std::cerr<<"not judger!"<<std::endl;
		exit(EXIT_FAILURE);
	}
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp){
	//std::cout<<(char *)buffer<<std::endl;
	Json::Reader reader;
	size_t realsize = size * nmemb;
	reader.parse((char *)buffer, *((Json::Value *)userp));
	if(OJ_DEBUG){
		std::cout<<(*((Json::Value *)userp))<<std::endl;
	}
	return realsize;
}

/**
 * Perform a get or post request
 * 
 * url: request url
 * data: data to be sent
 * code: http status code
 * isPost: post or get
 **/
Json::Value raw_post(std::string url,
                     std::string data,
                     bool isPost){
	int code;
	Json::Value ret;
	while(true){
		CURL *curl = curl_easy_init();
		if(!curl){
			std::cout<<"curl_easy_init() failed!";
			for(int i=0;i<5;++i){
				sleep(3);
				curl = curl_easy_init();
				if(!curl){
					std::cerr<<"retry failed!"<<std::endl;
				}else{
					std::cerr<<"ok."<<std::endl;
					break;
				}
			}
		}
		if(!curl){
			exit(EXIT_FAILURE);
		}

		/**
		 * Perform as an ajax request
		 **/
		struct curl_slist *chunk = NULL;
		chunk = curl_slist_append(chunk, "X-Requested-With: XMLHttpRequest");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

		/**
		 * url and data
		 **/
		std::string data_with_token = "_token=" + HTTP_TOKEN + data;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		if(isPost){
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS,data_with_token.c_str());
			//std::cerr<<data_with_token.c_str()<<std::endl;
		}

		/**
		 * set cookie
		 **/
		std::string cookie_path = OJ_HOME + std::string("/cookie");
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE,cookie_path.c_str());
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR,cookie_path.c_str()); 
		
		/**
		 * prepare to receive data
		 **/
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&ret);

		curl_easy_perform(curl);

		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &code);

		curl_easy_cleanup(curl);

		switch(code){
			case 500:
				std::cerr<<"invalid token!"<<std::endl;
				http_get_token();
				break;
			case 501:
			case 502:
			case 503:
				sleep(10);
				break;
			case 401:
				std::cerr<<"need login"<<std::endl;
				http_login();
				break;
			case 302:
			case 200:
				/*succeed*/
				//std::cout<<"ok:\n"<<ret<<std::endl;
				if(OJ_DEBUG){
					std::cout<<std::endl;
				}
				return ret;
			default:
				std::cerr<<"unrecognized status code:"<<code<<std::endl;
				sleep(10);
				break;
		}
	}
	if(OJ_DEBUG){
		std::cout<<std::endl;
	}
}

/**
 * perform a http get request
 * 
 * url: the url of the request
 * par: data to be sent
 **/
Json::Value http_get(std::string url,
                     std::map<std::string,std::string> par){
	CURL *curl = curl_easy_init();
	url = OJ_URL + url;
	//std::cerr<<url.c_str();
	
	bool first_param = true;
	for(auto const &p: par){
		char *value = curl_easy_escape(curl, p.second.c_str(), 0);
		if(first_param){
			first_param = false;
			url += "?" + p.first + "=" + value;
		}else{
			url += "&" + p.first + "=" + value;
		}
		curl_free(value);
	}
	if(OJ_DEBUG){
		if(OJ_DEBUG){
			std::cout<<"get url:"<<url<<std::endl;
		}
	}

	curl_easy_cleanup(curl);
	return raw_post(url,std::string(""),false);
}

/**
 * perform a http post request
 * 
 * url: the url of the request
 * par: data to send
 **/
Json::Value http_post(std::string url,
                      std::map<std::string,std::string> par){
	CURL *curl = curl_easy_init();
	url = OJ_URL + url;
	//std::cerr<<url.c_str();
	if(OJ_DEBUG){
		std::cout<<"post url:"<<url<<std::endl;
	}
	
	std::string data;
	for(auto const &p: par){
		char *value = curl_easy_escape(curl, p.second.c_str(), 0);
		data += "&" + p.first + "=" + value;
		curl_free(value);
	}
	if(OJ_DEBUG){
		std::cout<<"post data:"<<data<<std::endl;
	}

	curl_easy_cleanup(curl);
	return raw_post(url,data,true);
}