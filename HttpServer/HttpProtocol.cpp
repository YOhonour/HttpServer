#include "HttpProtocol.h"
#include "threadpool.h"
std::threadpool pool{ 10 };                    //�̳߳�
// ��������ʱ�������ת��
char* week[] = {
	(char*)"Sun,",
	(char*)"Mon,",
	(char*)"Tue,",
	(char*)"Wed,",
	(char*)"Thu,",
	(char*)"Fri,",
	(char*)"Sat,",
};

// ��������ʱ����·�ת��
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
//����Http����
bool HttpProtocol::StartHttpSrv() {
	WORD wVersionRequested = WINSOCK_VERSION;
	WSADATA wsaData;


	int nRet;
	nRet = WSAStartup(wVersionRequested, &wsaData);		// ���سɹ�����0
	if (nRet)
	{
		// ������
		std::cout << "��ʼ�� WinSock ʧ��"<< std::endl;
		return false;
	}
	// ���汾
	if (wsaData.wVersion != wVersionRequested)
	{
		// ������  
		std::cout << "Wrong WinSock Version" << std::endl;
		return false;
	}
	//�����׽���
	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_listenSocket == INVALID_SOCKET)
	{
		// �쳣����
		std::cout << "Could not create listen socket" << std::endl;
		return false;
	}
	//�����¼� �˹���λ ��ʼ״̬(�Ƿ����ź�)
	m_hExit = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hExit == NULL)
	{
		return false;
	}

	SOCKADDR_IN sockAddr;
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = INADDR_ANY;  // ָ���˿ں������Ĭ��IP�ӿ� 
	u_short httpPort = 8080;
	//cout << "������http����˿ڣ�";
	//cin >> httpPort;
	sockAddr.sin_port = htons(httpPort);
	CreateTypeMap();// ��ʼ��map

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
		closesocket(m_listenSocket);	// �Ͽ�����
		return false;
	}
	//���������߳�
	std::thread theListenThread{ ListenThread, this };
	theListenThread.detach();

	std::string strIP, strTemp;
	char hostname[255];
	PHOSTENT hostinfo;
	// ��ȡ�������
	if (gethostname(hostname, sizeof(hostname)) == 0)	// �õ�������
	{
		// ���������õ�������������Ϣ
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
	// ��ʼ��map
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

//�����׽�����������ÿһ�����󴴽�һ���̴߳�������
UINT HttpProtocol::ListenThread(LPVOID param)
{
	HttpProtocol* pHttpProtocol = (HttpProtocol*)param;

	SOCKET		socketClient;
	SOCKADDR_IN	SockAddr;
	PREQUEST	pReq;
	int			nLen;
	DWORD		dwRet;

	// ��ʼ��ClientNum������"no client"�¼�����
	HANDLE		hNoClients;
	hNoClients = pHttpProtocol->InitClientCount();

	while (1)	// ѭ���ȴ�,���пͻ���������,����ܿͻ�������Ҫ��
	{
		nLen = sizeof(SOCKADDR_IN);
		// �׽��ֵȴ�����,���ض�Ӧ�ѽ��ܵĿͻ������ӵ��׽���
		socketClient = accept(pHttpProtocol->m_listenSocket, (LPSOCKADDR)&SockAddr, &nLen);
		if (socketClient == INVALID_SOCKET)
		{
			break;
		}
		// ���ͻ��������ַת��Ϊ�õ�ָ��IP��ַ
		char* pstr = (char*)malloc(1024);
		sprintf(pstr, "%s Connecting on socket:%d", inet_ntoa(SockAddr.sin_addr), socketClient);
		std::cout << pstr << std::endl;

		pReq = new REQUEST;
		if (pReq == NULL)
		{
			// �������
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

		// ����client���̣�����request
		//std::thread clientRequestThread{ ClientThread, pReq };
		//clientRequestThread.detach();
		//�����̳߳��������
		pool.commit(ClientThread, pReq);

	} //while
	// �ȴ��߳̽���   INFINITE���������Ƶȴ���
	WaitForSingleObject((HANDLE)pHttpProtocol->m_hExit, INFINITE);
	// �ȴ�����client���̽���
	dwRet = WaitForSingleObject(hNoClients, 5000);
	if (dwRet == WAIT_TIMEOUT)
	{
		// ��ʱ���أ�����ͬ������δ�˳�
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

	pHttpProtocol->CountUp();			// ����

	// ����request data
	if (!pHttpProtocol->RecvRequest(pReq, buf, sizeof(buf)))
	{
		pHttpProtocol->Disconnect(pReq);
		delete pReq;
		pHttpProtocol->CountDown();
		return 0;
	}

	// ����request��Ϣ
	nRet = pHttpProtocol->Analyze(pReq, buf);
	if (nRet)
	{
		// �������
		std::string pStr = "Error occurs when analyzing client request";
		std::cout << pStr << std::endl;

		pHttpProtocol->Disconnect(pReq);
		delete pReq;
		pHttpProtocol->CountDown();
		return 0;
	}

	// ���ɲ�����ͷ��
	pHttpProtocol->SendHeader(pReq);

	// ��client��������
	if (pReq->nMethod == METHOD_GET)
	{
		pHttpProtocol->SendFile(pReq);
	}

	pHttpProtocol->Disconnect(pReq);
	delete pReq;
	pHttpProtocol->CountDown();	// client������1

	return 0;
}

bool HttpProtocol::RecvRequest(PREQUEST pReq, LPBYTE pBuf, DWORD dwBufSize)
{
	WSABUF			wsabuf;		// ����/���ջ������ṹ
	WSAOVERLAPPED	over;		// ָ������ص�����ʱָ����WSAOVERLAPPED�ṹ
	DWORD			dwRecv;
	DWORD			dwFlags;
	DWORD			dwRet;
	HANDLE			hEvents[2];
	bool			fPending;
	int				nRet;
	memset(pBuf, 0, dwBufSize);	// ��ʼ��������
	wsabuf.buf = (char*)pBuf;
	wsabuf.len = dwBufSize;	// �������ĳ���
	over.hEvent = WSACreateEvent();	// ����һ���µ��¼�����
	dwFlags = 0;
	fPending = FALSE;
	// ��������
	nRet = WSARecv(pReq->Socket, &wsabuf, 1, &dwRecv, &dwFlags, &over, NULL);
	if (nRet != 0)
	{
		// �������WSA_IO_PENDING��ʾ�ص������ɹ�����
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			// �ص�����δ�ܳɹ�
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
		// �ص�����δ���
		if (!WSAGetOverlappedResult(pReq->Socket, &over, &dwRecv, FALSE, &dwFlags))
		{
			CloseHandle(over.hEvent);
			return false;
		}
	}
	pReq->dwRecv += dwRecv;	// ͳ�ƽ�������
	CloseHandle(over.hEvent);
	return true;
}
void HttpProtocol::Disconnect(PREQUEST pReq)
{
	// �ر��׽��֣��ͷ���ռ�е���Դ
	int	nRet;
	std::cout << "Closing socket: " << pReq->Socket << std::endl;
	nRet = closesocket(pReq->Socket);
	if (nRet == SOCKET_ERROR)
	{
		// �������
		std::cout << "closesocket() error: %d"<< WSAGetLastError() << std::endl;
	}

	//HTTPSTATS	stats;
	//stats.dwRecv = pReq->dwRecv;
	//stats.dwSend = pReq->dwSend;
	//SendMessage(m_hwndDlg, DATA_MSG, (UINT)&stats, NULL);
}

// ����request��Ϣ
int HttpProtocol::Analyze(PREQUEST pReq, LPBYTE pBuf)
{
	// �������յ�����Ϣ
	char szSeps[] = " \n";
	char* cpToken;
	// ��ֹ�Ƿ�����
	if (strstr((const char*)pBuf, "..") != NULL)
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_BADREQUEST);
		return 1;
	}

	// �ж�ruquest��mothed	
	cpToken = strtok((char*)pBuf, szSeps);	// �������ַ����ֽ�Ϊһ���Ǵ���	
	if (!_stricmp(cpToken, "GET"))			// GET����
	{
		pReq->nMethod = METHOD_GET;
	}
	else if (!_stricmp(cpToken, "HEAD"))	// HEAD����
	{
		pReq->nMethod = METHOD_HEAD;
	}
	else
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_NOTIMPLEMENTED);
		return 1;
	}

	// ��ȡRequest-URI
	cpToken = strtok(NULL, szSeps);
	if (cpToken == NULL)
	{
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_BADREQUEST);
		return 1;
	}

	strcpy(pReq->szFileName, m_strRootDir);
	if (strlen(cpToken) > 1)
	{
		strcat(pReq->szFileName, cpToken);	// �Ѹ��ļ�����ӵ���β���γ�·��
	}
	else
	{
		strcat(pReq->szFileName, "/index.html");
	}
	return 0;
}

// ����ͷ��
void HttpProtocol::SendHeader(PREQUEST pReq)
{

	int n = FileExist(pReq);
	if (!n)		// �ļ������ڣ��򷵻�
	{
		return;
	}

	char Header[2048] = " ";
	char curTime[50] = " ";
	GetCurentTime((char*)curTime);
	// ȡ���ļ�����
	DWORD length;
	length = GetFileSize(pReq->hFile, NULL);
	// ȡ���ļ���last-modifiedʱ��
	char last_modified[60] = " ";
	GetLastModified(pReq->hFile, (char*)last_modified);
	// ȡ���ļ�������
	char ContenType[50] = " ";
	GetContenType(pReq, (char*)ContenType);

	sprintf((char*)Header, "HTTP/1.0 %s\r\nDate: %s\r\nServer: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nLast-Modified: %s\r\n\r\n",
		HTTP_STATUS_OK,
		curTime,				// Date
		"My Http Server",       // Server
		ContenType,				// Content-Type
		length,					// Content-length
		last_modified);			// Last-Modified

// ����ͷ��
	send(pReq->Socket, Header, strlen(Header), 0);
}

// �����ļ�
void HttpProtocol::SendFile(PREQUEST pReq)
{
	int n = FileExist(pReq);
	if (!n)			// �ļ������ڣ��򷵻�
	{
		return;
	}

	std::cout << &pReq->szFileName[strlen(m_strRootDir)] << std::endl;

	static BYTE buf[2048];
	DWORD  dwRead;
	BOOL   fRet;
	int flag = 1;
	// ��д����ֱ�����
	while (1)
	{
		// ��file�ж��뵽buffer��        
		fRet = ReadFile(pReq->hFile, buf, sizeof(buf), &dwRead, NULL);
		if (!fRet)
		{

			static char szMsg[512];
			wsprintf((LPWSTR)szMsg, (LPCWSTR)"%s", HTTP_STATUS_SERVERERROR);
			// ��ͻ��˷��ͳ�����Ϣ
			send(pReq->Socket, szMsg, strlen(szMsg), 0);
			break;
		}
		// ���
		if (dwRead == 0)
		{
			break;
		}
		// ��buffer���ݴ��͸�client
		if (!SendBuffer(pReq, buf, dwRead))
		{
			break;
		}
		pReq->dwSend += dwRead;
	}
	// �ر��ļ�
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
		std::cout << "�ļ���ʧ��" << std::endl;
		strcpy(pReq->StatuCodeReason, HTTP_STATUS_NOTFOUND);
		std::cout << "�����ļ������ڣ��ļ���" << pReq->szFileName << std::endl;
		return 0;
	}
	else
	{
		
		return 1;
	}
}
bool HttpProtocol::SendBuffer(PREQUEST pReq, LPBYTE pBuf, DWORD dwBufSize)
{
	// ���ͻ����е�����
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

	// �������� 
	nRet = WSASend(pReq->Socket, &wsabuf, 1, &dwRecv, 0, &over, NULL);
	if (nRet != 0)
	{
		// �������
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
	if (fPending)	// i/oδ���
	{
		hEvents[0] = over.hEvent;
		hEvents[1] = pReq->hExit;
		dwRet = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
		if (dwRet != 0)
		{
			CloseHandle(over.hEvent);
			return false;
		}
		// �ص�����δ���
		if (!WSAGetOverlappedResult(pReq->Socket, &over, &dwRecv, FALSE, &dwFlags))
		{
			// ������
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
	// ����"no client"�¼�����
	None = CreateEvent(NULL, TRUE, TRUE, NULL);
	return None;
}
void HttpProtocol::CountUp()
{
	// �����ų���
	cout_lock.lock();
	ClientNum++;
	// �뿪�ų���
	cout_lock.unlock();
}
void HttpProtocol::CountDown()
{
	// �����ų���
	cout_lock.lock();
	if (ClientNum > 0)
	{
		ClientNum--;
	}
	// �뿪�ų���
	cout_lock.unlock();
}
void HttpProtocol::DeleteClientCount()
{
	CloseHandle(None);
}
// �����ʱ��
void HttpProtocol::GetCurentTime(LPSTR lpszString)
{
	// �����ʱ��
	SYSTEMTIME st;
	GetLocalTime(&st);
	// ʱ���ʽ��
	sprintf(lpszString, "%s %02d %s %d %02d:%02d:%02d GMT", week[st.wDayOfWeek], st.wDay, month[st.wMonth - 1],
		st.wYear, st.wHour, st.wMinute, st.wSecond);
}
bool HttpProtocol::GetLastModified(HANDLE hFile, LPSTR lpszString)
{
	// ����ļ���last-modified ʱ��
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stCreate;
	FILETIME ftime;
	// ����ļ���last-modified��UTCʱ��
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		return false;
	FileTimeToLocalFileTime(&ftWrite, &ftime);
	// UTCʱ��ת���ɱ���ʱ��
	FileTimeToSystemTime(&ftime, &stCreate);
	// ʱ���ʽ��
	sprintf(lpszString, "%s %02d %s %d %02d:%02d:%02d GMT", week[stCreate.wDayOfWeek],
		stCreate.wDay, month[stCreate.wMonth - 1], stCreate.wYear, stCreate.wHour,
		stCreate.wMinute, stCreate.wSecond);
	return TRUE;
}
bool HttpProtocol::GetContenType(PREQUEST pReq, LPSTR type)
{
	// ȡ���ļ�������
	char* cpToken;
	cpToken = strstr(pReq->szFileName, ".");
	strcpy(pReq->postfix, cpToken);
	// �����������ļ����Ͷ�Ӧ��content-type
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