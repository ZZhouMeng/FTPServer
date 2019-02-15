#include "stdafx.h"
#include "connectSocket.h"
#include "cSocket.h"
#include "Server.h"
#include "connectThread.h"

connectSocket::connectSocket()
{
}


connectSocket::~connectSocket()
{
}




void connectSocket::OnAccept(int nErrorCode)
{
	// TODO: �ڴ����ר�ô����/����û���

	//���ڽ���������
	cSocket socket;//��������ʹ��һ����ʱ��CSocket����
	Accept(socket);//����һ���߳����������ӡ�
	CConnectThread* conThread = (CConnectThread*)AfxBeginThread(RUNTIME_CLASS(CConnectThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!conThread)
	{
		socket.Close();
		TRACE("Could not create thread\n");
		return;
	}
	Server *cServer = (Server*)parentCServer; 
	//AfxMessageBox("�̴߳����ɹ���"); 
	cServer->m_CriticalSection.Lock();
	cServer->m_ServerThreadList.AddTail(conThread);
	cServer->m_CriticalSection.Unlock();// ����ָ��
	conThread->m_parentWindowServer = cServer;	// ͨ���׽��ֵ��߳�ͨ���׽��ִ���
	conThread->m_handleListenSocket = socket.Detach();
	conThread->ResumeThread();// ���������̡߳�  
	CAsyncSocket::OnAccept(nErrorCode);
}
