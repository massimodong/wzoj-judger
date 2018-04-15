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

Json::Value raw_post(std::string,std::string,bool isPost);
Json::Value http_get(std::string, std::map<std::string, std::string>);
Json::Value http_post(std::string, std::map<std::string, std::string>);

void init_http(){
	/*remove old cookie file*/
	std::string cookie_path = OJ_HOME + std::string("/cookie");
	std::remove(cookie_path.c_str());

	/*initiate*/
	curl_global_init(CURL_GLOBAL_ALL);
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp){
	//std::cout<<(char *)buffer<<std::endl;
	size_t realsize = size * nmemb;
	(*((std::string *)userp)) += (char *)buffer;
	return realsize;
}

/**
 * Perform a get or post request
 * 
 * url: request url
 * data: data to be sent
 * isPost: post or get
 **/
Json::Value raw_post(std::string url,
                     std::string data,
                     bool isPost){
	int code;
	Json::Reader reader;
	std::string *ret_str;
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
		ret_str = new std::string;

		/**
		 * Perform as an ajax request
		 **/
		struct curl_slist *chunk = NULL;
		chunk = curl_slist_append(chunk, "X-Requested-With: XMLHttpRequest");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

		/**
		 * url and data
		 **/
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		if(isPost){
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS,data.c_str());
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
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)ret_str);

		curl_easy_perform(curl);
		reader.parse(ret_str->c_str(), ret);
		delete ret_str;

		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &code);

		curl_slist_free_all(chunk);
		curl_easy_cleanup(curl);

		switch(code){
			case 500:
			case 501:
			case 502:
			case 503:
				sleep(10);
				break;
			case 401:
				sleep(1);
				break;
			case 302:
			case 200:
				/*succeed*/
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
	url = OJ_URL + url + "?judger_token=" + OJ_TOKEN;
	//std::cerr<<url.c_str();
	
	for(auto const &p: par){
		char *value = curl_easy_escape(curl, p.second.c_str(), 0);
		url += "&" + p.first + "=" + value;
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
	url = OJ_URL + url + "?judger_token=" + OJ_TOKEN;
	//std::cerr<<url.c_str();
	if(OJ_DEBUG){
		std::cout<<"post url:"<<url<<std::endl;
	}
	
	std::string data;
	bool first_param = true;
	for(auto const &p: par){
		char *value = curl_easy_escape(curl, p.second.c_str(), 0);
		if(first_param){
			data += p.first + "=" + value;
			first_param = false;
		}else{
			data += "&" + p.first + "=" + value;
		}
		curl_free(value);
	}
	if(OJ_DEBUG){
		std::cout<<"post data:"<<data<<std::endl;
	}

	curl_easy_cleanup(curl);
	return raw_post(url,data,true);
}