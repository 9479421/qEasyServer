#include "utils.h"

int* Utils::u_pipefd = 0;

void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
	epoll_event event;
	event.data.fd = fd;
	if (1 == TRIGMode)
		event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
	else
		event.events = EPOLLIN | EPOLLRDHUP;

	if (one_shot)
	{
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnonblocking(fd);
}

int Utils::setnonblocking(int fd)
{
	int oldOpt = fcntl(fd, F_GETFL);
	int newOpt = oldOpt | O_NONBLOCK;
	fcntl(fd, F_SETFL, newOpt);
	return oldOpt;
}

void Utils::modfd(int epollfd, int fd, int ev, int TRIGMode)
{
	epoll_event event;
	event.data.fd = fd;
	if (1 == TRIGMode)
		event.events = ev | EPOLLONESHOT | EPOLLRDHUP | EPOLLET;
	else
		event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

	epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void Utils::removefd(int epollfd, int fd)
{
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	close(fd);
}

void Utils::sig_handler(int sig)
{
	int save_errno = errno;
	int msg = sig;
	send(u_pipefd[1], (char*)&msg, 1, 0);
	errno = save_errno;
}

void Utils::addsig(int sig, void(handler)(int), bool restart)
{
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if (restart)
		sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	sigaction(sig, &sa, NULL);
}
