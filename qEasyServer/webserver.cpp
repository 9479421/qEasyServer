#include "webserver.h"


WebServer::WebServer()
{
	users = new http[MAX_FD];
	users_timer = new client_data[MAX_FD];

	//获取当前路径
	char server_path[200];
	getcwd(server_path, 200); 
	//assets资源文件夹路径
	char root[8] = "/assets";
	//创建一个能放得下路径和文件夹长度的内存空间
	m_root = (char*)malloc(strlen(server_path) + strlen(root) + 1);
	//拼接路径+文件夹名
	strcpy(m_root, server_path);
	strcat(m_root, root);
}

WebServer::~WebServer()
{
	close(epollfd);
	close(listenfd);
	close(pipefd[1]);
	close(pipefd[0]);
	delete[] users;
	delete[] users_timer;
	delete m_pool;
}

void WebServer::get(std::string url, FunctionPtr functionPtr) {

	requestInfo requestInfo;
	requestInfo.url = url;
	requestInfo.functionPtr = functionPtr;
	requestInfo.type = GET;
	http::requestList.push_back(requestInfo);
	//printf("%d", http::requestList.size());
}

void WebServer::post(std::string url, FunctionPtr functionPtr) {
	requestInfo requestInfo;
	requestInfo.url = url;
	requestInfo.functionPtr = functionPtr;
	requestInfo.type = POST;
	http::requestList.push_back(requestInfo);
	//printf("%d", http::requestList.size());
}

void WebServer::run(int port, int threadNum, int maxRequest)
{
	m_pool = new threadpool<http>(threadNum, maxRequest); //线程数

	eventListen(port);
	eventLoop();
}

bool WebServer::eventListen(int port)
{
	listenfd = socket(PF_INET, SOCK_STREAM, 0);
	//复用地址，防止TIME_WAIT状态
	int flag = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	//固定套路
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
	listen(listenfd, 5);
	//epollfd
	epollfd = epoll_create(5);
	
	Utils::addfd(epollfd, listenfd, false, 1);
	//pipefd
	socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
	

	Utils::setnonblocking(pipefd[1]);
	Utils::addfd(epollfd, pipefd[0],false,0);

	Utils::addsig(SIGPIPE, SIG_IGN);

	Utils::addsig(SIGALRM, Utils::sig_handler, false);
	Utils::addsig(SIGTERM, Utils::sig_handler, false);


	
	alarm(TIMESLOT);


	Utils::u_pipefd = pipefd;
	http::m_epollfd = epollfd;//放在http操作类的静态成员里
	return false;
}

bool WebServer::eventLoop()
{
	bool timeout = false;
	bool stop_server = false;
	while (!stop_server)
	{
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		for (int i = 0; i < number; i++)
		{
			int sockfd = events[i].data.fd;
			//处理新到的客户连接
			if (sockfd == listenfd)
			{
				struct sockaddr_in client_address;
				socklen_t client_addrlength = sizeof(client_address);
				while (1)
				{
					
					int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
					if (connfd < 0)
					{
						break;
					}
					//添加users用户列表
					printf("add fd : %d\n", connfd);
					users[connfd].init(connfd, client_address, m_root);
					//添加users_timer计时器列表
					users_timer[connfd].address = client_address;
					users_timer[connfd].sockfd = connfd;
					util_timer* timer = new util_timer;
					timer->user_data = &users_timer[connfd];
					timer->cb_func = cb_func;
					time_t now = time(NULL);
					timer->expire = now + 3 * TIMESLOT;
					users_timer[connfd].timer = timer;
					sort_timer_lst::m_timer_lst->add_timer(timer);
				}
				continue;
			}
			else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
			{
				util_timer* timer = users_timer[sockfd].timer;
				deal_timer(timer, sockfd);
			}
			//处理信号
			else if ((sockfd == pipefd[0]) && (events[i].events & EPOLLIN))
			{
				char signals[1024];
				int ret = recv(pipefd[0], signals, sizeof(signals), 0);
				if (ret == -1)
				{
					
				}
				else if (ret == 0)
				{
					
				}
				else
				{
					for (int i = 0; i < ret; ++i)
					{
						switch (signals[i])
						{
						case SIGALRM:  //alarm或setitimer超时引起
							timeout = true;
							break;
						case SIGTERM:  //终止进程
							stop_server = true;
							break;
						}
					}
				}
				
			}
			//处理客户连接上接收到的数据
			else if (events[i].events & EPOLLIN)
			{
				

				util_timer* timer = users_timer[sockfd].timer;
				if (timer)
				{
					adjust_timer(timer);
				}

				m_pool->append(users + sockfd,0);

				while (true)
				{
					if (1 == users[sockfd].improv)
					{
						if (1 == users[sockfd].timer_flag)
						{
							deal_timer(timer, sockfd);
							users[sockfd].timer_flag = 0;
						}
						users[sockfd].improv = 0;
						break;
					}
				}
			}
			else if (events[i].events & EPOLLOUT)
			{
				util_timer* timer = users_timer[sockfd].timer;

				if (timer)
				{
					adjust_timer(timer);
				}

				m_pool->append(users + sockfd, 1);
				while (true)
				{
					if (1 == users[sockfd].improv)
					{
						if (1 == users[sockfd].timer_flag)
						{
							deal_timer(timer, sockfd);
							users[sockfd].timer_flag = 0;
						}
						users[sockfd].improv = 0;
						break;
					}
				}
			}
		}
		  
		if (timeout)
		{
			
			//tick删除超时 并且重新alarm(m_TIMESLOT)
			sort_timer_lst::timer_handler();

			timeout = false;
		}
	}
	return false;
}



void WebServer::deal_timer(util_timer* timer, int sockfd)
{
	printf("delete fd : %d\n", sockfd);
	timer->cb_func(&users_timer[sockfd]);
	if (timer)
	{
		sort_timer_lst::m_timer_lst->del_timer(timer);
	}
}

void WebServer::adjust_timer(util_timer* timer)
{
	time_t cur = time(NULL);
	timer->expire = cur + 3 * TIMESLOT;
	sort_timer_lst::m_timer_lst->adjust_timer(timer);

}


