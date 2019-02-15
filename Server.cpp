#include "stdafx.h"
#include "Server.h"
#include "FTPServer.h"

Server::Server()
{
	m_serverip = "localhost";
	m_serverport = 21; //���ü���21�Ŷ˿�
	m_welcomeInfor = "Welcome to our FTP Server��";//���û�ӭ��Ϣ
	m_goodbyeInfor = "Good Bye!";//���ý�����Ϣ
	m_maxlink = 10;//�����û��������
	m_timeout = 10;//��������ʱ��
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

	//���ڴ������ڵĺ������������Ϳ������ص����������ڣ����Ӵ��ڣ�������ʽ�ɲ���dwExStyleָ����
	if (!CWnd::CreateEx(0, AfxRegisterWndClass(0), _T("FTP Server Notification Sink"), WS_POPUP, 0, 0, 0, 0, NULL, 0))
	{

		return FALSE;//Ϊ��Ϣ·�ɴ������ⴰ��
	}
	//����һ�������˿�
	//CSocket����create()���������׽���
	/*BOOL CSocket::Create(
		UINT nSocketPort = 0;  ָ��������׽��ֵĶ˿ں�
		int nSocketType = SOCK_STREAM;  ָ�������׽��ֵ�����
		LPCTSTR lpszSocketAddress = NULL;  Ϊ�׽���ָ���ĵ�ַ��ָ��һ�����ʮ���Ƶ�IP��ַ����һ��DNS����
	)*/
	if (m_cListenSocket.Create(m_serverport, SOCK_STREAM, FD_CLOSE | FD_ACCEPT, m_serverip))
	{

		if (m_cListenSocket.Listen())//��ʼ����
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

	// ֹͣͳ�Ƽ�ʱ��


	m_serverState = FALSE;
	m_cListenSocket.Close();

	CConnectThread* pThread = NULL;

	//�ر������߳�
	do
	{
		m_CriticalSection.Lock();

		POSITION pos = m_ServerThreadList.GetHeadPosition();

		if (pos != NULL)
		{
			pThread = (CConnectThread *)m_ServerThreadList.GetAt(pos);

			m_CriticalSection.Unlock();
			// �����̳߳�Ա
			int nThreadID = pThread->m_nThreadID;
			HANDLE hThread = pThread->m_hThread;
			AddTraceLine(0, "[%d] Shutting down thread...", nThreadID);

			// ֪ͨ�߳�ֹͣ
			pThread->SetThreadPriority(THREAD_PRIORITY_HIGHEST);
			pThread->PostThreadMessage(WM_QUIT, 0, 0);
			//�ȴ��߳���ֹ��ͬʱ������Ϣ�ã����5s��
			if (WaitWithMessageLoop(hThread, 5) == FALSE)
			{
				// �̲߳�����ֹ
				//AddTraceLine(0, "[%d] Problem while killing thread.", nThreadID);
				//������һ�Σ�����ɾ��
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
	//���·�����״̬
	AddTraceLine(0, "FTP Server stopped.");
	if (IsWindow(m_hWnd))
		DestroyWindow();
	m_hWnd = NULL;
	return TRUE;
}
BOOL Server::WaitWithMessageLoop(HANDLE hEvent, int nTimeout)//�ڵȴ��¼�ʱ������Ϣ����
{
	DWORD dwRet;

	while (1)
	{
		// �ȴ��¼�����Ϣ���������Ϣ�������������ص��ȴ�״̬
		dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE, nTimeout, QS_ALLINPUT);
		if (dwRet == WAIT_OBJECT_0)
		{
			TRACE0("WaitWithMessageLoop() event triggered.\n");
			return TRUE;
		}
		else
			if (dwRet == WAIT_OBJECT_0 + 1)
			{
				// ��������Ϣ
				AfxGetApp()->PumpMessage();//�����̵߳���Ϣѭ����
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
LRESULT Server::OnThreadStart(WPARAM wParam, LPARAM)//һ���߳��Ѿ���ʼʱ����
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
	pThread->m_connectSocket.GetPeerName(pThread->m_strRemoteHost, port);//ͨ��getpeername()��������ȡ��ǰ���ӵĿͻ��˵�IP��ַ�Ͷ˿ںš�
	AddTraceLine(0, "[%d] Client disconnected from %s.", pThread->m_nThreadID, "FTP");
	m_linkcount--;
	return TRUE;
}
LRESULT Server::OnThreadMessage(WPARAM wParam, LPARAM lParam)//
{

	return TRUE;
}
void Server::Initialize(FtpEventSink *pEventSink)//��ʼ���¼�����
{
	m_pEventSink = pEventSink;
}

void Server::AddTraceLine(int nType, LPCTSTR pstrFormat, ...)
{
	CString str;
	// �õ���ʽ��д����
	va_list args;
	va_start(args, pstrFormat);
	str.FormatV(pstrFormat, args);
	m_pEventSink->OnFTPStatusChange(nType, str);
}
