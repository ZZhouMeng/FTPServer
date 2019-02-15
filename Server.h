#pragma once
#include "connectThread.h"
#include "connectSocket.h"
#include "resource.h"
#include "stdafx.h"
#include "ServerAction.h"
#include "FtpEventSink.h"



class Server :
	public CWnd
{
public:
	Server();
	virtual ~Server();

public:
	BOOL m_serverState;
	CString	m_serverip;
	UINT	m_serverport;
	CString	m_welcomeInfor;
	CString	m_goodbyeInfor;
	int     m_maxlink;
	int		m_timeout;
	int     m_linkcount;
	ServerAction cSPro;
	connectSocket m_cListenSocket;
	CCriticalSection m_CriticalSection;//控制链表
	CTypedPtrList <CObList, CConnectThread *>m_ServerThreadList;//线程池
public:
	BOOL ServerStart();
	BOOL ServerStop();
	LRESULT OnThreadStart(WPARAM wParam, LPARAM);
	LRESULT OnThreadStop(WPARAM wParam, LPARAM lParam);
	LRESULT OnThreadMessage(WPARAM wParam, LPARAM lParam);
	BOOL WaitWithMessageLoop(HANDLE hEvent, int nTimeout);
	FtpEventSink *m_pEventSink;
	void Initialize(FtpEventSink *pEventSink);
	void AddTraceLine(int nType, LPCTSTR pstrFormat, ...);
protected:
	DECLARE_MESSAGE_MAP()
};

