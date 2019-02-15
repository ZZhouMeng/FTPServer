#pragma once
class FtpEventSink
{
public:
	FtpEventSink();
	virtual ~FtpEventSink();
	virtual void OnFTPUserConnected(DWORD nThreadID, LPCTSTR lpszUser, LPCSTR lpszAddress) {};
	virtual void OnFTPUserDisconnected(DWORD nThreadID, LPCTSTR lpszUser) {};
	virtual void OnFTPStatusChange(int nType, LPCTSTR lpszText) {};
	virtual void OnFTPReceivedBytesChange(int nBytes) {};
	virtual void OnFTPSentBytesChange(int nBytes) {};
	virtual void OnFTPStatisticChange(int nType, int nValue) {};
};

