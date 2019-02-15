#pragma once
#include <afxwin.h>
#include "cSocket.h"
//#include"Server.h"

class CConnectThread : public CWinThread
{
	DECLARE_DYNCREATE(CConnectThread)
protected:
	CConnectThread();           // protected constructor used by dynamic creation

// Attributes
public:
	cSocket m_connectSocket;
	SOCKET m_handleListenSocket;
	CWinThread* m_cwThread;
	int m_handleListenSocketIndex;
	CString m_strRemoteHost;
	void UpdateStatistic(int nType);

	CWnd* m_parentWindowServer;
	
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	virtual ~CConnectThread();

	void OnThreadMessage(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};