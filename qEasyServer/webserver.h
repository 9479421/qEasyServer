#pragma once
#include "sys/socket.h"
#include "arpa/inet.h"
#include "sys/epoll.h"
#include<string.h>
#include<string>
#include<fcntl.h>
#include<signal.h>
#include<stdio.h>

#include <unistd.h>
#include<errno.h>
#include"http.h"
#include"utils.h"
#include"threadpool.h"
#include"lst_timer.h"


const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 10000; //最大事件数
const int TIMESLOT = 5;             //最小超时单位





class WebServer
{
public:
	WebServer();
	~WebServer();
	bool eventListen(int port = 9006);
	bool eventLoop();
	void deal_timer(util_timer* timer, int sockfd);
	void adjust_timer(util_timer* timer);

	void get(std::string url, FunctionPtr functionPtr);
	void post(std::string url, FunctionPtr functionPtr);

	void run(int port,int threadNum = 8 ,int maxRequest = 10000);
private:
	int listenfd;
	int epollfd;
	int pipefd[2];
	epoll_event events[MAX_EVENT_NUMBER];

	std::string m_string;

	http* users;

	char* m_root;

	//线程池相关
	threadpool<http>* m_pool;
	int m_thread_num;

	client_data* users_timer;


};
