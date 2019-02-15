// CConnectThread.cpp : implementation file
//

#include "stdafx.h"
#include "FTPServer.h"
#include "connectThread.h"
#include "Server.h"

//Cobject派生类的对象在运行时动态被创建。
IMPLEMENT_DYNCREATE(CConnectThread, CWinThread)
extern Server cServer;
CConnectThread::CConnectThread()
{
}

CConnectThread::~CConnectThread()
{
}

BOOL CConnectThread::InitInstance()
{
	try
	{
		//必须通过Detach()得到HANDLE之后在工作线程中重新Attach()才行
		m_connectSocket.Attach(m_handleListenSocket);
		m_connectSocket.m_parentThread = this;

		CString peerIpAddress;
		UINT peerPort;
		//通过getpeername()函数来获取当前连接的客户端的IP地址和端口号。
		m_connectSocket.GetPeerName(peerIpAddress, peerPort);

		m_parentWindowServer->SendMessage(WM_THREADSTART, (WPARAM)this, 0);

		//	char * welcomeInfor =cServer.CString2Charx("220 "+cServer.m_welcomeInfor + "\r\n");
		CString temp = "220 " + cServer.m_welcomeInfor + "\r\n";
		if (send(m_connectSocket, (temp), strlen(temp), 0) == SOCKET_ERROR)
		{

			//printf("Ftp client error:%d\n", WSAGetLastError());
			return FALSE;
		}
		//send(m_connectSocket, "501 aaa\r\n", strlen("501 aaa\r\n"), 0) == SOCKET_ERROR;
	}
	catch (CException *e)
	{
		e->Delete();
	}
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CConnectThread::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	Server *pWnd = (Server*)m_parentWindowServer;
	try
	{
		pWnd->m_CriticalSection.Lock();
		//从链表中删除当前线程
		POSITION pos = pWnd->m_ServerThreadList.Find(this);
		if (pos != NULL)
		{
			pWnd->m_ServerThreadList.RemoveAt(pos);
		}
		pWnd->m_CriticalSection.Unlock();
		//通知服务主循环
		pWnd->SendMessage(WM_THREADSTOP, (WPARAM)this, 0);
	}
	catch (CException *e)
	{
		pWnd->m_CriticalSection.Unlock();
		e->Delete();
	}
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CConnectThread, CWinThread)
	//{{AFX_MSG_MAP(CConnectThread)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_THREAD_MESSAGE(WM_THREADMESSAGE, OnThreadMessage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectThread message handlers
//自定义消息WM_THREADMSG的响应函数
void CConnectThread::OnThreadMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case 0: // destroy data socket
		m_connectSocket.DestroyDataConnection();
		break;
	case 1: // quit !
		PostThreadMessage(WM_QUIT, 0, 0);//关闭线程
		break;
	default:
		break;
	}
	//return 0L;
}
void CConnectThread::UpdateStatistic(int nType)
{
	// 通知server class
	m_parentWindowServer->PostMessage(WM_THREADMSG, (WPARAM)2, (LPARAM)nType);//向主窗口发送消息
	//该函数将一个消息放入（寄送）到与指定窗口创建的线程相联系消息队列里，不等待线程处理消息就返回，是异步消息模式。消息队列里的消息通过调用GetMessage和PeekMessage取得。
}