#pragma once

#include<sys/socket.h>
#include<arpa/inet.h>

#include<sys/epoll.h>
#include<fcntl.h>

#include<string.h>
#include<string>
#include<errno.h>
#include<stdlib.h>

#include<unistd.h>
#include<stdio.h>
#include<stdarg.h>

#include<sys/mman.h>
#include<sys/stat.h>
#include <sys/uio.h>
#include"utils.h"
#include"qJson.h"

#include <unordered_map>
#include<list>

enum METHOD
{
	GET = 0,
	POST,
	HEAD,
	PUT,
	DELETE,
	TRACE,
	OPTIONS,
	CONNECT,
	PATH
};

typedef std::string(*FunctionPtr)(std::unordered_map<std::string, std::string>);
typedef struct requestInfo {
	std::string url;
	FunctionPtr functionPtr;
	METHOD type;
};

class http
{
public:
	static const int FILENAME_LEN = 200;
	static const int READ_BUFFER_SIZE = 2048;
	static const int WRITE_BUFFER_SIZE = 1024;
	
	enum CHECK_STATE {
		CHECK_STATE_REQUESTLINE = 0,
		CHECK_STATE_HEADER,
		CHECK_STATE_CONTENT
	};
	enum HTTP_CODE
	{
		NO_REQUEST,
		GET_REQUEST,
		BAD_REQUEST,
		NO_RESOURCE,
		FORBIDDEN_REQUEST,
		FILE_REQUEST,
		TEXT_REQUEST,
		INTERNAL_ERROR,
		CLOSED_CONNECTION
	};
	enum LINE_STATUS
	{
		LINE_OK = 0,
		LINE_BAD,
		LINE_OPEN
	};
public:
	http() {}
	~http() {}

public:
	void init(int sockfd, const sockaddr_in& addr, char* root);
	bool read_once();
	void close_conn(bool real_close = true);
	void process();
	bool write();
	int timer_flag;
	int improv;
private:
	void init();

	LINE_STATUS parse_line();
	HTTP_CODE parse_request_line(char* text);
	HTTP_CODE parse_headers(char* text);
	HTTP_CODE parse_content(char* text);

	HTTP_CODE process_read();
	bool process_write(HTTP_CODE ret);

	HTTP_CODE do_request();

	char* get_line() { return m_read_buf + m_start_line; };

	bool add_response(const char* format, ...);
	bool add_content(const char* content);
	bool add_status_line(int status, const char* title);
	bool add_headers(int content_length);
	bool add_content_type();
	bool add_content_length(int content_length);
	bool add_linger();
	bool add_blank_line();

	void unmap();

public:
	static int m_epollfd;
	static int m_user_count;

	int m_state;  //读为0, 写为1

private:
	int m_sockfd;
	sockaddr_in m_address;
	char* doc_root;

	char m_read_buf[READ_BUFFER_SIZE];
	long m_read_idx;
	long m_checked_idx;
	

	CHECK_STATE m_check_state;
	METHOD m_method;

	char* m_url;
	char* m_version;
	char* m_host;
	long m_content_length;
	char* m_content_type;

	char* m_string; //存储请求头数据

	int m_start_line;
	
	
	char m_real_file[FILENAME_LEN];
	char* m_file_address;

	char m_write_buf[WRITE_BUFFER_SIZE];
	int m_write_idx;

	struct stat m_file_stat;
	struct iovec m_iv[2];
	int m_iv_count;

	int bytes_to_send;
	int bytes_have_send;
	bool m_linger; //是否是keep-alive


	std::unordered_map<std::string, std::string> RequestParams; //所有的东西都塞里面


public:
	static std::list<requestInfo> requestList;

};




