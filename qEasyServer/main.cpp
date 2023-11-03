
#include"webserver.h"


std::string login(std::unordered_map<std::string, std::string> RequestParams) {

	return "you input name = "+ RequestParams["username"] + "; pass = " + RequestParams["password"];
}

std::string sayHello(std::unordered_map<std::string, std::string> RequestParams) {

	return "Hello World!";
}


int main() {
	WebServer webserver;

	webserver.get("/api/login", login);
	webserver.post("/hello", sayHello);

	webserver.run(9006, 10, 10000);//端口、线程数量、最大http连接数
}

