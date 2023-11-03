#pragma once

#include<time.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<unistd.h>
#include "http.h"

class util_timer;

struct client_data
{
	sockaddr_in address;
	int sockfd;
	util_timer* timer;
};

class util_timer
{
public:
	util_timer() : prev(NULL), next(NULL) {}


public:
	time_t expire;

	void(*cb_func)(client_data *);
	client_data* user_data;
	util_timer* prev;
	util_timer* next;
};


class sort_timer_lst
{
public:
	sort_timer_lst();
	~sort_timer_lst();

	void add_timer(util_timer *timer);
	void adjust_timer(util_timer* timer);
	void del_timer(util_timer* timer);
	void tick();

	static void timer_handler();
private:
	void add_timer(util_timer* timer, util_timer* lst_head);

	util_timer* head;
	util_timer* tail;

public:
	static sort_timer_lst* m_timer_lst;
};


void cb_func(client_data* user_data);  //移除