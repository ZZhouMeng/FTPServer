#pragma once
#include <afxsock.h>
class connectSocket :
	public CAsyncSocket
{
public:
	connectSocket();
	virtual ~connectSocket();
	CWnd* parentCServer;
//	virtual void AssertValid() const;
	virtual void OnAccept(int nErrorCode);
};

