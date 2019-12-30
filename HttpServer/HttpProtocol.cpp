#include "HttpProtocol.h"
#include "threadpool.h"
std::threadpool pool{ 10 };                    //线程池
// 格林威治时间的星期转换
char* week[] = {
	(char*)"Sun,",
	(char*)"Mon,",
	(char*)"Tue,",
	(char*)"Wed,",
	(char*)"Thu,",
	(char*)"Fri,",
	(char*)"Sat,",
};

// 格林威治时间的月份转换
char* month[] =
{
	(char*)"Jan",
	(char*)"Feb",
	(char*)"Mar",
	(char*)"Apr",
	(char*)"May",
	(char*)"Jun",
	(char*)"Jul",
	(char*)"Aug",
	(char*)"Sep",
	(char*)"Oct",
	(char*)"Nov",
	(char*)"Dec",
};

HttpProtocol::HttpProtocol(void)
{
	m_hwndDlg = NULL;
}

HttpProtocol::~HttpProtocol(void)
{
}
//开启Http服务
bool HttpProtocol::StartHttpSrv() {
	WORD wVersionRequested = WINSOCK_VERSION;
	WSADATA wsaData;


	int nRet;
	nRet = WSAStartup(wVersionRequested, &wsaData);		// 加载成功返回0
	if (nRet)
	{
		// 错误处理
		std::cout << "初始化 WinSock 失败"<< std::endl;
		return false;
	}
	// 检测版本
	if (wsaData.wVersion != wVersionRequested)
	{
		// 错误处理  
		std::cout << "Wrong WinSock Version" << std::endl;
		return false;
	}
	//创建套接字
	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_listenSocket == INVALID_SOCKET)
	{
		// 异常处理
		std::cout << "Could not create listen socket" << std::endl;
		return false;
	}
	//创建事件 人工复位 创始状态(是否有信号)
	m_hExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hExit == NULL)
	{
		return false;
	}

	SOCKADDR_IN sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = INADDR_ANY;  // 指定端口号上面的默认IP接口 
	u_short httpPort = 8080;
	//cout << "请输入http服务端口：";
	//cin >> httpPort;
	sockAddr.sin_port = htons(httpPort);
	CreateTypeMap();// 初始化map

	nRet = bind(m_listenSocket, (LPSOCKADDR)&sockAddr, sizeof(struct sockaddr));
	if (nRet == SOCKET_ERROR) {
		std::cout << "bind() error" << std::endl;
		closesocket(m_listenSocket);
		return false;
	}
	nRet = listen(m_listenSocket, SOMAXCONN);
	if (nRet == SOCKET_ERROR)
	{
		std::cout << "listen() error" << std::endl;
		closesocket(m_listenSocket);	// 断开链接
		return false;
	}
	//开启监听线程
	std::thread theListenThread{ ListenThread, this };
	theListenThread.detach();

	std::string strIP, strTemp;
	char hostname[255];
	PHOSTENT hostinfo;
	// 获取计算机名
	if (gethostname(hostname, sizeof(hostname)) == 0)	// 得到主机名
	{
		// 由主机名得到主机的其他信息
		hostinfo = gethostbyname(hostname);
		if (hostinfo != NULL)
		{
			strIP = inet_ntoa(*(struct in_addr*) * (hostinfo->h_addr_list));
		}
	}
	std::cout << "****** My WebServer is Starting now! *******" << std::endl;
	std::cout << hostname << "[" << strIP << "]" << "  port: " << htons(sockAddr.sin_port) << std::endl;

	return true;
}
 
void HttpProtocol::CreateTypeMap()
{
	// 初始化map
	m_typeMap[".doc"] = "application/msword";
	m_typeMap[".bin"] = "application/octet-stream";
	m_typeMap[".dll"] = "application/octet-stream";
	m_typeMap[".exe"] = "application/octet-stream";
	m_typeMap[".pdf"] = "application/pdf";
	m_typeMap[".ai"] = "application/postscript";
	m_typeMap[".eps"] = "application/postscript";
	m_typeMap[".ps"] = "application/postscript";
	m_typeMap[".rtf"] = "application/rtf";
	m_typeMap[".fdf"] = "application/vnd.fdf";
	m_typeMap[".arj"] = "application/x-arj";
	m_typeMap[".gz"] = "application/x-gzip";
	m_typeMap[".class"] = "application/x-java-class";
	m_typeMap[".js"] = "application/x-javascript";
	m_typeMap[".lzh"] = "application/x-lzh";
	m_typeMap[".lnk"] = "application/x-ms-shortcut";
	m_typeMap[".tar"] = "application/x-tar";
	m_typeMap[".hlp"] = "application/x-winhelp";
	m_typeMap[".cert"] = "application/x-x509-ca-cert";
	m_typeMap[".zip"] = "application/zip";
	m_typeMap[".cab"] = "application/x-compressed";
	m_typeMap[".arj"] = "application/x-compressed";
	m_typeMap[".aif"] = "audio/aiff";
	m_typeMap[".aifc"] = "audio/aiff";
	m_typeMap[".aiff"] = "audio/aiff";
	m_typeMap[".au"] = "audio/basic";
	m_typeMap[".snd"] = "audio/basic";
	m_typeMap[".mid"] = "audio/midi";
	m_typeMap[".rmi"] = "audio/midi";
	m_typeMap[".mp3"] = "audio/mpeg";
	m_typeMap[".vox"] = "audio/voxware";
	m_typeMap[".wav"] = "audio/wav";
	m_typeMap[".ra"] = "audio/x-pn-realaudio";
	m_typeMap[".ram"] = "audio/x-pn-realaudio";
	m_typeMap[".bmp"] = "image/bmp";
	m_typeMap[".gif"] = "image/gif";
	m_typeMap[".jpeg"] = "image/jpeg";
	m_typeMap[".jpg"] = "image/jpeg";
	m_typeMap[".tif"] = "image/tiff";
	m_typeMap[".tiff"] = "image/tiff";
	m_typeMap[".xbm"] = "image/xbm";
	m_typeMap[".wrl"] = "model/vrml";
	m_typeMap[".htm"] = "text/html";
	m_typeMap[".html"] = "text/html";
	m_typeMap[".c"] = "text/plain";
	m_typeMap[".cpp"] = "text/plain";
	m_typeMap[".def"] = "text/plain";
	m_typeMap[".h"] = "text/plain";
	m_typeMap[".txt"] = "text/plain";
	m_typeMap[".rtx"] = "text/richtext";
	m_typeMap[".rtf"] = "text/richtext";
	m_typeMap[".java"] = "text/x-java-source";
	m_typeMap[".css"] = "text/css";
	m_typeMap[".mpeg"] = "video/mpeg";
	m_typeMap[".mpg"] = "video/mpeg";
	m_typeMap[".mpe"] = "video/mpeg";
	m_typeMap[".avi"] = "video/msvideo";
	m_typeMap[".mov"] = "video/quicktime";
	m_typeMap[".qt"] = "video/quicktime";
	m_typeMap[".shtml"] = "wwwserver/html-ssi";
	m_typeMap[".asa"] = "wwwserver/isapi";
	m_typeMap[".asp"] = "wwwserver/isapi";
	m_typeMap[".cfm"] = "wwwserver/isapi";
	m_typeMap[".dbm"] = "wwwserver/isapi";
	m_typeMap[".isa"] = "wwwserver/isapi";
	m_typeMap[".plx"] = "wwwserver/isapi";
	m_typeMap[".url"] = "wwwserver/isapi";
	m_typeMap[".cgi"] = "wwwserver/isapi";
	m_typeMap[".php"] = "wwwserver/isapi";
	m_typeMap[".wcgi"] = "wwwserver/isapi";

}

//监听套接字连接请求，每一条请求创建一个线程处理请求
UINT HttpProtocol::ListenThread(LPVOID param)
{
	HttpProtocol* pHttpProtocol = (HttpProtocol*)param;

	SOCKET		socketClient;
	SOCKADDR_IN	SockAddr;
	PREQUEST	pReq;
	int			nLen;
	DWORD		dwRet;

	// 初始化ClientNum，创建"no client"事件对象
	HANDLE		hNoClients;
	hNoClients = pHttpProtocol->InitClientCount();

	while (1)	// 循环等待,如有客户连接请求,则接受客户机连接要求
	{
		nLen = sizeof(SOCKADDR_IN);
		// 套接字等待链接,返回对应已接受的客户机连接的套接字
		socketClient = accept(pHttpProtocol->m_listenSocket, (LPSOCKADDR)&SockAddr, &nLen);
		if (socketClient == INVALID_SOCKET)
		{
			break;
		}
		// 将客户端网络地址转换为用点分割的IP地址
		char* pstr = (char*)malloc(1024);
		sprintf(pstr, "%s Connecting on socket:%d", inet_ntoa(SockAddr.sin_addr), socketClient);
		std::cout << pstr << std::endl;

		pReq = new REQUEST;
		if (pReq == NULL)
		{
			// 处理错误
			std::string pStr = "No memory for request";
			std::cout << pStr << std::endl;
			continue;
		}
		pReq->hExit = pHttpProtocol->m_hExit;
		pReq->Socket = socketClient;
		pReq->hFile = INVALID_HANDLE_VALUE;
		pReq->dwRecv = 0;
		pReq->dwSend = 0;
		pReq->pHttpProtocol = pHttpProtocol;

		// 创建client进程，处理request
		//std::thread clientRequestThread{ ClientThread, pReq };
		//clientRequestThread.detach();
		//加入线程池任务队列
		pool.commit(ClientThread, pReq);

	} //while
	// 等待线程结束   INFINITE？？无限制等待？
	WaitForSingleObject((HANDLE)pHttpProtocol->m_hExit, INFINITE);
	// 等待所有client进程结束
	dwRet = WaitForSingleObject(hNoClients, 5000);
	if (dwRet == WAIT_TIMEOUT)
	{
		// 超时返回，并且同步对象未退出
		std::cout << "One or more client threads did not exit" << std::endl;
	}
	pHttpProtocol->DeleteClientCount();
	return 0;
}
UINT HttpProtocol::ClientThread(LPVOID param)
{
	int nRet;
	BYTE buf[1024];
	PREQUEST pReq = (PREQUEST)param;
	HttpProtocol* pHttpProtocol = (HttpProtocol*)pReq->pHttpProtocol;

	pHttpProtocol->CountUp();			// 记数

	// 接收request data
	if (!pHttpProtocol->RecvRequest(pReq, buf, sizeof(buf)))
	{
		pHttpProtocol->Disconnect(pReq);
		delete pReq;
		pHttpProtocol->CountDown();
		return 0;
	}

	// 分析request信息
	nRet = pHttpProtocol->Analyze(pReq, buf);
	if (nRet)
	{
		// 处理错误
		std::string pStr = "Error occurs when analyzing client request";
		std::cout << pStr << std::endl;

		pHttpProtocol->Disconnect(pReq);
		delete pReq;
		pHttpProtocol->CountDown();
		return 0;
	}

	// 生成并返回头部
	pHttpProtocol->SendHeader(pReq);

	// 向client传送数据
	if (pReq->nMethod == METHOD_GET)
	{
		pHttpProtocol->SendFile(pReq);
	}

	pHttpProtocol->Disconnect(pReq);
	delete pReq;
	pHttpProtocol->CountDown();	// client数量减1

	return 0;
}

bool HttpProtocol::RecvRequest(PREQUEST pReq, LPBYTE pBuf, DWORD dwBufSize)
{
	WSABUF			wsabuf;		// 发送/接收缓冲区结构
	WSAOVERLAPPED	over;		// 指向调用重叠操作时指定的WSAOVERLAPPED结构
	DWORD			dwRecv;
	DWORD			dwFlags;
	DWORD			dwRet;
	HANDLE			hEvents[2];
	bool			fPending;
	int				nRet;
	memset(pBuf, 0, dwBufSize);	// 初始化缓冲区
	wsabuf.buf = (char*)pBuf;
	wsabuf.len = dwBufSize;	// 缓冲区的长度
	over.hEvent = WSACreateEvent();	// 创建一个新的事件对象
	dwFlags = 0;
	fPending = FALSE;
	// 接收数据
	nRet = WSARecv(pReq->Socket, &wsabuf, 1, &dwRecv, &dwFlags, &over, NULL);
	if (nRet != 0)
	{
		// 错误代码WSA_IO_PENDING表示重叠操作成功启动
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			// 重叠操作未能成功
			CloseHandle(over.hEvent);
			return false;
		}
		else
		{
			fPending = true;
		}
	}
	if (fPending)
	{
		hEvents[0] = over.hEvent;
		hEvents[1] = pReq->hExit;
		dwRet = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (dwRet != 0)
		{
			CloseHandle(over.hEvent);
			return false;
		}
		// 重叠操作未完成
		if (!WSAGetOverlappedResult(pReq->Socket, &over, &dwRecv, FALSE, &dwFlags))
		{
			CloseHandle(over.hEvent);
			return false;
		}
	}
	pReq->dwRecv += dwRecv;	// 统计接收数量
	CloseHandle(over.hEvent);
	return true;
}
void HttpProtocol::Disconnect(PREQUEST pReq)
{
	// 关闭套接字：释放所占有的资源
	int	nRet;
	std::cout << "Closing socket: " << pReq->Socket << std::endl;
	nRet = closesocket(pReq->Socket);
	if (nRet == SOCKET_ERROR)
	{
		// 处理错误
		std::cout << "closesocket() error: %d"<< WSAGetLastError() << std::endl;
	}

	//HTTPSTATS	stats;
	//stats.dwRecv = pReq->dwRecv;
	//stats.dwSend = pReq->dwSend;
	//SendMessage(m_hwndDlg, DATA_MSG, (UINT)&stats, NULL);
}

// 分析request信息
int HttpProtocol::Analyze(PREQUEST pReq, LPBYTE pBuf)
{
	// 分析接收到的信息
	char szSeps[] = " \n";
	char* cpToken;
	// 防止非法请求
	if (strstr((const char*)pBuf, "..") != NULL)
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_BADREQUEST);
		return 1;
	}

	// 判断ruquest的mothed	
	cpToken = strtok((char*)pBuf, szSeps);	// 缓存中字符串分解为一组标记串。	
	if (!_stricmp(cpToken, "GET"))			// GET命令
	{
		pReq->nMethod = METHOD_GET;
	}
	else if (!_stricmp(cpToken, "HEAD"))	// HEAD命令
	{
		pReq->nMethod = METHOD_HEAD;
	}
	else
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_NOTIMPLEMENTED);
		return 1;
	}

	// 获取Request-URI
	cpToken = strtok(NULL, szSeps);
	if (cpToken == NULL)
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_BADREQUEST);
		return 1;
	}

	strcpy(pReq->szFileName, m_strRootDir);
	if (strlen(cpToken) > 1)
	{
		strcat(pReq->szFileName, cpToken);	// 把该文件名添加到结尾处形成路径
	}
	else
	{
		strcat(pReq->szFileName, "/index.html");
	}
	return 0;
}

// 发送头部
void HttpProtocol::SendHeader(PREQUEST pReq)
{

	int n = FileExist(pReq);
	if (!n)		// 文件不存在，则返回
	{
		return;
	}

	char Header[2048] = " ";
	char curTime[50] = " ";
	GetCurentTime((char*)curTime);
	// 取得文件长度
	DWORD length;
	length = GetFileSize(pReq->hFile, NULL);
	// 取得文件的last-modified时间
	char last_modified[60] = " ";
	GetLastModified(pReq->hFile, (char*)last_modified);
	// 取得文件的类型
	char ContenType[50] = " ";
	GetContenType(pReq, (char*)ContenType);

	sprintf((char*)Header, "HTTP/1.0 %s\r\nDate: %s\r\nServer: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nLast-Modified: %s\r\n\r\n",
		HTTP_STATUS_OK,
		curTime,				// Date
		"My Http Server",       // Server
		ContenType,				// Content-Type
		length,					// Content-length
		last_modified);			// Last-Modified

// 发送头部
	send(pReq->Socket, Header, strlen(Header), 0);
}

// 发送文件
void HttpProtocol::SendFile(PREQUEST pReq)
{
	int n = FileExist(pReq);
	if (!n)			// 文件不存在，则返回
	{
		return;
	}

	std::cout << &pReq->szFileName[strlen(m_strRootDir)] << std::endl;

	static BYTE buf[2048];
	DWORD  dwRead;
	BOOL   fRet;
	int flag = 1;
	// 读写数据直到完成
	while (1)
	{
		// 从file中读入到buffer中        
		fRet = ReadFile(pReq->hFile, buf, sizeof(buf), &dwRead, NULL);
		if (!fRet)
		{

			static char szMsg[512];
			wsprintf((LPWSTR)szMsg, (LPCWSTR)"%s", HTTP_STATUS_SERVERERROR);
			// 向客户端发送出错信息
			send(pReq->Socket, szMsg, strlen(szMsg), 0);
			break;
		}
		// 完成
		if (dwRead == 0)
		{
			break;
		}
		// 将buffer内容传送给client
		if (!SendBuffer(pReq, buf, dwRead))
		{
			break;
		}
		pReq->dwSend += dwRead;
	}
	// 关闭文件
	if (CloseHandle(pReq->hFile))
	{
		pReq->hFile = INVALID_HANDLE_VALUE;
	}
	else
	{
		std::cout << "Error occurs when closing file" << std::endl;
	}
}


int HttpProtocol::FileExist(PREQUEST pReq)
{
	//string fileNamePath = pReq->szFileName;
	TCHAR* path = (TCHAR*)malloc(1024);
	CharToTchar((pReq->szFileName), path);
	pReq->hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//ifstream in;
	//in.open(pReq->szFileName, ifstream::binary);
	if (pReq->hFile == INVALID_HANDLE_VALUE) {
		std::cout << "文件打开失败" << std::endl;
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_NOTFOUND);
		std::cout << "请求文件不存在，文件：" << pReq->szFileName << std::endl;
		return 0;
	}
	else
	{
		
		return 1;
	}
}
bool HttpProtocol::SendBuffer(PREQUEST pReq, LPBYTE pBuf, DWORD dwBufSize)
{
	// 发送缓存中的内容
	WSABUF			wsabuf;
	WSAOVERLAPPED	over;
	DWORD			dwRecv;
	DWORD			dwFlags;
	DWORD			dwRet;
	HANDLE			hEvents[2];
	BOOL			fPending;
	int				nRet;
	wsabuf.buf = (char*)pBuf;
	wsabuf.len = dwBufSize;
	over.hEvent = WSACreateEvent();
	fPending = false;

	// 发送数据 
	nRet = WSASend(pReq->Socket, &wsabuf, 1, &dwRecv, 0, &over, NULL);
	if (nRet != 0)
	{
		// 错误出理
		if (WSAGetLastError() == WSA_IO_PENDING)
		{
			fPending = true;
		}
		else
		{
			std::cout << "WSASend() error: %d" << WSAGetLastError() << std::endl;
			CloseHandle(over.hEvent);
			return false;
		}
	}
	if (fPending)	// i/o未完成
	{
		hEvents[0] = over.hEvent;
		hEvents[1] = pReq->hExit;
		dwRet = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (dwRet != 0)
		{
			CloseHandle(over.hEvent);
			return false;
		}
		// 重叠操作未完成
		if (!WSAGetOverlappedResult(pReq->Socket, &over, &dwRecv, FALSE, &dwFlags))
		{
			// 错误处理
			std::cout << "WSAGetOverlappedResult() error: " << WSAGetLastError() << std::endl;
			CloseHandle(over.hEvent);
			return false;
		}
	}
	CloseHandle(over.hEvent);
	return true;
}

//////////////////////////////////////////////////////////////////////////////

HANDLE HttpProtocol::InitClientCount()
{
	ClientNum = 0;
	// 创建"no client"事件对象
	None = CreateEvent(NULL, TRUE, TRUE, NULL);
	return None;
}
void HttpProtocol::CountUp()
{
	// 进入排斥区
	cout_lock.lock();
	ClientNum++;
	// 离开排斥区
	cout_lock.unlock();
}
void HttpProtocol::CountDown()
{
	// 进入排斥区
	cout_lock.lock();
	if (ClientNum > 0)
	{
		ClientNum--;
	}
	// 离开排斥区
	cout_lock.unlock();
}
void HttpProtocol::DeleteClientCount()
{
	CloseHandle(None);
}
// 活动本地时间
void HttpProtocol::GetCurentTime(LPSTR lpszString)
{
	// 活动本地时间
	SYSTEMTIME st;
	GetLocalTime(&st);
	// 时间格式化
	sprintf(lpszString, "%s %02d %s %d %02d:%02d:%02d GMT", week[st.wDayOfWeek], st.wDay, month[st.wMonth - 1],
		st.wYear, st.wHour, st.wMinute, st.wSecond);
}
bool HttpProtocol::GetLastModified(HANDLE hFile, LPSTR lpszString)
{
	// 获得文件的last-modified 时间
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stCreate;
	FILETIME ftime;
	// 获得文件的last-modified的UTC时间
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		return false;
	FileTimeToLocalFileTime(&ftWrite, &ftime);
	// UTC时间转化成本地时间
	FileTimeToSystemTime(&ftime, &stCreate);
	// 时间格式化
	sprintf(lpszString, "%s %02d %s %d %02d:%02d:%02d GMT", week[stCreate.wDayOfWeek],
		stCreate.wDay, month[stCreate.wMonth - 1], stCreate.wYear, stCreate.wHour,
		stCreate.wMinute, stCreate.wSecond);
	return TRUE;
}
bool HttpProtocol::GetContenType(PREQUEST pReq, LPSTR type)
{
	// 取得文件的类型
	char* cpToken;
	cpToken = strstr(pReq->szFileName, ".");
	strcpy(pReq->postfix, cpToken);
	// 遍历搜索该文件类型对应的content-type
	std::map<std::string,const char*>::iterator it = m_typeMap.find(pReq->postfix);
	if (it != m_typeMap.end())
	{
		sprintf(type, "%s", (*it).second);
	}
	return TRUE;
}
void HttpProtocol::StopHttpSrv()
{

	int nRet;
	SetEvent(m_hExit);
	nRet = closesocket(m_listenSocket);
	//nRet = WaitForSingleObject((HANDLE)m_pListenThread, 10000);
	if (nRet == WAIT_TIMEOUT)
	{
		std::cout << "TIMEOUT waiting for ListenThread" << std::endl;
	}
	CloseHandle(m_hExit);

	std::cout << "Server Stopped" << std::endl;
}
void HttpProtocol::CharToTchar(const char* _char, TCHAR* tchar)
{
	int iLength;

	iLength = MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, tchar, iLength);
}