#include "stdafx.h"
#include "Server.h"
#include "FTPServer.h"

Server::Server()
{
	m_serverip = "localhost";
	m_serverport = 21; //设置监听21号端口
	m_welcomeInfor = "Welcome to our FTP Server！";//设置欢迎信息
	m_goodbyeInfor = "Good Bye!";//设置结束信息
	m_maxlink = 10;//设置用户最大数量
	m_timeout = 10;//设置连接时限
	m_linkcount = 0;
	m_serverState = FALSE;
}


Server::~Server()
{
}
BEGIN_MESSAGE_MAP(Server, CWnd)
	ON_MESSAGE(WM_THREADSTART, OnThreadStart)
	ON_MESSAGE(WM_THREADSTOP, OnThreadStop)
	ON_MESSAGE(WM_THREADMESSAGE, OnThreadMessage)
END_MESSAGE_MAP()

BOOL Server::ServerStart()
{
	if (m_serverState) {

		return FALSE;
	}

	//用于创建窗口的函数。窗口类型可以是重叠，弹出窗口，或子窗口，窗口样式由参数dwExStyle指定。
	if (!CWnd::CreateEx(0, AfxRegisterWndClass(0), _T("FTP Server Notification Sink"), WS_POPUP, 0, 0, 0, 0, NULL, 0))
	{

		return FALSE;//为消息路由创建虚拟窗口
	}
	//创建一个监听端口
	//CSocket调用create()方法创建套接字
	/*BOOL CSocket::Create(
		UINT nSocketPort = 0;  指定分配给套接字的端口号
		int nSocketType = SOCK_STREAM;  指定创建套接字的类型
		LPCTSTR lpszSocketAddress = NULL;  为套接字指定的地址，指向一个点分十进制的IP地址或者一个DNS域名
	)*/
	if (m_cListenSocket.Create(m_serverport, SOCK_STREAM, FD_CLOSE | FD_ACCEPT, m_serverip))
	{

		if (m_cListenSocket.Listen())//开始监听
		{
			m_cListenSocket.parentCServer = this;
			m_serverState = TRUE;
			AddTraceLine(0, "FTP Server started on port %d.", m_serverport);
			return TRUE;
		}
	}
	AddTraceLine(0, "FTP Server failed to listen on port %d.", m_serverport);
	return FALSE;
}
BOOL Server::ServerStop()
{
	if (!m_serverState)
		return FALSE;

	// 停止统计计时器


	m_serverState = FALSE;
	m_cListenSocket.Close();

	CConnectThread* pThread = NULL;

	//关闭所有线程
	do
	{
		m_CriticalSection.Lock();

		POSITION pos = m_ServerThreadList.GetHeadPosition();

		if (pos != NULL)
		{
			pThread = (CConnectThread *)m_ServerThreadList.GetAt(pos);

			m_CriticalSection.Unlock();
			// 保存线程成员
			int nThreadID = pThread->m_nThreadID;
			HANDLE hThread = pThread->m_hThread;
			AddTraceLine(0, "[%d] Shutting down thread...", nThreadID);

			// 通知线程停止
			pThread->SetThreadPriority(THREAD_PRIORITY_HIGHEST);
			pThread->PostThreadMessage(WM_QUIT, 0, 0);
			//等待线程终止，同时保持消息泵（最大5s）
			if (WaitWithMessageLoop(hThread, 5) == FALSE)
			{
				// 线程不能终止
				//AddTraceLine(0, "[%d] Problem while killing thread.", nThreadID);
				//不再试一次，所以删除
				m_CriticalSection.Lock();
				POSITION rmPos = m_ServerThreadList.Find(pThread);
				if (rmPos != NULL)
					m_ServerThreadList.RemoveAt(rmPos);
				m_CriticalSection.Unlock();
			}
			else
			{
				AddTraceLine(0, "[%d] Thread successfully stopped.", nThreadID);
			}
		}
		else
		{
			m_CriticalSection.Unlock();
			pThread = NULL;
		}
	} while (pThread != NULL);
	//更新服务器状态
	AddTraceLine(0, "FTP Server stopped.");
	if (IsWindow(m_hWnd))
		DestroyWindow();
	m_hWnd = NULL;
	return TRUE;
}
BOOL Server::WaitWithMessageLoop(HANDLE hEvent, int nTimeout)//在等待事件时发送消息函数
{
	DWORD dwRet;

	while (1)
	{
		// 等待事件或消息，如果是消息，则处理它并返回到等待状态
		dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, nTimeout, QS_ALLINPUT);
		if (dwRet == WAIT_OBJECT_0)
		{
			TRACE0("WaitWithMessageLoop() event triggered.\n");
			return TRUE;
		}
		else
			if (dwRet == WAIT_OBJECT_0 + 1)
			{
				// 处理窗口消息
				AfxGetApp()->PumpMessage();//包含线程的消息循环。
			}
			else
				if (dwRet == WAIT_TIMEOUT)
				{
					// timed out !
					return FALSE;
				}
				else
				{
					// WAIT_ABANDONED_0 ...
					return TRUE;
				}
	}
}
/////////////////////////////////////////////////////////////////////////////
// Sever message handlers
LRESULT Server::OnThreadStart(WPARAM wParam, LPARAM)//一个线程已经开始时调用
{
	CConnectThread *pThread = (CConnectThread *)wParam;

	UINT port;
	pThread->m_connectSocket.GetPeerName(pThread->m_strRemoteHost, port);
	AddTraceLine(0, "[%d] Client connected from %s.", pThread->m_nThreadID, pThread->m_strRemoteHost);
	m_linkcount++;

	return TRUE;
}
LRESULT Server::OnThreadStop(WPARAM wParam, LPARAM lParam)
{

	UINT port;
	CConnectThread *pThread = (CConnectThread *)wParam;
	pThread->m_connectSocket.GetPeerName(pThread->m_strRemoteHost, port);//通过getpeername()函数来获取当前连接的客户端的IP地址和端口号。
	AddTraceLine(0, "[%d] Client disconnected from %s.", pThread->m_nThreadID, "FTP");
	m_linkcount--;
	return TRUE;
}
LRESULT Server::OnThreadMessage(WPARAM wParam, LPARAM lParam)//
{

	return TRUE;
}
void Server::Initialize(FtpEventSink *pEventSink)//初始化事件接收
{
	m_pEventSink = pEventSink;
}

void Server::AddTraceLine(int nType, LPCTSTR pstrFormat, ...)
{
	CString str;
	// 得到格式和写数据
	va_list args;
	va_start(args, pstrFormat);
	str.FormatV(pstrFormat, args);
	m_pEventSink->OnFTPStatusChange(nType, str);
}
