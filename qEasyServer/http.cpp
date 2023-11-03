#include "http.h"

#include<string>
//定义http响应的一些状态信息
const char* ok_200_title = "OK";
const char* error_400_title = "Bad Request";
const char* error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char* error_403_title = "Forbidden";
const char* error_403_form = "You do not have permission to get file form this server.\n";
const char* error_404_title = "Not Found";
const char* error_404_form = "The requested file was not found on this server.\n";
const char* error_500_title = "Internal Error";
const char* error_500_form = "There was an unusual problem serving the request file.\n";


int http::m_user_count = 0;
int http::m_epollfd = -1;
std::list<requestInfo> http::requestList;



void http::init()
{
	bytes_to_send = 0;
	bytes_have_send = 0;

	m_check_state = CHECK_STATE_REQUESTLINE;
	m_linger = false;
	m_method = GET;
	m_url = 0;
	m_version = 0;
	m_content_length = 0;
	m_host = 0;
	
	m_start_line = 0;
	m_read_idx = 0;
	m_checked_idx = 0;
	m_write_idx = 0;
	memset(m_read_buf, '\0', READ_BUFFER_SIZE);
	memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
	memset(m_real_file, '\0', FILENAME_LEN);

	m_state = 0;
	timer_flag = 0;
	improv = 0;
}

void http::init(int sockfd, const sockaddr_in& addr,char *root)
{
	
	m_sockfd = sockfd;
	m_address = addr;
	doc_root = root;



	m_user_count++;

	

	Utils::addfd(m_epollfd, sockfd, true, 0);

	init();
}

bool http::read_once()
{
	if (m_read_idx >= READ_BUFFER_SIZE)
	{
		return false;
	}
	int bytes_read = 0;
	while (true)
	{
		//printf("%s\n", m_read_buf);
		bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
		if (bytes_read == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK) //read finished
				break;
			return false;
		}
		else if (bytes_read == 0)
		{
			return false;
		}
		m_read_idx += bytes_read;
	}
	return true;
}

http::LINE_STATUS http::parse_line()
{
	char temp;
	for (; m_checked_idx < m_read_idx; ++m_checked_idx)
	{
		temp = m_read_buf[m_checked_idx];
		if (temp == '\r')
		{
			if ((m_checked_idx + 1) == m_read_idx) {  //m_checked_idx最大值就是m_read_idx-1
				return LINE_OPEN;
			}
			else if (m_read_buf[m_checked_idx + 1] == '\n')
			{
				m_read_buf[m_checked_idx++] = '\0';
				m_read_buf[m_checked_idx++] = '\0';
				return LINE_OK;  //接收完整的一行
			}
			return LINE_BAD;
		}
		else if (temp == '\n')  //第二次接受与第一次产生对接
		{
			if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r') {
				m_read_buf[m_checked_idx - 1] = '\0';
				m_read_buf[m_checked_idx++] = '\0';
				return LINE_OK;
			}
			return LINE_BAD;
		}
	}
	return LINE_OPEN;
}

http::HTTP_CODE http::parse_request_line(char* text)
{
	m_url = strpbrk(text, " \t");
	if (!m_url)
	{
		return BAD_REQUEST;
	}
	*m_url++ = '\0';
	char* method = text;
	if (strcasecmp(method, "GET") == 0)
	{
		m_method = GET;
	}
	else if (strcasecmp(method, "POST") == 0) {
		m_method = POST;
	}
	else {
		return BAD_REQUEST;
	}
	m_url += strspn(m_url, " \t");

	m_version = strpbrk(m_url, " \t");
	if (!m_version)
	{
		return BAD_REQUEST;
	}
	*m_version++ = '\0';
	m_version += strspn(m_version, " \t");
	if (strcasecmp(m_version, "HTTP/1.1") != 0)
	{
		return BAD_REQUEST;
	}
	if (strncasecmp(m_url, "http://", 7) == 0)
	{
		m_url += 7;
		m_url = strchr(m_url, '/');
	}
	if (strncasecmp(m_url, "https://", 8) == 0)
	{
		m_url += 8;
		m_url = strchr(m_url, '/');
	}

	if (!m_url || m_url[0] != '/')
	{
		return BAD_REQUEST;
	}

	


	if (strlen(m_url) == 1)  // ==/
	{
		strcat(m_url, "1.html"); //默认的文件
	}
	m_check_state = CHECK_STATE_HEADER;

	return NO_REQUEST;
}

http::HTTP_CODE http::parse_headers(char* text)
{
	if (text[0] == '\0') //请求头与请求体的分隔行
	{
		if (m_content_length != 0) {   //说明是POST请求，仍需解析content
			m_check_state = CHECK_STATE_CONTENT;
			return NO_REQUEST;
		}
		return GET_REQUEST; //说明是get请求，不需要解析了，直接发送数据
	}
	else if (strncasecmp(text, "Connection:", 11) == 0) {
		text += 11;
		text += strspn(text, " \t");
		if (strcasecmp(text, "keep-alive") == 0)
		{
			m_linger = true;
		}
	}
	else if (strncasecmp(text, "Content-length:", 15) == 0) {
		text += 15;
		text += strspn(text, " \t");
		m_content_length = atol(text);
	}
	else if (strncasecmp(text, "Host:", 5) == 0) {
		text += 5;
		text += strspn(text, " \t");
		m_host = text;
	}
	else if (strncasecmp(text, "Content-Type:", 13) == 0) {
		text += 13;
		text += strspn(text, " \t");
		m_content_type = text;
	}
	else {
		//unknow header
	}
	return NO_REQUEST;
}

http::HTTP_CODE http::parse_content(char* text)
{
	if (m_read_idx >= (m_content_length + m_checked_idx))
	{
		text[m_content_length] = '\0';
		m_string = text;
		return GET_REQUEST;
	}
	return NO_REQUEST;
}

http::HTTP_CODE http::process_read()
{
	LINE_STATUS line_status = LINE_OK;
	HTTP_CODE ret = NO_REQUEST;
	char* text = 0;
	while ((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || ((line_status = parse_line()) == LINE_OK))
	{
		text = get_line();
		m_start_line = m_checked_idx;
		switch (m_check_state)
		{
		case http::CHECK_STATE_REQUESTLINE:
			ret = parse_request_line(text);
			if (ret == BAD_REQUEST)
			{
				return BAD_REQUEST;
			}
			break;
		case http::CHECK_STATE_HEADER:
			ret = parse_headers(text);
			if (ret == BAD_REQUEST)
			{
				return BAD_REQUEST;
			}
			else if (ret == GET_REQUEST)
			{
				return do_request();
			}
			break;
		case http::CHECK_STATE_CONTENT:
			ret = parse_content(text);
			if (ret == GET_REQUEST)
			{
				return do_request();
			}
			line_status = LINE_OPEN;
			break;
		default:
			return INTERNAL_ERROR;
		}
	}
	
	return NO_REQUEST;
}

bool http::process_write(HTTP_CODE ret)
{
	switch (ret)
	{
	case http::INTERNAL_ERROR:
		add_status_line(500, error_500_title);
		add_headers(strlen(error_500_form));
		if (!add_content(error_500_form))
			return false;
		break;
	case http::BAD_REQUEST:
		add_status_line(404, error_404_title);
		add_headers(strlen(error_404_form));
		if (!add_content(error_404_form))
		{
			return false;
		}
		break;
	case http::FORBIDDEN_REQUEST:
		add_status_line(403, error_403_title);
		add_headers(strlen(error_403_form));
		if (!add_content(error_403_form))
			return false;
		break;
	case http::FILE_REQUEST:
		add_status_line(200, ok_200_title);
		if (m_file_stat.st_size!=0)
		{
			add_headers(m_file_stat.st_size);
			m_iv[0].iov_base = m_write_buf;
			m_iv[0].iov_len = m_write_idx;
			m_iv[1].iov_base = m_file_address;
			m_iv[1].iov_len = m_file_stat.st_size;
			m_iv_count = 2;
			bytes_to_send = m_write_idx + m_file_stat.st_size;
			return true;
		}
		else {
			const char* ok_string = "<html><body></body></html>";
			add_headers(strlen(ok_string));
			if (!add_content(ok_string))
				return false;
		}
	case http::TEXT_REQUEST:
		add_status_line(200, ok_200_title);
		add_headers(strlen(m_file_address));
		m_iv[0].iov_base = m_write_buf;
		m_iv[0].iov_len = m_write_idx;
		m_iv[1].iov_base = m_file_address;
		m_iv[1].iov_len = strlen(m_file_address);
		m_iv_count = 2;
		bytes_to_send = m_write_idx + strlen(m_file_address);
		return true;
	default:
		return false;
	}


	m_iv[0].iov_base = m_write_buf;
	m_iv[0].iov_len = m_write_idx;
	m_iv_count = 1;
	bytes_to_send = m_write_idx;
	return true;
}

http::HTTP_CODE http::do_request()
{
	//http://127.0.0.1/getApi?
	//解析URL
	if (strstr(m_url, "?") != NULL) {
		char* params = strchr(m_url, '?');
		*params++ = '\0';

		char* tmpStr = strtok(params, "&");
		while (tmpStr != NULL)
		{
			char* val = strpbrk(tmpStr, "=");
			if (val)
			{
				*val++ = '\0';
				char* key = tmpStr;
				//printf("urlpath key:%s val:%s\n", key, val);
				
				RequestParams.insert(std::pair<std::string, std::string>(key, val));
			}
			tmpStr = strtok(NULL, "&");
		}
	}
	//解析请求体
	if (m_method == POST)
	{
		//判断ContentType再解析请求参数
		if (strncmp(m_content_type, "application/x-www-form-urlencoded", 33) == 0  ) {
			char* tmpStr = strtok(m_string, "&");
			while (tmpStr != NULL)
			{
				char* val = strpbrk(tmpStr, "=");
				if (val)
				{
					*val++ = '\0';
					char* key = tmpStr;
					//printf("www key:%s\n val:%s\n", key, val);
					RequestParams.insert(std::pair<std::string, std::string>(key, val));
				}
				tmpStr = strtok(NULL, "&");
			}
		}
		if (strncmp(m_content_type, "multipart/form-data", 19) == 0)
		{
			//这个比较复杂，可能涉及文件传输，后面遇到再加
		}
		if (strncmp(m_content_type, "application/json", 16) == 0) {
			QJsonObject * jsonObject = QJsonObject::parseObject(m_string);

			QJsonChild* temp = jsonObject->m_jsonChild;
			while (temp != NULL)
			{
				//printf("json key:%s val:%s\n", temp->key, temp->valStr);
				RequestParams.insert(std::pair<std::string, std::string>(temp->key, temp->valStr)); //这里只支持一级jsonObject
				temp = temp->next;
			}
		}
	}

	//如果能遍历到url，那就以TEXT_REQUEST返回
	std::list<requestInfo>::iterator it;
	for (it = http::requestList.begin();  it!= http::requestList.end() ; ++it)
	{
		//printf("%s\n", (*it).url.c_str());
		if ((*it).type == m_method && strcmp(m_url, (*it).url.c_str()) == 0) //POST和GET对上再做下一步操作
		{
			//把数据拷贝一份放在m_file_address
			std::string ret = (*it).functionPtr(RequestParams);
			m_file_address = (char*)malloc(sizeof(char)* (ret.length() + 1));
			strcpy(m_file_address, ret.data());
			return TEXT_REQUEST;
		}
	}


	strcpy(m_real_file, doc_root);
	int len = strlen(doc_root);
	const char* p = strrchr(m_url, '/');

	
	strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);
	//printf("directory: %s\n", m_real_file);
	if (stat(m_real_file,&m_file_stat) < 0)
	{
		return NO_RESOURCE;
	}

	if (!(m_file_stat.st_mode & S_IROTH))
	{
		return FORBIDDEN_REQUEST;
	}

	if (S_ISDIR(m_file_stat.st_mode))
	{
		return BAD_REQUEST;
	}

	int fd = open(m_real_file, O_RDONLY);
	m_file_address = (char*)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	return FILE_REQUEST;




}


void http::process()
{
	HTTP_CODE read_ret = process_read();
	if (read_ret == NO_REQUEST)
	{
		Utils::modfd(m_epollfd,m_sockfd, EPOLLIN,1);
		return;
	}
	bool write_ret = process_write(read_ret);
	if (!write_ret)
	{
		close_conn();
	}

	Utils::modfd(m_epollfd, m_sockfd, EPOLLOUT, 1);
}

bool http::write()
{
	int temp = 0;

	if (bytes_to_send == 0)
	{
		Utils::modfd(m_epollfd, m_sockfd, EPOLLIN, 1);
		init();
		return true;
	}
	while (1)
	{
		temp = writev(m_sockfd, m_iv, m_iv_count);
		if (temp < 0)
		{
			if (errno == EAGAIN)
			{
				Utils::modfd(m_epollfd, m_sockfd,EPOLLOUT, 1);
				return true;
			}
			unmap();
			return false;
		}
		bytes_have_send += temp;
		bytes_to_send -= temp;
		if (bytes_have_send >= m_iv[0].iov_len)
		{
			m_iv[0].iov_len = 0;
			m_iv[1].iov_base = m_file_address + (bytes_have_send - m_write_idx);
			m_iv[1].iov_len = bytes_to_send;
		}
		else
		{
			m_iv[0].iov_base = m_write_buf + bytes_have_send;
			m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
		}

		if (bytes_to_send <= 0)
		{
			unmap();
			Utils::modfd(m_epollfd, m_sockfd, EPOLLIN, 1);

			if (m_linger)
			{
				init();
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}


void http::close_conn(bool real_close)
{
	if (real_close && (m_sockfd != -1))
	{
		printf("close %d\n", m_sockfd);
		Utils::removefd(m_epollfd, m_sockfd);
		m_sockfd = -1;
		m_user_count--;
	}
}

bool http::add_response(const char* format, ...)
{
	if (m_write_idx >= WRITE_BUFFER_SIZE)
	{
		return false;
	}
	va_list arg_list;
	va_start(arg_list,format);
	int len = vsnprintf(m_write_buf + m_write_idx,WRITE_BUFFER_SIZE - 1 - m_write_idx, format , arg_list);
	if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
	{
		va_end(arg_list);
		return false;
	}
	m_write_idx += len;
	va_end(arg_list);

	return true;
}

bool http::add_content(const char* content)
{
	return add_response("%s", content);
}

bool http::add_status_line(int status, const char* title)
{
	return add_response("%s %d %s\r\n","HTTP/1.1" ,status, title);
}

bool http::add_headers(int content_length)
{
	return add_content_length(content_length) && add_linger() &&
		add_blank_line();
}

bool http::add_content_type()
{
	return add_response("Content-Type:%s\r\n", "text/html");
}

bool http::add_content_length(int content_length)
{
	return add_response("Content-Length:%d\r\n", content_length);
}

bool http::add_linger()
{
	return add_response("Connection:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}

bool http::add_blank_line()
{
	return add_response("%s", "\r\n");
}

void http::unmap()
{
	if (m_file_address)
	{
		munmap(m_file_address, m_file_stat.st_size);
		m_file_address = 0;
	}
}
