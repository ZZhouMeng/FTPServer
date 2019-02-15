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
	// TODO: 在此添加专用代码和/或调用基类

	//正在建立新连接
	cSocket socket;//接收连接使用一个临时的CSocket对象
	Accept(socket);//创建一个线程来处理连接。
	CConnectThread* conThread = (CConnectThread*)AfxBeginThread(RUNTIME_CLASS(CConnectThread), THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	if (!conThread)
	{
		socket.Close();
		TRACE("Could not create thread\n");
		return;
	}
	Server *cServer = (Server*)parentCServer; 
	//AfxMessageBox("线程创建成功！"); 
	cServer->m_CriticalSection.Lock();
	cServer->m_ServerThreadList.AddTail(conThread);
	cServer->m_CriticalSection.Unlock();// 保存指针
	conThread->m_parentWindowServer = cServer;	// 通过套接字到线程通过套接字处理。
	conThread->m_handleListenSocket = socket.Detach();
	conThread->ResumeThread();// 现在启动线程。  
	CAsyncSocket::OnAccept(nErrorCode);
}
