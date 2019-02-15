#include "stdafx.h"
#include "dataSocket.h"
#include "cSocket.h"
#include "connectThread.h"
#include"Resource.h"
#define PACKET_SIZE 4096
dataSocket::dataSocket(cSocket *pSocket, int nTransferType)
{
	m_nTransferType = nTransferType;
	m_pConnectSocket = pSocket;
	m_nStatus = XFERMODE_IDLE;
	m_strData = "";
	m_File.m_hFile = NULL;
	m_bConnected = FALSE;
	m_dwRestartOffset = 0;
	m_bInitialized = FALSE;
}


dataSocket::~dataSocket()
{
}
void dataSocket::SetRestartOffset(DWORD dwOffset)//设置启动时间
{
	m_dwRestartOffset = dwOffset;
}

void dataSocket::SetData(LPCTSTR lpszData)
{
	m_strData = lpszData;
	m_nTotalBytesSend = m_strData.GetLength();
	m_nTotalBytesTransfered = 0;
}

void dataSocket::SetTransferType(int nType, BOOL bWaitForAccept)
{
	m_nTransferType = nType;
	//这个判断一定不能少，m_bConnected初始化为FALSE,在OnAccept()函数中接受了客户端的连接后才置为TRUE;
	//没有这个判断，程序的执行会跳过OnAccept()函数，没有客户端连接的情况下想发送数据时错误的。
	if (bWaitForAccept && !m_bConnected)
	{
		m_bInitialized = FALSE;
		return;
	}
	if (m_bConnected && m_nTransferType != -1)
		m_pConnectSocket->ResToClient("150 Connection accepted");
	//AfxMessageBox(_T("150 Connection accepted"));
	m_bInitialized = TRUE;

	switch (m_nTransferType)
	{
	case 0:// List Directory
	{
		m_nStatus = XFERMODE_LIST;
		OnSend(0);
	}
	break;

	case 1:	// Send File
		if (PrepareSendFile(m_strData))
		{
			m_nStatus = XFERMODE_SEND;
			m_bConnected = TRUE;
			OnSend(0);
		}
		else
		{
			Close();
		}
		break;

	case 2://Receive File
		if (PrepareReceiveFile(m_strData))
		{
			m_nStatus = XFERMODE_RECEIVE;
			m_bConnected = TRUE;
			//OnSend(0);
		}
		else
		{
			Close();
			m_pConnectSocket->ResToClient("450 Can't access file.");
			//destroy this socket
			AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
			//upload failed
			//((CConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);
		}
		break;

	default:
		m_bInitialized = FALSE;
		break;

	}
}
BOOL dataSocket::PrepareSendFile(LPCTSTR lpszFilename)
{
	//close file if it's already open
	if (m_File.m_hFile != NULL)
	{
		m_File.Close();
	}
	//open source file
	if (!m_File.Open(lpszFilename, CFile::modeRead | CFile::typeBinary))
	{
		return FALSE;
	}
	m_nTotalBytesSend = m_File.GetLength();
	if (m_dwRestartOffset < m_nTotalBytesSend)
		m_nTotalBytesTransfered = m_dwRestartOffset;
	else
		m_nTotalBytesTransfered = 0;
	return TRUE;
}
BOOL dataSocket::PrepareReceiveFile(LPCTSTR lpszFilename)
{
	//close file if it's already open
	if (m_File.m_hFile != NULL)
	{
		m_File.Close();
	}

	//open destination file
	if (!m_File.Open(lpszFilename, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::shareDenyWrite))
	{
		return FALSE;
	}
	m_nTotalBytesReceive = 0;
	m_nTotalBytesTransfered = 0;

	if (m_dwRestartOffset)
	{
		m_File.SetLength(m_dwRestartOffset);
		m_File.SeekToEnd();
	}
	return TRUE;
}
void dataSocket::OnSend(int nErrorCode)
{
	CAsyncSocket::OnSend(nErrorCode);
	switch (m_nStatus)
	{
	case XFERMODE_LIST:
	{
		while (m_nTotalBytesTransfered < m_nTotalBytesSend)
		{
			DWORD dwRead;
			int dwBytes;

			CString strData;

			dwRead = m_strData.GetLength();

			if (dwRead <= PACKET_SIZE)
			{
				strData = m_strData;
			}
			else
			{
				strData = m_strData.Left(PACKET_SIZE);
				dwRead = strData.GetLength();
			}

			if ((dwBytes = Send(strData, dwRead)) == SOCKET_ERROR)
			{
				if (GetLastError() == WSAEWOULDBLOCK)
				{
					Sleep(0);
					break;
				}
				else
				{
					TCHAR szError[256];
					wsprintf(szError, "Server Socket failed to send: %d", GetLastError());

					// close the data connection.
					Close();

					m_nTotalBytesSend = 0;
					m_nTotalBytesTransfered = 0;

					// change status
					m_nStatus = XFERMODE_IDLE;

					m_pConnectSocket->ResToClient("426 Connection closed; transfer aborted.");

					// destroy this socket
					AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
				}
			}
			else
			{
				m_nTotalBytesTransfered += dwBytes;

				m_strData = m_strData.Mid(dwBytes);

			}
		}
		if (m_nTotalBytesTransfered == m_nTotalBytesSend)
		{
			// close the data connection.
			Close();  //只是关闭了连接并没有销毁m_pDataSocket对象，所以在有些地方要判断一下是否销毁m_pDataSocket对象

			m_nTotalBytesSend = 0;
			m_nTotalBytesTransfered = 0;

			// change status
			m_nStatus = XFERMODE_IDLE;

			// tell the client the transfer is complete.
			//刚开始这句话注释掉了，客户端在收到数据后没有显示文件列表信息就是因为没有收到这句
			//响应的消息，客户端在收到了响应的消息后才会显示列表信息。
			m_pConnectSocket->ResToClient("226 Transfer complete");//响应不能丢。
			// destroy this socket
			AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
		}
		break;
	}

	case XFERMODE_SEND:
	{
		while (m_nTotalBytesTransfered < m_nTotalBytesSend)
		{
			//allocate space to store data
			byte data[PACKET_SIZE];
			m_File.Seek(m_nTotalBytesTransfered, CFile::begin);
			DWORD dwRead = m_File.Read(data, PACKET_SIZE);
			int dwBytes;
			if ((dwBytes = Send(data, dwRead)) == SOCKET_ERROR)
			{
				if (GetLastError() == WSAEWOULDBLOCK)
				{
					Sleep(0);
					break;
				}
				else
				{
					TCHAR szError[256];
					wsprintf(szError, "Server Socket failed to send:%d", GetLastError());
					//close file.
					m_File.Close();
					m_File.m_hFile = NULL;

					//close the data connection.
					Close();

					m_nTotalBytesSend = 0;
					m_nTotalBytesTransfered = 0;

					//change status
					m_nStatus = XFERMODE_IDLE;

					m_pConnectSocket->ResToClient("426 Connection closed;transfer aborted.");

					//destroy this socket
					AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);

					//download failed
					((CConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_DOWNLOADFAILED);
				}
			}
			else
			{
				m_nTotalBytesTransfered += dwBytes;
				//	((CConnectThread *)AfxGetThread())->IncSentBytes(dwBytes);
			}
		}
		if (m_nTotalBytesTransfered == m_nTotalBytesSend)
		{
			//close file.
			m_File.Close();
			m_File.m_hFile = NULL;

			//close the data connection.
			Close();

			m_nTotalBytesSend = 0;
			m_nTotalBytesTransfered = 0;

			//change status
			m_nStatus = XFERMODE_IDLE;

			//tell the client the transfer is complete
			m_pConnectSocket->ResToClient("226 Transfer complete");
			//destroy this socket
			AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
			//download successfull
			((CConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_DOWNLOADSUCCEEDED);
		}
		break;
	}
	}
}

void dataSocket::OnAccept(int nErrorCode)
{
	//接受连接使用一个临时的CSocket对象。
	CAsyncSocket tmpSocket;
	Accept(tmpSocket);

	SOCKET socket = tmpSocket.Detach();
	Close();//关闭监听的CAsyncSocket

	Attach(socket);//把刚才关闭的监听的CAsyncSocket和socket绑定，用来收发数据
	m_bConnected = TRUE;//标记连接标志

	if (!m_bInitialized)
		SetTransferType(m_nTransferType);
	CAsyncSocket::OnAccept(nErrorCode);
}
void dataSocket::OnReceive(int nErrorCode)
{
	CAsyncSocket::OnReceive(nErrorCode);
	Receive();
}
int dataSocket::GetStatus()
{
	return m_nStatus;
}
int dataSocket::Receive()
{
	int nRead = 0;
	if (m_nStatus == XFERMODE_RECEIVE)
	{
		if (m_File.m_hFile == NULL)
			return 0;

		byte data[PACKET_SIZE];//每次读取PACKET_SIZE大小的数据
		nRead = CAsyncSocket::Receive(data, PACKET_SIZE);//一次接收不完怎么办，会分多次发送。

		switch (nRead)
		{
		case 0:
		{
			m_File.Close();
			m_File.m_hFile = NULL;
			Close();
			//tell the client the transfer is complete.
			m_pConnectSocket->ResToClient("226 Transfer complete.");
			//destroy this socket  
			AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
			//upload succesfull
			((CConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADSUCCEEDED);
			break;
		}
		case SOCKET_ERROR:
		{
			if (GetLastError() != WSAEWOULDBLOCK)
			{
				m_File.Close();
				m_File.m_hFile = NULL;
				Close();
				m_pConnectSocket->ResToClient("426 Connection closed;transfer aborted.");
				//destroy this socket
				AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
				//upload failed
				((CConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);
			}
			break;
		}
		default:
		{
			//	((CConnectThread *)AfxGetThread())->IncReceivedBytes(nRead);
			TRY
			{
				m_File.Write(data,nRead);
			}
				CATCH_ALL(e)
			{
				m_File.Close();
				m_File.m_hFile = NULL;
				Close();
				m_pConnectSocket->ResToClient("450 Can't access file.");
				//destroy this socket
				AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
				//upload failed
				((CConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);
				return 0;
			}
			END_CATCH_ALL;
			break;
		}
		}
	}
	return nRead;
}
void dataSocket::OnClose(int nErrorCode)
{
	TRACE0("CDataSocket() OnClose()\n");
	if (m_pConnectSocket)
	{
		// shutdown sends
		ShutDown(1);

		if (m_nStatus == XFERMODE_RECEIVE)
		{
			while (Receive() != 0)
			{
				// receive remaining data				
			}
		}
		else
		{
			m_pConnectSocket->ResToClient("426 Connection closed; transfer aborted.");
			// destroy this socket
			AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
			// upload failed
			((CConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);
		}
	}
	m_nStatus = XFERMODE_IDLE;
	m_bConnected = FALSE;

	CAsyncSocket::OnClose(nErrorCode);
}
void dataSocket::OnConnect(int nErrorCode)
{
	if (nErrorCode)
	{
		m_nStatus = XFERMODE_ERROR;
		m_pConnectSocket->ResToClient("425 Can't open data connection.");
		// destroy this socket
		AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
	}
	else
	{
		switch (m_nTransferType)
		{
		case 0:	// List Directory
			m_nStatus = XFERMODE_LIST;
			m_bConnected = TRUE;
			OnSend(0);
			break;
		case 1:	// Send File
			if (PrepareSendFile(m_strData))
			{
				m_nStatus = XFERMODE_SEND;
				m_bConnected = TRUE;
			}
			else
			{
				Close();
			}
			break;
		case 2:	// Receive File
			if (PrepareReceiveFile(m_strData))
			{
				m_nStatus = XFERMODE_RECEIVE;
				m_bConnected = TRUE;
			}
			else
			{
				Close();
				m_pConnectSocket->ResToClient("450 can't access file.");
				// destroy this socket
				AfxGetThread()->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
				// upload failed
				((CConnectThread *)AfxGetThread())->UpdateStatistic(FTPSTAT_UPLOADFAILED);
			}
			break;
		}
	}
	CAsyncSocket::OnConnect(nErrorCode);
}
