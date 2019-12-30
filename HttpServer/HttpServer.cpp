// HttpServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "HttpProtocol.h"
int main()
{
	HttpProtocol h;
	std::cout << "请输入Http根目录全路径：";
	//cin >> h.m_strRootDir = "C:\\MyCode\\Html";
	h.StartHttpSrv();
	int i;
	while (true)
	{
		std::cout << "输入4关闭服务";
		std::cin >> i;
		if (i == 4) {
			h.StopHttpSrv();
			break;
		}
	}
	std::cout << "Hello World!\n";
}