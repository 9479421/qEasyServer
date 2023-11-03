#pragma once
#include<sys/epoll.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<errno.h>
#include<sys/socket.h>

class Utils
{
public:
	static void addfd(int epollfd,int fd, bool one_shot, int TRIGMode);
	static int setnonblocking(int fd);
	static void modfd(int epollfd, int fd, int ev, int TRIGMode);
	static void removefd(int epollfd, int fd);


	static void sig_handler(int sig);
	static void addsig(int sig, void(handler)(int), bool restart = true);
private:

public:
	static int* u_pipefd;
};
