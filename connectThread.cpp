// CConnectThread.cpp : implementation file
//

#include "stdafx.h"
#include "FTPServer.h"
#include "connectThread.h"
#include "Server.h"

//Cobject������Ķ���������ʱ��̬��������
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
		//����ͨ��Detach()�õ�HANDLE֮���ڹ����߳�������Attach()����
		m_connectSocket.Attach(m_handleListenSocket);
		m_connectSocket.m_parentThread = this;

		CString peerIpAddress;
		UINT peerPort;
		//ͨ��getpeername()��������ȡ��ǰ���ӵĿͻ��˵�IP��ַ�Ͷ˿ںš�
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
		//��������ɾ����ǰ�߳�
		POSITION pos = pWnd->m_ServerThreadList.Find(this);
		if (pos != NULL)
		{
			pWnd->m_ServerThreadList.RemoveAt(pos);
		}
		pWnd->m_CriticalSection.Unlock();
		//֪ͨ������ѭ��
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
//�Զ�����ϢWM_THREADMSG����Ӧ����
void CConnectThread::OnThreadMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case 0: // destroy data socket
		m_connectSocket.DestroyDataConnection();
		break;
	case 1: // quit !
		PostThreadMessage(WM_QUIT, 0, 0);//�ر��߳�
		break;
	default:
		break;
	}
	//return 0L;
}
void CConnectThread::UpdateStatistic(int nType)
{
	// ֪ͨserver class
	m_parentWindowServer->PostMessage(WM_THREADMSG, (WPARAM)2, (LPARAM)nType);//�������ڷ�����Ϣ
	//�ú�����һ����Ϣ���루���ͣ�����ָ�����ڴ������߳�����ϵ��Ϣ��������ȴ��̴߳�����Ϣ�ͷ��أ����첽��Ϣģʽ����Ϣ���������Ϣͨ������GetMessage��PeekMessageȡ�á�
}