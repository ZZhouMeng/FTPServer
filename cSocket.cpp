// cSocket.cpp : implementation file
//

#include "stdafx.h"
#include "FTPServer.h"
#include "cSocket.h"
#include "UserInformation.h"
#include "Server.h"
#include "dataSocket.h"

/////////////////////////////////////////////////////////////////////////////
// cSocket
extern Server cServer;
extern bool state;
typedef struct myname
{
	char name[100];
	int year;
}myname;
cSocket::cSocket()
{
	m_bPassiveMode = FALSE;
	m_strRemoteHost = "";
	m_nRemotePort = -1;
	m_dwRestartOffset = 0;
	m_bLoggedon = FALSE;
	m_pDataSocket = NULL;
}

cSocket::~cSocket()
{

}
void cSocket::OnReceive(int nErrorCode)
//ÿ���̶߳���ͨ��һ��CConnectSocket����m_ConnectSocket��������ݵĽ��ܺͷ��͡����̴߳����ɹ��Ժ�
//m_ConnectSocket����ͨ��OnReceive����������ݣ�Ȼ������GetRxLine()��������������FTP����
{

	myname t;
	t.year = 1;
	char buff[4096];
	ZeroMemory(buff, 4096);
	int nRead = Receive(buff, 4096);//�����ݽ��յ���ʱ��������
	if (nRead != SOCKET_ERROR && nRead != 0)
	{
		buff[nRead] = '\0';
		m_RxBuffer += CString(buff);//�����յ���������ӵ�ȫ�ֻ�������
		GetRxLine();
	}
	CSocket::OnReceive(nErrorCode);
}
void cSocket::OnClose(int nErrorCode) {//socket���ر�ʱ��һ����Ӧ

	CSocket::OnClose(nErrorCode);
}
void cSocket::GetRxLine()//�������������к���
{
	CString strTemp;
	int nIndex;
	while (!m_RxBuffer.IsEmpty())//�н��յ������ݴ�����
	{
		//��һ����������Ľ�����
		nIndex = m_RxBuffer.Find("\r\n");
		if (nIndex != -1)
		{
			strTemp = m_RxBuffer.Left(nIndex);//������������ȡ����
			m_RxBuffer = m_RxBuffer.Mid(nIndex + 2);//����m_RxBuffer ȥ���Ѿ���ȡ����������
			if (!strTemp.IsEmpty())
			{
				m_strCommands.AddTail(strTemp);//����whileѭ������ȡ�����������������һ������
				//������ִ������
				DoFtpCmd(); //ȥ������Щ����,���ֱ�Ӵ�������Ļ�����û������m_strCommandsz������л�����
			}
		}
		else
			break;
	}
}
void cSocket::parserCmd(CString &cmdName, CString &cmdArg)//�ӽ��ջ�������ȡ����
{
	CString strBuff = m_strCommands.RemoveHead();
	int nIndex = strBuff.Find(" ");
	if (nIndex != -1)
	{
		CString strPassword = strBuff;
		cmdName = strBuff.Left(nIndex);
		cmdArg = strBuff.Mid(nIndex + 1);
	}
	else
	{
		cmdName = strBuff;
	}
	if (strcmp(cmdName, "") == 0)
	{
		cmdName = "";
	}
	else {
		cmdName.MakeUpper();
	}
}
void cSocket::DoFtpCmd()//�����ݿͻ����ύ�ĸ������������Ӧ�Ĳ���
{

	CString cmdName;
	CString cmdArg;
	parserCmd(cmdName, cmdArg);

	int cmdIndex = FTP_CMD_USER;
	//��������
	for (cmdIndex; cmdIndex < FTP_CMD_ERROR; cmdIndex++)
	{
		if (state == TRUE && m_strUserName == "anonymous"&&m_bLoggedon==FALSE) {
			cmdArg = "123";
		}
		//��������
		if (cmdName == commandList[cmdIndex].ftp_cmd_name)
		{
			if (commandList[cmdIndex].ftp_cmd_arg && (cmdArg.IsEmpty()))
			{
				ResToClient("501 Syntax error:Invalid number of parameters.");

				return;
			}
			break;
		}
	}
	if (cmdIndex == FTP_CMD_ERROR)
	{
		ResToClient("501 Syntax error:Command not understood.");//��Ч������
		return;
	}
	if ((cmdIndex != FTP_CMD_USER && cmdIndex != FTP_CMD_PASS) && !m_bLoggedon)
		//û�гɹ���½֮ǰ��ϣ������ָ��
	{
		ResToClient("530 Please login with USER and PASS.");
		return;
	}

	switch (cmdIndex)
	{
	case FTP_CMD_USER://����ָ�����û���
	{
		cmdArg.MakeLower();//�����д�д��ĸ���Сд
		m_bLoggedon = FALSE;
		m_strUserName = cmdArg;
		CString strPeerAddress;
		UINT nPeerPort;
		GetPeerName(strPeerAddress, nPeerPort);
		ResToClient("331 User name ok,need password.");//���߷�����һ�����û��Ѿ�������
		break;
	}
	case FTP_CMD_PASS://ָ������
	{
		if (m_bLoggedon)//�Ƿ��Ѿ���¼
		{
			ResToClient("503 Login with USER first.");
		}
		else
		{
			if (state == TRUE && m_strUserName == "anonymous") {
				m_strCurrentDir = "/";
				//�ɹ���¼��ʾ
				m_bLoggedon = TRUE;
				ResToClient(_T("230 User sucessfully logged in."));
				break;
			}
			else {
				//����û���������
				UserInformation useraccount;
				if (cServer.cSPro.GetUserAccount(m_strUserName, useraccount))
				{

					if ((!useraccount.m_password.Compare(cmdArg) || useraccount.m_password.IsEmpty()))
					{
						//�����û���Ŀ¼
						m_strCurrentDir = "/";
						//�ɹ���¼��ʾ
						m_bLoggedon = TRUE;
						ResToClient(_T("230 User sucessfully logged in."));
						break;
					}
				}
				ResToClient(_T("530 Not logged in,user or password incorrect!"));
			}
		}
		break;
	}
	case FTP_CMD_QUIT:
	{
		//�����ټ���Ϣ���ͻ���
		CConnectThread *pThread = (CConnectThread*)m_parentThread;
		ResToClient(_T("220 %s"), ((Server*)pThread->m_parentWindowServer)->m_goodbyeInfor);
		Close();
		//֪ͨ�߳��Ѿ������ر�����
		pThread->PostThreadMessage(WM_THREADMESSAGE, 1, 0);
		break;
	}
	case FTP_CMD_SYST:
	{
		ResToClient("215 Windows FTP Server.");
		break;
	}
	case FTP_CMD_TYPE:
	{
		ResToClient(("200 Type set to %s"), cmdArg);
		break;
	}
	case FTP_CMD_CWD://�ı䵱ǰĿ¼
	{
		int nResult = cServer.cSPro.ChangeDirectory(m_strUserName, m_strCurrentDir, cmdArg);

		switch (nResult)
		{
		case 0:
			ResToClient(("250 CWD command successful.\"%s\" is current directory."), m_strCurrentDir);
			break;
		case 1:
			ResToClient(("550 CWD command failed.\"%s\":Permission denied."), cmdArg);
			break;
		default:
			ResToClient(("550 CWD command failed.\"%s\":Directory not found."), cmdArg);
			break;
		}
		break;
	}
	case FTP_CMD_PWD://��ӡ��ǰĿ¼
	{
		ResToClient(("257 \"%s\" is current directory."), m_strCurrentDir);
		break;
	}
	case FTP_CMD_PASV:
	{

		DestroyDataConnection();
		//�����µ������׽�������
		m_pDataSocket = new dataSocket(this, -1);
		//AfxMessageBox("��ʼ��PASV��");
		if (!m_pDataSocket->Create())
		{
			DestroyDataConnection();
			ResToClient(_T("421 Failed to create socket."));
			break;
		}

		//��ʼ����
		m_pDataSocket->Listen();
		m_pDataSocket->AsyncSelect();

		CString strIP, strTmp;
		UINT nPort;
		//��ȡ���ǵ�IP��ַ
		GetSockName(strIP, nPort);//strIP�Ǳ�����IP��ַ��nPort��21
								  //��ȡ�µļ����׽��ֵĶ˿ں�
		m_pDataSocket->GetSockName(strTmp, nPort);
		strIP.Replace(("."), (","));
		//֪ͨ�ͻ���Ҫ���ӵ�IP��ַ�Ͷ˿�*/
		ResToClient(("227 Entering Passive Mode(%s,%d,%d)."), strIP, nPort / 256, nPort % 256);
		m_bPassiveMode = TRUE;
		break;
	}
	case FTP_CMD_PORT:
	{
		// specify IP and port (PORT a1,a2,a3,a4,p1,p2) -> IP address a1.a2.a3.a4, port p1*256+p2.
		CString strSub;
		int nCount = 0;
		while (AfxExtractSubString(strSub, cmdArg, nCount++, ','))
		{
			switch (nCount)
			{
			case 1://a1
				m_strRemoteHost = strSub;
				m_strRemoteHost += ".";
				break;
			case 2://a2
				m_strRemoteHost += strSub;
				m_strRemoteHost += ".";
				break;
			case 3://a3
				m_strRemoteHost += strSub;
				m_strRemoteHost += ".";
				break;
			case 4://a4
				m_strRemoteHost += strSub;
				break;
			case 5://p1
				m_nRemotePort = 256 * atoi(strSub);
				break;
			case 6://p2
				m_nRemotePort += atoi(strSub);
				break;
			}
		}
		m_bPassiveMode = FALSE;
		ResToClient(("200 Port command successful."));
		break;
	}
	case FTP_CMD_LIST:
	{
		//������Ǳ���ģʽ����������ҪPORT����ָ���˿�
		if (!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
		{
			ResToClient(("503 Bad sequence of commands."));
			//AfxMessageBox(_T("503 Bad sequence of commands."));
		}

		else
		{
			//����û�û��ָ��·�����õ�ǰ·��
			if (cmdArg == "")
			{
				cmdArg = m_strCurrentDir;
			}

			else
			{
				//���������ļ�������·��
				CString strResult;
				int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_LIST, strResult);
				if (nResult == 0)
				{
					cmdArg = strResult;
				}
			}
			CString strListing;
			int nResult = cServer.cSPro.GetDirectoryList(m_strUserName, cmdArg, strListing);

			switch (nResult)
			{
			case 1:
				ResToClient(("550 Permission denied."));
				break;
			case 2:
				ResToClient(("550 Directory not found."));
				break;
			default:
				//�����׽������ӻ��·���б�
				if (!CreateDataConnection(0, strListing))
				{
					DestroyDataConnection();
				}
				break;
			}
		}
		break;
	case FTP_CMD_NOOP:
	{
		ResToClient("220 ok");
	}
	break;
	case FTP_CMD_RETR:
	{
		//������Ǳ���ģʽ����������ҪPORT����ָ���˿�
		if (!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
		{
			ResToClient(("503 Bad sequence of commands."));
			break;
		}

		CString strResult;
		int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DOWNLOAD, strResult);

		switch (nResult)
		{
		case 1:
			ResToClient(("550 Permission denied."));
			break;
		case 2:
			ResToClient(("550 File not found."));
			break;
		default:
			//Ϊ�ļ����䴴��һ���׽���
			if (!CreateDataConnection(1, strResult))
			{
				DestroyDataConnection();
			}
			break;
		}
		break;
	}
	case FTP_CMD_STOR:
	{
		//������Ǳ���ģʽ����������ҪPORT����ָ���˿�
		if (!m_bPassiveMode && (m_strRemoteHost == "" || m_nRemotePort == -1))
		{
			ResToClient(("503 Bad sequence of commands."));
			break;
		}

		CString strResult;
		int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_UPLOAD, strResult);

		switch (nResult)
		{
		case 1:
			ResToClient(("550 Permission denied."));
			break;
		case 2:
			ResToClient(("550 Filename invalid."));
			break;
		default:
			//Ϊ�ļ����䴴��һ���׽���
			if (!CreateDataConnection(2, strResult))
			{
				DestroyDataConnection();
			}
			break;
		}
		break;
	}
	case FTP_CMD_REST:
	{

		if (!IsNumeric(cmdArg.GetBuffer(cmdArg.GetLength())))
		{
			cmdArg.ReleaseBuffer();
			ResToClient(("501 Invalid numeric!"));
		}
		else
		{
			cmdArg.ReleaseBuffer();
			m_dwRestartOffset = atol(cmdArg);
			ResToClient(("350 Restarting at %d."), m_dwRestartOffset);
		}
		break;
	}
	case FTP_CMD_DELE:
	{
		CString strResult;
		int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);
		ResToClient((""+nResult));
		switch (nResult)
		{
		case 1:
			ResToClient(("550 Permission denied."));
			break;
		case 2:
			ResToClient(("550 File not found."));
			break;
		default:
			//ɾ���ļ�
			if (!DeleteFile(strResult))
			{
				ResToClient("450 Internal error deleting the file:\"%s\".", cmdArg);
			}
			else
			{
				ResToClient("250 File \"%s\" wsa deleted successfully.", cmdArg);
			}
			break;
		}
	}
	break;
	case FTP_CMD_BYE:
	{
		//�����ټ���Ϣ���ͻ���
		CConnectThread *pThread = (CConnectThread*)m_parentThread;
		ResToClient(_T("220 %s"), ((Server*)pThread->m_parentWindowServer)->m_goodbyeInfor);
		Close();
		//֪ͨ�߳��Ѿ������ر�����
		pThread->PostThreadMessage(WM_THREADMESSAGE, 1, 0);
		break;
	}
	case FTP_CMD_HELP:
	{
		//����ͻ��˲�ָ����������ʾ��������
		if (cmdArg == "")
		{
			CString strResponse = "214-The following commands are recognized:\r\n";
			//�������б��в�������
			for (int i = FTP_CMD_ABOR; i < FTP_CMD_ERROR; i++)
			{
				strResponse += commandList[i].ftp_cmd_name;
				strResponse += "\r\n";
			}
			strResponse += "214 HELP command successful.";
			ResToClient(strResponse);
		}
		else
		{
			int nHelpCmd;
			//�������б��в�������
			for (nHelpCmd = FTP_CMD_ABOR; nHelpCmd < FTP_CMD_ERROR; nHelpCmd++)
			{
				//�ж��Ƿ��ҵ�����
				if (cmdArg.CompareNoCase(commandList[nHelpCmd].ftp_cmd_name) == 0)
				{
					break;
				}
			}
			if (nHelpCmd != FTP_CMD_ERROR)
			{
				//��ʾ���������Ϣ
				ResToClient("214 %s", commandList[nHelpCmd].ftp_cmd_desc);
			}
			else
			{
				ResToClient("501 Unknown command %s", cmdArg);
			}
		}
		break;
	}
	case FTP_CMD_ABOR:
	{
		if (m_pDataSocket)
		{
			if (m_pDataSocket->GetStatus() != XFERMODE_IDLE)
			{
				ResToClient("426 Data connection closed.");
			}
			//������������
			m_parentThread->PostThreadMessage(WM_THREADMESSAGE, 0, 0);
		}
		ResToClient("226 ABOR command successful.");
		break;
	}
	case FTP_CMD_RNFR:
	{
		//�û����������ļ�
		CString strResult;
		int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

		if (nResult == 0)
		{
			m_strRenameFile = strResult;
			m_bRenameFile = TRUE;
			ResToClient("350 File exists, ready for destination name.");
			break;
		}
		else
		{

			nResult = cServer.cSPro.CheckDirectory(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

			switch (nResult)
			{
			case 0:
				m_strRenameFile = strResult;
				m_bRenameFile = FALSE;
				ResToClient("350 Directory exists, ready for destination name.");
				break;
			case 1:
				ResToClient("550 Permission denied.");
				break;
			default:
				ResToClient("550 File/directory not found.");
				break;
			}
		}
	}
	break;

	// �������ļ�����Ŀ¼
	case FTP_CMD_RNTO:
	{
		if (m_strRenameFile.IsEmpty())
		{
			ResToClient("503 Bad sequence of commands.");
			break;
		}
		if (m_bRenameFile)
		{
			CString strResult;
			// ���Ŀ���ļ���
			int nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

			switch (nResult)
			{
			case 0:
				ResToClient("550 File already exists.");
				break;
			case 1:
				ResToClient("550 Permission denied.");
				break;
			default:
				// �������ļ�
				if (!MoveFile(m_strRenameFile, strResult))
				{
					ResToClient("450 Internal error renaming the file: \"%s\".", m_strRenameFile);
				}
				else
				{
					ResToClient("250 File \"%s\" renamed successfully.", m_strRenameFile);
				}
				break;
			}
		}
		else
		{
			CString strResult;
			// ���Ŀ��·����
			int  nResult = cServer.cSPro.CheckDirectory(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

			switch (nResult)
			{
			case 0:
				ResToClient("550 Directory already exists.");
				break;
			case 1:
				ResToClient("550 Permission denied.");
				break;
			case 3:
				ResToClient("550 Directory invalid.");
				break;
			default:
				// ������·��
				if (!MoveFile(m_strRenameFile, strResult))
				{
					ResToClient("450 Internal error renaming the directory: \"%s\".", m_strRenameFile);
				}
				else
				{
					ResToClient("250 Directory \"%s\" renamed successfully.", m_strRenameFile);
				}
				break;
			}
		}
	}
	break;
	case FTP_CMD_CDUP:
	{
		cmdArg = "..";
		break;
	}
	case FTP_CMD_RMD:
	{
		CString strResult;
		int nResult = cServer.cSPro.CheckDirectory(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

		switch (nResult)
		{
		case 1:
			ResToClient("550 Permission denied.");
			break;
		case 2:
			ResToClient("550 Directory not found.");
			break;
		default:
			//ɾ��Ŀ¼
			if (!RemoveDirectory(strResult))
			{
				if (GetLastError() == ERROR_DIR_NOT_EMPTY)
				{
					ResToClient("550 Directory not empty.");
				}
				else
				{
					ResToClient("450 Internal error deleting the directory.");
				}
			}
			else
			{
				ResToClient("250 Directory deleted successfully.");
			}
			break;
		}
	}
	break;
	case FTP_CMD_MKD:
	{
		CString strResult;
		int  nResult = cServer.cSPro.CheckFileName(m_strUserName, cmdArg, m_strCurrentDir, FTP_DELETE, strResult);

		switch (nResult)
		{
		case 0:
			ResToClient("550 Directory already exists.");
			break;
		case 1:
			ResToClient("550 Can't create directory.Permission denied.");
			break;
		default:
			//����Ŀ¼
			if (!MakeSureDirectoryPathExists(strResult))
			{
				ResToClient("450 Internal error creating the directory.");
			}
			else
			{
				ResToClient("250 Directory created successfully.");
			}
			break;
		}
	}
	break;
	}
	}
}
/*------------------������������------------------*/
BOOL cSocket::CreateDataConnection(int nTransferType, LPCTSTR lpszData)
{
	if (!m_bPassiveMode)
	{
		m_pDataSocket = new dataSocket(this, nTransferType);
		if (m_pDataSocket->Create())
		{
			m_pDataSocket->AsyncSelect();
			m_pDataSocket->SetRestartOffset(m_dwRestartOffset);//���ӵ�Զ��վ��
			m_pDataSocket->SetData(lpszData);

			if (m_pDataSocket->Connect(m_strRemoteHost, m_nRemotePort))
			{
				if (GetLastError() != WSAEWOULDBLOCK)
				{
					ResToClient(("425 Can't open data connection."));

					return FALSE;
				}
			}

			switch (nTransferType)
			{
			case 0:
				ResToClient(("150 Opening ASCII mode data connection for directory list."));
				break;
			case 1:
			case 2:
				ResToClient(("150 Opening BINARY mode data connection for file transfer."));
				break;
			}
		}
		else
		{
			AfxMessageBox(("421 Failed to create data connection socket."));
			return FALSE;
		}
	}
	else
	{
		m_pDataSocket->SetRestartOffset(m_dwRestartOffset);
		m_pDataSocket->SetData(lpszData);
		m_pDataSocket->SetTransferType(nTransferType, TRUE);
		//��3��ִ����Ż�ȥִ��m_pDataSocket��OnAccept()������������SetTransferType��������
		//��Ҫ�ж��ڱ���ģʽ�������������ͻ�����ģʽ���£��Ƿ�ͻ��������ˣ�������жϵĻ���
		//OnAccept()��������ִ�У���û�пͻ��������ӣ���Ҳ�ǿ�ʼһֱ�޷������ݷ���ȥ��ԭ��!!
	}
	return TRUE;
}
/*-----------------------------------*/


void cSocket::DestroyDataConnection() //����һ����Ϣ��Ӧ������Ҳ����һ���̶߳������ȥ���ղ�����һ����Ϣ
{
	if (!m_pDataSocket)
		return;

	delete m_pDataSocket;

	// reset transfer status
	m_pDataSocket = NULL;
	m_strRemoteHost = "";
	m_nRemotePort = -1;
	m_dwRestartOffset = 0;
	m_bPassiveMode = FALSE;
}
BOOL cSocket::ResToClient(LPCTSTR lpszStatus, ...)//���ͻ���������Ӧ
{
	CString str;
	va_list args;
	va_start(args, lpszStatus);
	str.FormatV(lpszStatus, args);
	va_end(args);
	//�ж������Ƿ��ڻ״̬
	CString response = str + "\r\n";
	if (CSocket::Send(response, strlen(response), 0) == SOCKET_ERROR)
	{
		Close();//�ر��߳�����
		return FALSE;
	}
	return TRUE;
}
BOOL cSocket::MakeSureDirectoryPathExists(LPCTSTR lpszDirPath)//�������Ŀ¼
{
	CString strDirPath = lpszDirPath;
	int nPos = 0;//lpszDirPathΪҪ������Ŀ¼���硰c:\dir1\dir2\"
	while ((nPos = strDirPath.Find('\\', nPos + 1)) != -1)
	{
		CreateDirectory(strDirPath.Left(nPos), NULL);//CreateDirectoryΪ�������㺯��Ŀ¼
	}
	return CreateDirectory(strDirPath, NULL);
}
BOOL cSocket::IsNumeric(char *buff)//�������Ƿ�Ϊ���ֻ������ַ���
{
	//validate data ��֤����
	char *ptr = buff;
	while (*ptr != '\0')
	{
		if (isdigit(*ptr))//�����ַ���true
		{
			ptr++;
		}
		else
			return FALSE;
	}
	return TRUE;
}


