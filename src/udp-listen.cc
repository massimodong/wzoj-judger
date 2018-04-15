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

#define OJ_UDP_PORT 13107
#define UDP_BUFF_SIZE 2048

#include <wzoj-judger.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

int create_udp_socket(){
	int ret_socket;
	while ((ret_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		std::cerr<<"Failed to create socket, retrying."<<std::endl;
		sleep(5);
	}

	//bind socket
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(OJ_UDP_PORT);
	while (bind(ret_socket, (sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Bind failed, retrying.");
		sleep(5);
	}
	
	return ret_socket;
}

void recv_udp_data(int s, char buff[UDP_BUFF_SIZE]){
	int recvlen;
	if(OJ_DEBUG){
		printf("waiting on port %d\n", OJ_UDP_PORT);
	}
	recvlen = recvfrom(s, buff, UDP_BUFF_SIZE, 0, 0, 0);
	buff[recvlen] = 0;
	if(OJ_DEBUG){
		printf("received %d bytes\n", recvlen);
		if (recvlen > 0) {		
			printf("received message: \"%s\"\n", buff);
		}
	}
}

void clear_udp_buffer(int s){
	char buff[UDP_BUFF_SIZE];
	pollfd poll_list[1];
	poll_list[0].fd = s;
	poll_list[0].events = POLLIN|POLLPRI;
	while(poll(poll_list,(unsigned long)1,0) > 0){
		recv_udp_data(s, buff);
	}
}

void listen_udp(){
	int udp_socket = create_udp_socket();
	char buff[UDP_BUFF_SIZE];

	while(daemon_work());
	
	while(true){
		recv_udp_data(udp_socket, buff);
		if(!strcmp(buff, OJ_TOKEN)){
			clear_udp_buffer(udp_socket);
			while(daemon_work());
		}
	}
	
	close(udp_socket);
}