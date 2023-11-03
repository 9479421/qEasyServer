<div align="center">
    <h1> QEasyServer </h1>
</div>


## 项目说明

本项目采用C/C++编写，运行环境为Linux系统，是一个简易的HttpServer脚手架项目。

项目核心使用Epoll，采用边缘触发ET，使用Reactor模型，手写线程池，手写双向链表，手写JSON解析实现的百万并发http服务器。

除支持文件传输以外，仿照SpringBoot和Express语法做了基本的API接口封装。两者优先寻找API接口。

学习参考书籍《TCP/IP网络编程》《Linux服务器高性能编程》

注：核心思路学习自TinyWebServer，在此基础上做了精简与功能完善。

## 开发环境

Visual Studio 2022

Ubuntu

依赖：g++、pthread、openssh-server

如果使用的是Vscode并且手动g++运行可以忽略此步。如使用VS，工作目录务必设置为$(RemoteDeployDir)，因为VS默认会跑到/bin/X64/里寻找文件，与我们项目本身的根目录资源文件位置/assets不符。

![image-20231103123910309](https://wqby-1304194722.cos.ap-nanjing.myqcloud.com/img/image-20231103123910309.png)

使用VS还需要添加库依赖项pthread

![image-20231103224437459](https://wqby-1304194722.cos.ap-nanjing.myqcloud.com/img/example.jpg)

## 运行项目

```c++
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
```

实现新的接口只需按webserver.post/get语法规则进行扩充即可

## 效果展示

![](https://wqby-1304194722.cos.ap-nanjing.myqcloud.com/img/image-20231103125259806.png)

![image-20231103125214572](https://wqby-1304194722.cos.ap-nanjing.myqcloud.com/img/image-20231103125214572.png)
