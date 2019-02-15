#pragma once
#include <afxsock.h>
#include "cSocket.h"
#define XFERMODE_IDLE	0
#define XFERMODE_LIST	1
#define XFERMODE_SEND	2
#define XFERMODE_RECEIVE 3
#define XFERMODE_ERROR	4
class dataSocket :
	public CAsyncSocket
{
public:
	virtual ~dataSocket();

public:
	CFile m_File;
	CString m_strData;
	DWORD m_dwRestartOffset;
	DWORD m_nTotalBytesTransfered;
	DWORD m_nTotalBytesReceive;
	DWORD m_nTotalBytesSend;
	int m_nTransferType;
	cSocket* m_pConnectSocket;
	int nTransferType;
	BOOL m_bConnected;
	BOOL m_bInitialized;
	int m_nStatus;
	dataSocket(cSocket* temp, int nTp);

public:
	int GetStatus();
	int Receive();
	BOOL PrepareSendFile(LPCTSTR lpszFilename);
	BOOL PrepareReceiveFile(LPCTSTR lpszFilename);
	void SetData(LPCTSTR lpszData);
	void SetTransferType(int nType, BOOL bWaitForAccept = FALSE);
	void SetRestartOffset(DWORD dwOffset);
	virtual void OnSend(int nErrorCode);
	virtual void OnAccept(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
};

